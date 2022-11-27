/**
 * @file lexer_processor.c
 * @author Jiří Gallo (xgallo04)
 * @brief Lexical processor
 * @date 2022-10-27
 */


#include "lexer_processor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    LEXER_PROCESSOR_BEGIN,
    LEXER_PROCESSOR_OPEN_TAG_DONE,
    LEXER_PROCESSOR_SOURCE_DONE
} lexerProcessorState;

Token getNextNonEmptyToken() {
    Token token;
    do {
        token = getNextUnprocessedToken();
    } while(token.type == TOKEN_WHITESPACE || token.type == TOKEN_COMMENT);
    return token;
}

Token getNextToken() {
    switch(lexerProcessorState) {
        case LEXER_PROCESSOR_BEGIN: {
            Token token;
            if((token=getNextUnprocessedToken()).type != TOKEN_LESS) {
                fprintf(stderr, "First character should be <\n");
                lexerError(token);
            }
            if((token=getNextUnprocessedToken()).type != TOKEN_QUESTIONMARK) {
                fprintf(stderr, "Second character should be ?\n");
                lexerError(token);
            }
            if((token=getNextUnprocessedToken()).type != TOKEN_IDENTIFIER || strcmp(getTokenText(token), "php") != 0) {
                fprintf(stderr, "Open tag <? should be followed by php\n");
                lexerError(token);
            }
            if((token=getNextUnprocessedToken()).type != TOKEN_WHITESPACE && token.type != TOKEN_COMMENT) {
                fprintf(stderr, "Open tag <?php should be followed by white character or comment\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_IDENTIFIER || strcmp(getTokenText(token), "declare") != 0) {
                fprintf(stderr, "Open tag <?php should be followed by declare call\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_OPEN_BRACKET) {
                fprintf(stderr, "Missing ( in declare(strict_types=1); statement\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_IDENTIFIER || strcmp(getTokenText(token), "strict_types") != 0) {
                fprintf(stderr, "Missing strict_types in declare(strict_types=1); statement\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_ASSIGN) {
                fprintf(stderr, "Missing = in declare(strict_types=1); statement\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_INTEGER && strcmp(getTokenText(token), "1") != 0) {
                fprintf(stderr, "Missing 1 in declare(strict_types=1); statement\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_CLOSE_BRACKET) {
                fprintf(stderr, "Missing ) in declare(strict_types=1); statement\n");
                lexerError(token);
            }
            if((token=getNextNonEmptyToken()).type != TOKEN_SEMICOLON) {
                fprintf(stderr, "Missing ; in declare(strict_types=1); statement\n");
                lexerError(token);
            }
            lexerProcessorState = LEXER_PROCESSOR_OPEN_TAG_DONE;
            return getNextToken();
        }
        case LEXER_PROCESSOR_OPEN_TAG_DONE: {
            Token token = getNextNonEmptyToken();
            if(token.type == TOKEN_DOLAR) {
                Token token2 = getNextUnprocessedToken();
                if(token2.type == TOKEN_IDENTIFIER) {
                    token.type = TOKEN_VARIABLE;
                    token.length += token2.length;
                    return token;
                } else {
                    fprintf(stderr, "Expected identifier after $\n");
                    lexerError(token2);
                }
            } else if(token.type == TOKEN_QUESTIONMARK) {
                Token secondToken = getNextUnprocessedToken();
                if(secondToken.type == TOKEN_GREATER) {
                    lexerProcessorState = LEXER_PROCESSOR_SOURCE_DONE;
                    Token newlineToken = getNextUnprocessedToken();
                    if((newlineToken.type == TOKEN_WHITESPACE && strcmp(getTokenText(newlineToken), "\n") == 0) || newlineToken.type == TOKEN_EOF) {
                    	return getNextToken();
                    } else {
                        fprintf(stderr, "Expected newline or EOF after closing tag\n");
                        lexerError(newlineToken);
                    }
                } else if(secondToken.type == TOKEN_IDENTIFIER && 
                        (strcmp(getTokenText(secondToken), "float") == 0 || 
                            strcmp(getTokenText(secondToken), "int") == 0 || 
                            strcmp(getTokenText(secondToken), "string") == 0 ||
                            strcmp(getTokenText(secondToken), "boolean") == 0)
                        ) {
                    token.type = TOKEN_TYPE;
                    token.length += secondToken.length;
                    return token;
                } else {
                    fprintf(stderr, "Expected > or type after ?\n");
                    lexerError(secondToken);
                }
            } else if(token.type == TOKEN_IDENTIFIER) {
                if(strcmp(getTokenText(token), "if") == 0) {
                    token.type = TOKEN_IF;
                    return token;
                } else if(strcmp(getTokenText(token), "else") == 0) {
                    token.type = TOKEN_ELSE;
                    return token;
                } else if(strcmp(getTokenText(token), "elseif") == 0) {
                    token.type = TOKEN_ELSEIF;
                    return token;
                } else if(strcmp(getTokenText(token), "true") == 0) {
                    token.type = TOKEN_BOOL;
                    return token;
                } else if(strcmp(getTokenText(token), "false") == 0) {
                    token.type = TOKEN_BOOL;
                    return token;
                } else if(strcmp(getTokenText(token), "function") == 0) {
                    token.type = TOKEN_FUNCTION;
                    return token;
                } else if(strcmp(getTokenText(token), "null") == 0) {
                    token.type = TOKEN_NULL;
                    return token;
                } else if(strcmp(getTokenText(token), "return") == 0) {
                    token.type = TOKEN_RETURN;
                    return token;
                } else if(strcmp(getTokenText(token), "float") == 0 ||
                            strcmp(getTokenText(token), "int") == 0 ||
                            strcmp(getTokenText(token), "string") == 0 ||
                            strcmp(getTokenText(token), "boolean") == 0) {
                    token.type = TOKEN_TYPE;
                    return token;
                } else if(strcmp(getTokenText(token), "void") == 0) {
                    token.type = TOKEN_VOID;
                    return token;
                } else if(strcmp(getTokenText(token), "while") == 0) {
                    token.type = TOKEN_WHILE;
                    return token;
                } else if(strcmp(getTokenText(token), "for") == 0) {
                    token.type = TOKEN_FOR;
                    return token;
                } else if(strcmp(getTokenText(token), "break") == 0) {
                    token.type = TOKEN_BREAK;
                    return token;
                } else if(strcmp(getTokenText(token), "continue") == 0) {
                    token.type = TOKEN_CONTINUE;
                    return token;
                }
            }
            return token;
        }
        case LEXER_PROCESSOR_SOURCE_DONE: {
            Token token = getNextUnprocessedToken();
            if(token.type != TOKEN_EOF) {
                fprintf(stderr, "Characters found after end of the source code\n");
                lexerError(token);
            } else {
                return token;
            }
        }
    }
    fprintf(stderr, "Unknown error in lexer_processor.c\n");
    exit(1);
}
