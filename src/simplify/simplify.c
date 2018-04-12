// Copyright Ian R. Shehadeh 2018

#include "simplify/scalar.h"
#include "simplify/expression.h"
#include "simplify/parser.h"
#include "simplify/lexer.h"

int main(int argc, char** argv) {
#if defined(SCALAR_FLOAT)
#   if defined(HAVE_MPFR)
        mpfr_set_default_prec(FLOAT_PRECISION);
#   elif defined(HAVE_GMP)
        mpf_set_default_prec(FLOAT_PRECISION);
#   endif
#endif

    lexer_t lexer;

    if (argc > 1) {
        if (lexer_init_from_string(&lexer, argv[1])) {
            printf("failed to initialize parser\n");
            return 1;
        }
    } else {
        lexer_init_from_file(&lexer, stdin);
    }

    expression_t* expr = parse_expression(&lexer);

    if (!expr) {
        lexer_clean(&lexer);
        return 1;
    }

    if (expression_simplify(expr) > 0) {
        return 1;
    }

    expression_print(expr);

    puts("");

    expression_free(expr);
    lexer_clean(&lexer);

#if defined(HAVE_MPFR) && !defined(SCALAR_INTEGER)
    mpfr_free_cache();
#endif

    return 0;
}
