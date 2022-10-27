/**
 * @brief Implementace zasobniku pro zasobnikovy automat (stack.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Initialize stack object
 * 
 * @param stack 
 */
void init_stack(Stack* stack) {
    stack->top = NULL;
}

/**
 * @brief Push symbol to stack
 * 
 * @param stack 
 * @param symbol 
 * @return true if push was successful
 * @return false if push failed
 */
bool push(Stack* stack, Symbol symbol) {
    StackItem* new_item = malloc(sizeof(StackItem));
    if(new_item == NULL) return false;
    new_item->symbol = symbol;
    new_item->next = stack->top;
    stack->top = new_item;
    return true;
}

/**
 * @brief Pop symbol from stack
 * 
 * @param stack 
 * @return true if pop was successful
 * @return false if pop failed
 */
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

/**
 * @brief Pop all symbols from stack
 * 
 * @param stack 
 */
void pop_all(Stack* stack) {
    StackItem* temp;
    while(!is_empty(stack)) {
        temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    stack->top = NULL;
}

/**
 * @brief Get top symbol from stack
 * 
 * @param stack 
 * @return StackItem* 
 */
StackItem* top(Stack* stack) {
    if(!is_empty(stack)) {
        return stack->top;
    }
}

/**
 * @brief Get top terminal symbol from stack
 * 
 * @param stack 
 * @return StackItem* <most top terminal symbol>
 * @return NULL if stack doesn't contain any terminal symbol
 */
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

/**
 * @brief Check if symbol is terminal
 * 
 * @param symbol 
 * @return true if symbol is terminal
 * @return false if symbol is non-terminal
 */
bool is_terminal(Symbol symbol) {
    if(symbol == SYM_NON_TERMINAL || symbol == SYM_START_DOLLAR) {
        return false;
    }
    return true;
}

/**
 * @brief Check if stack is empty
 * 
 * @param stack 
 * @return true if stack is empty
 * @return false if stack is not empty
 */
bool is_empty(Stack* stack) {
    return stack->top == NULL;
}

/**
 * @brief Convert symbol enum to string
 * 
 * @param symbol 
 * @return char* 
 */
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