/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/simplify.h"

expression_t* _expression_follow_variable_chain(expression_t* root, operator_t infix_op, int* count) {
    expression_t* left = root;
    while (infix_op == left->operator.infix
            && EXPRESSION_IS_OPERATOR(left)
            && EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(left))) {
        left = EXPRESSION_LEFT(left);
        ++(*count);
    }
    return left;
}

error_t _expression_collapse_variables_recursive(expression_t* expr) {
    if (EXPRESSION_IS_OPERATOR(expr)) {
        operator_t infix_op = expr->operator.infix;
        expression_t* final = NULL;
        int count = 2;
        if (EXPRESSION_IS_VARIABLE(expr->operator.right)) {
            int count = 1;
            final = _expression_follow_variable_chain(EXPRESSION_LEFT(expr),
                                                      expr->operator.infix,
                                                      &count);
        } else if (EXPRESSION_IS_OPERATOR(expr->operator.right)) {
            return _expression_collapse_variables_recursive(expr->operator.right);
        } else {
            return ERROR_NO_ERROR;
        }

        expression_t* num;
        operator_t*   op;
        if (EXPRESSION_IS_VARIABLE(final)) {
            ++count;
            expression_t right = *EXPRESSION_RIGHT(expr);
            *expr->operator.left = right;
            num = EXPRESSION_RIGHT(expr);
            op = &expr->operator.infix;
        } else if (EXPRESSION_IS_OPERATOR(final)) {
            ++count;
            expr->operator.left = EXPRESSION_LEFT(final);
            expression_t* var = malloc(sizeof(expression_t));
            expression_init_variable(var, expr->operator.right->variable.value,
                                            strlen(expr->operator.right->variable.value));
            expression_init_operator(expr->operator.right, var,
                                        final->operator.infix,
                                        (expression_t*)malloc(sizeof(expression_t)));
            num = EXPRESSION_RIGHT(EXPRESSION_RIGHT(expr));
            op = &expr->operator.infix;
        } else {
            expression_t* var = malloc(sizeof(expression_t));
            expression_copy(EXPRESSION_RIGHT(expr), var);
            expression_init_operator(expr,
                                        malloc(sizeof(expression_t)),
                                        infix_op,
                                        final);
            expression_init_operator(EXPRESSION_LEFT(expr),
                                        var,
                                        0,
                                        malloc(sizeof(expression_t)));
            op =  &EXPRESSION_LEFT(expr)->operator.infix;
            num = EXPRESSION_RIGHT(EXPRESSION_LEFT(expr));
        }

        switch (infix_op) {
            case '+':
                expression_init_number_si(num, count);
                *op = '*';
                break;
            case '-':
                expression_init_number_si(num, -(count - 2));
                *op = '*';
                break;
            case '*':
                expression_init_number_si(num, count);
                *op = '^';
                break;
            default:
                break;
        }
    }
    return ERROR_NO_ERROR;
}

error_t expression_simplify(expression_t* expr) {
    return _expression_collapse_variables_recursive(expr);
}
