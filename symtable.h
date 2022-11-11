/**
 * @brief Hlavičkový soubor pro implementaci tabulky symbolů (symtable.c) 
 * @author Jakub Kratochvíl (xkrato67)
 * @author Jiří Gallo (xgallo04)
 */

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#define TB_SIZE 256

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct TableItem {
    char* name;
    void* data;
    struct TableItem* next;
} TableItem;

typedef struct Table {
   TableItem* tb[TB_SIZE];
} Table;

Table* table_init();
int hash(char* str);
TableItem* table_insert(Table* b, char * name, void * value);
TableItem* table_find(Table* b, char* str);
TableItem* table_remove(Table* b, char* str);
void table_free(Table* b);
void item_free(TableItem* i);

// DEBUG
void debug_print(Table* b);

#endif // __SYMTABLE_H__

