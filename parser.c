/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file parser.c
 * @authors Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67), Michal Cejpek (xcejpe05), Jan Zajíček (xzajic22)
 * @brief Parser for IFJcode22
 * @date 2022-10-22
 */

#include "parser.h"
#include "code_generator.h"

Token nextToken;

void printParserError(Token token, char * message) {
    printTokenPreview(token);
    fprintf(stderr, "PARSER ERROR: %s on line %d, column %d\n", message, token.line, token.column);
}

//bool precedence_tb[PREC_TB_SIZE][PREC_TB_SIZE] = {
//  */      +-     .      <>     !=     =    empty    !     &&     ||
//  {false, false, false, false, false, false, false, true , false, false},   // */ 
/*  {true , false, false, false, false, false, false, true , false, false},   // +- 
  {true , false, false, false, false, false, false, true , false, false},   // .  
  {true , true , true , false, false, false, false, true , false, false},   // <> 
  {true , true , true , true , false, false, false, true , false, false},   // != 
  {true , true , true , true , true , true , false, true , true , true },   // = 
  {true , true , true , true , true , true , false, true , true , true },   // empty
  {true , false, false, false, false, false, false, false, false, false},   // !
  {true , true , true , true , true , false, false, true , false, false},   // &&
  {true , true , true , true , true , false, false, true , true , false},   // ||
};  */

int getPrecedence(TokenType type) {
    switch (type) {
    case TOKEN_NEGATE:
        return 10;
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
        return 9;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_CONCATENATE:
        return 8;
    case TOKEN_LESS:
    case TOKEN_LESS_OR_EQUALS:
    case TOKEN_GREATER:
    case TOKEN_GREATER_OR_EQUALS:
        return 7;
    case TOKEN_EQUALS:
    case TOKEN_NOT_EQUALS:
        return 6;
    case TOKEN_AND:
        return 5;
    case TOKEN_OR:
        return 4;
    case TOKEN_NULL_COALESCING:
        return 3;
    //case TOKEN_QUESTIONMARK: Figure out how to get precedence of ternary operator
    //    return 2;
    case TOKEN_ASSIGN:
    case TOKEN_PLUS_ASSIGN:
    case TOKEN_MINUS_ASSIGN:
    case TOKEN_CONCATENATE_ASSIGN:
    case TOKEN_MULTIPLY_ASSIGN:
    case TOKEN_DIVIDE_ASSIGN:
        return 1;
    default:
        return -1;
    }
}

bool is_constant(TokenType tokenType) {
    return 
        tokenType == TOKEN_BOOL ||
        tokenType == TOKEN_FLOAT ||
        tokenType == TOKEN_INTEGER ||
        tokenType == TOKEN_NULL ||
        tokenType == TOKEN_STRING;
}

bool is_binary_operator(TokenType tokenType) {
    return 
        tokenType == TOKEN_PLUS ||
        tokenType == TOKEN_MINUS ||
        tokenType == TOKEN_MULTIPLY ||
        tokenType == TOKEN_DIVIDE ||
        tokenType == TOKEN_CONCATENATE ||
        tokenType == TOKEN_LESS ||
        tokenType == TOKEN_LESS_OR_EQUALS ||
        tokenType == TOKEN_GREATER ||
        tokenType == TOKEN_GREATER_OR_EQUALS ||
        tokenType == TOKEN_EQUALS ||
        tokenType == TOKEN_NOT_EQUALS ||
        tokenType == TOKEN_ASSIGN ||
        tokenType == TOKEN_PLUS_ASSIGN ||
        tokenType == TOKEN_MINUS_ASSIGN ||
        tokenType == TOKEN_CONCATENATE_ASSIGN ||
        tokenType == TOKEN_MULTIPLY_ASSIGN ||
        tokenType == TOKEN_DIVIDE_ASSIGN ||
        tokenType == TOKEN_AND ||
        tokenType == TOKEN_OR;
}

