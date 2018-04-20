/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"
#include "simplify/parser.h"
#include "simplify/lexer.h"
#include "simplify/errors.h"

#define VERSION "0.1.2"
#define DATE __DATE__
#define COPYRIGHT "Copyright Ian Shehadeh 2018, all rights reserved."

#define INFO "simplify v" VERSION ". Compiled " DATE ".\n" COPYRIGHT \
"\n  simplify is a simple utility to evaluate a mathmatical expression."

#define TRUE_STRING  "true"
#define FALSE_STRING "false"


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

struct args {
    int     verbosity;
    int     help;
    scope_t scope;
    variable_t isolation_target;
};

error_t parse_args(int argc, char** argv, struct args* args) {
    scope_init(&args->scope);
    args->isolation_target = NULL;
    args->help = 0;
    args->verbosity = 0;
    error_t err;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            int x = 1;
            if (argv[i][1] == '-')
                ++x;
            switch (argv[i][x]) {
                case 'h':
                    args->help = 1;
                    break;
                case 'v':
                    args->verbosity = 1;
                    break;
                case 'q':
                    args->verbosity = 1;
                    break;
                case 'd':
                {
                    if (i == argc - 2) {
                        args->help = 1;
                        return ERROR_NO_ERROR;
                    }

                    expression_t result;
                    err = parse_string(argv[i + 1], &result);
                    if (err) return err;

                    if (result.type != EXPRESSION_TYPE_OPERATOR) {
                        return ERROR_INVALID_ASSIGNMENT_EXPRESSION;
                    } else if (result.operator.infix != '=' ||
                        result.operator.left->type != EXPRESSION_TYPE_VARIABLE) {
                        return ERROR_INVALID_ASSIGNMENT_EXPRESSION;
                    }

                    scope_define(&args->scope, result.operator.left->variable.value, result.operator.right);
                    break;
                }
                case 'i':
                {
                    if (i == argc - 2) {
                        args->help = 1;
                        return ERROR_NO_ERROR;
                    }

                    args->isolation_target = argv[i + 1];
                    break;
                }
                case '0'...'9':
                    return ERROR_NO_ERROR;
                default:
                    return ERROR_UNRECOGNIZED_ARGUMENT;
            }
            if (i == argc - 1) {
                return ERROR_NO_ERROR;
            }
        }
    }
    return ERROR_NO_ERROR;
}

int main(int argc, char** argv) {
    mpfr_set_default_prec(64);

    struct args args;
    error_t err = parse_args(argc, argv, &args);
    if (err) goto error;

    if (args.help || argc < 2) {
        usage(argv[0]);
        goto cleanup;
    }

    expression_t expr;
    err = parse_string(argv[argc - 1], &expr);
    if (err) goto error;

    err = expression_simplify(&expr, &args.scope);
    if (err) goto error;

    if (args.isolation_target) {
        err = expression_isolate_variable(&expr, args.isolation_target);
        if (!err) {
            err = expression_simplify(&expr, &args.scope);
            if (err) goto error;
        }
    }
    if (args.verbosity >= 0) {
        if (args.scope.boolean != -1) {
            if (args.scope.boolean) {
                puts(TRUE_STRING);
            } else {
                puts(FALSE_STRING);
            }
        } else {
            expression_print(&expr);
            puts("");
        }
    }
    expression_clean(&expr);
    goto cleanup;


error:
    printf("simplify: %s\n", error_string(err));

cleanup:
    scope_clean(&args.scope);
    mpfr_free_cache();

    return 0;
}
