// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#ifndef __CODE_GENERATOR_H__
#define __CODE_GENERATOR_H__

#include "ast.h"
#include "symtable.h"

void generateCode(StatementList * program, Table * functionTable);

#endif