bool is_right_associative(TokenType tokenType) {
    return 
        tokenType == TOKEN_ASSIGN ||
        tokenType == TOKEN_PLUS_ASSIGN ||
        tokenType == TOKEN_MINUS_ASSIGN ||
        tokenType == TOKEN_CONCATENATE_ASSIGN ||
        tokenType == TOKEN_MULTIPLY_ASSIGN ||
        tokenType == TOKEN_DIVIDE_ASSIGN;
}

bool is_unary_operator(TokenType tokenType) {
    return 
        tokenType == TOKEN_NEGATE ||
        tokenType == TOKEN_PLUS ||
        tokenType == TOKEN_MINUS ||
        tokenType == TOKEN_INCREMENT ||
        tokenType == TOKEN_DECREMENT;
}

char * decodeString(char * text) {
    int len = strlen(text);
    char * result = malloc(len + 1);
    char * retAddr = result;
    text++;
    while(*text != '"' && *text != '\0') {
        if(*text == '\\') {
            text++;
            switch(*text) {
                case '"':
                    *result = '"';
                    break;
                case 'n':
                    *result = '\n';
                    break;
                case 't':
                    *result = '\t';
                    break;
                case '\\':
                    *result = '\\';
                    break;
                case '$':
                    *result = '$';
                    break;
                case 'x':
                    if(isxdigit(text[1]) && isxdigit(text[2])) {
                        char hex[3];
                        hex[0] = text[1];
                        hex[1] = text[2];
                        hex[2] = '\0';
                        *result = (char)strtol(hex, NULL, 16);
                        if(*result == 0) {
                            goto undoHexParse;
                        }
                        text += 2;
                    } else {
                        undoHexParse:
                        text--;
                        *result = *text;
                    }
                    break;
                case '0' ... '3':
                    if('0' <= text[1] && text[1] <= '7' && '0' <= text[2] && text[2] <= '7') {
                        char oct[4];
                        oct[0] = text[0];
                        oct[1] = text[1];
                        oct[2] = text[2];
                        oct[3] = '\0';
                        *result = (char)strtol(oct, NULL, 8);
                        if(*result == 0) {
                            goto undoOctParse;
                        }
                        text += 2;
                    } else {
                        undoOctParse:
                        text--;
                        *result = *text;
                    }
                    break;
                default:
                    text--;
                    *result = *text;
                    break;
            }
        } else if(*text == '$') {
            fprintf(stderr, "Error: String interpolation is not allowed.\n");
            exit(1);
        } else {
            *result = *text;
        }
        result++;
        text++;
    }
    result[0] = 0;
    return retAddr;
}

bool is_first_terminal_expression(TokenType tokenType) {
    return 
        tokenType == TOKEN_OPEN_BRACKET ||
        tokenType == TOKEN_IDENTIFIER ||
        tokenType == TOKEN_VARIABLE ||
        is_constant(tokenType);
}

bool parse_terminal_expression(Expression ** expression) {
    if(! is_first_terminal_expression(nextToken.type)) {
        printParserError(nextToken, "Expected terminal expression");
        return false;
    }
    
    *expression = NULL;
    if(nextToken.type == TOKEN_OPEN_BRACKET) {
        nextToken = getNextToken();
        if(!parse_expression(expression, 0)) return false;
        (*expression)->isLValue = false;
        if(nextToken.type != TOKEN_CLOSE_BRACKET) {
            printParserError(nextToken, "Expected closing bracket");
            return false;
        }
        nextToken = getNextToken();
        return true;
    }
    if(nextToken.type == TOKEN_IDENTIFIER) {
        if(!parse_function_call(expression)) return false;
        return true;
    }
    if(nextToken.type == TOKEN_VARIABLE) {
        Expression__Variable * variable = Expression__Variable__init();
        *expression = (Expression*)variable;
        variable->name = getTokenTextPermanent(nextToken);
    } else if(is_constant(nextToken.type)) {
        Expression__Constant * constant = Expression__Constant__init();
        *expression = (Expression*)constant;
        Type type;
        type.isRequired = true;
        if(nextToken.type == TOKEN_INTEGER) {
            type.type = TYPE_INT;
            constant->type = type;
            constant->value.integer = atoll(getTokenText(nextToken));
        } else if(nextToken.type == TOKEN_FLOAT) {
            type.type = TYPE_FLOAT;
            constant->type = type;
            constant->value.real = atof(getTokenText(nextToken));
        } else if(nextToken.type == TOKEN_STRING) {
            type.type = TYPE_STRING;
            constant->type = type;
            constant->value.string = decodeString(getTokenText(nextToken));
        } else if(nextToken.type == TOKEN_BOOL) {
            type.type = TYPE_BOOL;
            constant->type = type;
            char* tokenText = getTokenText(nextToken);
            if(strcmp(tokenText, "true") == 0) {
                constant->value.boolean = true;
            } else {
                constant->value.boolean = false;
            }
        } else if(nextToken.type == TOKEN_NULL) {
            type.type = TYPE_NULL;
            constant->type = type;
        }
    }
    nextToken = getNextToken();
    return true;
}

