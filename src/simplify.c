/* Copyright Ian Shehadeh 2018 */

#include "simplify/parser.h"
#include "simplify/lexer.h"
#include "simplify/errors.h"
#include "flags/flags.h"

#include "simplify/expression/evaluate.h"
#include "simplify/expression/simplify.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/stringify.h"

#define VERSION "0.1.2"
#define DATE __DATE__
#define COPYRIGHT "Copyright Ian Shehadeh 2018, all rights reserved."

#define INFO "simplify v" VERSION ". Compiled " DATE ".\n" COPYRIGHT \
"\n  simplify is a simple utility to evaluate a mathmatical expression."

#define TRUE_STRING  "true"
#define FALSE_STRING "false"


void usage(char* arg0) {
    puts(INFO);
    printf("\nUSAGE: %s [OPTIONS] [...EXPRESSION]\n", arg0);
    puts("OPTIONS:");
    puts("\t-h,--help ..................... print this message");
    puts("\t-v,--verbose .................. print status updates while running, not just the expression's result");
    puts("\t-q,--quiet .................... only print errors (this overides -v)");
    puts("\t-d,--define NAME=EXPR ......... define a variable `NAME' as `EXPR'");
    puts("\t-i,--isolate NAME ............. if the variable `NAME' exists than attempt to isolate it");
    puts("\t-f,--file FILE ................ execute the file `FILE' before any expression(s)");
}

error_t do_assignment(char* assignment, scope_t* scope) {
    error_t err;
    expression_t result;
    err = parse_string(assignment, &result);
    if (err) return err;

    if (result.type != EXPRESSION_TYPE_OPERATOR) {
        return ERROR_INVALID_ASSIGNMENT_EXPRESSION;
    } else if (result.operator.infix != '=' ||
        result.operator.left->type != EXPRESSION_TYPE_VARIABLE) {
        return ERROR_INVALID_ASSIGNMENT_EXPRESSION;
    }

    scope_define(scope, result.operator.left->variable.value, result.operator.right);
    return ERROR_NO_ERROR;
}


error_t execute_file(char* fname, scope_t* scope) {
    int free_fname = 0;

    if (fname[0] == '~') {
        char* bad_fname = fname;
        char* home = getenv("HOME");
        int len = strlen(fname) - 1;
        int home_len = strlen(home);
        fname = malloc(len + home_len + 1);
        fname[len + home_len] = 0;

        memmove(fname, home, home_len);
        memmove(fname + home_len, bad_fname + 1, len);
        free_fname = 1;
    }

    FILE* f;
    if (fname[0] == '-' && fname[1] == 0) {
        f = stdin;
    } else {
        f = fopen(fname, "r");
    }
    if (!f)
        return ERROR_UNABLE_TO_OPEN_FILE;

    expression_list_t* exprs = malloc(sizeof(expression_list_t));
    expression_list_init(exprs);
    error_t err = parse_file(f, exprs);
    if (err) return err;

    expression_t* expr;
    EXPRESSION_LIST_FOREACH(expr, exprs) {
        err = expression_evaluate(expr, scope);
        if (err) {
            expression_list_free(exprs);
            return err;
        }
    }

    expression_list_free(exprs);
    if (f != stdin)
        fclose(f);
    if (free_fname)
        free(fname);
    return ERROR_NO_ERROR;
}

int main(int argc, char** argv) {
    int verbosity = 0;
    variable_t isolation_target = NULL;
    scope_t scope;

    scope_init(&scope);

    error_t err = ERROR_NO_ERROR;
    PARSE_FLAGS(
        FLAG('h', "help",    usage(argv[0]); goto cleanup)
        FLAG('v', "verbose", verbosity = 1)
        FLAG('q', "quiet",   verbosity = -1)
        FLAG('d', "define",  err = do_assignment(FLAG_VALUE, &scope); if (err) goto error)
        FLAG('i', "isolate", isolation_target = FLAG_VALUE)
        FLAG('f', "file",    err = execute_file(FLAG_VALUE, &scope); if (err) goto error)
    )

    if (err) goto error;
    if (!flag_argc) goto cleanup;

    for (int i = 0; i < flag_argc; ++i) {
        expression_t expr;
        err = parse_string(flag_argv[i], &expr);
        if (err) goto error;

        err = expression_evaluate(&expr, &scope);
        if (err) goto error;

        err = expression_simplify(&expr);
        if (err) goto error;

        if (isolation_target) {
            err = expression_isolate_variable(&expr, isolation_target);
            if (!err) {
                err = expression_evaluate(&expr, &scope);
                if (err) goto error;
            }
        }
        if (verbosity >= 0) {
            if (scope.boolean != -1) {
                if (scope.boolean) {
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
        scope.boolean = -1;
    }

    goto cleanup;

error:
    printf("simplify: %s\n", error_string(err));

cleanup:
    scope_clean(&scope);
    mpfr_free_cache();

    return 0;
}
