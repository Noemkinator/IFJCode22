// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

FILE * sourceFile;
char * sourceText = NULL;
size_t sourceTextLength = 0;
int currentLine = 1;
int currentColumn = 1;
int currentPosition = 0;

int getNextChar() {
    int nextChar;
    if(currentPosition < sourceTextLength) {
        nextChar = sourceText[currentPosition];
    } else {
        nextChar = EOF;
    }
    if(nextChar == '\n') {
        currentLine++;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
    currentPosition++;
    return nextChar;
}

void undoChar() {
    currentColumn--;
    currentPosition--;
}

void lexerError(Token token) {
    fprintf(stderr, "Lexer error on line %d, column %d\n", token.line, token.column);
    exit(1);
}

#define TOKEN(x) { \
    token.type = TOKEN_ ## x; \
    token.length = currentPosition - token.sourcePosition; \
    return token; \
} \

Token getNextUnprocessedToken() {
    Token token = {};
    token.line = currentLine;
    token.column = currentColumn;
    token.sourcePosition = currentPosition;
    int c = getNextChar();
    if(c == EOF) TOKEN(EOF)
    else if(isspace(c)) TOKEN(WHITESPACE)
    else if(c == ';') TOKEN(SEMICOLON)
    else if(c == '$') TOKEN(DOLAR)
    else if(c == '?') TOKEN(QUESTIONMARK)
    else if(c == '(') TOKEN(OPEN_BRACKET)
    else if(c == ')') TOKEN(CLOSE_BRACKET)
    else if(c == '{') TOKEN(OPEN_CURLY_BRACKET)
    else if(c == '}') TOKEN(CLOSE_CURLY_BRACKET)
    else if(c == '+') TOKEN(PLUS)
    else if(c == '-') TOKEN(MINUS)
    else if(c == '.') TOKEN(CONCATENATE)
    else if(c == '*') TOKEN(MULTIPLY)
    else if(c == '<') TOKEN(LESS)
    else if(c == '>') TOKEN(GREATER)
    else if(c == ',') TOKEN(COMMA)
    else if(c == ':') TOKEN(COLON)
    else if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        do {
            c = getNextChar();
        } while((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_');
        undoChar();
        TOKEN(IDENTIFIER)
    }
    else if(c >= '0' && c <= '9') {
        do {
            c = getNextChar();
        } while(c >= '0' && c <= '9');
        undoChar();
        TOKEN(INTEGER)
    }
    else if(c == '"') {
        do {
            c = getNextChar();
            if(c == EOF) lexerError(token);
            if(c == '\\') {
                getNextChar();
                continue;
            }
        } while(c != '"');
        TOKEN(STRING)
    }
    else if(c == '/') {
        char nextChar = getNextChar();
        if(nextChar == '/') {
            do {
                nextChar = getNextChar();
            } while(nextChar != '\n' && nextChar != EOF);
            TOKEN(COMMENT)
        } else if(nextChar == '*') {
            do {
                nextChar = getNextChar();
                if(nextChar == '*') {
                    nextChar = getNextChar();
                    if(nextChar == '/') {
                        break;
                    }
                    undoChar();
                } else if(nextChar == EOF) {
                    fprintf(stderr, "End of file reached while parsing multiline comment\n");
                    lexerError(token);
                }
            } while(1);
            TOKEN(COMMENT)
        } else {
            undoChar();
            TOKEN(DIVIDE)
        }
    }
    else if(c == '=') {
        char nextChar1 = getNextChar();
        char nextChar2 = getNextChar();
        if(nextChar1 == '=' && nextChar2 == '=') {
            TOKEN(EQUALS)
        } else {
            undoChar();
            undoChar();
            TOKEN(ASSIGN)
        }
    }
    else if(c == '!') {
        char nextChar1 = getNextChar();
        char nextChar2 = getNextChar();
        if(nextChar1 == '=' && nextChar2 == '=') {
            TOKEN(EQUALS)
        } else {
            undoChar();
            undoChar();
        }
    }
    else if(c == '<') {
        char nextChar = getNextChar();
        if(nextChar == '=') {
            TOKEN(LESS_OR_EQUALS)
        } else {
            undoChar();
            TOKEN(LESS)
        }
    }
    else if(c == '>') {
        char nextChar = getNextChar();
        if(nextChar == '=') {
            TOKEN(GREATER_OR_EQUALS)
        } else {
            undoChar();
            TOKEN(GREATER)
        }
    }
    fprintf(stderr, "Unsupported character: %c\n", c);
    lexerError(token);
    // lexer error causes exit
    __builtin_unreachable();
}

// requires call to free after
char * getTokenTextPermanent(Token token) {
	char * text = malloc(token.length + 1);
	memcpy(text, sourceText + token.sourcePosition, token.length);
	text[token.length] = '\0';
	return text;
}

char * temporaryTokenText = NULL;

// requires that there are not existing two results from the function at same time
char * getTokenText(Token token) {
    if(temporaryTokenText) {
        free(temporaryTokenText);
    }
    temporaryTokenText = getTokenTextPermanent(token);
    return temporaryTokenText;
}

void initLexer() {
    sourceFile = open_memstream(&sourceText, &sourceTextLength);
	char c;
	while((c = getc(stdin)) != EOF) {
        putc(c, sourceFile);
	}
    fflush(sourceFile);
    rewind(sourceFile);
}

void freeLexer() {
    fclose(sourceFile);
    free(sourceText);
    if(temporaryTokenText) {
        free(temporaryTokenText);
    }
}