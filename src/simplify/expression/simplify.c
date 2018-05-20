/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/simplify.h"
#include "simplify/expression/isolate.h"

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

static inline int _diff_precedence(expression_t* expr1, expression_t* expr2) {
    return (int)operator_precedence(expr1->operator.infix) - (int)operator_precedence(expr2->operator.infix);
}

int _expression_init_chain(expression_t* root, expression_t* var) {
    expression_t* right = EXPRESSION_RIGHT(var);

    if (EXPRESSION_IS_VARIABLE(right)
        && _diff_precedence(root, var) >= 0
        && !strcmp(right->variable.value, EXPRESSION_RIGHT(root)->variable.value)) {
            char* varname = right->variable.value;
            expression_init_operator(right,
                expression_new_variable(varname),
                _operator_collapsed_equivelent(root->operator.infix),
                expression_new_number_si(2));
            return 0;
    }

    return 1;
}

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

            operator_t equiv_op = _operator_collapsed_equivelent(expr->operator.infix);

            // Don't know how to deal with this operator yet
            if (!equiv_op)
                break;

            // If there are two variables right next to each other try to combine them
            if (EXPRESSION_IS_VARIABLE(EXPRESSION_LEFT(expr))
                && EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(expr))) {
                    expr->operator.infix = equiv_op;
                    expression_clean(expr->operator.right);
                    expression_init_number_si(EXPRESSION_RIGHT(expr), 2);

                    // the `while` loop condition will fail immediately, so just break here
                    break;
            }

            /* If possible try to add to any expressions up the tree
             To add to an expression it must look like (<variable> eqiv_op <number>) */
            while (EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(expr)) && EXPRESSION_IS_OPERATOR(EXPRESSION_LEFT(expr))) {
                /* While possible worm our way up the tree, looking for the expression */
                expression_t* right = EXPRESSION_LEFT(expr);
                while (EXPRESSION_IS_OPERATOR(right)
                        && EXPRESSION_IS_OPERATOR(EXPRESSION_RIGHT(right))
                        && _diff_precedence(right, expr) >= 0)
                            right = EXPRESSION_RIGHT(right);

                expression_t* variable = EXPRESSION_LEFT(right);
                expression_t* count = EXPRESSION_RIGHT(right);

                /* If the variable / count is an operator expression, if it includes the right
                    variable we may be able to start a new chain */
                if (!EXPRESSION_IS_NUMBER(count) ||!EXPRESSION_IS_VARIABLE(variable)) {
                    if (!EXPRESSION_IS_VARIABLE(variable)) {
                        if (!_expression_init_chain(expr, right)) {
                            *expr = *EXPRESSION_LEFT(expr);
                            continue;
                        }
                    }

                    if (!EXPRESSION_IS_NUMBER(count)) {
                        if (!_expression_init_chain(expr, right)) {
                            *expr = *EXPRESSION_LEFT(expr);
                            continue;
                        }
                    }
                    break;
                }

                if (strcmp(variable->variable.value, EXPRESSION_RIGHT(expr)->variable.value) != 0) break;

                if (right->operator.infix == equiv_op) {
                    mpfr_ptr x = count->number.value;
                    mpfr_add_si(x, x, 1, MPFR_RNDN);
                    *expr = *EXPRESSION_LEFT(expr);
                } else {
                    expr->operator.infix = equiv_op;
                    expression_clean(expr->operator.right);
                    expression_init_number_si(EXPRESSION_RIGHT(expr), 2);
                }
            }
        }
    }
    return ERROR_NO_ERROR;
}

error_t expression_do_logarithm(expression_t* b, expression_t* y, expression_t** out) {
    /* expression_isolate_variable can solve logarithms,
        so this function will set that up expand to b^x = y, then call
        expression_isolate_variable to solve it
    */
    error_t err;
    *out = expression_new_operator(
        expression_new_operator(b, '^', expression_new_variable("x") ),
        '=',
        y);

    err = expression_isolate_variable(*out, "x");
    if (err) return err;

    expression_collapse_left(*out);

    return err;
}

error_t expression_simplify(expression_t* expr) {
    return _expression_collapse_variables_recursive(expr);
}
