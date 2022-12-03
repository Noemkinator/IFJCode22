/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file stack.h
 * @author Jakub Kratochvíl (xkrato67)
 * @brief Header file for the stack implementation
 * @date 2022-10-22
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

/**
 * @brief Initialize stack object
 * 
 * @param stack 
 */
void init_stack(Stack* stack);
/**
 * @brief Initialize dollar token
 * 
 * @return Token 
 */
Token init_dollar();
/**
 * @brief Initialize non-terminal token
 * 
 * @return Token 
 */
Token init_non_term();
/**
 * @brief Initialize shift token
 * 
 * @return Token 
 */
Token init_shift();
/**
 * @brief Push token to stack
 * 
 * @param stack 
 * @param token 
 * @return true 
 * @return false 
 */
bool push(Stack* stack, Token token);
/**
 * @brief Push expression to stack
 * 
 * @param stack 
 * @param exp 
 * @return true 
 * @return false 
 */
bool push_exp(Stack* stack, Token token, Expression* exp);
/**
 * @brief Push non-terminal token to stack
 * 
 * @param stack 
 * @return Token 
 */
bool push_after_non_term(Stack* stack, Token token);
/**
 * @brief Pop token from stack
 * 
 * @param stack 
 */
void pop(Stack* stack);
/**
 * @brief Pop all tokens from stack
 * 
 * @param stack 
 */
void pop_all(Stack* stack);
/**
 * @brief Get top token from stack
 * 
 * @param stack 
 * @return Token 
 */
StackItem* top(Stack* stack);
/**
 * @brief Get top terminal token from stack
 * 
 * @param stack 
 * @return Token 
 */
StackItem* top_term(Stack* stack);
/**
 * @brief Get  second top non-terminal token from stack
 * 
 * @param stack 
 * @return StackItem*  
 */
StackItem* second_non_term(Stack* stack);
/**
 * @brief Analyze token type
 * 
 * @param token
 * @return true if token is terminal 
 */
bool is_terminal(Token token);
/**
 * @brief Analyze stack state
 * 
 * @param token
 * @return true if stack is empty 
 */
bool is_empty(Stack* stack);
/**
 * @brief Convert token to string
 * 
 * @param token
 * @return char*  
 */
char* token_to_string(Token token);
/**
 * @brief Print stack
 * 
 * @param stack 
 */
void print_stack(Stack* stack);

#endif // __STACK_H__