/**
 * @brief Hlavičkový soubor pro implementaci zpracovani vyrazu pomoci precedencni syntakticke analyzy (expression.h) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#define PREC_TB_SIZE 9

#include "stack.h"
#include "ast.h"

extern int precedence_tb[PREC_TB_SIZE][PREC_TB_SIZE];

typedef enum {
    SFT,  // Shift (<)
    RED,  // Reduction (>)
    EQU,  // Equal (=)
    ERR   // Error
} PrecTbOperators;

typedef enum {
    INDX_IDENTIFIER,    // i
    INDX_LEFT_BRACKET,  // (
    INDX_RIGHT_BRACKET, // )
    INDX_MULDIV,        // * /
    INDX_ADDSUB,        // + -
    INDX_CONCATENATE,   // .
    INDX_RELATION,      // <= >= < >
    INDX_EQNEQ,         // === !==
    INDX_DOLLAR         // $
} PrecTbIndex;

typedef enum {
    RL_VARIABLE,        // E = i
    RL_BRACKETS,        // E = ( E )
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

ExpRules rule_select(TokenType type1,TokenType type2, TokenType type3);
int get_prec_operator(StackItem* top_term, Token token);
char* prec_tb_op_to_char(PrecTbOperators op);
void shift(Stack* stack, Token token);
bool reduce(Stack* stack, Token token, Expression** exp, Expression** lSide, Expression** rSide, bool* var);
void reduce_expression(Stack* stack, Expression** exp);
void equal(Stack* stack, Token token);
bool parse_exp(Expression** exp, Token token);
bool is_valid_token(Token token);

#endif // __EXPRESSION_H__