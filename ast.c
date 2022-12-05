/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file ast.c
 * @author Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)
 * @brief Abstract syntax tree
 * @date 2022-10-26
 */

#include "ast.h"

/**
 * @brief Get all statements in a block
 * 
 * @param statement
 * @param count
 * @return Statement**
 */
Statement *** getAllStatements(Statement * parent, size_t * count) {
    int childrenCount = 0;
    *count = childrenCount;
    if(parent == NULL) return NULL;
    Statement *** children = parent->getChildren(parent, &childrenCount);
    *count = childrenCount;
    if(childrenCount == 0) return NULL;
    for(int i=0; i<childrenCount; i++) {
        size_t subchildrenCount = 0;
        if(*children[i] == NULL) continue;
        Statement *** subchildren = getAllStatements(*children[i], &subchildrenCount);
        if(subchildren == NULL) continue;
        *count += subchildrenCount;
        if(subchildrenCount == 0) continue;
        children = realloc(children, sizeof(Statement**) * (*count));
        memcpy(children + *count - subchildrenCount, subchildren, sizeof(Statement**) * subchildrenCount);
        free(subchildren);
    }
    return children;
}

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
UnionType Expression__Constant__getType(Expression__Constant *this, Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    UnionType type = typeToUnionType(this->type);
    type.constant = (Expression*) this;
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
    this->super.isLValue = false;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__Constant__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__Constant__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__Constant__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__Constant__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *, PointerTable *))Expression__Constant__getType;
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

