// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include <stddef.h>
#include <stdio.h>
#include "emitter.h"

void emit_header() {
	printf(".IFJcode22\n");
}

void emit_type(StorageType type) {
	char * typeName = NULL;
	switch(type) {
		case Type_int:		typeName = "int";	break;
		case Type_bool:		typeName = "bool";	break;
		case Type_float:	typeName = "float";	break;
		case Type_string:	typeName = "string";	break;
		case Type_variable:	typeName = "variable";	break;
	}
	printf("%s", typeName);
}

void emit_var(Var var) {
	char * frameName = NULL;
	switch(var.frameType) {
		case LF: frameName = "LF"; break;
		case TF: frameName = "TF"; break;
		case GF: frameName = "GF"; break;
	}
	printf("%s@%s", frameName, var.name);
}

void emit_symb(Symb symb) {
	switch(symb.type) {
		case Type_int:		printf("int@%d", symb.value.i);	break;
		case Type_bool:		printf("bool@%s", symb.value.b ? "true" : "false");	break;
		case Type_float:	printf("float@%a", symb.value.f);	break;
		case Type_string:	printf("string@%s", symb.value.s);	break;
		case Type_variable:	emit_var(symb.value.v);	break;
	}
}

void emit_label(char * label) {
	printf("%s", label);
}

void emit_MOVE(Var var, Symb symb) {
	printf("MOVE ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_CREATEFRAME() {
	printf("CREATEFRAME\n");
}

void emit_PUSHFRAME() {
	printf("PUSHFRAME\n");
}

void emit_POPFRAME() {
	printf("POPFRAME\n");
}

void emit_DEFVAR(Var var) {
	printf("DEFVAR ");
	emit_var(var);
	printf("\n");
}

void emit_CALL(char * label) {
	printf("CALL ");
	emit_label(label);
	printf("\n");
}

void emit_RETURN() {
	printf("RETURN\n");
}

void emit_PUSHS(Symb symb) {
	printf("PUSHS ");
	emit_symb(symb);
	printf("\n");
}

void emit_POPS(Var var) {
	printf("POPS ");
	emit_var(var);
	printf("\n");
}

void emit_CLEARS() {
	printf("CLEARS\n");
}

void emit_ADD(Var var, Symb symb1, Symb symb2) {
	printf("ADD ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_SUB(Var var, Symb symb1, Symb symb2) {
	printf("SUB ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_MUL(Var var, Symb symb1, Symb symb2) {
	printf("MUL ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_DIV(Var var, Symb symb1, Symb symb2) {
	printf("DIV ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_IDIV(Var var, Symb symb1, Symb symb2) {
	printf("IDIV ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_ADDS() {
	printf("ADDS\n");
}

void emit_SUBS() {
	printf("SUBS\n");
}

void emit_MULS() {
	printf("MULS\n");
}

void emit_DIVS() {
	printf("DIVS\n");
}

void emit_IDIVS() {
	printf("IDIVS\n");
}

void emit_LT(Var var, Symb symb1, Symb symb2) {
	printf("LT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_GT(Var var, Symb symb1, Symb symb2) {
	printf("GT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_EQ(Var var, Symb symb1, Symb symb2) {
	printf("EQ ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_LTS() {
	printf("LTS\n");
}

void emit_GTS() {
	printf("GTS\n");
}

void emit_EQS() {
	printf("EQS\n");
}

void emit_AND(Var var, Symb symb1, Symb symb2) {
	printf("AND ");
	emit_var(var);
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_OR(Var var, Symb symb1, Symb symb2) {
	printf("OR ");
	emit_var(var);
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_NOT(Var var, Symb symb) {
	printf("NOT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_ANDS() {
	printf("ANDS\n");
}

void emit_ORS() {
	printf("ORS\n");
}

void emit_NOTS() {
	printf("NOTS\n");
}

void emit_INT2FLOAT(Var var, Symb symb) {
	printf("INT2FLOAT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_FLOAT2INT(Var var, Symb symb) {
	printf("FLOAT2INT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_INT2CHAR(Var var, Symb symb) {
	printf("INT2CHAR ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_STRI2INT(Var var, Symb symb1, Symb symb2) {
	printf("STRI2INT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_INT2FLOATS() {
	printf("INT2FLOATS\n");
}

void emit_FLOAT2INTS() {
	printf("FLOAT2INTS\n");
}

void emit_INT2CHARS() {
	printf("INT2CHARS\n");
}

void emit_STRI2INTS() {
	printf("STRI2INTS\n");
}

void emit_READ(Var var, StorageType type) {
	printf("READ ");
	emit_var(var);
	printf(" ");
	emit_type(type);
	printf("\n");
}

void emit_WRITE(Symb symb) {
	printf("WRITE ");
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_CONCAT(Var var, Symb symb1, Symb symb2) {
	printf("CONCAT ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_STRLEN(Var var, Symb symb) {
	printf("STRLEN ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_GETCHAR(Var var, Symb symb1, Symb symb2) {
	printf("GETCHAR ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_SETCHAR(Var var, Symb symb1, Symb symb2) {
	printf("SETCHAR ");
	emit_var(var);
	printf(" ");
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_TYPE(Var var, Symb symb) {
	printf("TYPE ");
	emit_var(var);
	printf(" ");
	emit_symb(symb);
	printf("\n");
}

void emit_LABEL(char *label) {
	printf("LABEL %s\n", label);
}

void emit_JUMP(char *label) {
	printf("JUMP %s\n", label);
}

void emit_JUMPIFEQ(char *label, Symb symb1, Symb symb2) {
	printf("JUMPIFEQ %s ", label);
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_JUMPIFNEQ(char *label, Symb symb1, Symb symb2) {
	printf("JUMPIFNEQ %s ", label);
	emit_symb(symb1);
	printf(" ");
	emit_symb(symb2);
	printf("\n");
}

void emit_JUMPIFEQS(char * label) {
	printf("JUMPIFEQS %s\n", label);
}

void emit_JUMPIFNEQS(char * label) {
	printf("JUMPIFNEQS %s\n", label);
}

void emit_EXIT(Symb symb) {
	printf("EXIT ");
	emit_symb(symb);
	printf("\n");
}

void emit_BREAK() {
	printf("BREAK\n");
}

void emit_DPRINT(Symb symb) {
	printf("DPRINT ");
	emit_symb(symb);
	printf("\n");
}

void emit_COMMENT(char *comment) {
	printf("# %s\n", comment);
}