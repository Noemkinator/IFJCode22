// Implementace překladače imperativního jazyka IFJ22
// Authors: Jiří Gallo (xgallo04)

#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <stdbool.h>

typedef enum {
	Type_int,
	Type_bool,
	Type_float,
	Type_string,
	Type_null,
	Type_variable
} StorageType;

typedef enum {
	LF,
	TF,
	GF
} FrameType;

typedef struct {
	FrameType frameType;
	char * name;
} Var;

typedef struct {
	StorageType type;
	union {
		long long int i;
		double f;
		char * s;
		bool b;
		Var v;
	} value;
} Symb;

void emit_header();
void emit_instruction_start();
void emit_instruction_end();
void emit_MOVE(Var var, Symb symb);
void emit_CREATEFRAME();
void emit_PUSHFRAME();
void emit_POPFRAME();
void emit_DEFVAR_start();
void emit_DEFVAR(Var var);
void emit_DEFVAR_end();
void emit_CALL(char * label);
void emit_RETURN();
void emit_PUSHS(Symb symb);
void emit_POPS(Var var);
void emit_CLEARS();
void emit_ADD(Var var, Symb symb1, Symb symb2);
void emit_SUB(Var var, Symb symb1, Symb symb2);
void emit_MUL(Var var, Symb symb1, Symb symb2);
void emit_DIV(Var var, Symb symb1, Symb symb2);
void emit_IDIV(Var var, Symb symb1, Symb symb2);
void emit_ADDS();
void emit_SUBS();
void emit_MULS();
void emit_DIVS();
void emit_IDIVS();
void emit_LT(Var var, Symb symb1, Symb symb2);
void emit_GT(Var var, Symb symb1, Symb symb2);
void emit_EQ(Var var, Symb symb1, Symb symb2);
void emit_LTS();
void emit_GTS();
void emit_EQS();
void emit_AND(Var var, Symb symb1, Symb symb2);
void emit_OR(Var var, Symb symb1, Symb symb2);
void emit_NOT(Var var, Symb symb);
void emit_ANDS();
void emit_ORS();
void emit_NOTS();
void emit_INT2FLOAT(Var var, Symb symb);
void emit_FLOAT2INT(Var var, Symb symb);
void emit_INT2CHAR(Var var, Symb symb);
void emit_STRI2INT(Var var, Symb symb1, Symb symb2);
void emit_INT2FLOATS();
void emit_FLOAT2INTS();
void emit_INT2CHARS();
void emit_STRI2INTS();
void emit_INT2BOOL(Var var, Symb symb);
void emit_FLOAT2BOOL(Var var, Symb symb);
void emit_STRI2BOOL(Var var, Symb symb);
void emit_READ(Var var, StorageType type);
void emit_WRITE(Symb symb);
void emit_CONCAT(Var var, Symb symb1, Symb symb2);
void emit_STRLEN(Var var, Symb symb);
void emit_GETCHAR(Var var, Symb symb1, Symb symb2);
void emit_SETCHAR(Var var, Symb symb1, Symb symb2);
void emit_TYPE(Var var, Symb symb);
void emit_LABEL(char * label);
void emit_JUMP(char * label);
void emit_JUMPIFEQ(char * label, Symb symb1, Symb symb2);
void emit_JUMPIFNEQ(char * label, Symb symb1, Symb symb2);
void emit_JUMPIFEQS(char * label);
void emit_JUMPIFNEQS(char * label);
void emit_EXIT(Symb symb);
void emit_BREAK();
void emit_DPRINT(Symb symb);
void emit_COMMENT(char * comment);

void emit_init();
void emit_close();

#endif // __EMITTER_H__