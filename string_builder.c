#include "string_builder.h"

StringBuilder* StringBuilder__init(StringBuilder* this) {
    this->text = malloc(128);
    this->text[0] = '\0';
    this->length = 0;
    this->capacity = 128;
    return this;
}

void StringBuilder__free(StringBuilder* this) {
    free(this->text);
}

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

StringBuilder* StringBuilder__appendChar(StringBuilder *this, char c) {
    char str[2] = {c, '\0'};
    return StringBuilder__appendString(this, str);
}

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
        } else {
            StringBuilder__appendChar(this, c);
        }
    }
    return this;
}

StringBuilder* StringBuilder__appendInt(StringBuilder *this, int i) {
    char str[12];
    sprintf(str, "%d", i);
    return StringBuilder__appendString(this, str);
}

StringBuilder* StringBuilder__appendFloat(StringBuilder *this, float f) {
    char str[20];
    sprintf(str, "%f", f);
    return StringBuilder__appendString(this, str);
}

StringBuilder* StringBuilder__removeLastChar(StringBuilder* this) {
    if(this->length > 0) {
        this->length--;
        this->text[this->length] = '\0';
    }
    return this;
}