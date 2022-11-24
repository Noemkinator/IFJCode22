#include "ast.h"
#include "symtable.h"
#include "emitter.h"
#include "string_builder.h"
#include "optimizer.h"

/**
 * @brief Join two strings together
 * 
 * @param str1 
 * @param str2 
 * @return char* 
 */
char * join_strings(char * str1, char * str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    char * result = malloc(len1 + len2 + 1);
    memcpy(result, str1, len1);
    memcpy(result + len1, str2, len2 + 1);
    return result;
}

/**
 * @brief Get the Next Code Gen U I D object
 * 
 * @return size_t 
 */
size_t getNextCodeGenUID() {
    static size_t codeGenUID = 0;
    return codeGenUID++;
}

typedef struct {
    char * name;
    bool isGlobal;
    bool isUsed;
    bool isTemporary;
} VariableInfo;

typedef struct {
    Table * varTable;
    Table * functionTable;
    bool isGlobal;
    Function * currentFunction;
    StatementList * program;
} Context;

void generateStatement(Statement * statement, Context ctx);

/**
 * @brief Generate code for a constant
 * 
 * @param constant 
 * @return Symb 
 */
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

/**
 * @brief Generate code for a variable type comment
 * 
 * @param variable
 * @param context
 */
void generateVarTypeComment(Expression__Variable * statement, Context ctx) {
    StringBuilder sb;
    StringBuilder__init(&sb);
    StringBuilder__appendString(&sb, "Type of variable ");
    StringBuilder__appendString(&sb, statement->name);
    StringBuilder__appendString(&sb, " is ");
    UnionType type = statement->super.getType((Expression *) statement, ctx.functionTable, ctx.program, ctx.currentFunction);
    if(type.isBool) {
        StringBuilder__appendString(&sb, "bool|");
    }
    if(type.isFloat) {
        StringBuilder__appendString(&sb, "float|");
    }
    if(type.isInt) {
        StringBuilder__appendString(&sb, "int|");
    }
    if(type.isString) {
        StringBuilder__appendString(&sb, "string|");
    }
    if(type.isUndefined) {
        StringBuilder__appendString(&sb, "undefined|");
    }
    if(type.isNull) {
        StringBuilder__appendString(&sb, "null|");
    }
    StringBuilder__removeLastChar(&sb);
    StringBuilder__appendString(&sb, ".");
    emit_COMMENT(sb.text);
    StringBuilder__free(&sb);
}

/**
 * @brief Generate code for temporarz variable
 * 
 * @param context
 * @return Var
 */
Var generateTemporaryVariable(Context ctx) {
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem * item = ctx.varTable->tb[i];
        while(item != NULL) {
            VariableInfo * info = item->data;
            if(!info->isUsed && info->isTemporary && info->isGlobal == ctx.isGlobal) {
                info->isUsed = true;
                return (Var){.name = info->name, .frameType = ctx.isGlobal ? GF : LF};
            }
            item = item->next;
        }
    }
    size_t tempVarUID = getNextCodeGenUID();
    StringBuilder tempVarName;
    StringBuilder__init(&tempVarName);
    StringBuilder__appendString(&tempVarName, "tempVar&");
    StringBuilder__appendInt(&tempVarName, tempVarUID);
    Var tempVar = (Var){.name = tempVarName.text, .frameType = ctx.isGlobal ? GF : LF};
    emit_DEFVAR(tempVar);
    VariableInfo * tempVarInfo = malloc(sizeof(VariableInfo));
    tempVarInfo->name = tempVarName.text;
    tempVarInfo->isGlobal = ctx.isGlobal;
    tempVarInfo->isUsed = true;
    tempVarInfo->isTemporary = true;
    table_insert(ctx.varTable, tempVarName.text, tempVarInfo);
    return tempVar;
}

/**
 * @brief Destroy temporary variable
 * 
 * @param variable 
 * @param context
 */
void freeTemporaryVariable(Var var, Context ctx) {
    TableItem * item = table_find(ctx.varTable, var.name);
    if(item == NULL) return;
    VariableInfo * info = item->data;
    if(info->isTemporary) {
        if(info->isUsed) {
            info->isUsed = false;
        } else {
            fprintf(stderr, "!!!!! Trying to free unused temporary variable %s!!!!!\n", var.name);
        }
    }
}

/**
 * @brief Generate code for a variable
 * 
 * @param variable 
 * @param context
 * @return Var
 */
void freeTemporarySymbol(Symb symb, Context ctx) {
    if(symb.type == Type_variable) {
        freeTemporaryVariable(symb.value.v, ctx);
    }
}

/**
 * @brief Generate code for a variable
 * 
 * @param statement
 * @param context
 * @return Symb
 */
Symb generateVariable(Expression__Variable * statement, Context ctx) {
    // causes mem leak
    generateVarTypeComment(statement, ctx);
    char * varId = statement->name;
    Var variable = (Var){.name = varId, .frameType = ((VariableInfo*)table_find(ctx.varTable, statement->name)->data)->isGlobal ? GF : LF};
    Symb symb = (Symb){.type = Type_variable, .value.v = variable};
    UnionType type = statement->super.getType((Expression *) statement, ctx.functionTable, ctx.program, ctx.currentFunction);
    if(type.isUndefined) {
        size_t variableCheckUID = getNextCodeGenUID();
        Var var = generateTemporaryVariable(ctx);
        StringBuilder variableDefined;
        StringBuilder__init(&variableDefined);
        StringBuilder__appendString(&variableDefined, "variable_defined&");
        StringBuilder__appendInt(&variableDefined, variableCheckUID);
        StringBuilder errorMessage;
        StringBuilder__init(&errorMessage);
        StringBuilder__appendString(&errorMessage, "Variable ");
        StringBuilder__appendString(&errorMessage, statement->name);
        StringBuilder__appendString(&errorMessage, " is not defined.");
        emit_TYPE(var, symb);
        emit_JUMPIFNEQ(variableDefined.text, (Symb){.type = Type_variable, .value.v = var}, (Symb){.type = Type_string, .value.s = ""});
        emit_DPRINT((Symb){.type = Type_string, .value.s = errorMessage.text});
        emit_EXIT((Symb){.type = Type_int, .value.i = 5});
        emit_LABEL(variableDefined.text);
        StringBuilder__free(&variableDefined);
        StringBuilder__free(&errorMessage);
        freeTemporaryVariable(var, ctx);
    }
    return symb;
}

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
 * @brief Generate symbol type value
 * 
 * @param expression
 * @param symbol
 * @param context
 * @return Symb
 */
