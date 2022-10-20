// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "parser.h"
#include "lexer_processor.h"
#include <stdio.h>

Token nextToken;

void printParserError(Token token, char * message) {
    printTokenPreview(token);
    fprintf(stderr, "PARSER ERROR: %s on line %d, column %d\n", message, token.line, token.column);
}

bool is_operator(Token token) {
    return token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_CONCATENATE || token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS || token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS;
}

bool parse_expression() {
    if(nextToken.type == TOKEN_OPEN_BRACKET) {
        if(parse_expression()) return false;
        if(nextToken.type != TOKEN_CLOSE_BRACKET) {
            printParserError(nextToken, "Expected closing bracket");
            return false;
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

bool parse_statement_list() {
    if(nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_IF || nextToken.type == TOKEN_WHILE || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_RETURN) {
        if(!parse_statement()) return false;
        if(!parse_statement_list()) return false;
    }
    return true;
}

bool parse_if() {
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after if");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_expression()) return false;
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
    if(!parse_statement_list()) return false;
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
    if(!parse_statement_list()) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after else");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_while() {
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after while");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_expression()) return false;
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
    if(!parse_statement_list()) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after while");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_function_arguments() {
    if(nextToken.type == TOKEN_CLOSE_BRACKET) return true;
    if(nextToken.type != TOKEN_TYPE) {
        printParserError(nextToken, "Expected type as first token in function parameter list");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_VARIABLE) {
        printParserError(nextToken, "Expected type as second token in function parameter list");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type == TOKEN_CLOSE_BRACKET) return true;
    while(nextToken.type == TOKEN_COMMA) {
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_TYPE) {
            printParserError(nextToken, "Expected type as first token in function parameter list");
            return false;
        }
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_VARIABLE) {
            printParserError(nextToken, "Expected type as second token in function parameter list");
            return false;
        }
        nextToken = getNextToken();
    }
    return true;
}

bool parse_function_call() {
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after function call");
        return false;
    }
    nextToken = getNextToken();
    parse_function_arguments();
    if(nextToken.type != TOKEN_CLOSE_BRACKET) {
        printParserError(nextToken, "Missing ) after function call");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Missing ; after function call");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_return() {
    nextToken = getNextToken();
    // expression after return is optional
    if(nextToken.type == TOKEN_OPEN_BRACKET || nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_INTEGER || nextToken.type == TOKEN_FLOAT || nextToken.type == TOKEN_STRING) {
        if(!parse_expression()) return false;
    }
    if(nextToken.type != TOKEN_SEMICOLON) {
        printParserError(nextToken, "Missing ; after return");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse_statement() {
    switch (nextToken.type) {
        case TOKEN_VARIABLE:
            return parse_assignment();
        case TOKEN_IF:
            return parse_if();
        case TOKEN_WHILE:
            return parse_while();
        case TOKEN_IDENTIFIER:
            return parse_function_call();
        case TOKEN_RETURN:
            return parse_return();
        default:
            printParserError(nextToken, "Unexpected token at start of statement");
            return false;
    }
    return true;
}

bool parse_function_parameters() {
    if(nextToken.type != TOKEN_TYPE) {
        return true;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_VARIABLE) {
        printParserError(nextToken, "Expected variable after type");
        return false;
    }
    nextToken = getNextToken();
    while(nextToken.type == TOKEN_COMMA) {
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_TYPE) {
            printParserError(nextToken, "Expected type after comma");
            return false;
        }
        nextToken = getNextToken();
        if(nextToken.type != TOKEN_VARIABLE) {
            printParserError(nextToken, "Expected variable after type");
            return false;
        }
        nextToken = getNextToken();
    }
    return true;
}

bool parse_function() {
    Token functionIdentifier = getNextToken();
    if(functionIdentifier.type != TOKEN_IDENTIFIER) {
        printParserError(functionIdentifier, "Missing function name");
        return false;
    }
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_BRACKET) {
        printParserError(nextToken, "Missing ( after function name");
        return false;
    }
    nextToken = getNextToken();
    parse_function_parameters();
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
    nextToken = getNextToken();
    if(nextToken.type != TOKEN_OPEN_CURLY_BRACKET) {
        printParserError(nextToken, "Missing { after function");
        return false;
    }
    nextToken = getNextToken();
    if(!parse_statement_list()) return false;
    if(nextToken.type != TOKEN_CLOSE_CURLY_BRACKET) {
        printParserError(nextToken, "Missing } after function");
        return false;
    }
    nextToken = getNextToken();
    return true;
}

bool parse() {
    nextToken = getNextToken();
    while(nextToken.type != TOKEN_EOF) {
        if(nextToken.type == TOKEN_FUNCTION) {
            if(!parse_function()) return false;
        } else if(nextToken.type == TOKEN_VARIABLE || nextToken.type == TOKEN_IF || nextToken.type == TOKEN_WHILE || nextToken.type == TOKEN_IDENTIFIER || nextToken.type == TOKEN_RETURN) {
            if(!parse_statement_list()) return false;
        } else if(nextToken.type == TOKEN_RETURN) {
            if(!parse_return()) return false;
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