/**
 * @file ast.c
 * @author Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)
 * @brief Abstract syntax tree
 * @date 2022-10-26
 */

#include "ast.h"

/**
 * @brief Statement list serializer
 * 
 * @param this 
 * @param stringBuilder 
 */
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

/**
 * @brief Get children of statement list
 * 
 * @param this 
 * @param childrenCount 
 * @return Statement*** 
 */
Statement *** StatementList__getChildren(StatementList * this, int * childrenCount) {
    *childrenCount = this->listSize;
    Statement *** children = malloc(sizeof(Statement**) * this->listSize);
    for(int i = 0; i < this->listSize; i++) {
        children[i] = &this->statements[i];
    }
    return children;
}

StatementList* StatementList__duplicate(StatementList* this) {
    StatementList* duplicate = StatementList__init();
    for(int i=0; i < this->listSize; ++i) {
        StatementList__addStatement(duplicate, this->statements[i]->duplicate(this->statements[i]));
    }
    duplicate->listSize = this->listSize;
    return duplicate;
}

void StatementList__free(StatementList* this) {
    if(this == NULL) return;
    for(int i=0; i < this->listSize; ++i) {
        this->statements[i]->free(this->statements[i]);
    }
    free(this->statements);
    free(this);
}

/**
 * @brief Statement list constructor
 * 
 * @return StatementList* 
 */
StatementList* StatementList__init() {
    StatementList* this = malloc(sizeof(StatementList));
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementList__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementList__getChildren;
    this->super.statementType = STATEMENT_LIST;
    this->super.duplicate = (struct Statement* (*)(struct Statement *))StatementList__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementList__free;
    this->listSize = 0;
    this->statements = NULL;
    return this;
}

/**
 * @brief Add statement to statement list
 * 
 * @param this 
 * @param statement 
 * @return StatementList* 
 */
StatementList* StatementList__addStatement(StatementList* this, Statement* statement) {
    this->listSize++;
    this->statements = realloc(this->statements, this->listSize * sizeof(Statement*));
    this->statements[this->listSize - 1] = statement;
    return this;
}

/**
 * @brief Append statement list to statement list
 * 
 * @param this 
 * @param statementList 
 * @return StatementList* 
 */
StatementList* StatementList__append(StatementList* this, StatementList* statementList) {
    this->listSize += statementList->listSize;
    this->statements = realloc(this->statements, this->listSize * sizeof(Statement*));
    for(int i = 0; i < statementList->listSize; ++i) {
        this->statements[this->listSize - statementList->listSize + i] = statementList->statements[i];
    }
    return this;
}

/**
 * @brief Convert token to type
 * 
 * @param token 
 * @return Type 
 */
Type tokenToType(Token token) {
    Type type;
    type.isRequired = false;
    type.type = TYPE_UNKNOWN;
    if(token.type != TOKEN_TYPE) {
        if(token.type == TOKEN_VOID) {
            type.type = TYPE_VOID;
            type.isRequired = true;
        }
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
    } else if(strcmp(tokenText, "boolean") == 0) {
        type.type = TYPE_BOOL;
    }
    return type;
}

/**
 * @brief Constant expression serializer
 * 
 * @param type 
 * @return char* 
 */
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
        case TYPE_BOOL:
            StringBuilder__appendString(stringBuilder, "boolean");
            break;
        case TYPE_NULL:
            StringBuilder__appendString(stringBuilder, "null");
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
            StringBuilder__appendEscapedStr(stringBuilder, this->value.string);
            break;
        case TYPE_BOOL:
            StringBuilder__appendString(stringBuilder, this->value.boolean ? "true" : "false");
            break;
        case TYPE_NULL:
            StringBuilder__appendString(stringBuilder, "null");
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

