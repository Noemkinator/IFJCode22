/**
 * @brief Implementace tabulky symbolů
 * @author Jakub Kratochvíl (xkrato67)
 */
 
#include "symtable.h"

Tb_item* tb[TB_SIZE];

int hash(char* str) {
    int hash = 0;
    char c = 0;
    for(int i=0; i < strlen(str); ++i) {
        c = str[i];
        hash += c - '0'; 
        hash ^= 0x1234;
    }
    return hash % TB_SIZE;
}

void tb_init() {
    for(int i=0; i < TB_SIZE; ++i) {
        tb[i] = NULL;
    }
}

int tb_insert(Tb_item* tb_item) {
    if(tb_item == NULL) {
        return 1;
    } 
    int h_id = hash(tb_item->data->name);
    tb_item->next = tb[h_id];
    tb[h_id] = tb_item;
    return 0;
}

Tb_item* tb_lookup(char* str) {
    int h_id = hash(str);
    Tb_item* temp = tb[h_id];
    while(temp != NULL && strncmp(tb[h_id]->data->name, str, NM_SIZE) != 0) {
        temp = temp->next;
    }
    return temp;
}

Tb_item* tb_remove(char* str) {
    int h_id = hash(str);
    Tb_item* temp = tb[h_id];
    Tb_item* prev = NULL;

    while(temp != NULL && strncmp(tb[h_id]->data->name, str, NM_SIZE) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if(temp == NULL) {
        return NULL;
    }
    if(prev == NULL) {
        tb[h_id] = temp->next;
    } else {
        prev->next = temp->next;
    }
    return temp;
}

void debug_print() {
    for(int i=0; i < TB_SIZE; ++i) {
        if(tb[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t", i);
            Tb_item* temp = tb[i];
            while(temp != NULL) {
                printf("%s -> ", temp->data->name);
                temp = temp->next;
            }
            puts("");
        }
    }
    puts("----------------------------------------");
}

void debug_insert() {
    Tb_item* hello = malloc(sizeof(Tb_item*));
    hello->data = malloc(sizeof(Item_data*));
    hello->next = NULL;

    hello->data->data_type = Type_string;
    hello->data->name = "helloo";

    Tb_item* world = malloc(sizeof(Tb_item*));
    world->data = malloc(sizeof(Item_data*));
    world->next = NULL;

    world->data->data_type = Type_string;
    world->data->name = "world";

    if(tb_insert(hello) == 1) {
        puts("error insert1");
    } 
    if(tb_insert(world) == 1) {
        puts("error insert2");
    }
}
void debug_lookup() {
    puts("-----LOOKUP-----");
    char* str1 = "world";
    Tb_item* abc = tb_lookup(str1);
    if(abc != NULL) {
        printf("Found: %s\n", str1);
    } else {
        printf("Not found: %s\n", str1);
    }

    char* str2 = "lkdlsl";
    Tb_item* cba = tb_lookup(str2);
    if(cba != NULL) {
        printf("Found: %s\n", str2);
    } else {
        printf("Not found: %s\n", str2);
    }
}

void debug_remove() {
    puts("-----REMOVE-----");
    char* str1 = "ABC";
    Tb_item* aaa = tb_remove(str1);
    if(aaa != NULL) {
        printf("Removed: %s\n", str1);
    } else {
        printf("Couldn't find: %s\n", str1);
    }

    char* str2 = "world";
    Tb_item* bbb = tb_remove(str2);
    if(bbb != NULL) {
        printf("Removed: %s\n", str2);
    } else {
        printf("Couldn't find: %s\n", str2);
    }

}

int main(void) {
    tb_init();
    debug_print();
    debug_insert();
    debug_lookup();
    debug_print();
    debug_remove();
    debug_print();
    return 0;
}


