#include "ast.h"
#include "symtable.h"
#include "emitter.h"
#include "string_builder.h"
#include "optimizer.h"

char * join_strings(char * str1, char * str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    char * result = malloc(len1 + len2 + 1);
    memcpy(result, str1, len1);
    memcpy(result + len1, str2, len2 + 1);
    return result;
}


size_t getNextCodeGenUID() {
    static size_t codeGenUID = 0;
    return codeGenUID++;
}

typedef struct {
    char * name;
    bool isGlobal;
    Type type;
} VariableInfo;

typedef struct {
    Table * varTable;
    Table * functionTable;
    bool isGlobal;
} Context;


void generateStatement(Statement * statement, Context ctx);

Symb generateConstant(Expression__Constant * constant) {
    switch (constant->type.type) {
        case TYPE_INT:
            return (Symb){.type = Type_int, .value.i = constant->value.integer};
            break;
        case TYPE_FLOAT:
            return (Symb){.type = Type_float, .value.f = constant->value.real};
            break;
        case TYPE_STRING:
            return (Symb){.type = Type_string, .value.s = constant->value.string};
            break;
        case TYPE_NULL:
            return (Symb){.type = Type_null};
            break;
        case TYPE_BOOL:
            return (Symb){.type = Type_bool, .value.b = constant->value.boolean};
            break;
        default:
            fprintf(stderr, "ERR: Invalid constant\n");
            exit(99);
    }
}

Symb generateVariable(Expression__Variable * statement, Context ctx) {
    // causes mem leak
    char * varId = join_strings("var&", statement->name);
    return (Symb){.type = Type_variable, .value.v.frameType = ((VariableInfo*)table_find(ctx.varTable, statement->name)->data)->isGlobal ? GF : LF, .value.v.name = varId};
}

