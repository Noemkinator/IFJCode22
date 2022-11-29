// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include "optimizer.h"

Expression__Constant * performConstantCast(Expression__Constant * in, Type targetType) {
    if(in->type.type == targetType.type) {
        in->type.isRequired = targetType.isRequired;
        return in;
    }
    if(targetType.isRequired == false && in->type.type == TYPE_NULL) {
        return in;
    }
    if(in->type.type != TYPE_INT && in->type.type != TYPE_FLOAT && in->type.type != TYPE_STRING && in->type.type != TYPE_BOOL && in->type.type != TYPE_NULL) {
        return in;
    }
    if(targetType.type != TYPE_INT && targetType.type != TYPE_FLOAT && targetType.type != TYPE_STRING && targetType.type != TYPE_BOOL) {
        return in;
    }
    Expression__Constant * result = Expression__Constant__init();
    result->type = targetType;
    switch(targetType.type) {
        case TYPE_INT:
            switch(in->type.type) {
                case TYPE_FLOAT:
                    result->value.integer = (long long int) in->value.real;
                    break;
                case TYPE_STRING:
                    result->value.integer = atoll(in->value.string);
                    break;
                case TYPE_BOOL:
                    result->value.integer = in->value.boolean;
                    break;
                case TYPE_VOID:
                case TYPE_NULL:
                    result->value.integer = 0;
                    break;
                default:
                    fprintf(stderr, "Bad constant cast");
                    exit(99);
                    break;
            }
            break;
        case TYPE_FLOAT:
            switch(in->type.type) {
                case TYPE_INT:
                    result->value.real = (double) in->value.integer;
                    break;
                case TYPE_STRING:
                    result->value.real = atof(in->value.string);
                    break;
                case TYPE_BOOL:
                    result->value.real = in->value.boolean;
                    break;
                case TYPE_VOID:
                case TYPE_NULL:
                    result->value.real = 0.0;
                    break;
                default:
                    fprintf(stderr, "Bad constant cast");
                    exit(99);
                    break;
            }
            break;
        case TYPE_STRING:
            switch(in->type.type) {
                case TYPE_INT:
                    result->value.string = malloc(128);
                    sprintf(result->value.string, "%lld", in->value.integer);
                    break;
                case TYPE_FLOAT:
                    result->value.string = malloc(128);
                    sprintf(result->value.string, "%g", in->value.real);
                    break;
                case TYPE_BOOL:
                    result->value.string = malloc(2);
                    sprintf(result->value.string, "%s", in->value.boolean ? "1" : "");
                    break;
                case TYPE_VOID:
                case TYPE_NULL:
                    result->value.string = malloc(1);
                    result->value.string[0] = '\0';
                    break;
                default:
                    fprintf(stderr, "Bad constant cast");
                    exit(99);
                    break;
            }
            break;
        case TYPE_BOOL:
            switch(in->type.type) {
                case TYPE_INT:
                    result->value.boolean = in->value.integer != 0;
                    break;
                case TYPE_FLOAT:
                    result->value.boolean = in->value.real != 0.0;
                    break;
                case TYPE_STRING:
                    result->value.boolean = in->value.string[0] != '\0';
                    break;
                case TYPE_VOID:
                case TYPE_NULL:
                    result->value.boolean = false;
                    break;
                default:
                    fprintf(stderr, "Bad constant cast");
                    exit(99);
                    break;
            }
            break;
        default:
            break;
    }
    return result;
}

Expression__Constant * performConstantCastCondition(Expression__Constant * in) {
    if(in->type.type == TYPE_BOOL) {
        in->type.isRequired = true;
        return in;
    }
    if(in->type.type != TYPE_INT && in->type.type != TYPE_FLOAT && in->type.type != TYPE_STRING && in->type.type != TYPE_BOOL && in->type.type != TYPE_NULL) {
        return in;
    }
    Expression__Constant * result = Expression__Constant__init();
    result->type.isRequired = true;
    result->type.type = TYPE_BOOL;
    switch(in->type.type) {
        case TYPE_INT:
            result->value.boolean = in->value.integer != 0;
            break;
        case TYPE_FLOAT:
            result->value.boolean = in->value.real != 0.0;
            break;
        case TYPE_STRING:
            result->value.boolean = in->value.string[0] != '\0' && strcmp(in->value.string, "0") != 0;
            break;
        case TYPE_VOID:
        case TYPE_NULL:
            result->value.boolean = false;
            break;
        default:
            fprintf(stderr, "Bad constant cast");
            exit(99);
            break;
    }
    return result;
}

