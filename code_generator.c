#include "ast.h"
#include "symtable.h"
#include "emitter.h"
#include "string_builder.h"

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

void generateStatement(Statement * statement, Table * varTable, Table * functionTable);

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
        default:
            fprintf(stderr, "ERR: Invalid constant\n");
            exit(99);
    }
}

Symb generateVariable(Expression__Variable * statement, Table * varTable, Table * functionTable) {
    // causes mem leak
    char * varId = join_strings("var&", statement->name);
    return (Symb){.type = Type_variable, .value.v.frameType = ((VariableInfo*)table_find(varTable, statement->name)->data)->isGlobal ? GF : LF, .value.v.name = varId};
}

Symb generateExpression(Expression * expression, Table * varTable, Table * functionTable);

Symb generateFunctionCall(Expression__FunctionCall * expression, Table * varTable, Table * functionTable) {
    emit_CREATEFRAME();
    Function * function = (Function*)table_find(functionTable, expression->name)->data;
    if(function->arity != expression->arity) {
        fprintf(stderr, "ERR: Function %s called with wrong number of arguments\n", expression->name);
        exit(4);
    }
    for(int i=0; i<expression->arity; i++) {
        emit_DEFVAR((Var){.frameType = TF, .name = join_strings("var$", function->parameterNames[i])});
        emit_MOVE((Var){.frameType = TF, .name = join_strings("var$", function->parameterNames[i])}, generateExpression(expression->arguments[i], varTable, functionTable));
    }
    char * functionLabel = join_strings("function&", expression->name);
    emit_CALL(functionLabel);
    free(functionLabel);
    return (Symb){.type = Type_variable, .value.v=(Var){.frameType = TF, .name = "returnValue"}};
}

Symb generateExpression(Expression * expression, Table * varTable, Table * functionTable) {
    switch(expression->expressionType) {
        case EXPRESSION_CONSTANT:
            return generateConstant((Expression__Constant*)expression);
            break;
        case EXPRESSION_VARIABLE:
            return generateVariable((Expression__Variable*)expression, varTable, functionTable);
            break;
        case EXPRESSION_FUNCTION_CALL:
            return generateFunctionCall((Expression__FunctionCall*)expression, varTable, functionTable);
            break;
        case EXPRESSION_BINARY_OPERATOR:
            break;
    }
}

void generateStatementList(StatementList* statementList, Table * varTable, Table * functionTable) {
    for(int i = 0; i < statementList->listSize; i++) {
        generateStatement(statementList->statements[i], varTable, functionTable);
    }
}

void generateIf(StatementIf * statement, Table * varTable, Table * functionTable) {
    size_t ifUID = getNextCodeGenUID();
    StringBuilder ifElseSb;
    StringBuilder__init(&ifElseSb);
    StringBuilder__appendString(&ifElseSb, "ifElse&");
    StringBuilder__appendInt(&ifElseSb, ifUID);
    StringBuilder ifEndSb;
    StringBuilder__init(&ifEndSb);
    StringBuilder__appendString(&ifEndSb, "ifEnd&");
    StringBuilder__appendInt(&ifEndSb, ifUID);

    emit_JUMPIFNEQ(ifElseSb.text, generateExpression(statement->condition, varTable, functionTable), (Symb){.type=Type_bool, .value.b = true});
    generateStatement(statement->ifBody, varTable, functionTable);
    emit_JUMP(ifEndSb.text);
    emit_LABEL(ifElseSb.text);
    generateStatement(statement->elseBody, varTable, functionTable);
    emit_LABEL(ifEndSb.text);

    StringBuilder__free(&ifElseSb);
    StringBuilder__free(&ifEndSb);
}

void generateWhile(StatementWhile * statement, Table * varTable, Table * functionTable) {
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
    emit_JUMPIFNEQ(whileEndSb.text, generateExpression(statement->condition, varTable, functionTable), (Symb){.type=Type_bool, .value.b = true});
    generateStatement(statement->body, varTable, functionTable);
    emit_JUMP(whileStartSb.text);
    emit_LABEL(whileEndSb.text);

    StringBuilder__free(&whileStartSb);
    StringBuilder__free(&whileEndSb);
}

void generateReturn(StatementReturn * statement, Table * varTable, Table * functionTable) {
    // TODO global return
    if(statement->expression != NULL) {
        emit_MOVE((Var){.frameType = LF, .name = "returnValue"}, generateExpression(statement->expression,  varTable, functionTable));
    }
    emit_RETURN();
}

void generateStatement(Statement * statement, Table * varTable, Table * functionTable) {
    if(statement == NULL) return;
    switch(statement->statementType) {
        case STATEMENT_EXPRESSION:
            generateExpression((Expression*) statement,  varTable, functionTable);
            break;
        case STATEMENT_LIST:
            generateStatementList((StatementList*)statement, varTable, functionTable);
            break;
        case STATEMENT_IF:
            generateIf((StatementIf*)statement, varTable, functionTable);
            break;
        case STATEMENT_WHILE:
            generateWhile((StatementWhile*)statement, varTable, functionTable);
            break;
        case STATEMENT_RETURN:
            generateReturn((StatementReturn*)statement, varTable, functionTable);
            break;
        case STATEMENT_FUNCTION:
            fprintf(stderr, "OFF, ignoring function...\n");
            break;
    }
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

void generateFunction(Function* function, Table * functionTable) {
    if(function->body == NULL) return;
    Table* localTable = table_init();
    char * functionLabel = join_strings("function&", function->name);
    emit_LABEL(functionLabel);
    free(functionLabel);
    emit_CREATEFRAME();
    emit_DEFVAR((Var){frameType: TF, name: "returnValue"});
    size_t statementCount;
    Statement *** allStatements = getAllStatements(function->body, &statementCount);
    for(int i=0; i<statementCount; i++) {
        Statement * statement = *allStatements[i];
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
    for(int i = 0; i < function->arity; i++) {
        
    }
    emit_PUSHFRAME();
    generateStatement(function->body, localTable, functionTable);
    emit_POPFRAME();
    emit_RETURN();
}

void generateCode(StatementList * program, Table * functionTable) {
    emit_header();
    Table * globalTable = table_init();
    generateStatementList(program, globalTable, functionTable);
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem* item = functionTable->tb[i];
        while(item != NULL) {
            Function* function = (Function*) item->data;
            generateFunction(function, functionTable);
            item = item->next;
        }
    }
}