// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)

#include "code_generator.h"
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
 * @brief Creates a label
 * 
 * @param label 
 * @param uid
 * @return char* 
 */
char* create_label(const char* label, size_t uid) {
    StringBuilder sb;
    StringBuilder__init(&sb);
    StringBuilder__appendString(&sb, label);
    StringBuilder__appendInt(&sb, uid);
    return sb.text;
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
    PointerTable * resultTable;
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
    UnionType type = statement->super.getType((Expression *) statement, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable);
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
    UnionType type = statement->super.getType((Expression *) statement, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable);
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
 * @brief Generate symbol type value
 * 
 * @param expression
 * @param symbol
 * @param context
 * @return Symb
 */
Symb generateSymbType(Expression * expression, Symb symb, Context ctx) {
    Type type = unionTypeToType(expression->getType(expression, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
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
    UnionType unionType = expression->getType(expression, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_BOOL && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        if(!isCondtion) {
            Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_BOOL, .isRequired=true}, false);
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
    char* castEnd = create_label("cast_end&", castUID);
    Var result = generateTemporaryVariable(ctx);
    if(unionType.isBool) {
        char* notBool = create_label("not_bool&", castUID);
        emit_JUMPIFNEQ(notBool, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_MOVE(result, symb);
        emit_JUMP(castEnd);
        emit_LABEL(notBool);
        free(notBool);
    }
    if(unionType.isNull) {
        char* notNil = create_label("not_nil&", castUID);
        emit_JUMPIFNEQ(notNil, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMP(castEnd);
        emit_LABEL(notNil);
        free(notNil);
    }
    if(unionType.isInt) {
        char* notInt = create_label("not_int&", castUID);
        emit_JUMPIFNEQ(notInt, symbType, (Symb){.type = Type_string, .value.s = "int"});
        emit_PUSHS(symb);
        emit_PUSHS((Symb){.type = Type_int, .value.i = 0});
        emit_EQS();
        emit_NOTS();
        emit_POPS(result);
        emit_JUMP(castEnd);
        emit_LABEL(notInt);
        free(notInt);
    }
    if(unionType.isFloat) {
        char* notFloat = create_label("not_float&", castUID);
        emit_JUMPIFNEQ(notFloat, symbType, (Symb){.type = Type_string, .value.s = "float"});
        emit_PUSHS(symb);
        emit_PUSHS((Symb){.type = Type_float, .value.f = 0.0});
        emit_EQS();
        emit_NOTS();
        emit_POPS(result);
        emit_JUMP(castEnd);
        emit_LABEL(notFloat);
        free(notFloat);
    }
    if(unionType.isString) {
        char* notString = create_label("not_string&", castUID);
        emit_JUMPIFNEQ(notString, symbType, (Symb){.type = Type_string, .value.s = "string"});
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
        emit_JUMP(castEnd);
        emit_LABEL(notString);
        free(notString);
    }
    emit_LABEL(castEnd);
    free(castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}

Symb generateCastToInt(Symb symb, Expression * expression, Context * ctx, Symb * typeSymb, bool isBuiltin) {
    UnionType unionType = expression->getType(expression, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_INT && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_INT, .isRequired=true}, isBuiltin);
        if(constant != NULL) {
            return generateConstant(constant);
        } 
    }
    Symb symbType;
    if(typeSymb == NULL) {
        symbType = generateSymbType(expression, symb, *ctx);
    } else {
        symbType = *typeSymb;
    }
    size_t castUID = getNextCodeGenUID();
    char* castEnd = create_label("cast_end&", castUID);
    Var result = generateTemporaryVariable(*ctx);
    if(unionType.isBool) {
        char* notBool = create_label("not_bool&", castUID);
        char* isTrue = create_label("is_true&", castUID);
        emit_JUMPIFNEQ(notBool, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_JUMPIFNEQ(isTrue, symb, (Symb){.type = Type_bool, .value.b = 0});
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMP(castEnd);
        emit_LABEL(isTrue);
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMP(castEnd);
        emit_LABEL(notBool);
        free(notBool);
        free(isTrue);
    }
    if(unionType.isNull) {
        char* notNil = create_label("not_nil&", castUID);
        emit_JUMPIFNEQ(notNil, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMP(castEnd);
        emit_LABEL(notNil);
        free(notNil);
    }
    if(unionType.isInt) {
        char* notInt = create_label("not_int&", castUID);
        emit_JUMPIFNEQ(notInt, symbType, (Symb){.type = Type_string, .value.s = "int"});
        emit_MOVE(result, symb);
        emit_JUMP(castEnd);
        emit_LABEL(notInt);
        free(notInt);
    }
    if(unionType.isFloat) {
        char* notFloat = create_label("not_float&", castUID);
        emit_JUMPIFNEQ(notFloat, symbType, (Symb){.type = Type_string, .value.s = "float"});
        emit_FLOAT2INT(result, symb);
        emit_LABEL(notFloat);
        free(notFloat);
    }
    if(unionType.isString) {
        Var temp_value = generateTemporaryVariable(*ctx);
        Var index = generateTemporaryVariable(*ctx);
        Var length = generateTemporaryVariable(*ctx);
        Var is_invalid = generateTemporaryVariable(*ctx);
        Var is_builtin = generateTemporaryVariable(*ctx);
        char* notString = create_label("not_string&", castUID);
        char* intval_loop = create_label("intval_loop&", castUID);
        char* empty_loop = create_label("empty_loop&", castUID);
        char* throw_error = create_label("throw_error&", castUID);
        char* skip_throw_error = create_label("skip_throw_error&", castUID);
        char* skip_beginning = create_label("skip_beginning&", castUID);
        emit_JUMPIFNEQ(notString, symbType, (Symb){.type = Type_string, .value.s = "string"});
        emit_MOVE(index, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 0});
        if(isBuiltin) {
            emit_MOVE(is_builtin, (Symb){.type = Type_bool, .value.b = true});
        } else {
            emit_MOVE(is_builtin, (Symb){.type = Type_bool, .value.b = false});
        }
        emit_STRLEN(length, symb);
        emit_JUMPIFNEQ(empty_loop, (Symb){.type = Type_variable, .value.v = length}, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMPIFEQ(empty_loop, (Symb){.type = Type_variable, .value.v = is_builtin}, (Symb){.type = Type_bool, .value.b = true});
        emit_EXIT((Symb){.type = Type_int, .value.i = 7});
        // loop for going through empty chars
        emit_LABEL(empty_loop);
        emit_STRI2INT(temp_value, symb, (Symb){.type = Type_variable, .value.v = index});
        emit_ADD(index, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFEQ(empty_loop, (Symb){.type = Type_variable, .value.v = temp_value}, (Symb){.type = Type_int, .value.i = 32});
        emit_JUMP(skip_beginning);
        // loop for calculating int value
        emit_LABEL(intval_loop);
        emit_STRI2INT(temp_value, symb, (Symb){.type = Type_variable, .value.v = index});
        emit_ADD(index, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_LABEL(skip_beginning);
        // check if char < '0'
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 48});
        emit_LTS();
        // check if char > '9'
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 57});
        emit_GTS();
        emit_ORS();
        emit_POPS(is_invalid);
        // if char != digit then break
        emit_JUMPIFEQ(throw_error, (Symb){.type = Type_variable, .value.v = is_invalid}, (Symb){.type = Type_bool, .value.b = true});
        // get the actual number from char
        emit_SUB(temp_value, (Symb){.type = Type_variable, .value.v = temp_value}, (Symb){.type = Type_int, .value.i = 48});
        // multiply result by 10
        emit_MUL(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_int, .value.i = 10});
        // add current digit into result
        emit_ADD(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_variable, .value.v = temp_value});
        emit_JUMPIFNEQ(intval_loop, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_variable, .value.v = length});
        emit_LABEL(throw_error);
        emit_JUMPIFEQ(skip_throw_error, (Symb){.type = Type_variable, .value.v = is_builtin}, (Symb){.type = Type_bool, .value.b = true});
        emit_JUMPIFEQ(skip_throw_error, (Symb){.type = Type_variable, .value.v = is_invalid}, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMPIFNEQ(skip_throw_error, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_EXIT((Symb){.type = Type_int, .value.i = 7});
        emit_LABEL(skip_throw_error);
        emit_JUMP(castEnd);
        emit_LABEL(notString);
        free(notString);
        free(intval_loop);
        free(empty_loop);
        free(throw_error);
        free(skip_throw_error);
        free(skip_beginning);
    }
    emit_LABEL(castEnd);
    free(castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}

Symb generateCastToFloat(Symb symb, Expression * expression, Context * ctx, Symb * typeSymb, bool isBuiltin) {
    UnionType unionType = expression->getType(expression, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_FLOAT && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_FLOAT, .isRequired=true}, isBuiltin);
        if(constant != NULL) {
            return generateConstant(constant);
        } 
    }
    Symb symbType;
    if(typeSymb == NULL) {
        symbType = generateSymbType(expression, symb, *ctx);
    } else {
        symbType = *typeSymb;
    }
    size_t castUID = getNextCodeGenUID();
    char* castEnd = create_label("cast_end&", castUID);
    Var result = generateTemporaryVariable(*ctx);
    if(unionType.isBool) {
        char* notBool = create_label("not_bool&", castUID);
        char* isTrue = create_label("is_true&", castUID);
        emit_JUMPIFNEQ(notBool, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_JUMPIFNEQ(isTrue, symb, (Symb){.type = Type_bool, .value.b = 0});
        emit_MOVE(result, (Symb){.type = Type_float, .value.f = 0});
        emit_JUMP(castEnd);
        emit_LABEL(isTrue);
        emit_MOVE(result, (Symb){.type = Type_float, .value.f = 1});
        emit_JUMP(castEnd);
        emit_LABEL(notBool);
        free(notBool);
        free(isTrue);
    }
    if(unionType.isNull) {
        char* notNil = create_label("not_nul&", castUID);
        emit_JUMPIFNEQ(notNil, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type = Type_float, .value.f = 0});
        emit_JUMP(castEnd);
        emit_LABEL(notNil);
        free(notNil);
    }
    if(unionType.isInt) {
        char* notInt = create_label("not_int&", castUID);
        emit_JUMPIFNEQ(notInt, symbType, (Symb){.type = Type_string, .value.s = "int"});
        emit_INT2FLOAT(result, symb);
        emit_JUMP(castEnd);
        emit_LABEL(notInt);
        free(notInt);
    }
    if(unionType.isFloat) {
        char* notFloat = create_label("not_float&", castUID);
        emit_JUMPIFNEQ(notFloat, symbType, (Symb){.type = Type_string, .value.s = "float"});
        emit_MOVE(result, symb);
        emit_LABEL(notFloat);
        free(notFloat);
    }
    if(unionType.isString) {
        Var temp_value = generateTemporaryVariable(*ctx);
        Var index = generateTemporaryVariable(*ctx);
        Var length = generateTemporaryVariable(*ctx);
        Var has_decimal = generateTemporaryVariable(*ctx);
        Var has_exponent = generateTemporaryVariable(*ctx);
        Var has_operator = generateTemporaryVariable(*ctx);
        Var is_invalid = generateTemporaryVariable(*ctx);
        Var decimal_places_counter = generateTemporaryVariable(*ctx);
        Var exponent_parameter = generateTemporaryVariable(*ctx);
        Var exponent_operator = generateTemporaryVariable(*ctx);
        Var divider = generateTemporaryVariable(*ctx);
        Var exponent_divider = generateTemporaryVariable(*ctx);
        Var is_builtin = generateTemporaryVariable(*ctx);
        char* not_string = create_label("not_string&", castUID);
        char* floatval_loop = create_label("floatval_loop&", castUID);
        char* empty_loop = create_label("empty_loop&", castUID);
        char* floatval_loop_end = create_label("floatval_loop_end&", castUID);
        char* skip_decimal_check = create_label("skip_decimal_check&", castUID);
        char* skip_decimal_counter = create_label("skip_decimal_counter&", castUID);
        char* divider_loop = create_label("divider_loop&", castUID);
        char* skip_divider_loop = create_label("skip_divider_loop&", castUID);
        char* skip_exponent_check = create_label("skip_exponent_check&", castUID);
        char* skip_exponent_operator_check = create_label("skip_exponent_operator_check&", castUID);
        char* skip_exponent_parameter = create_label("skip_exponent_parameter&", castUID);
        char* exponent_loop = create_label("exponent_loop&", castUID);
        char* skip_exponent_loop = create_label("skip_exponent_loop&", castUID);
        char* operator_is_minus = create_label("operator_is_minus&", castUID);
        char* operator_is_plus = create_label("operator_is_plus&", castUID);
        char* skip_fix_no_operator = create_label("skip_fix_no_operator&", castUID);
        char* skip_throw_error = create_label("skip_throw_error&", castUID);
        char* skip_beginning = create_label("skip_beginning&", castUID);
        emit_JUMPIFNEQ(not_string, symbType, (Symb){.type = Type_string, .value.s = "string"});
        if(isBuiltin) {
            emit_MOVE(is_builtin, (Symb){.type = Type_bool, .value.b = true});
        } else {
            emit_MOVE(is_builtin, (Symb){.type = Type_bool, .value.b = false});
        }
        // initialization of values
        emit_MOVE(index, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(has_decimal, (Symb){.type = Type_bool, .value.b = false});
        emit_MOVE(has_exponent, (Symb){.type = Type_bool, .value.b = false});
        emit_MOVE(has_operator, (Symb){.type = Type_bool, .value.b = false});
        emit_MOVE(decimal_places_counter, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(exponent_parameter, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(exponent_operator, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(divider, (Symb){.type = Type_float, .value.f = 1});
        emit_MOVE(exponent_divider, (Symb){.type = Type_float, .value.f = 1});
        emit_MOVE(result, (Symb){.type = Type_int, .value.i = 0});
        // get length of string
        emit_STRLEN(length, symb);
        emit_JUMPIFNEQ(empty_loop, (Symb){.type = Type_variable, .value.v = length}, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMPIFEQ(empty_loop, (Symb){.type = Type_variable, .value.v = is_builtin}, (Symb){.type = Type_bool, .value.b = true});
        emit_EXIT((Symb){.type = Type_int, .value.i = 7});
        // loop for going through empty chars
        emit_LABEL(empty_loop);
        emit_STRI2INT(temp_value, symb, (Symb){.type = Type_variable, .value.v = index});
        emit_ADD(index, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFEQ(empty_loop, (Symb){.type = Type_variable, .value.v = temp_value}, (Symb){.type = Type_int, .value.i = 32});
        emit_JUMP(skip_beginning);
        // floatval loop
        emit_LABEL(floatval_loop);
        emit_STRI2INT(temp_value, symb, (Symb){.type = Type_variable, .value.v = index});                                                   
        emit_ADD(index, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});                           
        emit_LABEL(skip_beginning);
        // check if temp_val is decimal dot
        emit_JUMPIFEQ(skip_decimal_check, (Symb){.type = Type_variable, .value.v = has_decimal}, (Symb){.type = Type_bool, .value.b = true});
        emit_EQ(has_decimal, (Symb){.type = Type_variable, .value.v = temp_value}, (Symb){.type = Type_int, .value.i = 46});
        emit_JUMPIFEQ(floatval_loop, (Symb){.type = Type_variable, .value.v = has_decimal}, (Symb){.type = Type_bool, .value.b = true});
        emit_LABEL(skip_decimal_check);
        // check if char is exponent
        emit_JUMPIFEQ(skip_exponent_check, (Symb){.type = Type_variable, .value.v = has_exponent}, (Symb){.type = Type_bool, .value.b = true});
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 69});
        emit_EQS();
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 101});
        emit_EQS();
        emit_ORS();
        emit_POPS(has_exponent);
        emit_JUMPIFEQ(floatval_loop, (Symb){.type = Type_variable, .value.v = has_exponent}, (Symb){.type = Type_bool, .value.b = true});
        emit_LABEL(skip_exponent_check);
         // check if char is exponent operator
        emit_JUMPIFEQ(skip_exponent_operator_check, (Symb){.type = Type_variable, .value.v = has_operator}, (Symb){.type = Type_bool, .value.b = true});
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 43});
        emit_EQS();
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 45});
        emit_EQS();
        emit_ORS();
        emit_POPS(has_operator);
        emit_MOVE(exponent_operator, (Symb){.type = Type_variable, .value.v = temp_value});
        emit_JUMPIFEQ(floatval_loop, (Symb){.type = Type_variable, .value.v = has_operator}, (Symb){.type = Type_bool, .value.b = true});
        emit_LABEL(skip_exponent_operator_check);
        // check if char < '0'
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 48});
        emit_LTS();
        // check if char > '9'
        emit_PUSHS((Symb){.type = Type_variable, .value.v = temp_value});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 57});
        emit_GTS();
        emit_ORS();
        emit_POPS(is_invalid);
        // get exponent_parameter if current char isn't invalid and has_exponent is true
        emit_JUMPIFEQ(skip_exponent_parameter, (Symb){.type = Type_variable, .value.v = is_invalid}, (Symb){.type = Type_bool, .value.b = true});
        emit_JUMPIFEQ(skip_exponent_parameter, (Symb){.type = Type_variable, .value.v = has_exponent}, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMPIFEQ(skip_exponent_parameter, (Symb){.type = Type_variable, .value.v = has_operator}, (Symb){.type = Type_bool, .value.b = false});
        emit_SUB(temp_value, (Symb){.type = Type_variable, .value.v = temp_value}, (Symb){.type = Type_int, .value.i = 48});
        emit_MUL(exponent_parameter, (Symb){.type = Type_variable, .value.v = exponent_parameter}, (Symb){.type = Type_int, .value.i = 10});
        emit_ADD(exponent_parameter, (Symb){.type = Type_variable, .value.v = exponent_parameter}, (Symb){.type = Type_variable, .value.v = temp_value});
        emit_JUMPIFNEQ(floatval_loop, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_variable, .value.v = length});
        emit_JUMP(floatval_loop_end);
        emit_LABEL(skip_exponent_parameter);
        // increment decimal_places_counter
        emit_JUMPIFEQ(skip_decimal_counter, (Symb){.type = Type_variable, .value.v = is_invalid}, (Symb){.type = Type_bool, .value.b = true});
        emit_JUMPIFEQ(skip_decimal_counter, (Symb){.type = Type_variable, .value.v = has_decimal}, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMPIFEQ(skip_decimal_counter, (Symb){.type = Type_variable, .value.v = has_exponent}, (Symb){.type = Type_bool, .value.b = true});
        emit_ADD(decimal_places_counter, (Symb){.type = Type_variable, .value.v = decimal_places_counter}, (Symb){.type = Type_int, .value.i = 1});
        emit_LABEL(skip_decimal_counter);
        // if char != digit then break
        emit_JUMPIFEQ(floatval_loop_end, (Symb){.type = Type_variable, .value.v = is_invalid}, (Symb){.type = Type_bool, .value.b = true});
        emit_SUB(temp_value, (Symb){.type = Type_variable, .value.v = temp_value}, (Symb){.type = Type_int, .value.i = 48});
        emit_MUL(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_int, .value.i = 10});
        emit_ADD(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_variable, .value.v = temp_value});
        // go to the next iteration if index < length
        emit_JUMPIFNEQ(floatval_loop, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_variable, .value.v = length});
        // after the loop
        emit_LABEL(floatval_loop_end);
        emit_INT2FLOAT(result, (Symb){.type = Type_variable, .value.v = result});
        // skip calculation of exponent if has_exponent is false
        emit_JUMPIFEQ(skip_exponent_loop, (Symb){.type = Type_variable, .value.v = has_exponent}, (Symb){.type = Type_bool, .value.b = false});
        // skip calculation of exponent if has_operator is false, since if its false it has 0, which means the number is unchanged
        emit_JUMPIFEQ(skip_exponent_loop, (Symb){.type = Type_variable, .value.v = has_operator}, (Symb){.type = Type_bool, .value.b = false});
        // skip exponent loop if exponent_parameter < 1
        emit_PUSHS((Symb){.type = Type_variable, .value.v = exponent_parameter});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 1});
        emit_LTS();
        emit_PUSHS((Symb){.type = Type_bool, .value.b = true});
        emit_JUMPIFEQS(skip_exponent_loop);
        // calculate exponent_parameter loop
        emit_LABEL(exponent_loop);
        // if exponent operator is + then MUL
        emit_JUMPIFEQ(operator_is_minus, (Symb){.type = Type_variable, .value.v = exponent_operator}, (Symb){.type = Type_int, .value.i = 45});
        emit_DIV(exponent_divider, (Symb){.type = Type_variable, .value.v = exponent_divider}, (Symb){.type = Type_float, .value.f = 10});
        emit_JUMP(operator_is_plus);
        emit_LABEL(operator_is_minus);
        emit_MUL(exponent_divider, (Symb){.type = Type_variable, .value.v = exponent_divider}, (Symb){.type = Type_float, .value.f = 10});
        emit_LABEL(operator_is_plus);
        emit_SUB(exponent_parameter, (Symb){.type = Type_variable, .value.v = exponent_parameter}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFNEQ(exponent_loop, (Symb){.type = Type_variable, .value.v = exponent_parameter}, (Symb){.type = Type_int, .value.i = 0});
        emit_LABEL(skip_exponent_loop);
        // divide result by exponent_divider to move the result x decimal spaces (x=exponent_parameter)
        emit_DIV(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_variable, .value.v = exponent_divider});
        // move one more decimal place to the left because there was no exponent operator used
        emit_JUMPIFEQ(skip_fix_no_operator, (Symb){.type = Type_variable, .value.v = has_exponent}, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMPIFEQ(skip_fix_no_operator, (Symb){.type = Type_variable, .value.v = has_operator}, (Symb){.type = Type_bool, .value.b = true});
        emit_ADD(decimal_places_counter, (Symb){.type = Type_variable, .value.v = decimal_places_counter}, (Symb){.type = Type_int, .value.i = 1});
        emit_LABEL(skip_fix_no_operator);
        // skip divider loop if decimal_places_counter < 1
        emit_PUSHS((Symb){.type = Type_variable, .value.v = decimal_places_counter});
        emit_PUSHS((Symb){.type = Type_int, .value.i = 1});
        emit_LTS();
        emit_PUSHS((Symb){.type = Type_bool, .value.b = true});
        emit_JUMPIFEQS(skip_divider_loop);
        // calculate divider loop
        emit_LABEL(divider_loop);
        emit_MUL(divider, (Symb){.type = Type_variable, .value.v = divider}, (Symb){.type = Type_float, .value.f = 10});
        emit_SUB(decimal_places_counter, (Symb){.type = Type_variable, .value.v = decimal_places_counter}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFNEQ(divider_loop, (Symb){.type = Type_variable, .value.v = decimal_places_counter}, (Symb){.type = Type_int, .value.i = 0});
        emit_LABEL(skip_divider_loop);
        // divide result by divider to move the result x decimal spaces (x=decimal_places_counter)
        emit_DIV(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_variable, .value.v = divider});
        emit_JUMPIFEQ(skip_throw_error, (Symb){.type = Type_variable, .value.v = is_builtin}, (Symb){.type = Type_bool, .value.b = true});
        emit_JUMPIFEQ(skip_throw_error, (Symb){.type = Type_variable, .value.v = is_invalid}, (Symb){.type = Type_bool, .value.b = false});
        emit_JUMPIFNEQ(skip_throw_error, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_EXIT((Symb){.type = Type_int, .value.i = 7});
        emit_LABEL(skip_throw_error);
        emit_JUMP(castEnd);
        emit_LABEL(not_string);
        free(not_string);
        free(floatval_loop);
        free(floatval_loop_end);
        free(skip_decimal_check);
        free(skip_decimal_counter);
        free(divider_loop);
        free(skip_divider_loop);
        free(skip_exponent_check);
        free(skip_exponent_operator_check);
        free(skip_exponent_parameter);
        free(exponent_loop);
        free(empty_loop);
        free(skip_exponent_loop);
        free(operator_is_minus);
        free(operator_is_plus);
        free(skip_fix_no_operator);
        free(skip_throw_error);
        free(skip_beginning);
    }
    emit_LABEL(castEnd);
    free(castEnd);
    return (Symb){.type=Type_variable, .value.v=result};
}


