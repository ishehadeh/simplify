/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"


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
