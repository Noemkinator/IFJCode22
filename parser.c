// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "parser.h"
#include "lexer_processor.h"
#include "ast.h"
#include "symtable.h"
#include <stdio.h>

Token nextToken;

void printParserError(Token token, char * message) {
    printTokenPreview(token);
    fprintf(stderr, "PARSER ERROR: %s on line %d, column %d\n", message, token.line, token.column);
}

bool is_operator(Token token) {
    return token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_CONCATENATE || token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS || token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS;
}

extern bool parse_function_call();

bool parse_expression() {
    if(nextToken.type == TOKEN_OPEN_BRACKET) {
        if(parse_expression()) return false;
        if(nextToken.type != TOKEN_CLOSE_BRACKET) {
            printParserError(nextToken, "Expected closing bracket");
            return false;
        }
        nextToken = getNextToken();
        if(is_operator(nextToken)) {
            if(parse_expression()) return false;
        }
        return true;
    }
    if(nextToken.type == TOKEN_IDENTIFIER) {
        if(!parse_function_call()) return false;
        if(is_operator(nextToken)) {
            if(parse_expression()) return false;
        }
        return true;
    }
    if(nextToken.type != TOKEN_VARIABLE && nextToken.type != TOKEN_INTEGER && nextToken.type != TOKEN_FLOAT && nextToken.type != TOKEN_STRING) {
        printParserError(nextToken, "Expected expression");
        return false;
    }
    nextToken = getNextToken();
    while(is_operator(nextToken)) {
        nextToken = getNextToken();
        parse_expression();
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
    if(!parse_expression()) return false;
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
    if(!parse_expression(&statementIf->condition)) return false;
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
    if(!parse_expression(&statementWhile->condition)) return false;
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
    bool success = parse_expression(&expression);
    Expression__FunctionCall__addArgument(functionCall, expression);
    if(!success) return false;
    while(nextToken.type == TOKEN_COMMA) {
        nextToken = getNextToken();
        success = parse_expression(&expression);
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
        if(!parse_expression(&statementReturn->expression)) return false;
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
    return true;
}

void initParser() {
    initLexer();
}

void freeParser() {
    freeLexer();
}