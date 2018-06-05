/* Copyright Ian Shehadeh 2018 */

#include <time.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#   include <io.h>
#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(__MACH__)
#   include <unistd.h>
#else
#   error "Your system doesn't appear to be Windows or unix-like!"
#endif

#ifndef STDIN_FILENO
#   define STDIN_FILENO 0
#endif

#include "simplify/parser.h"
#include "simplify/lexer.h"
#include "simplify/errors.h"
#include "simplify/builtins.h"
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

static gmp_randstate_t _g_rand_state;
static bool            _g_rand_state_initialized;
static mpc_ptr        _g_eulers_constant;

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

    FILE* f = fopen(fname, "r");
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

// DEFINE_mpc_FUNCTION(cos)
// DEFINE_mpc_FUNCTION(sin)
// DEFINE_mpc_FUNCTION(tan)
// DEFINE_mpc_FUNCTION(acos)
// DEFINE_mpc_FUNCTION(asin)
// DEFINE_mpc_FUNCTION(atan)
// DEFINE_mpc_FUNCTION(sec)
// DEFINE_mpc_FUNCTION(csc)
// DEFINE_mpc_FUNCTION(cot)
// DEFINE_mpc_FUNCTION(cosh)
// DEFINE_mpc_FUNCTION(sinh)
// DEFINE_mpc_FUNCTION(tanh)
// DEFINE_mpc_FUNCTION(acosh)
// DEFINE_mpc_FUNCTION(asinh)
// DEFINE_mpc_FUNCTION(atanh)
// DEFINE_mpc_FUNCTION(sech)
// DEFINE_mpc_FUNCTION(csch)
// DEFINE_mpc_FUNCTION(coth)

// DEFINE_mpc_FUNCTION_NRND(ceil)
// DEFINE_mpc_FUNCTION_NRND(floor)
// DEFINE_mpc_FUNCTION_NRND(round)
// DEFINE_mpc_FUNCTION_NRND(roundeven)
// DEFINE_mpc_FUNCTION_NRND(trunc)
// DEFINE_mpc_FUNCTION(frac)

// DEFINE_mpc_FUNCTION2(min)
// DEFINE_mpc_FUNCTION2(max)

DEFINE_mpc_CONST(pi)
DEFINE_mpc_CONST(euler)
DEFINE_mpc_CONST(catalan)

error_t builtin_func_random(scope_t* scope, expression_t** out) {
    (void)scope;

    if (!_g_rand_state_initialized) {
        _g_rand_state_initialized = true;
        gmp_randinit_default(_g_rand_state);
        gmp_randseed_ui(_g_rand_state, (unsigned long)time(NULL));
    }

    mpc_ptr num = malloc(sizeof(mpc_t));
    mpc_init2(num, 256);

    mpc_urandom(num, _g_rand_state);
    *out = expression_new_number(num);

    return ERROR_NO_ERROR;
}


error_t builtin_func_ln(scope_t* scope, expression_t** out) {
    expression_t input;
    scope_get_value(scope, "__arg0", &input);
    if (!EXPRESSION_IS_NUMBER(&input)) {
        return ERROR_NO_ERROR;
        expression_clean(&input);
    }
    mpc_ptr num = malloc(sizeof(mpc_t));
    mpc_init2(num, 256);
    mpc_log(num, input.number.value, MPC_RNDNN);
    *out = expression_new_number(num);
    mpc_clear(num);
    expression_clean(&input);
    return ERROR_NO_ERROR;
}

error_t builtin_func_log(scope_t* scope, expression_t** out) {
    error_t err;
    expression_t* b = malloc(sizeof(expression_t));
    expression_t* y = malloc(sizeof(expression_t));

    scope_get_value(scope, "__arg0", b);
    scope_get_value(scope, "__arg1", y);

    err = expression_do_logarithm(b, y, out);
    if (err) return err;

    return expression_evaluate(*out, scope);
}


