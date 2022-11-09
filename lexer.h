/**
 * @file lexer.h
 * @author Jiří Gallo (xgallo04)
 * @brief Lexical analyzer library
 * @date 2022-10-26
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdlib.h>

extern char * sourceText;
extern size_t sourceTextLength;
/**
 * @brief TokenTypes enum
 * @details Enum of all possible token types
 */
typedef enum {
    TOKEN_EOF,
    TOKEN_WHITESPACE,
    TOKEN_SEMICOLON,
    TOKEN_DOLAR, // not used by paraser
    TOKEN_QUESTIONMARK, // not used by parser
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,
    TOKEN_OPEN_CURLY_BRACKET,
    TOKEN_CLOSE_CURLY_BRACKET,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_CONCATENATE,
    TOKEN_MULTIPLY,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_DIVIDE,
    TOKEN_COMMENT,
    TOKEN_ASSIGN,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LESS_OR_EQUALS,
    TOKEN_GREATER_OR_EQUALS,
    TOKEN_ELSE,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_NULL,
    TOKEN_RETURN,
    TOKEN_TYPE, // represents keywords float, int, string
    TOKEN_VOID,
    TOKEN_WHILE,
    TOKEN_FLOAT,
    TOKEN_VARIABLE,
    TOKEN_ERROR
} TokenType;

/**
 * @brief Token structure
 */
typedef struct {
    int line;/*< Line number>*/
    int column;/*< Column number>*/
    int sourcePosition;/*< Position in source code>*/
    int length;/*< Length of token>*/
    TokenType type;/*< Type of token>*/
} Token;

void printTokenPreview(Token token);
void lexerError(Token token);
Token getNextUnprocessedToken();
char * getTokenTextPermanent(Token token);
char * getTokenText(Token token);
void initLexer();
void freeLexer();

#endif // __LEXER_H__