Statement *** getAllStatements(Statement * parent, size_t * count) {
    int childrenCount = 0;
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

Symb generateSymbType(Expression * expression, Symb symb, Context ctx) {
    Type type = expression->getType(expression);
    if(type.isRequired == true) {
        switch (type.type) {
            case TYPE_INT:
                return (Symb){.type = Type_string, .value.s = "int"};
            case TYPE_BOOL:
                return (Symb){.type = Type_string, .value.s = "bool"};
            case TYPE_FLOAT:
                return (Symb){.type = Type_string, .value.s = "float"};
            case TYPE_STRING:
                return (Symb){.type = Type_string, .value.s = "string"};
            default:
                break;
        }
    }
    if(type.type == TYPE_NULL) {
        return (Symb){.type = Type_string, .value.s = "nil"};
    }
    StringBuilder sb;
    StringBuilder__init(&sb);
    StringBuilder__appendString(&sb, "var&type_output&");
    StringBuilder__appendInt(&sb, getNextCodeGenUID());
    Var typeOut = (Var){.frameType=ctx.isGlobal ? GF : LF, .name=sb.text};
    emit_DEFVAR(typeOut);
    emit_TYPE(typeOut, symb);
    return (Symb){.type = Type_variable, .value.v = typeOut};
}

Symb generateCastToBool(Expression * expression, Symb symb, Context ctx) {
    Type type = expression->getType(expression);
    if(type.type == TYPE_BOOL && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_BOOL, .isRequired=true});
        if(constant != NULL) {
            return generateConstant(constant);
        } else {
            fprintf(stderr, "ERR: Failed constant cast to bool, but this should always succeed\n");
            exit(99);
        }
    }
    // TODO: do known type cast
    Symb symbType = generateSymbType(expression, symb, ctx);
    size_t castUID = getNextCodeGenUID();
    StringBuilder castOutput;
    StringBuilder__init(&castOutput);
    StringBuilder__appendString(&castOutput, "var&cast_output&");
    StringBuilder__appendInt(&castOutput, castUID);
    StringBuilder castEnd;
    StringBuilder__init(&castEnd);
    StringBuilder__appendString(&castEnd, "cast_end&");
    StringBuilder__appendInt(&castEnd, castUID);
    Var result = (Var){.frameType=ctx.isGlobal ? GF : LF, .name=castOutput.text};
    emit_DEFVAR(result);
    StringBuilder notBool;
    StringBuilder__init(&notBool);
    StringBuilder__appendString(&notBool, "not_bool&");
    StringBuilder__appendInt(&notBool, castUID);
    emit_JUMPIFNEQ(notBool.text, symbType, (Symb){.type = Type_string, .value.s = "bool"});
    emit_MOVE(result, symb);
    emit_JUMP(castEnd.text);
    emit_LABEL(notBool.text);
    StringBuilder notNil;
    StringBuilder__init(&notNil);
    StringBuilder__appendString(&notNil, "not_nil&");
    StringBuilder__appendInt(&notNil, castUID);
    emit_JUMPIFNEQ(notNil.text, symbType, (Symb){.type = Type_string, .value.s = "nil"});
    emit_MOVE(result, (Symb){.type = Type_bool, .value.b = false});
    emit_JUMP(castEnd.text);
    emit_LABEL(notNil.text);
    StringBuilder notInt;
    StringBuilder__init(&notInt);
    StringBuilder__appendString(&notInt, "not_int&");
    StringBuilder__appendInt(&notInt, castUID);
    emit_JUMPIFNEQ(notInt.text, symbType, (Symb){.type = Type_string, .value.s = "int"});
    emit_PUSHS(symb);
    emit_PUSHS((Symb){.type = Type_int, .value.i = 0});
    emit_EQS();
    emit_NOTS();
    emit_POPS(result);
    emit_JUMP(castEnd.text);
    emit_LABEL(notInt.text);
    StringBuilder notFloat;
    StringBuilder__init(&notFloat);
    StringBuilder__appendString(&notFloat, "not_float&");
    StringBuilder__appendInt(&notFloat, castUID);
    emit_JUMPIFNEQ(notFloat.text, symbType, (Symb){.type = Type_string, .value.s = "float"});
    emit_PUSHS(symb);
    emit_PUSHS((Symb){.type = Type_float, .value.f = 0.0});
    emit_EQS();
    emit_NOTS();
    emit_POPS(result);
    emit_JUMP(castEnd.text);
    emit_LABEL(notFloat.text);
    StringBuilder notString;
    StringBuilder__init(&notString);
    StringBuilder__appendString(&notString, "not_string&");
    StringBuilder__appendInt(&notString, castUID);
    emit_JUMPIFNEQ(notString.text, symbType, (Symb){.type = Type_string, .value.s = "string"});
    emit_PUSHS(symb);
    emit_PUSHS((Symb){.type = Type_string, .value.s = ""});
    emit_EQS();
    emit_NOTS();
    emit_POPS(result);
    emit_JUMP(castEnd.text);
    emit_LABEL(notString.text);
    emit_DPRINT((Symb){.type = Type_string, .value.s = "ERR: Probably undefined variable while casting to bool"});
    emit_EXIT((Symb){.type = Type_int, .value.i = 5});
    emit_LABEL(castEnd.text);
    return symb;
}

Symb generateExpression(Expression * expression, Context ctx, bool throwaway, Var * outVar);

