/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/isolate.h"
#include "simplify/expression/evaluate.h"

/* evaluate an expression, try to put it in it's simplest terms. Only apply operator and expand variables and function.
 *
 * @expr the expression to evaluate
 * @scope the expression's scope
 * @return returns an error code
 */
error_t _expression_evaluate_recursive(expression_t* expr, scope_t* scope);

/* apply a comparison operator expression, `expr`, modify scope if the result is noteworthy.
 *
 * @expr the expression to apply
 * @scope the expression's scope
 * @return returns an error code
 */
error_t _expression_apply_comparison(expression_t* expr, scope_t* scope) {
    assert(EXPRESSION_IS_OPERATOR(expr));

    if (scope->boolean != 0) {
        int x = mpfr_cmp(expr->operator.left->number.value, expr->operator.right->number.value);
        scope->boolean = ((expr->operator.infix == '<' && x < 0)
                            || (expr->operator.infix == '>' && x > 0)
                            || (expr->operator.infix == '=' && x == 0));
    }

    expression_collapse_left(expr);
    return ERROR_NO_ERROR;
}

/* apply a assignment operator expression, assume the left side of the expression contains a variable or function.
 *
 * @expr the expression to apply
 * @scope the expression's scope
 * @return returns an error code
 */
error_t _expression_apply_assignment(expression_t* expr, scope_t* scope) {
    assert(EXPRESSION_IS_OPERATOR(expr));

    error_t err;
    if (EXPRESSION_IS_FUNCTION(expr->operator.left)) {
        expression_t* body = malloc(sizeof(expression_t));
        expression_list_t* params = malloc(sizeof(expression_list_t));

        expression_list_init(params);
        expression_list_copy(expr->operator.left->function.parameters, params);
        expression_copy(expr->operator.right, body);

        err = scope_define_function(scope, expr->operator.left->function.name, body, params);
        if (err) return err;

        expression_collapse_left(expr);
    } else {
        err = _expression_evaluate_recursive(expr->operator.right, scope);
        if (err) return err;

        /* if there's not a variable on the left try to simplify the expression to get one */
        if (!EXPRESSION_IS_VARIABLE(expr->operator.left)) {
            err = _expression_evaluate_recursive(expr->operator.left, scope);
            if (err) return err;

            variable_t varname = expression_find_variable(expr->operator.left);
            if (varname) {
                expression_isolate_variable(expr, varname);
                err = _expression_evaluate_recursive(expr->operator.right, scope);
                if (err) return err;
            }
        }

        if (EXPRESSION_IS_VARIABLE(expr->operator.left)) {
            expression_t* value_copy = malloc(sizeof(expression_t));
            expression_copy(expr->operator.right, value_copy);
            scope_define(scope, expr->operator.left->variable.value, value_copy);

            expression_collapse_left(expr);
        }
    }

    return err;
}

/* apply a prefix expression's prefix to it's right arm
 *
 * @expr the prefix expression to apply
 * @scope the expression's scope
 * @return returns an error code
 */
error_t _expression_apply_prefix(expression_t* expr, scope_t* scope) {
    assert(EXPRESSION_IS_PREFIX(expr));
    error_t err;

    err = _expression_evaluate_recursive(expr->prefix.right, scope);
    if (err) return err;

    if (EXPRESSION_IS_NUMBER(EXPRESSION_RIGHT(expr))) {
        switch (expr->prefix.prefix) {
            case '+':
                break;
            case '-':
                mpfr_neg(expr->prefix.right->number.value, expr->prefix.right->number.value, MPFR_RNDF);
                break;
            default:
                return ERROR_INVALID_PREFIX;
        }
        expression_collapse_left(expr);
    }

    return ERROR_NO_ERROR;
}

/* apply an operator expression.
 *
 * @expr input operator expression
 * @scope the expression's scope
 * @return returns an  error code
 */
error_t _expression_apply_operator(expression_t* expr, scope_t* scope) {
    assert(EXPRESSION_IS_OPERATOR(expr));

    static const int round_mode = MPFR_RNDF;

    mpfr_t result;

    mpfr_ptr left  = expr->operator.left->number.value;
    mpfr_ptr right = expr->operator.right->number.value;

    switch (expr->operator.infix) {
        case '+':
            mpfr_init(result);
            mpfr_add(result, left, right, round_mode);
            break;
        case '-':
            mpfr_init(result);
            mpfr_sub(result, left, right, round_mode);
            break;
        case '/':
            mpfr_init(result);
            mpfr_div(result, left, right, round_mode);
            break;
        case '*':
        case '(':
            mpfr_init(result);
            mpfr_mul(result, left, right, round_mode);
            break;
        case '^':
            mpfr_init(result);
            mpfr_pow(result, left, right, round_mode);
            break;
        case '\\':
            mpfr_init(result);
            mpfr_rootn_ui(result, left, mpfr_get_ui(right, MPFR_RNDN), round_mode);
            break;
        case '=':
        case '>':
        case '<':
            /* defer to _expression_apply_comparison if a comparison is found
                this function cleans up itself, so we can return immediately */
            return _expression_apply_comparison(expr, scope);
        default:
            return ERROR_INVALID_OPERATOR;
    }

    expression_free(expr->operator.left);
    expression_free(expr->operator.right);

    expression_init_number(expr, result);

    return ERROR_NO_ERROR;
}


/* subsitute a variable expression with the variable's value, if available.
 *
 * This function will __not__ simplify the variable's result. 
 * 
 * @expr the variable expression to substitute
 * @scope the scope to search for the variable
 * @return returns an error code
 */ 
error_t _expression_substitute_variable(expression_t* expr, scope_t* scope) {
    assert(EXPRESSION_IS_VARIABLE(expr));

    expression_t variable_value;
    error_t err;

    if (expr->variable.binding)
        err = scope_get_value(expr->variable.binding, expr->variable.value, &variable_value);
    else
        err = scope_get_value(scope, expr->variable.value, &variable_value);

    if (!err) {
        expression_clean(expr);
        *expr = variable_value;
    } else if (!expr->variable.binding) {
        /* couldn't find the variable. let future executor know that this is the
            scope where the variable's value should be found */
        expr->variable.binding = scope;
    }
    return ERROR_NO_ERROR;
}

error_t _expression_evaluate_recursive(expression_t* expr, scope_t* scope) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            /* numbers can't be evaluated */
            break;
        case EXPRESSION_TYPE_VARIABLE:
            return _expression_substitute_variable(expr, scope);
        case EXPRESSION_TYPE_FUNCTION:
            return scope_call(scope, expr->function.name, expr->function.parameters, expr);
        case EXPRESSION_TYPE_PREFIX:
            return _expression_apply_prefix(expr, scope);
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expr->operator.infix == ':') {
                return _expression_apply_assignment(expr, scope);
            } else {
                error_t err = _expression_evaluate_recursive(expr->operator.right, scope);
                if (err) return err;

                err = _expression_evaluate_recursive(expr->operator.left, scope);
                if (expr->operator.right->type == expr->operator.left->type &&
                        expr->operator.left->type == EXPRESSION_TYPE_NUMBER) {
                    return _expression_apply_operator(expr, scope);
                } else if (expression_is_comparison(expr)) {
                    /* if we failed to evaluate a conditional we can't be sure about the expression's boolean result. */
                    scope->boolean = -1;
                }
                return err;
            }
        }
    }
    return ERROR_NO_ERROR;
}

error_t expression_evaluate(expression_t* expr, scope_t* scope) {
    return  _expression_evaluate_recursive(expr, scope);
}
