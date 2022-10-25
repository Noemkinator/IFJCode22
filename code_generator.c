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

void generateStatement(Statement * statement, Table * varTable, Table * functionTable);

void generateExpression(Expression * expression, Table * varTable, Table * functionTable) {
    switch(expression->expressionType) {
        case EXPRESSION_CONSTANT:
            break;
        case EXPRESSION_VARIABLE:
            break;
        case EXPRESSION_FUNCTION_CALL:
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
    StringBuilder__appendString(&ifElseSb, "ifElse$");
    StringBuilder__appendInt(&ifElseSb, ifUID);
    StringBuilder ifEndSb;
    StringBuilder__init(&ifEndSb);
    StringBuilder__appendString(&ifEndSb, "ifEnd$");
    StringBuilder__appendInt(&ifEndSb, ifUID);

    generateExpression(statement->condition, varTable, functionTable);
    emit_PUSHS((Symb){.type=Type_bool, .value.b = true});
    emit_JUMPIFNEQS(ifElseSb.text);
    generateStatement(statement->ifBody, varTable, functionTable);
    emit_JUMP(ifEndSb.text);
    emit_LABEL(ifElseSb.text);
    generateStatement(statement->elseBody, varTable, functionTable);
    emit_LABEL(ifEndSb.text);

    StringBuilder__free(&ifElseSb);
    StringBuilder__free(&ifEndSb);
}

void generateReturn(StatementReturn * statement, Table * varTable, Table * functionTable) {
    generateExpression(statement->expression,  varTable, functionTable);
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
        case STATEMENT_WHILE:
            break;
        case STATEMENT_RETURN:
            generateReturn(statement, varTable, functionTable);
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

typedef struct {
    char * name;
    bool isGlobal;
    Type type;
} VariableInfo;

void generateFunction(Function* function, Table * functionTable) {
    if(function->body == NULL) return;
    Table* localTable = table_init();
    emit_LABEL(join_strings("function&", function->name));
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
                emit_DEFVAR((Var){.frameType=TF, .name=join_strings("var&", variable->name)});
            }
        }
    }
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
    generateStatement(program, globalTable, functionTable);
    for(int i = 0; i < TB_SIZE; i++) {
        TableItem* item = functionTable->tb[i];
        while(item != NULL) {
            Function* function = (Function*) item->data;
            generateFunction(function, functionTable);
            item = item->next;
        }
    }
}