/**
 * @brief Get constant expression children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** Expression__Constant__getChildren(Expression__Constant *this, int * childrenCount) {
    *childrenCount = 0;
    return NULL;
}

UnionType typeToUnionType(Type type) {
    return (UnionType) {
        .isBool = type.type == TYPE_BOOL || type.type == TYPE_UNKNOWN,
        .isFloat = type.type == TYPE_FLOAT || type.type == TYPE_UNKNOWN,
        .isInt = type.type == TYPE_INT || type.type == TYPE_UNKNOWN,
        .isNull = type.type == TYPE_NULL || type.type == TYPE_UNKNOWN || type.type == TYPE_VOID || !type.isRequired,
        .isString = type.type == TYPE_STRING || type.type == TYPE_UNKNOWN,
        .isUndefined = type.type == TYPE_UNKNOWN,
        .constant = NULL
    };
}

Type unionTypeToType(UnionType unionType) {
    Type type;
    type.isRequired = !unionType.isNull;
    if(unionType.isUndefined) {
        type.type = TYPE_UNKNOWN;
        return type;
    }
    if(unionType.isBool && !unionType.isFloat && !unionType.isInt && !unionType.isString) {
        type.type = TYPE_BOOL;
        return type;
    } else if(!unionType.isBool && unionType.isFloat && !unionType.isInt && !unionType.isString) {
        type.type = TYPE_FLOAT;
        return type;
    } else if(!unionType.isBool && !unionType.isFloat && unionType.isInt && !unionType.isString) {
        type.type = TYPE_INT;
        return type;
    } else if(!unionType.isBool && !unionType.isFloat && !unionType.isInt && unionType.isString) {
        type.type = TYPE_STRING;
        return type;
    } else if(!unionType.isBool && !unionType.isFloat && !unionType.isInt && !unionType.isString && unionType.isNull) {
        type.type = TYPE_NULL;
        return type;
    }
    type.type = TYPE_UNKNOWN;
    return type;
}

/**
 * @brief Get constant expression type
 * 
 * @param this 
 * @return Type 
 */
UnionType Expression__Constant__getType(Expression__Constant *this, Table * functionTable, StatementList * program, Function * currentFunction) {
    UnionType type = typeToUnionType(this->type);
    type.constant = this;
    return type;
}

/**
 * @brief Duplicates Expression__Constant
 * 
 * @param this 
 * @return Expression__Constant* 
 */
Expression__Constant* Expression__Constant__duplicate(Expression__Constant* this) {
    Expression__Constant* duplicate = Expression__Constant__init();
    duplicate->type = this->type;
    duplicate->value = this->value;
    return duplicate;
}

void Expression__Constant__free(Expression__Constant* this) {
    if(this->type.type == TYPE_STRING) {
        //free(this->value.string);
    }
    free(this);
}

/**
 * @brief Constant expression constructor
 * 
 * @param type 
 * @return Expression__Constant* 
 */
Expression__Constant* Expression__Constant__init() {
    Expression__Constant *this = malloc(sizeof(Expression__Constant));
    this->super.expressionType = EXPRESSION_CONSTANT;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__Constant__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__Constant__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__Constant__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__Constant__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *))Expression__Constant__getType;
    this->type.isRequired = false;
    this->type.type = TYPE_UNKNOWN;
    return this;
}

/**
 * @brief Variable expression serializer
 * 
 * @param type 
 */
void Expression__Variable__serialize(Expression__Variable *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_VARIABLE\", \"name\": \"");
    StringBuilder__appendEscapedStr(stringBuilder, this->name);
    StringBuilder__appendString(stringBuilder, "\"}");
}