bool is_first_expression(TokenType tokenType) {
    return 
        is_unary_operator(tokenType) ||
        is_first_terminal_expression(tokenType);
}

bool parse_expression(Expression ** expression, int previousPrecedence) {
    if(! is_first_expression(nextToken.type)) {
        printParserError(nextToken, "Expected expression");
        return false;
    }

    if(is_unary_operator(nextToken.type)) {
        Token operatorToken = nextToken;
        if(nextToken.type == TOKEN_PLUS) {
            Expression__BinaryOperator * binaryOperator = Expression__BinaryOperator__init();
            *expression = (Expression*)binaryOperator;
            binaryOperator->operator = TOKEN_PLUS;
            nextToken = getNextToken();
            if(!parse_expression(&binaryOperator->rSide, 8)) return false; // TODO handle priority correctly
            Expression__Constant * constant = Expression__Constant__init();
            binaryOperator->lSide = (Expression*)constant;
            constant->type.type = TYPE_INT;
            constant->type.isRequired = true;
            constant->value.integer = 0;
            return true;
        } else if(nextToken.type == TOKEN_MINUS) {
            Expression__BinaryOperator * binaryOperator = Expression__BinaryOperator__init();
            *expression = (Expression*)binaryOperator;
            binaryOperator->operator = TOKEN_MINUS;
            nextToken = getNextToken();
            if(!parse_expression(&binaryOperator->rSide, 8)) return false; // TODO handle priority correctly
            Expression__Constant * constant = Expression__Constant__init();
            binaryOperator->lSide = (Expression*)constant;
            constant->type.type = TYPE_INT;
            constant->type.isRequired = true;
            constant->value.integer = 0;
            return true;
        } else if(nextToken.type == TOKEN_INCREMENT) {
            Expression__BinaryOperator * binaryOperator = Expression__BinaryOperator__init();
            *expression = (Expression*)binaryOperator;
            binaryOperator->operator = TOKEN_ASSIGN;
            nextToken = getNextToken();
            if(!parse_expression(&binaryOperator->lSide, 8)) return false; // TODO handle priority correctly
            if(!binaryOperator->lSide->isLValue) {
                printParserError(operatorToken, "Expected l-value");
                return false;
            }
            Expression__BinaryOperator * binaryOperator2 = Expression__BinaryOperator__init();
            binaryOperator->rSide = (Expression*)binaryOperator2;
            binaryOperator2->lSide = (Expression*)binaryOperator->lSide->super.duplicate(&binaryOperator->lSide->super);
            Expression__Constant * constant = Expression__Constant__init();
            constant->type.type = TYPE_INT;
            constant->type.isRequired = true;
            constant->value.integer = 1;
            binaryOperator2->rSide = (Expression*)constant;
            binaryOperator2->operator = TOKEN_PLUS;
            return true;
        } else if(nextToken.type == TOKEN_DECREMENT) {
            Expression__BinaryOperator * binaryOperator = Expression__BinaryOperator__init();
            *expression = (Expression*)binaryOperator;
            binaryOperator->operator = TOKEN_ASSIGN;
            nextToken = getNextToken();
            if(!parse_expression(&binaryOperator->lSide, 8)) return false; // TODO handle priority correctly
            if(!binaryOperator->lSide->isLValue) {
                printParserError(operatorToken, "Expected l-value");
                return false;
            }
            Expression__BinaryOperator * binaryOperator2 = Expression__BinaryOperator__init();
            binaryOperator->rSide = (Expression*)binaryOperator2;
            binaryOperator2->lSide = (Expression*)binaryOperator->lSide->super.duplicate(&binaryOperator->lSide->super);
            Expression__Constant * constant = Expression__Constant__init();
            constant->type.type = TYPE_INT;
            constant->type.isRequired = true;
            constant->value.integer = 1;
            binaryOperator2->rSide = (Expression*)constant;
            binaryOperator2->operator = TOKEN_MINUS;
            return true;
        } else {
            Expression__UnaryOperator * operator = Expression__UnaryOperator__init();
            operator->operator = operatorToken.type;
            *expression = (Expression*)operator;
            nextToken = getNextToken();
            if(!parse_expression(&operator->rSide, getPrecedence(operatorToken.type))) return false;
        }
        return true;
    }
    if(!parse_terminal_expression(expression)) return false;
    while(is_binary_operator(nextToken.type)) {
        Token operatorToken = nextToken;
        int nextPrecedence = getPrecedence(operatorToken.type);
        if(previousPrecedence < nextPrecedence || (previousPrecedence == nextPrecedence && is_right_associative(operatorToken.type))) {
            Expression__BinaryOperator * operator = Expression__BinaryOperator__init();
            operator->operator = operatorToken.type;
            operator->lSide = *expression;
            Expression ** rSide = &operator->rSide;
            if(operatorToken.type == TOKEN_PLUS_ASSIGN) {
                Expression__BinaryOperator * virtualOperator = Expression__BinaryOperator__init();
                virtualOperator->operator = TOKEN_PLUS;
                virtualOperator->lSide = (Expression*) operator->lSide->super.duplicate(&operator->lSide->super);
                *rSide = (Expression*) virtualOperator;
                rSide = &virtualOperator->rSide;
                operator->operator = TOKEN_ASSIGN;
            } else if(operatorToken.type == TOKEN_MINUS_ASSIGN) {
                Expression__BinaryOperator * virtualOperator = Expression__BinaryOperator__init();
                virtualOperator->operator = TOKEN_MINUS;
                virtualOperator->lSide = (Expression*) operator->lSide->super.duplicate(&operator->lSide->super);
                *rSide = (Expression*) virtualOperator;
                rSide = &virtualOperator->rSide;
                operator->operator = TOKEN_ASSIGN;
            } else if(operatorToken.type == TOKEN_CONCATENATE_ASSIGN) {
                Expression__BinaryOperator * virtualOperator = Expression__BinaryOperator__init();
                virtualOperator->operator = TOKEN_CONCATENATE;
                virtualOperator->lSide = (Expression*) operator->lSide->super.duplicate(&operator->lSide->super);
                *rSide = (Expression*) virtualOperator;
                rSide = &virtualOperator->rSide;
                operator->operator = TOKEN_ASSIGN;
            } else if(operatorToken.type == TOKEN_MULTIPLY_ASSIGN) {
                Expression__BinaryOperator * virtualOperator = Expression__BinaryOperator__init();
                virtualOperator->operator = TOKEN_MULTIPLY;
                virtualOperator->lSide = (Expression*) operator->lSide->super.duplicate(&operator->lSide->super);
                *rSide = (Expression*) virtualOperator;
                rSide = &virtualOperator->rSide;
                operator->operator = TOKEN_ASSIGN;
            } else if(operatorToken.type == TOKEN_DIVIDE_ASSIGN) {
                Expression__BinaryOperator * virtualOperator = Expression__BinaryOperator__init();
                virtualOperator->operator = TOKEN_DIVIDE;
                virtualOperator->lSide = (Expression*) operator->lSide->super.duplicate(&operator->lSide->super);
                *rSide = (Expression*) virtualOperator;
                rSide = &virtualOperator->rSide;
                operator->operator = TOKEN_ASSIGN;
            }
            if(operator->operator == TOKEN_ASSIGN && !operator->lSide->isLValue) {
                printParserError(nextToken, "Cannot assign to non-lvalue");
                return false;
            }
            *expression = (Expression*)operator;
            nextToken = getNextToken();
            if(!parse_expression(rSide, nextPrecedence)) return false;
        } else {
            break;
        }
    }
    return true;
}

