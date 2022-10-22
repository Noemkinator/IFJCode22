/**
 * @brief Hlavičkový soubor pro implementaci zasobniku (stack.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#ifndef __STACK_H__
#define __STACK_H__

#include "lexer_processor.h"
#include <stdbool.h>

typedef enum {
    SYM_START_DOLLAR,
    SYM_NON_TERMINAL,
    SYM_OPEN_BRACKET,
    SYM_CLOSE_BRACKET,
    SYM_OPEN_CURLY_BRACKET,
    SYM_CLOSE_CURLY_BRACKET,
    SYM_PLUS,
    SYM_MINUS,
    SYM_CONCATENATE,
    SYM_MULTIPLY,
    SYM_IDENTIFIER,
    SYM_INTEGER,
    SYM_FLOAT,
    SYM_STRING,
    SYM_DIVIDE,
    SYM_EQUALS,
    SYM_NOT_EQUALS,
    SYM_LESS,
    SYM_GREATER,
    SYM_LESS_OR_EQUALS,
    SYM_GREATER_OR_EQUALS,
    SYM_ERROR
} Symbol;

typedef struct StackItem {
    Symbol symbol;
    struct StackItem* next;
} StackItem;

typedef struct {
    StackItem* top;
} Stack;

void init_stack(Stack* stack);
bool push(Stack* stack, Symbol symbol);
void pop(Stack* stack);
StackItem* top(Stack* stack);

#endif // __STACK_H__