/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/simplify.h"

/* get the operator that would could be used to collapse a chain of `op` operators
 *
 * @op the source op
 * @return returns the collapse operator, or zero if `op` is not collapseable
 */
operator_t _operator_collapsed_equivelent(operator_t op) {
    switch (op) {
        case '+':
            return '*';
        case '*':
            return '^';
    }

    return 0;
}

// TODO(IanS5): solve when the far left operator has a higher precedence (e.g. 3 * x * x * x * 10)
error_t _expression_collapse_variables_recursive(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
        case EXPRESSION_TYPE_VARIABLE:
        case EXPRESSION_TYPE_FUNCTION:
        case EXPRESSION_TYPE_PREFIX:
            break;
        case EXPRESSION_TYPE_OPERATOR:
        {
            error_t err = _expression_collapse_variables_recursive(EXPRESSION_LEFT(expr));
            if (err) return err;

            err = _expression_collapse_variables_recursive(EXPRESSION_RIGHT(expr));
            if (err) return err;

            if (EXPRESSION_IS_VARIABLE(EXPRESSION_LEFT(expr))
                && EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(expr))
                && !strcmp(EXPRESSION_LEFT(expr)->variable.value,
                          EXPRESSION_RIGHT(expr)->variable.value)
                && _operator_collapsed_equivelent(expr->operator.infix)) {
                expr->operator.infix = _operator_collapsed_equivelent(expr->operator.infix);
                expression_clean(expr->operator.right);
                expression_init_number_si(EXPRESSION_RIGHT(expr), 2);
            }

            if (EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(expr))
                && EXPRESSION_IS_OPERATOR(EXPRESSION_LEFT(expr))
                && EXPRESSION_IS_VARIABLE(EXPRESSION_LEFT(EXPRESSION_LEFT(expr)))
                && EXPRESSION_IS_NUMBER(EXPRESSION_RIGHT(EXPRESSION_LEFT(expr)))
                && !strcmp(EXPRESSION_LEFT(EXPRESSION_LEFT(expr))->variable.value,
                           EXPRESSION_RIGHT(expr)->variable.value)
                && EXPRESSION_LEFT(expr)->operator.infix == _operator_collapsed_equivelent(expr->operator.infix)) {
                mpfr_ptr x = EXPRESSION_RIGHT(EXPRESSION_LEFT(expr))->number.value;
                mpfr_add_si(x, x, 1, MPFR_RNDN);
                *expr = *EXPRESSION_LEFT(expr);
            }
        }
    }
    return ERROR_NO_ERROR;
}

error_t expression_simplify(expression_t* expr) {
    return _expression_collapse_variables_recursive(expr);
}
