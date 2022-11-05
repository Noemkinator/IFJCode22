/**
 * @brief Implementace zpracovani vyrazu pomoci precedencni syntakticke analyzy (expression.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#include "expression.h"

int precedence_tb[PREC_TB_SIZE][PREC_TB_SIZE] = {
//  i    (    )   */   +-   .    <>   !=    $
  {ERR, ERR, RED, RED, RED, RED, RED, RED, RED},   // i  
  {SFT, SFT, EQU, SFT, SFT, SFT, SFT, SFT, ERR},   // (  
  {ERR, ERR, RED, RED, RED, RED, RED, RED, RED},   // )  
  {SFT, SFT, RED, RED, RED, RED, RED, RED, RED},   // */ 
  {SFT, SFT, RED, SFT, RED, RED, RED, RED, RED},   // +- 
  {SFT, SFT, RED, SFT, RED, RED, RED, RED, RED},   // .  
  {SFT, SFT, RED, SFT, SFT, SFT, RED, RED, RED},   // <> 
  {ERR, SFT, RED, SFT, SFT, SFT, SFT, RED, RED},   // != 
  {SFT, SFT, ERR, SFT, SFT, SFT, SFT, SFT, ERR},   // $  
};   

Symbol token_to_symbol(Token token) {
    switch(token.type) {
        case TOKEN_OPEN_CURLY_BRACKET:
            return SYM_OPEN_CURLY_BRACKET;
        case TOKEN_CLOSE_CURLY_BRACKET:
            return SYM_CLOSE_CURLY_BRACKET;
        case TOKEN_PLUS:
            return SYM_PLUS; 
        case TOKEN_MINUS:
            return SYM_MINUS; 
        case TOKEN_CONCATENATE:
            return SYM_CONCATENATE;
        case TOKEN_MULTIPLY:
            return SYM_MULTIPLY;
        case TOKEN_IDENTIFIER:
            return SYM_IDENTIFIER;
        case TOKEN_INTEGER:
            return SYM_INTEGER;
        case TOKEN_FLOAT:
            return SYM_FLOAT;
        case TOKEN_STRING:
            return SYM_STRING;
        case TOKEN_DIVIDE:
            return SYM_DIVIDE;
        case TOKEN_EQUALS:
            return SYM_EQUALS;
        case TOKEN_NOT_EQUALS:
            return SYM_NOT_EQUALS;
        case TOKEN_LESS:
            return SYM_LESS;
        case TOKEN_GREATER:
            return SYM_GREATER;
        case TOKEN_LESS_OR_EQUALS:
            return SYM_LESS_OR_EQUALS;
        case TOKEN_GREATER_OR_EQUALS:
            return SYM_GREATER_OR_EQUALS;
        default:
            return SYM_ERROR;
    }
}

ExpRules rule_select(Symbol s1,Symbol s2, Symbol s3) {
    if(s1 == SYM_NON_TERMINAL && s3 == SYM_NON_TERMINAL) {
        switch (s2) {
            case SYM_PLUS:
                return RL_PLUS;
            case SYM_MINUS:
                return RL_MINUS;
            case SYM_MULTIPLY:
                return RL_MULIPLY;
            case SYM_DIVIDE:
                return RL_DIVIDE;
            case SYM_CONCATENATE:
                return RL_CONCATENATE;
            case SYM_LESS:
                return RL_LESS;
            case SYM_GREATER:
                return RL_GREATER;
            case SYM_LESS_OR_EQUALS:
                return RL_LESS_OR_EQUALS;
            case SYM_GREATER_OR_EQUALS:
                return RL_GREAT_OR_EQUALS;
            case SYM_EQUALS:
                return RL_EQUALS;
            case SYM_NOT_EQUALS:
                return RL_NOT_EQUALS;
            default:
                return RL_ERROR;
        } 
    }
    else if(s1 == SYM_OPEN_CURLY_BRACKET && s2 == SYM_NON_TERMINAL && s3 == SYM_CLOSE_CURLY_BRACKET) {
        return RL_CURLY_BRACKETS;
    }
    else if(s1 == SYM_IDENTIFIER) {
        return RL_IDENTIFIER;
    }
    else return RL_ERROR;
}

