/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file main.c
 * @author Jiří Gallo (xgallo04)
 * @brief Main file of the program
 * @date 2022-10-27
 */

#include <unistd.h>
#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
	initParser();
	if(parse()) {
		fprintf(stderr, "OK\n");
		return 0;
	} else {
		fprintf(stderr, "ERROR\n");
		return 2;
	}
}
