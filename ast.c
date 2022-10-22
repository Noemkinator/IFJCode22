// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "ast.h"

StatementList* StatementList__init() {
    StatementList* this = malloc(sizeof(StatementList));
    this->super.statementType = STATEMENT_LIST;
    this->listSize = 0;
    this->statements = NULL;
    return this;
}

StatementList* StatementList__addStatement(StatementList* this, Statement* statement) {
    this->listSize++;
    this->statements = realloc(this->statements, this->listSize * sizeof(Statement*));
    this->statements[this->listSize - 1] = statement;
    return this;
}

StatementList* StatementList__append(StatementList* this, StatementList* statementList) {
    this->listSize += statementList->listSize;
    this->statements = realloc(this->statements, this->listSize * sizeof(Statement*));
    for(int i = 0; i < statementList->listSize; ++i) {
        this->statements[this->listSize - statementList->listSize + i] = statementList->statements[i];
    }
    return this;
}

Type tokenToType(Token token) {
    Type type;
    type.isRequired = false;
    type.type = TYPE_UNKNOWN;
    if(token.type != TOKEN_TYPE) {
        return type;
    }
    char * tokenText = getTokenText(token);
    if(tokenText[0] == '?') {
        type.isRequired = false;
        tokenText++;
    } else {
        type.isRequired = true;
    }
    if(strcmp(tokenText, "int") == 0) {
        type.type = TYPE_INT;
    } else if(strcmp(tokenText, "float") == 0) {
        type.type = TYPE_FLOAT;
    } else if(strcmp(tokenText, "string") == 0) {
        type.type = TYPE_STRING;
    } else if(strcmp(tokenText, "void") == 0) {
        type.type = TYPE_VOID;
    }
    return type;
}

Type Expression__Constant__getType(Expression__Constant *this) {
    return this->type;
}

Expression__Constant* Expression__Constant__init() {
    Expression__Constant *this = malloc(sizeof(Expression__Constant));
    this->super.expressionType = EXPRESSION_CONSTANT;
    this->super.getType = Expression__Constant__getType;
    this->type.isRequired = false;
    this->type.type = TYPE_UNKNOWN;
    return this;
}

Type Expression__Variable__getType(Expression__Variable *this) {
    Type type;
    type.isRequired = false;
    type.type = TYPE_UNKNOWN;
    return type;
}

Expression__Variable* Expression__Variable__init() {
    Expression__Variable *this = malloc(sizeof(Expression__Variable));
    this->super.expressionType = EXPRESSION_VARIABLE;
    this->super.getType = Expression__Variable__getType;
    this->name = NULL;
    return this;
}

Expression__FunctionCall* Expression__FunctionCall__init() {
    Expression__FunctionCall *this = malloc(sizeof(Expression__FunctionCall));
    this->super.expressionType = EXPRESSION_FUNCTION_CALL;
    this->name = NULL;
    this->arity = 0;
    this->arguments = NULL;
    return this;
}

Expression__FunctionCall* Expression__FunctionCall__addArgument(Expression__FunctionCall *this, Expression *argument) {
    this->arity++;
    this->arguments = realloc(this->arguments, this->arity * sizeof(Expression*));
    this->arguments[this->arity - 1] = argument;
    return this;
}

Expression__BinaryOperator* Expression__BinaryOperator__init() {
    Expression__BinaryOperator *this = malloc(sizeof(Expression__BinaryOperator));
    this->super.expressionType = EXPRESSION_BINARY_OPERATOR;
    this->lSide = NULL;
    this->rSide = NULL;
    return this;
}

StatementIf* StatementIf__init() {
    StatementIf *this = malloc(sizeof(StatementIf));
    this->super.statementType = STATEMENT_IF;
    this->condition = NULL;
    this->ifBody = NULL;
    this->elseBody = NULL;
    return this;
}

StatementWhile* StatementWhile__init() {
    StatementWhile *this = malloc(sizeof(StatementWhile));
    this->super.statementType = STATEMENT_WHILE;
    this->condition = NULL;
    this->body = NULL;
    return this;
}

StatementReturn* StatementReturn__init() {
    StatementReturn *this = malloc(sizeof(StatementReturn));
    this->super.statementType = STATEMENT_RETURN;
    this->expression = NULL;
    return this;
}

Function* Function__init() {
    Function *this = malloc(sizeof(Function));
    this->name = NULL;
    this->returnType.isRequired = false;
    this->returnType.type = TYPE_UNKNOWN;
    this->arity = 0;
    this->parameterTypes = NULL;
    this->parameterNames = NULL;
    return this;
}

Function* Function__addParameter(Function *this, Type type, char *name) {
    this->arity++;
    this->parameterTypes = realloc(this->parameterTypes, this->arity * sizeof(Type));
    this->parameterTypes[this->arity - 1] = type;
    this->parameterNames = realloc(this->parameterNames, this->arity * sizeof(char*));
    this->parameterNames[this->arity - 1] = name;
    return this;
}