// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "ast.h"

void StatementList__serialize(StatementList * this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_LIST\", \"statements\": [");
    for (int i = 0; i < this->listSize; i++) {
        if(this->statements[i] != NULL) {
            this->statements[i]->serialize(this->statements[i], stringBuilder);
        } else {
            StringBuilder__appendString(stringBuilder, "null");
        }
        StringBuilder__appendChar(stringBuilder, ',');
    }
    if(this->listSize > 0) {
        StringBuilder__removeLastChar(stringBuilder);
    }
    StringBuilder__appendString(stringBuilder, "]}");
}

StatementList* StatementList__init() {
    StatementList* this = malloc(sizeof(StatementList));
    this->super.serialize = StatementList__serialize;
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

void Expression__Constant__serialize(Expression__Constant *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_CONSTANT\", \"type\": \"");
    switch (this->type.type) {
        case TYPE_INT:
            StringBuilder__appendString(stringBuilder, "int");
            break;
        case TYPE_FLOAT:
            StringBuilder__appendString(stringBuilder, "float");
            break;
        case TYPE_STRING:
            StringBuilder__appendString(stringBuilder, "string");
            break;
        case TYPE_VOID:
            StringBuilder__appendString(stringBuilder, "void");
            break;
        case TYPE_UNKNOWN:
            StringBuilder__appendString(stringBuilder, "unknown");
            break;
    }
    StringBuilder__appendString(stringBuilder, "\", \"value\": \"");
    switch (this->type.type) {
        case TYPE_INT:
            StringBuilder__appendInt(stringBuilder, this->value.integer);
            break;
        case TYPE_FLOAT:
            StringBuilder__appendFloat(stringBuilder, this->value.real);
            break;
        case TYPE_STRING:
            StringBuilder__appendString(stringBuilder, this->value.string);
            break;
        case TYPE_VOID:
            StringBuilder__appendString(stringBuilder, "");
            break;
        case TYPE_UNKNOWN:
            StringBuilder__appendString(stringBuilder, "");
            break;
    }
    StringBuilder__appendString(stringBuilder, "\"}");
}

Type Expression__Constant__getType(Expression__Constant *this) {
    return this->type;
}

Expression__Constant* Expression__Constant__init() {
    Expression__Constant *this = malloc(sizeof(Expression__Constant));
    this->super.expressionType = EXPRESSION_CONSTANT;
    this->super.super.serialize = Expression__Constant__serialize;
    this->super.getType = Expression__Constant__getType;
    this->type.isRequired = false;
    this->type.type = TYPE_UNKNOWN;
    return this;
}

void Expression__Variable__serialize(Expression__Variable *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_VARIABLE\", \"name\": \"");
    StringBuilder__appendString(stringBuilder, this->name);
    StringBuilder__appendString(stringBuilder, "\"}");
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
    this->super.super.serialize = Expression__Variable__serialize;
    this->super.getType = Expression__Variable__getType;
    this->name = NULL;
    return this;
}

void Expression__FunctionCall__serialize(Expression__FunctionCall *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_FUNCTION_CALL\", \"name\": \"");
    StringBuilder__appendString(stringBuilder, this->name);
    StringBuilder__appendString(stringBuilder, "\", \"arguments\": [");
    for (int i = 0; i < this->arity; i++) {
        if(this->arguments[i] != NULL) {
            this->arguments[i]->super.serialize(this->arguments[i], stringBuilder);
        } else {
            StringBuilder__appendString(stringBuilder, "null");
        }
        StringBuilder__appendChar(stringBuilder, ',');
    }
    if(this->arity > 0) {
        StringBuilder__removeLastChar(stringBuilder);
    }
    StringBuilder__appendString(stringBuilder, "]}");
}

Type Expression__FunctionCall__getType(Expression__FunctionCall *this) {
    Type type;
    type.isRequired = false;
    type.type = TYPE_UNKNOWN;
    return type;
}

