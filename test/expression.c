/* Copyright Ian Shehadeh 2018 */

#include "test/test.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/stringify.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/simplify.h"
#include "simplify/expression/evaluate.h"
#include "simplify/builtins.h"

#define OP_EVALUATE         1
#define OP_ISOLATE_X        2
#define OP_COMPLEX_SIMPLIFY 4

DEFINE_MPFR_FUNCTION(cos)
DEFINE_MPFR_CONST(pi)

int main() {
    struct {
        char*         string;
        int           ops;
        expression_t* expr;
    } __string_expr_pairs[] = {
        { "2 * 5.5", OP_EVALUATE,
            expression_new_number_d(11),
        },
        { "2 * 9x", OP_EVALUATE,
            expression_new_operator(
                expression_new_number_d(18),
                '*',
                expression_new_variable("x"))
        },
        { "4 \\ 2 = 5", OP_EVALUATE,
            expression_new_number_d(5)
        },
        { "x ^ 3 * 5(6 + 0 \\ 1)", OP_EVALUATE,
            expression_new_operator(
                expression_new_operator(
                    expression_new_variable("x"),
                    '^',
                    expression_new_number_d(3)),
                '*',
                expression_new_number_d(30))
        },
        {"(0 - x) = 1", OP_EVALUATE | OP_ISOLATE_X,
            expression_new_operator(
                expression_new_variable("x"),
                '=',
                expression_new_number_d(-1))
        },
        {"5 / x = 2", OP_EVALUATE | OP_ISOLATE_X,
            expression_new_operator(
                expression_new_variable("x"),
                '=',
                expression_new_number_d(2.5))
        },
        {"4 \\ x = 2", OP_EVALUATE | OP_ISOLATE_X,
            expression_new_operator(
                expression_new_variable("x"),
                '=',
                expression_new_number_d(2))
        },
        {"cos(pi2)", OP_EVALUATE,
            expression_new_number_si(1),
        }
    };


    error_t err;
    for (int i = 0; i < (int) (sizeof(__string_expr_pairs) / sizeof(__string_expr_pairs[0])); ++i) {
        expression_t expr;
        scope_t      scope;

        printf("starting test #%d...\n", i + 1);
        err = parse_string(__string_expr_pairs[i].string, &expr);
        if (err)
            FATAL("failed to parse string \"%s\": %s", __string_expr_pairs[i].string, error_string(err));

        scope_init(&scope);
        EXPORT_MPFR_FUNCTION(&scope, cos);
        EXPORT_MPFR_CONST(&scope, pi);


        if (__string_expr_pairs[i].ops & OP_EVALUATE) {
            err = expression_evaluate(&expr, &scope);
            if (err)
                FATAL("failed to evaluate \"%s\": %s", __string_expr_pairs[i].string, error_string(err));
        }

        if (__string_expr_pairs[i].ops & OP_COMPLEX_SIMPLIFY) {
            err = expression_simplify(&expr);
            if (err)
                FATAL("failed to simplify expression \"%s\": %s", __string_expr_pairs[i].string, error_string(err));
        }

        if (__string_expr_pairs[i].ops & OP_ISOLATE_X) {
            err = expression_isolate_variable(&expr, "x");
            if (err)
                FATAL("failed to isolate 'x' in expression \"%s\": %s",
                    __string_expr_pairs[i].string, error_string(err));
            err = expression_evaluate(&expr, &scope);
            if (err)
                FATAL("failed to evaluate (pass 2) \"%s\": %s", __string_expr_pairs[i].string, error_string(err));
        }

        expression_assert_eq(&expr, __string_expr_pairs[i].expr);
        scope_clean(&scope);
        expression_clean(&expr);
    }
}