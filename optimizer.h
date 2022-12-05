// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include <stdbool.h>
#include <ctype.h>
#include "ast.h"
#include "symtable.h"

Expression__Constant * performConstantCast(Expression__Constant * in, Type targetType, bool isBuiltin);
Expression__Constant * performConstantCastCondition(Expression__Constant * in);
Expression__Constant * performConstantFolding(Expression__BinaryOperator * in);
Statement * performStatementFolding(Statement * in);
void optimize(StatementList * program, Table * functionTable);

#endif // __OPTIMIZER_H__