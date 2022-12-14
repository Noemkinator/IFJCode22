/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file optimizer.c
 * @author Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)
 * @brief Optimizer of IFJcode22
 * @date 2022-10-26
 */

#include "optimizer.h"
#include <time.h>

Expression__Constant * performConstantCast(Expression__Constant * in, Type targetType, bool isBuiltin) {
    if(in->type.type == targetType.type) {
        in->type.isRequired = targetType.isRequired;
        return in;
    }
    if(targetType.isRequired == false && in->type.type == TYPE_NULL) {
        return in;
    }
    if(in->type.type != TYPE_INT && in->type.type != TYPE_FLOAT && in->type.type != TYPE_STRING && in->type.type != TYPE_BOOL && in->type.type != TYPE_NULL) {
        fprintf(stderr, "Bad type of constant for cast\n");
        return NULL;
    }
    if(targetType.type != TYPE_INT && targetType.type != TYPE_FLOAT && targetType.type != TYPE_STRING && targetType.type != TYPE_BOOL) {
        fprintf(stderr, "Bad target type for cast\n");
        return NULL;
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
                    if(!isBuiltin) {
                        if(strlen(in->value.string) == 0) {
                            // Change this
                            fprintf(stderr, "Invalid string operand type");
                            exit(7);
                        } else {
                            for(size_t i = 0; i < strlen(in->value.string); ++i) {
                                if(in->value.string[i] == (int)' ') continue;
                                if(isdigit(in->value.string[i]) == 0) {
                                    // Change this
                                    fprintf(stderr, "Invalid string operand type");
                                    exit(7);
                                } else break;
                            }
                        }
                    }
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
                    if(!isBuiltin) {
                        if(strlen(in->value.string) == 0) {
                            // Change this
                            fprintf(stderr, "Invalid string operand type");
                            exit(7);
                        } else {
                            for(size_t i = 0; i < strlen(in->value.string); ++i) {
                                if(in->value.string[i] == (int)' ') continue;
                                if(isdigit(in->value.string[i]) == 0) {
                                    // Change this
                                    fprintf(stderr, "Invalid string operand type");
                                    exit(7);
                                } else break;
                            }
                        }
                    }
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
        fprintf(stderr, "Bad constant cast for condition\n");
        return NULL;
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

Expression__Constant * performConstantCastWriteArg(Expression__Constant * in) {
    if(in->type.type == TYPE_STRING) {
        in->type.isRequired = true;
        return in;
    }
    if(in->type.type != TYPE_INT && in->type.type != TYPE_FLOAT && in->type.type != TYPE_STRING && in->type.type != TYPE_BOOL && in->type.type != TYPE_NULL) {
        fprintf(stderr, "Bad constant cast for condition\n");
        return NULL;
    }
    Expression__Constant * result = Expression__Constant__init();
    result->type.isRequired = true;
    result->type.type = TYPE_STRING;
    switch(in->type.type) {
        case TYPE_INT:
            result->value.string = malloc(128);
            sprintf(result->value.string, "%lld", in->value.integer);
            break;
        case TYPE_FLOAT:
            result->value.string = malloc(128);
            sprintf(result->value.string, "%a", in->value.real);
            break;
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
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real + performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                result->value.integer = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false)->value.integer + performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false)->value.integer;
                result->type.type = TYPE_INT;
                return result;
            }
        case TOKEN_MINUS:
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real - performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                result->value.integer = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false)->value.integer - performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false)->value.integer;
                result->type.type = TYPE_INT;
                return result;
            }
        case TOKEN_CONCATENATE: {
            StringBuilder sb;
            StringBuilder__init(&sb);
            StringBuilder__appendString(&sb, performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true}, false)->value.string);
            StringBuilder__appendString(&sb, performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true}, false)->value.string);
            result->value.string = sb.text;
            result->type.type = TYPE_STRING;
            return result;
        }
        case TOKEN_MULTIPLY:
            if(left->type.type == TYPE_FLOAT || right->type.type == TYPE_FLOAT) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real * performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real;
                result->type.type = TYPE_FLOAT;
                return result;
            } else {
                result->value.integer = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false)->value.integer * performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false)->value.integer;
                result->type.type = TYPE_INT;
                return result;
            }
        case TOKEN_DIVIDE:
            if((left->type.type == TYPE_FLOAT || left->type.type == TYPE_INT) && ((right->type.type == TYPE_FLOAT && right->value.real != 0.0) || (right->type.type == TYPE_INT && right->value.integer != 0))) {
                result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real / performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false)->value.real;
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
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                result->value.boolean = castL->value.boolean < castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true}, false);
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
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                result->value.boolean = castL->value.real < castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false);
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false);
            result->value.boolean = castL->value.integer < castR->value.integer;
            return result;
            break;
        }
        case TOKEN_GREATER: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                result->value.boolean = castL->value.boolean > castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true}, false);
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
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                result->value.boolean = castL->value.real > castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false);
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false);
            result->value.boolean = castL->value.integer > castR->value.integer;
            return result;
            break;
        }
        case TOKEN_LESS_OR_EQUALS: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                result->value.boolean = castL->value.boolean <= castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true}, false);
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
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                result->value.boolean = castL->value.real <= castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false);
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false);
            result->value.boolean = castL->value.integer <= castR->value.integer;
            return result;
            break;
        }
        case TOKEN_GREATER_OR_EQUALS: {
            result->type.type = TYPE_BOOL;
            if(left->type.type == TYPE_NULL || right->type.type == TYPE_NULL) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
                result->value.boolean = castL->value.boolean >= castR->value.boolean;
                return result;
            }
            if(left->type.type == TYPE_STRING || right->type.type == TYPE_STRING) {
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true}, false);
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
                Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true}, false);
                result->value.boolean = castL->value.real >= castR->value.real;
                return result;
            }
            Expression__Constant * castL = performConstantCast(left, (Type){.type = TYPE_INT, .isRequired = true}, false);
            Expression__Constant * castR = performConstantCast(right, (Type){.type = TYPE_INT, .isRequired = true}, false);
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
        /*case STATEMENT_WHILE: {
            StatementWhile* whileStatement = (StatementWhile *) in;
            if(whileStatement->condition->expressionType == EXPRESSION_CONSTANT) {
                Expression__Constant * condition = (Expression__Constant *) whileStatement->condition;
                condition = performConstantCastCondition(condition);
                if(!condition->value.boolean) {
                    return (Statement*)StatementList__init();
                }
            }
            break;
        }*/ // TODO: doesnt work with break, continue, but loop unrolling gets the job done anyways
        default:
            break;
    }
    return NULL;
}

