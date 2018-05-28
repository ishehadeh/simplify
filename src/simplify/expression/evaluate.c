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
error_t _expression_apply_operator(expression_t* expr) {
    assert(EXPRESSION_IS_OPERATOR(expr));

    static const int round_mode = MPFR_RNDF;

    mpfr_ptr result;

    mpfr_ptr left  = expr->operator.left->number.value;
    mpfr_ptr right = expr->operator.right->number.value;

    switch (expr->operator.infix) {
        case '+':
            result = malloc(sizeof(mpfr_t));
            mpfr_init(result);
            mpfr_add(result, left, right, round_mode);
            break;
        case '-':
            result = malloc(sizeof(mpfr_t));
            mpfr_init(result);
            mpfr_sub(result, left, right, round_mode);
            break;
        case '/':
            result = malloc(sizeof(mpfr_t));
            mpfr_init(result);
            mpfr_div(result, left, right, round_mode);
            break;
        case '*':
        case '(':
            result = malloc(sizeof(mpfr_t));
            mpfr_init(result);
            mpfr_mul(result, left, right, round_mode);
            break;
        case '^':
            result = malloc(sizeof(mpfr_t));
            mpfr_init(result);
            mpfr_pow(result, left, right, round_mode);
            break;
        case '\\':
            result = malloc(sizeof(mpfr_t));
            mpfr_init(result);
            mpfr_rootn_ui(result, left, mpfr_get_ui(right, MPFR_RNDN), round_mode);
            break;
        case '=':
        case '>':
        case '<':
            return ERROR_NO_ERROR;
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
        _expression_evaluate_recursive(expr, scope);
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
        {
            expression_t old = *expr;
            error_t err = scope_call(scope, expr->function.name, expr->function.parameters, expr);
            if (err && err != ERROR_NONEXISTANT_KEY) return err;
            if (!err) expression_clean(&old);
            return ERROR_NO_ERROR;
        }
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
                if (!expression_is_comparison(expr) && expr->operator.right->type == expr->operator.left->type &&
                        expr->operator.left->type == EXPRESSION_TYPE_NUMBER) {
                    return _expression_apply_operator(expr);
                }
                return err;
            }
        }
    }
    return ERROR_NO_ERROR;
}

expression_result_t _expression_evaluate_comparisons_recursive(expression_t* expr) {
    expression_result_t result;

    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return EXPRESSION_RESULT_NONBINARY;
        case EXPRESSION_TYPE_VARIABLE:
            return EXPRESSION_RESULT_NONBINARY;
        case EXPRESSION_TYPE_FUNCTION:
            return EXPRESSION_RESULT_NONBINARY;
        case EXPRESSION_TYPE_PREFIX:
            return EXPRESSION_RESULT_NONBINARY;
        case EXPRESSION_TYPE_OPERATOR:
        {
            expression_result_t right = _expression_evaluate_comparisons_recursive(expr->operator.right);
            expression_result_t left = _expression_evaluate_comparisons_recursive(expr->operator.left);

            if (right == EXPRESSION_RESULT_FALSE || left == EXPRESSION_RESULT_FALSE) {
                result = EXPRESSION_RESULT_FALSE;
                break;
            }

            if (expression_is_comparison(expr)) {
                compare_result_t x = expression_compare(EXPRESSION_LEFT(expr), EXPRESSION_RIGHT(expr));
                if (x == COMPARE_RESULT_INCOMPARABLE)
                    return  EXPRESSION_RESULT_NONBINARY;
                else if (expr->operator.infix == '<' && x == COMPARE_RESULT_LESS)
                    result = EXPRESSION_RESULT_TRUE;
                else if (expr->operator.infix == '>' && x == COMPARE_RESULT_GREATER)
                    result = EXPRESSION_RESULT_TRUE;
                else if (expr->operator.infix == '=' && x == COMPARE_RESULT_EQUAL)
                    result = EXPRESSION_RESULT_TRUE;
                else
                    result = EXPRESSION_RESULT_FALSE;
                break;
            }

            if (right == EXPRESSION_RESULT_TRUE || left == EXPRESSION_RESULT_TRUE) {
                result = EXPRESSION_RESULT_TRUE;
            } else {
                return EXPRESSION_RESULT_NONBINARY;
            }
        }
    }

    expression_collapse_left(expr);
    return result;
}

error_t expression_evaluate(expression_t* expr, scope_t* scope) {
    return _expression_evaluate_recursive(expr, scope);
}

expression_result_t expression_evaluate_comparisons(expression_t* expr) {
    return _expression_evaluate_comparisons_recursive(expr);
}
