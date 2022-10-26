#include "ast.h"

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
                    result->value.string = malloc(6);
                    sprintf(result->value.string, "%s", in->value.boolean ? "1" : "");
                    break;
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

Expression * performConstantFolding(Expression__BinaryOperator * in) {
    if(in->lSide->expressionType == EXPRESSION_CONSTANT && in->rSide->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * left = (Expression__Constant *) in->lSide;
        Expression__Constant * right = (Expression__Constant *) in->rSide;
        Expression__Constant * result = Expression__Constant__init();
        switch(in->operator) {
            switch(in->operator) {
                case TOKEN_PLUS:
                    if(left->type.type == TYPE_INT && right->type.type == TYPE_INT) {
                        result->value.integer = left->value.integer + right->value.integer;
                        result->type.type = TYPE_INT;
                        return result;
                    } else if((left->type.type == TYPE_FLOAT || left->type.type == TYPE_INT) && right->type.type == TYPE_FLOAT || right->type.type == TYPE_INT) {
                        result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real + performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                        result->type.type = TYPE_FLOAT;
                        return result;
                    } else {
                        free(result);
                        return NULL;
                    }
                case TOKEN_MINUS:
                    if(left->type.type == TYPE_INT && right->type.type == TYPE_INT) {
                        result->value.integer = left->value.integer - right->value.integer;
                        result->type.type = TYPE_INT;
                        return result;
                    } else if((left->type.type == TYPE_FLOAT || left->type.type == TYPE_INT) && right->type.type == TYPE_FLOAT || right->type.type == TYPE_INT) {
                        result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real - performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                        result->type.type = TYPE_FLOAT;
                        return result;
                    } else {
                        free(result);
                        return NULL;
                    }
                case TOKEN_CONCATENATE:
                    StringBuilder sb;
                    StringBuilder__init(&sb);
                    StringBuilder__appendString(&sb, performConstantCast(left, (Type){.type = TYPE_STRING, .isRequired = true})->value.string);
                    StringBuilder__appendString(&sb, performConstantCast(right, (Type){.type = TYPE_STRING, .isRequired = true})->value.string);
                    result->value.string = sb.text;
                    result->type.type = TYPE_STRING;
                    return result;
                case TOKEN_MULTIPLY:
                    if(left->type.type == TYPE_INT && right->type.type == TYPE_INT) {
                        result->value.integer = left->value.integer * right->value.integer;
                        result->type.type = TYPE_INT;
                        return result;
                    } else if((left->type.type == TYPE_FLOAT || left->type.type == TYPE_INT) && right->type.type == TYPE_FLOAT || right->type.type == TYPE_INT) {
                        result->value.real = performConstantCast(left, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real * performConstantCast(right, (Type){.type = TYPE_FLOAT, .isRequired = true})->value.real;
                        result->type.type = TYPE_FLOAT;
                        return result;
                    } else {
                        free(result);
                        return NULL;
                    }
                default:
                    free(result);
                    return NULL;
            }
        }
    }
}

Statement * performStatementFolding(Statement * in) {
    switch(in->statementType) {
        default:
            return NULL;
    }
}