/**
 * @brief Get variable expression children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** Expression__Variable__getChildren(Expression__Variable *this, int * childrenCount) {
    *childrenCount = 0;
    return NULL;
}

Table * dublicateVarTypeTable(Table * table) {
    Table * new_table = table_init();
    for (int i = 0; i < TB_SIZE; i++) {
        TableItem * item = table->tb[i];
        TableItem ** new_item = &new_table->tb[i];
        while (item != NULL) {
            UnionType type = *(UnionType *)item->data;
            UnionType * new_type = malloc(sizeof(UnionType));
            *new_type = type;
            *new_item = malloc(sizeof(TableItem));
            (*new_item)->name = item->name;
            (*new_item)->data = new_type;
            (*new_item)->next = NULL;
            item = item->next;
            new_item = &(*new_item)->next;
        }
    }
    return new_table;
}

UnionType orUnionType(UnionType type1, UnionType type2) {
    UnionType ret;
    ret.isBool = type1.isBool || type2.isBool;
    ret.isFloat = type1.isFloat || type2.isFloat;
    ret.isInt = type1.isInt || type2.isInt;
    ret.isNull = type1.isNull || type2.isNull;
    ret.isString = type1.isString || type2.isString;
    ret.isUndefined = type1.isUndefined || type2.isUndefined;
    ret.constant = NULL;
    return ret;
}

UnionType propagateUnionType(UnionType type1, UnionType type2) {
    UnionType ret;
    ret.isBool = type1.isBool || type2.isBool;
    ret.isFloat = type1.isFloat || type2.isFloat;
    ret.isInt = type1.isInt || type2.isInt;
    ret.isNull = type1.isNull || type2.isNull;
    ret.isString = type1.isString || type2.isString;
    ret.isUndefined = type1.isUndefined || type2.isUndefined;
    if(type1.constant != NULL) {
        ret.constant = type1.constant;
    } else {
        ret.constant = type2.constant;
    }
    return ret;
}

UnionType getExpressionVarType(Expression__Variable * variable, Table * functionTable, Expression * expression, Table * variableTable, UnionType * exprTypeRet) {
    switch (expression->expressionType) {
        case EXPRESSION_CONSTANT:
            if(exprTypeRet != NULL) {
                Expression__Constant* constant = (Expression__Constant*)expression;
                *exprTypeRet = typeToUnionType(constant->type);
                exprTypeRet->constant = constant;
            }
            return (UnionType){0};
        case EXPRESSION_VARIABLE: {
            Expression__Variable* var = (Expression__Variable*)expression;
            UnionType type = *(UnionType*)table_find(variableTable, var->name)->data;
            if(exprTypeRet != NULL) {
                *exprTypeRet = type;
            }
            if((Expression__Variable*)expression == variable) {
                return type;
            }
            return (UnionType){0};
        }
        case EXPRESSION_FUNCTION_CALL: {
            Expression__FunctionCall* func = (Expression__FunctionCall*)expression;
            UnionType ret = (UnionType){0};
            for(int i = 0; i < func->arity; i++) {
                ret = propagateUnionType(ret, getExpressionVarType(variable, functionTable, func->arguments[i], variableTable, NULL));
            }
            if(exprTypeRet != NULL) {
                Function * function = (Function*)table_find(functionTable, func->name)->data;
                *exprTypeRet = typeToUnionType(function->returnType);
            }
            return ret;
        }
        case EXPRESSION_BINARY_OPERATOR: {
            Expression__BinaryOperator* binOp = (Expression__BinaryOperator*)expression;
            UnionType lType;
            UnionType rType;
            UnionType type1 = getExpressionVarType(variable, functionTable, binOp->lSide, variableTable, &lType);
            UnionType type2 = getExpressionVarType(variable, functionTable, binOp->rSide, variableTable, &rType);
            if(binOp->operator == TOKEN_ASSIGN && binOp->lSide->expressionType == EXPRESSION_VARIABLE) {
                UnionType assignedType = rType;
                assignedType.isUndefined = false;
                *(UnionType*)table_find(variableTable, ((Expression__Variable*)binOp->lSide)->name)->data = assignedType;
            }
            if(exprTypeRet == NULL) {
                return propagateUnionType(type1, type2);
            }
            *exprTypeRet = (UnionType){0};
            switch (binOp->operator) {
                case TOKEN_PLUS:
                case TOKEN_MINUS:
                case TOKEN_MULTIPLY: {
                    exprTypeRet->isInt = lType.isInt && rType.isInt;
                    exprTypeRet->isFloat = lType.isFloat || rType.isFloat;
                    break;
                }
                case TOKEN_CONCATENATE:
                    exprTypeRet->isString = true;
                    break;
                case TOKEN_DIVIDE:
                    exprTypeRet->isFloat = true;
                    break;
                case TOKEN_ASSIGN:
                    *exprTypeRet = rType;
                    break;
                case TOKEN_EQUALS:
                case TOKEN_NOT_EQUALS:
                case TOKEN_LESS:
                case TOKEN_GREATER:
                case TOKEN_LESS_OR_EQUALS:
                case TOKEN_GREATER_OR_EQUALS:
                    exprTypeRet->isBool = true;
                    break;
                default:
                    break;
            }
            return propagateUnionType(type1, type2);
            break;
        }
    }
    return (UnionType){0};
}

UnionType getStatementListVarType(Expression__Variable * variable, Table * functionTable, StatementList * statementList, Table * variableTable);

UnionType getStatementVarType(Expression__Variable * variable, Table * functionTable, Statement * statement, Table * variableTable) {
    switch(statement->statementType) {
        case STATEMENT_EXPRESSION:
            return getExpressionVarType(variable, functionTable, (Expression*)statement, variableTable, NULL);
        case STATEMENT_RETURN:
            return getExpressionVarType(variable, functionTable, ((StatementReturn*)statement)->expression, variableTable, NULL);
        case STATEMENT_IF: {
            StatementIf* ifStatement = (StatementIf*)statement;
            UnionType typeCond = getExpressionVarType(variable, functionTable, ifStatement->condition, variableTable, NULL);
            Table * duplTable = dublicateVarTypeTable(variableTable);
            UnionType type1 = getStatementVarType(variable, functionTable, ifStatement->ifBody, variableTable);
            UnionType type2 = getStatementVarType(variable, functionTable, ifStatement->elseBody, duplTable);
            // or the tables
            for(int j=0; j<TB_SIZE; j++) {
                TableItem * itemA = variableTable->tb[j];
                TableItem * itemB = duplTable->tb[j];
                while(itemA != NULL && itemB != NULL) {
                    if(strcmp(itemA->name, itemB->name) != 0) {
                        fprintf(stderr, "Error: merging of variable tables failed");
                        exit(99);
                    }
                    UnionType * typeA = (UnionType*)itemA->data;
                    UnionType * typeB = (UnionType*)itemB->data;
                    *typeA = orUnionType(*typeA, *typeB);
                    itemA = itemA->next;
                    itemB = itemB->next;
                }
                if(itemA != NULL || itemB != NULL) {
                    fprintf(stderr, "Error: merging of variable tables failed");
                    exit(99);
                }
            }
            // TODO: free also content
            table_free(duplTable);
            return propagateUnionType(typeCond, propagateUnionType(type1, type2));
        }
        case STATEMENT_WHILE: {
            StatementWhile* whileStatement = (StatementWhile*)statement;
            UnionType typeCond = getExpressionVarType(variable, functionTable, whileStatement->condition, variableTable, NULL);
            Table * duplTable = dublicateVarTypeTable(variableTable);
            UnionType type = (UnionType){0};
            bool changed = true;
            while(changed) {
                changed = false;
                type = orUnionType(type, getStatementVarType(variable, functionTable, whileStatement->body, duplTable));
                type = orUnionType(type, getExpressionVarType(variable, functionTable, whileStatement->condition, duplTable, NULL));
                // or the tables
                for(int j=0; j<TB_SIZE; j++) {
                    TableItem * item1 = variableTable->tb[j];
                    TableItem * item2 = duplTable->tb[j];
                    while(item1 != NULL && item2 != NULL) {
                        if(strcmp(item1->name, item2->name) != 0) {
                            fprintf(stderr, "Error: merging of variable tables failed");
                            exit(99);
                        }
                        UnionType * type1 = (UnionType*)item1->data;
                        UnionType * type2 = (UnionType*)item2->data;
                        changed |= (!type1->isBool && type2->isBool) || (!type1->isFloat && type2->isFloat) || (!type1->isInt && type2->isInt) || (!type1->isString && type2->isString) || (!type1->isUndefined && type2->isUndefined);
                        *type1 = orUnionType(*type1, *type2);
                        item1 = item1->next;
                        item2 = item2->next;
                    }
                    if(item1 != NULL || item2 != NULL) {
                        fprintf(stderr, "Error: merging of variable tables failed");
                        exit(99);
                    }
                }
            }
            // TODO: free also content
            table_free(duplTable);
            return orUnionType(typeCond, type);
            break;
        }
        case STATEMENT_LIST:
            return getStatementListVarType(variable, functionTable, (StatementList*)statement, variableTable);
        default:
            break;
    }
    return (UnionType){0};
}

UnionType getStatementListVarType(Expression__Variable * variable, Table * functionTable, StatementList * statementList, Table * variableTable) {
    UnionType type = {0};
    for(int i=0; i<statementList->listSize; i++) {
        type = propagateUnionType(type, getStatementVarType(variable, functionTable, statementList->statements[i], variableTable));
    }
    return type;
}

// TODO: move the function elsewhere...
Statement *** getAllStatements(Statement*, size_t*);

/**
 * @brief Get variable expression type
 * 
 * @param this 
 * @return Type 
 */
