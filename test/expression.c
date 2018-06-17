/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"
#include "simplify/builtins.h"
#include "simplify/expression/evaluate.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/simplify.h"
#include "simplify/expression/stringify.h"
#include "test/test.h"

#define OP_EVALUATE 1
#define OP_ISOLATE_X 2
#define OP_COMPLEX_SIMPLIFY 4
#define OP_BOOLEAN_TRUE 8
#define OP_BOOLEAN_FALSE 16

int main() {
    struct {
        char* string;
        int ops;
        expression_t* expr;
    } __string_expr_pairs[] = {
        {
            "2 * 5.5",
            OP_EVALUATE,
            expression_new_number_d(11),
        },
        {"2 * 9x", OP_EVALUATE,
         expression_new_operator(expression_new_number_d(18), '*', expression_new_variable("x"))},
        {"4 \\ 2 = 5", OP_EVALUATE | OP_BOOLEAN_FALSE, expression_new_number_d(5)},
        {"4 < 10", OP_EVALUATE | OP_BOOLEAN_TRUE, expression_new_number_d(10)},
        {"9 > 51", OP_EVALUATE | OP_BOOLEAN_FALSE, expression_new_number_d(51)},
        {"9 > 2", OP_EVALUATE | OP_BOOLEAN_TRUE, expression_new_number_d(2)},
        {"9 = 9", OP_EVALUATE | OP_BOOLEAN_TRUE, expression_new_number_d(9)},
        {"x ^ 3 * 5(6 + 0 \\ 1)", OP_EVALUATE,
         expression_new_operator(expression_new_operator(expression_new_variable("x"), '^', expression_new_number_d(3)),
                                 '*', expression_new_number_d(30))},
        {"(0 - x) = 1", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(-1))},
        {"5 / x = 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(2.5))},
        {"4 \\ x = 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(2))},
        {
            "cos(pi2)",
            OP_EVALUATE,
            expression_new_number_si(1),
        },
        {"f(x, y): 10x^y", OP_EVALUATE,
         expression_new_operator(
             expression_new_number_d(10), '*',
             expression_new_operator(expression_new_variable("x"), '^', expression_new_variable("y")))},
        {"5 / x : 2 * 10", OP_EVALUATE, expression_new_number_d(0.25)},
        {"+(-5 * 2)", OP_EVALUATE, expression_new_number_d(-10)},
        {"-(3 ^ 4)", OP_EVALUATE, expression_new_number_d(-81)},
        {"-x = 4", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(-4))},
        {"99 = -x * 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(-49.5))},
        {"99 = x / 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(198))},
        {"8 = 2 * x ^ 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(2))},
        {"11 = x \\ 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(121))},
        {"0 = x - 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(2))},
        {"-8 = +x + 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(-10))},
        {"-8 = x(y) + 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_function("x", 1, expression_new_variable("y")), '=',
                                 expression_new_number_d(-10))},
        {"x(y, z) + 10", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(
             expression_new_function("x", 2, expression_new_variable("y"), expression_new_variable("z")), '=',
             expression_new_number_d(-10))},
        {"x * x * 3", OP_EVALUATE | OP_COMPLEX_SIMPLIFY,
         expression_new_operator(expression_new_operator(expression_new_variable("x"), '^', expression_new_number_d(2)),
                                 '*', expression_new_number_d(3))},
        {"3 + x * x * x * 3", OP_EVALUATE | OP_COMPLEX_SIMPLIFY,
         expression_new_operator(expression_new_number_d(3), '+',
                                 expression_new_operator(expression_new_operator(expression_new_variable("x"), '^',
                                                                                 expression_new_number_d(3)),
                                                         '*', expression_new_number_d(3)))},
        {"3 * x * x * x * 3", OP_EVALUATE | OP_COMPLEX_SIMPLIFY,
         expression_new_operator(expression_new_operator(expression_new_number_d(3), '*',
                                                         expression_new_operator(expression_new_variable("x"), '^',
                                                                                 expression_new_number_d(3))),
                                 '*', expression_new_number_d(3))},
        {"100 ^ x * x * x * 3", OP_EVALUATE | OP_COMPLEX_SIMPLIFY,
         expression_new_operator(
             expression_new_operator(
                 expression_new_operator(expression_new_number_d(100), '^', expression_new_variable("x")), '*',
                 expression_new_operator(expression_new_variable("x"), '^', expression_new_number_d(2))),
             '*', expression_new_number_d(3))},
        {"5 + x", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_si(-5))},
        {"2 ^ x = 2", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(1))},
        {"2 ^ x = 4", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_d(2))},
        {"log(2, x + 5) = 3", OP_EVALUATE | OP_ISOLATE_X,
         expression_new_operator(expression_new_variable("x"), '=', expression_new_number_si(3))},
    };

    error_t err;
    for (int i = 0; i < (int)(sizeof(__string_expr_pairs) / sizeof(__string_expr_pairs[0])); ++i) {
        expression_t expr;
        scope_t scope;

        printf("starting test #%d (%s)...", i + 1, __string_expr_pairs[i].string);
        err = parse_string(__string_expr_pairs[i].string, &expr);
        if (err) FATAL("failed to parse string \"%s\": %s", __string_expr_pairs[i].string, error_string(err));

        scope_init(&scope);
        simplify_export_builtins(&scope);

        if (__string_expr_pairs[i].ops & OP_EVALUATE) {
            err = expression_evaluate(&expr, &scope);
            if (err) FATAL("failed to evaluate \"%s\": %s", __string_expr_pairs[i].string, error_string(err));
        }

        if (__string_expr_pairs[i].ops & OP_COMPLEX_SIMPLIFY) {
            err = expression_simplify(&expr);
            if (err)
                FATAL("failed to simplify expression \"%s\": %s", __string_expr_pairs[i].string, error_string(err));
        }

        if (__string_expr_pairs[i].ops & OP_ISOLATE_X) {
            err = expression_isolate_variable(&expr, "x");
            if (err)
                FATAL("failed to isolate 'x' in expression \"%s\": %s", __string_expr_pairs[i].string,
                      error_string(err));
            err = expression_evaluate(&expr, &scope);
            if (err) FATAL("failed to evaluate (pass 2) \"%s\": %s", __string_expr_pairs[i].string, error_string(err));
        }

        if (__string_expr_pairs[i].ops & OP_BOOLEAN_FALSE &&
            expression_evaluate_comparisons(&expr) != EXPRESSION_RESULT_FALSE) {
            FATAL("failed to evaluate, expression was not false \"%s\"", __string_expr_pairs[i].string);
        }

        if (__string_expr_pairs[i].ops & OP_BOOLEAN_TRUE &&
            expression_evaluate_comparisons(&expr) != EXPRESSION_RESULT_TRUE) {
            FATAL("failed to evaluate, expression was not true \"%s\"", __string_expr_pairs[i].string);
        }

        expression_assert_eq(&expr, __string_expr_pairs[i].expr);
        scope_clean(&scope);
        expression_clean(&expr);
        printf("done\n");
    }
}