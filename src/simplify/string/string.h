/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_STRING_STRING_H_
#define SIMPLIFY_STRING_STRING_H_

#include <string.h>
#include <stdlib.h>

#ifndef STRING_DEFAULT_CAPACITY
#   define STRING_DEFAULT_CAPACITY 512
#endif

/* string is a dynamically sized, ASCII encoded string */
typedef struct string string_t;

struct string {
    size_t cap;
    size_t len;
    char*  buffer;
};

/* initialize a string with the default capacity (this is a compile-time option) */
void string_init(string_t* string);

/* initialize a string with a custom initial capacity */
void string_init_cap(string_t* string, size_t cap);

/* get the length of a string */
static inline size_t string_len(string_t* string) { return string->len; }

/* get the capacity of a string */
static inline size_t string_cap(string_t* string) { return string->cap; }

/* grow the string's capacity */
static inline void string_grow(string_t* string) {
    string->buffer = realloc(string->buffer, string->cap *= 2);
}

/* grow a string to fit an extra `size` bytes */
static inline void string_fit(string_t* string, size_t size) {
    while (string_cap(string) <= string_len(string) + size) {
        string_grow(string);
    }
}

/* copy `len` chars from a buffer, and append them to the end of the string
 *
 * @string
 * @buf the buffer to draw from
 * @len the number of chars to pull
 */ 
void string_append_buffer(string_t* string, char* buf, size_t len);

/* append a NULL terminated char* to the end of the string
 *
 * @string
 * @str the c string to copy
 */
void string_append_cstring(string_t* string, char* str);

/* push a character onto the end of a string
 *
 * @string
 * @c the char to push
 */
void string_append_char(string_t* string, char c);

/* insert a character into a string
 *
 * @string
 * @index the index to insert the character
 * @c the character to insert
 */
void string_insert_char(string_t* string, size_t index, char c);

/* insert a NULL terminated c string into a string
 *
 * @string
 * @index the index to insert the character
 * @c the string to insert
 */
void string_insert_cstr(string_t* string, size_t index, char* c);

/* free a string's internal buffer 
 *
 * @string
 */
static inline void string_clean(string_t* string) {
    free(string->buffer);
}

#endif  // SIMPLIFY_STRING_STRING_H_
