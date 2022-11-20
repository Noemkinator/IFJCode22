// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include "emitter.h"
#include "string_builder.h"

void emit_header() {
	printf(".IFJcode22\n");
}

StringBuilder instructionBuilder;

void emit_instruction_start() {
	StringBuilder__init(&instructionBuilder);
}

void emit_instruction_end() {
	printf("%s", instructionBuilder.text);
	StringBuilder__free(&instructionBuilder);
}

void emit_type(StorageType type) {
	char * typeName = NULL;
	switch(type) {
		case Type_int:		typeName = "int";	break;
		case Type_bool:		typeName = "bool";	break;
		case Type_float:	typeName = "float";	break;
		case Type_string:	typeName = "string";	break;
		case Type_null: 	typeName = "nil";	break;
		case Type_variable:	typeName = "variable";	break;
	}
	StringBuilder__appendString(&instructionBuilder, typeName);
}

void emit_var(Var var) {
	char * frameName = NULL;
	switch(var.frameType) {
		case LF: frameName = "LF"; break;
		case TF: frameName = "TF"; break;
		case GF: frameName = "GF"; break;
	}
	StringBuilder__appendString(&instructionBuilder, frameName);
	StringBuilder__appendChar(&instructionBuilder, '@');
	StringBuilder__appendString(&instructionBuilder, var.name);;
}

void emit_string(char * string) {
	StringBuilder__appendString(&instructionBuilder, "string@");
	while(*string) {
		if(isspace(*string) || *string == '#' || *string == '\\' || *string < 32) {
			unsigned int c = (unsigned int)(unsigned char)*string;
			StringBuilder__appendChar(&instructionBuilder, '\\');
			StringBuilder__appendInt(&instructionBuilder, c/100);
			c %= 100;
			StringBuilder__appendInt(&instructionBuilder, c/10);
			c %= 10;
			StringBuilder__appendInt(&instructionBuilder, c);
		} else {
			StringBuilder__appendChar(&instructionBuilder, *string);
		}
		string++;
	}
}

void emit_symb(Symb symb) {
	switch(symb.type) {
		case Type_int: {
			StringBuilder__appendString(&instructionBuilder, "int@");
			StringBuilder__appendInt(&instructionBuilder, symb.value.i);
			break;
		}
		case Type_bool:	{
			StringBuilder__appendString(&instructionBuilder, "bool@");
			StringBuilder__appendString(&instructionBuilder, symb.value.b ? "true" : "false");
			break;
		}
		case Type_float: {
			StringBuilder__appendString(&instructionBuilder, "float@");
			StringBuilder__appendFloat(&instructionBuilder, symb.value.f);
			break;
		}
		case Type_string: {
			emit_string(symb.value.s);
			break;
		}
		case Type_null: {
			StringBuilder__appendString(&instructionBuilder, "nil@nil");
			break;
		}
		case Type_variable:	{
			emit_var(symb.value.v);
			break;
		}
	}
}

void emit_label(char * label) {
	StringBuilder__appendString(&instructionBuilder, label);
}

void emit_MOVE(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "MOVE ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_CREATEFRAME() {
	StringBuilder__appendString(&instructionBuilder, "CREATEFRAME\n");
}

void emit_PUSHFRAME() {
	StringBuilder__appendString(&instructionBuilder, "PUSHFRAME\n");
}

void emit_POPFRAME() {
	StringBuilder__appendString(&instructionBuilder, "POPFRAME\n");
}

StringBuilder defVarBuilder;

void emit_DEFVAR_start() {
	StringBuilder__init(&defVarBuilder);
}

void emit_DEFVAR(Var var) {
	if(var.frameType != TF) {
		StringBuilder__appendString(&defVarBuilder, "DEFVAR ");
		char * frameName = NULL;
		switch(var.frameType) {
			case LF: frameName = "LF"; break;
			case TF: frameName = "TF"; break;
			case GF: frameName = "GF"; break;
		}
		StringBuilder__appendString(&defVarBuilder, frameName);
		StringBuilder__appendChar(&defVarBuilder, '@');
		StringBuilder__appendString(&defVarBuilder, var.name);
		StringBuilder__appendString(&defVarBuilder, "\n");
	} else {
		StringBuilder__appendString(&instructionBuilder, "DEFVAR TF@");
		StringBuilder__appendString(&instructionBuilder, var.name);
		StringBuilder__appendString(&instructionBuilder, "\n");
	}
}

void emit_DEFVAR_end() {
	printf("%s", defVarBuilder.text);
	StringBuilder__free(&defVarBuilder);
}

