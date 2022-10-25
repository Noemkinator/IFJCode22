// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "lexer.h"
#include "string_builder.h"
#include <stdbool.h>
#include <string.h>

#ifndef __AST_H__
#define __AST_H__

typedef enum {
    STATEMENT_EXPRESSION,
    STATEMENT_LIST,
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_RETURN,
    STATEMENT_FUNCTION
} StatementType;

typedef struct Statement {
    StatementType statementType;
    void (*serialize)(struct Statement *this, StringBuilder * stringBuilder);
    struct Statement *** (*getChildren)(struct Statement *this, int * childrenCount);
} Statement;

typedef struct {
    Statement super;
    int listSize;
    Statement ** statements;
} StatementList;

StatementList* StatementList__init();

StatementList* StatementList__addStatement(StatementList* this, Statement* statement);

StatementList* StatementList__append(StatementList* this, StatementList* statementList);

typedef struct {
    bool isRequired;
    enum {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_VOID,
        TYPE_UNKNOWN
    } type;
} Type;

Type tokenToType(Token token);

typedef enum {
    EXPRESSION_CONSTANT,
    EXPRESSION_VARIABLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_BINARY_OPERATOR
} ExpressionType;

typedef struct {
    Statement super;
    ExpressionType expressionType;
    Type (*getType)();
} Expression;

typedef struct {
    Expression super;


    Type type;
    union {
        int integer;
        double real;
        char *string;
    } value;
} Expression__Constant;

Type Expression__Constant__getType(Expression__Constant *this);

Expression__Constant* Expression__Constant__init();

typedef struct {
    Expression super;

    char *name;
} Expression__Variable;

Type Expression__Variable__getType(Expression__Variable *this);

Expression__Variable* Expression__Variable__init();

typedef struct {
    Expression super;

    char * name;
    int arity;
    Expression ** arguments;
} Expression__FunctionCall;

Expression__FunctionCall* Expression__FunctionCall__init();

Expression__FunctionCall* Expression__FunctionCall__addArgument(Expression__FunctionCall *this, Expression *argument);

typedef struct {
    Expression super;

    Expression * lSide;
    Expression * rSide;
    TokenType operator;
} Expression__BinaryOperator;

Expression__BinaryOperator* Expression__BinaryOperator__init();

typedef struct {
    Statement super;

    Expression * condition;
    Statement * ifBody;
    Statement * elseBody;
} StatementIf;

StatementIf* StatementIf__init();

typedef struct {
    Statement super;

    Expression * condition;
    Statement * body;
} StatementWhile;

StatementWhile* StatementWhile__init();

typedef struct {
    Statement super;

    Expression * expression;
} StatementReturn;

StatementReturn* StatementReturn__init();

typedef struct {
    Statement super;
    char * name;
    Type returnType;
    int arity;
    Type * parameterTypes;
    char ** parameterNames;
    Statement * body;
} Function;

Function* Function__init();

Function* Function__addParameter(Function *this, Type type, char *name);

#endif