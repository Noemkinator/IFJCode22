/**
 * @brief Implementace zasobniku pro zasobnikovy automat (stack.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void init_stack(Stack* stack) {
    stack->top = NULL;
}

// returns true if push was successful, otherwise returns false
bool push(Stack* stack, Symbol symbol) {
    StackItem* new_item = malloc(sizeof(StackItem));
    if(new_item == NULL) return false;
    new_item->symbol = symbol;
    new_item->next = stack->top;
    stack->top = new_item;
    return true;
}

bool pop(Stack* stack) {
    StackItem* temp;
    if(!is_empty(stack)) {
        temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
        return true;
    }
    return false;
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
    StackItem* temp = stack->top;
    while(temp != NULL) {
        if(is_terminal(temp->symbol)) {
            return temp;
        } else {
            temp = temp->next;
        }
    }
    return temp;
}

bool is_terminal(Symbol symbol) {
    if(symbol == SYM_NON_TERMINAL || symbol == SYM_START_DOLLAR) {
        return false;
    }
    return true;
}

bool is_empty(Stack* stack) {
    return stack->top == NULL;
}

char* symbol_to_string(Symbol symbol) {
    switch(symbol) {
        case SYM_START_DOLLAR:
            return "$";
        case SYM_NON_TERMINAL:
            return "E";
        case SYM_OPEN_CURLY_BRACKET:
            return "(";
        case SYM_CLOSE_CURLY_BRACKET:
            return ")";
        case SYM_PLUS:
            return "+";
        case SYM_MINUS:
            return "-";
        case SYM_CONCATENATE:
            return ".";
        case SYM_MULTIPLY:
            return "*";
        case SYM_IDENTIFIER:
            return "i";
        case SYM_INTEGER:
            return "(int)i";
        case SYM_FLOAT:
            return "(float)i";
        case SYM_STRING:
            return "(str)i";
        case SYM_DIVIDE:
            return "/";
        case SYM_EQUALS:
            return "===";
        case SYM_NOT_EQUALS:
            return "!==";
        case SYM_LESS:
            return "<";
        case SYM_GREATER:
            return ">";
        case SYM_LESS_OR_EQUALS:
            return "<=";
        case SYM_GREATER_OR_EQUALS:
            return ">=";
        default:
            return "ERROR";
    }
}