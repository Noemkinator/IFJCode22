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

/**
 * @brief Initialize string builder object
 * 
 * @param builder 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__init(StringBuilder *this);
/**
 * @brief String builder destructor
 * 
 * @param this 
 */
void StringBuilder__free(StringBuilder *this);
/**
 * @brief Append string to string builder
 * 
 * @param builder 
 * @param string 
 * @return true if append was successful
 * @return false if append failed
 */
StringBuilder* StringBuilder__appendString(StringBuilder *this, const char *str);
/**
 * @brief Append char to string builder
 * 
 * @param this 
 * @param c 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendChar(StringBuilder *this, char c);
/**
 * @brief Append escaped string to string builder
 * 
 * @param this 
 * @param str 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendEscapedStr(StringBuilder *this, const char *str);
/**
 * @brief Append int to string builder
 * 
 * @param this 
 * @param i 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendInt(StringBuilder *this, long long int i);
/**
 * @brief Append double to string builder
 * 
 * @param this 
 * @param d 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__appendFloat(StringBuilder *this, double f);
/**
 * @brief Remove last character from string builder
 * 
 * @param this 
 * @return StringBuilder* 
 */
StringBuilder* StringBuilder__removeLastChar(StringBuilder* this);

#endif // __STRING_BUILDER_H__
