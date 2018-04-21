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
    error_t err = scope_get_value(scope, expr->variable.value, &temp_expr);
    if (!err) {
        expression_clean(expr);
        memmove(expr, &temp_expr, sizeof(expression_t));
        err = expression_simplify(expr, scope);
        if (err) return err;
    }
    return err;
}

variable_t _expression_find_var_recursive(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
        case EXPRESSION_TYPE_NUMBER:
            return NULL;
        case EXPRESSION_TYPE_OPERATOR:
        {
            variable_t var = _expression_find_var_recursive(expr->operator.left);
            if (var) return var;
            var = _expression_find_var_recursive(expr->operator.right);
            return var;
        }
        case EXPRESSION_TYPE_PREFIX:
            return _expression_find_var_recursive(expr->operator.right);
        case EXPRESSION_TYPE_VARIABLE:
            return expr->variable.value;
    }
    return NULL;
}

error_t _expression_run_function(expression_t* expr, scope_t* scope) {
    scope_t fn_scope;
    scope_init(&fn_scope);
    fn_scope.parent = scope;

    expression_t body;
    expression_list_t args_def;
    error_t err = scope_get_function(scope, expr->function.name, &body, &args_def);
    if (err) return err;

    expression_t* arg_def;
    expression_list_t* args_top = expr->function.parameters;

    EXPRESSION_LIST_FOREACH(arg_def, &args_def) {
        if (!args_top)
            return ERROR_MISSING_ARGUMENTS;
        expression_t* op_expr = malloc(sizeof(expression_t));
        expression_init_operator(op_expr, arg_def, ':', args_top->value);
        error_t err = expression_simplify(op_expr, &fn_scope);
        if (err) goto cleanup;
        args_top = args_top->next;
    }

    err = expression_simplify(&body, &fn_scope);
    if (err) return err;

    memmove(expr, &body, sizeof(expression_t));

cleanup:
    scope_clean(&fn_scope);
    return  ERROR_NO_ERROR;
}

error_t _expression_simplify_recursive(expression_t* expr, scope_t* scope) {
    switch (expr->type) {
        case EXPRESSION_TYPE_VARIABLE:
            return _expression_substitute_variable(expr, scope);
        case EXPRESSION_TYPE_FUNCTION:
            return _expression_run_function(expr, scope);
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
            if (expr->operator.infix == ':') {
                if (expr->operator.left->type == EXPRESSION_TYPE_FUNCTION) {
                    error_t err = scope_define_function(scope,
                                            expr->operator.left->function.name,
                                            expr->operator.right,
                                            expr->operator.left->function.parameters);
                    if (err) return err;

                    // copy the body into the parent expression
                    expression_copy(expr->operator.right, expr);
                    return ERROR_NO_ERROR;
                } else {
                    error_t err = _expression_simplify_recursive(expr->operator.right, scope);
                    if (err) return err;

                    if (expr->operator.left->type != EXPRESSION_TYPE_VARIABLE) {
                        variable_t varname = _expression_find_var_recursive(expr->operator.left);
                        if (varname) {
                            expression_isolate_variable(expr, varname);
                            err = _expression_simplify_recursive(expr->operator.right, scope);
                            if (err) return err;
                        }
                    }
                    if (expr->operator.left->type == EXPRESSION_TYPE_VARIABLE
                        && expr->operator.right->type == EXPRESSION_TYPE_NUMBER) {
                        scope_define(scope, expr->operator.left->variable.value, expr->operator.right);
                        mpfr_t result;
                        mpfr_init(result);
                        mpfr_set(result, expr->operator.right->number.value, MPFR_RNDF);
                        expression_free(expr->operator.left);
                        expression_init_number(expr, result);
                        mpfr_clear(result);
                    }
                }
            } else {
                error_t err = _expression_simplify_recursive(expr->operator.right, scope);
                if (err) return err;
                err = _expression_simplify_recursive(expr->operator.left, scope);
                if (expr->operator.right->type == EXPRESSION_TYPE_NUMBER
                    && expr->operator.left->type == EXPRESSION_TYPE_NUMBER) {
                    if (err) return err;

                    return _expression_apply_operator(expr, expr, scope);
                } else if (expression_is_comparison(expr)) {
                    // if we failed to evaluate a conditional we can't be sure about the expression's boolean result.
                    scope->boolean = -1;
                }
            }
            return ERROR_NO_ERROR;
        }
    }
    return ERROR_NO_ERROR;
}

int _expression_has_variable_recursive(expression_t* expr, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
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
        case EXPRESSION_TYPE_FUNCTION:
        case EXPRESSION_TYPE_NUMBER:
            return ERROR_VARIABLE_NOT_PRESENT;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expression_is_comparison(expr) || expr->operator.infix == ':') {
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
    if (!expression_is_comparison(expr) && expr->operator.infix != ':') {
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