UnionType Expression__Variable__getType(Expression__Variable *this, Table * functionTable, StatementList * program, Function * currentFunction) {
    UnionType returnType;
    Table * variableTable = table_init();
    if(currentFunction != NULL) {
        for(int i=0; i<currentFunction->arity; i++) {
            UnionType type = typeToUnionType(currentFunction->parameterTypes[i]);
            UnionType * typePerm = malloc(sizeof(UnionType));
            *typePerm = type;
            table_insert(variableTable, currentFunction->parameterNames[i], typePerm);
        }
        size_t statementCount;
        Statement *** allStatements = getAllStatements((Statement*)currentFunction->body, &statementCount);
        for(int i=0; i<statementCount; i++) {
            Statement * statement = *allStatements[i];
            if(statement == NULL) continue;
            if(statement->statementType == STATEMENT_EXPRESSION && ((Expression*)statement)->expressionType == EXPRESSION_VARIABLE) {
                Expression__Variable* variable = (Expression__Variable*) statement;
                if(table_find(variableTable, variable->name) == NULL) {
                    UnionType * type = malloc(sizeof(UnionType));
                    *type = (UnionType){0};
                    type->isUndefined = true;
                    table_insert(variableTable, variable->name, type);
                }
            }
        }
        returnType = getStatementVarType(this, functionTable, currentFunction->body, variableTable);
    } else {
        size_t statementCount;
        Statement *** allStatements = getAllStatements((Statement*)program, &statementCount);
        for(int i=0; i<statementCount; i++) {
            Statement * statement = *allStatements[i];
            if(statement == NULL) continue;
            if(statement->statementType == STATEMENT_EXPRESSION && ((Expression*)statement)->expressionType == EXPRESSION_VARIABLE) {
                Expression__Variable* variable = (Expression__Variable*) statement;
                if(table_find(variableTable, variable->name) == NULL) {
                    UnionType * type = malloc(sizeof(UnionType));
                    *type = (UnionType){0};
                    type->isUndefined = true;
                    table_insert(variableTable, variable->name, type);
                }
            }
        }
        returnType = getStatementListVarType(this, functionTable, program, variableTable);
    }
    // TODO: free also content
    table_free(variableTable);
    return returnType;
}

