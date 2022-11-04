/**
 * @brief Implementace zpracovani vyrazu pomoci precedencni syntakticke analyzy (expression.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#include "expression.h"
#include "ast.h"

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
    }
}

int get_prec_operator(StackItem* top_term, Symbol s) {
    if(top_term == NULL) return -1;
    int top_term_indx = get_prec_tb_indx(top_term->symbol);
    int input_symbol_indx = get_prec_tb_indx(s);
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
    }
}

void shift(Stack* stack, Symbol s) {
    if(is_empty(stack)) return;
    push(stack, SYM_SHIFT);
    push(stack, s);
}

void reduce(Stack* stack, Symbol s) {
    StackItem* temp = top(stack);
    if(temp == NULL) return;

    Symbol buffer[3]; 
    for(int i=0; i < 3; ++i) {
        if(temp->symbol != SYM_SHIFT) {
            buffer[i] = temp->symbol;
            if(temp->next == NULL) break;
            temp = temp->next;
        }
    }

    ExpRules rule = rule_select(buffer[0], buffer[1], buffer[2]);
  
    // create nodes in ast

    while(stack->top->symbol != SYM_START_DOLLAR && stack->top->symbol != SYM_SHIFT) {
        pop(stack);
    } 

    push(stack, SYM_NON_TERMINAL);
}

void equal(Stack* stack, Symbol s) {
    if(is_empty(stack)) return;
    push(stack, s);
    reduce(stack, s);
}

bool parse_expression() {
    Stack stack;
    init_stack(&stack);
    push(&stack, SYM_START_DOLLAR);
    //push(&stack, SYM_OPEN_CURLY_BRACKET);
    //push(&stack, SYM_NON_TERMINAL);

    //Token token = getNextToken();
    Symbol input_symbol;
    //while(is_operator(token)) {
        //input_symbol = token_to_symbol(token);
        //input_symbol = SYM_CLOSE_CURLY_BRACKET;
        input_symbol = SYM_IDENTIFIER;

        int op = get_prec_operator(top_term(&stack), input_symbol);
        if(op == -1) return false;
        //printf("op: %s\n", prec_tb_op_to_char(op));

        switch(op) {
        case SFT:
            shift(&stack, input_symbol); break;
        case RED:
            reduce(&stack, input_symbol); break;
        case EQU:
            equal(&stack, input_symbol); break;
        case ERR:
            fprintf(stderr, "Expression error");
            pop_all(&stack);
            return false;
        }
        //token = getNextToken();
    //}
    //puts("-------------");
    //print_stack(&stack);

    pop_all(&stack);
    return true;
}

int main(void) {
    parse_expression();
}