extern bool parse_statement();

bool is_first_statement(TokenType tokenType) {
    return 
        tokenType == TOKEN_IF ||
        tokenType == TOKEN_WHILE ||
        tokenType == TOKEN_RETURN ||
        tokenType == TOKEN_FOR ||
        tokenType == TOKEN_BREAK ||
        tokenType == TOKEN_CONTINUE ||
        tokenType == TOKEN_OPEN_BRACKET ||
        is_first_expression(tokenType);
}

bool is_first_statement_list(TokenType tokenType) {
    return is_first_statement(tokenType) ||
        tokenType == TOKEN_CLOSE_CURLY_BRACKET ||
        tokenType == TOKEN_EOF;   
}
bool parse_statement_list(StatementList ** statementListRet) {
    if(! is_first_statement_list(nextToken.type)) {
        printParserError(nextToken, "Expected statement");
        return false;
    }
    StatementList * statementList = StatementList__init();
    *statementListRet = statementList;
    while(is_first_statement(nextToken.type)) {
        Statement * statement;
        bool success = parse_statement(&statement);
        StatementList__addStatement(statementList, statement);
        if(!success) return false;
    }
    return true;
}

bool is_first_if(TokenType tokenType) {
    return 
        tokenType == TOKEN_IF ||
        tokenType == TOKEN_ELSEIF;     
}