Symb generateSymbType(Expression * expression, Symb symb, Context ctx) {
    Type type = unionTypeToType(expression->getType(expression, ctx.functionTable, ctx.program, ctx.currentFunction));
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
    Var typeOut = generateTemporaryVariable(ctx);
    emit_TYPE(typeOut, symb);
    return (Symb){.type = Type_variable, .value.v = typeOut};
}

/**
 * @brief Generate code for a bool type cast
 * 
 * @param expression
 * @param symbol
 * @param context
 * @return Symb
 */
Symb generateCastToBool(Expression * expression, Symb symb, Context ctx, bool isCondtion) {
    UnionType unionType = expression->getType(expression, ctx.functionTable, ctx.program, ctx.currentFunction);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_BOOL && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        if(!isCondtion) {
            Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_BOOL, .isRequired=true});
            if(constant != NULL) {
                return generateConstant(constant);
            } else {
                fprintf(stderr, "ERR: Failed constant cast to bool, but this should always succeed\n");
                exit(99);
            }
        } else {
            Expression__Constant * constant = performConstantCastCondition((Expression__Constant*)expression);
            return generateConstant(constant);
        }
    }
    Symb symbType = generateSymbType(expression, symb, ctx);
    size_t castUID = getNextCodeGenUID();
    StringBuilder castEnd;
    StringBuilder__init(&castEnd);
    StringBuilder__appendString(&castEnd, "cast_end&");
    StringBuilder__appendInt(&castEnd, castUID);
    Var result = generateTemporaryVariable(ctx);
    if(unionType.isBool) {
        StringBuilder notBool;
        StringBuilder__init(&notBool);
        StringBuilder__appendString(&notBool, "not_bool&");
        StringBuilder__appendInt(&notBool, castUID);
        emit_JUMPIFNEQ(notBool.text, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_MOVE(result, symb);
        emit_JUMP(castEnd.text);
        emit_LABEL(notBool.text);
        StringBuilder__free(&notBool);
    }
    if(unionType.isNull) {
        StringBuilder notNil;
        StringBuilder__init(&notNil);
        StringBuilder__appendString(&notNil, "not_nil&");
        StringBuilder__appendInt(&notNil, castUID);
        emit_JUMPIFNEQ(notNil.text, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMP(castEnd.text);
        emit_LABEL(notNil.text);
        StringBuilder__free(&notNil);
    }
    if(unionType.isInt) {
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
        StringBuilder__free(&notInt);
    }
    if(unionType.isFloat) {
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
        StringBuilder__free(&notFloat);
    }
    if(unionType.isString) {
        StringBuilder notString;
        StringBuilder__init(&notString);
        StringBuilder__appendString(&notString, "not_string&");
        StringBuilder__appendInt(&notString, castUID);
        emit_JUMPIFNEQ(notString.text, symbType, (Symb){.type = Type_string, .value.s = "string"});
        emit_PUSHS(symb);
        emit_PUSHS((Symb){.type = Type_string, .value.s = ""});
        emit_EQS();
        if(isCondtion) {
            emit_PUSHS(symb);
            emit_PUSHS((Symb){.type = Type_string, .value.s = "0"});
            emit_EQS();
            emit_ORS();
        }
        emit_NOTS();
        emit_POPS(result);
        emit_JUMP(castEnd.text);
        emit_LABEL(notString.text);
        StringBuilder__free(&notString);
    }
    emit_LABEL(castEnd.text);
    StringBuilder__free(&castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}

Symb generateCastToInt(Symb symb, Expression * expression, Context * ctx, Symb * typeSymb) {
    UnionType unionType = expression->getType(expression, ctx->functionTable, ctx->program, ctx->currentFunction);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_INT && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_INT, .isRequired=true});
        if(constant != NULL) {
            return generateConstant(constant);
        } else {
            fprintf(stderr, "ERR: Failed constant cast to int, but this should always succeed\n");
            exit(99);
        }
    }
    Symb symbType;
    if(typeSymb == NULL) {
        symbType = generateSymbType(expression, symb, *ctx);
    } else {
        symbType = *typeSymb;
    }
    size_t castUID = getNextCodeGenUID();
    StringBuilder castEnd;
    StringBuilder__init(&castEnd);
    StringBuilder__appendString(&castEnd, "cast_end&");
    StringBuilder__appendInt(&castEnd, castUID);
    Var result = generateTemporaryVariable(*ctx);
    if(unionType.isBool) {
        StringBuilder notBool;
        StringBuilder__init(&notBool);
        StringBuilder__appendString(&notBool, "not_bool&");
        StringBuilder__appendInt(&notBool, castUID);
        StringBuilder isTrue;
        StringBuilder__init(&isTrue);
        StringBuilder__appendString(&isTrue, "is_true&");
        StringBuilder__appendInt(&isTrue, castUID);
        emit_JUMPIFNEQ(notBool.text, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_JUMPIFEQ(isTrue.text, symbType, (Symb){.type = Type_bool, .value.b = true});
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMP(castEnd.text);
        emit_LABEL(isTrue.text);
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMP(castEnd.text);
        emit_LABEL(notBool.text);
        StringBuilder__free(&notBool);
    }
    if(unionType.isNull) {
        StringBuilder notNil;
        StringBuilder__init(&notNil);
        StringBuilder__appendString(&notNil, "not_nil&");
        StringBuilder__appendInt(&notNil, castUID);
        emit_JUMPIFNEQ(notNil.text, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMP(castEnd.text);
        emit_LABEL(notNil.text);
        StringBuilder__free(&notNil);
    }
    if(unionType.isInt) {
        StringBuilder notInt;
        StringBuilder__init(&notInt);
        StringBuilder__appendString(&notInt, "not_int&");
        StringBuilder__appendInt(&notInt, castUID);
        emit_JUMPIFNEQ(notInt.text, symbType, (Symb){.type = Type_string, .value.s = "int"});
        emit_MOVE(result, symb);
        emit_JUMP(castEnd.text);
        emit_LABEL(notInt.text);
        StringBuilder__free(&notInt);
    }
    if(unionType.isFloat) {
        StringBuilder notFloat;
        StringBuilder__init(&notFloat);
        StringBuilder__appendString(&notFloat, "not_float&");
        StringBuilder__appendInt(&notFloat, castUID);
        emit_JUMPIFNEQ(notFloat.text, symbType, (Symb){.type = Type_string, .value.s = "float"});
        emit_FLOAT2INT(result, symb);
        emit_LABEL(notFloat.text);
        StringBuilder__free(&notFloat);
    }
    if(unionType.isString) {
        StringBuilder notString;
        StringBuilder__init(&notString);
        StringBuilder__appendString(&notString, "not_string&");
        StringBuilder__appendInt(&notString, castUID);
        emit_JUMPIFNEQ(notString.text, symbType, (Symb){.type = Type_string, .value.s = "string"});
        // TODO
        emit_JUMP(castEnd.text);
        emit_LABEL(notString.text);
        StringBuilder__free(&notString);
    }
    emit_LABEL(castEnd.text);
    StringBuilder__free(&castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}

Symb generateCastToFloat(Symb symb, Expression * expression, Context * ctx, Symb * typeSymb) {
    UnionType unionType = expression->getType(expression, ctx->functionTable, ctx->program, ctx->currentFunction);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_FLOAT && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_FLOAT, .isRequired=true});
        if(constant != NULL) {
            return generateConstant(constant);
        } else {
            fprintf(stderr, "ERR: Failed constant cast to float, but this should always succeed\n");
            exit(99);
        }
    }
    Symb symbType;
    if(typeSymb == NULL) {
        symbType = generateSymbType(expression, symb, *ctx);
    } else {
        symbType = *typeSymb;
    }
    size_t castUID = getNextCodeGenUID();
    StringBuilder castEnd;
    StringBuilder__init(&castEnd);
    StringBuilder__appendString(&castEnd, "cast_end&");
    StringBuilder__appendInt(&castEnd, castUID);
    Var result = generateTemporaryVariable(*ctx);
    if(unionType.isBool) {
        StringBuilder notBool;
        StringBuilder__init(&notBool);
        StringBuilder__appendString(&notBool, "not_bool&");
        StringBuilder__appendInt(&notBool, castUID);
        StringBuilder isTrue;
        StringBuilder__init(&isTrue);
        StringBuilder__appendString(&isTrue, "is_true&");
        StringBuilder__appendInt(&isTrue, castUID);
        emit_JUMPIFNEQ(notBool.text, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_JUMPIFEQ(isTrue.text, symbType, (Symb){.type = Type_bool, .value.b = true});
        emit_MOVE(result, (Symb){.type = Type_float, .value.f = 0});
        emit_JUMP(castEnd.text);
        emit_LABEL(isTrue.text);
        emit_MOVE(result, (Symb){.type = Type_float, .value.f = 1});
        emit_JUMP(castEnd.text);
        emit_LABEL(notBool.text);
        StringBuilder__free(&notBool);
    }
    if(unionType.isNull) {
        StringBuilder notNil;
        StringBuilder__init(&notNil);
        StringBuilder__appendString(&notNil, "not_nil&");
        StringBuilder__appendInt(&notNil, castUID);
        emit_JUMPIFNEQ(notNil.text, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type = Type_float, .value.f = 0});
        emit_JUMP(castEnd.text);
        emit_LABEL(notNil.text);
        StringBuilder__free(&notNil);
    }
    if(unionType.isInt) {
        StringBuilder notInt;
        StringBuilder__init(&notInt);
        StringBuilder__appendString(&notInt, "not_int&");
        StringBuilder__appendInt(&notInt, castUID);
        emit_JUMPIFNEQ(notInt.text, symbType, (Symb){.type = Type_string, .value.s = "int"});
        emit_INT2FLOAT(result, symb);
        emit_JUMP(castEnd.text);
        emit_LABEL(notInt.text);
        StringBuilder__free(&notInt);
    }
    if(unionType.isFloat) {
        StringBuilder notFloat;
        StringBuilder__init(&notFloat);
        StringBuilder__appendString(&notFloat, "not_float&");
        StringBuilder__appendInt(&notFloat, castUID);
        emit_JUMPIFNEQ(notFloat.text, symbType, (Symb){.type = Type_string, .value.s = "float"});
        emit_MOVE(result, symb);
        emit_LABEL(notFloat.text);
        StringBuilder__free(&notFloat);
    }
    if(unionType.isString) {
        StringBuilder notString;
        StringBuilder__init(&notString);
        StringBuilder__appendString(&notString, "not_string&");
        StringBuilder__appendInt(&notString, castUID);
        emit_JUMPIFNEQ(notString.text, symbType, (Symb){.type = Type_string, .value.s = "string"});
        // TODO
        emit_JUMP(castEnd.text);
        emit_LABEL(notString.text);
        StringBuilder__free(&notString);
    }
    emit_LABEL(castEnd.text);
    StringBuilder__free(&castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}


