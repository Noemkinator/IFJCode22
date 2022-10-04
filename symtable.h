/**
 * @brief Hlavičkový soubor pro implementaci tabulky symbolů (symtable.c) 
 * @author Jakub Kratochvíl (xkrato67)
 */

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#define TB_SIZE 24  // just for debug purposes

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
} Type_t;

typedef struct {
    int id;
    Type_t data_type;
    char* name;
} Data_t;

typedef struct Item_t {
    Data_t* data;
    struct Item_t* next;
} Item_t;

typedef struct Block_t {
   Item_t* tb[TB_SIZE];
   struct Block_t* next; 
} Block_t;

Block_t* block_init(Block_t* next);
int hash(char* str);
int tb_insert(Item_t* tb_item, Block_t* b);
Item_t* tb_lookup(char* str, Block_t* b);
Item_t* tb_remove(char* str, Block_t* b);
void block_free(Block_t* b);
void item_free(Item_t* i);

// DEBUG
void debug_print(Block_t* b);
Item_t* debug_insert(Block_t* b, char* str);
void debug_lookup(Block_t* b, char* str);
void debug_remove(Block_t* b, char* str);

#endif // __SYMTABLE_H__

