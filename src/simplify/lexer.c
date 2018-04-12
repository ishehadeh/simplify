// Copyright Ian R. Shehadeh 2018

#include "lexer.h"


#define __LEXER_LOOP_END() __lexer_loop_continue = 0

#define __LEXER_LOOP(LEXER, BLOCK) {                                \
    int __lexer_loop_continue = 1;                                  \
    while (1) {                                                     \
        if ((LEXER)->buffer_position >= (LEXER)->buffer_length      \
                && lexer_try_extend(LEXER)) break;                  \
        BLOCK;                                                      \
        if(__lexer_loop_continue) lexer_advance(LEXER); else break; \
    }                                                               \
}

#define __LEXER_SKIP_WHILE(LEXER, FUNC) __LEXER_LOOP(LEXER, if (!FUNC(lexer_current(LEXER))) __LEXER_LOOP_END())

static inline char lexer_current(lexer_t* lexer) {
    return lexer->buffer[lexer->buffer_position];
}

static inline void lexer_advance(lexer_t* lexer) {
    ++lexer->buffer_position;
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

void lexer_get_number(lexer_t* lexer) {
    lexer->token.start = lexer->buffer + lexer->buffer_position;
    if (lexer_current(lexer) == '-')
        lexer_advance(lexer);

    __LEXER_SKIP_WHILE(lexer, isdigit);

#   if defined(SCALAR_INTEGER)
        lexer->token.length = lexer->buffer + lexer->buffer_position - lexer->token.start;
#   endif

    if(lexer_current(lexer) == '.') {
        lexer_advance(lexer);
        __LEXER_SKIP_WHILE(lexer, isdigit);
    }

    if(lexer_current(lexer) == 'e' || lexer_current(lexer) == 'E') {
        if (lexer_current(lexer) == '-')
            lexer_advance(lexer);
        __LEXER_SKIP_WHILE(lexer, isdigit);
    }

    lexer->token.type = TOKEN_TYPE_NUMBER;

#   if !defined(SCALAR_INTEGER)
        lexer->token.length = lexer->buffer + lexer->buffer_position - lexer->token.start;
#   endif

}

token_t* lexer_next(lexer_t* lexer) {
    __LEXER_SKIP_WHILE(lexer, isspace);

    switch (lexer->buffer[lexer->buffer_position]) {
        case '+':
        case '-':
        case '/':
        case '*':
        case '^':
        case '=':
        case '>':
        case '<':
            lexer->token.type = TOKEN_TYPE_OPERATOR;
        case '(':
            lexer->token.type = TOKEN_TYPE_OPERATOR;
        case ')':
            lexer->token.type = TOKEN_TYPE_OPERATOR;
            lexer->token.start = lexer->buffer + lexer->buffer_position;
            lexer->token.length = 1;
            break;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            lexer->token.type  = TOKEN_TYPE_IDENTIFIER;
            lexer->token.start = lexer->buffer + lexer->buffer_position;
            __LEXER_SKIP_WHILE(lexer, isident)
            lexer->token.length = lexer->buffer + lexer->buffer_position - lexer->token.start;
            break;
        case '0' ... '9':
        case '.':
            lexer_get_number(lexer);
            break;
        default:
            printf("ERROR: invalid character '%c'", lexer_current(lexer));
            return NULL;
    }

    return &lexer->token;
}