/**
 * @brief Duplicates Expression__Variable
 * 
 * @param this 
 * @return Expression__Variable* 
 */
Expression__Variable* Expression__Variable__duplicate(Expression__Variable* this) {
    Expression__Variable* duplicate = Expression__Variable__init();
    duplicate->name = (this->name != NULL ? strdup(this->name) : NULL);
    return duplicate;
}

void Expression__Variable__free(Expression__Variable* this) {
    //free(this->name);
    free(this);
}

/**
 * @brief Variable expression constructor
 * 
 * @param type 
 * @return Expression__Variable* 
 */
Expression__Variable* Expression__Variable__init() {
    Expression__Variable *this = malloc(sizeof(Expression__Variable));
    this->super.expressionType = EXPRESSION_VARIABLE;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__Variable__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__Variable__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__Variable__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__Variable__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *))Expression__Variable__getType;
    this->name = NULL;
    return this;
}

/**
 * @brief Function call expression serializer
 * 
 * @param type 
 */
void Expression__FunctionCall__serialize(Expression__FunctionCall *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_FUNCTION_CALL\", \"name\": \"");
    StringBuilder__appendEscapedStr(stringBuilder, this->name);
    StringBuilder__appendString(stringBuilder, "\", \"arguments\": [");
    for (int i = 0; i < this->arity; i++) {
        if(this->arguments[i] != NULL) {
            this->arguments[i]->super.serialize((Statement*)this->arguments[i], stringBuilder);
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

/**
 * @brief Get function call expression children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** Expression__FunctionCall__getChildren(Expression__FunctionCall *this, int * childrenCount) {
    *childrenCount = this->arity;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    for(int i = 0; i < *childrenCount; ++i) {
        children[i] = (Statement**) &this->arguments[i];
    }
    return children;
}

/**
 * @brief Get function call expression type
 * 
 * @param this 
 * @return Type 
 */
UnionType Expression__FunctionCall__getType(Expression__FunctionCall *this, Table * functionTable, StatementList * program, Function * currentFunction) {
    Type type;
    TableItem * item = table_find(functionTable, this->name);
    if(item != NULL) {
        type = ((Function*)item->data)->returnType;
    } else {
        type.isRequired = false;
        type.type = TYPE_UNKNOWN;
    }
    return typeToUnionType(type);
}

/**
 * @brief Duplicates Expression__FunctionCall
 * 
 * @param this 
 * @return Expression__FunctionCall* 
 */
Expression__FunctionCall* Expression__FunctionCall__duplicate(Expression__FunctionCall* this) {
    Expression__FunctionCall* duplicate = Expression__FunctionCall__init();
    for(int i=0; i < this->arity; ++i) {
        Expression__FunctionCall__addArgument(duplicate, (Expression*)this->arguments[i]->super.duplicate((Statement*)this->arguments[i]));
    }
    duplicate->arity = this->arity;
    duplicate->name = (this->name != NULL ? strdup(this->name) : NULL);
    return duplicate;
}

void Expression__FunctionCall__free(Expression__FunctionCall* this) {
    if(this == NULL) return;
    for(int i=0; i < this->arity; ++i) {
        this->arguments[i]->super.free((Statement*)this->arguments[i]);
    }
    free(this->arguments);
    free(this);
}


/**
 * @brief Function call expression constructor
 * 
 * @param type 
 * @return Expression__FunctionCall* 
 */
Expression__FunctionCall* Expression__FunctionCall__init() {
    Expression__FunctionCall *this = malloc(sizeof(Expression__FunctionCall));
    this->super.expressionType = EXPRESSION_FUNCTION_CALL;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *))Expression__FunctionCall__getType;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__FunctionCall__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__FunctionCall__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__FunctionCall__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__FunctionCall__free;
    this->name = NULL;
    this->arity = 0;
    this->arguments = NULL;
    return this;
}

/**
 * @brief Add argument to function call expression
 * 
 * @param type 
 */
