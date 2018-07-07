/* Copyright Ian Shehadeh 2018 */

#include "simplify/string/string.h"
#include <stdlib.h>

/* grow the string's capacity */
static inline void string_grow(string_t* string) { string->buffer = realloc(string->buffer, string->cap *= 2); }

/* grow a string to fit an extra `size` bytes */
static inline void string_fit(string_t* string, size_t size) {
    while (string_cap(string) <= string_len(string) + size) {
        string_grow(string);
    }
}

void string_init(string_t* string) { string_init_cap(string, STRING_DEFAULT_CAPACITY); }

void string_init_cap(string_t* string, size_t cap) {
    string->cap = cap;
    string->len = 0;
    string->buffer = malloc(cap);
}

void string_append_buffer(string_t* string, char* buffer, size_t len) {
    string_fit(string, len);
    memcpy(string->buffer + string_len(string), buffer, len);
    string->len += len;
}

void string_append_cstring(string_t* string, char* str) { string_append_buffer(string, str, strlen(str)); }

void string_append_char(string_t* string, char c) {
    string_fit(string, 1);
    string->buffer[string_len(string)] = c;
    ++string->len;
}

void string_insert_char(string_t* string, size_t index, char c) {
    string_fit(string, 1);
    memmove(string->buffer + index + 1, string->buffer + index, string_len(string) - index);
    string->buffer[index] = c;
}

void string_insert_cstr(string_t* string, size_t index, char* str) {
    size_t len = strlen(str);
    string_fit(string, len);
    memmove(string->buffer + index + len, string->buffer + index, string_len(string) - index);
    memcpy(string->buffer + index, str, len);
}