Symb generateFunctionCall(Expression__FunctionCall * expression, Context ctx, Var * outVarAlt) {
    TableItem * tableItem = table_find(ctx.functionTable, expression->name);
    if(tableItem == NULL) {
        fprintf(stderr, "Trying to call undefined function\n");
        exit(3);
    }
    Function * function = (Function*)tableItem->data;
    if(function->body == NULL) {
        // this is built in function
        if(strcmp(function->name, "write") == 0) {
            for(int i=0; i<expression->arity; i++) {
                Symb symb = generateExpression(expression->arguments[i], ctx, false, NULL);
                emit_WRITE(symb);
            }
            // TODO add return void
            return (Symb){.type=Type_int, .value.i=0};
        } else if(strcmp(function->name, "strval") == 0) {
            if(expression->arity == 1) {
                Type type = expression->arguments[0]->getType(expression->arguments[0]);
                if(type.type == TYPE_STRING && type.isRequired == true) {
                    return generateExpression(expression->arguments[0], ctx, false, NULL);
                } else if(type.type == TYPE_NULL) {
                    return (Symb){.type = Type_string, .value.s = ""};
                }
            }
        }
    }
    if(function->arity != expression->arity) {
        fprintf(stderr, "ERR: Function %s called with wrong number of arguments\n", expression->name);
        exit(4);
    }
    StringBuilder sbRet;
    StringBuilder__init(&sbRet);
    StringBuilder__appendString(&sbRet, "var&returnValue&");
    StringBuilder__appendInt(&sbRet, getNextCodeGenUID());
    if(strcmp(function->name, "reads") == 0) {
        Var var = (Var){.frameType=ctx.isGlobal ? GF : LF, .name=sbRet.text};
        if(outVarAlt != NULL) var = *outVarAlt;
        if(outVarAlt == NULL) emit_DEFVAR(var);
        emit_READ(var, Type_string);
        return (Symb){.type = Type_variable, .value.v=var};
    } else if(strcmp(function->name, "readi") == 0) {
        Var var = (Var){.frameType=ctx.isGlobal ? GF : LF, .name=sbRet.text};
        if(outVarAlt != NULL) var = *outVarAlt;
        if(outVarAlt == NULL) emit_DEFVAR(var);
        emit_READ(var, Type_int);
        return (Symb){.type = Type_variable, .value.v=var};
    } else if(strcmp(function->name, "readf") == 0) {
        Var var = (Var){.frameType=ctx.isGlobal ? GF : LF, .name=sbRet.text};
        if(outVarAlt != NULL) var = *outVarAlt;
        if(outVarAlt == NULL) emit_DEFVAR(var);
        emit_READ(var, Type_float);
        return (Symb){.type = Type_variable, .value.v=var};
    }
    emit_CREATEFRAME();
    if(strcmp(function->name, "floatval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        emit_DEFVAR((Var){.frameType=TF, .name="returnValue"});
        emit_INT2FLOAT((Var){.frameType=TF, .name="returnValue"}, symb);
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    } else if(strcmp(function->name, "intval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        emit_DEFVAR((Var){.frameType=TF, .name="returnValue"});
        emit_FLOAT2INT((Var){.frameType=TF, .name="returnValue"}, symb);
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    } else if(strcmp(function->name, "strval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        emit_DEFVAR((Var){.frameType=TF, .name="returnValue"});
        size_t strvalUID = getNextCodeGenUID();
        Symb type = generateSymbType(expression->arguments[0], symb, ctx);
        StringBuilder strval_end;
        StringBuilder__init(&strval_end);
        StringBuilder__appendString(&strval_end, "strval_end&");
        StringBuilder__appendInt(&strval_end, strvalUID);
        StringBuilder strval_not_string;
        StringBuilder__init(&strval_not_string);
        StringBuilder__appendString(&strval_not_string, "strval_not_string&");
        StringBuilder__appendInt(&strval_not_string, strvalUID);
        emit_JUMPIFNEQ(strval_not_string.text, type, (Symb){.type=Type_string, .value.s="string"});
        emit_MOVE((Var){.frameType=TF, .name="returnValue"}, symb);
        emit_JUMP(strval_end.text);
        emit_LABEL(strval_not_string.text);
        StringBuilder strval_not_null;
        StringBuilder__init(&strval_not_null);
        StringBuilder__appendString(&strval_not_null, "strval_not_null&");
        StringBuilder__appendInt(&strval_not_null, strvalUID);
        emit_JUMPIFNEQ(strval_not_null.text, type, (Symb){.type=Type_string, .value.s="nil"});
        emit_MOVE((Var){.frameType=TF, .name="returnValue"}, (Symb){.type=Type_string, .value.s=""});
        emit_JUMP(strval_end.text);
        emit_LABEL(strval_not_null.text);
        emit_DPRINT((Symb){.type=Type_string, .value.s="Unsupported argument type of strval\n"});
        emit_EXIT((Symb){.type=Type_int, .value.i=4});
        emit_LABEL(strval_end.text);
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    } else if(strcmp(function->name, "strlen") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        emit_DEFVAR((Var){.frameType=TF, .name="returnValue"});
        emit_STRLEN((Var){.frameType=TF, .name="returnValue"}, symb);
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    } else if(strcmp(function->name, "substring") == 0) {
        Symb symb1 = generateExpression(expression->arguments[0], ctx, false, NULL);
        Symb symb2 = generateExpression(expression->arguments[1], ctx, false, NULL);
        Symb symb3 = generateExpression(expression->arguments[2], ctx, false, NULL);
        Var returnValue = {.frameType = TF, .name = "returnValue"};
        emit_DEFVAR(returnValue);
        size_t substringUID = getNextCodeGenUID();
        StringBuilder tempVarName;
        StringBuilder__init(&tempVarName);
        StringBuilder__appendString(&tempVarName, "tempVar&");
        StringBuilder__appendInt(&tempVarName, substringUID);
        StringBuilder func_substring_end;
        StringBuilder__init(&func_substring_end);
        StringBuilder__appendString(&func_substring_end, "func_substring_end&");
        StringBuilder__appendInt(&func_substring_end, substringUID);
        Var tempVar = (Var){.frameType=TF, .name=tempVarName.text};
        emit_DEFVAR(tempVar);
        emit_PUSHS(symb2);
        emit_PUSHS((Symb){.type = Type_int, .value.i = 0});
        emit_LTS();
        emit_PUSHS(symb2);
        emit_PUSHS((Symb){.type = Type_int, .value.i = 0});
        emit_LTS();
        emit_ORS();
        emit_PUSHS(symb2);
        emit_PUSHS(symb3);
        emit_GTS();
        emit_ORS();
        emit_PUSHS(symb2);
        emit_STRLEN(tempVar, symb1);
        emit_PUSHS((Symb){.type=Type_variable, .value.v=tempVar});
        emit_LTS();
        emit_NOTS();
        emit_ORS();
        emit_PUSHS(symb3);
        emit_PUSHS((Symb){.type=Type_variable, .value.v=tempVar});
        emit_GTS();
        emit_ORS();
        emit_PUSHS((Symb){.type=Type_bool, .value.b=true});
        StringBuilder func_substring_loop_start;
        StringBuilder__init(&func_substring_loop_start);
        StringBuilder__appendString(&func_substring_loop_start, "func_substring_loop_start&");
        StringBuilder__appendInt(&func_substring_loop_start, substringUID);
        StringBuilder func_substring_loop_index;
        StringBuilder__init(&func_substring_loop_index);
        StringBuilder__appendString(&func_substring_loop_index, "var&func_substring_loop_index&");
        StringBuilder__appendInt(&func_substring_loop_index, substringUID);
        Var func_substring_loop_indexVar = (Var){.frameType=TF, .name=func_substring_loop_index.text};
        emit_DEFVAR(func_substring_loop_indexVar);
        emit_MOVE(func_substring_loop_indexVar, symb2);
        emit_MOVE(returnValue, (Symb){.type=Type_string, .value.s=""});
        emit_JUMPIFNEQS(func_substring_loop_start.text);
        emit_MOVE(returnValue, (Symb){.type=Type_null});
        emit_JUMP(func_substring_end.text);
        emit_LABEL(func_substring_loop_start.text);
        emit_JUMPIFEQ(func_substring_end.text, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar}, symb3);
        emit_GETCHAR(tempVar, symb1, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar});
        emit_CONCAT(returnValue, (Symb){.type=Type_variable, .value.v=returnValue}, (Symb){.type=Type_variable, .value.v=tempVar});
        emit_ADD(func_substring_loop_indexVar, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar}, (Symb){.type=Type_int, .value.i=1});
        emit_JUMP(func_substring_loop_start.text);
        emit_LABEL(func_substring_end.text);
        return (Symb){.type = Type_variable, .value.v=returnValue};
    } else if(strcmp(function->name, "ord") == 0) {
        size_t ordId = getNextCodeGenUID();
        StringBuilder sb;
        StringBuilder__init(&sb);
        StringBuilder__appendString(&sb, "ord_end&");
        StringBuilder__appendInt(&sb, ordId);
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        emit_DEFVAR((Var){.frameType=TF, .name="returnValue"});
        emit_STRLEN((Var){.frameType=TF, .name="returnValue"}, symb);
        emit_JUMPIFEQ(sb.text, (Symb){.type = Type_variable, .value.v = (Var){.frameType=TF, .name="returnValue"}}, (Symb){.type=Type_int, .value.i=0});
        emit_STRI2INT((Var){.frameType=TF, .name="returnValue"}, symb, (Symb){.type=Type_int, .value.i=0});
        emit_LABEL(sb.text);
        StringBuilder__free(&sb);
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    } else if(strcmp(function->name, "chr") == 0) {
        Symb symb1 = generateExpression(expression->arguments[0], ctx, false, NULL);
        emit_DEFVAR((Var){.frameType=TF, .name="returnValue"});
        emit_INT2CHAR((Var){.frameType=TF, .name="returnValue"}, symb1);
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    }
    bool containsNestedFunctionCall = false;
    for(int i = 0; i < expression->arity; i++) {
        size_t count;
        if(expression->arguments[i]->expressionType == EXPRESSION_FUNCTION_CALL) {
            containsNestedFunctionCall = true;
            break;
        }
        Statement *** statements = getAllStatements((Statement*)expression->arguments[i], &count);
        for(int j = 0; j < count; j++) {
            if((*statements[j])->statementType == STATEMENT_EXPRESSION) {
                Expression * expr = (Expression*)*statements[j];
                if(expr->expressionType == EXPRESSION_FUNCTION_CALL) {
                    containsNestedFunctionCall = true;
                    break;
                }
            }
        }
        free(statements);
        if(containsNestedFunctionCall) break;
    }
    if(!containsNestedFunctionCall) {
        for(int i=0; i<expression->arity; i++) {
            emit_DEFVAR((Var){.frameType = TF, .name = join_strings("var&", function->parameterNames[i])});
            emit_MOVE((Var){.frameType = TF, .name = join_strings("var&", function->parameterNames[i])}, generateExpression(expression->arguments[i], ctx, false, NULL));
        }
    } else {
        for(int i=0; i<expression->arity; i++) {
            emit_PUSHS(generateExpression(expression->arguments[i], ctx, true, NULL));
        }
        emit_CREATEFRAME();
        for(int i=expression->arity-1; i>=0; i--) {
            emit_DEFVAR((Var){.frameType = TF, .name = join_strings("var&", function->parameterNames[i])});
            emit_POPS((Var){.frameType = TF, .name = join_strings("var&", function->parameterNames[i])});
        }
    }
    
    char * functionLabel = join_strings("function&", expression->name);
    emit_CALL(functionLabel);
    free(functionLabel);
    return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
}

