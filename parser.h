// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer_processor.h"
#include "ast.h"
#include "symtable.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define PREC_TB_SIZE 7

extern bool parse_function_call();
extern bool parse_expression(Expression ** expression, TokenType previousToken);
extern bool precedence_tb[PREC_TB_SIZE][PREC_TB_SIZE];

bool parse();
void initParser();
void freeParser();
bool is_operator(Token token);
int get_prec_tb_indx(TokenType type);

#endif