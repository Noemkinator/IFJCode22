/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file pointer_hashtable.h
 * @authors Jakub Kratochvíl (xkrato67), Jiří Gallo (xgallo04)
 * @brief Header file for pointer_hashtable.c
 */

#ifndef __HASHTABLE_STATEMENT_H
#define __HASHTABLE_STATEMENT_H

#define TB_SIZE 256

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Data type of symbol table item
 */
typedef struct PointerTableItem {
    void* name;
    void* data;
    struct PointerTableItem* next;
} PointerTableItem;

typedef struct PointerTable {
   PointerTableItem* tb[TB_SIZE];
} PointerTable;

/**
 * @brief Initialize symbol table
 */
PointerTable* pointer_table_init();

/**
 * @brief Insert item to symbol table
 * 
 * @param table 
 * @param ptr 
 * @param value
 * @return PointerTableItem* 
 */
PointerTableItem* pointer_table_insert(PointerTable* b, void * ptr, void * value);
/**
 * @brief Find item in symbol table
 * 
 * @param table 
 * @param ptr 
 * @return PointerTableItem* 
 */
PointerTableItem* pointer_table_find(PointerTable* b, void* ptr);
/**
 * @brief Delete item from symbol table
 * 
 * @param table 
 * @param ptr 
 * @return PointerTableItem* 
 */
PointerTableItem* pointer_table_remove(PointerTable* b, void* ptr);
/**
 * @brief Free symbol table
 * 
 * @param table
 */
void pointer_table_free(PointerTable* b);

#endif /* __HASHTABLE_STATEMENT_H */

