/* Copyright Ian Shehadeh 2018 */

#include "simplify/lexer.h"
#include "simplify/errors.h"

#define __LEXER_LOOP_END() __lexer_loop_continue = 0

#define __LEXER_LOOP(LEXER, BLOCK) {                           \
    int __lexer_loop_continue = 1;                             \
    while (1) {                                                \
        if ((LEXER)->buffer_position >= (LEXER)->buffer_length)\
            break;                                             \
        BLOCK;                                                 \
        if (__lexer_loop_continue) {                           \
            lexer_advance(LEXER);                              \
        } else {                                               \
            break;                                             \
        }                                                      \
    }                                                          \
}

#define __LEXER_SKIP_WHILE(LEXER, FUNC) __LEXER_LOOP(LEXER, if (!FUNC(lexer_current(LEXER))) __LEXER_LOOP_END())

/* initialize a lexer from a file
 *
 * This function reads the file block-by-block into an intermediate buffer,
 * instead of immediately checking it's length and reading the entire file.
 * Generally it's optimal to use lexer_init_from_file, instead of this function.
 * `lexer_init_from_file` will call this function if it can't determine the length anyway.
 * 
 * @lexer the lexer to initialize
 * @file the file read
 */
void lexer_init_from_file_buffered(lexer_t* lexer, FILE* file) {
    lexer->buffer = malloc(LEXER_FILE_BUFFER_SIZE);
    char buffer[LEXER_FILE_BUFFER_SIZE];

    size_t size = 0;
    while (fgets(buffer, LEXER_FILE_BUFFER_SIZE, file)) {
        size_t buf_size = strlen(buffer);
        lexer->buffer = realloc(lexer->buffer, size + buf_size + 1);
        strncpy(lexer->buffer + size, buffer, buf_size);
        size += buf_size;
    }

    lexer->buffer_length = size;
    lexer->buffer_position = 0;
}

void lexer_init_from_file(lexer_t* lexer, FILE* file) {
    fseek(file, 0, SEEK_END);
    size_t len = (size_t)ftell(file);
    if (len == (size_t)-1) {
        lexer_init_from_file_buffered(lexer, file);
        return;
    }

    lexer->buffer = malloc(len + 1);
    lexer->buffer_length = len;
    lexer->buffer_position = 0;

    lexer->buffer[len] = 0;
    printf("%s\n", lexer->buffer);
    fseek(file, 0, SEEK_SET);
    fread(lexer->buffer, len, 1, file);
}

static inline char lexer_current(lexer_t* lexer) {
    return lexer->buffer[lexer->buffer_position];
}

static inline void lexer_advance(lexer_t* lexer) {
    ++lexer->buffer_position;
}

static inline int lexer_eof(lexer_t* lexer) {
    return lexer->buffer_position >= lexer->buffer_length;
}

error_t lexer_get_number(lexer_t* lexer, token_t* token) {
    token->start = lexer->buffer + lexer->buffer_position;

    __LEXER_SKIP_WHILE(lexer, isdigit);

    token->length = lexer->buffer + lexer->buffer_position - token->start;
    if (!lexer_eof(lexer) && lexer_current(lexer) == '.') {
        lexer_advance(lexer);
        __LEXER_SKIP_WHILE(lexer, isdigit);
    }

    if (!lexer_eof(lexer) && (lexer_current(lexer) == 'e' || lexer_current(lexer) == 'E')) {
        lexer_advance(lexer);
        if (lexer_current(lexer) == '-')
            lexer_advance(lexer);
        __LEXER_SKIP_WHILE(lexer, isdigit);
    }

    token->type = TOKEN_TYPE_NUMBER;

    token->length = lexer->buffer + lexer->buffer_position - token->start;
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
        case '\\':
        case '*':
        case '^':
        case '=':
        case '>':
        case '<':
        case ':':
            token->type = TOKEN_TYPE_OPERATOR;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            ++lexer->buffer_position;
            break;
        case '(':
            token->type = TOKEN_TYPE_LEFT_PAREN;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            ++lexer->buffer_position;
            break;
        case ')':
            token->type = TOKEN_TYPE_RIGHT_PAREN;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            ++lexer->buffer_position;
            break;
        case ',':
            token->type = TOKEN_TYPE_COMMA;
            token->start = lexer->buffer + lexer->buffer_position;
            token->length = 1;
            ++lexer->buffer_position;
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e': 
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o': 
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': 
        case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': 
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O': 
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': 
        case 'Z':
        case '_':
            token->type  = TOKEN_TYPE_IDENTIFIER;
            token->start = lexer->buffer + lexer->buffer_position;
            __LEXER_SKIP_WHILE(lexer, isident)
            token->length = lexer->buffer + lexer->buffer_position - token->start;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '.':
            return lexer_get_number(lexer, token);
        default:
            return ERROR_INVALID_CHARACTER;
        }

    return ERROR_NO_ERROR;
}
