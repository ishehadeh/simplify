// Copyright Ian R. Shehadeh 2018

#include "simplify/lexer.h"
#include "simplify/errors.h"

#define __LEXER_LOOP_END() __lexer_loop_continue = 0

#define __LEXER_LOOP(LEXER, BLOCK) {                           \
    int __lexer_loop_continue = 1;                             \
    while (1) {                                                \
        if ((LEXER)->buffer_position >= (LEXER)->buffer_length \
                && lexer_try_extend(LEXER)) break;             \
        BLOCK;                                                 \
        if (__lexer_loop_continue) {                           \
            lexer_advance(LEXER);                              \
        } else {                                               \
            break;                                             \
        }                                                      \
    }                                                          \
}

#define __LEXER_SKIP_WHILE(LEXER, FUNC) __LEXER_LOOP(LEXER, if (!FUNC(lexer_current(LEXER))) __LEXER_LOOP_END())

static inline char lexer_current(lexer_t* lexer) {
    return lexer->buffer[lexer->buffer_position];
}

static inline void lexer_advance(lexer_t* lexer) {
    ++lexer->buffer_position;
}

static inline int lexer_eof(lexer_t* lexer) {
    return lexer->buffer_position >= lexer->buffer_length && lexer->source == NULL || lexer_current(lexer) == 0;
}

int lexer_try_extend(lexer_t* lexer) {
    if (lexer->source != NULL) {
        lexer->buffer_length = fread(lexer->buffer, lexer->buffer_capacity, 1, lexer->source);
        lexer->buffer_position = 0;
        if (lexer->buffer_length < lexer->buffer_capacity) {
            lexer->source = NULL;
        }
        return lexer->buffer_length == 0;
    } else {
        return 1;
    }
}

error_t lexer_get_number(lexer_t* lexer, token_t* token) {
    token->start = lexer->buffer + lexer->buffer_position;
    if (lexer_current(lexer) == '-')
        lexer_advance(lexer);

    __LEXER_SKIP_WHILE(lexer, isdigit);

#   if defined(SCALAR_INTEGER)
        token->length = lexer->buffer + lexer->buffer_position - token->start;
#   endif

    if (!lexer_eof(lexer) && lexer_current(lexer) == '.') {
        lexer_advance(lexer);
        __LEXER_SKIP_WHILE(lexer, isdigit);
    }

    if (!lexer_eof(lexer) && (lexer_current(lexer) == 'e' || lexer_current(lexer) == 'E')) {
        if (lexer_current(lexer) == '-')
            lexer_advance(lexer);
        __LEXER_SKIP_WHILE(lexer, isdigit);
    }

    token->type = TOKEN_TYPE_NUMBER;

#   if !defined(SCALAR_INTEGER)
        token->length = lexer->buffer + lexer->buffer_position - token->start;
#   endif
    return ERROR_NO_ERROR;
}

error_t lexer_next(lexer_t* lexer, token_t* token) {
    __LEXER_SKIP_WHILE(lexer, isspace);

    if (lexer_eof(lexer)) {
        token->type = TOKEN_TYPE_EOF;
        token->length = 0;
        token->start = lexer->buffer + lexer->buffer_position;
        return ERROR_NO_ERROR;
    }

    switch (lexer_current(lexer)) {
        case '+':
        case '-':
        case '/':
        case '*':
        case '^':
        case '=':
        case '>':
        case '<':
            token->type = TOKEN_TYPE_OPERATOR;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            break;
        case '(':
            token->type = TOKEN_TYPE_LEFT_PAREN;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            break;
        case ')':
            token->type = TOKEN_TYPE_RIGHT_PAREN;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            break;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            token->type  = TOKEN_TYPE_IDENTIFIER;
            token->start = lexer->buffer + lexer->buffer_position;
            __LEXER_SKIP_WHILE(lexer, isident)
            token->length = lexer->buffer + lexer->buffer_position - token->start;
            break;
        case '0' ... '9':
        case '.':
            return lexer_get_number(lexer, token);
        default:
            return ERROR_INVALID_CHARACTER;
        }

    ++lexer->buffer_position;

    return ERROR_NO_ERROR;
}
