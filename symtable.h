/**
 * @brief Hlavičkový soubor pro implementaci tabulky symbolů (symtable.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#define TB_SIZE 1024
#define NM_SIZE 256

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
	Type_int,
	Type_bool,
	Type_float,
	Type_string,
	Type_variable
} Type;

typedef struct {
    int id;
    Type data_type;
    char* name;
} Item_data;

typedef struct Tb_item {
    Item_data* data;
    struct Tb_item* next;
} Tb_item;

extern Tb_item* tb[TB_SIZE];

void tb_init();
int hash(char* str);
int tb_insert(Tb_item* tb_item);
Tb_item* tb_lookup(char* str);
Tb_item* tb_remove(char* str);

// DEBUG
void debug_insert();
void debug_lookup();
void debug_remove();
void debug_print();

#endif // __SYMTABLE_H__

