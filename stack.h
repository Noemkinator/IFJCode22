/**
 * @brief Hlavičkový soubor pro implementaci zasobniku (stack.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#ifndef __STACK_H__
#define __STACK_H__

#include "lexer_processor.h"
#include "ast.h"
#include <stdbool.h>

typedef struct StackItem {
    Token token;
    Expression* exp;
    struct StackItem* next;
} StackItem;

typedef struct Stack {
    StackItem* top;
} Stack;

void init_stack(Stack* stack);
Token init_dollar();
Token init_non_term();
Token init_shift();
bool push(Stack* stack, Token token);
void pop(Stack* stack);
void pop_all(Stack* stack);
StackItem* top(Stack* stack);
StackItem* top_term(Stack* stack);
StackItem* second_non_term(Stack* stack);
bool is_terminal(Token token);
bool is_empty(Stack* stack);
char* token_to_string(Token token);
void print_stack(Stack* stack);

#endif // __STACK_H__