Symb saveTempSymb(Symb symb, FrameType frameType) {
    if(symb.type != Type_variable || symb.value.v.frameType != TF) {
        return symb;
    }
    size_t saveId = getNextCodeGenUID();
    // causes mem leak
    StringBuilder sb;
    StringBuilder__init(&sb);
    StringBuilder__appendString(&sb, "var&temp_symb_save&");
    StringBuilder__appendInt(&sb, saveId);
    Var var = (Var){.frameType = frameType, .name = sb.text};
    emit_DEFVAR(var);
    emit_MOVE(var, symb);
    return (Symb){.type = Type_variable, .value.v = var};
}

Symb generateBinaryOperator(Expression__BinaryOperator * expression, Context ctx, bool throwaway, Var * outVarAlt) {
    Symb left = generateExpression(expression->lSide, ctx, false, NULL);
    if(expression->operator == TOKEN_ASSIGN) {
        if(left.type != Type_variable) {
            fprintf(stderr, "Left side of assigment needs to be variable\n");
            // NOTE: not sure if right exit code
            exit(2);
        }
        Symb right;
        if(throwaway || outVarAlt != NULL) {
            right = generateExpression(expression->rSide, ctx, false, &left.value.v);
        } else {
            right = generateExpression(expression->rSide, ctx, false, NULL);
        }
        if(!throwaway) {
            right = saveTempSymb(right, ctx.isGlobal ? GF : LF);
        }
        if((!throwaway && outVarAlt == NULL) || right.type != left.type || right.value.v.frameType != left.value.v.frameType || strcmp(right.value.v.name, left.value.v.name) != 0) {
            emit_MOVE(left.value.v, right);
        }
        return right;
    }
    Symb right = generateExpression(expression->rSide, ctx, false, NULL);
    left = saveTempSymb(left, ctx.isGlobal ? GF : LF);
    size_t outVarId = getNextCodeGenUID();
    StringBuilder sb;
    StringBuilder__init(&sb);
    StringBuilder__appendString(&sb, "var&operator_output&");
    StringBuilder__appendInt(&sb, outVarId);
    Var outVar;
    if(outVarAlt == NULL) {
        outVar = (Var){.frameType=ctx.isGlobal ? GF : LF, .name=sb.text};
        emit_DEFVAR(outVar);
    } else {
        outVar = *outVarAlt;
    }
    Symb outSymb = (Symb){.type=Type_variable, .value.v = outVar};
    switch(expression->operator) {
        case TOKEN_PLUS:
            emit_ADD(outVar, left, right);
            break;
        case TOKEN_MINUS:
            emit_SUB(outVar, left, right);
            break;
        case TOKEN_CONCATENATE:
            emit_CONCAT(outVar, left, right);
            break;
        case TOKEN_MULTIPLY:
            emit_MUL(outVar, left, right);
            break;
        case TOKEN_DIVIDE:
            emit_DIV(outVar, left, right);
            break;
        case TOKEN_EQUALS: {
            Symb typeOut1 = generateSymbType(expression->lSide, left, ctx);
            Symb typeOut2 = generateSymbType(expression->rSide, right, ctx);
            if(typeOut1.type == Type_string && typeOut2.type == Type_string) {
                if(strcmp(typeOut1.value.s, typeOut2.value.s) == 0) {
                    emit_EQ(outVar, left, right);
                } else {
                    return (Symb){.type = Type_bool, .value.b = false};
                }
            } else {
                size_t operatorTypeCheckId = getNextCodeGenUID();
                StringBuilder sb3;
                StringBuilder__init(&sb3);
                StringBuilder__appendString(&sb3, "type_check_ok&");
                StringBuilder__appendInt(&sb3, operatorTypeCheckId);
                emit_JUMPIFEQ(sb3.text, typeOut1, typeOut2);
                emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=false});
                StringBuilder sb4;
                StringBuilder__init(&sb4);
                StringBuilder__appendString(&sb4, "operator_done&");
                StringBuilder__appendInt(&sb4, operatorTypeCheckId);
                emit_JUMP(sb4.text);
                emit_LABEL(sb3.text);
                emit_EQ(outVar, left, right);
                emit_LABEL(sb4.text);
            }
            break;
        }
        case TOKEN_NOT_EQUALS: {
            Symb typeOut1 = generateSymbType(expression->lSide, left, ctx);
            Symb typeOut2 = generateSymbType(expression->rSide, right, ctx);
            if(typeOut1.type == Type_string && typeOut2.type == Type_string) {
                if(strcmp(typeOut1.value.s, typeOut2.value.s) == 0) {
                    emit_EQ(outVar, left, right);
                    emit_NOT(outVar, outSymb);
                } else {
                    return (Symb){.type = Type_bool, .value.b = true};
                }
            } else {
                size_t operatorTypeCheckId = getNextCodeGenUID();
                StringBuilder sb3;
                StringBuilder__init(&sb3);
                StringBuilder__appendString(&sb3, "type_check_ok&");
                StringBuilder__appendInt(&sb3, operatorTypeCheckId);
                emit_JUMPIFEQ(sb3.text, typeOut1, typeOut2);
                emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=true});
                StringBuilder sb4;
                StringBuilder__init(&sb4);
                StringBuilder__appendString(&sb4, "operator_done&");
                StringBuilder__appendInt(&sb4, operatorTypeCheckId);
                emit_JUMP(sb4.text);
                emit_LABEL(sb3.text);
                emit_EQ(outVar, left, right);
                emit_NOT(outVar, outSymb);
                emit_LABEL(sb4.text);
            }
            break;
        }
        case TOKEN_LESS:
            emit_LT(outVar, left, right);
            break;
        case TOKEN_GREATER:
            emit_GT(outVar, left, right);
            break;
        case TOKEN_LESS_OR_EQUALS:
            emit_GT(outVar, left, right);
            emit_NOT(outVar, outSymb);
            break;
        case TOKEN_GREATER_OR_EQUALS:
            emit_LT(outVar, left, right);
            emit_NOT(outVar, outSymb);
            break;
        default:
            fprintf(stderr, "Unknown operator found while generating output code\n");
            exit(99);
    }
    return outSymb;
}

