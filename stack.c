/**
 * @brief Implementace zasobniku pro zasobnikovy automat (stack.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void init_stack(Stack* stack) {
    stack->top = NULL;      // mozna upravit tak aby se rovnou do zasobniku dal $
}

// returns true if push was successful, otherwise returns false
bool push(Stack* stack, Symbol symbol) {
    StackItem* new_item = malloc(sizeof(StackItem));
    if(new_item == NULL) {
        //fprintf(stderr, "Not enough space for aloccating new memory.\n");
        return false;
    }
    new_item->symbol = symbol;
    new_item->next = stack->top;
    stack->top = new_item;
    return true;
}

void pop(Stack* stack) {
    StackItem* temp;
    if(!is_empty(stack)) {
        temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
}

void pop_all(Stack* stack) {
    StackItem* temp;
    while(!is_empty(stack)) {
        temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    stack->top = NULL;
}

StackItem* top(Stack* stack) {
    if(!is_empty(stack)) {
        return stack->top;
    }
}

// returns most top terminal, returns NULL if stack doesn't contain any terminal
StackItem* top_term(Stack* stack) {
    while(stack->top != NULL) {
        if(is_terminal(stack->top)) {
            return stack->top;
        } else {
            stack->top = stack->top->next;
        }
    }
    return NULL;
}

bool is_terminal(Symbol symbol) {
    if(symbol == SYM_NON_TERMINAL) {
        return false;
    }
    return true;
}

bool is_empty(Stack* stack) {
    return stack->top == NULL;
}
