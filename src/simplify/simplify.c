// Copyright Ian R. Shehadeh 2018

#include "simplify/scalar/scalar.h"
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
    scope_t scope;

    scope_init(&scope);
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

    if (err) goto error;

    err = expression_simplify(&expression, &scope);
    if (err) goto error;

    if (argc > 2) {
        err = expression_isolate_variable(&expression, argv[2]);
        if (err) goto error;

        err = expression_simplify(&expression, &scope);
        if (err) goto error;
    }
    expression_print(&expression);
    puts("");
    goto cleanup;


error:
    printf("simplify: %s\n", error_string(err));

cleanup:
    expression_clean(&expression);
    lexer_clean(&lexer);
    scope_clean(&scope);

#if defined(HAVE_MPFR) && !defined(SCALAR_INTEGER)
    mpfr_free_cache();
#endif

    return 0;
}
