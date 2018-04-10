// Copyright Ian R. Shehadeh

#ifndef SIMPLIFY_PARSER_H_
#define SIMPLIFY_PARSER_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "simplify/expression.h"

#ifndef TOKEN_STREAM_BUFFER_INITIAL_CAPACITY
#   define TOKEN_STREAM_BUFFER_INITIAL_CAPACITY 50
#endif

typedef struct token_stream token_stream_t;
typedef struct token        token_t;
typedef enum   token_type   token_type_t;

enum token_type {
    TOKEN_TYPE_NULL,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_LEFT_PAREN,
    TOKEN_TYPE_RIGHT_PAREN,
};

struct token {
    token_type_t type;

    size_t length;
    char*  start;
};

struct token_stream {
    token_t* buffer;

    size_t   buffer_capacity;
    size_t   buffer_length;
    size_t   position;
};

int token_stream_init(token_stream_t* stream);
int token_stream_push(token_stream_t* stream, token_type_t type, char* start, size_t length);
int token_stream_clean(token_stream_t* stream);

token_t* token_stream_next(token_stream_t* stream);
token_t* token_stream_peek(token_stream_t* stream);

extern expression_t* parse_expression(token_stream_t*);

#endif  // SIMPLIFY_PARSER_H_
