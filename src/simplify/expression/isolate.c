/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/isolate.h"
#include "simplify/expression/expression.h"


error_t _expression_isolate_variable_recursive(expression_t* expr, expression_t** target, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return ERROR_VARIABLE_NOT_PRESENT;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expression_is_comparison(expr) || expr->operator.infix == ':') {
                error_t err = _expression_isolate_variable_recursive(expr->operator.left, &expr->operator.right, var);
                if (err && err != ERROR_VARIABLE_NOT_PRESENT)
                    return err;

                if (err) {
                    err = _expression_isolate_variable_recursive(expr->operator.right, &expr->operator.left, var);
                    if (err) return err;
                }
                return ERROR_NO_ERROR;
            }

            expression_t* new_target = malloc(sizeof(expression_t));
            new_target->type = EXPRESSION_TYPE_OPERATOR;

            new_target->operator.left  = *target;
            if (expression_has_variable_or_function(expr->operator.left, var)) {
                new_target->operator.right = expr->operator.right;
            } else if (expression_has_variable_or_function(expr->operator.right, var)) {
                new_target->operator.right = expr->operator.left;
            } else {
                free(new_target);
                return ERROR_VARIABLE_NOT_PRESENT;
            }

            switch (expr->operator.infix) {
                case '+':
                    new_target->operator.infix = '-';
                    break;
                case '-':
                    new_target->operator.infix = '+';
                    break;
                case '/':
                    new_target->operator.infix = '*';
                    break;
                case '*':
                case '(':
                    new_target->operator.infix = '/';
                    break;
                case '^':
                    new_target->operator.infix = '\\';
                    break;
                case '\\':
                    new_target->operator.infix = '^';
                    break;
                default:
                    break;
            }

            *target = new_target;
            expression_t new_expr;
            if (!_expression_isolate_variable_recursive(expr->operator.left, target, var)) {
                new_expr = *expr->operator.left;
            } else if (!_expression_isolate_variable_recursive(expr->operator.right, target, var)) {
                new_expr = *expr->operator.right;
            } else {
                return ERROR_VARIABLE_NOT_PRESENT;
            }

            *expr = new_expr;
            return ERROR_NO_ERROR;
        }
        case EXPRESSION_TYPE_PREFIX:
        {
            switch (EXPRESSION_OPERATOR(expr)) {
                case '+':
                    break;
                case '-':
                    mpfr_neg(expr->prefix.right->number.value, expr->prefix.right->number.value, MPFR_RNDF);
                    break;
                default:
                    return ERROR_INVALID_PREFIX;
            }
            expression_t num = *expr->prefix.right;
            free(expr->prefix.right);
            *expr = num;
            return ERROR_VARIABLE_NOT_PRESENT;
        }
        case EXPRESSION_TYPE_FUNCTION:
            if (strcmp(var, expr->function.name) == 0) {
                return ERROR_NO_ERROR;
            } else {
                return ERROR_VARIABLE_NOT_PRESENT;
            }
        case EXPRESSION_TYPE_VARIABLE:
            if (strcmp(var, expr->variable.value) == 0) {
                return ERROR_NO_ERROR;
            } else {
                return ERROR_VARIABLE_NOT_PRESENT;
            }
    }
    return ERROR_NO_ERROR;
}

error_t expression_isolate_variable(expression_t* expr, variable_t var) {
    if (!expression_has_variable_or_function(expr, var))
        return ERROR_VARIABLE_NOT_PRESENT;
    
    if (!expression_is_comparison(expr) && expr->operator.infix != ':') {
        expression_t* new_left = malloc(sizeof(expression_t));

        *new_left = *expr;
        expr->type = EXPRESSION_TYPE_OPERATOR;
        expr->operator.infix = '=';
        expr->operator.left = new_left;
        expr->operator.right = malloc(sizeof(expression_t));
        expression_init_number_si(expr->operator.right, 0);
    }

    error_t err = _expression_isolate_variable_recursive(expr, NULL, var);
    if (err) return err;

    /* make sure the variable is always on the left */
    if (expr->operator.right->type == EXPRESSION_TYPE_VARIABLE) {
        expression_t* right = expr->operator.right;
        expr->operator.right = expr->operator.left;
        expr->operator.left = right;
    } else {
        /* flip the sign if it was a < or > */
        if (expr->operator.infix == '<') {
            expr->operator.infix = '>';
        } else if (expr->operator.infix == '>') {
            expr->operator.infix = '<';
        }
    }

    return err;
}