Symb generateExpression(Expression * expression, Context ctx, bool throwaway, Var * outVar) {
    switch(expression->expressionType) {
        case EXPRESSION_CONSTANT:
            return generateConstant((Expression__Constant*)expression);
            break;
        case EXPRESSION_VARIABLE:
            return generateVariable((Expression__Variable*)expression, ctx);
            break;
        case EXPRESSION_FUNCTION_CALL:
            return generateFunctionCall((Expression__FunctionCall*)expression, ctx, outVar);
            break;
        case EXPRESSION_BINARY_OPERATOR:
            return generateBinaryOperator((Expression__BinaryOperator*)expression, ctx, throwaway, outVar);
            break;
        default:
            fprintf(stderr, "Unknown expression type found while generating output code\n");
            exit(99);
    }
}

void generateStatementList(StatementList* statementList, Context ctx) {
    for(int i = 0; i < statementList->listSize; i++) {
        generateStatement(statementList->statements[i], ctx);
    }
}

void generateIf(StatementIf * statement, Context ctx) {
    size_t ifUID = getNextCodeGenUID();
    StringBuilder ifElseSb;
    StringBuilder__init(&ifElseSb);
    StringBuilder__appendString(&ifElseSb, "ifElse&");
    StringBuilder__appendInt(&ifElseSb, ifUID);
    StringBuilder ifEndSb;
    StringBuilder__init(&ifEndSb);
    StringBuilder__appendString(&ifEndSb, "ifEnd&");
    StringBuilder__appendInt(&ifEndSb, ifUID);

    bool isElseEmpty = statement->elseBody == NULL || (statement->elseBody->statementType == STATEMENT_LIST && ((StatementList*)statement->elseBody)->listSize == 0);
    Symb condition = generateExpression(statement->condition, ctx, false, NULL);
    emit_JUMPIFNEQ(ifElseSb.text, generateCastToBool(statement->condition, condition, ctx), (Symb){.type=Type_bool, .value.b = true});
    generateStatement(statement->ifBody, ctx);
    if(!isElseEmpty) emit_JUMP(ifEndSb.text);
    emit_LABEL(ifElseSb.text);
    if(!isElseEmpty) generateStatement(statement->elseBody, ctx);
    if(!isElseEmpty) emit_LABEL(ifEndSb.text);
    if(isElseEmpty) emit_COMMENT("Else body is empty");

    StringBuilder__free(&ifElseSb);
    StringBuilder__free(&ifEndSb);
}

