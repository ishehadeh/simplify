/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/simplify.h"
#include "simplify/expression/isolate.h"
#include "simplify/math/algebra.h"

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

error_t _expression_do_quadratic(char* var, expression_t* out, expression_t* x, expression_t* y, expression_t* z) {
    assert(EXPRESSION_IS_OPERATOR(x));
    assert(EXPRESSION_IS_OPERATOR(y));
    if (z) assert(EXPRESSION_IS_NUMBER(z));

    mpc_ptr a = NULL;
    mpc_ptr b = NULL;
    mpc_ptr c = z ? z->number.value : NULL;
    if (!c) {
        c = malloc(sizeof(mpc_t));
        mpc_init2(c, 256);
        mpc_set_si(c, 0, MPC_RNDNN);
    }

    if (x->operator.infix == '*') {
        if (EXPRESSION_IS_NUMBER(EXPRESSION_LEFT(x))) {
            a = EXPRESSION_LEFT(x)->number.value;
            free(EXPRESSION_LEFT(x));
        } else if (EXPRESSION_IS_NUMBER(EXPRESSION_RIGHT(x))) {
            a = EXPRESSION_RIGHT(x)->number.value;
            free(EXPRESSION_RIGHT(x));
        }
    }

    if (EXPRESSION_IS_NUMBER(EXPRESSION_LEFT(y))) {
        b = EXPRESSION_LEFT(y)->number.value;
        free(EXPRESSION_LEFT(y));
    } else if (EXPRESSION_IS_NUMBER(EXPRESSION_RIGHT(y))) {
        b = EXPRESSION_RIGHT(y)->number.value;
        free(EXPRESSION_RIGHT(y));
    } else {
        return ERROR_NO_ERROR;
    }

    /* use `b` and `c` as output variables, to avoid extra allocations */
    perform_quadratic_equation(c, b, a, b, c, MPC_RNDNN);

    operator_t b_op = '-';
    operator_t c_op = '-';
    if (mpfr_sgn(mpc_realref(b)) < 0) {
        b_op = '+';
        mpc_neg(b, b, MPC_RNDNN);
    }
    if (mpfr_sgn(mpc_realref(c)) < 0) {
        c_op = '+';
        mpc_neg(c, c, MPC_RNDNN);
    }

    if (mpfr_zero_p(mpc_realref(c)) && mpfr_zero_p(mpc_imagref(c))) {
        expression_init_operator(out, expression_new_variable(var), b_op, expression_new_number(b));
        mpc_clear(c);
    } else if (mpfr_zero_p(mpc_realref(b)) && mpfr_zero_p(mpc_imagref(b))) {
        expression_init_operator(out, expression_new_variable(var), c_op, expression_new_number(c));
        mpc_clear(b);
    } else {
        expression_init_operator(out,
            expression_new_operator(expression_new_variable(var), b_op, expression_new_number(b)),
            '*',
            expression_new_operator(expression_new_variable(var), c_op, expression_new_number(c)));
    }

    if (a) mpc_clear(a);
    free(x);
    free(y);
    free(z);

    return ERROR_NO_ERROR;
}

bool _expression_check_for_polynomial(expression_t* expr) {
    assert(EXPRESSION_IS_OPERATOR(expr));

    if ((expr->operator.infix == '+' || expr->operator.infix == '-') && EXPRESSION_IS_OPERATOR(expr->operator.left)) {
        expression_t* x = NULL;
        expression_t* y = NULL;
        expression_t* z = NULL;

        char* varl;
        char* varr;
        if (expr->operator.left->operator.infix == '+' || expr->operator.left->operator.infix == '-') {
            varl = expression_find_variable(EXPRESSION_LEFT(expr->operator.left));
            varr = expression_find_variable(EXPRESSION_RIGHT(expr->operator.left));
            if (varl && varr && !strcmp(varl, varr)) {
                x = EXPRESSION_LEFT(expr->operator.left);
                y = EXPRESSION_RIGHT(expr->operator.left);
                z = expr->operator.right;
            } else {
                return false;
            }
        } else if (expr->operator.left->operator.infix == '^') {
            varl = expression_find_variable(EXPRESSION_LEFT(expr));
            varr = expression_find_variable(EXPRESSION_RIGHT(expr));
            if (varl && varr && !strcmp(varl, varr)) {
                x = EXPRESSION_LEFT(expr);
                y = EXPRESSION_RIGHT(expr);
            } else {
                return false;
            }
        } else {
            return false;
        }

        if ((z && expr->operator.infix == '-') || (!z && EXPRESSION_LEFT(expr)->operator.infix == '-')) {
            if (EXPRESSION_IS_NUMBER(EXPRESSION_LEFT(y)))
                mpc_neg(EXPRESSION_LEFT(y)->number.value, EXPRESSION_LEFT(y)->number.value, MPC_RNDNN);
            else
                mpc_neg(EXPRESSION_RIGHT(y)->number.value, EXPRESSION_RIGHT(y)->number.value, MPC_RNDNN);
        }

        if (z && expr->operator.infix == '-')
            mpc_neg(z->number.value, z->number.value, MPC_RNDNN);

        if (EXPRESSION_IS_OPERATOR(x) && EXPRESSION_IS_OPERATOR(y) && y->operator.infix == '*'
            && (EXPRESSION_IS_VARIABLE(EXPRESSION_LEFT(y)) || EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(y))))
                _expression_do_quadratic(varr, expr, x, y, z);
        else
            return false;
        return true;
    }
    return false;
}