Expression__Constant * performConstantFolding(Expression__BinaryOperator * in) {
    if(in->lSide->expressionType != EXPRESSION_CONSTANT || in->rSide->expressionType != EXPRESSION_CONSTANT) return NULL;
    Expression__Constant * left = (Expression__Constant *) in->lSide;
    Expression__Constant * right = (Expression__Constant *) in->rSide;
    Expression__Constant * result = Expression__Constant__init();
    result->type.isRequired = true;
    switch(in->operator) {
        case TOKEN_PLUS:
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real + performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                result->value.integer = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true})->value.integer + performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true})->value.integer;
                result->type.type = TYPE_INT;
                return result;
            }
        case TOKEN_MINUS:
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real - performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                result->value.integer = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true})->value.integer - performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true})->value.integer;
                result->type.type = TYPE_INT;
                return result;
            }
        case TOKEN_CONCATENATE: {
            StringBuilder sb;
            StringBuilder__init(&sb);
            StringBuilder__appendString(&sb, performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true})->value.string);
            StringBuilder__appendString(&sb, performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true})->value.string);
            result->value.string = sb.text;
            result->type.type = TYPE_STRING;
            return result;
        }
        case TOKEN_MULTIPLY:
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real * performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                result->value.integer = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true})->value.integer * performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true})->value.integer;
                result->type.type = TYPE_INT;
                return result;
            }
        case TOKEN_DIVIDE:
            if((left->type.type == TYPE_FLOAT || left->type.type == TYPE_INT) && ((right->type.type == TYPE_FLOAT && right->value.real != 0.0) || (right->type.type == TYPE_INT && right->value.integer != 0))) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real / performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                free(result);
                return NULL;
            }
        case TOKEN_EQUALS: {
            bool equals = false;
            if(left->type.type == right->type.type) {
                switch(left->type.type) {
                    case TYPE_INT:
                        equals = left->value.integer == right->value.integer;
                        break;
                    case TYPE_FLOAT:
                        equals = left->value.real == right->value.real;
                        break;
                    case TYPE_STRING:
                        equals = strcmp(left->value.string, right->value.string) == 0;
                        break;
                    case TYPE_BOOL:
                        equals = left->value.boolean == right->value.boolean;
                        break;
                    case TYPE_NULL:
                        equals = true;
                        break;
                    default:
                        fprintf(stderr, "Bad constant cast for ===");
                        exit(99);
                        break;
                }
            }
            result->value.boolean = equals;
            result->type.type = TYPE_BOOL;
            return result;
            break;
        }
        case TOKEN_NOT_EQUALS: {
            bool equals = false;
            if(left->type.type == right->type.type) {
                switch(left->type.type) {
                    case TYPE_INT:
                        equals = left->value.integer == right->value.integer;
                        break;
                    case TYPE_FLOAT:
                        equals = left->value.real == right->value.real;
                        break;
                    case TYPE_STRING:
                        equals = strcmp(left->value.string, right->value.string) == 0;
                        break;
                    case TYPE_BOOL:
                        equals = left->value.boolean == right->value.boolean;
                        break;
                    case TYPE_NULL:
                        equals = true;
                        break;
                    default:
                        fprintf(stderr, "Bad constant cast for !==");
                        exit(99);
                        break;
                }
            }
            result->value.boolean = !equals;
            result->type.type = TYPE_BOOL;
            return result;
            break;
        }
        case TOKEN_LESS: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true});
                result->value.boolean = castL->value.boolean < castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true});
                bool less = false;
                char * s1 = castL->value.string;
                char * s2 = castR->value.string;
                while(*s1 != '\0' || *s2 != '\0') {
                    if(*s1 < *s2) {
                        less = true;
                        break;
                    } else if(*s1 > *s2) {
                        break;
                    }
                    s1++;
                    s2++;
                }
                result->value.boolean = less;
                return result;
            }
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true});
                result->value.boolean = castL->value.real < castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true});
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true});
            result->value.boolean = castL->value.integer < castR->value.integer;
            return result;
            break;
        }
        case TOKEN_GREATER: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true});
                result->value.boolean = castL->value.boolean > castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true});
                bool less = false;
                char * s1 = castL->value.string;
                char * s2 = castR->value.string;
                while(*s1 != '\0' || *s2 != '\0') {
                    if(*s1 > *s2) {
                        less = true;
                        break;
                    } else if(*s1 < *s2) {
                        break;
                    }
                    s1++;
                    s2++;
                }
                result->value.boolean = less;
                return result;
            }
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true});
                result->value.boolean = castL->value.real > castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true});
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true});
            result->value.boolean = castL->value.integer > castR->value.integer;
            return result;
            break;
        }
        case TOKEN_LESS_OR_EQUALS: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true});
                result->value.boolean = castL->value.boolean <= castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true});
                bool less = true;
                char * s1 = castL->value.string;
                char * s2 = castR->value.string;
                while(*s1 != '\0' || *s2 != '\0') {
                    if(!(*s1 <= *s2)) {
                        less = false;
                        break;
                    } else if(*s1 < *s2) {
                        break;
                    }
                    s1++;
                    s2++;
                }
                result->value.boolean = less;
                return result;
            }
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true});
                result->value.boolean = castL->value.real <= castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true});
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true});
            result->value.boolean = castL->value.integer <= castR->value.integer;
            return result;
            break;
        }
        case TOKEN_GREATER_OR_EQUALS: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true});
                result->value.boolean = castL->value.boolean >= castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true});
                bool less = true;
                char * s1 = castL->value.string;
                char * s2 = castR->value.string;
                while(*s1 != '\0' || *s2 != '\0') {
                    if(!(*s1 >= *s2)) {
                        less = false;
                        break;
                    } else if(*s1 > *s2) {
                        break;
                    }
                    s1++;
                    s2++;
                }
                result->value.boolean = less;
                return result;
            }
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true});
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true});
                result->value.boolean = castL->value.real >= castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true});
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true});
            result->value.boolean = castL->value.integer >= castR->value.integer;
            return result;
            break;
        }
        default:
            free(result);
            return NULL;
    }
    return NULL;
}

