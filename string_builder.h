/**
 * @file string_builder.h
 * @author Jiří Gallo (xgallo04)
 * @brief String builder library
 * @date 2022-10-27
 */
#ifndef __STRING_BUILDER_H__
#define __STRING_BUILDER_H__

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief String builder structure
 */
typedef struct {
    char *text; /*<Pointer to the text>*/
    size_t length;/*<Length of the text>*/
    size_t capacity;/*<Capacity of the string>*/
} StringBuilder;

StringBuilder* StringBuilder__init(StringBuilder *this);
void StringBuilder__free(StringBuilder *this);
StringBuilder* StringBuilder__appendString(StringBuilder *this, const char *str);
StringBuilder* StringBuilder__appendChar(StringBuilder *this, char c);
StringBuilder* StringBuilder__appendEscapedStr(StringBuilder *this, const char *str);
StringBuilder* StringBuilder__appendInt(StringBuilder *this, long long int i);
StringBuilder* StringBuilder__appendFloat(StringBuilder *this, double f);
StringBuilder* StringBuilder__removeLastChar(StringBuilder* this);

#endif // __STRING_BUILDER_H__