void _expression_distribute(expression_t* expr) {
    assert(EXPRESSION_IS_OPERATOR(expr));
    assert(expr->operator.infix == '*');

    expression_t* target;
    expression_t* mul;
    if (EXPRESSION_IS_OPERATOR(EXPRESSION_RIGHT(expr))
        && operator_precedence(expr->operator.infix) > operator_precedence(EXPRESSION_RIGHT(expr)->operator.infix)) {
            target = EXPRESSION_RIGHT(expr);
            mul = EXPRESSION_LEFT(expr);
    } else if (EXPRESSION_IS_OPERATOR(EXPRESSION_LEFT(expr))
        && operator_precedence(expr->operator.infix) > operator_precedence(EXPRESSION_LEFT(expr)->operator.infix)) {
            target = EXPRESSION_LEFT(expr);
            mul = EXPRESSION_RIGHT(expr);
    } else {
        return;
    }
    expression_t* left = NULL;
    expression_t* right = NULL;
    if (EXPRESSION_IS_NUMBER(mul)) {
        if (EXPRESSION_IS_NUMBER(EXPRESSION_LEFT(target))) {
            left = expression_new_number(EXPRESSION_LEFT(target)->number.value);
            mpc_mul(left->number.value, left->number.value, mul->number.value, MPC_RNDNN);
            free(EXPRESSION_LEFT(target));
        }

        if (EXPRESSION_IS_NUMBER(EXPRESSION_RIGHT(target))) {
            right = expression_new_number(EXPRESSION_RIGHT(target)->number.value);
            mpc_mul(right->number.value, right->number.value, mul->number.value, MPC_RNDNN);
            free(EXPRESSION_RIGHT(target));
        }
    }

    if (!right && !left) {
        expression_t* mul_cpy = malloc(sizeof(expression_t));
        expression_copy(mul, mul_cpy);
        right = expression_new_operator(mul, '*', EXPRESSION_RIGHT(target));
        left = expression_new_operator(mul_cpy, '*', EXPRESSION_LEFT(target));
    } else if (!right) {
        right = expression_new_operator(mul, '*', EXPRESSION_RIGHT(target));
    } else if (!left) {
        left = expression_new_operator(mul, '*', EXPRESSION_LEFT(target));
    }

    expression_init_operator(expr, left, target->operator.infix, right);
    if (EXPRESSION_IS_OPERATOR(left) && left->operator.infix == '*')
        _expression_distribute(left);

    if (EXPRESSION_IS_OPERATOR(right) && right->operator.infix == '*')
        _expression_distribute(right);
    free(target);
}

void _expression_simplify_polynomials_recursive(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
        case EXPRESSION_TYPE_VARIABLE:
        case EXPRESSION_TYPE_FUNCTION:
        case EXPRESSION_TYPE_PREFIX:
            break;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (_expression_check_for_polynomial(expr)) return;
            _expression_simplify_polynomials_recursive(EXPRESSION_LEFT(expr));
            _expression_simplify_polynomials_recursive(EXPRESSION_RIGHT(expr));
            if (expr->operator.infix == '*') _expression_distribute(expr);
        }
    }
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
                && EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(expr))
                && !strcmp(EXPRESSION_RIGHT(expr)->variable.value, EXPRESSION_LEFT(expr)->variable.value)) {
                    expr->operator.infix = equiv_op;
                    expression_clean(expr->operator.right);
                    expression_init_number_si(EXPRESSION_RIGHT(expr), 2);

                    // the `while` loop condition will fail immediately, so just break here
                    break;
            }

            /* If possible try to add to any expressions up the tree
             To add to an expression it must look like (<variable> equiv_op <number>) */
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
                    mpc_ptr x = count->number.value;
                    mpc_add_si(x, x, 1, MPC_RNDNN);
                    *expr = *EXPRESSION_LEFT(expr);
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
        expression_new_operator(b, '^', expression_new_variable(MANGLE_INTERNAL_VARIABLE("LogarithmResult"))),
        '=',
        y);

    err = expression_isolate_variable(*out, MANGLE_INTERNAL_VARIABLE("LogarithmResult"));
    if (err) return err;

    expression_collapse_left(*out);

    return err;
}

error_t expression_simplify(expression_t* expr) {
    _expression_simplify_polynomials_recursive(expr);
    return _expression_collapse_variables_recursive(expr);
}