Statement * performStatementFolding(Statement * in) {
    switch(in->statementType) {
        case STATEMENT_IF: {
            StatementIf* ifStatement = (StatementIf *) in;
            if(ifStatement->condition->expressionType == EXPRESSION_CONSTANT) {
                Expression__Constant * condition = (Expression__Constant *) ifStatement->condition;
                condition = performConstantCastCondition(condition);
                if(condition->value.boolean) {
                    return ifStatement->ifBody;
                } else {
                    return ifStatement->elseBody;
                }
            }
            break;
        }
        case STATEMENT_WHILE: {
            StatementWhile* whileStatement = (StatementWhile *) in;
            if(whileStatement->condition->expressionType == EXPRESSION_CONSTANT) {
                Expression__Constant * condition = (Expression__Constant *) whileStatement->condition;
                condition = performConstantCastCondition(condition);
                if(!condition->value.boolean) {
                    return (Statement*)StatementList__init();
                }
            }
            break;
        }
        default:
            break;
    }
    return NULL;
}

bool removeCodeAfterReturn(StatementList * in) {
    for(int i = 0; i < in->listSize-1; i++) {
        Statement * statement = in->statements[i];
        if(statement->statementType == STATEMENT_RETURN || statement->statementType == STATEMENT_EXIT) {
            in->listSize = i + 1;
            return true;
        }
    }
    return false;
}