bool parse_if(StatementIf ** statementIfRet) {
    if(! is_first_if(nextToken.type)) {
        printParserError(nextToken, "Expected if statement");
        return false;
    }

    StatementIf * statementIf = StatementIf__init();
    *statementIfRet = statementIf;
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after if");
        return false;
    }
    nextToken = getNextToken(&statementIf->condition);
    if(!parse_expression(&statementIf->condition, 0)) return false;
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) if");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
        printParserError(nextToken, "Missing { after if");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_statement_list((StatementList**)&statementIf->ifBody)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after if");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type == TOKEN_ELSE) {
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
            printParserError(nextToken, "Missing { after else");
            return false;
        }
        nextToken = getNextToken();
        if(!parse_statement_list((StatementList**)&statementIf->elseBody)) return false;
        if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
            printParserError(nextToken, "Missing } after else");
            return false;
        }
        nextToken = getNextToken();
    } else if(nextToken.type == TOKEN_ELSEIF) { // Add if statement to the else branch
        if(!parse_if((StatementIf**)&statementIf->elseBody)) return false;
    } else {
        statementIf->elseBody = (Statement*)StatementList__init();
    }
    return true;
}


bool is_first_while(TokenType tokenType) {
    return tokenType == TOKEN_WHILE;     
}

bool parse_while(StatementWhile ** statementWhileRet) {
    if(! is_first_while(nextToken.type)) {
        printParserError(nextToken, "Expected while");
        return false;
    }

    StatementWhile * statementWhile = StatementWhile__init();
    *statementWhileRet = statementWhile;
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after while");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_expression(&statementWhile->condition, 0)) return false;
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) after while");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
        printParserError(nextToken, "Missing { after while");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_statement_list((StatementList**)&statementWhile->body)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after while");
        return false;
    }
    nextToken = getNextToken();
    return true;
}
bool parse_for(StatementFor ** statementForRet) {
    StatementFor * statementFor = StatementFor__init();
    *statementForRet = statementFor;
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after for");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type == TOKEN_SEMICOLON) {
        statementFor->init = NULL;
    } else {
        if(!parse_expression(&statementFor->init, 0)) return false;
    }
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Missing ; after for init");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type == TOKEN_SEMICOLON) {
        statementFor->condition = NULL;
    } else {
        if(!parse_expression(&statementFor->condition, 0)) return false;
    }
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Missing ; after for condition");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type == TOKEN_CLOSE_BRACKET) {
        statementFor->increment = NULL;
    } else {
        if(!parse_expression(&statementFor->increment, 0)) return false;
    }
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) after for");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
        printParserError(nextToken, "Missing { after for");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_statement_list((StatementList**)&statementFor->body)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after for");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