Table * duplicateVarTypeTable(Table * table) {
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

PointerTable * duplicateTableStatement(PointerTable * table) {
    PointerTable * new_table = table_statement_init();
    for (int i = 0; i < TB_SIZE; i++) {
        PointerTableItem * item = table->tb[i];
        PointerTableItem ** new_item = &new_table->tb[i];
        while (item != NULL) {
            UnionType type = *(UnionType *)item->data;
            UnionType * new_type = malloc(sizeof(UnionType));
            *new_type = type;
            *new_item = malloc(sizeof(PointerTableItem));
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
    if(type1.constant == type2.constant) {
        ret.constant = type1.constant;
    } else if(type1.constant != NULL && type2.constant == NULL) {
        if(!type2.isBool && !type2.isFloat && !type2.isInt && !type2.isNull && !type2.isString && !type2.isUndefined) {
            ret.constant = type1.constant;
        } else {
            ret.constant = NULL;
        }
    } else if(type1.constant == NULL && type2.constant != NULL) {
        if(!type1.isBool && !type1.isFloat && !type1.isInt && !type1.isNull && !type1.isString && !type1.isUndefined) {
            ret.constant = type2.constant;
        } else {
            ret.constant = NULL;
        }
    } else {
        ret.constant = NULL;
    }
    return ret;
}

bool orVariableTables(Table * variableTable, Table * duplTable) {
    bool changed = false;
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
            if(type1->constant != type2->constant) {
                if(type1->constant != NULL) {
                    type1->constant = NULL;
                    changed = true;
                }
                type2->constant = NULL;
            }
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
    return changed;
}

void orResultTables(PointerTable * resultTable, PointerTable * duplResultTable) {
    for(int j=0; j<TB_SIZE; j++) {
        PointerTableItem * itemB = duplResultTable->tb[j];
        while(itemB != NULL) {
            PointerTableItem * itemA = table_statement_find(resultTable, itemB->name);
            if(itemA == NULL) {
                table_statement_insert(resultTable, itemB->name, itemB->data);
            } else {
                UnionType * typeA = (UnionType*)itemA->data;
                UnionType * typeB = (UnionType*)itemB->data;
                *typeA = orUnionType(*typeA, *typeB);
            }
            itemB = itemB->next;
        }
    }
}

void getExpressionVarType(Table * functionTable, Expression * expression, Table * variableTable, UnionType * exprTypeRet, PointerTable * resultTable) {
    switch (expression->expressionType) {
        case EXPRESSION_CONSTANT:
            if(exprTypeRet != NULL) {
                Expression__Constant* constant = (Expression__Constant*)expression;
                *exprTypeRet = typeToUnionType(constant->type);
                exprTypeRet->constant = expression;
            }
            break;
        case EXPRESSION_VARIABLE: {
            Expression__Variable* var = (Expression__Variable*)expression;
            UnionType * type = (UnionType*)table_find(variableTable, var->name)->data;
            if(exprTypeRet != NULL) {
                *exprTypeRet = *type;
                exprTypeRet->constant = expression;
            }
            UnionType * typePtr = malloc(sizeof(UnionType));
            *typePtr = *type;
            table_statement_insert(resultTable, (Statement*)var, typePtr);
            // access to undefined variable causes crash, this means that after first access we can say that variable is defined
            type->isUndefined = false;
            break;
        }
        case EXPRESSION_FUNCTION_CALL: {
            Expression__FunctionCall* func = (Expression__FunctionCall*)expression;
            for(int i = 0; i < func->arity; i++) {
                getExpressionVarType(functionTable, func->arguments[i], variableTable, NULL, resultTable);
            }
            if(exprTypeRet != NULL) {
                Function * function = (Function*)table_find(functionTable, func->name)->data;
                *exprTypeRet = typeToUnionType(function->returnType);
            }
            break;
        }
        case EXPRESSION_BINARY_OPERATOR: {
            Expression__BinaryOperator* binOp = (Expression__BinaryOperator*)expression;
            if(binOp->operator == TOKEN_ASSIGN && binOp->lSide->expressionType == EXPRESSION_VARIABLE) {
                UnionType assignedType;
                getExpressionVarType(functionTable, binOp->rSide, variableTable, &assignedType, resultTable);
                assignedType.isUndefined = false;
                for(int i = 0; i < TB_SIZE; i++) {
                    TableItem * item = variableTable->tb[i];
                    while (item != NULL) {
                        UnionType * type = (UnionType*)item->data;
                        if(type->constant != NULL && type->constant->expressionType == EXPRESSION_VARIABLE && strcmp(((Expression__Variable*)type->constant)->name, ((Expression__Variable*)binOp->lSide)->name) == 0) {
                            type->constant = NULL;
                        }
                        item = item->next;
                    }
                }
                *(UnionType*)table_find(variableTable, ((Expression__Variable*)binOp->lSide)->name)->data = assignedType;
                if(exprTypeRet != NULL) {
                    *exprTypeRet = assignedType;
                }
                UnionType * emptyType = malloc(sizeof(UnionType));
                *emptyType = (UnionType){0};
                table_statement_insert(resultTable, binOp->lSide, emptyType);
                return;
            }
            UnionType lType;
            UnionType rType;
            getExpressionVarType(functionTable, binOp->lSide, variableTable, &lType, resultTable);
            getExpressionVarType(functionTable, binOp->rSide, variableTable, &rType, resultTable);
            if(exprTypeRet == NULL) {
                return;
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
                case TOKEN_NEGATE:
                    exprTypeRet->isBool = true;
                case TOKEN_EQUALS:
                    exprTypeRet->isBool = true;
                case TOKEN_NOT_EQUALS:
                    exprTypeRet->isBool = true;
                case TOKEN_LESS:
                    exprTypeRet->isBool = true;
                case TOKEN_GREATER:
                    exprTypeRet->isBool = true;
                case TOKEN_LESS_OR_EQUALS:
                    exprTypeRet->isBool = true;
                case TOKEN_GREATER_OR_EQUALS:
                    exprTypeRet->isBool = true;
                    break;
                default:
                    break;
            }
            break;
        }
        case EXPRESSION_UNARY_OPERATOR: {
            Expression__UnaryOperator* unOp = (Expression__UnaryOperator*)expression;
            UnionType rType;
            getExpressionVarType(functionTable, unOp->rSide, variableTable, &rType, resultTable);
             if(exprTypeRet == NULL) {
                return;
            }
            *exprTypeRet = (UnionType){0};
            switch (unOp->operator) {
                case TOKEN_NEGATE:
                    exprTypeRet->isBool = true;
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

void getStatementListVarType(Table * functionTable, StatementList * statementList, Table * variableTable, PointerTable * resultTable);

void getStatementVarType(Table * functionTable, Statement * statement, Table * variableTable, PointerTable * resultTable) {
    switch(statement->statementType) {
        case STATEMENT_EXPRESSION:
            getExpressionVarType(functionTable, (Expression*)statement, variableTable, NULL, resultTable);
            break;
        case STATEMENT_RETURN:
            if(((StatementReturn*)statement)->expression != NULL) {
                getExpressionVarType(functionTable, ((StatementReturn*)statement)->expression, variableTable, NULL, resultTable);
            }
            break;
        case STATEMENT_IF: {
            StatementIf* ifStatement = (StatementIf*)statement;
            getExpressionVarType(functionTable, ifStatement->condition, variableTable, NULL, resultTable);
            // duplicate result table
            Table * duplTable = duplicateVarTypeTable(variableTable);
            PointerTable * duplResultTable = duplicateTableStatement(resultTable);
            getStatementVarType(functionTable, ifStatement->ifBody, variableTable, resultTable);
            getStatementVarType(functionTable, ifStatement->elseBody, duplTable, duplResultTable);
            orVariableTables(variableTable, duplTable);
            orResultTables(resultTable, duplResultTable);
            // TODO: free also content
            table_free(duplTable);
            //table_statement_free(duplResultTable);
            break;
        }
        case STATEMENT_WHILE: {
            StatementWhile* whileStatement = (StatementWhile*)statement;
            Table * duplTable = duplicateVarTypeTable(variableTable);
            getExpressionVarType(functionTable, whileStatement->condition, variableTable, NULL, resultTable);
            PointerTable * duplResultTable = duplicateTableStatement(resultTable);
            bool changed = true;
            while(changed) {
                changed = false;
                getStatementVarType(functionTable, whileStatement->body, duplTable, duplResultTable);
                getExpressionVarType(functionTable, whileStatement->condition, duplTable, NULL, duplResultTable);
                changed |= orVariableTables(variableTable, duplTable);
                orResultTables(resultTable, duplResultTable);
            }
            // TODO: free also content
            table_free(duplTable);
            //table_statement_free(duplResultTable);
            break;
        }
        case STATEMENT_FOR: {
            StatementFor* forStatement = (StatementFor*)statement; // initialize for statement
            // get variable type of initialization
            if(forStatement->init != NULL) {
                getExpressionVarType(functionTable, forStatement->init, variableTable, NULL, resultTable);
            }
            // get variable type of condition
            if(forStatement->condition != NULL) {
                getExpressionVarType(functionTable, forStatement->condition, variableTable, NULL, resultTable);
            }
            Table * duplTable = duplicateVarTypeTable(variableTable); // duplicate variable table
            PointerTable * duplResultTable = duplicateTableStatement(resultTable);
            bool changed = true;
            while(changed) {
                changed = false;
                // get variable type of body
                getStatementVarType(functionTable, forStatement->body, duplTable, duplResultTable);
                // get variable type of increment
                if(forStatement->increment != NULL) {
                    getExpressionVarType(functionTable, forStatement->increment, duplTable, NULL, duplResultTable);
                }
                // get variable type of condition
                if(forStatement->condition != NULL) {
                    getExpressionVarType(functionTable, forStatement->condition, duplTable, NULL, duplResultTable);
                }
                changed |= orVariableTables(variableTable, duplTable);
                orResultTables(resultTable, duplResultTable);
            }
            // TODO: free also content
            table_free(duplTable);
            //table_statement_free(duplResultTable);
            break;
        }
        case STATEMENT_BREAK: {
            // TODO
            break;
        }
        case STATEMENT_CONTINUE: {
            // TODO
            break;
        }
        case STATEMENT_LIST:
            getStatementListVarType(functionTable, (StatementList*)statement, variableTable, resultTable);
            break;
        default:
            break;
    }
}

void getStatementListVarType(Table * functionTable, StatementList * statementList, Table * variableTable, PointerTable * resultTable) {
    for(int i=0; i<statementList->listSize; i++) {
        getStatementVarType(functionTable, statementList->statements[i], variableTable, resultTable);
    }
}

void generateResultsTypeForFunction(Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    Table * variableTable = table_init();
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
    getStatementVarType(functionTable, currentFunction->body, variableTable, resultTable);
    table_free(variableTable);
}

void generateResultsTypeForProgram(Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    Table * variableTable = table_init();
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
    getStatementListVarType(functionTable, program, variableTable, resultTable);
    table_free(variableTable);
}

/**
 * @brief Get variable expression type
 * 
 * @param this 
 * @return Type 
 */
UnionType Expression__Variable__getType(Expression__Variable *this, Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    if(currentFunction != NULL) {
        PointerTableItem * tableStatItem = table_statement_find(resultTable, (Statement*)currentFunction);
        PointerTable * tableStat = tableStatItem != NULL ? (PointerTable*)tableStatItem->data : NULL;
        if(tableStat == NULL) {
            tableStat = table_statement_init();
            table_statement_insert(resultTable, (Statement*)currentFunction, tableStat);
            generateResultsTypeForFunction(functionTable, program, currentFunction, tableStat);
        }
        PointerTableItem * typeItem = table_statement_find(tableStat, (Statement*)this);
        if(typeItem == NULL) {
            table_statement_remove(resultTable, (Statement*)currentFunction);
            return Expression__Variable__getType(this, functionTable, program, currentFunction, resultTable);
        }
        UnionType * type = typeItem->data;
        return *type;
    } else {
        PointerTableItem * tableStatItem = table_statement_find(resultTable, (Statement*)program);
        PointerTable * tableStat = tableStatItem != NULL ? (PointerTable*)tableStatItem->data : NULL;
        if(tableStat == NULL) {
            tableStat = table_statement_init();
            table_statement_insert(resultTable, (Statement*)program, tableStat);
            generateResultsTypeForProgram(functionTable, program, currentFunction, tableStat);
        }
        PointerTableItem * typeItem = table_statement_find(tableStat, (Statement*)this);
        if(typeItem == NULL) {
            table_statement_remove(resultTable, (Statement*)program);
            return Expression__Variable__getType(this, functionTable, program, currentFunction, resultTable);
        }
        UnionType * type = typeItem->data;
        return *type;
    }
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
    this->super.isLValue = true;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__Variable__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__Variable__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__Variable__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__Variable__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *, PointerTable *))Expression__Variable__getType;
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
UnionType Expression__FunctionCall__getType(Expression__FunctionCall *this, Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
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
    this->super.isLValue = false;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *,  PointerTable *))Expression__FunctionCall__getType;
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
        case TOKEN_AND:
            StringBuilder__appendString(stringBuilder, "&&");
            break;
        case TOKEN_OR:
            StringBuilder__appendString(stringBuilder, "||");
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
UnionType Expression__BinaryOperation__getType(Expression__BinaryOperator *this, Table * functionTable, StatementList * program, Function * currentFunction,  PointerTable * resultTable) {
    UnionType type = {0};
    switch (this->operator) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY: {
            UnionType lType = this->lSide->getType(this->lSide, functionTable, program, currentFunction, resultTable);
            UnionType rType = this->rSide->getType(this->rSide, functionTable, program, currentFunction, resultTable);
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
            type = this->rSide->getType(this->rSide, functionTable, program, currentFunction, resultTable);
            break;
        case TOKEN_EQUALS:
        case TOKEN_NOT_EQUALS:
        case TOKEN_LESS:
        case TOKEN_GREATER:
        case TOKEN_LESS_OR_EQUALS:
        case TOKEN_GREATER_OR_EQUALS:
        case TOKEN_AND:
        case TOKEN_OR:
        case TOKEN_NEGATE:
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
    this->super.isLValue = false;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__BinaryOperator__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__BinaryOperator__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__BinaryOperator__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__BinaryOperator__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *, PointerTable *))Expression__BinaryOperation__getType;
    this->lSide = NULL;
    this->rSide = NULL;
    return this;
}

/**
 * @brief Unary operator expression serializer
 * 
 * @param type 
 */
void Expression__UnaryOperator__serialize(Expression__UnaryOperator *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"expressionType\": \"EXPRESSION_UNARY_OPERATION\", \"operator\": \"");
    switch (this->operator) {
        case TOKEN_NEGATE:
            StringBuilder__appendString(stringBuilder, "!");
            break;
        default:
            StringBuilder__appendString(stringBuilder, "TODO");
    }
    StringBuilder__appendString(stringBuilder, "\", \"rSide\": ");
    if(this->rSide != NULL) {
        this->rSide->super.serialize((Statement*)this->rSide, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, "}");
}

/**
 * @brief Get unary operator expression children
 * 
 * @param type 
 * @return Statement*** 
 */
Statement *** Expression__UnaryOperator__getChildren(Expression__UnaryOperator *this, int * childrenCount) {
    *childrenCount = 1;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->rSide;
    return children;
}

/**
 * @brief Get unary operator expression type
 * 
 * @param this 
 * @return Type 
 */
UnionType Expression__UnaryOperation__getType(Expression__UnaryOperator *this, Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    UnionType type = {0};
    switch (this->operator) {
        case TOKEN_NEGATE:
            type.isBool = true;
            break;
        default:
            fprintf(stderr, "Unknown unary operator, unable to generate type\n");
            exit(99);
            break;
    }
    return type;
}

/**
 * @brief Duplicates Expression__UnaryOperator
 * 
 * @param this 
 * @return Expression__UnaryOperator* 
 */
Expression__UnaryOperator* Expression__UnaryOperator__duplicate(Expression__UnaryOperator* this) {
    Expression__UnaryOperator* duplicate = Expression__UnaryOperator__init();
    duplicate->operator = this->operator;
    duplicate->rSide = (this->rSide != NULL ? (Expression*)this->rSide->super.duplicate((Statement*)this->rSide) : NULL);
    return duplicate;
}

void Expression__UnaryOperator__free(Expression__UnaryOperator* this) {
    if(this == NULL) return;
    this->rSide->super.free((Statement*)this->rSide);
    free(this);
}

/**
 * @brief Unary operator expression constructor
 * 
 * @param type 
 * @return Expression__UnaryOperator* 
 */
Expression__UnaryOperator* Expression__UnaryOperator__init() {
    Expression__UnaryOperator *this = malloc(sizeof(Expression__UnaryOperator));
    this->super.expressionType = EXPRESSION_UNARY_OPERATOR;
    this->super.isLValue = false;
    this->super.super.statementType = STATEMENT_EXPRESSION;
    this->super.super.serialize = (void (*)(struct Statement *, StringBuilder *))Expression__UnaryOperator__serialize;
    this->super.super.getChildren = (struct Statement *** (*)(struct Statement *, int *))Expression__UnaryOperator__getChildren;
    this->super.super.duplicate = (struct Statement * (*)(struct Statement *))Expression__UnaryOperator__duplicate;
    this->super.super.free = (void (*)(struct Statement *))Expression__UnaryOperator__free;
    this->super.getType = (UnionType (*)(struct Expression *, Table *, StatementList *, Function *, PointerTable *))Expression__UnaryOperation__getType;
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
 * @brief <for> statement serializer
 * 
 * @param this 
 * @param stringBuilder 
 */
void StatementFor__serialize(StatementFor *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_FOR\", \"init\": ");
    if(this->init != NULL) {
        this->init->super.serialize((Statement*)this->init, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"condition\": ");
    if(this->condition != NULL) {
        this->condition->super.serialize((Statement*)this->condition, stringBuilder);
    } else {
        StringBuilder__appendString(stringBuilder, "null");
    }
    StringBuilder__appendString(stringBuilder, ", \"increment\": ");
    if(this->increment != NULL) {
        this->increment->super.serialize((Statement*)this->increment, stringBuilder);
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
 * @brief Get <for> statement children
 * 
 * @param this 
 * @param childrenCount 
 * @return Statement*** 
 */
Statement *** StatementFor__getChildren(StatementFor *this, int * childrenCount) {
    *childrenCount = 4;
    Statement *** children = malloc(*childrenCount * sizeof(Statement**));
    children[0] = (Statement**) &this->init;
    children[1] = (Statement**) &this->condition;
    children[2] = (Statement**) &this->increment;
    children[3] = (Statement**) &this->body;
    return children;
}

/**
 * @brief Duplicates <for> statement
 * 
 * @param this 
 * @return StatementFor* 
 */
StatementFor* StatementFor__duplicate(StatementFor* this) {
    StatementFor* duplicate = StatementFor__init();
    duplicate->body = (this->body != NULL ? this->body->duplicate(this->body) : NULL);
    duplicate->condition = (this->condition != NULL ? (Expression*)this->condition->super.duplicate((Statement*)this->condition) : NULL);
    duplicate->init = (this->init != NULL ? (Expression*)this->init->super.duplicate((Statement*)this->init) : NULL);
    duplicate->increment = (this->increment != NULL ? (Expression*)this->increment->super.duplicate((Statement*)this->increment) : NULL);
    return duplicate;
}

void StatementFor__free(StatementFor* this) {
    if(this == NULL) return;
    if(this->body != NULL){
        this->body->free(this->body);
    }
    if(this->condition != NULL){
        this->condition->super.free((Statement*)this->condition);
    }
    if(this->init != NULL){
        this->init->super.free((Statement*)this->init);
    }
    if(this->increment != NULL){
        this->increment->super.free((Statement*)this->increment);
    }
    free(this);
}

/**
 * @brief <for> statement constructor
 */
StatementFor* StatementFor__init() {
    StatementFor *this = malloc(sizeof(StatementFor));
    this->super.statementType = STATEMENT_FOR;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementFor__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementFor__getChildren;
    this->super.duplicate = (struct Statement * (*)(struct Statement *))StatementFor__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementFor__free;
    this->condition = NULL;
    this->body = NULL;
    this->init = NULL;
    this->increment = NULL;
    return this;
}

/**
 * @brief <continue> statement serializer
 * 
 * @param this 
 * @param stringBuilder 
 */
void StatementContinue__serialize(StatementContinue *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_CONTINUE\", \"depth\": ");
    StringBuilder__appendInt(stringBuilder, this->depth);
    StringBuilder__appendString(stringBuilder, "}");
}

/**
 * @brief Get <continue> statement children
 * 
 * @param this 
 * @param childrenCount 
 * @return Statement*** 
 */
Statement *** StatementContinue__getChildren(StatementContinue *this, int * childrenCount) {
    *childrenCount = 0;
    return NULL;
}

/**
 * @brief Duplicates <continue> statement
 * 
 * @param this 
 * @return StatementContinue* 
 */
StatementContinue* StatementContinue__duplicate(StatementContinue* this) {
    StatementContinue* duplicate = StatementContinue__init();
    duplicate->depth = this->depth;
    return duplicate;
}

void StatementContinue__free(StatementContinue* this) {
    if(this == NULL) return;
    free(this);
}

/**
 * @brief <continue> statement constructor
 */
StatementContinue* StatementContinue__init() {
    StatementContinue *this = malloc(sizeof(StatementContinue));
    this->super.statementType = STATEMENT_CONTINUE;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementContinue__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementContinue__getChildren;
    this->super.duplicate = (struct Statement * (*)(struct Statement *))StatementContinue__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementContinue__free;
    return this;
}

/**
 * @brief <break> statement serializer
 * 
 * @param this 
 * @param stringBuilder 
 */
void StatementBreak__serialize(StatementBreak *this, StringBuilder * stringBuilder) {
    StringBuilder__appendString(stringBuilder, "{\"statementType\": \"STATEMENT_BREAK\", \"depth\": ");
    StringBuilder__appendInt(stringBuilder, this->depth);
    StringBuilder__appendString(stringBuilder, "}");
}

/**
 * @brief Get <break> statement children
 * 
 * @param this 
 * @param childrenCount 
 * @return Statement*** 
 */
Statement *** StatementBreak__getChildren(StatementBreak *this, int * childrenCount) {
    *childrenCount = 0;
    return NULL;
}

/**
 * @brief Duplicates <break> statement
 * 
 * @param this 
 * @return StatementBreak* 
 */
StatementBreak* StatementBreak__duplicate(StatementBreak* this) {
    StatementBreak* duplicate = StatementBreak__init();
    duplicate->depth = this->depth;
    return duplicate;
}

void StatementBreak__free(StatementBreak* this) {
    if(this == NULL) return;
    free(this);
}

/**
 * @brief <break> statement constructor
 */
StatementBreak* StatementBreak__init() {
    StatementBreak *this = malloc(sizeof(StatementBreak));
    this->super.statementType = STATEMENT_BREAK;
    this->super.serialize = (void (*)(struct Statement *, StringBuilder *))StatementBreak__serialize;
    this->super.getChildren = (struct Statement *** (*)(struct Statement *, int *))StatementBreak__getChildren;
    this->super.duplicate = (struct Statement * (*)(struct Statement *))StatementBreak__duplicate;
    this->super.free = (void (*)(struct Statement *))StatementBreak__free;
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
