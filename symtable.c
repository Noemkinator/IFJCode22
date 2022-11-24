/**
 * @file symtable.c
 * @authors Jakub Kratochvíl (xkrato67), Jiří Gallo (xgallo04)
 * @brief Hash table implementation
 * @date 2022-10-22
 */
 
#include "symtable.h"

/**
 * @brief Generates hash from given string
 * @param str
 * @return hash
 * 
 * http://www.cse.yorku.ca/~oz/hash.html
 */
int hash(char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != 0) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TB_SIZE;
}

/**
 * @brief Initializes new hash table
 * @return pointer to hash table
 */
Table* table_init() {
    Table* b = malloc(sizeof(Table));
    for(int i=0; i < TB_SIZE; ++i) {
        b->tb[i] = NULL;
    }
    return b;
}

/**
 * @brief Allocates new item and inserts data into it in specified hash table
 * @param b hash table
 * @param name 
 * @param value pointer to data
 * @return pointer to inserted item
 * @warning returns NULL if memory couldn't be allocated
 */
TableItem* table_insert(Table* b, char * name, void * value) {
    TableItem* tb_item = malloc(sizeof(TableItem));
    if(tb_item == NULL) return tb_item;
    tb_item->name = name;
    tb_item->data = value;
    int h_id = hash(tb_item->name);
    tb_item->next = b->tb[h_id];
    b->tb[h_id] = tb_item;
    return tb_item;
}

/**
 * @brief Looks for item in hash table
 * @param b hash table
 * @param str 
 * @return pointer to item
 * @warning if item wasn't found returns NULL
 */
TableItem* table_find(Table* b, char* str) {
    int h_id = hash(str);
    TableItem* temp = b->tb[h_id];

    while(temp != NULL && strcmp(temp->name, str) != 0) {
        temp = temp->next;
    }
    return temp;
}

/**
 * @brief Removes item from hash table
 * @param b hash table
 * @param str 
 * @return pointer to item
 * @warning if item wasn't found returns NULL
 */
TableItem* table_remove(Table* b, char* str) {
    int h_id = hash(str);
    TableItem* temp = b->tb[h_id];
    TableItem* prev = NULL;
    
    
    while(temp != NULL && strcmp(temp->name, str) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if(temp == NULL) {
        return NULL;
    }
    if(prev == NULL) {
        b->tb[h_id] = temp->next;
    } else {
        prev->next = temp->next;
    }
    return temp;
}

/**
 * @brief Frees the hash table and all it's items
 * @param b hash table
 */
void table_free(Table* b) {
    if(b == NULL) return;
    TableItem* temp = NULL;

    for(int i=0; i < TB_SIZE; ++i){
        while(b->tb[i] != NULL) {
            temp = b->tb[i];
            b->tb[i] = b->tb[i]->next;
            free(temp);
        }
    }
    free(b);
}
/*
void debug_print(Table* b) {
    for(int i=0; i < TB_SIZE; ++i) {
        if(b->tb[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t", i);
            TableItem* temp = b->tb[i];
            while(temp != NULL) {
                printf("%s -> ", temp->name);
                temp = temp->next;
            }
            puts("");
        }
    }
    puts("----------------------------------------");
}
*/