Symb generateCastToString(Symb symb, Expression * expression, Context * ctx, Symb * typeSymb) {
    UnionType unionType = expression->getType(expression, ctx->functionTable, ctx->program, ctx->currentFunction);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_STRING && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_STRING, .isRequired=true});
        if(constant != NULL) {
            return generateConstant(constant);
        } else {
            fprintf(stderr, "ERR: Failed constant cast to float, but this should always succeed\n");
            exit(99);
        }
    }
    Symb symbType;
    if(typeSymb == NULL) {
        symbType = generateSymbType(expression, symb, *ctx);
    } else {
        symbType = *typeSymb;
    }
    size_t castUID = getNextCodeGenUID();
    StringBuilder castEnd;
    StringBuilder__init(&castEnd);
    StringBuilder__appendString(&castEnd, "cast_end&");
    StringBuilder__appendInt(&castEnd, castUID);
    Var result = generateTemporaryVariable(*ctx);
    if(unionType.isBool) {
        StringBuilder notBool;
        StringBuilder__init(&notBool);
        StringBuilder__appendString(&notBool, "not_bool&");
        StringBuilder__appendInt(&notBool, castUID);
        emit_JUMPIFNEQ(notBool.text, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        // TODO
        emit_JUMP(castEnd.text);
        emit_LABEL(notBool.text);
        StringBuilder__free(&notBool);
    }
    if(unionType.isNull) {
        StringBuilder notNil;
        StringBuilder__init(&notNil);
        StringBuilder__appendString(&notNil, "not_nil&");
        StringBuilder__appendInt(&notNil, castUID);
        emit_JUMPIFNEQ(notNil.text, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type=Type_string, .value.s=""});
        emit_JUMP(castEnd.text);
        emit_LABEL(notNil.text);
        StringBuilder__free(&notNil);
    }
    if(unionType.isInt) {
        StringBuilder notInt;
        StringBuilder__init(&notInt);
        StringBuilder__appendString(&notInt, "not_int&");
        StringBuilder__appendInt(&notInt, castUID);
        emit_JUMPIFNEQ(notInt.text, symbType, (Symb){.type = Type_string, .value.s = "int"});
        // TODO
        emit_JUMP(castEnd.text);
        emit_LABEL(notInt.text);
        StringBuilder__free(&notInt);
    }
    if(unionType.isFloat) {
        StringBuilder notFloat;
        StringBuilder__init(&notFloat);
        StringBuilder__appendString(&notFloat, "not_float&");
        StringBuilder__appendInt(&notFloat, castUID);
        emit_JUMPIFNEQ(notFloat.text, symbType, (Symb){.type = Type_string, .value.s = "float"});
        // TODO
        emit_LABEL(notFloat.text);
        StringBuilder__free(&notFloat);
    }
    if(unionType.isString) {
        StringBuilder notString;
        StringBuilder__init(&notString);
        StringBuilder__appendString(&notString, "not_string&");
        StringBuilder__appendInt(&notString, castUID);
        emit_JUMPIFNEQ(notString.text, symbType, (Symb){.type = Type_string, .value.s = "string"});
        emit_MOVE(result, symb);
        emit_JUMP(castEnd.text);
        emit_LABEL(notString.text);
        StringBuilder__free(&notString);
    }
    emit_LABEL(castEnd.text);
    StringBuilder__free(&castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}