/**
 * @brief Parses continue statement
 * 
 * @param statementContinueRet 
 * @return true 
 * @return false 
 */
bool parse_continue(StatementContinue ** statementContinueRet) {
    StatementContinue * statementContinue = StatementContinue__init();
    *statementContinueRet = statementContinue;
    nextToken = getNextToken();
    // optional parameter
    if(nextToken.type != TOKEN_SEMICOLON) {
        // parameter must be integer
        if(nextToken.type !=  TOKEN_INTEGER) {
            printParserError(nextToken, "Continue parameter must be integer");
            return false;
        }
        statementContinue->depth = atoll(getTokenText(nextToken));
        // if parameter is integer, get next token
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_SEMICOLON) {
            printParserError(nextToken, "Missing ; after continue");
            return false;
        }
        nextToken = getNextToken();
        return true;
    }
    statementContinue->depth = 1;
    if (nextToken.type == TOKEN_SEMICOLON) {
        nextToken = getNextToken();
        return true;
    }
    printParserError(nextToken, "Missing ; after continue");
    return false;
}

bool parse_break(StatementBreak ** statementBreakRet) {
    StatementBreak * statementBreak = StatementBreak__init();
    *statementBreakRet = statementBreak;
    nextToken = getNextToken();
    // optional parameter
    if(nextToken.type != TOKEN_SEMICOLON) {
        // parameter must be integer
        if(nextToken.type != TOKEN_INTEGER) {
            printParserError(nextToken, "Break parameter must be integer");
            return false;
        }
        statementBreak->depth = atoll(getTokenText(nextToken));
        // if parameter is integer, get next token
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_SEMICOLON) {
            printParserError(nextToken, "Missing ; after break");
            return false;
        }
        nextToken = getNextToken();
        return true;
    }
    statementBreak->depth = 1;
    if (nextToken.type == TOKEN_SEMICOLON) {
        nextToken = getNextToken();
        return true;
    }
    printParserError(nextToken, "Missing ; after break");
    return false;
}

bool is_first_function_call_arguments(TokenType tokenType) {
    return 
        tokenType == TOKEN_CLOSE_BRACKET ||
        is_first_expression(tokenType);  
}

bool parse_function_arguments(Expression__FunctionCall * functionCall) {
    if(! is_first_function_call_arguments(nextToken.type)) {
        printParserError(nextToken, "Expected expression or closing bracket");
        return false;
    }
    
    if(nextToken.type == TOKEN_CLOSE_BRACKET) return true;
    Expression * expression;
    bool success = parse_expression(&expression, 0);
    Expression__FunctionCall__addArgument(functionCall, expression);
    if(!success) return false;
    while(nextToken.type == TOKEN_COMMA) {
        nextToken = getNextToken();
        success = parse_expression(&expression, 0);
        Expression__FunctionCall__addArgument(functionCall, expression);
        if(!success) return false;
    }
    return true;
}

bool is_first_function_call(TokenType tokenType) {
    return tokenType == TOKEN_IDENTIFIER;
}

bool parse_function_call(Expression__FunctionCall ** functionCallRet) {
    if(! is_first_function_call(nextToken.type)) {
        printParserError(nextToken, "Expected function name");
        return false;
    }
    Expression__FunctionCall * functionCall = Expression__FunctionCall__init();
    *functionCallRet = functionCall;
    functionCall->name = getTokenTextPermanent(nextToken);
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after function call");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_function_arguments(functionCall)) return false;
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) after function call");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool is_first_return(TokenType tokenType) {
    return tokenType == TOKEN_RETURN;
}

