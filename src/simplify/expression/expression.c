/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"

error_t _expression_apply_operator(expression_t* expr, expression_t* out, scope_t* scope) {
    mpfr_t result;
    mpfr_init(result);

    switch (expr->operator.infix) {
        case '+':
            mpfr_add(result, expr->operator.left->number.value, expr->operator.right->number.value, MPFR_RNDF);
            break;
        case '-':
            mpfr_sub(result, expr->operator.left->number.value, expr->operator.right->number.value, MPFR_RNDF);
            break;
        case '/':
            mpfr_div(result, expr->operator.left->number.value, expr->operator.right->number.value, MPFR_RNDF);
            break;
        case '*':
        case '(':
            mpfr_mul(result, expr->operator.left->number.value, expr->operator.right->number.value, MPFR_RNDF);
            break;
        case '^':
            mpfr_pow(result, expr->operator.left->number.value, expr->operator.right->number.value, MPFR_RNDF);
            break;
        case '\\':
            mpfr_rootn_ui(result, expr->operator.left->number.value,
                                  mpfr_get_ui(expr->operator.right->number.value, MPFR_RNDD),
                                  MPFR_RNDF);
            break;
        case '=':
        case '>':
        case '<':
        {
            if (scope->boolean != 0) {
                int x = mpfr_cmp(expr->operator.left->number.value, expr->operator.right->number.value);
                scope->boolean = ((expr->operator.infix == '<' && x < 0)
                                    || (expr->operator.infix == '>' && x > 0)
                                    || (expr->operator.infix == '=' && x == 0));
            }
            mpfr_swap(result, expr->operator.right->number.value);
            break;
        }
        default:
            mpfr_clear(result);
            return ERROR_INVALID_OPERATOR;
    }

    expression_clean(expr->operator.left);
    expression_clean(expr->operator.right);
    free(expr->operator.left);
    free(expr->operator.right);

    expression_init_number(out, result);
    mpfr_clear(result);

    return ERROR_NO_ERROR;
}

error_t _expression_substitute_variable(expression_t* expr, scope_t* scope) {
    expression_t temp_expr;
    error_t err = scope_value_of(scope, expr->variable.value, &temp_expr);
    if (!err) {
        expression_clean(expr);
        memmove(expr, &temp_expr, sizeof(expression_t));
        err = expression_simplify(expr, scope);
        if (err) return err;
    }
    return err;
}

error_t _expression_simplify_recursive(expression_t* expr, scope_t* scope) {
    switch (expr->type) {
        case EXPRESSION_TYPE_VARIABLE:
            _expression_substitute_variable(expr, scope);
            break;
        case EXPRESSION_TYPE_NUMBER:
            break;
        case EXPRESSION_TYPE_PREFIX:
        {
            error_t err = _expression_simplify_recursive(expr->prefix.right, scope);
            if (err) return err;

            if (EXPRESSION_IS_NUMBER(EXPRESSION_RIGHT(expr))) {
                switch (EXPRESSION_OPERATOR(expr)) {
                    case '+':
                        break;
                    case '-':
                        mpfr_neg(expr->prefix.right->number.value, expr->prefix.right->number.value, MPFR_RNDF);
                        break;
                    default:
                        return ERROR_INVALID_PREFIX;
                }
                mpfr_swap(expr->number.value, expr->prefix.right->number.value);

                expr->type = EXPRESSION_TYPE_NUMBER;
            }
            break;
        }
        case EXPRESSION_TYPE_OPERATOR:
        {
            error_t err = _expression_simplify_recursive(expr->operator.left, scope);
            if (err) return err;

            err = _expression_simplify_recursive(expr->operator.right, scope);
            if (err) return err;

            if (expr->operator.right->type == EXPRESSION_TYPE_NUMBER
                && expr->operator.left->type == EXPRESSION_TYPE_NUMBER) {
                err = _expression_apply_operator(expr, expr, scope);
            } else if (expression_is_comparison(expr)) {

                // if we failed to evaluate a conditional we can't be sure about the expression's boolean result.
                scope->boolean = -1;
            }
            return err;
        }
    }
    return ERROR_NO_ERROR;
}

int _expression_has_variable_recursive(expression_t* expr, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return 0;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (_expression_has_variable_recursive(expr->operator.left, var))
                return 1;
            if (_expression_has_variable_recursive(expr->operator.right, var))
                return 1;
            return 0;
        }
        case EXPRESSION_TYPE_PREFIX:
            if (_expression_has_variable_recursive(expr->operator.right, var))
                return 1;
            return 0;
        case EXPRESSION_TYPE_VARIABLE:
            return strcmp(var, expr->variable.value) == 0;
    }
    return 0;
}

error_t _expression_isolate_variable_recursive(expression_t* expr, expression_t** target, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return ERROR_VARIABLE_NOT_PRESENT;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expression_is_comparison(expr)) {
                target = &expr->operator.left;
                error_t err = _expression_isolate_variable_recursive(expr->operator.left, &expr->operator.right, var);
                if (err && err != ERROR_VARIABLE_NOT_PRESENT)
                    return err;

                target = &expr->operator.right;
                if (err) {
                    err = _expression_isolate_variable_recursive(expr->operator.right, &expr->operator.left, var);
                    if (err) return err;
                }
                return ERROR_NO_ERROR;
            }

            expression_t* new_target = malloc(sizeof(expression_t));
            new_target->type = EXPRESSION_TYPE_OPERATOR;

            new_target->operator.left  = *target;
            if (_expression_has_variable_recursive(expr->operator.left, var)) {
                new_target->operator.right = expr->operator.right;
            } else {
                new_target->operator.right = expr->operator.left;
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
            variable_t nv;
            if (!_expression_isolate_variable_recursive(expr->operator.left, target, var)) {
                nv = expr->operator.left->variable.value;
                free(expr->operator.left);
            } else if (!_expression_isolate_variable_recursive(expr->operator.right, target, var)) {
                nv = expr->operator.right->variable.value;
                free(expr->operator.right);
            } else {
                return ERROR_VARIABLE_NOT_PRESENT;
            }

            expr->type = EXPRESSION_TYPE_VARIABLE;
            expr->variable.value = nv;
            return ERROR_NO_ERROR;
        }
        case EXPRESSION_TYPE_PREFIX:
        {
            printf("TODO(IanS5): isolate with prefix");
            exit(1);
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

error_t expression_simplify(expression_t* expr, scope_t* scope) {
    return  _expression_simplify_recursive(expr, scope);
}

error_t expression_isolate_variable(expression_t* expr, variable_t var) {
    if (!expression_is_comparison(expr)) {
        if (!_expression_has_variable_recursive(expr, var))
            return ERROR_VARIABLE_NOT_PRESENT;

        expr->type = EXPRESSION_TYPE_OPERATOR;
        expr->operator.infix = '=';
        expr->operator.left = expr;
        expression_init_number_si(expr->operator.right, 0);
    } else {
        if (!_expression_has_variable_recursive(expr->operator.right, var)
            && !_expression_has_variable_recursive(expr->operator.left, var))
            return ERROR_VARIABLE_NOT_PRESENT;
    }

    error_t err = _expression_isolate_variable_recursive(expr, NULL, var);
    if (err) return err;

    if (expr->operator.right->type == EXPRESSION_TYPE_VARIABLE) {
        expression_t* right = expr->operator.right;
        expr->operator.right = expr->operator.left;
        expr->operator.left = right;
    } else {
        if (expr->operator.infix == '<') {
            expr->operator.infix = '>';
        } else if (expr->operator.infix == '>') {
            expr->operator.infix = '<';
        }
    }

    return err;
}