/**
 * @brief Save temporary symbol to variable
 * 
 * @param symbol
 * @param context
 * @return Symb
 */
Symb saveTempSymb(Symb symb, Context ctx) {
    if(symb.type != Type_variable || symb.value.v.frameType != TF) {
        return symb;
    }
    Var var = generateTemporaryVariable(ctx);
    emit_MOVE(var, symb);
    return (Symb){.type = Type_variable, .value.v = var};
}

/**
 * @brief Generate expression
 * 
 * @param expression
 * @param context
 * @param bool
 * @param variable
 * @return Symb
 */
Symb generateExpression(Expression * expression, Context ctx, bool throwaway, Var * outVar);

void emitTypeCheck(Type requiredType, Expression * subTypeExpression, Symb subTypeSymbol, Context ctx, char * typeCheckFailMsg) {
    Type subType = unionTypeToType(subTypeExpression->getType(subTypeExpression, ctx.functionTable, ctx.program, ctx.currentFunction));
    if(requiredType.type == subType.type && (requiredType.isRequired == subType.isRequired || requiredType.isRequired == false)) {
        return;
    }
    if(!requiredType.isRequired && subType.type == TYPE_NULL) {
        return;
    }
    StringBuilder typeCheckPassed;
    StringBuilder__init(&typeCheckPassed);
    StringBuilder__appendString(&typeCheckPassed, "type_check_passed&");
    StringBuilder__appendInt(&typeCheckPassed, getNextCodeGenUID());
    Symb realType = generateSymbType(subTypeExpression, subTypeSymbol, ctx);
    char * requiredTypeStr = NULL;
    if(requiredType.type == TYPE_INT) {
        requiredTypeStr = "int";
    } else if(requiredType.type == TYPE_FLOAT) {
        requiredTypeStr = "float";
    } else if(requiredType.type == TYPE_STRING) {
        requiredTypeStr = "string";
    } else if(requiredType.type == TYPE_BOOL) {
        requiredTypeStr = "bool";
    } else if(requiredType.type == TYPE_NULL) {
        requiredTypeStr = "nil";
    } else {
        fprintf(stderr, "ERR: Unexpected type of func parameter\n");
        exit(99);
    }
    emit_JUMPIFEQ(typeCheckPassed.text, realType, (Symb){.type=Type_string, .value.s=requiredTypeStr});
    if(!requiredType.isRequired) {
        emit_JUMPIFEQ(typeCheckPassed.text, realType, (Symb){.type=Type_string, .value.s="nil"});
    }
    emit_DPRINT((Symb){.type=Type_string, .value.s=typeCheckFailMsg});
    emit_EXIT((Symb){.type=Type_int, .value.i=4});
    emit_LABEL(typeCheckPassed.text);
    freeTemporarySymbol(realType, ctx);
    StringBuilder__free(&typeCheckPassed);
}

void freeArguments(Symb * arguments, int argumentCount, Context ctx) {
    for(int i = 0; i < argumentCount; i++) {
        freeTemporarySymbol(arguments[i], ctx);
    }
}