bool parse_return(StatementReturn ** statementReturnRet) {
    if(! is_first_return(nextToken.type)) {
        printParserError(nextToken, "Expected return");
        return false;
    }
    StatementReturn * statementReturn = StatementReturn__init();
    *statementReturnRet = statementReturn;
    nextToken = getNextToken();
    // expression after return is optional
    if(nextToken.type != TOKEN_SEMICOLON) {
        if(!parse_expression(&statementReturn->expression, 0)) return false;
    }
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Missing ; after return");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

// is_first_statement is above parse_statement_list

bool parse_statement(Statement ** retStatement) {
    if(! is_first_statement(nextToken.type)) {
        printParserError(nextToken, "Expected statement");
        return false;
    }

    switch (nextToken.type) {
        case TOKEN_IF:
            return parse_if((StatementIf**)retStatement);
        case TOKEN_WHILE:
            return parse_while((StatementWhile**)retStatement);
        case TOKEN_RETURN:
            return parse_return((StatementReturn**)retStatement);
        case TOKEN_FOR:
            return parse_for((StatementFor**)retStatement);
        case TOKEN_BREAK:
            return parse_break((StatementBreak**)retStatement);
        case TOKEN_CONTINUE:
            return parse_continue((StatementContinue**)retStatement);
        default:
            if(is_first_expression(nextToken.type)) {
                if(!parse_expression((Expression**)retStatement, 0)) return false;
                if(nextToken.type != TOKEN_SEMICOLON) {
                    printParserError(nextToken, "Missing ; after expression");
                    return false;
                }
                nextToken = getNextToken();
                return true;
            }
            return false;
    }
    return true;
}

bool is_first_function_parameters(TokenType tokenType) {
    return 
        tokenType == TOKEN_TYPE ||
        tokenType == TOKEN_CLOSE_BRACKET;
}

bool parse_function_parameters(Function * function) {
    if(! is_first_function_parameters(nextToken.type)) {
        printParserError(nextToken, "Expected function parameters");
        return false;
    }

    if(nextToken.type != TOKEN_TYPE) {
        return true;
    }
    Type firstType = tokenToType(nextToken);
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_VARIABLE) {
        printParserError(nextToken, "Expected variable after type");
        return false;
    }
    Function__addParameter(function, firstType, getTokenTextPermanent(nextToken));
    nextToken = getNextToken();
    while(nextToken.type == TOKEN_COMMA) {
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_TYPE) {
            printParserError(nextToken, "Expected type after comma");
            return false;
        }
        Type type = tokenToType(nextToken);
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_VARIABLE) {
            printParserError(nextToken, "Expected variable after type");
            return false;
        }
        Function__addParameter(function, type, getTokenTextPermanent(nextToken));
        nextToken = getNextToken();
    }
    return true;
}

bool is_first_function(TokenType tokenType) {
    return tokenType == TOKEN_FUNCTION;
}

bool parse_function(Function ** retFunction) {
    if(! is_first_function(nextToken.type)) {
        printParserError(nextToken, "Expected function");
        return false;
    }

    Function * function = Function__init();
    *retFunction = function;
    Token functionIdentifier = getNextToken();
    if(functionIdentifier.type != TOKEN_IDENTIFIER) {
        printParserError(functionIdentifier, "Missing function name");
        return false;
    }
    function->name = getTokenTextPermanent(functionIdentifier);
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after function name");
        return false;
    }
    nextToken = getNextToken();
    parse_function_parameters(function);
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) after function");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_COLON) {
        printParserError(nextToken, "Missing : after function");
        return false;
    }
    Token returnType = getNextToken();
    if(returnType.type != TOKEN_TYPE && returnType.type != TOKEN_VOID) {
        printParserError(returnType, "Missing return type of function");
        return false;
    }
    function->returnType = tokenToType(returnType);
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
        printParserError(nextToken, "Missing { after function");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_statement_list((StatementList**)&function->body)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after function");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

