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
    {"   094351267 ",
        {{TOKEN_TYPE_NUMBER, "094351267", 9},
         {TOKEN_TYPE_EOF,    "",          0}},
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


void assert_token_eq(token_t* tok1, token_t* tok2) {
    if (tok1->length != tok2->length || strncmp(tok1->start, tok2->start, tok1->length) != 0) {
        FATAL("tokens strings don't match! ('%.*s' != '%.*s')",
            (int)tok1->length, tok1->start, (int)tok2->length, tok2->start);
    }

    if (tok1->type != tok2->type) {
        FATAL("tokens are not of the same type! (%d != %d)", tok1->type, tok2->type);
    }
}

int main() {
    token_t tok;
    error_t err;
    tok.type = 0;

    for (size_t i = 0; i < sizeof(__string_token_pairs) / sizeof(__string_token_pairs[0]); ++i) {
        INFO("analyzing: '%s'", __string_token_pairs[i].string);

        lexer_t lexer;
        lexer_init_from_string(&lexer, __string_token_pairs[i].string);

        for (int j = 0; ; ++j) {
            err = lexer_next(&lexer, &tok);
            if (err) {
                FATAL("failed to get next token: %s", error_string(err));
            }
            assert_token_eq(&tok, &__string_token_pairs[i].tokens[j]);
            if (tok.type == TOKEN_TYPE_EOF)
                break;
        }
        lexer_clean(&lexer);
    }
}
