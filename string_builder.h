#ifndef __STRING_BUILDER_H__
#define __STRING_BUILDER_H__

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} StringBuilder;

StringBuilder* StringBuilder__init(StringBuilder *this);
void StringBuilder__free(StringBuilder *this);
StringBuilder* StringBuilder__appendString(StringBuilder *this, const char *str);
StringBuilder* StringBuilder__appendChar(StringBuilder *this, char c);
StringBuilder* StringBuilder__appendEscapedStr(StringBuilder *this, const char *str);
StringBuilder* StringBuilder__appendInt(StringBuilder *this, int i);
StringBuilder* StringBuilder__appendFloat(StringBuilder *this, float f);
StringBuilder* StringBuilder__removeLastChar(StringBuilder* this);

#endif // __STRING_BUILDER_H__
