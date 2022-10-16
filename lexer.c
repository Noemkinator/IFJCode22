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
int prevLineLastColumn = 1;
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
        prevLineLastColumn = currentColumn;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
    currentPosition++;
    return nextChar;
}

void undoChar() {
    currentPosition--;
    if(currentPosition < sourceTextLength && sourceText[currentPosition] == '\n') {
        currentColumn = prevLineLastColumn;
        prevLineLastColumn = 1;
        currentLine--;
    } else {
        currentColumn--;
    }
}

void printTokenPreview(Token token) {
    token.length = currentPosition - token.sourcePosition;
    int printStart = token.sourcePosition - 60;
    int printEnd = token.sourcePosition + token.length + 60;
    for(int i = token.sourcePosition - 1; i >= printStart && i >= 0; i--) {
        if(sourceText[i] == '\n') {
            printStart = i + 1;
            break;
        }
    }
    for(int i = token.sourcePosition + token.length; i <= printEnd && i < sourceTextLength; i++) {
        if(sourceText[i] == '\n') {
            printEnd = i;
            break;
        }
    }
    putc('>', stderr);
    putc(' ', stderr);
    for(int i = printStart; i < printEnd; i++) {
        putc(sourceText[i], stderr);
    }
    putc('\n', stderr);
    putc(' ', stderr);
    putc(' ', stderr);
    for(int i = printStart; i < token.sourcePosition; i++) {
        putc(' ', stderr);
    }
    putc('^', stderr);
    for(int i = 0; i < token.length - 1; i++) {
        putc('~', stderr);
    }
    putc('\n', stderr);
}

void lexerError(Token token) {
    printTokenPreview(token);
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
        if(c == '.') {
            c = getNextChar();
            if(!(c >= '0' && c <= '9')) {
                fprintf(stderr, "Expected digit after decimal point\n");
                lexerError(token);
            }
            do {
                c = getNextChar();
            } while(c >= '0' && c <= '9');
            if(c == 'e' || c == 'E') {
                goto FLOAT_PREEXPONENT;
            } else {
                undoChar();
                TOKEN(FLOAT)
            }
        } else if(c == 'e' || c == 'E') {
            FLOAT_PREEXPONENT:
            c = getNextChar();
            if(c == '+' || c == '-') {
                c = getNextChar();
            }
            if(c >= '0' && c <= '9') {
                do {
                    c = getNextChar();
                } while(c >= '0' && c <= '9');
                undoChar();
                TOKEN(FLOAT)
            } else {
                fprintf(stderr, "Expected digit after exponent\n");
                lexerError(token);
            }
        } else {
            undoChar();
            TOKEN(INTEGER)
        }
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
                while(nextChar == '*') {
                    nextChar = getNextChar();
                    if(nextChar == '/') {
                        TOKEN(COMMENT)
                    }
                }
                if(nextChar == EOF) {
                    fprintf(stderr, "End of file reached while parsing multiline comment\n");
                    lexerError(token);
                }
            } while(1);
        } else {
            undoChar();
            TOKEN(DIVIDE)
        }
    }
    else if(c == '=') {
        char nextChar = getNextChar();
        if(nextChar == '=') {
            nextChar = getNextChar();
            if(nextChar == '=') {
                TOKEN(EQUALS)
            } else {
                undoChar();
                fprintf(stderr, "Found invalid token ==, did you mean ===?\n");
                lexerError(token);
            }
        } else {
            undoChar();
            TOKEN(ASSIGN)
        }
    }
    else if(c == '!') {
        char nextChar = getNextChar();
        if(nextChar == '=') {
            nextChar = getNextChar();
            if(nextChar == '=') {
                TOKEN(NOT_EQUALS)
            } else {
                undoChar();
                fprintf(stderr, "Found invalid token !=, did you mean !==?\n");
                lexerError(token);
            }
        } else {
            undoChar();
            fprintf(stderr, "Found invalid token !, negation isn't supported\n");
            lexerError(token);
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