void generateWhile(StatementWhile * statement, Context ctx) {
    size_t whileUID = getNextCodeGenUID();
    StringBuilder whileStartSb;
    StringBuilder__init(&whileStartSb);
    StringBuilder__appendString(&whileStartSb, "whileStart&");
    StringBuilder__appendInt(&whileStartSb, whileUID);
    StringBuilder whileEndSb;
    StringBuilder__init(&whileEndSb);
    StringBuilder__appendString(&whileEndSb, "whileEnd&");
    StringBuilder__appendInt(&whileEndSb, whileUID);

    emit_LABEL(whileStartSb.text);
    Symb condition = generateExpression(statement->condition, ctx, false, NULL);
    emit_JUMPIFNEQ(whileEndSb.text, generateCastToBool(statement->condition, condition, ctx), (Symb){.type=Type_bool, .value.b = true});
    generateStatement(statement->body, ctx);
    emit_JUMP(whileStartSb.text);
    emit_LABEL(whileEndSb.text);

    StringBuilder__free(&whileStartSb);
    StringBuilder__free(&whileEndSb);
}

void generateReturn(StatementReturn * statement, Context ctx) {
    if(statement != NULL && statement->expression != NULL) {
        Symb expr = generateExpression(statement->expression, ctx, ctx.isGlobal, NULL);
        if(!ctx.isGlobal) {
            emit_MOVE((Var){.frameType = LF, .name = "returnValue"}, expr);
        }
    }
    if(!ctx.isGlobal) {
        emit_POPFRAME();
        emit_RETURN();
    } else {
        emit_EXIT((Symb){.type=Type_int, .value.i=0});
    }
}