/**
 * @brief Generate code for a function call
 * 
 * @param expression
 * @param context
 * @param variable
 * @return Symb
 */
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
            return (Symb){.type=Type_null};
        }
    }
    if(function->arity != expression->arity) {
        fprintf(stderr, "ERR: Function %s called with wrong number of arguments\n", expression->name);
        exit(4);
    }
    if(strcmp(function->name, "reads") == 0) {
        Var var;
        if(outVarAlt != NULL) var = *outVarAlt;
        else var = generateTemporaryVariable(ctx);
        emit_READ(var, Type_string);
        return (Symb){.type = Type_variable, .value.v=var};
    } else if(strcmp(function->name, "readi") == 0) {
        Var var;
        if(outVarAlt != NULL) var = *outVarAlt;
        else var = generateTemporaryVariable(ctx);
        emit_READ(var, Type_int);
        return (Symb){.type = Type_variable, .value.v=var};
    } else if(strcmp(function->name, "readf") == 0) {
        Var var;
        if(outVarAlt != NULL) var = *outVarAlt;
        else var = generateTemporaryVariable(ctx);
        emit_READ(var, Type_float);
        return (Symb){.type = Type_variable, .value.v=var};
    } else if(strcmp(function->name, "floatval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        Symb retSymb = generateCastToFloat(symb, expression->arguments[0], &ctx, NULL);
        if(symb.type != Type_variable || retSymb.type != Type_variable || symb.value.v.frameType != retSymb.value.v.frameType || strcmp(symb.value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(symb, ctx);
        }
        return retSymb;
    } else if(strcmp(function->name, "intval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        Symb retSymb = generateCastToInt(symb, expression->arguments[0], &ctx, NULL);
        if(symb.type != Type_variable || retSymb.type != Type_variable || symb.value.v.frameType != retSymb.value.v.frameType || strcmp(symb.value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(symb, ctx);
        }
        return retSymb;
    } else if(strcmp(function->name, "boolval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        Symb retSymb = generateCastToBool(expression->arguments[0], symb, ctx, false);
        if(symb.type != Type_variable || retSymb.type != Type_variable || symb.value.v.frameType != retSymb.value.v.frameType || strcmp(symb.value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(symb, ctx);
        }
        return retSymb;
    } else if(strcmp(function->name, "strval") == 0) {
        Symb symb = generateExpression(expression->arguments[0], ctx, false, NULL);
        Symb retSymb = generateCastToString(symb, expression->arguments[0], &ctx, NULL);
        if(symb.type != Type_variable || retSymb.type != Type_variable || symb.value.v.frameType != retSymb.value.v.frameType || strcmp(symb.value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(symb, ctx);
        }
        return retSymb;
    }
    Symb * arguments = malloc(sizeof(Symb) * expression->arity);
    for(int i=0; i<expression->arity; i++) {
        arguments[i] = saveTempSymb(generateExpression(expression->arguments[i], ctx, false, NULL), ctx);
    }
    for(int i=0; i<expression->arity; i++) {
        Type expectedType = function->parameterTypes[i];
        StringBuilder typeCheckFailMsg;
        StringBuilder__init(&typeCheckFailMsg);
        StringBuilder__appendString(&typeCheckFailMsg, "Type check failed for argument ");
        StringBuilder__appendInt(&typeCheckFailMsg, i);
        StringBuilder__appendString(&typeCheckFailMsg, " of function ");
        StringBuilder__appendString(&typeCheckFailMsg, function->name);
        StringBuilder__appendString(&typeCheckFailMsg, "\n");
        emitTypeCheck(expectedType, expression->arguments[i], arguments[i], ctx, typeCheckFailMsg.text);
        StringBuilder__free(&typeCheckFailMsg);
    }
    if(strcmp(function->name, "strlen") == 0) {
        Symb symb = arguments[0];
        Var retVar = generateTemporaryVariable(ctx);
        emit_STRLEN(retVar, symb);
        freeTemporarySymbol(symb, ctx);
        return (Symb){.type = Type_variable, .value.v=retVar};
    } else if(strcmp(function->name, "substring") == 0) {
        Symb symb1 = arguments[0];
        Symb symb2 = arguments[1];
        Symb symb3 = arguments[2];
        if(symb2.type == Type_int && symb2.value.i < 0) {
            freeArguments(arguments, expression->arity, ctx);
            return (Symb){.type = Type_null};
        }
        if(symb3.type == Type_int && symb3.value.i < 0) {
            freeArguments(arguments, expression->arity, ctx);
            return (Symb){.type = Type_null};
        }
        if(symb2.type == Type_int && symb3.type == Type_int && symb2.value.i > symb3.value.i) {
            freeArguments(arguments, expression->arity, ctx);
            return (Symb){.type = Type_null};
        }
        if(symb1.type == Type_string && symb2.type == Type_int && symb2.value.i >= strlen(symb1.value.s)) {
            freeArguments(arguments, expression->arity, ctx);
            return (Symb){.type = Type_null};
        }
        if(symb1.type == Type_string && symb3.type == Type_int && symb3.value.i > strlen(symb1.value.s)) {
            freeArguments(arguments, expression->arity, ctx);
            return (Symb){.type = Type_null};
        }
        Var retVar = generateTemporaryVariable(ctx);
        size_t substringUID = getNextCodeGenUID();
        StringBuilder func_substring_end;
        StringBuilder__init(&func_substring_end);
        StringBuilder__appendString(&func_substring_end, "func_substring_end&");
        StringBuilder__appendInt(&func_substring_end, substringUID);
        Var tempVar = generateTemporaryVariable(ctx);
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
        Var func_substring_loop_indexVar = generateTemporaryVariable(ctx);
        emit_MOVE(func_substring_loop_indexVar, symb2);
        emit_MOVE(retVar, (Symb){.type=Type_string, .value.s=""});
        emit_JUMPIFNEQS(func_substring_loop_start.text);
        emit_MOVE(retVar, (Symb){.type=Type_null});
        emit_JUMP(func_substring_end.text);
        emit_LABEL(func_substring_loop_start.text);
        emit_JUMPIFEQ(func_substring_end.text, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar}, symb3);
        emit_GETCHAR(tempVar, symb1, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar});
        emit_CONCAT(retVar, (Symb){.type=Type_variable, .value.v=retVar}, (Symb){.type=Type_variable, .value.v=tempVar});
        emit_ADD(func_substring_loop_indexVar, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar}, (Symb){.type=Type_int, .value.i=1});
        emit_JUMP(func_substring_loop_start.text);
        emit_LABEL(func_substring_end.text);
        freeTemporaryVariable(tempVar, ctx);
        freeTemporaryVariable(func_substring_loop_indexVar, ctx);
        freeArguments(arguments, expression->arity, ctx);
        return (Symb){.type = Type_variable, .value.v=retVar};
    } else if(strcmp(function->name, "ord") == 0) {
        size_t ordId = getNextCodeGenUID();
        StringBuilder sb;
        StringBuilder__init(&sb);
        StringBuilder__appendString(&sb, "ord_end&");
        StringBuilder__appendInt(&sb, ordId);
        Symb symb = arguments[0];
        Var retVar = generateTemporaryVariable(ctx);
        emit_STRLEN(retVar, symb);
        emit_JUMPIFEQ(sb.text, (Symb){.type = Type_variable, .value.v = retVar}, (Symb){.type=Type_int, .value.i=0});
        emit_STRI2INT(retVar, symb, (Symb){.type=Type_int, .value.i=0});
        emit_LABEL(sb.text);
        StringBuilder__free(&sb);
        freeArguments(arguments, expression->arity, ctx);
        return (Symb){.type = Type_variable, .value.v=retVar};
    } else if(strcmp(function->name, "chr") == 0) {
        Symb symb1 = arguments[0];
        Var retVar = generateTemporaryVariable(ctx);
        emit_INT2CHAR(retVar, symb1);
        freeArguments(arguments, expression->arity, ctx);
        return (Symb){.type = Type_variable, .value.v=retVar};
    }
    emit_CREATEFRAME();
    for(int i=0; i<expression->arity; i++) {
        emit_DEFVAR((Var){.frameType = TF, .name = function->parameterNames[i]});
        emit_MOVE((Var){.frameType = TF, .name = function->parameterNames[i]}, arguments[i]);
    }
    
    char * functionLabel = join_strings("function&", expression->name);
    emit_CALL(functionLabel);
    free(functionLabel);
    freeArguments(arguments, expression->arity, ctx);
    if(function->returnType.type != TYPE_VOID) {
        return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
    } else {
        return (Symb){.type = Type_null};
    }
}

void emitAddMulSubCast(Symb symb1, Symb symb2, Expression * expr1, Expression * expr2, Symb * out1, Symb * out2, Context * ctx) {
    UnionType unionType1 = expr1->getType(expr1, ctx->functionTable, ctx->program, ctx->currentFunction);
    UnionType unionType2 = expr2->getType(expr2, ctx->functionTable, ctx->program, ctx->currentFunction);
    bool canBeFloat = unionType1.isFloat || unionType2.isFloat || unionType1.isString || unionType2.isString;
    bool isType1Float = !unionType1.isBool && unionType1.isFloat && !unionType1.isInt && !unionType1.isNull && !unionType1.isString;
    bool isType2Float = !unionType2.isBool && unionType2.isFloat && !unionType2.isInt && !unionType2.isNull && !unionType2.isString;
    bool isGuaranteedFloat = isType1Float || isType2Float;
    if(isGuaranteedFloat) {
        *out1 = generateCastToFloat(symb1, expr1, ctx, NULL);
        *out2 = generateCastToFloat(symb2, expr2, ctx, NULL);
    } else if(canBeFloat) {
        Symb type1 = generateSymbType(expr1, symb1, *ctx);
        Symb type2 = generateSymbType(expr2, symb2, *ctx);
        size_t castUID = getNextCodeGenUID();
        StringBuilder castEnd;
        StringBuilder__init(&castEnd);
        StringBuilder__appendString(&castEnd, "cast_end&");
        StringBuilder__appendInt(&castEnd, castUID);
        Var result1 = generateTemporaryVariable(*ctx);
        *out1 = (Symb){.type=Type_variable, .value.v=result1};
        Var result2 = generateTemporaryVariable(*ctx);
        *out2 = (Symb){.type=Type_variable, .value.v=result2};
        StringBuilder isFloat;
        StringBuilder__init(&isFloat);
        StringBuilder__appendString(&isFloat, "is_float&");
        StringBuilder__appendInt(&isFloat, castUID);
        emit_JUMPIFEQ(isFloat.text, type1, (Symb){.type = Type_string, .value.s = "float"});
        emit_JUMPIFEQ(isFloat.text, type2, (Symb){.type = Type_string, .value.s = "float"});
        emit_MOVE(result1, generateCastToInt(symb1, expr1, ctx, &type1));
        emit_MOVE(result2, generateCastToInt(symb2, expr2, ctx, &type2));
        emit_JUMP(castEnd.text);
        emit_LABEL(isFloat.text);
        emit_MOVE(result1, generateCastToFloat(symb1, expr1, ctx, &type1));
        emit_MOVE(result2, generateCastToFloat(symb2, expr2, ctx, &type2));
        emit_LABEL(castEnd.text);
        StringBuilder__free(&isFloat);
        StringBuilder__free(&castEnd);
        freeTemporarySymbol(type1, *ctx);
        freeTemporarySymbol(type2, *ctx);
    } else {
        *out1 = generateCastToInt(symb1, expr1, ctx, NULL);
        *out2 = generateCastToInt(symb2, expr2, ctx, NULL);
    }
}

/**
 * @brief Generates binary operator code
 * 
 * @param expression Expression to generate
 * @param ctx Context
 * @return Symb
 */
Symb generateBinaryOperator(Expression__BinaryOperator * expression, Context ctx, bool throwaway, Var * outVarAlt) {
    if(expression->operator == TOKEN_ASSIGN) {
        if(expression->lSide->expressionType != EXPRESSION_VARIABLE) {
            fprintf(stderr, "Assigment to something else than variable found, but it should be checked elsewhere???\n");
            exit(1);
        }
        Expression__Variable * var = (Expression__Variable*)expression->lSide;
        char * varId = var->name;
        Var left = (Var) {.frameType = ((VariableInfo*)table_find(ctx.varTable, var->name)->data)->isGlobal ? GF : LF, .name = varId};;
        Symb right;
        if(throwaway || outVarAlt != NULL) {
            right = generateExpression(expression->rSide, ctx, false, &left);
        } else {
            right = generateExpression(expression->rSide, ctx, false, NULL);
        }
        if(!throwaway) {
            right = saveTempSymb(right, ctx);
        } else {
            freeTemporarySymbol(right, ctx);
        }
        if((!throwaway && outVarAlt == NULL) || right.type != Type_variable || right.value.v.frameType != left.frameType || strcmp(right.value.v.name, left.name) != 0) {
            emit_MOVE(left, right);
        }
        return right;
    }
    Symb left = generateExpression(expression->lSide, ctx, false, NULL);
    left = saveTempSymb(left, ctx);
    Symb right;
    if(expression->operator != TOKEN_AND && expression->operator != TOKEN_OR) {
        right = generateExpression(expression->rSide, ctx, false, NULL);
    }
    Var outVar;
    if(outVarAlt == NULL) {
        outVar = generateTemporaryVariable(ctx);
    } else {
        outVar = *outVarAlt;
    }
    Symb outSymb = (Symb){.type=Type_variable, .value.v = outVar};
    switch(expression->operator) {
        case TOKEN_PLUS:
            emitAddMulSubCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_ADD(outVar, left, right);
            break;
        case TOKEN_MINUS:
            emitAddMulSubCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_SUB(outVar, left, right);
            break;
        case TOKEN_CONCATENATE:
            left = generateCastToString(left, expression->lSide, &ctx, NULL);
            right = generateCastToString(right, expression->rSide, &ctx, NULL);
            emit_CONCAT(outVar, left, right);
            break;
        case TOKEN_MULTIPLY:
            emitAddMulSubCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_MUL(outVar, left, right);
            break;
        case TOKEN_DIVIDE: {
            Symb left2 = generateCastToFloat(left, expression->lSide, &ctx, NULL);
            Symb right2 = generateCastToFloat(right, expression->rSide, &ctx, NULL);
            emit_DIV(outVar, left2, right2);
            break;
        }
        case TOKEN_EQUALS: {
            Type typeL = unionTypeToType(expression->lSide->getType(expression->lSide, ctx.functionTable, ctx.program, ctx.currentFunction));
            Type typeR = unionTypeToType(expression->rSide->getType(expression->rSide, ctx.functionTable, ctx.program, ctx.currentFunction));
            if(typeL.type != TYPE_UNKNOWN && typeR.type != TYPE_UNKNOWN) {
                if(typeL.type == typeR.type || (typeL.type == TYPE_NULL && !typeR.isRequired) || (typeR.type == TYPE_NULL && !typeL.isRequired)) {
                    emit_EQ(outVar, left, right);
                } else {
                    outSymb = (Symb){.type = Type_bool, .value.b = false};
                }
            } else {
                Symb typeOut1 = generateSymbType(expression->lSide, left, ctx);
                Symb typeOut2 = generateSymbType(expression->rSide, right, ctx);
                size_t operatorTypeCheckId = getNextCodeGenUID();
                StringBuilder sb3;
                StringBuilder__init(&sb3);
                StringBuilder__appendString(&sb3, "type_check_ok&");
                StringBuilder__appendInt(&sb3, operatorTypeCheckId);
                emit_JUMPIFEQ(sb3.text, typeOut1, typeOut2);
                freeTemporarySymbol(typeOut1, ctx);
                freeTemporarySymbol(typeOut2, ctx);
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
            Type typeL = unionTypeToType(expression->lSide->getType(expression->lSide, ctx.functionTable, ctx.program, ctx.currentFunction));
            Type typeR = unionTypeToType(expression->rSide->getType(expression->rSide, ctx.functionTable, ctx.program, ctx.currentFunction));
            if(typeL.type != TYPE_UNKNOWN && typeR.type != TYPE_UNKNOWN) {
                if(typeL.type == typeR.type || (typeL.type == TYPE_NULL && !typeR.isRequired) || (typeR.type == TYPE_NULL && !typeL.isRequired)) {
                    emit_EQ(outVar, left, right);
                    emit_NOT(outVar, outSymb);
                } else {
                    outSymb = (Symb){.type = Type_bool, .value.b = true};
                }
            } else {
                Symb typeOut1 = generateSymbType(expression->lSide, left, ctx);
                Symb typeOut2 = generateSymbType(expression->rSide, right, ctx);
                size_t operatorTypeCheckId = getNextCodeGenUID();
                StringBuilder sb3;
                StringBuilder__init(&sb3);
                StringBuilder__appendString(&sb3, "type_check_ok&");
                StringBuilder__appendInt(&sb3, operatorTypeCheckId);
                emit_JUMPIFEQ(sb3.text, typeOut1, typeOut2);
                freeTemporarySymbol(typeOut1, ctx);
                freeTemporarySymbol(typeOut2, ctx);
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
        case TOKEN_AND: {
            size_t operatorAndId = getNextCodeGenUID();
            StringBuilder sb1;
            StringBuilder__init(&sb1);
            StringBuilder__appendString(&sb1, "and_is_false&");
            StringBuilder__appendInt(&sb1, operatorAndId);
            StringBuilder sb2;
            StringBuilder__init(&sb2);
            StringBuilder__appendString(&sb2, "operator_and_done&");
            StringBuilder__appendInt(&sb2, operatorAndId);
            Symb leftBool = generateCastToBool(expression->lSide, left, ctx, false);
            emit_JUMPIFEQ(sb1.text, leftBool, (Symb){.type=Type_bool, .value.b=false});
            freeTemporarySymbol(leftBool, ctx);
            right = generateExpression(expression->rSide, ctx, false, NULL);
            Symb rightBool = generateCastToBool(expression->rSide, right, ctx, false);
            emit_MOVE(outVar, rightBool);
            freeTemporarySymbol(rightBool, ctx);
            emit_JUMP(sb2.text);
            emit_LABEL(sb1.text);
            emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=false});
            emit_LABEL(sb2.text);
            break;
        }
        case TOKEN_OR: {
            size_t operatorOrId = getNextCodeGenUID();
            StringBuilder sb1;
            StringBuilder__init(&sb1);
            StringBuilder__appendString(&sb1, "or_is_true&");
            StringBuilder__appendInt(&sb1, operatorOrId);
            StringBuilder sb2;
            StringBuilder__init(&sb2);
            StringBuilder__appendString(&sb2, "operator_or_done&");
            StringBuilder__appendInt(&sb2, operatorOrId);
            Symb leftBool = generateCastToBool(expression->lSide, left, ctx, false);
            emit_JUMPIFEQ(sb1.text, leftBool, (Symb){.type=Type_bool, .value.b=true});
            freeTemporarySymbol(leftBool, ctx);
            right = generateExpression(expression->rSide, ctx, false, NULL);
            Symb rightBool = generateCastToBool(expression->rSide, right, ctx, false);
            emit_MOVE(outVar, rightBool);
            freeTemporarySymbol(rightBool, ctx);
            emit_JUMP(sb2.text);
            emit_LABEL(sb1.text);
            emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=true});
            emit_LABEL(sb2.text);
            break;
        }
        default:
            fprintf(stderr, "Unknown operator found while generating output code\n");
            exit(99);
    }
    freeTemporarySymbol(left, ctx);
    freeTemporarySymbol(right, ctx);
    return outSymb;
}

/**
 * @brief Generates unary operator code
 * 
 * @param expression Expression to generate
 * @param ctx Context
 * @return Symb
 */
Symb generateUnaryOperator(Expression__UnaryOperator * expression, Context ctx, bool throwaway, Var * outVarAlt) {
    Symb right = generateExpression(expression->rSide, ctx, false, NULL);
    Var outVar;
    if(outVarAlt == NULL) {
        outVar = generateTemporaryVariable(ctx);
    } else {
        outVar = *outVarAlt;
    }
    Symb outSymb = (Symb){.type=Type_variable, .value.v = outVar};
    switch(expression->operator) {
        case TOKEN_NEGATE: {
            Symb rightBool = generateCastToBool(expression->rSide, right, ctx, false);
            emit_NOT(outVar, rightBool);
            freeTemporarySymbol(rightBool, ctx);
            break;
        }
        default:
            fprintf(stderr, "Unknown operator found while generating output code\n");
            exit(99);
    }
    freeTemporarySymbol(right, ctx);
    return outSymb;
}

/**
 * @brief Generates code for expression
 * 
 * @param expression 
 * @param ctx 
 * @param away 
 * @param outVar 
 * @return Symb 
 */
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
        case EXPRESSION_UNARY_OPERATOR:
            return generateUnaryOperator((Expression__UnaryOperator*)expression, ctx, throwaway, outVar);
            break;
        default:
            fprintf(stderr, "Unknown expression type found while generating output code\n");
            exit(99);
    }
}

