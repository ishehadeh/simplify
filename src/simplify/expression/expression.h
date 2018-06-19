/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_EXPRESSION_H_

#include <stdarg.h>

#include "simplify/errors.h"
#include "simplify/expression/expr_types.h"
#include "simplify/rbtree/rbtree.h"

/* the result of comparing two expressions */
typedef enum compare_result {
    COMPARE_RESULT_INCOMPARABLE = 0x0,
    COMPARE_RESULT_EQUAL = 0x1,
    COMPARE_RESULT_LESS = 0x2,
    COMPARE_RESULT_GREATER = 0x4,
} compare_result_t;

/* check for a variable or function in the expression
 *
 * @expr the expression to search
 * @var the variable to search for
 */
int expression_has_variable_or_function(expression_t* expr, variable_t var);

/* Compare two expressions
 *
 * @expr1
 * @expr2
 * @return returns a comparison result
 */
compare_result_t expression_compare(expression_t* expr1, expression_t* expr2);

/* Compare the structure of two expressions (i.e. perform expression_compare, but don't check the numbers)
 *
 * @expr1
 * @expr2
 * @returns true if the expressions have the same structure
 */
bool expression_compare_structure(expression_t* expr1, expression_t* expr2);

/* get the name of the first variable that appears in an expression
 *
 * @expr the expression to search
 * @return returns the variable's name, or NULL if it was not found
 */
variable_t expression_find_variable(expression_t* expr);

/* swap an operator expression's left and right branch
 *
 * @expr _expr->type must be EXPRESSION_TYPE_OPERATOR_ expression to swap
 */
static inline void expression_swap(expression_t* expr) {
    expression_t* lx = EXPRESSION_LEFT(expr);
    expr->operator.left = EXPRESSION_RIGHT(expr);
    expr->operator.right = lx;
}

expression_t* expression_new_uninialized();
expression_list_t* expression_list_new_uninialized();

static inline expression_t* expression_new_operator(expression_t* left, operator_t op, expression_t* right) {
    expression_t* x = expression_new_uninialized();
    expression_init_operator(x, left, op, right);
    return x;
}

static inline expression_t* expression_new_prefix(operator_t op, expression_t* right) {
    expression_t* x = expression_new_uninialized();
    expression_init_prefix(x, op, right);
    return x;
}

static inline expression_t* expression_new_number(mpc_ptr num) {
    expression_t* x = expression_new_uninialized();
    expression_init_number(x, num);
    return x;
}

static inline expression_t* expression_new_number_d(double num) {
    expression_t* x = expression_new_uninialized();
    expression_init_number_d(x, num);
    return x;
}

static inline expression_t* expression_new_number_si(long num) {
    expression_t* x = expression_new_uninialized();
    expression_init_number_si(x, num);
    return x;
}

static inline expression_t* expression_new_variable(variable_t var) {
    expression_t* x = expression_new_uninialized();
    expression_init_variable(x, var, strlen(var));
    return x;
}

static inline expression_t* expression_new_function(variable_t name, int param_count, ...) {
    expression_t* x = expression_new_uninialized();
    va_list args;
    va_start(args, param_count);
    expression_list_t* params = expression_list_new_uninialized();
    expression_list_init(params);

    for (int i = 0; i < param_count; ++i) {
        expression_list_append(params, va_arg(args, expression_t*));
    }
    va_end(args);
    expression_init_function(x, name, strlen(name), params);
    return x;
}

#endif  // SIMPLIFY_EXPRESSION_EXPRESSION_H_
