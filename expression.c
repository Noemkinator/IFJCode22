/**
 * @brief Implementace zpracovani vyrazu pomoci precedencni syntakticke analyzy (expression.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#include "expression.h"

int precedence_tb[TB_SIZE][TB_SIZE] = {
//  i    (    )   */   +-   .    <>   !=    $
  {ERR, ERR, RED, RED, RED, RED, RED, RED, RED},   // i  (ID)
  {SFT, SFT, EQU, SFT, SFT, SFT, SFT, SFT, ERR},   // (  (LPA)
  {ERR, ERR, RED, RED, RED, RED, RED, RED, RED},   // )  (RPA)
  {SFT, SFT, RED, RED, RED, RED, RED, RED, RED},   // */ (MULDIV)
  {SFT, SFT, RED, SFT, RED, RED, RED, RED, RED},   // +- (ADDSUB)
  {SFT, SFT, RED, SFT, RED, RED, RED, RED, RED},   // .  (CON)
  {SFT, SFT, RED, SFT, SFT, SFT, RED, RED, RED},   // <> (REL)
  {ERR, SFT, RED, SFT, SFT, SFT, SFT, RED, RED},   // != (EQNEQ)
  {SFT, SFT, ERR, SFT, SFT, SFT, SFT, SFT, ERR},   // $  (DLR)
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