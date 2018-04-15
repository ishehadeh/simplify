/* Copyright Ian Shehadeh 2018 */

#include "simplify/scalar/scalar.h"
#include "simplify/expression.h"
#include "simplify/parser.h"
#include "simplify/lexer.h"
#include "simplify/errors.h"

#define VERSION "0.0.1"
#define DATE __DATE__
#define COPYRIGHT "Copyright Ian Shehadeh 2018, all rights reserved."

#define INFO "simplify v" VERSION ". Compiled " DATE ".\n" COPYRIGHT \
"\n  simplify is a simple utility to evaluate a mathmatical expression."

void usage(char* arg0) {
    puts(INFO);
    printf("\nUSAGE: %s [OPTIONS] EXPRESSION\n", arg0);
    puts("OPTIONS:");
    puts("\t-h,--help ............... print this message");
    puts("\t-v,--verbose ............ print status updates while running, not just the expression's result");
    puts("\t-q,--quite .............. only print errors (this overides -v)");
    puts("\t-d,--define NAME=EXPR ... define a variable `NAME' as `EXPR'");
    puts("\t-i,--isolate NAME ....... if the variable `NAME' exists than attempt to isolate it");
}

error_t parse(char* source, expression_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_string(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = parse_expression(&parser, result);
    lexer_clean(&lexer);
    return err;
}

int main(int argc, char** argv) {
#if defined(SCALAR_FLOAT)
#   if defined(HAVE_MPFR)
        mpfr_set_default_prec(FLOAT_PRECISION);
#   elif defined(HAVE_GMP)
        mpf_set_default_prec(FLOAT_PRECISION);
#   endif
#endif

    int verbose = 0;
    variable_t isolation_target = NULL;
    error_t err;
    scope_t scope;

    scope_init(&scope);

    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i][0] == '-') {
            int x = 1;
            if (argv[i][1] == '-')
                ++x;
            switch (argv[i][x]) {
                case 'h':
                    usage(argv[0]);
                    goto cleanup;
                case 'v':
                    verbose = 1;
                    break;
                case 'q':
                    verbose = -1;
                    break;
                case 'd':
                {
                    if (i == argc - 2) {
                        usage(argv[0]);
                        goto cleanup;
                    }

                    expression_t result;
                    err = parse(argv[i + 1], &result);
                    if (err) goto error;

                    if (result.type != EXPRESSION_TYPE_OPERATOR) {
                        err = ERROR_INVALID_ASSIGNMENT_EXPRESSION;
                        goto error;
                    } else if (result.operator.infix != '=' ||
                        result.operator.left->type != EXPRESSION_TYPE_VARIABLE) {
                        err = ERROR_INVALID_ASSIGNMENT_EXPRESSION;
                        goto error;
                    }

                    scope_define(&scope, result.operator.left->variable.value, result.operator.right);
                    break;
                }
                case 'i':
                {
                    if (i == argc - 2) {
                        usage(argv[0]);
                        goto cleanup;
                    }

                    expression_t result;
                    err = parse(argv[i + 1], &result);
                    if (result.type != EXPRESSION_TYPE_VARIABLE) {
                        err = ERROR_INVALID_ASSIGNMENT_EXPRESSION;
                        goto error;
                    }
                    isolation_target = result.variable.value;
                    break;
                }
                default:
                    err = ERROR_UNRECOGNIZED_ARGUMENT;
                    usage(argv[0]);
                    goto cleanup;
            }
        }
    }

    if (argc < 2) goto cleanup;

    expression_t expr;
    err = parse(argv[argc - 1], &expr);
    if (err) goto error;

    err = expression_simplify(&expr, &scope);
    if (err) goto error;

    if (isolation_target) {
        err = expression_isolate_variable(&expr, isolation_target);
        if (err) goto error;

        err = expression_simplify(&expr, &scope);
        if (err) goto error;
    }
    if (verbose >= 0) {
        expression_print(&expr);
        puts("");
    }
    expression_clean(&expr);
    goto cleanup;


error:
    printf("simplify: %s\n", error_string(err));

cleanup:
    scope_clean(&scope);

#if defined(HAVE_MPFR) && !defined(SCALAR_INTEGER)
    mpfr_free_cache();
#endif

    return 0;
}
