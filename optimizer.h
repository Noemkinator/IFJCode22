/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file optimizer.h
 * @author Jiří Gallo (xgallo04), Jakub Kratochvíl (xkrato67)
 * @brief Header file for the optimizer of IFJcode22
 * @date 2022-10-26
 */

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