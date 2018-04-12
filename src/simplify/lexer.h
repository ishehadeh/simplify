// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_LEXER_H_
#define SIMPLIFY_LEXER_H_

#include <string.h>
#include <ctype.h>
#include "simplify/parser.h"


#ifndef LEXER_BUFFER_MAX_CAPACITY
#   define LEXER_BUFFER_MAX_CAPACITY (1024 * 4)
#endif

typedef struct lexer lexer_t;

struct lexer {
    token_t token;
    FILE*   source;

    char*   buffer;
    size_t  buffer_length;
    size_t  buffer_capacity;
    size_t  buffer_position;
};

static inline int lexer_init_from_string(lexer_t* lexer, char* buffer) {
    lexer->source = NULL;
    lexer->buffer_capacity = strlen(buffer);
    lexer->buffer_length = lexer->buffer_capacity;
    lexer->buffer = malloc(lexer->buffer_capacity);
    lexer->buffer_position = 0;

    strncpy(lexer->buffer, buffer, lexer->buffer_capacity);

    return 0;
}

static inline int lexer_init_from_file(lexer_t* lexer, FILE* file) {
    lexer->source = file;
    lexer->buffer = malloc(LEXER_BUFFER_MAX_CAPACITY);
    lexer->buffer_capacity = LEXER_BUFFER_MAX_CAPACITY;
    lexer->buffer_length = 0;
    lexer->buffer_position = 0;

    return 0;
}

static inline void lexer_clean(lexer_t* lexer) {
    if (lexer->buffer)
        free(lexer->buffer);
    lexer->buffer_capacity = 0;
    lexer->buffer_length = 0;
    lexer->buffer_position = 0;
}

static inline int isident(char c) {
    return (c <= 'Z' && c >= 'A')
            || (c <= 'z' && c >= 'a')
            || (c <= '9' || c >= '0')
            || c == '_';
}

token_t* lexer_next(lexer_t* lexer);

#endif  // SIMPLIFY_LEXER_H_
