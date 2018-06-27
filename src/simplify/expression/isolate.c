/* Copyright Ian Shehadeh 2018 */

#include <stdbool.h>

#include "simplify/expression/evaluate.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/isolate.h"
#include "simplify/math/math.h"

error_t _expression_isolate_variable_recursive(expression_t* expr, expression_t** target, variable_t var);

void _expression_invert_operand(expression_t* expr) {
    switch (expr->operator.infix) {
        case '+':
            expr->operator.infix = '-';
            break;
        case '-':
            expr->operator.infix = '+';
            break;
        case '/':
            expr->operator.infix = '*';
            break;
        case '*':
        case '(':
            expr->operator.infix = '/';
            break;
        case '^':
            expr->operator.infix = '\\';
            break;
        case '\\':
            expr->operator.infix = '^';
            break;
        default:
            break;
    }
}

bool _expression_operator_is_reversible(expression_t* expr) {
    switch (expr->operator.infix) {
        case '-':
        case '/':
        case '^':
        case '\\':
            return false;
        default:
            return true;
    }
}

/* transform expr into `e^x = y`, the call _expression_isolate_variable_recursive on `y`
 * @y the y value of the logarithm
 * @expr the output expression
 * @returns returns an error code
 */
error_t _expression_setup_natural_log(expression_t* y, expression_t* expr) {
    error_t err = ERROR_NO_ERROR;

    if (EXPRESSION_IS_NUMBER(y)) {
        mpc_ptr loge = malloc(sizeof(mpc_t));
        mpc_init2(loge, simplify_get_default_precision());
        mpc_log(loge, y->number.value, MPC_RNDNN);
        expression_init_number(expr, loge);
        expression_clean(y);
        return ERROR_NO_ERROR;
    }

#if defined(NATURAL_LOG_BUILTIN)
    expression_list_t* args = malloc(sizeof(expression_list_t));
    expression_list_init(args);
    expression_list_append(args, y);
    expression_init_function(expr, NATURAL_LOG_BUILTIN, sizeof NATURAL_LOG_BUILTIN, args);
#else
    expression_t* expr_e;
    expr_e = expression_new_variable(E_BUILTIN);
    expression_init_operator(
        expr, y, '=',
        expression_new_operator(expr_e, '^',
                                expression_new_variable(MANGLE_INTERNAL_VARIABLE("NaturalLogarithmResult"))));

    err = _expression_isolate_variable_recursive(expr->operator.right, & expr->operator.left,
                                                 MANGLE_INTERNAL_VARIABLE("NaturalLogarithmResult"));
    if (err) return err;
    expression_collapse_right(expr);
#endif
    return err;
}

error_t _expression_isolate_variable_recursive(expression_t* expr, expression_t** target, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return ERROR_VARIABLE_NOT_PRESENT;
        case EXPRESSION_TYPE_OPERATOR: {
            if (expression_is_comparison(expr) || expr->operator.infix == ':') {
                error_t err = _expression_isolate_variable_recursive(expr->operator.left, & expr->operator.right, var);
                if (err && err != ERROR_VARIABLE_NOT_PRESENT) return err;

                if (err) {
                    err = _expression_isolate_variable_recursive(expr->operator.right, & expr->operator.left, var);
                    if (err) return err;
                }
                return ERROR_NO_ERROR;
            }

            expression_t* new_target = malloc(sizeof(expression_t));
            new_target->type = EXPRESSION_TYPE_OPERATOR;
            new_target->operator.infix = expr->operator.infix;

            if (expression_has_variable_or_function(expr->operator.left, var)) {
                new_target->operator.right = expr->operator.right;
                new_target->operator.left =* target;
                _expression_invert_operand(new_target);
            } else if (expression_has_variable_or_function(expr->operator.right, var)) {
                if (expr->operator.infix == '^') {
                    // Looks Logarithm-ish so handle it as a special case
                    expression_t* y = expr->operator.right;
                    expression_t* b = expr->operator.left;
                    expression_t* x = *target;

                    if ((EXPRESSION_IS_VARIABLE(b) && !strcmp(b->variable.value, E_BUILTIN))) {
                        *expr = *y;
                        *target = expression_new_operator(expression_new_variable(E_BUILTIN), '^', *target);
                        _expression_isolate_variable_recursive(expr, target, var);
                        return ERROR_NO_ERROR;
                    }
                    error_t err = _expression_setup_natural_log(x, new_target);
                    if (err) return err;

                    expression_t oldy = *y;
                    err = _expression_setup_natural_log(b, expr->operator.right);
                    if (err) return err;

                    *expr->operator.left = oldy;
                    expr->operator.infix = '*';
                    *target = new_target;
                    _expression_isolate_variable_recursive(expr, target, var);
                    break;
                } else if (_expression_operator_is_reversible(expr)) {
                    new_target->operator.right = expr->operator.left;
                    new_target->operator.left =* target;
                    _expression_invert_operand(new_target);
                } else {
                    new_target->operator.left = expr->operator.left;
                    new_target->operator.right =* target;
                }
            } else {
                free(new_target);
                return ERROR_VARIABLE_NOT_PRESENT;
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
        case EXPRESSION_TYPE_PREFIX: {
            if (expression_has_variable_or_function(expr->prefix.right, var)) {
                expression_t* new_target = malloc(sizeof(expression_t));
                new_target->type = EXPRESSION_TYPE_PREFIX;
                new_target->prefix.right = *target;
                switch (EXPRESSION_OPERATOR(expr)) {
                    case '+':
                        new_target->prefix.prefix = '+';
                        break;
                    case '-':
                        new_target->prefix.prefix = '-';
                        break;
                    default:
                        return ERROR_INVALID_PREFIX;
                }
                *target = new_target;
                if (!_expression_isolate_variable_recursive(expr->prefix.right, target, var)) {
                    *expr = *expr->prefix.right;
                } else {
                    return ERROR_VARIABLE_NOT_PRESENT;
                }
                return ERROR_NO_ERROR;
            }
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
    if (!expression_has_variable_or_function(expr, var)) return ERROR_VARIABLE_NOT_PRESENT;

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
    if (EXPRESSION_IS_VARIABLE(expr->operator.right) || EXPRESSION_IS_FUNCTION(expr->operator.right)) {
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