Expression__FunctionCall* Expression__FunctionCall__addArgument(Expression__FunctionCall *this, Expression *argument) {
    this->arity++;
    this->arguments = realloc(this->arguments, this->arity * sizeof(Expression*));
    this->arguments[this->arity - 1] = argument;
    return this;
}

/**
 * @brief Binary operator expression serializer
 * 
 * @param type 
 */
void Expression__BinaryOperator__serialize(Expression__BinaryOperator *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_BINARY_OPERATION\", \"operator\": \"");
    switch (this->operator) {
        case TOKEN_PLUS:
            StringBuilder__appendString(stringBuilder, "+");
            break;
        case TOKEN_MINUS:
            StringBuilder__appendString(stringBuilder, "-");
            break;
        case TOKEN_CONCATENATE:
            StringBuilder__appendString(stringBuilder, ".");
            break;
        case TOKEN_MULTIPLY:
            StringBuilder__appendString(stringBuilder, "*");
            break;
        case TOKEN_DIVIDE:
            StringBuilder__appendString(stringBuilder, "/");
            break;
        case TOKEN_ASSIGN:
            StringBuilder__appendString(stringBuilder, "=");
            break;
        case TOKEN_EQUALS:
            StringBuilder__appendString(stringBuilder, "===");
            break;
        case TOKEN_NOT_EQUALS:
            StringBuilder__appendString(stringBuilder, "!==");
            break;
        case TOKEN_LESS:
            StringBuilder__appendString(stringBuilder, "<");
            break;
        case TOKEN_GREATER:
            StringBuilder__appendString(stringBuilder, ">");
            break;
        case TOKEN_LESS_OR_EQUALS:
            StringBuilder__appendString(stringBuilder, "<=");
            break;
        case TOKEN_GREATER_OR_EQUALS:
            StringBuilder__appendString(stringBuilder, ">=");
            break;
        default:
            StringBuilder__appendString(stringBuilder, "TODO");
    }
    StringBuilder__appendString(stringBuilder, "\", \"lSide\": ");
    if(this->lSide != NULL) {
        this->lSide->super.serialize((Statement*)this->lSide, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"rSide\": ");
    if(this->rSide != NULL) {
        this->rSide->super.serialize((Statement*)this->rSide, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

/**
 * @brief Get binary operator expression children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** Expression__BinaryOperator__getChildren(Expression__BinaryOperator *this, int * childrenCount) {
    *childrenCount = 2;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->lSide;
    children[1] = (Statement**) &this->rSide;
    return children;
}

/**
 * @brief Get binary operator expression type
 * 
 * @param this 
 * @return Type 
 */
UnionType Expression__BinaryOperation__getType(Expression__BinaryOperator *this, Table * functionTable, StatementList * program, Function * currentFunction) {
    UnionType type = {0};
    switch (this->operator) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY: {
            UnionType lType = this->lSide->getType(this->lSide, functionTable, program, currentFunction);
            UnionType rType = this->rSide->getType(this->rSide, functionTable, program, currentFunction);
            if(lType.isInt && rType.isInt) {
                type.isInt = true;
            }
            if(lType.isFloat || rType.isFloat) {
                type.isFloat = true;
            }
            break;
        }
        case TOKEN_CONCATENATE:
            type.isString = true;
            break;
        case TOKEN_DIVIDE:
            type.isFloat = true;
            break;
        case TOKEN_ASSIGN:
            type = this->rSide->getType(this->rSide, functionTable, program, currentFunction);
            break;
        case TOKEN_EQUALS:
        case TOKEN_NOT_EQUALS:
        case TOKEN_LESS:
        case TOKEN_GREATER:
        case TOKEN_LESS_OR_EQUALS:
        case TOKEN_GREATER_OR_EQUALS:
            type.isBool = true;
            break;
        default:
            fprintf(stderr, "Unknown binary operator, unable to generate type\n");
            exit(99);
            break;
    }
    return type;
}

/**
 * @brief Duplicates Expression__BinaryOperator
 * 
 * @param this 
 * @return Expression__BinaryOperator* 
 */
Expression__BinaryOperator* Expression__BinaryOperator__duplicate(Expression__BinaryOperator* this) {
    Expression__BinaryOperator* duplicate = Expression__BinaryOperator__init();
    duplicate->operator = this->operator;
    duplicate->lSide = (this->lSide != NULL ? (Expression*)this->lSide->super.duplicate((Statement*)this->lSide) : NULL);
    duplicate->rSide = (this->rSide != NULL ? (Expression*)this->rSide->super.duplicate((Statement*)this->rSide) : NULL);
    return duplicate;
}

void Expression__BinaryOperator__free(Expression__BinaryOperator* this) {
    if(this == NULL) return;
    this->lSide->super.free((Statement*)this->lSide);
    this->rSide->super.free((Statement*)this->rSide);
    free(this);
}

/**
 * @brief Binary operator expression constructor
 * 
 * @param type 
 * @return Expression__BinaryOperator* 
 */
Expression__BinaryOperator* Expression__BinaryOperator__init() {
    Expression__BinaryOperator *this = malloc(sizeof(Expression__BinaryOperator));
    this->super.expressionType = EXPRESSION_BINARY_OPERATOR;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__BinaryOperator__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__BinaryOperator__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__BinaryOperator__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__BinaryOperator__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *))Expression__BinaryOperation__getType;
    this->lSide = NULL;
    this->rSide = NULL;
    return this;
}

