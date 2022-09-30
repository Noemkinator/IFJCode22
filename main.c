// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include <unistd.h>
#include "lexer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char * substr(char * text, int start, int length) {
	char * result = malloc(length + 1);
	strncpy(result, text + start, length);
	result[length] = '\0';
	return result;
}

int main(int argc, char ** argv) {
	initLexer();
	Token token;
	do {
		token = getNextToken();
		printf("Token: %d %s\n", token.type, substr(sourceText, token.sourcePosition, token.length));
	} while(token.type != TOKEN_EOF);
	freeLexer();
	return 0;
}
