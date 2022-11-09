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

Token init_dollar() {
    Token t = {.column = 0, .length=0, .line=0, .sourcePosition=0, .type=TOKEN_STACK_DOLLAR};
    return t;
}

Token init_non_term() {
    Token t = {.column = 0, .length=0, .line=0, .sourcePosition=0, .type=TOKEN_NON_TERMINAL};
    return t;
}

Token init_shift() {
    Token t = {.column = 0, .length=0, .line=0, .sourcePosition=0, .type=TOKEN_SHIFT};
    return t;
}

bool push(Stack* stack, Token token) {
    StackItem* new_item = malloc(sizeof(StackItem));
    if(new_item == NULL) return false;
    new_item->token = token;
    new_item->next = stack->top;
    stack->top = new_item;
    return true;
}
/**
 * @brief Push symbol to stack
 * 
 * @param stack 
 * @param symbol 
 * @return true if push was successful
 * @return false if push failed
 */
bool push_exp(Stack* stack, Token token, Expression* exp) {
    StackItem* new_item = malloc(sizeof(StackItem));
    if(new_item == NULL) return false;
    new_item->token = token;
    new_item->next = stack->top;
    new_item->exp = exp;
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
void pop(Stack* stack) {
    StackItem* temp;
    if(stack->top != NULL) {
        temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
}

/**
 * @brief Pop all symbols from stack
 * 
 * @param stack 
 */
void pop_all(Stack* stack) {
    StackItem* temp;
    while(stack->top != NULL) {
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
 * @return NULL if stack is empty
 */
StackItem* top(Stack* stack) {
    if(stack->top != NULL) {
        return stack->top;
    } else return NULL;
}

bool push_after_non_term(Stack* stack, Token token) {
    StackItem* new_item = malloc(sizeof(StackItem));
    StackItem* temp = stack->top;
    while(temp != NULL) {
        if(!is_terminal(temp->token)) {
            new_item->token = token;
            new_item->exp = NULL;
            new_item->next = temp->next;
            temp->next = new_item;
            return true;
        } else {
            temp = temp->next;
        }

    }
    return false;
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
        if(is_terminal(temp->token)) {
            return temp;
        } else {
            temp = temp->next;
        }
    }
    return temp;
}

StackItem* second_non_term(Stack* stack) {
    StackItem* temp = stack->top;
    int cnt = 0;
    while(temp != NULL) {
        if(!is_terminal(temp->token)) {
            cnt++;
            if(cnt == 2) return temp;
        }
        temp = temp->next;
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
bool is_terminal(Token token) {
    if(token.type == TOKEN_NON_TERMINAL) {
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
char* token_to_string(Token token) {
    switch(token.type) {
        case TOKEN_STACK_DOLLAR:
            return "$";
        case TOKEN_NON_TERMINAL:
            return "E";
        case TOKEN_SHIFT:
            return "SFT(<)";
        case TOKEN_OPEN_BRACKET:
            return "(";
        case TOKEN_CLOSE_BRACKET:
            return ")";
        case TOKEN_PLUS:
            return "+";
        case TOKEN_MINUS:
            return "-";
        case TOKEN_CONCATENATE:
            return ".";
        case TOKEN_MULTIPLY:
            return "*";
        case TOKEN_VARIABLE:
            return "i";
        case TOKEN_INTEGER:
            return "(int)i";
        case TOKEN_FLOAT:
            return "(float)i";
        case TOKEN_STRING:
            return "(str)i";
        case TOKEN_DIVIDE:
            return "/";
        case TOKEN_EQUALS:
            return "===";
        case TOKEN_NOT_EQUALS:
            return "!==";
        case TOKEN_LESS:
            return "<";
        case TOKEN_GREATER:
            return ">";
        case TOKEN_LESS_OR_EQUALS:
            return "<=";
        case TOKEN_GREATER_OR_EQUALS:
            return ">=";
        default:
            return "ERROR";
    }
}

void print_stack(Stack* stack) {
    StackItem* top = stack->top;
    while(top != NULL) {
        printf("%s ", token_to_string(top->token));
        if(top->exp != NULL) {
            printf("(%d)", top->exp->expressionType);
        }
        puts("");
        top = top->next;
    }
}