Symb generateCastToString(Symb symb, Expression * expression, Context * ctx, Symb * typeSymb) {
    UnionType unionType = expression->getType(expression, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    unionType.isUndefined = false;
    Type type = unionTypeToType(unionType);
    if(type.type == TYPE_STRING && type.isRequired == true) {
        return symb;
    }
    if(expression->expressionType == EXPRESSION_CONSTANT) {
        Expression__Constant * constant = performConstantCast((Expression__Constant*)expression, (Type){.type=TYPE_STRING, .isRequired=true}, false);
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
    char* castEnd = create_label("cast_end&", castUID);
    Var result = generateTemporaryVariable(*ctx);
    if(unionType.isBool) {
        char* notBool = create_label("not_bool&", castUID);
        char* isTrue = create_label("is_true&", castUID);
        emit_JUMPIFNEQ(notBool, symbType, (Symb){.type = Type_string, .value.s = "bool"});
        emit_JUMPIFNEQ(isTrue, symb, (Symb){.type = Type_bool, .value.b = 0});
        emit_MOVE(result, (Symb){.type = Type_string, .value.s = ""});
        emit_JUMP(castEnd);
        emit_LABEL(isTrue);
        emit_MOVE(result, (Symb){.type = Type_string, .value.s = "1"});
        emit_JUMP(castEnd);
        emit_LABEL(notBool);
        free(notBool);
        free(isTrue);
    }
    if(unionType.isNull) {
        char* notNil = create_label("not_nil&", castUID);
        emit_JUMPIFNEQ(notNil, symbType, (Symb){.type = Type_string, .value.s = "nil"});
        emit_MOVE(result, (Symb){.type=Type_string, .value.s=""});
        emit_JUMP(castEnd);
        emit_LABEL(notNil);
        free(notNil);
    }
    if(unionType.isInt) {
        Var q = generateTemporaryVariable(*ctx);
        Var r = generateTemporaryVariable(*ctx);
        Var temp_char = generateTemporaryVariable(*ctx);
        Var index = generateTemporaryVariable(*ctx);
        Var is_negative = generateTemporaryVariable(*ctx);
        char* notInt = create_label("not_int&", castUID);
        char* strval_push_loop = create_label("strval_push_loop&", castUID);
        char* strval_pop_loop = create_label("strval_pop_loop&", castUID);
        char* check_negative = create_label("check_negative&", castUID);
        char* is_positive = create_label("is_positive&", castUID);
        emit_JUMPIFNEQ(notInt, symbType, (Symb){.type = Type_string, .value.s = "int"});
        emit_MOVE(result, (Symb){.type = Type_string, .value.s = ""});
        emit_MOVE(index, (Symb){.type = Type_int, .value.i = 0});
        // check if number is negative
        emit_LT(is_negative, symb, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(q, symb);
        // if number is negative, turn it to positive and add - to the output at the beginning of the pop loop
        emit_JUMPIFNEQ(is_positive, (Symb){.type = Type_variable, .value.v = is_negative}, (Symb){.type = Type_bool, .value.b = true});
        emit_MUL(q, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = -1});
        emit_LABEL(is_positive);
        // loop for pushing converted numbers to stack
        emit_LABEL(strval_push_loop);
        // r = number % 10
        emit_IDIV(r, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = 10});
        emit_MUL(r, (Symb){.type = Type_variable, .value.v = r}, (Symb){.type = Type_int, .value.i = 10});
        emit_SUB(r, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_variable, .value.v = r});
        emit_PUSHS((Symb){.type = Type_variable, .value.v = r});
        // q = number / 10 
        emit_IDIV(q, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = 10});
        // print remainder
        emit_ADD(index, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFEQ(check_negative, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = 0});
        // jump to next iteration
        emit_JUMP(strval_push_loop); 
        // adding - if number is negative
        emit_LABEL(check_negative);
        emit_JUMPIFNEQ(strval_pop_loop, (Symb){.type = Type_variable, .value.v = is_negative}, (Symb){.type = Type_bool, .value.b = true});
        emit_CONCAT(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_string, .value.s = "-"});
        // loop for poping converted numbers from stack and printing
        emit_LABEL(strval_pop_loop);
        emit_POPS(r);
        emit_ADD(r, (Symb){.type = Type_variable, .value.v = r}, (Symb){.type = Type_int, .value.i = 48});
        emit_INT2CHAR(temp_char, (Symb){.type = Type_variable, .value.v = r});
        emit_CONCAT(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_variable, .value.v = temp_char});
        emit_SUB(index, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFNEQ(strval_pop_loop, (Symb){.type = Type_variable, .value.v = index}, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMP(castEnd);
        emit_LABEL(notInt);
        free(notInt);
        free(strval_push_loop);
        free(strval_pop_loop);
        free(check_negative);
        free(is_positive);
    }
    if(unionType.isFloat) {
        Var q = generateTemporaryVariable(*ctx);
        Var r = generateTemporaryVariable(*ctx);
        Var temp_char = generateTemporaryVariable(*ctx);
        Var char_counter = generateTemporaryVariable(*ctx);
        Var dot_counter = generateTemporaryVariable(*ctx);
        Var is_negative = generateTemporaryVariable(*ctx);
        char* notFloat = create_label("not_float&", castUID);
        char* strval_push_loop = create_label("strval_push_loop&", castUID);
        char* strval_pop_loop = create_label("strval_pop_loop&", castUID);
        char* not_float_dot = create_label("not_float_dot&", castUID);
        char* check_negative = create_label("check_negative&", castUID);
        char* is_positive = create_label("is_positive&", castUID);
        emit_JUMPIFNEQ(notFloat, symbType, (Symb){.type = Type_string, .value.s = "float"});
        emit_MOVE(result, (Symb){.type = Type_string, .value.s = ""});
        emit_MOVE(char_counter, (Symb){.type = Type_int, .value.i = 0});
        emit_MOVE(dot_counter, (Symb){.type = Type_int, .value.i = 0});
        // check if number is negative
        emit_LT(is_negative, symb, (Symb){.type = Type_float, .value.f = 0});
        emit_MOVE(q, symb);
        // if number is negative, turn it to positive and add - to the output at the beginning of the pop loop
        emit_JUMPIFNEQ(is_positive, (Symb){.type = Type_variable, .value.v = is_negative}, (Symb){.type = Type_bool, .value.b = true});
        emit_MUL(q, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_float, .value.f = -1});
        emit_LABEL(is_positive);
        // multiply by 10000 to move 4 decimal places to the right
        emit_MUL(q, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_float, .value.f = 10000});
        emit_FLOAT2INT(q, (Symb){.type = Type_variable, .value.v = q});
        // loop for pushing converted numbers to stack
        emit_LABEL(strval_push_loop);
        // r = number % 10
        emit_IDIV(r, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = 10});
        emit_MUL(r, (Symb){.type = Type_variable, .value.v = r}, (Symb){.type = Type_int, .value.i = 10});
        emit_SUB(r, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_variable, .value.v = r});
        emit_PUSHS((Symb){.type = Type_variable, .value.v = r});
        // q = number / 10 
        emit_IDIV(q, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = 10});
        emit_ADD(char_counter, (Symb){.type = Type_variable, .value.v = char_counter}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFEQ(check_negative, (Symb){.type = Type_variable, .value.v = q}, (Symb){.type = Type_int, .value.i = 0});
        // jump to next iteration
        emit_JUMP(strval_push_loop); 
        // adding - if number is negative
        emit_LABEL(check_negative);
        emit_JUMPIFNEQ(strval_pop_loop, (Symb){.type = Type_variable, .value.v = is_negative}, (Symb){.type = Type_bool, .value.b = true});
        emit_CONCAT(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_string, .value.s = "-"});
        // loop for poping converted numbers from stack and printing
        emit_LABEL(strval_pop_loop);
        // jumping to not_float_dot if 4 numbers were not poped
        emit_JUMPIFNEQ(not_float_dot, (Symb){.type = Type_variable, .value.v = dot_counter}, (Symb){.type = Type_int, .value.i = 6});
        emit_CONCAT(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_string, .value.s = "."});
        emit_LABEL(not_float_dot);
        emit_ADD(dot_counter, (Symb){.type = Type_variable, .value.v = char_counter}, (Symb){.type = Type_int, .value.i = 1});
        emit_POPS(r);
        emit_ADD(r, (Symb){.type = Type_variable, .value.v = r}, (Symb){.type = Type_int, .value.i = 48});
        emit_INT2CHAR(temp_char, (Symb){.type = Type_variable, .value.v = r});
        emit_CONCAT(result, (Symb){.type = Type_variable, .value.v = result}, (Symb){.type = Type_variable, .value.v = temp_char});
        emit_SUB(char_counter, (Symb){.type = Type_variable, .value.v = char_counter}, (Symb){.type = Type_int, .value.i = 1});
        emit_JUMPIFNEQ(strval_pop_loop, (Symb){.type = Type_variable, .value.v = char_counter}, (Symb){.type = Type_int, .value.i = 0});
        emit_JUMP(castEnd);
        emit_LABEL(notFloat);
        free(notFloat);
        free(strval_push_loop);
        free(strval_pop_loop);
        free(not_float_dot);
        free(check_negative);
        free(is_positive);
    }
    if(unionType.isString) {
        char* notString = create_label("not_string&", castUID);
        emit_JUMPIFNEQ(notString, symbType, (Symb){.type = Type_string, .value.s = "string"});
        emit_MOVE(result, symb);
        emit_JUMP(castEnd);
        emit_LABEL(notString);
        free(notString);
    }
    emit_LABEL(castEnd);
    free(castEnd);
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
    Type subType = unionTypeToType(subTypeExpression->getType(subTypeExpression, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
    if(requiredType.type == subType.type && (requiredType.isRequired == subType.isRequired || requiredType.isRequired == false)) {
        return;
    }
    if(!requiredType.isRequired && subType.type == TYPE_NULL) {
        return;
    }
    char* typeCheckPassed = create_label("type_check_passed&", getNextCodeGenUID());
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
    emit_JUMPIFEQ(typeCheckPassed, realType, (Symb){.type=Type_string, .value.s=requiredTypeStr});
    if(!requiredType.isRequired) {
        emit_JUMPIFEQ(typeCheckPassed, realType, (Symb){.type=Type_string, .value.s="nil"});
    }
    emit_DPRINT((Symb){.type=Type_string, .value.s=typeCheckFailMsg});
    emit_EXIT((Symb){.type=Type_int, .value.i=4});
    emit_LABEL(typeCheckPassed);
    freeTemporarySymbol(realType, ctx);
    free(typeCheckPassed);
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
    Symb * arguments = malloc(sizeof(Symb) * expression->arity);
    for(int i=0; i<expression->arity; i++) {
        arguments[i] = saveTempSymb(generateExpression(expression->arguments[i], ctx, false, NULL), ctx);
    }
    Function * function = (Function*)tableItem->data;
    if(function->body == NULL) {
        // this is built in function
        if(strcmp(function->name, "write") == 0) {
            for(int i=0; i<expression->arity; i++) {
                emit_WRITE(arguments[i]);
            }
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
        Symb retSymb = generateCastToFloat(arguments[0], expression->arguments[0], &ctx, NULL, true);
        if(arguments[0].type != Type_variable || retSymb.type != Type_variable || arguments[0].value.v.frameType != retSymb.value.v.frameType || strcmp(arguments[0].value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(arguments[0], ctx);
        }
        return retSymb;
    } else if(strcmp(function->name, "intval") == 0) {
        Symb retSymb = generateCastToInt(arguments[0], expression->arguments[0], &ctx, NULL, true);
        if(arguments[0].type != Type_variable || retSymb.type != Type_variable || arguments[0].value.v.frameType != retSymb.value.v.frameType || strcmp(arguments[0].value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(arguments[0], ctx);
        }
        return retSymb;
    } else if(strcmp(function->name, "boolval") == 0) {
        Symb retSymb = generateCastToBool(expression->arguments[0], arguments[0], ctx, false);
        if(arguments[0].type != Type_variable || retSymb.type != Type_variable || arguments[0].value.v.frameType != retSymb.value.v.frameType || strcmp(arguments[0].value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(arguments[0], ctx);
        }
        return retSymb;
    } else if(strcmp(function->name, "strval") == 0) {
        Symb retSymb = generateCastToString(arguments[0], expression->arguments[0], &ctx, NULL);
        if(arguments[0].type != Type_variable || retSymb.type != Type_variable || arguments[0].value.v.frameType != retSymb.value.v.frameType || strcmp(arguments[0].value.v.name, retSymb.value.v.name) != 0) {
            freeTemporarySymbol(arguments[0], ctx);
        }
        return retSymb;
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
        Var retVar = generateTemporaryVariable(ctx);
        size_t substringUID = getNextCodeGenUID();
        char* func_substring_end = create_label("func_substring_end&", substringUID);
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
        char* func_substring_loop_start = create_label("func_substring_loop_start&", substringUID);
        Var func_substring_loop_indexVar = generateTemporaryVariable(ctx);
        emit_MOVE(func_substring_loop_indexVar, symb2);
        emit_MOVE(retVar, (Symb){.type=Type_string, .value.s=""});
        emit_JUMPIFNEQS(func_substring_loop_start);
        emit_MOVE(retVar, (Symb){.type=Type_null});
        emit_JUMP(func_substring_end);
        emit_LABEL(func_substring_loop_start);
        emit_JUMPIFEQ(func_substring_end, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar}, symb3);
        emit_GETCHAR(tempVar, symb1, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar});
        emit_CONCAT(retVar, (Symb){.type=Type_variable, .value.v=retVar}, (Symb){.type=Type_variable, .value.v=tempVar});
        emit_ADD(func_substring_loop_indexVar, (Symb){.type=Type_variable, .value.v=func_substring_loop_indexVar}, (Symb){.type=Type_int, .value.i=1});
        emit_JUMP(func_substring_loop_start);
        emit_LABEL(func_substring_end);
        free(func_substring_end);
        free(func_substring_loop_start);
        freeTemporaryVariable(tempVar, ctx);
        freeTemporaryVariable(func_substring_loop_indexVar, ctx);
        freeArguments(arguments, expression->arity, ctx);
        return (Symb){.type = Type_variable, .value.v=retVar};
    } else if(strcmp(function->name, "ord") == 0) {
        size_t ordId = getNextCodeGenUID();
        char* ordEnd = create_label("ord_end&", ordId);
        Symb symb = arguments[0];
        Var retVar = generateTemporaryVariable(ctx);
        emit_STRLEN(retVar, symb);
        emit_JUMPIFEQ(ordEnd, (Symb){.type = Type_variable, .value.v = retVar}, (Symb){.type=Type_int, .value.i=0});
        emit_STRI2INT(retVar, symb, (Symb){.type=Type_int, .value.i=0});
        emit_LABEL(ordEnd);
        free(ordEnd);
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
    UnionType unionType1 = expr1->getType(expr1, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    UnionType unionType2 = expr2->getType(expr2, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    bool canBeFloat = unionType1.isFloat || unionType2.isFloat || unionType1.isString || unionType2.isString;
    bool isType1Float = !unionType1.isBool && unionType1.isFloat && !unionType1.isInt && !unionType1.isNull && !unionType1.isString;
    bool isType2Float = !unionType2.isBool && unionType2.isFloat && !unionType2.isInt && !unionType2.isNull && !unionType2.isString;
    bool isGuaranteedFloat = isType1Float || isType2Float;
    if(isGuaranteedFloat) {
        *out1 = generateCastToFloat(symb1, expr1, ctx, NULL, false);
        *out2 = generateCastToFloat(symb2, expr2, ctx, NULL, false);
    } else if(canBeFloat) {
        Symb type1 = generateSymbType(expr1, symb1, *ctx);
        Symb type2 = generateSymbType(expr2, symb2, *ctx);
        size_t castUID = getNextCodeGenUID();
        char* castEnd = create_label("cast_end&", castUID);
        Var result1 = generateTemporaryVariable(*ctx);
        *out1 = (Symb){.type=Type_variable, .value.v=result1};
        Var result2 = generateTemporaryVariable(*ctx);
        *out2 = (Symb){.type=Type_variable, .value.v=result2};
        char* isFloat = create_label("is_float&", castUID);
        emit_JUMPIFEQ(isFloat, type1, (Symb){.type = Type_string, .value.s = "float"});
        emit_JUMPIFEQ(isFloat, type2, (Symb){.type = Type_string, .value.s = "float"});
        emit_MOVE(result1, generateCastToInt(symb1, expr1, ctx, &type1, false));
        emit_MOVE(result2, generateCastToInt(symb2, expr2, ctx, &type2, false));
        emit_JUMP(castEnd);
        emit_LABEL(isFloat);
        emit_MOVE(result1, generateCastToFloat(symb1, expr1, ctx, &type1, false));
        emit_MOVE(result2, generateCastToFloat(symb2, expr2, ctx, &type2, false));
        emit_LABEL(castEnd);
        free(castEnd);
        free(isFloat);
        freeTemporarySymbol(type1, *ctx);
        freeTemporarySymbol(type2, *ctx);
    } else {
        *out1 = generateCastToInt(symb1, expr1, ctx, NULL, false);
        *out2 = generateCastToInt(symb2, expr2, ctx, NULL, false);
    }
}

void relationalOperatorCast(Symb symb1, Symb symb2, Expression * expr1, Expression * expr2, Symb * out1, Symb * out2, Context * ctx) {
    UnionType unionType1 = expr1->getType(expr1, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    UnionType unionType2 = expr2->getType(expr2, ctx->functionTable, ctx->program, ctx->currentFunction, ctx->resultTable);
    Symb type1 = generateSymbType(expr1, symb1, *ctx);
    Symb type2 = generateSymbType(expr2, symb2, *ctx);
    if(type1.type != Type_variable && type2.type != Type_variable && strcmp(type1.value.s, type2.value.s) == 0 && strcmp(type1.value.s, "nil") != 0) {
        *out1 = symb1;
        *out2 = symb2;
        return;
    }
    Var result1 = generateTemporaryVariable(*ctx);
    *out1 = (Symb){.type=Type_variable, .value.v=result1};
    Var result2 = generateTemporaryVariable(*ctx);
    *out2 = (Symb){.type=Type_variable, .value.v=result2};
    char* isNull = create_label("is_null&", getNextCodeGenUID());
    char* isFloat = create_label("is_float&", getNextCodeGenUID());
    char* castEnd = create_label("cast_end&", getNextCodeGenUID());
    char* arentEqualTypes = create_label("arent_equal_types&", getNextCodeGenUID());
    char* isString = create_label("is_string&", getNextCodeGenUID());
    if(unionType1.isNull || unionType2.isNull) {
        emit_JUMPIFEQ(isNull, type1, (Symb){.type = Type_string, .value.s = "nil"});
        emit_JUMPIFEQ(isNull, type2, (Symb){.type = Type_string, .value.s = "nil"});
    }
    emit_JUMPIFNEQ(arentEqualTypes, type1, type2);
    emit_MOVE(result1, symb1);
    emit_MOVE(result2, symb2);
    emit_JUMP(castEnd);
    emit_LABEL(arentEqualTypes);
    if(unionType1.isString || unionType2.isString) {
        emit_JUMPIFEQ(isString, type1, (Symb){.type = Type_string, .value.s = "string"});
        emit_JUMPIFEQ(isString, type2, (Symb){.type = Type_string, .value.s = "string"});
    }
    if(unionType1.isFloat || unionType2.isFloat) {
        emit_JUMPIFEQ(isFloat, type1, (Symb){.type = Type_string, .value.s = "float"});
        emit_JUMPIFEQ(isFloat, type2, (Symb){.type = Type_string, .value.s = "float"});
    }
    emit_MOVE(result1, generateCastToInt(symb1, expr1, ctx, &type1, false));
    emit_MOVE(result2, generateCastToInt(symb2, expr2, ctx, &type2, false));
    emit_JUMP(castEnd);
    if(unionType1.isNull || unionType2.isNull) {
        emit_LABEL(isNull);
        emit_MOVE(result1, generateCastToBool(expr1, symb1, *ctx, false));
        emit_MOVE(result2, generateCastToBool(expr2, symb2, *ctx, false));
        emit_JUMP(castEnd);
    }
    if(unionType1.isFloat || unionType2.isFloat) {
        emit_LABEL(isFloat);
        emit_MOVE(result1, generateCastToFloat(symb1, expr1, ctx, &type1, false));
        emit_MOVE(result2, generateCastToFloat(symb2, expr2, ctx, &type2, false));
        emit_JUMP(castEnd);
    }
    if(unionType1.isString || unionType2.isString) {
        emit_LABEL(isString);
        emit_MOVE(result1, generateCastToString(symb1, expr1, ctx, &type1));
        emit_MOVE(result2, generateCastToString(symb2, expr2, ctx, &type2));
    }
    emit_LABEL(castEnd);
    free(castEnd);
    free(isNull);
    free(isFloat);
    free(arentEqualTypes);
    free(isString);
    freeTemporarySymbol(type1, *ctx);
    freeTemporarySymbol(type2, *ctx);
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
            Symb left2 = generateCastToFloat(left, expression->lSide, &ctx, NULL, false);
            Symb right2 = generateCastToFloat(right, expression->rSide, &ctx, NULL, false);
            emit_DIV(outVar, left2, right2);
            break;
        }
        case TOKEN_EQUALS: {
            Type typeL = unionTypeToType(expression->lSide->getType(expression->lSide, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
            Type typeR = unionTypeToType(expression->rSide->getType(expression->rSide, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
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
                char* type_check_ok = create_label("type_check_ok&", operatorTypeCheckId);
                char* operator_done = create_label("operator_done&", operatorTypeCheckId);
                emit_JUMPIFEQ(type_check_ok, typeOut1, typeOut2);
                freeTemporarySymbol(typeOut1, ctx);
                freeTemporarySymbol(typeOut2, ctx);
                emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=false});
                emit_JUMP(operator_done);
                emit_LABEL(type_check_ok);
                emit_EQ(outVar, left, right);
                emit_LABEL(operator_done);
                free(type_check_ok);
                free(operator_done);
            }
            break;
        }
        case TOKEN_NOT_EQUALS: {
            Type typeL = unionTypeToType(expression->lSide->getType(expression->lSide, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
            Type typeR = unionTypeToType(expression->rSide->getType(expression->rSide, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
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
                char* type_check_ok = create_label("type_check_ok&", operatorTypeCheckId);
                char* operator_done = create_label("operator_done&", operatorTypeCheckId);
                emit_JUMPIFEQ(type_check_ok, typeOut1, typeOut2);
                freeTemporarySymbol(typeOut1, ctx);
                freeTemporarySymbol(typeOut2, ctx);
                emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=true});
                emit_JUMP(operator_done);
                emit_LABEL(type_check_ok);
                emit_EQ(outVar, left, right);
                emit_NOT(outVar, outSymb);
                emit_LABEL(operator_done);
                free(type_check_ok);
                free(operator_done);
            }
            break;
        }
        case TOKEN_LESS:
            relationalOperatorCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_LT(outVar, left, right);
            break;
        case TOKEN_GREATER:
            relationalOperatorCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_GT(outVar, left, right);
            break;
        case TOKEN_LESS_OR_EQUALS:
            relationalOperatorCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_GT(outVar, left, right);
            emit_NOT(outVar, outSymb);
            break;
        case TOKEN_GREATER_OR_EQUALS:
            relationalOperatorCast(left, right, expression->lSide, expression->rSide, &left, &right, &ctx);
            emit_LT(outVar, left, right);
            emit_NOT(outVar, outSymb);
            break;
        case TOKEN_AND: {
            size_t operatorAndId = getNextCodeGenUID();
            char* and_is_false = create_label("and_is_false&", operatorAndId);
            char* operator_and_done = create_label("operator_and_done&", operatorAndId);
            Symb leftBool = generateCastToBool(expression->lSide, left, ctx, false);
            emit_JUMPIFEQ(and_is_false, leftBool, (Symb){.type=Type_bool, .value.b=false});
            freeTemporarySymbol(leftBool, ctx);
            right = generateExpression(expression->rSide, ctx, false, NULL);
            Symb rightBool = generateCastToBool(expression->rSide, right, ctx, false);
            emit_MOVE(outVar, rightBool);
            freeTemporarySymbol(rightBool, ctx);
            emit_JUMP(operator_and_done);
            emit_LABEL(and_is_false);
            emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=false});
            emit_LABEL(operator_and_done);
            free(and_is_false);
            return outSymb;
            break;
        }
        case TOKEN_OR: {
            size_t operatorOrId = getNextCodeGenUID();
            char* or_is_true = create_label("or_is_true&", operatorOrId);
            char* operator_or_done = create_label("operator_or_done&", operatorOrId);
            Symb leftBool = generateCastToBool(expression->lSide, left, ctx, false);
            emit_JUMPIFEQ(or_is_true, leftBool, (Symb){.type=Type_bool, .value.b=true});
            freeTemporarySymbol(leftBool, ctx);
            right = generateExpression(expression->rSide, ctx, false, NULL);
            Symb rightBool = generateCastToBool(expression->rSide, right, ctx, false);
            emit_MOVE(outVar, rightBool);
            freeTemporarySymbol(rightBool, ctx);
            emit_JUMP(operator_or_done);
            emit_LABEL(or_is_true);
            emit_MOVE(outVar, (Symb){.type=Type_bool, .value.b=true});
            emit_LABEL(operator_or_done);
            free(or_is_true);
            free(operator_or_done);
            return outSymb;
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

void generateConditionJump(Expression * expression, Context ctx, char * label, bool valueToJump) {
    if(expression->expressionType == EXPRESSION_BINARY_OPERATOR) {
        Expression__BinaryOperator * binaryOperator = (Expression__BinaryOperator*)expression;
        Type typeL = unionTypeToType(binaryOperator->lSide->getType(binaryOperator->lSide, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
        Type typeR = unionTypeToType(binaryOperator->rSide->getType(binaryOperator->rSide, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable));
        if(typeL.type != TYPE_UNKNOWN && typeR.type != TYPE_UNKNOWN) {
            if(binaryOperator->operator == TOKEN_EQUALS || binaryOperator->operator == TOKEN_NOT_EQUALS) {
                if(binaryOperator->operator == TOKEN_NOT_EQUALS) {
                    valueToJump = !valueToJump;
                }
                if(typeL.type == typeR.type || (typeL.type == TYPE_NULL && !typeR.isRequired) || (typeR.type == TYPE_NULL && !typeL.isRequired)) {
                    Symb left = generateExpression(binaryOperator->lSide, ctx, false, NULL);
                    Symb right = generateExpression(binaryOperator->rSide, ctx, false, NULL);
                    if(valueToJump) {
                        emit_JUMPIFEQ(label, left, right);
                    } else {
                        emit_JUMPIFNEQ(label, left, right);
                    }
                } else {
                    if(!valueToJump) {
                        emit_JUMP(label);
                    }
                }
                return;
            }
        }
    }
    Symb condition = generateExpression(expression, ctx, false, NULL);
    Symb conditionBool = generateCastToBool(expression, condition, ctx, true);
    emit_JUMPIFEQ(label, conditionBool, (Symb){.type=Type_bool, .value.b=valueToJump});
    if(conditionBool.type != Type_variable || condition.type != Type_variable || conditionBool.value.v.frameType != condition.value.v.frameType || strcmp(conditionBool.value.v.name, condition.value.v.name) != 0) {
        freeTemporarySymbol(conditionBool, ctx);
    }
    freeTemporarySymbol(condition, ctx);
}

/**
 * @brief Generates code for if statement
 * 
 * @param statement 
 * @param ctx 
 */
void generateIf(StatementIf * statement, Context ctx) {
    size_t ifUID = getNextCodeGenUID();
    char* ifElse = create_label("ifElse&", ifUID);
    char* ifEnd = create_label("ifEnd&", ifUID);
    bool isElseEmpty = statement->elseBody == NULL || (statement->elseBody->statementType == STATEMENT_LIST && ((StatementList*)statement->elseBody)->listSize == 0);
    generateConditionJump(statement->condition, ctx, ifElse, false);
    generateStatement(statement->ifBody, ctx);
    if(!isElseEmpty) emit_JUMP(ifEnd);
    emit_LABEL(ifElse);
    if(!isElseEmpty) generateStatement(statement->elseBody, ctx);
    if(!isElseEmpty) emit_LABEL(ifEnd);
    if(isElseEmpty) emit_COMMENT("Else body is empty");

    free(ifElse);
    free(ifEnd);
}

/**
 * @brief Generates code for while statement
 * 
 * @param statement 
 * @param ctx 
 */
void generateWhile(StatementWhile * statement, Context ctx) {
    size_t whileUID = getNextCodeGenUID();
    char* whileStart = create_label("whileStart&", whileUID);
    char* whileEnd = create_label("whileEnd&", whileUID);

    emit_LABEL(whileStart);
    generateConditionJump(statement->condition, ctx, whileEnd, false);
    generateStatement(statement->body, ctx);
    emit_JUMP(whileStart);
    emit_LABEL(whileEnd);

    free(whileStart);
    free(whileEnd);
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
            UnionType returnUnionType = statement->expression->getType(statement->expression, ctx.functionTable, ctx.program, ctx.currentFunction, ctx.resultTable);
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
void generateFunction(Function* function, Table * functionTable, PointerTable * resultTable) {
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
    ctx.resultTable = resultTable;
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
    PointerTable * resultTable = table_statement_init();
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
    ctx.resultTable = resultTable;
    generateStatementList(program, ctx);
    emit_DEFVAR_end();
    emit_instruction_end();
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem* item = functionTable->tb[i];
        while(item != NULL) {
            Function* function = (Function*) item->data;
            if(function->body != NULL) {
                generateFunction(function, functionTable, resultTable);
            }
            item = item->next;
        }
    }
}