void generateStatement(Statement * statement, Context ctx) {
    if(statement == NULL) return;
    switch(statement->statementType) {
        case STATEMENT_EXPRESSION:
            generateExpression((Expression*) statement, ctx, true, NULL);
            break;
        case STATEMENT_LIST:
            generateStatementList((StatementList*)statement, ctx);
            break;
        case STATEMENT_IF:
            generateIf((StatementIf*)statement, ctx);
            break;
        case STATEMENT_WHILE:
            generateWhile((StatementWhile*)statement, ctx);
            break;
        case STATEMENT_RETURN:
            generateReturn((StatementReturn*)statement, ctx);
            break;
        case STATEMENT_FUNCTION:
            fprintf(stderr, "OFF, ignoring function...\n");
            break;
    }
}

void generateFunction(Function* function, Table * functionTable) {
    if(function->body == NULL) return;
    Table* localTable = table_init();
    char * functionLabel = join_strings("function&", function->name);
    emit_instruction_start();
    emit_LABEL(functionLabel);
    emit_instruction_end();
    free(functionLabel);
    emit_DEFVAR_start();
    emit_instruction_start();
    emit_DEFVAR((Var){frameType: TF, name: "returnValue"});
    size_t statementCount;
    Statement *** allStatements = getAllStatements(function->body, &statementCount);
    for(int i=0; i<statementCount; i++) {
        Statement * statement = *allStatements[i];
        if(statement == NULL) continue;
        if(statement->statementType == STATEMENT_EXPRESSION && ((Expression*)statement)->expressionType == EXPRESSION_VARIABLE) {
            Expression__Variable* variable = (Expression__Variable*) statement;
            if(table_find(localTable, variable->name) == NULL) {
                VariableInfo * variableInfo = malloc(sizeof(VariableInfo));
                variableInfo->name = variable->name;
                variableInfo->isGlobal = false;
                variableInfo->type.type = TYPE_UNKNOWN;
                variableInfo->type.isRequired = false;
                table_insert(localTable, variable->name, variableInfo);
                bool isParameter = false;
                for(int j=0; j<function->arity; j++) {
                    if(strcmp(function->parameterNames[j], variable->name) == 0) {
                        isParameter = true;
                        break;
                    }
                }
                if(!isParameter) {
                    emit_DEFVAR((Var){.frameType=TF, .name=join_strings("var&", variable->name)});
                }
            }
        }
    }
    free(allStatements);
    emit_DEFVAR_end();
    emit_DEFVAR_start();
    emit_PUSHFRAME();
    emit_instruction_end();
    emit_instruction_start();
    Context ctx;
    ctx.varTable = localTable;
    ctx.functionTable = functionTable;
    ctx.isGlobal = false;
    generateStatement(function->body, ctx);
    if(function->body->statementType != STATEMENT_LIST || ((StatementList*)function->body)->listSize == 0 || ((StatementList*)function->body)->statements[((StatementList*)function->body)->listSize-1]->statementType != STATEMENT_RETURN) {
        if(function->returnType.type != TYPE_VOID) {
            emit_DPRINT((Symb){.type=Type_string, .value.s="ERR: Function didn't return a value!"});
            emit_EXIT((Symb){.type=Type_int, .value.i=4});
        } else {
            generateReturn(NULL, ctx);
        }
    }
    emit_DEFVAR_end();
    emit_instruction_end();
}

