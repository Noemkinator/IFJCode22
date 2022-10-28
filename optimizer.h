// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include "ast.h"

Expression__Constant * performConstantCast(Expression__Constant * in, Type targetType);
Expression__Constant * performConstantFolding(Expression__BinaryOperator * in);
Statement * performStatementFolding(Statement * in);

#endif // __OPTIMIZER_H__