int getExpressionError(Expression * expression, Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    if(expression == NULL) {
        return -1;
    }
    switch(expression->expressionType) {
        case EXPRESSION_CONSTANT: 
            return 0;
        case EXPRESSION_VARIABLE: {
            UnionType type =  expression->getType(expression, functionTable, program, currentFunction, resultTable);
            if(!type.isBool && !type.isFloat && !type.isInt && !type.isNull && !type.isString && type.isUndefined) {
                return 5;
            }
            if(!type.isUndefined) {
                return 0;
            } else {
                return -1;
            }
        }
        case EXPRESSION_FUNCTION_CALL: 
            return -1;
        case EXPRESSION_BINARY_OPERATOR: {
            Expression__BinaryOperator * op = (Expression__BinaryOperator *) expression;
            if(op->operator != TOKEN_ASSIGN) {
                int leftError = getExpressionError(op->lSide, functionTable, program, currentFunction, resultTable);
                if(leftError != 0) {
                    return leftError;
                }
            } else {
                if(op->lSide->expressionType != EXPRESSION_VARIABLE) {
                    return 2;
                } else {
                    return getExpressionError(op->rSide, functionTable, program, currentFunction, resultTable);
                }
            }

            int rightError = getExpressionError(op->rSide, functionTable, program, currentFunction, resultTable);
            if(rightError != 0) {
                return rightError;
            }
            return -1;
        }
        case EXPRESSION_UNARY_OPERATOR: {
            Expression__UnaryOperator * op = (Expression__UnaryOperator *) expression;
            return getExpressionError(op->rSide, functionTable, program, currentFunction, resultTable);
        }
    }
    return -1;
}

bool replaceErrorsWithExit(Statement ** statement, Table * functionTable, StatementList * program, Function * currentFunction, PointerTable * resultTable) {
    if((*statement)->statementType == STATEMENT_IF) {
        StatementIf * ifStatement = (StatementIf *) *statement;
        int error = getExpressionError(ifStatement->condition, functionTable, program, currentFunction, resultTable);
        if(error > 0) {
            StatementExit * exitStatement = StatementExit__init();
            exitStatement->exitCode = error;
            *statement = (Statement*)exitStatement;
            return true;
        }
        bool ret = replaceErrorsWithExit(&ifStatement->ifBody, functionTable, program, currentFunction, resultTable);
        ret |= replaceErrorsWithExit(&ifStatement->elseBody, functionTable, program, currentFunction, resultTable);
        return ret;
    } else if((*statement)->statementType == STATEMENT_WHILE) {
        StatementWhile * whileStatement = (StatementWhile *) *statement;
        int error = getExpressionError(whileStatement->condition, functionTable, program, currentFunction, resultTable);
        if(error > 0) {
            StatementExit * exitStatement = StatementExit__init();
            exitStatement->exitCode = error;
            *statement = (Statement*)exitStatement;
            return true;
        }
        return replaceErrorsWithExit(&whileStatement->body, functionTable, program, currentFunction, resultTable);
    } else if((*statement)->statementType == STATEMENT_RETURN) {
        StatementReturn * returnStatement = (StatementReturn *) *statement;
        int error = getExpressionError(returnStatement->expression, functionTable, program, currentFunction, resultTable);
        if(error > 0) {
            StatementExit * exitStatement = StatementExit__init();
            exitStatement->exitCode = error;
            *statement = (Statement*)exitStatement;
            return true;
        }
    } else if((*statement)->statementType == STATEMENT_EXPRESSION) {
        Expression * expression = ((Expression *) *statement);
        int error = getExpressionError(expression, functionTable, program, currentFunction, resultTable);
        if(error > 0) {
            StatementExit * exitStatement = StatementExit__init();
            exitStatement->exitCode = error;
            *statement = (Statement*)exitStatement;
            return true;
        }
    } else if((*statement)->statementType == STATEMENT_LIST) {
        StatementList * list = (StatementList *) *statement;
        bool ret = false;
        for(int i = 0; i < list->listSize; i++) {
            ret |= replaceErrorsWithExit(&list->statements[i], functionTable, program, currentFunction, resultTable);
        }
        return ret;
    } else if((*statement)->statementType == STATEMENT_FUNCTION) {
        Function * function = (Function *) *statement;
        if(function->body != NULL) {
            return replaceErrorsWithExit(&function->body, functionTable, program, currentFunction, resultTable);
        }
    }
    return false;
}

