#include "lexer.h"

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

void lexer_get_ident(lexer_t* lexer) {
    char* start = lexer->buffer + lexer->buffer_position;
    while (lexer->buffer_position < lexer->buffer_length) {
        switch (lexer->buffer[lexer->buffer_position]) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
            case '_':
                break;
            default:
                token_stream_push(&lexer->tokens, TOKEN_TYPE_IDENTIFIER, start, (lexer->buffer + lexer->buffer_position) - start);
                return;
        }
        ++lexer->buffer_position;
    }
    token_stream_push(&lexer->tokens, TOKEN_TYPE_IDENTIFIER, start, (lexer->buffer + lexer->buffer_position) - start);
    return;
}

void lexer_get_number(lexer_t* lexer) {
    char* start = lexer->buffer + lexer->buffer_position;
    size_t len = 0;
    while (1) {
        if (lexer->buffer_position >= lexer->buffer_length) {
            if (lexer_try_extend(lexer)) {
                break;
            }
        }
        switch (lexer->buffer[lexer->buffer_position]) {
            case '0' ... '9':
                ++len;
                break;
            case '.':
                ++lexer->buffer_position, ++len;
                goto lexer_get_number_after_decimal;
            case 'E':
            case 'e':
                ++lexer->buffer_position, ++len;
                goto lexer_get_number_after_exponent;
            default:
                goto lexer_get_number_exit;
        }
        ++lexer->buffer_position;
    }

lexer_get_number_after_decimal:
    while (1) {
        if (lexer->buffer_position >= lexer->buffer_length) {
            if (lexer_try_extend(lexer)) {
                break;
            }
        }
        switch (lexer->buffer[lexer->buffer_position]) {
            case '0' ... '9':
#               if !defined(SCALAR_INTEGER)
                    ++len;
#               endif
                break;
            case 'E':
            case 'e':
                ++lexer->buffer_position, ++len;
                goto lexer_get_number_after_exponent;
            default:
                goto lexer_get_number_exit;
        }
        ++lexer->buffer_position;
    }

lexer_get_number_after_exponent:
    while (1) {
        if (lexer->buffer_position >= lexer->buffer_length) {
            if (lexer_try_extend(lexer)) {
                break;
            }
        }
        if (lexer->buffer[lexer->buffer_position] == '-') {
#               if !defined(SCALAR_INTEGER)
                ++len;
#               endif
                ++lexer->buffer_position;
        }
        switch (lexer->buffer[lexer->buffer_position]) {
            case '0' ... '9':
#               if !defined(SCALAR_INTEGER)
                ++len;
#               endif
                break;
            default:
                goto lexer_get_number_exit;
        }
        ++lexer->buffer_position;
    }


lexer_get_number_exit:
    --lexer->buffer_position;
    token_stream_push(&lexer->tokens, TOKEN_TYPE_NUMBER, start, len);
    return;
}

token_stream_t* lexer_tokenize(lexer_t* lexer) {
    while (1) {
        if (lexer->buffer_position >= lexer->buffer_length) {
            if (lexer_try_extend(lexer)) {
                break;
            }
        }
        switch (lexer->buffer[lexer->buffer_position]) {
            case '\n':
            case '\v':
            case '\t':
            case ' ' :
                break;
            case '+':
            case '-':
            case '/':
            case '*':
            case '^':
            case '=':
                token_stream_push(&lexer->tokens, TOKEN_TYPE_OPERATOR, lexer->buffer + lexer->buffer_position, 1);
                break;
            case '(':
                token_stream_push(&lexer->tokens, TOKEN_TYPE_LEFT_PAREN, lexer->buffer + lexer->buffer_position, 1);
                break;
            case ')':
                token_stream_push(&lexer->tokens, TOKEN_TYPE_RIGHT_PAREN, lexer->buffer + lexer->buffer_position, 1);
                break;
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_':
                lexer_get_ident(lexer);
                break;
            case '0' ... '9':
            case '.':
                lexer_get_number(lexer);
                break;
            default:
                printf("ERROR: invalid character '%c'", lexer->buffer[lexer->buffer_position]);
                return NULL;
        }
        ++lexer->buffer_position;
    }

    return &lexer->tokens;
}