/**
 * @brief Generates code for statement list
 * 
 * @param statement 
 * @param ctx 
 */
void generateStatementList(StatementList* statementList, Context ctx) {
    for(int i = 0; i < statementList->listSize; i++) {
        generateStatement(statementList->statements[i], ctx);
    }
}

/**
 * @brief Generates code for if statement
 * 
 * @param statement 
 * @param ctx 
 */
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
    condition = generateCastToBool(statement->condition, condition, ctx, true);
    emit_JUMPIFNEQ(ifElseSb.text, condition, (Symb){.type=Type_bool, .value.b = true});
    freeTemporarySymbol(condition, ctx);
    generateStatement(statement->ifBody, ctx);
    if(!isElseEmpty) emit_JUMP(ifEndSb.text);
    emit_LABEL(ifElseSb.text);
    if(!isElseEmpty) generateStatement(statement->elseBody, ctx);
    if(!isElseEmpty) emit_LABEL(ifEndSb.text);
    if(isElseEmpty) emit_COMMENT("Else body is empty");

    StringBuilder__free(&ifElseSb);
    StringBuilder__free(&ifEndSb);
}

/**
 * @brief Generates code for while statement
 * 
 * @param statement 
 * @param ctx 
 */
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
    condition = generateCastToBool(statement->condition, condition, ctx, true);
    emit_JUMPIFNEQ(whileEndSb.text, condition, (Symb){.type=Type_bool, .value.b = true});
    freeTemporarySymbol(condition, ctx);
    generateStatement(statement->body, ctx);
    emit_JUMP(whileStartSb.text);
    emit_LABEL(whileEndSb.text);

    StringBuilder__free(&whileStartSb);
    StringBuilder__free(&whileEndSb);
}

