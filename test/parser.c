/* Copyright Ian Shehadeh 2018 */

#include "simplify/parser.h"
#include "test/test.h"

int main() {
    struct {
        char* string;
        expression_t* expr;
    } __string_expr_pairs[] = {
        {"2 * 5.5", expression_new_operator(expression_new_number_d(2), '*', expression_new_number_d(5.5))},
        {"2.5x", expression_new_operator(expression_new_number_d(2.5), '*', expression_new_variable("x"))},
        {
            "2 * ( 5 + 9 )",
            expression_new_operator(
                expression_new_number_d(2), '*',
                expression_new_operator(expression_new_number_d(5), '+', expression_new_number_d(9))),
        },
        {"2 + 3^2 * 5.5",
         expression_new_operator(expression_new_number_d(2), '+',
                                 expression_new_operator(expression_new_operator(expression_new_number_d(3), '^',
                                                                                 expression_new_number_d(2)),
                                                         '*', expression_new_number_d(5.5)))},
        {"1 \\ 2 = 73",
         expression_new_operator(expression_new_operator(expression_new_number_d(1), '\\', expression_new_number_d(2)),
                                 '=', expression_new_number_d(73))},
        {"p = n * 8 - t",
         expression_new_operator(expression_new_variable("p"), '=',
                                 expression_new_operator(expression_new_operator(expression_new_variable("n"), '*',
                                                                                 expression_new_number_d(8)),
                                                         '-', expression_new_variable("t")))},
        {"10 * 20 * 30^1 * 40",
         expression_new_operator(
             expression_new_operator(
                 expression_new_operator(expression_new_number_d(10), '*', expression_new_number_d(20)), '*',
                 expression_new_operator(expression_new_number_d(30), '^', expression_new_number_d(1))),
             '*', expression_new_number_d(40))},
        {"2 * -5.5", expression_new_operator(expression_new_number_d(2), '*',
                                             expression_new_prefix('-', expression_new_number_d(5.5)))},
        {
            "f(x, y)",
            expression_new_function("f", 2, expression_new_variable("x"), expression_new_variable("y")),
        },
        {"4 * (3 * x) \\ 4 - 36",
         expression_new_operator(
             expression_new_operator(expression_new_number_d(4), '*',
                                     expression_new_operator(expression_new_operator(expression_new_number_d(3), '*',
                                                                                     expression_new_variable("x")),
                                                             '\\', expression_new_number_d(4))),
             '-', expression_new_number_d(36))},
        {
            "f(x + 2) : 5 * x * 3",
            expression_new_operator(
                expression_new_function(
                    "f", 1, expression_new_operator(expression_new_variable("x"), '+', expression_new_number_d(2))),
                ':',
                expression_new_operator(
                    expression_new_operator(expression_new_number_d(5), '*', expression_new_variable("x")), '*',
                    expression_new_number_d(3))),
        },
        {"x^ 3 * 5(6 + 0 \\ 1)",
         expression_new_operator(
             expression_new_operator(expression_new_variable("x"), '^', expression_new_number_d(3)), '*',
             expression_new_operator(expression_new_number_d(5), '*',
                                     expression_new_operator(expression_new_number_d(6), '+',
                                                             expression_new_operator(expression_new_number_d(0), '\\',
                                                                                     expression_new_number_d(1)))))}};

    error_t err;
    for (int i = 0; i < (int)(sizeof(__string_expr_pairs) / sizeof(__string_expr_pairs[0])); ++i) {
        expression_t expr;
        err = parse_string(__string_expr_pairs[i].string, &expr);
        printf("starting test #%d...\n", i + 1);
        if (err) FATAL("failed to parse string: %s", error_string(err));
        expression_assert_eq(&expr, __string_expr_pairs[i].expr);
    }
}