void emit_CALL(char * label) {
	StringBuilder__appendString(&instructionBuilder, "CALL ");
	StringBuilder__appendString(&instructionBuilder, label);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_RETURN() {
	StringBuilder__appendString(&instructionBuilder, "RETURN\n");
}

void emit_PUSHS(Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "PUSHS ");
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_POPS(Var var) {
	StringBuilder__appendString(&instructionBuilder, "POPS ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_CLEARS() {
	StringBuilder__appendString(&instructionBuilder, "CLEARS\n");
}

void emit_ADD(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "ADD ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_SUB(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "SUB ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_MUL(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "MUL ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_DIV(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "DIV ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_IDIV(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "IDIV ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_ADDS() {
	StringBuilder__appendString(&instructionBuilder, "ADDS\n");
}

void emit_SUBS() {
	StringBuilder__appendString(&instructionBuilder, "SUBS\n");
}

void emit_MULS() {
	StringBuilder__appendString(&instructionBuilder, "MULS\n");
}

void emit_DIVS() {
	StringBuilder__appendString(&instructionBuilder, "DIVS\n");
}

void emit_IDIVS() {
	StringBuilder__appendString(&instructionBuilder, "IDIVS\n");
}

void emit_LT(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "LT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_GT(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "GT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_EQ(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "EQ ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_LTS() {
	StringBuilder__appendString(&instructionBuilder, "LTS\n");
}

void emit_GTS() {
	StringBuilder__appendString(&instructionBuilder, "GTS\n");
}

void emit_EQS() {
	StringBuilder__appendString(&instructionBuilder, "EQS\n");
}

void emit_AND(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "AND ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_OR(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "OR ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_NOT(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "NOT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_ANDS() {
	StringBuilder__appendString(&instructionBuilder, "ANDS\n");
}

void emit_ORS() {
	StringBuilder__appendString(&instructionBuilder, "ORS\n");
}

void emit_NOTS() {
	StringBuilder__appendString(&instructionBuilder, "NOTS\n");
}

void emit_INT2FLOAT(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "INT2FLOAT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_FLOAT2INT(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "FLOAT2INT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_INT2CHAR(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "INT2CHAR ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_STRI2INT(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "STRI2INT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_INT2BOOL(Var var, Symb symb1) {
	StringBuilder__appendString(&instructionBuilder, "INT2BOOL ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_FLOAT2BOOL(Var var, Symb symb1) {
	StringBuilder__appendString(&instructionBuilder, "FLOAT2BOOL ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_STRI2BOOL(Var var, Symb symb1) {
	StringBuilder__appendString(&instructionBuilder, "STRI2BOOL ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_INT2FLOATS() {
	StringBuilder__appendString(&instructionBuilder, "INT2FLOATS\n");
}

void emit_FLOAT2INTS() {
	StringBuilder__appendString(&instructionBuilder, "FLOAT2INTS\n");
}

void emit_INT2CHARS() {
	StringBuilder__appendString(&instructionBuilder, "INT2CHARS\n");
}

void emit_STRI2INTS() {
	StringBuilder__appendString(&instructionBuilder, "STRI2INTS\n");
}

void emit_READ(Var var, StorageType type) {
	StringBuilder__appendString(&instructionBuilder, "READ ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_type(type);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_WRITE(Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "WRITE ");
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_CONCAT(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "CONCAT ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_STRLEN(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "STRLEN ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_GETCHAR(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "GETCHAR ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_SETCHAR(Var var, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "SETCHAR ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_TYPE(Var var, Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "TYPE ");
	emit_var(var);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_LABEL(char *label) {
	StringBuilder__appendString(&instructionBuilder, "LABEL ");
	StringBuilder__appendString(&instructionBuilder, label);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_JUMP(char *label) {
	StringBuilder__appendString(&instructionBuilder, "JUMP ");
	StringBuilder__appendString(&instructionBuilder, label);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_JUMPIFEQ(char *label, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "JUMPIFEQ ");
	emit_label(label);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_JUMPIFNEQ(char *label, Symb symb1, Symb symb2) {
	StringBuilder__appendString(&instructionBuilder, "JUMPIFNEQ ");
	emit_label(label);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb1);
	StringBuilder__appendChar(&instructionBuilder, ' ');
	emit_symb(symb2);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_JUMPIFEQS(char * label) {
	StringBuilder__appendString(&instructionBuilder, "JUMPIFEQS ");
	emit_label(label);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_JUMPIFNEQS(char * label) {
	StringBuilder__appendString(&instructionBuilder, "JUMPIFNEQS ");
	emit_label(label);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_EXIT(Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "EXIT ");
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_BREAK() {
	StringBuilder__appendString(&instructionBuilder, "BREAK\n");
}

void emit_DPRINT(Symb symb) {
	StringBuilder__appendString(&instructionBuilder, "DPRINT ");
	emit_symb(symb);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}

void emit_COMMENT(char *comment) {
	StringBuilder__appendString(&instructionBuilder, "# ");
	StringBuilder__appendString(&instructionBuilder, comment);
	StringBuilder__appendChar(&instructionBuilder, '\n');
}