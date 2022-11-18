/**
 * @file ast.h
 * @author Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)
 * @brief Abstract syntax tree library
 * @date 2022-10-26
 */

#include "lexer.h"
#include "string_builder.h"
#include "symtable.h"
#include <stdbool.h>
#include <string.h>

#ifndef __AST_H__
#define __AST_H__

/**
 * @brief Statement type enumeration
 */
typedef enum {
    STATEMENT_EXPRESSION,
    STATEMENT_LIST,
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_RETURN,
    STATEMENT_EXIT,
    STATEMENT_FUNCTION
} StatementType;

/**
 * @brief Statement structure
 */
typedef struct Statement {
    StatementType statementType;/*<Statement type>*/
    void (*serialize)(struct Statement *this, StringBuilder * stringBuilder);/*<Serialize function pointer>*/
    struct Statement *** (*getChildren)(struct Statement *this, int * childrenCount); /*<Get children function pointer>*/
    struct Statement * (*duplicate)(struct Statement *this);
    void (*free)(struct Statement *this);
} Statement;

/**
 * @brief Statement list structure
 */
typedef struct {
    Statement super;/*<Superclass>*/
    int listSize;/*<Size of the list>*/
    Statement ** statements;/*<List of statements>*/
} StatementList;

StatementList* StatementList__init();

StatementList* StatementList__addStatement(StatementList* this, Statement* statement);

StatementList* StatementList__append(StatementList* this, StatementList* statementList);

typedef struct {
    bool isRequired; /*<Is the parameter required>*/
    enum {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_BOOL,
        TYPE_NULL,
        TYPE_VOID,
        TYPE_UNKNOWN
    } type; /*<Type of the parameter>*/
} Type;

typedef struct {
    bool isInt;
    bool isFloat;
    bool isString;
    bool isBool;
    bool isNull;
    bool isUndefined;
    struct Expression * constant;
} UnionType;

Type tokenToType(Token token);
UnionType typeToUnionType(Type type);
Type unionTypeToType(UnionType unionType);

/**
 * @brief Expression type enumeration
 */
typedef enum {
    EXPRESSION_CONSTANT,
    EXPRESSION_VARIABLE,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_BINARY_OPERATOR
} ExpressionType;

struct Function;

/**
 * @brief Expression structure
 */
typedef struct Expression {
    Statement super;/*<Superclass>*/
    ExpressionType expressionType;/*<Expression type>*/
    UnionType (*getType)(struct Expression * expression, Table * functionTable, StatementList * program, struct Function * currentFunction);/*<Get type function pointer>*/
} Expression;

typedef struct Expression__Constant {
    Expression super;/*<Superclass>*/


    Type type;/*<Type of the constant>*/
    union {
        long long int integer;/*<Integer value>*/
        double real;/*<Real value>*/
        char *string;/*<String value>*/
        bool boolean;/*<Boolean value>*/
    } value;/*<Value of the constant>*/
} Expression__Constant;

Expression__Constant* Expression__Constant__init();

typedef struct {
    Expression super; /*<Superclass>*/

    char *name; /*<Name of the variable>*/
} Expression__Variable;

Expression__Variable* Expression__Variable__init();

typedef struct {
    Expression super; /*<Superclass>*/

    char * name; /*<Name of the function>*/
    int arity; /*<Arity of the function>*/
    Expression ** arguments;
} Expression__FunctionCall;

Expression__FunctionCall* Expression__FunctionCall__init();
Expression__FunctionCall* Expression__FunctionCall__addArgument(Expression__FunctionCall *this, Expression *argument);

typedef struct {
    Expression super; /*<Superclass>*/

    Expression * lSide; /*<Left side of the expression>*/
    Expression * rSide; /*<Right side of the expression>*/
    TokenType operator; /*<Operator>*/
} Expression__BinaryOperator;

Expression__BinaryOperator* Expression__BinaryOperator__init();

typedef struct {
    Statement super; /*<Superclass>*/

    Expression * condition; /*<Condition of the if statement>*/
    Statement * ifBody; /*<If body>*/
    Statement * elseBody; /*<Else body>*/
} StatementIf;

StatementIf* StatementIf__init();

typedef struct {
    Statement super; /*<Superclass>*/

    Expression * condition; /*<Condition of the while statement>*/
    Statement * body; /*<Body of the while statement>*/
} StatementWhile;

StatementWhile* StatementWhile__init();

typedef struct {
    Statement super; /*<Superclass>*/

    Expression * expression; /*<Expression to be returned>*/
} StatementReturn;

StatementReturn* StatementReturn__init();

typedef struct {
    Statement super; /*<Superclass>*/

    int exitCode; /*<Exit code of the program>*/
} StatementExit;

StatementExit* StatementExit__init();

typedef struct Function {
    Statement super; /*<Superclass>*/
    char * name; /*<Name of the function>*/
    Type returnType; /*<Return type of the function>*/
    int arity; /*<Arity of the function>*/
    Type * parameterTypes; /*<Types of the parameters>*/
    char ** parameterNames; /*<Names of the parameters>*/
    Table * globalVariables; /*<Global variables of the function>*/
    Statement * body; /*<Body of the function>*/
} Function;

Function* Function__init();

Function* Function__addParameter(Function *this, Type type, char *name);

#endif