/**
 * @brief Implementace tabulky symbolů
 * @author Jakub Kratochvíl (xkrato67)
 */
 
#include "symtable.h"

int hash(char* str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TB_SIZE;
}

Block_t* block_init(Block_t* next) {
    Block_t* b = malloc(sizeof(Block_t));
    if(next != NULL) {
        b->next = next;
    }
    for(int i=0; i < TB_SIZE; ++i) {
        b->tb[i] = NULL;
    }
    return b;
}

int tb_insert(Item_t* tb_item, Block_t* b) {
    if(tb_item == NULL) {
        return 1;
    } 
    int h_id = hash(tb_item->data->name);
    tb_item->next = b->tb[h_id];
    b->tb[h_id] = tb_item;
    return 0;
}

Item_t* tb_lookup(char* str, Block_t* b) {
    int h_id = hash(str);
    Item_t* temp = b->tb[h_id];

    while(temp != NULL && strncmp(b->tb[h_id]->data->name, str, sizeof(str)) != 0) {
        temp = temp->next;
    }
    return temp;
}

Item_t* tb_remove(char* str, Block_t* b) {
    int h_id = hash(str);
    Item_t* temp = b->tb[h_id];
    Item_t* prev = NULL;
    
    while(temp != NULL && strncmp(b->tb[h_id]->data->name, str, sizeof(str)) != 0) {
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

void block_free(Block_t* b) {
    while(b->next != NULL) {
        free(b->next);
    }
    free(b);
}

void item_free(Item_t* i) {
    while(i->next != NULL) {
        free(i->next);
    }
    free(i->data);
    free(i);
}

void debug_print(Block_t* b) {
    for(int i=0; i < TB_SIZE; ++i) {
        if(b->tb[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t", i);
            Item_t* temp = b->tb[i];
            while(temp != NULL) {
                printf("%s -> ", temp->data->name);
                temp = temp->next;
            }
            puts("");
        }
    }
    puts("----------------------------------------");
}

Item_t* debug_insert(Block_t* b, char* str) {
    Item_t* hello = malloc(sizeof(Item_t));
    hello->data = malloc(sizeof(Data_t));
    hello->next = NULL;

    hello->data->data_type = Type_string;
    hello->data->name = str;

    if(tb_insert(hello, b) == 1) {
        puts("error insert1");
    } 
    return hello;
}

void debug_lookup(Block_t* b, char* str) {
    puts("-----LOOKUP-----");
    Item_t* abc = tb_lookup(str, b);
    if(abc != NULL) {
        printf("Found: %s\n", str);
    } else {
        printf("Not found: %s\n", str);
    }
}

void debug_remove(Block_t* b, char* str) {
    puts("-----REMOVE-----");
    Item_t* aaa = tb_remove(str, b);
    if(aaa != NULL) {
        printf("Removed: %s\n", str);
    } else {
        printf("Couldn't find: %s\n", str);
    }
}

int main(void) {
    Block_t* b0 = block_init(NULL);
    Item_t* inserted_item = NULL;

    puts("[b0]"); debug_print(b0);

    inserted_item = debug_insert(b0, "abc");

    puts("[b0]"); debug_print(b0);

    debug_lookup(b0, "abc");
    debug_lookup(b0, "def");

    debug_remove(b0, "abc");
    debug_remove(b0, "def");

    puts("[b0]"); debug_print(b0);

    item_free(inserted_item); 
    block_free(b0);

    return 0;
}