error_t builtin_const_e(scope_t* _, expression_t** out) {
    (void)_;

    if (!_g_eulers_constant) {
        // Approximate euler's constant and then cache it
        _g_eulers_constant = malloc(sizeof(mpc_t));
        mpc_t n;
        mpc_t x;
        mpc_init2(n, 256);
        mpc_init2(x, 256);
        mpc_init2(_g_eulers_constant, 256);

        mpc_set_ui(n, 9999999999UL, MPC_RNDNN);
        mpc_ui_div(x, 1, n, MPC_RNDNN);
        mpc_add_ui(x, x, 1, MPC_RNDNN);
        mpc_pow(_g_eulers_constant, x, n, MPC_RNDNN);

        mpc_clear(n);
        mpc_clear(x);
    }

    mpc_ptr copy = malloc(sizeof(mpc_t));
    mpc_init2(copy, 256);
    mpc_set(copy, _g_eulers_constant, MPC_RNDNN);
    *out = expression_new_number(copy);

    return ERROR_NO_ERROR;
}

error_t builtin_const_nan(scope_t* _, expression_t** out) {
    (void)_;
    *out = expression_new_number_si(0);
    mpc_set_nan(out[0]->number.value);
    return ERROR_NO_ERROR;
}

error_t builtin_const_inf(scope_t* _, expression_t** out) {
    (void)_;
    *out = expression_new_number_si(0);
    mpfr_set_inf(mpc_realref(out[0]->number.value), 1);
    return ERROR_NO_ERROR;
}

int main(int argc, char** argv) {
    int verbosity = 0;
    variable_t isolation_target = NULL;
    scope_t scope;

    scope_init(&scope);

    // EXPORT_BUILTIN_FUNCTION(&scope, cos);
    // EXPORT_BUILTIN_FUNCTION(&scope, sin);
    // EXPORT_BUILTIN_FUNCTION(&scope, tan);
    // EXPORT_BUILTIN_FUNCTION(&scope, acos);
    // EXPORT_BUILTIN_FUNCTION(&scope, asin);
    // EXPORT_BUILTIN_FUNCTION(&scope, atan);
    // EXPORT_BUILTIN_FUNCTION(&scope, sec);
    // EXPORT_BUILTIN_FUNCTION(&scope, csc);
    // EXPORT_BUILTIN_FUNCTION(&scope, cot);
    // EXPORT_BUILTIN_FUNCTION(&scope, cosh);
    // EXPORT_BUILTIN_FUNCTION(&scope, sinh);
    // EXPORT_BUILTIN_FUNCTION(&scope, tanh);
    // EXPORT_BUILTIN_FUNCTION(&scope, acosh);
    // EXPORT_BUILTIN_FUNCTION(&scope, asinh);
    // EXPORT_BUILTIN_FUNCTION(&scope, atanh);
    // EXPORT_BUILTIN_FUNCTION(&scope, sech);
    // EXPORT_BUILTIN_FUNCTION(&scope, csch);
    // EXPORT_BUILTIN_FUNCTION(&scope, coth);

    // EXPORT_BUILTIN_FUNCTION(&scope, ceil);
    // EXPORT_BUILTIN_FUNCTION(&scope, floor);
    // EXPORT_BUILTIN_FUNCTION(&scope, round);
    // EXPORT_BUILTIN_FUNCTION(&scope, roundeven);
    // EXPORT_BUILTIN_FUNCTION(&scope, trunc);
    // EXPORT_BUILTIN_FUNCTION(&scope, frac);
    // EXPORT_BUILTIN_FUNCTION(&scope, random);
    // EXPORT_BUILTIN_FUNCTION(&scope, ln);

    // EXPORT_BUILTIN_FUNCTION2(&scope, log);
    // EXPORT_BUILTIN_FUNCTION2(&scope, min);
    // EXPORT_BUILTIN_FUNCTION2(&scope, max);

    EXPORT_BUILTIN_CONST(&scope, pi);
    EXPORT_BUILTIN_CONST(&scope, euler);
    EXPORT_BUILTIN_CONST(&scope, catalan);
    EXPORT_BUILTIN_CONST(&scope, e);
    EXPORT_BUILTIN_CONST(&scope, nan);
    EXPORT_BUILTIN_CONST(&scope, inf);

    ALIAS(&scope, NaN,      nan);
    ALIAS(&scope, NAN,      nan);
    ALIAS(&scope, Infinity, inf);
    ALIAS(&scope, Inf,      inf);
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


    if (stdin && !isatty(STDIN_FILENO)) {
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
    }
    if (err) goto error;

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
    if (_g_rand_state_initialized)
        gmp_randclear(_g_rand_state);

    if (_g_eulers_constant) {
        mpc_clear(_g_eulers_constant);
        free(_g_eulers_constant);
    }
    return 0;
}
