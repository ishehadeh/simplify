/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_LEXER_H_
#define SIMPLIFY_LEXER_H_

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "simplify/errors.h"

/* lexical analyzer
 * The lexer walks through a buffer or file, picking out tokens
 */
typedef struct lexer lexer_t;

/* A token labels part of a buffer
 */
typedef struct token        token_t;

/* Enumerates the different kinds of tokens
 */
typedef enum   token_type   token_type_t;

enum token_type {
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_LEFT_PAREN,
    TOKEN_TYPE_RIGHT_PAREN,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_EOF,
};

struct token {
    token_type_t type;

    char*  start;
    size_t length;
};


struct lexer {
    token_t token;
    FILE*   source;

    char*   buffer;
    size_t  buffer_length;
    size_t  buffer_position;
};

/* initialize lexer from a null terminated string 
 * @lexer the lexer to initialize
 * @buffer the buffer to read, the buffer is copied
 */
static inline void lexer_init_from_string(lexer_t* lexer, char* buffer) {
    lexer->source = NULL;
    lexer->buffer_length = strlen(buffer);
    lexer->buffer = malloc(lexer->buffer_length);
    lexer->buffer_position = 0;

    strncpy(lexer->buffer, buffer, lexer->buffer_length);
}

/* initialize a lexer from a file
 * @lexer the lexer to initialize
 * @file the file read
 */
static inline void lexer_init_from_file(lexer_t* lexer, FILE* file) {
    fseek(file, 0, SEEK_END);
    size_t len = (size_t)ftell(file);

    lexer->source = file;
    lexer->buffer = malloc(len + 1);
    lexer->buffer_length = len;
    lexer->buffer_position = 0;

    lexer->buffer[len] = 0;
    fseek(file, 0, SEEK_SET);
    fread(lexer->buffer, len, 1, file);
}

/* free all the lexer's resources, except the file
 * @lexer the lexer to clean
 */
static inline void lexer_clean(lexer_t* lexer) {
    if (lexer->buffer)
        free(lexer->buffer);
    lexer->buffer_length = 0;
    lexer->buffer_position = 0;
}


/* draw the next token from the lexer
 * @lexer the lexer to draw from
 * @token a location to store the token
 * @return returns an error code
 */
error_t lexer_next(lexer_t* lexer, token_t* token);

/* check if a character is a valid identifier
 * @c the character to check
 * @return a boolean integer, true if `c` is valid
 */
static inline int isident(char c) {
    return (c <= 'Z' && c >= 'A')
            || (c <= 'z' && c >= 'a')
            || c == '_';
}

#endif  // SIMPLIFY_LEXER_H_
