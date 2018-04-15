/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_H_

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "simplify/scalar/scalar.h"
#include "simplify/errors.h"
#include "simplify/rbtree/rbtree.h"

typedef char                  operator_t;
typedef char*                 variable_t;
typedef union expression      expression_t;
typedef enum  expression_type expression_type_t;
typedef struct scope          scope_t;

enum expression_type {
    EXPRESSION_TYPE_NULL,
    EXPRESSION_TYPE_NUMBER,
    EXPRESSION_TYPE_PREFIX,
    EXPRESSION_TYPE_OPERATOR,
    EXPRESSION_TYPE_VARIABLE,
};

struct scope {
    rbtree_t variables;
    rbtree_t functions;
};

struct expression_number {
    expression_type_t type;
    scalar_t value;
};

struct expression_prefix {
    expression_type_t type;
    operator_t    prefix;
    expression_t* right;
};

struct expression_variable {
    expression_type_t type;
    variable_t value;
};

struct expression_operator {
    expression_type_t type;

    expression_t* left;
    operator_t    infix;
    expression_t* right;
};

union expression {
    expression_type_t type;
    struct expression_number   number;
    struct expression_variable variable;
    struct expression_prefix   prefix;
    struct expression_operator operator;
};

static inline expression_t* new_expression() {
    return malloc(sizeof(expression_t));
}

static inline expression_t* new_number_expression(scalar_t value) {
    expression_t* expr = new_expression();
    expr->type = EXPRESSION_TYPE_NUMBER;

    SCALAR_INIT(expr->number.value);
    SCALAR_SET(value, expr->number.value);

    return expr;
}

static inline expression_t* new_variable_expression(char* value, size_t length) {
    expression_t* expr = new_expression();
    expr->type = EXPRESSION_TYPE_VARIABLE;
    expr->variable.value = malloc(length + 1);
    expr->variable.value[length] = 0;
    strncpy(expr->variable.value, value, length);

    return expr;
}

static inline expression_t* new_prefix_expression(operator_t op, expression_t* right) {
    expression_t* expr = new_expression();
    expr->type = EXPRESSION_TYPE_PREFIX;
    expr->prefix.prefix = op;
    expr->prefix.right  = right;
    return expr;
}

static inline expression_t* new_operator_expression(expression_t* left, operator_t op, expression_t* right) {
    expression_t* expr = new_expression();
    expr->type = EXPRESSION_TYPE_OPERATOR;
    expr->operator.infix = op;
    expr->operator.right = right;
    expr->operator.left  = left;
    return expr;
}

void expression_free(expression_t* expr);
void expression_clean(expression_t* expr);
error_t expression_simplify(expression_t* expr, scope_t* scope);
error_t expression_print(expression_t* expr);
error_t expression_isolate_variable(expression_t*, variable_t);

static inline void scope_init(scope_t* scope) {
    rbtree_init(&scope->variables);
    rbtree_init(&scope->functions);
}

static inline error_t scope_define(scope_t* scope, char* variable, expression_t* value) {
    return rbtree_insert(&scope->variables, variable, value);
}

static inline void scope_clean(scope_t* scope) {
    rbtree_clean(&scope->variables, (void(*)(void*))&expression_free);
    rbtree_clean(&scope->functions, (void(*)(void*))&expression_free);
}

#endif  // SIMPLIFY_EXPRESSION_H_
