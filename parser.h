/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file parser.h
 * @authors Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)
 * @brief Parser library
 * @date 2022-09-25
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer_processor.h"
#include "ast.h"
#include "symtable.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define PREC_TB_SIZE 10

extern bool parse_function_call();
extern bool parse_expression(Expression ** expression, int previousPrecedence);
extern bool precedence_tb[PREC_TB_SIZE][PREC_TB_SIZE];

bool parse();
void initParser();
void freeParser();
bool is_binary_operator(TokenType tokenType);
int get_prec_tb_indx(TokenType type);



#endif