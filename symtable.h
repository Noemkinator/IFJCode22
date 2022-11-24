/**
 * @file symtable.h
 * @authors Jakub Kratochvíl (xkrato67), Jiří Gallo (xgallo04)
 * @brief Hlavičkový soubor pro implementaci tabulky symbolů (symtable.c) 
 * @date 2022-10-22
 */

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#define TB_SIZE 256

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Data type of symbol table item
 */
typedef struct TableItem {
    char* name;
    void* data;
    struct TableItem* next;
} TableItem;

typedef struct Table {
   TableItem* tb[TB_SIZE];
} Table;

/**
 * @brief Initialize symbol table
 */
Table* table_init();
/**
 * @brief create hash value from string
 * 
 * @param str
 * @return int 
 */
int hash(char* str);
/**
 * @brief Insert item to symbol table
 * 
 * @param table 
 * @param name 
 * @param value
 * @return TableItem* 
 */
TableItem* table_insert(Table* b, char * name, void * value);
/**
 * @brief Find item in symbol table
 * 
 * @param table 
 * @param name 
 * @return TableItem* 
 */
TableItem* table_find(Table* b, char* str);
/**
 * @brief Delete item from symbol table
 * 
 * @param table 
 * @param name 
 * @return TableItem* 
 */
TableItem* table_remove(Table* b, char* str);
/**
 * @brief Free symbol table
 * 
 * @param table
 */
void table_free(Table* b);
/**
 * @brief Free item from symbol table
 * 
 * @param TableItem* 
 */
void item_free(TableItem* i);

// DEBUG
void debug_print(Table* b);

#endif // __SYMTABLE_H__

