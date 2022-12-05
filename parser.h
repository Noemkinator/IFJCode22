// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)

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
bool is_binary_operator(Token token);
int get_prec_tb_indx(TokenType type);

bool is_constant(TokenType tokenType);
bool is_first_terminal_expression(TokenType tokenType);
bool is_first_expression(TokenType tokenType);
bool is_first_if(TokenType tokenType);
bool is_first_while(TokenType tokenType);
bool is_first_function_call_arguments(TokenType tokenType);
bool is_first_function_call(TokenType tokenType);
bool is_first_return(TokenType tokenType);
bool is_first_statement(TokenType tokenType);

#endif