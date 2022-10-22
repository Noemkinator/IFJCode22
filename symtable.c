/**
 * @brief Implementace tabulky symbolů
 * @author Jakub Kratochvíl (xkrato67)
 * @author Jiří Gallo (xgallo04)
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

int tb_insert(Block_t* b, Item_t* tb_item) {
    if(tb_item == NULL) {
        return 1;
    } 
    int h_id = hash(tb_item->name);
    tb_item->next = b->tb[h_id];
    b->tb[h_id] = tb_item;
    return 0;
}

Item_t* tb_lookup(Block_t* b, char* str) {
    int h_id = hash(str);
    Item_t* temp = b->tb[h_id];

    while(temp != NULL && strncmp(temp->name, str, sizeof(str)) != 0) {
        temp = temp->next;
    }
    return temp;
}

Item_t* tb_remove(Block_t* b, char* str) {
    int h_id = hash(str);
    Item_t* temp = b->tb[h_id];
    Item_t* prev = NULL;
    
    
    while(temp != NULL && strncmp(temp->name, str, sizeof(str)) != 0) {
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

/*void debug_print(Block_t* b) {
    for(int i=0; i < TB_SIZE; ++i) {
        if(b->tb[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t", i);
            Item_t* temp = b->tb[i];
            while(temp != NULL) {
                printf("%s -> ", temp->name);
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
    hello->name = str;

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
    Item_t* items[11] = {NULL}; 
    char strings[11][2] = {"a","b","c","d","f","g","h","i","j","k","l"};

    for(int i=0; i<11; ++i){
        items[i] = debug_insert(b0, strings[i]);
    }

    puts("[b0]"); debug_print(b0);

    debug_lookup(b0, "g");
    debug_remove(b0, "g");
    debug_lookup(b0, "g");

    puts("[b0]"); debug_print(b0);

    for(int i=0; i<11; ++i){
        free(items[i]->data);
        free(items[i]);
    }

    free(b0);

    return 0;
}*/