/**
 * @brief <if> statement serializer
 * 
 * @param type 
 */
void StatementIf__serialize(StatementIf *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_IF\", \"condition\": ");
    if(this->condition != NULL) {
        this->condition->super.serialize((Statement*)this->condition, stringBuilder);
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

/**
 * @brief Get <if> statement children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** StatementIf__getChildren(StatementIf *this, int * childrenCount) {
    *childrenCount = 3;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->condition;
    children[1] = (Statement**) &this->ifBody;
    children[2] = (Statement**) &this->elseBody;
    return children;
}

/**
 * @brief Duplicates <if> statement
 * 
 * @param type 
 * @return StatementIf* 
 */
StatementIf* StatementIf__duplicate(StatementIf* this) {
    StatementIf* duplicate = StatementIf__init();
    duplicate->elseBody = (this->elseBody != NULL ? this->elseBody->duplicate(this->elseBody) : NULL);
    duplicate->ifBody = (this->ifBody != NULL ? this->ifBody->duplicate(this->ifBody) : NULL);
    duplicate->condition = (this->condition != NULL ? (Expression*)this->condition->super.duplicate((Statement*)this->condition) : NULL);
    return duplicate;
}

void StatementIf__free(StatementIf* this) {
    if(this == NULL) return;
    this->elseBody->free(this->elseBody);
    this->ifBody->free(this->ifBody);
    this->condition->super.free((Statement*)this->condition);
    free(this);
}

/**
 * @brief <if> statement constructor
 * 
 * @param type 
 * @return StatementIf* 
 */
StatementIf* StatementIf__init() {
    StatementIf *this = malloc(sizeof(StatementIf));
    this->super.statementType = STATEMENT_IF;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementIf__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementIf__getChildren;
    this->super.duplicate = (struct Statement * (*)(struct Statement *))StatementIf__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementIf__free;
    this->condition = NULL;
    this->ifBody = NULL;
    this->elseBody = NULL;
    return this;
}

/**
 * @brief <while> statement serializer
 * 
 * @param type 
 */
void StatementWhile__serialize(StatementWhile *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_WHILE\", \"condition\": ");
    if(this->condition != NULL) {
        this->condition->super.serialize((Statement*)this->condition, stringBuilder);
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

/**
 * @brief Get <while> statement children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** StatementWhile__getChildren(StatementWhile *this, int * childrenCount) {
    *childrenCount = 2;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->condition;
    children[1] = (Statement**) &this->body;
    return children;
}

/**
 * @brief Duplicates <while> statement
 * 
 * @param type 
 * @return StatementWhile* 
 */
StatementWhile* StatementWhile__duplicate(StatementWhile* this) {
    StatementWhile* duplicate = StatementWhile__init();
    duplicate->body = (this->body != NULL ? this->body->duplicate(this->body) : NULL);
    duplicate->condition = (this->condition != NULL ? (Expression*)this->condition->super.duplicate((Statement*)this->condition) : NULL);
    return duplicate;
}

void StatementWhile__free(StatementWhile* this) {
    if(this == NULL) return;
    this->body->free(this->body);
    this->condition->super.free((Statement*)this->condition);
    free(this);
}

/**
 * @brief <while> statement constructor
 * 
 * @param type 
 * @return StatementWhile* 
 */
StatementWhile* StatementWhile__init() {
    StatementWhile *this = malloc(sizeof(StatementWhile));
    this->super.statementType = STATEMENT_WHILE;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementWhile__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementWhile__getChildren;    
    this->super.duplicate = (struct Statement * (*)(struct Statement *))StatementWhile__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementWhile__free;
    this->condition = NULL;
    this->body = NULL;
    return this;
}

/**
 * @brief <return> statement serializer
 * 
 * @param type 
 */
void StatementReturn__serialize(StatementReturn *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_RETURN\", \"expression\": ");
    if(this->expression != NULL) {
        this->expression->super.serialize((Statement*)this->expression, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

/**
 * @brief Get <return> statement children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** StatementReturn__getChildren(StatementReturn *this, int * childrenCount) {
    *childrenCount = 1;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->expression;
    return children;
}

/**
 * @brief Duplicates <return> statement
 * 
 * @param type 
 * @return StatementReturn* 
 */
StatementReturn* StatementReturn__duplicate(StatementReturn* this) {
    StatementReturn* duplicate = StatementReturn__init();
    duplicate->expression = (this->expression != NULL ? (Expression*)this->expression->super.duplicate((Statement*)this->expression) : NULL);
    return duplicate;
}

void StatementReturn__free(StatementReturn* this) {
    if(this == NULL || this->expression == NULL) return;
    this->expression->super.free((Statement*)this->expression);
    free(this);
}

/**
 * @brief <return> statement constructor
 * 
 * @param type 
 * @return StatementReturn* 
 */
StatementReturn* StatementReturn__init() {
    StatementReturn *this = malloc(sizeof(StatementReturn));
    this->super.statementType = STATEMENT_RETURN;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementReturn__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementReturn__getChildren;
    this->super.duplicate = (struct Statement * (*)(struct Statement *))StatementReturn__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementReturn__free;
    this->expression = NULL;
    return this;
}

/**
 * @brief <exit> statement serializer
 * 
 * @param type 
 */
void StatementExit__serialize(StatementExit *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_EXIT\", \"exitCode\": ");
    StringBuilder__appendInt(stringBuilder, this->exitCode);
    StringBuilder__appendString(stringBuilder, "}");
}

/**
 * @brief Get <exit> statement children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** StatementExit__getChildren(StatementExit *this, int * childrenCount) {
    *childrenCount = 0;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    return children;
}

/**
 * @brief Duplicates <exit> statement
 * 
 * @param type 
 * @return StatementExit* 
 */
StatementExit* StatementExit__duplicate(StatementExit* this) {
    StatementExit* duplicate = StatementExit__init();
    duplicate->exitCode = this->exitCode;
    return duplicate;
}

void StatementExit__free(StatementExit* this) {
    free(this);
}

/**
 * @brief <exit> statement constructor
 * 
 * @param type 
 * @return StatementExit* 
 */
StatementExit* StatementExit__init() {
    StatementExit *this = malloc(sizeof(StatementExit));
    this->super.statementType = STATEMENT_EXIT;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementExit__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementExit__getChildren;
    this->super.duplicate =(struct Statement* (*)(struct Statement *))StatementExit__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementExit__free;
    this->exitCode = 0;
    return this;
}

/**
 * @brief Function serializer
 * 
 * @param type 
 */
void Function__serialize(Function *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"functionType\": \"FUNCTION\", \"name\": \"");
    StringBuilder__appendString(stringBuilder, this->name);
    StringBuilder__appendString(stringBuilder, "\", \"arity\": ");
    StringBuilder__appendInt(stringBuilder, this->arity);
    StringBuilder__appendString(stringBuilder, ", \"arguments\": [");
    for(int i = 0; i < this->arity; i++) {
        StringBuilder__appendString(stringBuilder, "{\"name\": \"");
        StringBuilder__appendEscapedStr(stringBuilder, this->parameterNames[i]);
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
            case TYPE_BOOL:
                StringBuilder__appendString(stringBuilder, "bool");
                break;
            case TYPE_NULL:
                StringBuilder__appendString(stringBuilder, "null");
                break;
            case TYPE_UNKNOWN:
                StringBuilder__appendString(stringBuilder, "unknown");
                break;
            case TYPE_VOID:
                StringBuilder__appendString(stringBuilder, "void");
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

/**
 * @brief Get function children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** Function__getChildren(Function *this, int * childrenCount) {
    *childrenCount = 1;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->body;
    return children;
}

/**
 * @brief Function constructor
 * 
 * @param type 
 * @return Function* 
 */
Function* Function__init() {
    Function *this = malloc(sizeof(Function));
    this->super.statementType = STATEMENT_FUNCTION;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))Function__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Function__getChildren;
    this->name = NULL;
    this->returnType.isRequired = false;
    this->returnType.type = TYPE_UNKNOWN;
    this->arity = 0;
    this->parameterTypes = NULL;
    this->parameterNames = NULL;
    this->globalVariables = table_init();
    this->body = NULL;
    return this;
}

/**
 * @brief Add function parameter
 * 
 * @param type 
 */
Function* Function__addParameter(Function *this, Type type, char *name) {
    this->arity++;
    this->parameterTypes = realloc(this->parameterTypes, this->arity * sizeof(Type));
    this->parameterTypes[this->arity - 1] = type;
    this->parameterNames = realloc(this->parameterNames, this->arity * sizeof(char*));
    this->parameterNames[this->arity - 1] = name;
    return this;
}