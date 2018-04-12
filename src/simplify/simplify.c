// Copyright Ian R. Shehadeh 2018

#include "simplify/scalar.h"
#include "simplify/expression.h"
#include "simplify/parser.h"
#include "simplify/lexer.h"
#include "simplify/errors.h"

int main(int argc, char** argv) {
#if defined(SCALAR_FLOAT)
#   if defined(HAVE_MPFR)
        mpfr_set_default_prec(FLOAT_PRECISION);
#   elif defined(HAVE_GMP)
        mpf_set_default_prec(FLOAT_PRECISION);
#   endif
#endif

    lexer_t lexer;
    expression_t expression;
    expression_parser_t parser;

    if (argc > 1) {
        if (lexer_init_from_string(&lexer, argv[1])) {
            printf("failed to initialize parser\n");
            return 1;
        }
    } else {
        lexer_init_from_file(&lexer, stdin);
    }

    expression_parser_init(&parser, &lexer);
    error_t err = parse_expression(&parser, &expression);

    if (err) {
        printf("simplify [%d]: %s", lexer.buffer_position, error_string(err));
    } else {
        expression_simplify(&expression);
        expression_print(&expression);
    }

    expression_clean(&expression);
    lexer_clean(&lexer);

#if defined(HAVE_MPFR) && !defined(SCALAR_INTEGER)
    mpfr_free_cache();
#endif
    puts("");

    return 0;
}
