// Copyright Ian R. Shehadeh

#include <string.h>

#include "simplify/parser.h"

expression_t* parse_expression_prec(token_stream_t* stream, int precedence);
expression_t* parse_infix(token_stream_t* stream, expression_t* left, token_t* tok);
expression_t* parse_prefix(token_stream_t* stream, token_t* tok);

inline static int token_precedence(token_t* tok) {
    switch (tok->type) {
        case TOKEN_TYPE_OPERATOR:
        {
            switch (*tok->start) {
                case '=':
                case '>':
                case '<':
                    return 10;
                case '+':
                case '-':
                    return 20;
                case '*':
                case '/':
                    return 30;
                case '^':
                    return 40;
                default:
                    return -1;
            }
        }
        case TOKEN_TYPE_IDENTIFIER:
        case TOKEN_TYPE_NUMBER:
            return 50;
        default:
            return -1;
    }
}

int token_stream_init(token_stream_t* stream) {
    assert(stream != NULL);

    stream->buffer_capacity = TOKEN_STREAM_BUFFER_INITIAL_CAPACITY;
    stream->buffer_length = 0;
    stream->position = 0;

    stream->buffer = malloc(stream->buffer_capacity * sizeof(token_t));

    return !stream->buffer;
}

token_t* token_stream_next(token_stream_t* stream) {
    assert(stream != NULL);

    if (stream->position >= stream->buffer_length) {
        return NULL;
    }

    token_t* tok = stream->buffer + stream->position;
    ++stream->position;
    return tok;
}

token_t* token_stream_peek(token_stream_t* stream) {
    assert(stream != NULL);

    if (stream->position == stream->buffer_length) {
        return NULL;
    }

    return stream->buffer + stream->position;
}

int token_stream_push(token_stream_t* stream, token_type_t type, char* start, size_t length) {
    assert(stream != NULL);

    if (stream->buffer_length >= stream->buffer_capacity) {
        stream->buffer_capacity *= 2;
        stream->buffer = realloc(stream->buffer, stream->buffer_capacity);
        if (!stream->buffer)
            return 1;
    }

    stream->buffer[stream->buffer_length].type = type;
    stream->buffer[stream->buffer_length].start = start;
    stream->buffer[stream->buffer_length].length = length;
    ++stream->buffer_length;

    return 0;
}

int token_stream_clean(token_stream_t* stream) {
    assert(stream != NULL);

    if (stream->buffer)
        free(stream->buffer);
    stream->buffer = NULL;

    stream->buffer_length = 0;
    stream->buffer_capacity = 0;
    stream->position = 0;

    return 0;
}


expression_t* parse_prefix(token_stream_t* stream, token_t* tok) {
    if (tok->type != TOKEN_TYPE_OPERATOR) {
        printf("ERROR: expected operator, found %s\n", tok->start);
        return NULL;
    }

    expression_t* op = parse_expression_prec(stream, 50);
    if (!op) {
        return NULL;
    }
    return new_prefix_expression(*tok->start, op);
}

expression_t* parse_infix(token_stream_t* stream, expression_t* left, token_t* tok) {
    if (tok->type != TOKEN_TYPE_OPERATOR) {
        printf("ERROR: expected operator, found %.*s\n", tok->length, tok->start);
        return NULL;
    }

    expression_t* op = parse_expression_prec(stream, token_precedence(tok));
    if (!op) {
        return NULL;
    }

    return new_operator_expression(left, *tok->start, op);
}

expression_t* parse_expression_prec(token_stream_t* stream, int precedence) {
    token_t* tok = token_stream_next(stream);
    if (!tok) {
        return NULL;
    }

    expression_t* left;

    switch (tok->type) {
        case TOKEN_TYPE_OPERATOR:
            left = parse_prefix(stream, tok);
            break;
        case TOKEN_TYPE_NUMBER:
        {
            scalar_t num;

            char* del = alloca(tok->length + 1);
            del[tok->length] = 0;
            strncpy(del, tok->start, tok->length);
            if (SCALAR_FROM_STRING(del, num)) {
                printf("ERROR: invalid number: %s\n", del);
                return NULL;
            }

            left = new_number_expression(num);
            SCALAR_CLEAN(num);
            break;
        }
        case TOKEN_TYPE_IDENTIFIER:
            left = new_variable_expression(*tok->start);
            break;
        default:
            printf("ERROR: invalid token '%s', expeected operator number or variable", tok->start);
            return NULL;
    }

    for (tok = token_stream_peek(stream);
            tok && precedence < token_precedence(tok);
            tok = token_stream_peek(stream)) {
        token_stream_next(stream);
        left = parse_infix(stream, left, tok);
    }

    return left;
}

inline expression_t* parse_expression(token_stream_t* stream) {
    return parse_expression_prec(stream, 0);
}