void generateCode(StatementList * program, Table * functionTable) {
    emit_header();
    emit_DEFVAR_start();
    Table * globalTable = table_init();
    size_t statementCount;
    Statement *** allStatements = getAllStatements((Statement*)program, &statementCount);
    for(int i=0; i<statementCount; i++) {
        Statement * statement = *allStatements[i];
        if(statement == NULL) continue;
        if(statement->statementType == STATEMENT_EXPRESSION && ((Expression*)statement)->expressionType == EXPRESSION_VARIABLE) {
            Expression__Variable* variable = (Expression__Variable*) statement;
            if(table_find(globalTable, variable->name) == NULL) {
                VariableInfo * variableInfo = malloc(sizeof(VariableInfo));
                variableInfo->name = variable->name;
                variableInfo->isGlobal = true;
                variableInfo->type.type = TYPE_UNKNOWN;
                variableInfo->type.isRequired = false;
                table_insert(globalTable, variable->name, variableInfo);
                emit_DEFVAR((Var){.frameType=GF, .name=join_strings("var&", variable->name)});
            }
        }
    }
    free(allStatements);
    emit_instruction_start();
    Context ctx;
    ctx.varTable = globalTable;
    ctx.functionTable = functionTable;
    ctx.isGlobal = true;
    generateStatementList(program, ctx);
    if(program->listSize == 0 || program->statements[program->listSize-1]->statementType != STATEMENT_RETURN) {
        emit_EXIT((Symb){.type=TYPE_INT, .value.i=0});
    }
    emit_DEFVAR_end();
    emit_instruction_end();
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem* item = functionTable->tb[i];
        while(item != NULL) {
            Function* function = (Function*) item->data;
            if(function->body != NULL) {
                generateFunction(function, functionTable);
            }
            item = item->next;
        }
    }
}