void loadBuiltinFunctions(Table * functionTable) {
    Function * reads = Function__init();
    reads->name = "reads";
    reads->returnType.isRequired = false;
    reads->returnType.type = TYPE_STRING;
    table_insert(functionTable, "reads", reads);
    Function * readi = Function__init();
    readi->name = "readi";
    readi->returnType.isRequired = false;
    readi->returnType.type = TYPE_INT;
    table_insert(functionTable, "readi", readi);
    Function * readf = Function__init();
    readf->name = "readf";
    readf->returnType.isRequired = false;
    readf->returnType.type = TYPE_FLOAT;
    table_insert(functionTable, "readf", readf);
    
    Function * write = Function__init();
    write->name = "write";
    write->returnType.type = TYPE_VOID;
    table_insert(functionTable, "write", write);

    Function * floatval = Function__init();
    floatval->name = "floatval";
    floatval->returnType.isRequired = true;
    floatval->returnType.type = TYPE_FLOAT;
    Function__addParameter(floatval, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "term");
    table_insert(functionTable, "floatval", floatval);

    Function * intval = Function__init();
    intval->name = "intval";
    intval->returnType.isRequired = true;
    intval->returnType.type = TYPE_INT;
    Function__addParameter(intval, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "term");
    table_insert(functionTable, "intval", intval);

    Function * strval = Function__init();
    strval->name = "strval";
    strval->returnType.isRequired = true;
    strval->returnType.type = TYPE_STRING;
    Function__addParameter(strval, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "term");
    table_insert(functionTable, "strval", strval);

    Function * boolval = Function__init();
    boolval->name = "boolval";
    boolval->returnType.isRequired = true;
    boolval->returnType.type = TYPE_BOOL;
    Function__addParameter(boolval, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "term");
    table_insert(functionTable, "boolval", boolval);

    Function * strlen = Function__init();
    strlen->name = "strlen";
    strlen->returnType.isRequired = true;
    strlen->returnType.type = TYPE_INT;
    Function__addParameter(strlen, (Type){.isRequired = true, .type = TYPE_STRING}, "s");
    table_insert(functionTable, "strlen", strlen);

    Function * substring = Function__init();
    substring->name = "substring";
    substring->returnType.isRequired = false;
    substring->returnType.type = TYPE_STRING;
    Function__addParameter(substring, (Type){.isRequired = true, .type = TYPE_STRING}, "s");
    Function__addParameter(substring, (Type){.isRequired = true, .type = TYPE_INT}, "i");
    Function__addParameter(substring, (Type){.isRequired = true, .type = TYPE_INT}, "j");
    table_insert(functionTable, "substring", substring);

    Function * ord = Function__init();
    ord->name = "ord";
    ord->returnType.isRequired = true;
    ord->returnType.type = TYPE_INT;
    Function__addParameter(ord, (Type){.isRequired = true, .type = TYPE_STRING}, "c");
    table_insert(functionTable, "ord", ord);

    Function * chr = Function__init();
    chr->name = "chr";
    chr->returnType.isRequired = true;
    chr->returnType.type = TYPE_STRING;
    Function__addParameter(chr, (Type){.isRequired = true, .type = TYPE_INT}, "i");
    table_insert(functionTable, "chr", chr);
}

bool parse() {
    Table * function_table = table_init();
    loadBuiltinFunctions(function_table);
    StatementList * program = StatementList__init();
    nextToken = getNextToken();
    while(nextToken.type != TOKEN_EOF) {
        if(nextToken.type == TOKEN_FUNCTION) {
            Function * function = NULL;
            if(!parse_function(&function)) return false;
            if(table_find(function_table, function->name) != NULL) {
                fprintf(stderr, "Function %s already defined\n", function->name);
                exit(3);
            }
            table_insert(function_table, function->name, function);
        } else {
            StatementList * statementList;
            if(!parse_statement_list(&statementList)) return false;
            if(statementList->listSize == 0) {
                printParserError(nextToken, "Expected statement");
                return false;
            }
            StatementList__append(program, statementList);
            free(statementList);
        }
    }
    // https://jsoncrack.com/editor
    // https://vanya.jp.net/vtree/
    //StringBuilder stringBuilder;
    //StringBuilder__init(&stringBuilder);
    //program->super.serialize((Statement*)program, &stringBuilder);
    //fprintf(stderr, "%s\n", stringBuilder.text);
    generateCode(program, function_table);
    return true;
}

void initParser() {
    initLexer();
}

void freeParser() {
    freeLexer();
}