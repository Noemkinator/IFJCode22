// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "parser.h"
#include "lexer_processor.h"
#include "ast.h"
#include "symtable.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

Token nextToken;

void printParserError(Token token, char * message) {
    printTokenPreview(token);
    fprintf(stderr, "PARSER ERROR: %s on line %d, column %d\n", message, token.line, token.column);
}

bool is_operator(Token token) {
    return token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_CONCATENATE || token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS || token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS || token.type == TOKEN_ASSIGN;
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
    result[0] = 0;
    return retAddr;
}

bool parse_terminal_expression(Expression ** expression) {
    *expression = NULL;
    if(nextToken.type == TOKEN_OPEN_BRACKET) {
        nextToken = getNextToken();
        if(!parse_expression(expression, 0)) return false;
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
    if(nextToken.type != TOKEN_VARIABLE && nextToken.type != TOKEN_INTEGER && nextToken.type != TOKEN_FLOAT && nextToken.type != TOKEN_STRING && nextToken.type != TOKEN_NULL) {
        printParserError(nextToken, "Expected expression");
        return false;
    }
    if(nextToken.type == TOKEN_VARIABLE) {
        Expression__Variable * variable = Expression__Variable__init();
        *expression = (Expression*)variable;
        variable->name = getTokenTextPermanent(nextToken);
    } else if(nextToken.type == TOKEN_INTEGER || nextToken.type == TOKEN_FLOAT || nextToken.type == TOKEN_STRING || nextToken.type == TOKEN_NULL) {
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
        } else if(nextToken.type == TOKEN_NULL) {
            type.type = TYPE_NULL;
            constant->type = type;
        }
    }
    nextToken = getNextToken();
    return true;
}

int getOperatorPrecedence(Token token) {
    if(token.type == TOKEN_ASSIGN) return 1;
    if(token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS) return 2;
    if(token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS) return 3;
    if(token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_CONCATENATE) return 4;
    if(token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE) return 5;
    return 0;
}

bool parse_expression(Expression ** expression, int previousPrecedence) {
    if(!parse_terminal_expression(expression)) return false;
    while(is_operator(nextToken)) {
        Token operatorToken = nextToken;
        int currentPrecedence = getOperatorPrecedence(operatorToken);
        if(previousPrecedence < currentPrecedence || (previousPrecedence == 1 && currentPrecedence == 1)) { // hack because = is right associative unlike rest of operators
            Expression__BinaryOperator * operator = Expression__BinaryOperator__init();
            operator->operator = operatorToken.type;
            operator->lSide = *expression;
            *expression = (Expression*)operator;
            nextToken = getNextToken();
            if(!parse_expression(&operator->rSide, currentPrecedence)) return false;
        } else {
            break;
        }
    }
    return true;
}

extern bool parse_statement();

bool parse_statement_list(StatementList ** statementListRet) {
    StatementList * statementList = StatementList__init();
    *statementListRet = statementList;
    while(nextToken.type == TOKEN_OPEN_BRACKET || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_INTEGER || nextToken.type == TOKEN_FLOAT || nextToken.type == TOKEN_STRING || nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_IF || nextToken.type == TOKEN_WHILE || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_RETURN || nextToken.type == TOKEN_NULL) {
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
    if(!parse_statement_list((StatementList**)&statementIf->ifBody)) return false;
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
    if(!parse_statement_list((StatementList**)&statementIf->elseBody)) return false;
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
    if(!parse_statement_list((StatementList**)&statementWhile->body)) return false;
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
        case TOKEN_IF:
            return parse_if((StatementIf**)retStatement);
        case TOKEN_WHILE:
            return parse_while((StatementWhile**)retStatement);
        case TOKEN_RETURN:
            return parse_return((StatementReturn**)retStatement);
        default:
            if(nextToken.type == TOKEN_OPEN_BRACKET || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_INTEGER || nextToken.type == TOKEN_FLOAT || nextToken.type == TOKEN_STRING || nextToken.type == TOKEN_NULL) {
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

#include "code_generator.c"

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

    Function * strlen = Function__init();
    strlen->name = "strlen";
    strlen->returnType.isRequired = true;
    strlen->returnType.type = TYPE_INT;
    Function__addParameter(strlen, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "s");
    table_insert(functionTable, "strlen", strlen);

    Function * substring = Function__init();
    substring->name = "substring";
    substring->returnType.isRequired = false;
    substring->returnType.type = TYPE_STRING;
    Function__addParameter(substring, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "s");
    Function__addParameter(substring, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "i");
    Function__addParameter(substring, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "j");
    table_insert(functionTable, "substring", substring);

    Function * ord = Function__init();
    ord->name = "ord";
    ord->returnType.isRequired = true;
    ord->returnType.type = TYPE_INT;
    Function__addParameter(ord, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "c");
    table_insert(functionTable, "ord", ord);

    Function * chr = Function__init();
    chr->name = "chr";
    chr->returnType.isRequired = true;
    chr->returnType.type = TYPE_STRING;
    Function__addParameter(chr, (Type){.isRequired = false, .type = TYPE_UNKNOWN}, "i");
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
            StatementList__append(program, statementList);
            free(statementList);
        }
    }
    // https://jsoncrack.com/editor
    StringBuilder stringBuilder;
    StringBuilder__init(&stringBuilder);
    program->super.serialize((Statement*)program, &stringBuilder);
    fprintf(stderr, "%s\n", stringBuilder.text);
    generateCode(program, function_table);
    return true;
}

void initParser() {
    initLexer();
}

void freeParser() {
    freeLexer();
}