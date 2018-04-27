/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/simplify.h"

error_t _expression_collapse_variables(expression_t* expr) {
    if (EXPRESSION_IS_OPERATOR(expr)) {
        if (EXPRESSION_IS_VARIABLE(expr->operator.right)) {
            expression_t* left = EXPRESSION_LEFT(expr);
            operator_t infix_op = expr->operator.infix;
            int count = 1;
            while (infix_op == left->operator.infix
                    && EXPRESSION_IS_OPERATOR(left)
                    && EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(left))) {
                expression_t* l2 = EXPRESSION_LEFT(left);
                expression_free(EXPRESSION_RIGHT(left));
                free(left);
                left = l2;
                ++count;
            }

            ++count;
            expression_t* num;
            operator_t*   op;
            if (EXPRESSION_IS_VARIABLE(left)) {
                expression_t right = *EXPRESSION_RIGHT(expr);
                *expr->operator.left = right;
                num = EXPRESSION_RIGHT(expr);
                op = &expr->operator.infix;
            } else if (EXPRESSION_IS_OPERATOR(left)) {
                expr->operator.left = EXPRESSION_LEFT(left);
                expression_t* var = malloc(sizeof(expression_t));
                expression_init_variable(var, expr->operator.right->variable.value,
                                              strlen(expr->operator.right->variable.value));
                expression_init_operator(expr->operator.right, var,
                                         left->operator.infix,
                                         (expression_t*)malloc(sizeof(expression_t)));
                num = EXPRESSION_RIGHT(EXPRESSION_RIGHT(expr));
                op = &expr->operator.infix;
            } else {
                --count;
                expression_t* var = malloc(sizeof(expression_t));
                expression_copy(EXPRESSION_RIGHT(expr), var);
                expression_init_operator(expr,
                                         malloc(sizeof(expression_t)),
                                         infix_op,
                                         left);
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
                    _expression_simplify_recursive(left);
            }
        }
    }
    return ERROR_NO_ERROR;
}

error_t expression_simplify(expression_t* expr) {
    return _expression_simplify_recursive(expr);
}
