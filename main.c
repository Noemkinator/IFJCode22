// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include <unistd.h>
#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
	initParser();
	if(parse()) {
		printf("OK\n");
		return 0;
	} else {
		printf("ERROR\n");
		return 2;
	}
}