Expression__Constant * performBuiltinFolding(Expression__FunctionCall * in, Function * function) {
    if(in->arity != function->arity) return NULL;
    if(function->body != NULL) return NULL;
    if(strcmp(in->name, "floatval") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * constant = (Expression__Constant *) arg;
            return performConstantCast(constant, (Type){.type = TYPE_FLOAT, .isRequired = true}, true);
        }
    } else if(strcmp(in->name, "intval") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * constant = (Expression__Constant *) arg;
            return performConstantCast(constant, (Type){.type = TYPE_INT, .isRequired = true}, true);
        }
    } else if(strcmp(in->name, "strval") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * constant = (Expression__Constant *) arg;
            return performConstantCast(constant, (Type){.type = TYPE_STRING, .isRequired = true}, false);
        }
    } else if(strcmp(in->name, "boolval") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * constant = (Expression__Constant *) arg;
            return performConstantCast(constant, (Type){.type = TYPE_BOOL, .isRequired = true}, false);
        }
    } else if(strcmp(in->name, "strlen") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * argC = (Expression__Constant *) arg;
            if(argC->type.type == TYPE_STRING) {
                Expression__Constant * result = Expression__Constant__init();
                result->type.type = TYPE_INT;
                result->type.isRequired = true;
                result->value.integer = strlen(argC->value.string);
                return result;
            }
        }
    } else if(strcmp(in->name, "substring") == 0) {
        Expression * str = in->arguments[0];
        Expression * start = in->arguments[1];
        Expression * end = in->arguments[2];
        if(start->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * startC = (Expression__Constant *) start;
            if(startC->type.type == TYPE_INT) {
                if(startC->value.integer < 0) {
                    Expression__Constant * result = Expression__Constant__init();
                    result->type.type = TYPE_NULL;
                    result->type.isRequired = false;
                    return result;
                }
            }
        }
        if(end->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * endC = (Expression__Constant *) end;
            if(endC->type.type == TYPE_INT) {
                if(endC->value.integer < 0) {
                    Expression__Constant * result = Expression__Constant__init();
                    result->type.type = TYPE_NULL;
                    result->type.isRequired = false;
                    return result;
                }
            }
        }
        if(start->expressionType == EXPRESSION_CONSTANT && end->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * startC = (Expression__Constant *) start;
            Expression__Constant * endC = (Expression__Constant *) end;
            if(startC->type.type == TYPE_INT && endC->type.type == TYPE_INT) {
                if(startC->value.integer > endC->value.integer) {
                    Expression__Constant * result = Expression__Constant__init();
                    result->type.type = TYPE_NULL;
                    result->type.isRequired = false;
                    return result;
                }
            }
        }
        if(str->expressionType == EXPRESSION_CONSTANT && start->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * strC = (Expression__Constant *) str;
            Expression__Constant * startC = (Expression__Constant *) start;
            if(strC->type.type == TYPE_STRING && startC->type.type == TYPE_INT) {
                if((size_t)startC->value.integer >= strlen(strC->value.string)) {
                    Expression__Constant * result = Expression__Constant__init();
                    result->type.type = TYPE_NULL;
                    result->type.isRequired = false;
                    return result;
                }
            }
        }
        if(str->expressionType == EXPRESSION_CONSTANT && end->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * strC = (Expression__Constant *) str;
            Expression__Constant * endC = (Expression__Constant *) end;
            if(strC->type.type == TYPE_STRING && endC->type.type == TYPE_INT) {
                if((size_t)endC->value.integer > strlen(strC->value.string)) {
                    Expression__Constant * result = Expression__Constant__init();
                    result->type.type = TYPE_NULL;
                    result->type.isRequired = false;
                    return result;
                }
            }
        }
        if(str->expressionType == EXPRESSION_CONSTANT && start->expressionType == EXPRESSION_CONSTANT && end->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * strC = (Expression__Constant *) str;
            Expression__Constant * startC = (Expression__Constant *) start;
            Expression__Constant * endC = (Expression__Constant *) end;
            if(strC->type.type == TYPE_STRING && startC->type.type == TYPE_INT && endC->type.type == TYPE_INT) {
                Expression__Constant * result = Expression__Constant__init();
                result->type.type = TYPE_STRING;
                result->type.isRequired = true;
                result->value.string = malloc(endC->value.integer - startC->value.integer + 1);
                memcpy(result->value.string, strC->value.string + startC->value.integer, endC->value.integer - startC->value.integer);
                result->value.string[endC->value.integer - startC->value.integer] = '\0';
                return result;
            }
        }
    } else if(strcmp(in->name, "ord") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * argC = (Expression__Constant *) arg;
            if(argC->type.type == TYPE_STRING) {
                Expression__Constant * result = Expression__Constant__init();
                result->type.type = TYPE_INT;
                result->type.isRequired = true;
                result->value.integer = (unsigned int)(unsigned char)argC->value.string[0];
                return result;
            }
        }
    } else if(strcmp(in->name, "chr") == 0) {
        Expression * arg = in->arguments[0];
        if(arg->expressionType == EXPRESSION_CONSTANT) {
            Expression__Constant * argC = (Expression__Constant *) arg;
            if(argC->type.type == TYPE_INT && argC->value.integer > 0 && argC->value.integer < 256) {
                Expression__Constant * result = Expression__Constant__init();
                result->type.type = TYPE_STRING;
                result->type.isRequired = true;
                result->value.string = (char *) malloc(2);
                result->value.string[0] = (char) argC->value.integer;
                result->value.string[1] = '\0';
                return result;
            }
        }
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
        case EXPRESSION_PREFIX_OPERATOR: {
            Expression__PrefixOperator * op = (Expression__PrefixOperator *) expression;
            return getExpressionError(op->rSide, functionTable, program, currentFunction, resultTable);
        }
        case EXPRESSION_POSTFIX_OPERATOR: {
            Expression__PostfixOperator * op = (Expression__PostfixOperator *) expression;
            return getExpressionError(op->operand, functionTable, program, currentFunction, resultTable);
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

void clearTable(Table * table) {
    for(int i=0; i<TB_SIZE; i++) {
        TableItem * item = table->tb[i];
        while(item != NULL) {
            TableItem * next = item->next;
            free(item);
            item = next;
        }
        table->tb[i] = NULL;
    }
}

bool optimizeStatement(Statement ** statement, Table * functionTable, StatementList * program, Function * currentFunction, Table * optimizerVarInfo, PointerTable * resultTable) {
    if(statement == NULL) return false;
    if(*statement == NULL) return false;
    Statement * foldedStatement = performStatementFolding(*statement);
    if(foldedStatement != NULL) {
        *statement = foldedStatement;
        return true;
    }
    if((*statement)->statementType == STATEMENT_LIST) {
        // code for unwrapping statement lists, so there arent many nested lists after some optimizations
        if(removeCodeAfterReturn((StatementList *) *statement)) return true;
        for(int i=0; i<((StatementList *) *statement)->listSize; i++) {
            Statement * statementItem = ((StatementList *) *statement)->statements[i];
            if(statementItem->statementType == STATEMENT_LIST) {
                // inject the list into the parent list
                StatementList * parentList = (StatementList *) *statement;
                StatementList * list = (StatementList *) statementItem;
                int additionalListSize = list->listSize;
                int oldListSize = parentList->listSize;
                parentList->listSize += additionalListSize - 1;
                parentList->statements = realloc(parentList->statements, sizeof(Statement*) * (parentList->listSize + 1));
                if(additionalListSize - 1 >= 0) {
                    for(int j = oldListSize-1; j > i; j--) {
                        parentList->statements[j+additionalListSize-1] = parentList->statements[j];
                    }
                    for(int j = 0; j < additionalListSize; j++) {
                        parentList->statements[i+j] = list->statements[j];
                    }
                } else {
                    for(int j = i; j < oldListSize-1; j++) {
                        parentList->statements[j] = parentList->statements[j+1];
                    }
                }
                return true;
            }
        }
        // code for removing useless assignments, actually speedups optimization by around 13%
        int destIndex = 0;
        bool ret = false;
        bool isTableEmpty = true;
        Table * assignments = table_init();
        for(int i=0; i<((StatementList *) *statement)->listSize; i++) {
            Statement * statementItem = ((StatementList *) *statement)->statements[i];
            if(statementItem->statementType == STATEMENT_EXPRESSION && ((Expression*)statementItem)->expressionType == EXPRESSION_BINARY_OPERATOR) {
                Expression__BinaryOperator * binaryOperator = (Expression__BinaryOperator *) statementItem;
                if(binaryOperator->operator == TOKEN_ASSIGN && binaryOperator->lSide->expressionType == EXPRESSION_VARIABLE && binaryOperator->rSide->expressionType == EXPRESSION_CONSTANT) {
                    Expression__Variable * variable = (Expression__Variable *) binaryOperator->lSide;
                    TableItem * item = table_find(assignments, variable->name);
                    if(item != NULL) {
                        Expression__BinaryOperator * previousAssignment = (Expression__BinaryOperator *) item->data;
                        previousAssignment->rSide = binaryOperator->rSide;
                        ret = true;
                        continue;
                    } else {
                        table_insert(assignments, variable->name, binaryOperator);
                        destIndex++;
                    }
                    isTableEmpty = false;
                } else {
                    if(!isTableEmpty) clearTable(assignments);
                    isTableEmpty = true;
                    destIndex++;
                }
            } else if(statementItem->statementType == STATEMENT_EXPRESSION && ((Expression*)statementItem)->expressionType == EXPRESSION_FUNCTION_CALL) {
                // ignore write calls
                Expression__FunctionCall * functionCall = (Expression__FunctionCall *) statementItem;
                if(strcmp(functionCall->name, "write") == 0 && functionCall->arity == 1 && functionCall->arguments[0]->expressionType == EXPRESSION_CONSTANT) {
                    // is ok, can be ignored
                } else {
                    if(!isTableEmpty) clearTable(assignments);
                    isTableEmpty = true;
                }
                destIndex++;
            } else {
                if(!isTableEmpty) clearTable(assignments);
                isTableEmpty = true;
                destIndex++;
            }
            ((StatementList *) *statement)->statements[destIndex-1] = statementItem;
        }
        ((StatementList *) *statement)->listSize = destIndex;
        table_free(assignments);
        if(ret) return true;
        // code for merging multiple writes into one
        bool isPrevStatementWrite = false;
        Expression__FunctionCall * prevFunctionCall = NULL;
        bool mergedWrites = false;
        for(int i=0; i<((StatementList *) *statement)->listSize; i++) {
            Statement * statementItem = ((StatementList *) *statement)->statements[i];
            if(statementItem->statementType == STATEMENT_EXPRESSION && ((Expression*)statementItem)->expressionType == EXPRESSION_FUNCTION_CALL) {
                Expression__FunctionCall * funcCall = (Expression__FunctionCall *) statementItem;
                if(strcmp(funcCall->name, "write") == 0) {
                    if(isPrevStatementWrite) {
                        int copied = 0;
                        for(int j=0; j<funcCall->arity; j++) {
                            Expression * arg = funcCall->arguments[j];
                            // we need to copy only stuff that doesnt causes side effects, otherwise oof
                            if(arg->expressionType == EXPRESSION_CONSTANT) {
                                Expression__FunctionCall__addArgument(prevFunctionCall, arg);
                                copied++;
                            } else {
                                break;
                            }
                        }
                        for(int j=copied; j<funcCall->arity; j++) {
                            funcCall->arguments[j-copied] = funcCall->arguments[j];
                        }
                        funcCall->arity -= copied;
                        mergedWrites |= copied > 0 || funcCall->arity == 0;
                        if(copied == funcCall->arity) {
                            for(int j=i; j<((StatementList *) *statement)->listSize-1; j++) {
                                ((StatementList *) *statement)->statements[j] = ((StatementList *) *statement)->statements[j+1];
                            }
                            ((StatementList *) *statement)->listSize--;
                            i--;
                        } else {
                            prevFunctionCall = funcCall;
                        }
                    } else {
                        prevFunctionCall = funcCall;
                    }
                    isPrevStatementWrite = true;
                } else {
                    isPrevStatementWrite = false;
                    prevFunctionCall = NULL;
                }
            } else {
                isPrevStatementWrite = false;
                prevFunctionCall = NULL;
            }
        }
        if(mergedWrites) return true;
        // code for removing useless statements such as empty lists or constants
        bool containsUselessStatements = false;
        for(int i=0; i<((StatementList *) *statement)->listSize; i++) {
            Statement * statementItem = ((StatementList *) *statement)->statements[i];
            if(statementItem->statementType == STATEMENT_EXPRESSION) {
                Expression * expression = (Expression *) statementItem;
                if(expression->expressionType == EXPRESSION_CONSTANT) {
                    containsUselessStatements = true;
                    break;
                }
            } else if(statementItem->statementType == STATEMENT_LIST && ((StatementList *) statementItem)->listSize == 0) {
                containsUselessStatements = true;
                break;
            }
        }
        if(containsUselessStatements) {
            int newIndex = 0;
            for(int i=0; i<((StatementList *) *statement)->listSize; i++) {
                Statement * statementItem = ((StatementList *) *statement)->statements[i];
                if(statementItem->statementType == STATEMENT_EXPRESSION) {
                    Expression * expression = (Expression *) statementItem;
                    if(expression->expressionType == EXPRESSION_CONSTANT) {
                        //statementItem->free(statementItem);
                        continue;
                    }
                } else if(statementItem->statementType == STATEMENT_LIST && ((StatementList *) statementItem)->listSize == 0) {
                    //statementItem->free(statementItem);
                    continue;
                }
                ((StatementList *) *statement)->statements[newIndex] = statementItem;
                newIndex++;
            }
            ((StatementList *) *statement)->listSize = newIndex;
        }
        return containsUselessStatements;
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
                Statement * constant = type.constant->super.duplicate((Statement *) type.constant);
                // (*statement)->free(*statement); // TODO: doesnt work yes
                *statement = constant;
                return true;
            }
        } else if(expression->expressionType == EXPRESSION_FUNCTION_CALL) {
            Expression__FunctionCall * call = (Expression__FunctionCall *) expression;
            Function * function = table_find(functionTable, call->name)->data;
            Expression__Constant * constant = performBuiltinFolding(call, function);
            if(constant != NULL) {
                *statement = (Statement *) constant;
                return true;
            }
            if(strcmp(call->name, "write") == 0) {
                bool merged = false;
                Expression__Constant * previousConstant = NULL;
                for(int i=0; i<call->arity; i++) {
                    if(call->arguments[i]->expressionType == EXPRESSION_CONSTANT) {
                        Expression__Constant * constant = performConstantCastWriteArg((Expression__Constant *) call->arguments[i]);
                        call->arguments[i] = (Expression*) constant;
                        if(previousConstant != NULL) {
                            char * newString = malloc(strlen(previousConstant->value.string) + strlen(constant->value.string) + 1);
                            strcpy(newString, previousConstant->value.string);
                            strcat(newString, constant->value.string);
                            previousConstant->value.string = newString;
                            for(int j=i; j<call->arity-1; j++) {
                                call->arguments[j] = call->arguments[j+1];
                            }
                            call->arity--;
                            i--;
                            merged = true;
                        } else {
                            previousConstant = constant;
                        }
                    } else {
                        previousConstant = NULL;
                    }
                }
                return merged;
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
        unrollWhile(statement, 1);
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
    bool canLoopsBeOptimized = true;
    size_t count = 0;
    Statement *** statements = getAllStatements((Statement*)program, &count);
    for(int i=0; i<count; i++) {
        if(statements[i] == NULL) continue;
        if(*statements[i] == NULL) continue;
        Statement * statement = *statements[i];
        if(statement->statementType == STATEMENT_BREAK || statement->statementType == STATEMENT_CONTINUE) {
            canLoopsBeOptimized = false;
            break;
        }
    }
    free(statements);
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem* item = functionTable->tb[i];
        while(item != NULL) {
            Function * function = (Function *) item->data;
            if(function->body == NULL) {
                item = item->next;
                continue;
            }
            size_t count = 0;
            Statement *** statements = getAllStatements(function->body, &count);
            for(int i=0; i<count; i++) {
                if(statements[i] == NULL) continue;
                if(*statements[i] == NULL) continue;
                Statement * statement = *statements[i];
                if(statement->statementType == STATEMENT_BREAK || statement->statementType == STATEMENT_CONTINUE) {
                    canLoopsBeOptimized = false;
                    break;
                }
            }
            free(statements);
            if(!canLoopsBeOptimized) break;
            item = item->next;
        }
        if(!canLoopsBeOptimized) break;
    }
    if(canLoopsBeOptimized) {
        fprintf(stderr, "Loops can be optimized\n");
    } else {
        fprintf(stderr, "Loops can not be optimized\n");
    }
    clock_t start = clock();
    float optimizationTime = 0;
    bool continueOptimizing = true;
    bool continueUpdatingTypes = true;
    while(continueUpdatingTypes) {
        if(canLoopsBeOptimized) performNestedStatementsExpansion((Statement**)&program, functionTable, program, NULL);
        continueUpdatingTypes = false;
        while(continueOptimizing) {
            continueOptimizing = false;
            PointerTable * resultTable = pointer_table_init();
            Table * optimizerVarInfo = table_init();
            buildNestedStatementVarUsages((Statement*)program, optimizerVarInfo);
            bool continueSameTableOptimizing = true;
            while(continueSameTableOptimizing) {
                continueSameTableOptimizing = false;
                continueSameTableOptimizing |= optimizeNestedStatements((Statement**)&program, functionTable, program, NULL, optimizerVarInfo, resultTable);
                continueSameTableOptimizing |= replaceErrorsWithExit((Statement**)&program, functionTable, program, NULL, resultTable);
                continueOptimizing |= continueSameTableOptimizing;
            }
            table_free(optimizerVarInfo);
            pointer_table_free(resultTable);
            for(int i = 0; i < TB_SIZE; i++) {
                TableItem* item = functionTable->tb[i];
                while(item != NULL) {
                    resultTable = pointer_table_init();
                    optimizerVarInfo = table_init();
                    buildNestedStatementVarUsages((Statement*)item->data, optimizerVarInfo);
                    bool continueSameTableOptimizing = true;
                    while(continueSameTableOptimizing) {
                        continueSameTableOptimizing = false;
                        continueSameTableOptimizing |= optimizeNestedStatements((Statement**)&item->data, functionTable, program, (Function*)item->data, optimizerVarInfo, resultTable);
                        continueSameTableOptimizing |= replaceErrorsWithExit((Statement**)&item->data, functionTable, program, (Function*)item->data, resultTable);
                        continueOptimizing |= continueSameTableOptimizing;
                    }
                    table_free(optimizerVarInfo);
                    pointer_table_free(resultTable);
                    item = item->next;
                }
            }
            if(continueOptimizing) continueUpdatingTypes = true;
            optimizationTime = (float)(clock() - start) / CLOCKS_PER_SEC;
            if(optimizationTime >= 0.5) break;
        }
        continueOptimizing = true;
        if(optimizationTime >= 0.3) break;
    }
}