int get_prec_tb_indx(Symbol s) {
  switch (s) {
    case SYM_IDENTIFIER:
        return 0;
    case SYM_OPEN_CURLY_BRACKET:
        return 1;
    case SYM_CLOSE_CURLY_BRACKET:
        return 2;
    case SYM_MULTIPLY:
    case SYM_DIVIDE:
        return 3;
    case SYM_PLUS:
    case SYM_MINUS:
        return 4;
    case SYM_CONCATENATE:
        return 5;
    case SYM_LESS:
    case SYM_LESS_OR_EQUALS:
    case SYM_GREATER:
    case SYM_GREATER_OR_EQUALS:
        return 6;
    case SYM_EQUALS:
    case SYM_NOT_EQUALS:
        return 7;
    case SYM_START_DOLLAR:
        return 8;
    default:
        return -1;
    }
}

int get_prec_operator(StackItem* top_term, Symbol s) {
    if(top_term == NULL) return -1;
    int top_term_indx = get_prec_tb_indx(top_term->symbol);
    int input_symbol_indx = get_prec_tb_indx(s);
    if(top_term_indx == -1 || input_symbol_indx == -1) return -1;
    return precedence_tb[top_term_indx][input_symbol_indx];
}

// debug
char* prec_tb_op_to_char(PrecTbOperators op) {
  switch(op) {
    case SFT:
        return "<";
    case RED:
        return ">";
    case EQU:
        return "=";
    case ERR:
        return "ERR";
    default:
        return "ERR";
    }
}

bool is_operator(Token token) {
    return token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_CONCATENATE || token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS || token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS || token.type == TOKEN_ASSIGN;
}

void shift(Stack* stack, Symbol s) {
    if(is_empty(stack)) return;
    push(stack, SYM_SHIFT);
    push(stack, s);
}

bool reduce(Stack* stack, Symbol s, Token token, Expression** exp) {
    StackItem* temp = top(stack);
    if(temp == NULL) return false;

    Symbol buffer[3]; 
    // loads symbols until symbol SFT(<) from stack
    for(int i=0; i < 3; ++i) {
        if(temp->symbol != SYM_SHIFT) {
            buffer[i] = temp->symbol;
            if(temp->next == NULL) break;
            temp = temp->next;
        }
    }

    // selects rule based on symbols in buffer
    ExpRules rule = rule_select(buffer[0], buffer[1], buffer[2]);
    if(rule == RL_ERROR) return false;
  
    // creates nodes in ast

    // TODOOOO

    //Expression__BinaryOperator * operator = Expression__BinaryOperator__init();
    //operator->operator = token.type;
    //*exp = (Expression*)operator; 

    //switch(rule) {
        //case RL_IDENTIFIER:      // E = i
        //case RL_CURLY_BRACKETS:  // E = ( E )
        //case RL_MULIPLY:         // E = E * E
        //case RL_DIVIDE:          // E = E / E
        //case RL_PLUS:            // E = E + E
        //case RL_MINUS:           // E = E - E
        //case RL_CONCATENATE:     // E = E . E
        //case RL_LESS:            // E = E < E
        //case RL_GREATER:         // E = E > E
        //case RL_LESS_OR_EQUALS:  // E = E <= E
        //case RL_GREAT_OR_EQUALS: // E = E >= E
        //case RL_EQUALS:          // E = E === E
        //case RL_NOT_EQUALS:      // E = E !== E
        //case RL_ERROR:            // error
    //}

    // changes expression for non-terminal on top of the stack
    while(stack->top->symbol != SYM_START_DOLLAR && stack->top->symbol != SYM_SHIFT) {
        pop(stack);
    } 
    push(stack, SYM_NON_TERMINAL);

    return true;
}

void equal(Stack* stack, Symbol s) {
    if(is_empty(stack)) return;
    push(stack, s);
}

bool parse_exp(Expression** exp) {
    Stack stack;
    init_stack(&stack);
    push(&stack, SYM_START_DOLLAR);

    Token token = getNextToken();
    Symbol input_symbol;
    while(is_operator(token)) {
        input_symbol = token_to_symbol(token);

        int prec_tb_op = get_prec_operator(top_term(&stack), input_symbol);
        if(prec_tb_op == -1) return false;

        switch(prec_tb_op) {
        case SFT:
            shift(&stack, input_symbol); break;
        case RED:
            if(!reduce(&stack, input_symbol, token, exp)) {
                fprintf(stderr, "Expression error"); // temporary
                pop_all(&stack);
                return false;
            }
            break;
        case EQU:
            equal(&stack, input_symbol);
            if(!reduce(&stack, input_symbol, token, exp)) {
                fprintf(stderr, "Expression error"); // temporary
                pop_all(&stack);
                return false;
            }
            break;
        case ERR:
            fprintf(stderr, "Expression error"); // temporary
            pop_all(&stack);
            return false;
        }
        token = getNextToken();
    }

    pop_all(&stack);
    return true;
}
