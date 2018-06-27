/* Copyright Ian Shehadeh 2018 */

#include <stdio.h>
#include <time.h>

#include "flags/flags.h"
#include "simplify/simplify.h"

#define VERSION "0.1.2"
#define DATE __DATE__
#define COPYRIGHT "Copyright Ian Shehadeh 2018, all rights reserved."

#define INFO                                                \
    "simplify v" VERSION ". Compiled " DATE ".\n" COPYRIGHT \
    "\n  simplify is a simple utility to evaluate a mathmatical expression."

#define TRUE_STRING "true"
#define FALSE_STRING "false"

void usage(char* arg0) {
    puts(INFO);
    printf("\nUSAGE: %s [OPTIONS] [...EXPRESSION]\n", arg0);
    puts("OPTIONS:");
    puts("\t-h, --help ..................... print this message");
    puts("\t-v, --verbose .................. print status updates while running, not just the expression's result");
    puts("\t-q, --quiet .................... only print errors (this overides -v)");
    puts("\t-d, --define NAME=EXPR ......... define a variable `NAME' as `EXPR'");
    puts("\t-i, --isolate NAME ............. if the variable `NAME' exists than attempt to isolate it");
    puts("\t-f, --file FILE ................ execute the file `FILE' before any expression(s)");
}

error_t do_assignment(char* assignment, scope_t* scope) {
    error_t err;
    expression_t result;
    err = parse_string(assignment, &result);
    if (err) return err;

    if (result.type != EXPRESSION_TYPE_OPERATOR) {
        return ERROR_INVALID_ASSIGNMENT_EXPRESSION;
    } else if (result.operator.infix != '=' || result.operator.left->type != EXPRESSION_TYPE_VARIABLE) {
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

    FILE* f = fopen(fname, "r");
    if (!f) return ERROR_UNABLE_TO_OPEN_FILE;

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
    if (f != stdin) fclose(f);
    if (free_fname) free(fname);
    return ERROR_NO_ERROR;
}

error_t simplify_and_print(scope_t* scope, expression_t* expr, char* isolate_target, int print) {
    error_t err;

    err = expression_evaluate(expr, scope);
    if (err) return err;

    err = expression_simplify(expr);
    if (err) return err;

    if (isolate_target) {
        err = expression_isolate_variable(expr, isolate_target);
        if (!err) {
            err = expression_evaluate(expr, scope);
            if (err) return err;
        }
    }
    if (print) {
        expression_result_t result = expression_evaluate_comparisons(expr);
        if (result == EXPRESSION_RESULT_TRUE) {
            puts(TRUE_STRING);
        } else if (result == EXPRESSION_RESULT_FALSE) {
            puts(FALSE_STRING);
        } else {
            expression_print(expr);
            puts("");
        }
    }

    return ERROR_NO_ERROR;
}

int main(int argc, char** argv) {
    int verbosity = 0;
    variable_t isolation_target = NULL;
    scope_t scope;

    scope_init(&scope);
    simplify_export_builtins(&scope);
    error_t err = ERROR_NO_ERROR;

    /* clang-format off */
    PARSE_FLAGS(
        FLAG('h', "help", usage(argv[0]); goto cleanup)
        FLAG('v', "verbose", verbosity = 1)
        FLAG('q', "quiet", verbosity = -1)
        FLAG('d', "define", err = do_assignment(FLAG_VALUE, &scope); if (err) goto error)
        FLAG('i', "isolate", isolation_target = FLAG_VALUE)
        FLAG('f', "file", err = execute_file(FLAG_VALUE, &scope); if (err) goto error)
    )
    /* clang-format on */

    if (err) goto error;

    if (flag_argc == 0 || (flag_argc == 1 && !strcmp(flag_argv[0], "-"))) {
        expression_t* expr;
        expression_list_t* expr_list = malloc(sizeof(expression_list_t));
        expression_list_init(expr_list);

        err = parse_file(stdin, expr_list);
        if (err) goto error;

        EXPRESSION_LIST_FOREACH(expr, expr_list) {
            err = simplify_and_print(&scope, expr, isolation_target, verbosity >= 0);
            if (err) goto error;
        }

        expression_list_free(expr_list);
        if (err) goto error;
        goto cleanup;
    }

    for (int i = 0; i < flag_argc; ++i) {
        expression_t expr;
        err = parse_string(flag_argv[i], &expr);
        if (err) goto error;

        err = simplify_and_print(&scope, &expr, isolation_target, verbosity >= 0);
        if (err) goto error;

        expression_clean(&expr);
    }

    goto cleanup;

error:
    printf("simplify: %s\n", error_string(err));

cleanup:
    scope_clean(&scope);
    mpfr_free_cache();
    return 0;
}