/**
 * @brief Generates code for return statement
 * 
 * @param statement 
 * @param ctx 
 */
void generateReturn(StatementReturn * statement, Context ctx) {
    if(statement != NULL && statement->expression != NULL) {
        Var returnValue = (Var){.frameType = LF, .name = "returnValue"};
        Symb expr = generateExpression(statement->expression, ctx, ctx.isGlobal, ctx.isGlobal ? NULL : &returnValue);
        if(!ctx.isGlobal) {
            Type functionType = ctx.currentFunction->returnType;
            UnionType returnUnionType = statement->expression->getType(statement->expression, ctx.functionTable, ctx.program, ctx.currentFunction);
            Type returnType = unionTypeToType(returnUnionType);
            if(returnType.type != functionType.type || (returnType.isRequired != functionType.isRequired && functionType.isRequired) ) {
                StringBuilder typeCheckFailMsg;
                StringBuilder__init(&typeCheckFailMsg);
                StringBuilder__appendString(&typeCheckFailMsg, "Type check failed for return statement of function ");
                StringBuilder__appendString(&typeCheckFailMsg, ctx.currentFunction->name);
                StringBuilder__appendString(&typeCheckFailMsg, "\n");
                emitTypeCheck(functionType, statement->expression, expr, ctx, typeCheckFailMsg.text);
                StringBuilder__free(&typeCheckFailMsg);
            }
        }
        if(!ctx.isGlobal && (expr.type != Type_variable || expr.value.v.frameType != LF || strcmp(expr.value.v.name, "returnValue") != 0)) {
            emit_MOVE(returnValue, expr);
        }
    }
    if(!ctx.isGlobal) {
        emit_POPFRAME();
        emit_RETURN();
    } else {
        emit_EXIT((Symb){.type=Type_int, .value.i=0});
    }
}

