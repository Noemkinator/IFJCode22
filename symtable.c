/**
 * @brief Implementace tabulky symbolů
 * @author Jakub Kratochvíl (xkrato67)
 */
 
#include "symtable.h"

int hash(char* str) {
    int hash = 0;
    char c = 0;
    // change later
    for(int i=0; i < strlen(str); ++i) {
        c = str[i];
        hash += c - '0'; 
        hash ^= 0x1234;
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

Item_t* tb_lookup(char* str, Block_t* top_b) {
    int h_id = hash(str);
    Item_t* temp = top_b->tb[h_id];
    while(temp != NULL && strncmp(top_b->tb[h_id]->data->name, str, NM_SIZE) != 0) {
        temp = temp->next;
    }
    // if not found in this table, check next
    if(temp == NULL && top_b->next != NULL) {
        temp = tb_lookup(str, top_b->next);
    }
    return temp;
}

Item_t* tb_remove(char* str, Block_t* b) {
    int h_id = hash(str);
    Item_t* temp = b->tb[h_id];
    Item_t* prev = NULL;

    while(temp != NULL && strncmp(b->tb[h_id]->data->name, str, NM_SIZE) != 0) {
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

void debug_insert(Block_t* b, char* str) {
    Item_t* hello = malloc(sizeof(Item_t));
    hello->data = malloc(sizeof(Data_t));
    hello->next = NULL;

    hello->data->data_type = Type_string;
    hello->data->name = str;

    if(tb_insert(hello, b) == 1) {
        puts("error insert1");
    } 
}

void debug_lookup(Block_t* top_b, char* str) {
    puts("-----LOOKUP-----");
    Item_t* abc = tb_lookup(str, top_b);
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
    Block_t* b1 = block_init(b0);

    puts("[b1]"); debug_print(b1);
    puts("[b0]"); debug_print(b0);

    debug_insert(b0, "abc");
    debug_insert(b1, "def");

    puts("[b1]"); debug_print(b1);
    puts("[b0]"); debug_print(b0);

    debug_lookup(b1, "abc");
    debug_lookup(b1, "def");

    debug_remove(b1, "abc");
    debug_remove(b1, "def");

    puts("[b1]"); debug_print(b1);
    puts("[b0]"); debug_print(b0);

    return 0;
}


