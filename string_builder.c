/**
 * Implementace překladače imperativního jazyka IFJ22
 * @file string_builder.c
 * @author Jiří Gallo (xgallo04)
 * @brief String builder
 * @date 2022-10-27
 */

#include "string_builder.h"

/**
 * @brief Initialize string builder object
 * 
 * @param builder 
 */
StringBuilder* StringBuilder__init(StringBuilder* this) {
    this->text = malloc(128);
    this->text[0] = '\0';
    this->length = 0;
    this->capacity = 128;
    return this;
}

/**
 * @brief String builder destructor
 * 
 * @param this 
 */
void StringBuilder__free(StringBuilder* this) {
    free(this->text);
}

/**
 * @brief Append string to string builder
 * 
 * @param builder 
 * @param string 
 * @return true if append was successful
 * @return false if append failed
 */
StringBuilder* StringBuilder__appendString(StringBuilder *this, const char *str) {
    size_t len = strlen(str);
    while(this->length + len >= this->capacity) {
        this->capacity = this->capacity * 2;
        this->text = realloc(this->text, this->capacity);
    }
    memcpy(this->text + this->length, str, len+1);
    this->length += len;
    return this;
}

/**
 * @brief Append char to string builder
 * 
 * @param this 
 * @param c 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendChar(StringBuilder *this, char c) {
    char str[2] = {c, '\0'};
    return StringBuilder__appendString(this, str);
}

/**
 * @brief Append escaped string to string builder
 * 
 * @param this 
 * @param i 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendEscapedStr(StringBuilder *this, const char *str) {
    int len = strlen(str);
    for(int i = 0; i < len; ++i) {
        char c = str[i];
        if(c == '\\') {
            StringBuilder__appendChar(this, '\\');
            StringBuilder__appendChar(this, '\\');
        } else if(c == '\"') {
            StringBuilder__appendChar(this, '\\');
            StringBuilder__appendChar(this, '\"');
        } else if(c == '\n') {
            StringBuilder__appendChar(this, '\\');
            StringBuilder__appendChar(this, 'n');
        } else {
            StringBuilder__appendChar(this, c);
        }
    }
    return this;
}

/**
 * @brief Append int to string builder
 * 
 * @param this 
 * @param i 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendInt(StringBuilder *this, long long int i) {
    char str[128];
    sprintf(str, "%lld", i);
    return StringBuilder__appendString(this, str);
}

/**
 * @brief Append double to string builder
 * 
 * @param this 
 * @param f 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendFloat(StringBuilder *this, double f) {
    char str[128];
    sprintf(str, "%a", f);
    return StringBuilder__appendString(this, str);
}

/**
 * @brief Remove last character from string builder
 * 
 * @param this 
 * @param b 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__removeLastChar(StringBuilder* this) {
    if(this->length > 0) {
        this->length--;
        this->text[this->length] = '\0';
    }
    return this;
}