/**
 * @brief Generates code for statement
 * 
 * @param statement 
 * @param ctx 
 */
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
        case STATEMENT_EXIT:
            emit_EXIT((Symb){.type=Type_int, .value.i=((StatementExit*)statement)->exitCode});
            break;
        case STATEMENT_FUNCTION:
            fprintf(stderr, "OFF, ignoring function...\n");
            break;
    }
}

/**
 * @brief Generates code for function
 * 
 * @param function 
 * @param ctx 
 */
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
    if(function->returnType.type != TYPE_VOID) {
        emit_DEFVAR((Var){frameType: TF, name: "returnValue"});
    }
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
                variableInfo->isUsed = true;
                variableInfo->isTemporary = false;
                table_insert(localTable, variable->name, variableInfo);
                bool isParameter = false;
                for(int j=0; j<function->arity; j++) {
                    if(strcmp(function->parameterNames[j], variable->name) == 0) {
                        isParameter = true;
                        break;
                    }
                }
                if(!isParameter) {
                    emit_DEFVAR((Var){.frameType=TF, .name=variable->name});
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
    ctx.currentFunction = function;
    ctx.program = NULL;
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


void performPreoptimizationChecksOnStatement(Statement * statement, Table * functionTable) {
    if(statement->statementType == STATEMENT_EXPRESSION) {
    Expression* expression = (Expression*) statement;
    if(expression->expressionType == EXPRESSION_BINARY_OPERATOR) {
        if(((Expression__BinaryOperator*)expression)->operator == TOKEN_ASSIGN) {
            Expression* left = ((Expression__BinaryOperator*)expression)->lSide;
            if(left->expressionType != EXPRESSION_VARIABLE) {
                fprintf(stderr, "Left side of assigment needs to be variable\n");
                exit(2);
            }
        }
    } else if(expression->expressionType == EXPRESSION_FUNCTION_CALL) {
        Expression__FunctionCall* functionCall = (Expression__FunctionCall*) expression;
        TableItem* calledFunction = table_find(functionTable, functionCall->name);
        if(calledFunction == NULL) {
            fprintf(stderr, "Function %s not defined\n", functionCall->name);
            exit(3);
        }
    }
}
}

void performPreoptimizationChecks(StatementList * program, Table * functionTable) {
    size_t statementCount = 0;
    Statement *** statements = getAllStatements((Statement*)program, &statementCount);
    for(int i=0; i<statementCount; i++) {
        if(*statements[i] == NULL) continue;
        performPreoptimizationChecksOnStatement(*statements[i], functionTable);
    }
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem* item = functionTable->tb[i];
        while(item != NULL) {
            Function* function = (Function*) item->data;
            size_t statementCount = 0;
            Statement *** statements = getAllStatements(function->body, &statementCount);
            for(int j=0; j < statementCount; j++) {
                if(statements[j] == NULL || *statements[j] == NULL) continue;
                Statement* statement = *statements[j];
                if(statement == NULL) continue;
                performPreoptimizationChecksOnStatement(statement, functionTable);
                if(statement->statementType == STATEMENT_RETURN) {
                    StatementReturn* returnStatement = (StatementReturn*) statement;
                    if(returnStatement->expression != NULL) {
                        if(function->returnType.type == TYPE_VOID) {
                            fprintf(stderr, "Return value in function %s with void return type\n", function->name);
                            exit(6);
                        }
                    } else {
                        if(function->returnType.type != TYPE_VOID) {
                            fprintf(stderr, "Missing return value in function %s with non-void return type\n", function->name);
                            exit(6);
                        }
                    }
                }
            }
            free(statements);
            item = item->next;
        }
    }
}

/**
 * @brief Generates code for program
 * 
 * @param program 
 * @param functionTable 
 */
void generateCode(StatementList * program, Table * functionTable) {
    performPreoptimizationChecks(program, functionTable);
    StatementExit * exit = StatementExit__init();
    exit->exitCode = 0;
    StatementList__addStatement(program, (Statement*)exit);
    optimize(program, functionTable);
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
                variableInfo->isUsed = true;
                variableInfo->isTemporary = false;
                table_insert(globalTable, variable->name, variableInfo);
                emit_DEFVAR((Var){.frameType=GF, .name=variable->name});
            }
        }
    }
    free(allStatements);
    emit_instruction_start();
    Context ctx;
    ctx.varTable = globalTable;
    ctx.functionTable = functionTable;
    ctx.isGlobal = true;
    ctx.currentFunction = NULL;
    ctx.program = program;
    generateStatementList(program, ctx);
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