void unrollWhile(Statement ** statement, int iterationCount) {
    if(iterationCount <= 0) return;
    if((*statement)->statementType != STATEMENT_WHILE) return;
    StatementWhile * whileStatement = (StatementWhile *) *statement;
    StatementIf * generatedCondition = StatementIf__init();
    generatedCondition->condition = (Expression*) whileStatement->condition->super.duplicate(&whileStatement->condition->super);
    StatementList * generatedBody = StatementList__init();
    generatedCondition->ifBody = (Statement*)generatedBody;
    generatedCondition->elseBody = (Statement*)StatementList__init();
    StatementList__addStatement(generatedBody, whileStatement->body);
    StatementList__addStatement(generatedBody, (Statement*)whileStatement->super.duplicate(&whileStatement->super));
    unrollWhile(&generatedBody->statements[generatedBody->listSize-1], iterationCount-1);
    *statement = (Statement*)generatedCondition;
}

typedef struct {
    int assigments;
    int uses;
    Expression__BinaryOperator ** latestUnaccessedAssignment;
} OptimizerVarInfo;

bool optimizeStatement(Statement ** statement, Table * functionTable, StatementList * program, Function * currentFunction, Table * optimizerVarInfo, PointerTable * resultTable) {
    if(statement == NULL) return false;
    if(*statement == NULL) return false;
    Statement * foldedStatement = performStatementFolding(*statement);
    if(foldedStatement != NULL) {
        *statement = foldedStatement;
        return true;
    }
    if((*statement)->statementType == STATEMENT_LIST) {
        if(removeCodeAfterReturn((StatementList *) *statement)) return true;
    }
    if((*statement)->statementType == STATEMENT_EXPRESSION) {
        Expression * expression = (Expression *) *statement;
        if(expression->expressionType == EXPRESSION_BINARY_OPERATOR) {
            Expression__BinaryOperator * op = (Expression__BinaryOperator *) expression;
            Expression__Constant * constant = performConstantFolding(op);
            if(constant != NULL) {
                *statement = (Statement *) constant;
                return true;
            }
            if(op->operator == TOKEN_ASSIGN && op->lSide->expressionType == EXPRESSION_VARIABLE) {
                OptimizerVarInfo * info = table_find(optimizerVarInfo, ((Expression__Variable*)op->lSide)->name)->data;
                if(info->assigments == info->uses) {
                    *statement = (Statement*)op->rSide;
                    return true;
                }
            }
        } else if(expression->expressionType == EXPRESSION_VARIABLE) {
            UnionType type = expression->getType(expression, functionTable, program, currentFunction, resultTable);
            if(type.constant != NULL) {
                // TODO free
                *statement = (Statement *) type.constant;
                return true;
            }
        }
    }
    return false;
}

void buildStatementVarUsages(Statement * statement, Table * optimizerVarInfo) {
    if(statement->statementType == STATEMENT_EXPRESSION) {
        Expression * expression = (Expression *) statement;
        if(expression->expressionType == EXPRESSION_BINARY_OPERATOR) {
            Expression__BinaryOperator * op = (Expression__BinaryOperator *) expression;
            if(op->operator == TOKEN_ASSIGN && op->lSide->expressionType == EXPRESSION_VARIABLE) {
                Expression__Variable * variable = (Expression__Variable *) op->lSide;
                TableItem * optInfo = table_find(optimizerVarInfo, variable->name);
                OptimizerVarInfo * info = NULL;
                if(optInfo == NULL) {
                    info = (OptimizerVarInfo *) malloc(sizeof(OptimizerVarInfo));
                    info->assigments = 0;
                    info->uses = 0;
                    table_insert(optimizerVarInfo, variable->name, info);
                } else {
                    info = (OptimizerVarInfo *) optInfo->data;
                }
                info->assigments++;
            }
        } else if(expression->expressionType == EXPRESSION_VARIABLE) {
            TableItem * optInfo = table_find(optimizerVarInfo, ((Expression__Variable*)expression)->name);
            OptimizerVarInfo * info = NULL;
            if(optInfo == NULL) {
                info = (OptimizerVarInfo *) malloc(sizeof(OptimizerVarInfo));
                info->assigments = 0;
                info->uses = 0;
                table_insert(optimizerVarInfo, ((Expression__Variable*)expression)->name, info);
            } else {
                info = (OptimizerVarInfo *) optInfo->data;
            }
            info->uses++;
        }
    }
}

