/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_EXPRESSION_H_

#include <stdarg.h>

#include "simplify/errors.h"
#include "simplify/rbtree/rbtree.h"
#include "simplify/expression/expr_types.h"

/* check for a variable or function in the expression
 *
 * @expr the expression to search
 * @var the variable to search for
 */
int expression_has_variable_or_function(expression_t* expr, variable_t var);

/* get the name of the first variable that appears in an expression
 *
 * @expr the expression to search
 * @return returns the variable's name, or NULL if it was not found
 */ 
variable_t expression_find_variable(expression_t* expr);

/* get the name of the first function that appears in an expression
 *
 * @expr the expression to search
 * @return returns the function's name, or NULL if it was not found
 */ 
variable_t expression_find_function(expression_t* expr);

/* swap an operator expression's left and right branch
 * 
 * @expr _expr->type must be EXPRESSION_TYPE_OPERATOR_ expression to swap
 */
static inline void expression_swap(expression_t* expr) {
    expression_t* lx = EXPRESSION_LEFT(expr);
    expr->operator.left = EXPRESSION_RIGHT(expr);
    expr->operator.right = lx;
}

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


static inline expression_t* expression_new_number(mpfr_t num) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_number(x, num);
    return x;
}


static inline expression_t* expression_new_number_d(double num) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_number_d(x, num);
    return x;
}

static inline expression_t* expression_new_number_si(long num) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_number_si(x, num);
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


#endif  // SIMPLIFY_EXPRESSION_EXPRESSION_H_
