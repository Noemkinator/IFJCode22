/**
 * @brief Hlavičkový soubor pro implementaci zpracovani vyrazu pomoci precedencni syntakticke analyzy (expression.h) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#define TB_SIZE 9

#include "stack.h"

extern int precedence_tb[TB_SIZE][TB_SIZE];

typedef enum {
    SFT,  // Shift (<)
    RED,  // Reduction (>)
    EQU,  // Equal (=)
    ERR   // Error
} PrecTbSymbols;

typedef enum {
    INDX_IDENTIFIER,    // i
    INDX_LEFT_CURLY,    // (
    INDX_RIGHT_CURLY,   // )
    INDX_MULDIV,        // * /
    INDX_ADDSUB,        // + -
    INDX_CONCATENATE,   // .
    INDX_RELATION,      // <= >= < >
    INDX_EQNEQ,         // === !==
    INDX_DOLLAR         // $
} PrecTbIndex;

typedef enum {
    RL_IDENTIFIER,      // E = i
    RL_CURLY_BRACKETS,  // E = ( E )
    RL_MULIPLY,         // E = E * E
    RL_DIVIDE,          // E = E / E
    RL_PLUS,            // E = E + E
    RL_MINUS,           // E = E - E
    RL_CONCATENATE,     // E = E . E
    RL_LESS,            // E = E < E
    RL_GREATER,         // E = E > E
    RL_LESS_OR_EQUALS,  // E = E <= E
    RL_GREAT_OR_EQUALS, // E = E >= E
    RL_EQUALS,          // E = E === E
    RL_NOT_EQUALS,      // E = E !== E
    RL_ERROR            // error
} ExpRules;

Symbol token_to_symbol(Token token);

void shift(Stack stack);
void reduction(Stack stack);
void equal(Stack stack);


#endif // __EXPRESSION_H__