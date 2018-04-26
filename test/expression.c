/* Copyright Ian Shehadeh 2018 */

#include "test/test.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/stringify.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/evaluate.h"

int main() {
    struct {
        char*        string;
        expression_t* expr;
    } __string_expr_pairs[] = {
        { "2 * 5.5",
            expression_new_number(11),
        },
        { "2 * 9x",
            expression_new_operator(
                expression_new_number(18),
                '*',
                expression_new_variable("x"))
        },
        { "4 \\ 2 = 5",
            expression_new_number(5)
        },
        { "x ^ 3 * 5(6 + 0 \\ 1)",
            expression_new_operator(
                expression_new_operator(
                    expression_new_variable("x"),
                    '^',
                    expression_new_number(3)),
                '*',
                expression_new_number(30))
        }
    };


    error_t err;
    for (int i = 0; i < (int) (sizeof(__string_expr_pairs) / sizeof(__string_expr_pairs[0])); ++i) {
        expression_t expr;
        scope_t      scope;

        printf("starting test #%d...\n", i + 1);
        err = parse_string(__string_expr_pairs[i].string, &expr);
        if (err)
            FATAL("failed to parse string: %s", error_string(err));

        scope_init(&scope);
        err = expression_evaluate(&expr, &scope);
        if (err)
            FATAL("failed to parse string: %s", error_string(err));

        expression_assert_eq(&expr, __string_expr_pairs[i].expr);
        scope_clean(&scope);
        expression_clean(&expr);
    }
}