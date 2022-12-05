/**
 * Implementace překladače imperativního jazyka IFJ22
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
  {SFT, SFT, RED, SFT, SFT, SFT, SFT, RED, RED},   // != 
  {SFT, SFT, ERR, SFT, SFT, SFT, SFT, SFT, ERR},   // $  
};   

ExpRules rule_select(TokenType type1,TokenType type2, TokenType type3) {
    //printf("s1: %s, s2: %s, s3: %s\n", symbol_to_string(s1), symbol_to_string(s2), symbol_to_string(s3));
    if(type1 == TOKEN_NON_TERMINAL && type3 == TOKEN_NON_TERMINAL) {
        switch (type2) {
            case TOKEN_PLUS:
                return RL_PLUS;
            case TOKEN_MINUS:
                return RL_MINUS;
            case TOKEN_MULTIPLY:
                return RL_MULIPLY;
            case TOKEN_DIVIDE:
                return RL_DIVIDE;
            case TOKEN_CONCATENATE:
                return RL_CONCATENATE;
            case TOKEN_LESS:
                return RL_LESS;
            case TOKEN_GREATER:
                return RL_GREATER;
            case TOKEN_LESS_OR_EQUALS:
                return RL_LESS_OR_EQUALS;
            case TOKEN_GREATER_OR_EQUALS:
                return RL_GREAT_OR_EQUALS;
            case TOKEN_EQUALS:
                return RL_EQUALS;
            case TOKEN_NOT_EQUALS:
                return RL_NOT_EQUALS;
            default:
                return RL_ERROR;
        } 
    }
    else if(type1 == TOKEN_OPEN_BRACKET && type2 == TOKEN_NON_TERMINAL && type3 == TOKEN_CLOSE_BRACKET) {
        return RL_BRACKETS;
    }
    else if(type3 == TOKEN_VARIABLE || type3 == TOKEN_VARIABLE || type3 == TOKEN_FLOAT || type3 == TOKEN_INTEGER) {
        return RL_VARIABLE;
    }
    else return RL_ERROR;
}

int get_prec_tb_indx(TokenType type) {
  switch (type) {
    case TOKEN_VARIABLE:
        return 0;
    case TOKEN_OPEN_BRACKET:
        return 1;
    case TOKEN_CLOSE_BRACKET:
        return 2;
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
        return 3;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 4;
    case TOKEN_CONCATENATE:
        return 5;
    case TOKEN_LESS:
    case TOKEN_LESS_OR_EQUALS:
    case TOKEN_GREATER:
    case TOKEN_GREATER_OR_EQUALS:
        return 6;
    case TOKEN_EQUALS:
    case TOKEN_NOT_EQUALS:
        return 7;
    case TOKEN_STACK_DOLLAR:
        return 8;
    default:
        return -1;
    }
}

int get_prec_operator(StackItem* top_term, Token token) {
    if(top_term == NULL) return -1;
    int top_term_indx = get_prec_tb_indx(top_term->token.type);
    int input_symbol_indx = get_prec_tb_indx(token.type);
    if(top_term_indx == -1 || input_symbol_indx == -1) return -1;
    //printf("tb[%d][%d]\n", top_term_indx, input_symbol_indx);
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

char* rule_to_char(ExpRules rule) {
    switch(rule) {
        case RL_VARIABLE:        
            return "E = i";
        case RL_BRACKETS:        
            return "E = ( E )";
        case RL_MULIPLY:     
            return "E = E * E";
        case RL_DIVIDE:         
            return "E = E / E";
        case RL_PLUS:            
            return "E = E + E";
        case RL_MINUS:           
            return "E = E - E";
        case RL_CONCATENATE:     
            return "E = E . E";
        case RL_LESS:            
            return "E = E < E";
        case RL_GREATER:         
            return "E = E > E";
        case RL_LESS_OR_EQUALS:  
            return "E = E <= E";
        case RL_GREAT_OR_EQUALS: 
            return "E = E >= E";
        case RL_EQUALS:          
            return "E = E === E";
        case RL_NOT_EQUALS:      
            return "E = E !== E";
        case RL_ERROR:         
            return "error";
        default:
            return "not rule";
    }
}
bool is_valid_token(Token token) {
    return token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || token.type == TOKEN_CONCATENATE || token.type == TOKEN_LESS || token.type == TOKEN_LESS_OR_EQUALS || token.type == TOKEN_GREATER || token.type == TOKEN_GREATER_OR_EQUALS || token.type == TOKEN_EQUALS || token.type == TOKEN_NOT_EQUALS || token.type == TOKEN_VARIABLE || token.type == TOKEN_OPEN_BRACKET || token.type == TOKEN_CLOSE_BRACKET || token.type == TOKEN_STACK_DOLLAR;
}

void shift(Stack* stack, Token token) {
    if(is_empty(stack)) return;
    Token shift = init_shift();
    Token non_term = init_non_term();
    StackItem* temp = NULL;
    if(top(stack)->token.type == TOKEN_NON_TERMINAL) {
        temp = top(stack);
        printf("temp: %s (%d)\n", token_to_string(temp->token), temp->exp->expressionType);
        pop(stack);
        push(stack, shift);
        push(stack, non_term);
        top(stack)->exp = temp->exp;
        printf("top: %s (%d)\n", token_to_string(top(stack)->token), top(stack)->exp->expressionType);
    } else {
        push(stack, shift);
    }
    push(stack, token);
}

void reduce_expression(Stack* stack, Expression** exp) {
    while(stack->top->token.type != TOKEN_STACK_DOLLAR && stack->top->token.type != TOKEN_SHIFT) {
        pop(stack);
    } 
    pop(stack);
    Token non_term = init_non_term();
    push(stack, non_term);
    top(stack)->exp = *exp;
}

bool reduce(Stack* stack, Token token, Expression** exp, Expression** lSide, Expression** rSide, bool* is_var) {
    StackItem* temp = top(stack);
    if(temp == NULL) return false;
    TokenType buffer[3]; 
    // loads symbols until symbol SFT(<) from stack
    for(int i=0; i < 3; ++i) {
        if(temp->token.type != TOKEN_SHIFT) {
            buffer[i] = temp->token.type;
            //printf("buffer[%d]: %d\n", i, buffer[i]);
            if(temp->next == NULL) return false;
            temp = temp->next;
        }
    }

    // selects rule based on symbols in buffer
    ExpRules rule = rule_select(buffer[2], buffer[1], buffer[0]);
    printf("rule: %s\n", rule_to_char(rule));
    if(rule == RL_ERROR) return false;
  
    // creates nodes in ast
    Expression__BinaryOperator* operator = NULL;
    Expression__Variable* var = NULL;

    // changes expression for non-terminal on top of the stack

    switch(rule) {
        case RL_VARIABLE:        // E = i
            var = Expression__Variable__init();
            *exp = (Expression*)var;
            var->name = getTokenTextPermanent(token);
            *is_var = true;
            reduce_expression(stack, exp);
            break;
        case RL_BRACKETS:        // E = ( E )
            reduce_expression(stack, exp);
            break;
        case RL_MULIPLY:         // E = E * E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_MULTIPLY;
            operator->lSide = *lSide;
            operator->rSide = *rSide;
            *exp = (Expression*)operator; 
            *is_var = false;
            reduce_expression(stack, exp);
            break;
        case RL_DIVIDE:          // E = E / E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_DIVIDE;
            *exp = (Expression*)operator; 
            *is_var = false;
            break;
        case RL_PLUS:            // E = E + E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_PLUS;
            operator->lSide = *lSide;
            operator->rSide = *rSide;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_MINUS:           // E = E - E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_MINUS;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_CONCATENATE:     // E = E . E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_CONCATENATE;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_LESS:            // E = E < E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_LESS;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_GREATER:         // E = E > E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_GREATER;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_LESS_OR_EQUALS:  // E = E <= E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_LESS_OR_EQUALS;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_GREAT_OR_EQUALS: // E = E >= E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_GREATER_OR_EQUALS;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_EQUALS:          // E = E === E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_EQUALS;
            *exp = (Expression*)operator; 
            *is_var = false;
            reduce_expression(stack, exp);
            break;
        case RL_NOT_EQUALS:      // E = E !== E
            operator = Expression__BinaryOperator__init();
            operator->operator = TOKEN_NOT_EQUALS;
            *exp = (Expression*)operator; 
            reduce_expression(stack, exp);
            *is_var = false;
            break;
        case RL_ERROR:            // error
            break;
        default:
    }    

    return true;
}

void equal(Stack* stack, Token token) {
    if(is_empty(stack)) return;
    push(stack, token);
}

bool parse_exp(Expression** exp, Token first_token) {
    Stack stack;
    init_stack(&stack);
    Token dollar = init_dollar();
    push(&stack, dollar);

    Expression* lSide = NULL;
    Expression* rSide = NULL;
    bool var = false;

    int bracket_cnt = -1;
    bool start = false;

    Token token = first_token;
    Token previous = first_token;
    while(is_valid_token(token)) {
        if(token.type == TOKEN_CLOSE_BRACKET && bracket_cnt == 0) token.type = TOKEN_STACK_DOLLAR;
        // temp exit for json output
        if(token.type == TOKEN_STACK_DOLLAR && top_term(&stack)->token.type == TOKEN_STACK_DOLLAR) {
            pop_all(&stack);
            return true;
        }

        if(!start) {
            ++bracket_cnt;
            start = true;
        }

        puts("------");
        print_stack(&stack);
        int prec_tb_op = get_prec_operator(top_term(&stack), token);
        printf("operator: %s\n", prec_tb_op_to_char(prec_tb_op));
        if(prec_tb_op == -1) return false;

        switch(prec_tb_op) {
            case SFT:
                if(token.type == TOKEN_OPEN_BRACKET) ++bracket_cnt;
                shift(&stack, token);
                previous = token;
                token = getNextToken();
                break;
            case RED:
                if(!reduce(&stack, previous, exp, &lSide, &rSide, &var)) {
                    fprintf(stderr, "Expression error.\n"); // temporary
                    pop_all(&stack);
                    return false;
                }
                if(var) {
                    if(lSide == NULL) {
                        lSide = top(&stack)->exp;
                    } else {
                        rSide = top(&stack)->exp;
                    }
                } else {
                    printf("HERE\n");
                    print_stack(&stack);
                    printf("HERE\n");
                    rSide = top(&stack)->exp;
                }
                break;
            case EQU:
                --bracket_cnt;
                equal(&stack, token);
                if(!reduce(&stack, token, exp, &lSide, &rSide, &var)) {
                    fprintf(stderr, "Expression error.\n"); // temporary
                    pop_all(&stack);
                    return false;
                }
                previous = token;
                token = getNextToken();
                break;
            case ERR:
                fprintf(stderr, "Expression error.\n"); // temporary
                pop_all(&stack);
                return false;
        }
        //printf("cnt: %d\n", bracket_cnt);
        //printf("while token: %s\n", token_to_string(token));
    }
    puts("------ after while");
    print_stack(&stack);
    pop_all(&stack);
    return true;
}