void buildNestedStatementVarUsages(Statement * parent, Table * optimizerVarInfo) {
    if(parent == NULL) return;
    buildStatementVarUsages(parent, optimizerVarInfo);
    int childrenCount = 0;
    Statement *** children = parent->getChildren(parent, &childrenCount);
    if(childrenCount == 0) return;
    for(int i=0; i<childrenCount; i++) {
        if(children[i] != NULL) buildNestedStatementVarUsages(*children[i], optimizerVarInfo);
    }
    free(children);
}

bool optimizeNestedStatements(Statement ** parent, Table * functionTable, StatementList * program, Function * currentFunction, Table * optimizerVarInfo, PointerTable * resultTable) {
    if(parent == NULL || *parent == NULL) return false;
    bool optimized = false;
    optimized |= optimizeStatement(parent, functionTable, program, currentFunction, optimizerVarInfo, resultTable);
    int childrenCount = 0;
    Statement *** children = (*parent)->getChildren(*parent, &childrenCount);
    if(childrenCount == 0) return optimized;
    for(int i=0; i<childrenCount; i++) {
        optimized |= optimizeNestedStatements(children[i], functionTable, program, currentFunction, optimizerVarInfo, resultTable);
    }
    free(children);
    return optimized;
}

bool expandStatement(Statement ** statement, Table * functionTable, StatementList * program, Function * currentFunction) {
    if(statement == NULL) return false;
    if(*statement == NULL) return false;
    if((*statement)->statementType == STATEMENT_WHILE) {
        unrollWhile(statement, 3);
        return true;
    }
    return false;
}

bool performNestedStatementsExpansion(Statement ** parent, Table * functionTable, StatementList * program, Function * currentFunction) {
    if(parent == NULL || *parent == NULL) return false;
    bool optimized = false;
    int childrenCount = 0;
    Statement *** children = (*parent)->getChildren(*parent, &childrenCount);
    if(childrenCount == 0) return optimized;
    for(int i=0; i<childrenCount; i++) {
        optimized |= performNestedStatementsExpansion(children[i], functionTable, program, currentFunction);
    }
    free(children);
    optimized |= expandStatement(parent, functionTable, program, currentFunction);
    return optimized;
}

void optimize(StatementList * program, Table * functionTable) {
    performNestedStatementsExpansion((Statement**)&program, functionTable, program, NULL);
    bool continueOptimizing = true;
    bool continueUpdatingTypes = true;
    while(continueUpdatingTypes) {
        PointerTable * resultTable = table_statement_init();
        continueUpdatingTypes = false;
        while(continueOptimizing) {
            continueOptimizing = false;
            Table * optimizerVarInfo = table_init();
            buildNestedStatementVarUsages((Statement*)program, optimizerVarInfo);
            continueOptimizing |= optimizeNestedStatements((Statement**)&program, functionTable, program, NULL, optimizerVarInfo, resultTable);
            table_free(optimizerVarInfo);
            continueOptimizing |= replaceErrorsWithExit((Statement**)&program, functionTable, program, NULL, resultTable);
            for(int i = 0; i < TB_SIZE; i++) {
                TableItem* item = functionTable->tb[i];
                while(item != NULL) {
                    optimizerVarInfo = table_init();
                    buildNestedStatementVarUsages((Statement*)item->data, optimizerVarInfo);
                    continueOptimizing |= optimizeNestedStatements((Statement**)&item->data, functionTable, program, (Function*)item->data, optimizerVarInfo, resultTable);
                    table_free(optimizerVarInfo);
                    continueOptimizing |= replaceErrorsWithExit((Statement**)&item->data, functionTable, program, (Function*)item->data, resultTable);
                    item = item->next;
                }
            }
            if(continueOptimizing) continueUpdatingTypes = true;
        }
        table_statement_free(resultTable);
        continueOptimizing = true;
    }
}