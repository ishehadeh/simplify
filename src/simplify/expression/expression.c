/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"

variable_t _expression_find_variable_recursive(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
        case EXPRESSION_TYPE_NUMBER:
            return NULL;
        case EXPRESSION_TYPE_OPERATOR:
        {
            variable_t var = _expression_find_variable_recursive(expr->operator.left);
            if (var) return var;
            var = _expression_find_variable_recursive(expr->operator.right);
            return var;
        }
        case EXPRESSION_TYPE_PREFIX:
            return _expression_find_variable_recursive(expr->operator.right);
        case EXPRESSION_TYPE_VARIABLE:
            return expr->variable.value;
    }
    return NULL;
}

variable_t _expression_find_function_recursive(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_VARIABLE:
        case EXPRESSION_TYPE_NUMBER:
            return NULL;
        case EXPRESSION_TYPE_OPERATOR:
        {
            variable_t var = _expression_find_function_recursive(expr->operator.left);
            if (var) return var;
            var = _expression_find_function_recursive(expr->operator.right);
            return var;
        }
        case EXPRESSION_TYPE_PREFIX:
            return _expression_find_function_recursive(expr->operator.right);
        case EXPRESSION_TYPE_FUNCTION:
            return expr->function.name;
    }
    return NULL;
}


int _expression_has_variable_or_function_recursive(expression_t* expr, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return 0;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (_expression_has_variable_or_function_recursive(expr->operator.left, var))
                return 1;
            if (_expression_has_variable_or_function_recursive(expr->operator.right, var))
                return 1;
            return 0;
        }
        case EXPRESSION_TYPE_PREFIX:
        {
            if (_expression_has_variable_or_function_recursive(expr->prefix.right, var))
                return 1;
            return 0;
        }
        case EXPRESSION_TYPE_VARIABLE:
            return strcmp(var, expr->variable.value) == 0;
        case EXPRESSION_TYPE_FUNCTION:
            return strcmp(var, expr->function.name) == 0;
    }
    return 0;
}


int expression_has_variable_or_function(expression_t* expr, variable_t var) {
    return _expression_has_variable_or_function_recursive(expr, var);
}

variable_t expression_find_variable(expression_t* expr) {
    return _expression_find_variable_recursive(expr);
}

variable_t expression_find_function(expression_t* expr) {
    return _expression_find_function_recursive(expr);
}
