// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "parser.h"
#include "lexer_processor.h"
#include "ast.h"
#include "symtable.h"
#include <stdio.h>
#include <stdint.h>

Token nextToken;

void printParserError(Token token, char * message) {
    printTokenPreview(token);
    fprintf(stderr, "PARSER ERROR: %s on line %d, column %d\n", message, token.line, token.column);
}

bool is_operator(Token token) {
    return token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_CONCATENATE || token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS || token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS;
}

extern bool parse_function_call();
extern bool parse_expression(Expression ** expression, int previousPrecedence);

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
                    if('0' <= text[1] && text[1] <= 7 && '0' <= text[2] && text[2] <= 7) {
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
        } else {
            *result = *text;
        }
        result++;
        text++;
    }
    return retAddr;
}

bool parse_unary_expression(Expression ** expression) {
    *expression = NULL;
    if(nextToken.type == TOKEN_OPEN_BRACKET) {
        if(parse_expression(expression, 0)) return false;
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
    if(nextToken.type != TOKEN_VARIABLE && nextToken.type != TOKEN_INTEGER && nextToken.type != TOKEN_FLOAT && nextToken.type != TOKEN_STRING) {
        printParserError(nextToken, "Expected expression");
        return false;
    }
    if(nextToken.type == TOKEN_VARIABLE) {
        Expression__Variable * variable = Expression__Variable__init();
        *expression = variable;
        variable->name = getTokenTextPermanent(nextToken);
    } else if(nextToken.type == TOKEN_INTEGER || nextToken.type == TOKEN_FLOAT || nextToken.type == TOKEN_STRING) {
        Expression__Constant * constant = Expression__Constant__init();
        *expression = constant;
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
        }
    }
    nextToken = getNextToken();
    return true;
}

int getOperatorPrecedence(Token token) {
    if(token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS) return 1;
    if(token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS) return 2;
    if(token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_CONCATENATE) return 3;
    if(token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE) return 4;
    return 0;
}

bool parse_expression(Expression ** expression, int previousPrecedence) {
    if(!parse_unary_expression(expression)) return false;
    while(is_operator(nextToken)) {
        Token operatorToken = nextToken;
        int currentPrecedence = getOperatorPrecedence(operatorToken);
        if(previousPrecedence < currentPrecedence) {
            Expression__BinaryOperator * operator = Expression__BinaryOperator__init();
            operator->lSide = *expression;
            *expression = operator;
            nextToken = getNextToken();
            parse_expression(&operator->rSide, currentPrecedence);
        } else {
            break;
        }
    }
    return true;
}

bool parse_assignment() {
    Token variable = nextToken;
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_ASSIGN) {
        printParserError(nextToken, "Expected assignment");
        return false;
    }
    nextToken = getNextToken();
    Expression * expression;
    if(!parse_expression(&expression, 0)) return false;
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Expected semicolon after assignment");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

extern bool parse_statement();

bool parse_statement_list(StatementList ** statementListRet) {
    StatementList * statementList = StatementList__init();
    *statementListRet = statementList;
    while(nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_IF || nextToken.type == TOKEN_WHILE || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_RETURN) {
        Statement * statement;
        bool success = parse_statement(&statement);
        StatementList__addStatement(statementList, statement);
        if(!success) return false;
    }
    return true;
}

bool parse_if(StatementIf ** statementIfRet) {
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
    if(!parse_statement_list(&statementIf->ifBody)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after if");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_ELSE) {
        printParserError(nextToken, "Missing else after if");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
        printParserError(nextToken, "Missing { after else");
        return false;
    }
    nextToken = getNextToken();
    StatementList * statementListElse;
    if(!parse_statement_list(&statementIf->elseBody)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after else");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_while(StatementWhile ** statementWhileRet) {
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
    if(!parse_statement_list(&statementWhile->body)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after while");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_function_arguments(Expression__FunctionCall * functionCall) {
    if(nextToken.type == TOKEN_CLOSE_BRACKET) return true;
    Expression * expression;
    bool success = parse_expression(&expression, 0);
    Expression__FunctionCall__addArgument(functionCall, expression);
    if(!success) return false;
    while(nextToken.type == TOKEN_COMMA) {
        nextToken = getNextToken();
        success = parse_expression(&expression,0);
        Expression__FunctionCall__addArgument(functionCall, expression);
        if(!success) return false;
    }
    return true;
}

bool parse_function_call(Expression__FunctionCall ** functionCallRet) {
    Expression__FunctionCall * functionCall = Expression__FunctionCall__init();
    *functionCallRet = functionCall;
    functionCall->name = getTokenTextPermanent(nextToken);
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after function call");
        return false;
    }
    nextToken = getNextToken();
    parse_function_arguments(functionCall);
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) after function call");
        return false;
    }
    // TODO: remove this after fixing expression parsing
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Missing ; after function call");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_return(StatementReturn ** statementReturnRet) {
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

bool parse_statement(Statement ** retStatement) {
    switch (nextToken.type) {
        case TOKEN_VARIABLE:
            return parse_assignment(retStatement);
        case TOKEN_IF:
            return parse_if(retStatement);
        case TOKEN_WHILE:
            return parse_while(retStatement);
        case TOKEN_IDENTIFIER:
            return parse_function_call(retStatement);
        case TOKEN_RETURN:
            return parse_return(retStatement);
        default:
            printParserError(nextToken, "Unexpected token at start of statement");
            return false;
    }
    return true;
}

bool parse_function_parameters(Function * function) {
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

bool parse_function(Function ** retFunction) {
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
    if(returnType.type != TOKEN_TYPE) {
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
    if(!parse_statement_list(&function->body)) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after function");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse() {
    Table * function_table = table_init();
    StatementList * program = StatementList__init();
    nextToken = getNextToken();
    while(nextToken.type != TOKEN_EOF) {
        if(nextToken.type == TOKEN_FUNCTION) {
            Function * function;
            if(!parse_function(&function)) return false;
            if(table_find(function_table, function->name) != NULL) {
                fprintf(stderr, "Function %s already defined\n", function->name);
                exit(3);
            }
            table_insert(function_table, function->name, function);
        } else if(nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_IF || nextToken.type == TOKEN_WHILE || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_RETURN) {
            StatementList * statementList;
            if(!parse_statement_list(&statementList)) return false;
            StatementList__append(program, statementList);
            free(statementList);
        } else {
            printParserError(nextToken, "Unexpected token at start of statement");
            return false;
        }
    }
    // https://jsoncrack.com/editor
    /*StringBuilder stringBuilder;
    StringBuilder__init(&stringBuilder);
    program->super.serialize(&program->super, &stringBuilder);
    puts(stringBuilder.text);*/
    return true;
}

void initParser() {
    initLexer();
}

void freeParser() {
    freeLexer();
}