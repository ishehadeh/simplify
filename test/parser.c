/* Copyright Ian Shehadeh 2018 */

#include <stdarg.h>

#include "test/test.h"
#include "simplify/parser.h"

static inline expression_t* expression_new_operator(expression_t* left, operator_t op, expression_t* right) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_operator(x, left, op, right);
    return x;
}

static inline expression_t* expression_new_prefix(operator_t op, expression_t* right) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_prefix(x, op, right);
    return x;
}

static inline expression_t* expression_new_number(double num) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_number_d(x, num);
    return x;
}

static inline expression_t* expression_new_variable(variable_t var) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_variable(x, var, strlen(var));
    return x;
}

static inline expression_t* expression_new_function(variable_t name, int param_count, ...) {
    expression_t* x = malloc(sizeof(expression_t));
    va_list args;
    va_start(args, param_count);
    expression_list_t* params = malloc(sizeof(expression_list_t));
    expression_list_init(params);

    for (int i = 0; i < param_count; ++i) {
        expression_list_append(params, va_arg(args, expression_t*));
    }
    va_end(args);
    expression_init_function(x, name, strlen(name), params);
    return x;
}

int main() {
    struct {
        char*        string;
        expression_t* expr;
    } __string_expr_pairs[10] = {
        { "2 * 5.5",
            expression_new_operator(expression_new_number(2), '*', expression_new_number(5.5))
        },
        {
            "2.5x",
            expression_new_operator(expression_new_number(2.5), '*', expression_new_variable("x"))
        },
        { "2 * ( 5 + 9 )",
            expression_new_operator(
                expression_new_number(2),
                '*',
                expression_new_operator(expression_new_number(5), '+', expression_new_number(9))),
        },
        { "2 + 3^2 * 5.5",
            expression_new_operator(
                expression_new_number(2),
                '+',
                expression_new_operator(
                    expression_new_operator(expression_new_number(3), '^', expression_new_number(2)),
                    '*',
                    expression_new_number(5.5)))
        },
        { "1 \\ 2 = 73",
            expression_new_operator(
                expression_new_operator(expression_new_number(1), '\\', expression_new_number(2)),
                '=',
                expression_new_number(73))
        },
        { "p = n * 8 - t",
            expression_new_operator(
                expression_new_variable("p"),
                '=',
                expression_new_operator(
                    expression_new_operator(
                        expression_new_variable("n"),
                        '*',
                        expression_new_number(8)),
                    '-',
                    expression_new_variable("t")))
        },
        { "10 * 20 * 30^1 * 40",
            expression_new_operator(
                expression_new_operator(
                    expression_new_operator(
                        expression_new_number(10),
                        '*',
                        expression_new_number(20)),
                    '*',
                    expression_new_operator(
                        expression_new_number(30),
                        '^',
                        expression_new_number(1))),
                '*',
                expression_new_number(40))
        },
        { "2 * -5.5",
            expression_new_operator(
                expression_new_number(2),
                '*',
                expression_new_prefix('-', expression_new_number(5.5)))
        },
        { "f(x, y)",
            expression_new_function(
                "f", 2,
                expression_new_variable("x"),
                expression_new_variable("y")),
        },
        { "f(x + 2) : 5 * x * 3",
            expression_new_operator(
                expression_new_function(
                    "f", 1,
                    expression_new_operator(
                        expression_new_variable("x"),
                        '+',
                        expression_new_number(2))),
                ':',
                expression_new_operator(
                    expression_new_operator(
                        expression_new_number(5),
                        '*',
                        expression_new_variable("x")),
                    '*',
                    expression_new_number(3))),
        },
    };


    error_t err;
    for (int i = 0; i < 10; ++i) {
        expression_t expr;
        err = parse_string(__string_expr_pairs[i].string, &expr);
        if (err)
            FATAL("failed to parse string: %s", error_string(err));
        expression_assert_eq(&expr, __string_expr_pairs[i].expr);
    }
}