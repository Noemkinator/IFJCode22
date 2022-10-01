// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include <unistd.h>
#include "lexer_processor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
	initLexer();
	Token token;
	do {
		token = getNextToken();
		printf("Token: %d %s\n", token.type, getTokenText(token));
	} while(token.type != TOKEN_EOF);
	freeLexer();
	return 0;
}
