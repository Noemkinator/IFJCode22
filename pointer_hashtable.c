/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file pointer_hashtable.c
 * @authors Jakub Kratochvíl (xkrato67), Jiří Gallo (xgallo04)
 * @brief Pointer hash table implementation
 */
 
#include "pointer_hashtable.h"

/**
 * @brief Generates hash from given pointer
 * @param ptr
 * @return hash
 * 
 * http://www.cse.yorku.ca/~oz/hash.html
 */
int hashPtr(void* ptr) {
    unsigned long hash = 5381;
    int c;

    for(size_t i=0; i<sizeof(ptr); i++) {
        c = ((char*)&ptr)[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TB_SIZE;
}

/**
 * @brief Initializes new hash table
 * @return pointer to hash table
 */
PointerTable* pointer_table_init() {
    PointerTable* b = malloc(sizeof(PointerTable));
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
PointerTableItem* pointer_table_insert(PointerTable* b, void * name, void * value) {
    if(pointer_table_find(b, name) != NULL) pointer_table_remove(b, name);
    PointerTableItem* tb_item = malloc(sizeof(PointerTableItem));
    if(tb_item == NULL) return tb_item;
    tb_item->name = name;
    tb_item->data = value;
    int h_id = hashPtr(tb_item->name);
    tb_item->next = b->tb[h_id];
    b->tb[h_id] = tb_item;
    return tb_item;
}

/**
 * @brief Looks for item in hash table
 * @param b hash table
 * @param ptr 
 * @return pointer to item
 * @warning if item wasn't found returns NULL
 */
PointerTableItem* pointer_table_find(PointerTable* b, void* ptr) {
    int h_id = hashPtr(ptr);
    PointerTableItem* temp = b->tb[h_id];

    while(temp != NULL && temp->name != ptr) {
        temp = temp->next;
    }
    return temp;
}

/**
 * @brief Removes item from hash table
 * @param b hash table
 * @param ptr 
 * @return pointer to item
 * @warning if item wasn't found returns NULL
 */
PointerTableItem* pointer_table_remove(PointerTable* b, void* ptr) {
    int h_id = hashPtr(ptr);
    PointerTableItem* temp = b->tb[h_id];
    PointerTableItem* prev = NULL;
    
    
    while(temp != NULL && temp->name != ptr) {
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
void pointer_table_free(PointerTable* b) {
    if(b == NULL) return;
    PointerTableItem* temp = NULL;

    for(int i=0; i < TB_SIZE; ++i){
        while(b->tb[i] != NULL) {
            temp = b->tb[i];
            b->tb[i] = b->tb[i]->next;
            free(temp);
        }
    }
    free(b);
}
