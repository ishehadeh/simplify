/* Copyright Ian Shehadeh 2018 */

#include "test/test.h"
#include "simplify/lexer.h"

struct {
    char*    string;
    token_t tokens[7];
} __string_token_pairs[] = {
    {"",
        {{TOKEN_TYPE_EOF, "", 0}}
    },
    {"5",
        {{TOKEN_TYPE_NUMBER, "5", 1},
         {TOKEN_TYPE_EOF,    "",  0}},
    },
    {"   094351267 ",
        {{TOKEN_TYPE_NUMBER, "094351267", 9},
         {TOKEN_TYPE_EOF,    "",          0}},
    },
    {"1E",
        {{TOKEN_TYPE_NUMBER, "1E", 2},
         {TOKEN_TYPE_EOF,    "",   0}},
    },
    {"1e90",
        {{TOKEN_TYPE_NUMBER, "1e90", 4},
         {TOKEN_TYPE_EOF,    "",     0}},
    },
    {"1.1e02",
        {{TOKEN_TYPE_NUMBER, "1.1e02", 6},
         {TOKEN_TYPE_EOF,    "",       0}},
    },
    {"1.4e-2",
        {{TOKEN_TYPE_NUMBER, "1.4e-2", 6},
         {TOKEN_TYPE_EOF,    "",       0}},
    },
    {"_",
        {{TOKEN_TYPE_IDENTIFIER, "_", 1},
         {TOKEN_TYPE_EOF,        "",  0}},
    },
    {"ALL CAPS",
        {{TOKEN_TYPE_IDENTIFIER, "ALL",  3},
        {TOKEN_TYPE_IDENTIFIER, "CAPS",  4},
        {TOKEN_TYPE_EOF,        "",     0}},
    },
    {"12345 + 32134 ) \\",
        {{TOKEN_TYPE_NUMBER,    "12345",  5},
        {TOKEN_TYPE_OPERATOR,    "+",     1},
        {TOKEN_TYPE_NUMBER,      "32134", 5},
        {TOKEN_TYPE_RIGHT_PAREN, ")",     1},
        {TOKEN_TYPE_OPERATOR,    "\\",    1},
        {TOKEN_TYPE_EOF,         "",      0}}
    },
    {"var / * () other_var",
        {{TOKEN_TYPE_IDENTIFIER, "var",       3},
        {TOKEN_TYPE_OPERATOR,    "/",         1},
        {TOKEN_TYPE_OPERATOR,    "*",         1},
        {TOKEN_TYPE_LEFT_PAREN,  "(",         1},
        {TOKEN_TYPE_RIGHT_PAREN, ")",         1},
        {TOKEN_TYPE_IDENTIFIER,  "other_var", 9},
        {TOKEN_TYPE_EOF,         "",          0}}
    },
};

struct {
    char*    string;
    error_t  error;
} __token_error_pairs[] = {
    {"$",       ERROR_INVALID_CHARACTER},
    {"easda$_", ERROR_INVALID_CHARACTER},
    {"123e&",   ERROR_INVALID_CHARACTER},
};


int main() {
    token_t tok;
    token_t ftok;
    error_t err;
    tok.type = 0;

    for (size_t i = 0; i < sizeof(__string_token_pairs) / sizeof(__string_token_pairs[0]); ++i) {
        lexer_t lexer;
        lexer_t flexer;

        FILE* f = fopen("lexer_test.txt", "w+");
        if (!f) FATAL("%s", "Failed to open file lexer_test.txt");

        lexer_init_from_string(&lexer, __string_token_pairs[i].string);
        fwrite(__string_token_pairs[i].string, strlen(__string_token_pairs[i].string), 1, f);
        fflush(f);
        fseek(f, 0, SEEK_SET);

        // switch file read methods every other string
        if (i % 2 == 0) {
            lexer_init_from_file(&flexer, f);
        } else {
            lexer_init_from_file_buffered(&flexer, f);
        }
        fclose(f);

        for (int j = 0; ; ++j) {
            err = lexer_next(&lexer, &tok);
            if (err) {
                FATAL("failed to get next token (string lexer): %s", error_string(err));
            }

            err = lexer_next(&flexer, &ftok);
            if (err) {
                FATAL("failed to get next token (file lexer): %s", error_string(err));
            }
            assert_token_eq(&tok, &ftok);
            assert_token_eq(&tok,  &__string_token_pairs[i].tokens[j]);
            assert_token_eq(&ftok, &__string_token_pairs[i].tokens[j]);
            if (tok.type == TOKEN_TYPE_EOF)
                break;
        }
        lexer_clean(&lexer);
    }

    for (size_t i = 0; i < sizeof(__token_error_pairs) / sizeof(__token_error_pairs[0]); ++i) {
        INFO("analyzing: '%s'", __token_error_pairs[i].string);

        lexer_t lexer;
        lexer_init_from_string(&lexer, __token_error_pairs[i].string);

        for (int j = 0; ; ++j) {
            err = lexer_next(&lexer, &tok);
            if (err == __token_error_pairs[i].error)
                break;
            if (tok.type == TOKEN_TYPE_EOF)
                FATAL("%s", "error never encountered");
        }
        lexer_clean(&lexer);
    }
}