Expression__FunctionCall* Expression__FunctionCall__init() {
    Expression__FunctionCall *this = malloc(sizeof(Expression__FunctionCall));
    this->super.expressionType = EXPRESSION_FUNCTION_CALL;
    this->super.getType = Expression__FunctionCall__getType;
    this->super.super.serialize = Expression__FunctionCall__serialize;
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

void Expression__BinaryOperator__serialize(Expression__BinaryOperator *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_BINARY_OPERATION\", \"operator\": \"");
    StringBuilder__appendChar(stringBuilder, "TODO");
    StringBuilder__appendString(stringBuilder, "\", \"left\": ");
    if(this->lSide != NULL) {
        this->lSide->super.serialize(this->lSide, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"right\": ");
    if(this->rSide != NULL) {
        this->rSide->super.serialize(this->rSide, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

Type Expression__BinaryOperation__getType(Expression__BinaryOperator *this) {
    Type type;
    type.isRequired = false;
    type.type = TYPE_UNKNOWN;
    return type;
}

Expression__BinaryOperator* Expression__BinaryOperator__init() {
    Expression__BinaryOperator *this = malloc(sizeof(Expression__BinaryOperator));
    this->super.expressionType = EXPRESSION_BINARY_OPERATOR;
    this->lSide = NULL;
    this->rSide = NULL;
    return this;
}

void StatementIf__serialize(StatementIf *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_IF\", \"condition\": ");
    if(this->condition != NULL) {
        this->condition->super.serialize(this->condition, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"ifBody\": ");
    if(this->ifBody != NULL) {
        this->ifBody->serialize(this->ifBody, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"elseBody\": ");
    if(this->elseBody != NULL) {
        this->elseBody->serialize(this->elseBody, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

StatementIf* StatementIf__init() {
    StatementIf *this = malloc(sizeof(StatementIf));
    this->super.statementType = STATEMENT_IF;
    this->super.serialize = StatementIf__serialize;
    this->condition = NULL;
    this->ifBody = NULL;
    this->elseBody = NULL;
    return this;
}

void StatementWhile__serialize(StatementWhile *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_WHILE\", \"condition\": ");
    if(this->condition != NULL) {
        this->condition->super.serialize(this->condition, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"body\": ");
    if(this->body != NULL) {
        this->body->serialize(this->body, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

StatementWhile* StatementWhile__init() {
    StatementWhile *this = malloc(sizeof(StatementWhile));
    this->super.statementType = STATEMENT_WHILE;
    this->super.serialize = StatementWhile__serialize;
    this->condition = NULL;
    this->body = NULL;
    return this;
}

void StatementReturn__serialize(StatementReturn *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_RETURN\", \"expression\": ");
    if(this->expression != NULL) {
        this->expression->super.serialize(this->expression, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

StatementReturn* StatementReturn__init() {
    StatementReturn *this = malloc(sizeof(StatementReturn));
    this->super.statementType = STATEMENT_RETURN;
    this->super.serialize = StatementReturn__serialize;
    this->expression = NULL;
    return this;
}

void Function__serialize(Function *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"functionType\": \"FUNCTION\", \"name\": \"");
    StringBuilder__appendString(stringBuilder, this->name);
    StringBuilder__appendString(stringBuilder, "\", \"arity\": ");
    StringBuilder__appendInt(stringBuilder, this->arity);
    StringBuilder__appendString(stringBuilder, ", \"arguments\": [");
    for(int i = 0; i < this->arity; i++) {
        StringBuilder__appendString(stringBuilder, "{\"name\": \"");
        StringBuilder__appendString(stringBuilder, this->parameterNames[i]);
        StringBuilder__appendString(stringBuilder, "\", \"type\": \"");
        if(!this->parameterTypes[i].isRequired) {
            StringBuilder__appendString(stringBuilder, "?");
        }
        switch(this->parameterTypes[i].type) {
            case TYPE_INT:
                StringBuilder__appendString(stringBuilder, "int");
                break;
            case TYPE_FLOAT:
                StringBuilder__appendString(stringBuilder, "float");
                break;
            case TYPE_STRING:
                StringBuilder__appendString(stringBuilder, "string");
                break;
            case TYPE_UNKNOWN:
                StringBuilder__appendString(stringBuilder, "unknown");
                break;
        }
        StringBuilder__appendString(stringBuilder, "\"},");
    }
    if(this->arity > 0) {
        StringBuilder__removeLastChar(stringBuilder);
    }
    StringBuilder__appendString(stringBuilder, "], \"body\": ");
    if(this->body != NULL) {
        this->body->serialize(this->body, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

Function* Function__init() {
    Function *this = malloc(sizeof(Function));
    this->serialize = Function__serialize;
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