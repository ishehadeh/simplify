// Copyright Ian R. Shehadeh 2018

#include "expression.h"


void expression_free(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_PREFIX:
            expression_free(expr->prefix.right);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            expression_free(expr->operator.left);
            expression_free(expr->operator.right);
            break;
        case EXPRESSION_TYPE_NUMBER:
            SCALAR_CLEAN(expr->number.value);
            break;
        default:
            break;
    }

    free(expr);
}

int variable_id(variable_t var) {
    if (var <= 'Z' && var >= 'A') {
        return var - 'A' + 25;
    } else if (var <= 'z' && var >= 'a') {
        return var - 'a';
    }
    return -1;
}

void expression_print(expression_t* expr) {
    if (!expr) {
        printf("(NULL)");
        return;
    }

    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
        {
            int len = SCALAR_REQUIRED_CHARS(expr->number.value);
            char* buf = NULL;

            if (len < 256) {
                buf = alloca(len + 1);
            } else {
                buf = malloc(len + 1);
            }

            buf[len] = 0;

            SCALAR_TO_STRING(expr->number.value, buf);
            printf("%s", buf);
            if (len >= 256) {
                free(buf);
            }
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            printf("%c", expr->variable.value);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            printf("( ");
            expression_print(expr->operator.left);
            printf(" %c ", expr->operator.infix);
            expression_print(expr->operator.right);
            printf(" )");
            break;
        case EXPRESSION_TYPE_PREFIX:
            printf("%c", expr->prefix.prefix);
            expression_print(expr->prefix.right);
            break;
        default:
            printf("(UNDEFINED TYPE %d)", expr->type);
            break;
    }
}


int expression_simplify_vars(expression_t* expr, expression_t** variables) {
    switch (expr->type) {
        case EXPRESSION_TYPE_VARIABLE:
        {
            if (variables[variable_id(expr->variable.value)] != NULL) {
                *expr = *variables[variable_id(expr->variable.value)];
            }
        }
        case EXPRESSION_TYPE_NUMBER:
            break;
        case EXPRESSION_TYPE_PREFIX:
        {
            if (expression_simplify_vars(expr->prefix.right, variables)) {
                return 1;
            }
            if (expr->prefix.right->type == EXPRESSION_TYPE_NUMBER) {
                SCALAR_DEFINE(result);

                switch (expr->prefix.prefix) {
                    case '+':
                        SCALAR_SET(expr->prefix.right->number.value, result);
                        break;
                    case '-':
                        SCALAR_NEGATE(expr->prefix.right->number.value, result);
                        break;
                    default:
                        printf("ERROR: invalid prefix '%c'", expr->prefix.prefix);
                        return 1;
                }
                expression_free(expr->prefix.right);

                expr->type = EXPRESSION_TYPE_NUMBER;
                SCALAR_SET(result, expr->number.value);
            }
            break;
        }
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expression_simplify_vars(expr->operator.right, variables)) {
                return 1;
            }
            if (expression_simplify_vars(expr->operator.left, variables)) {
                return 1;
            }
            if (expr->operator.right->type == EXPRESSION_TYPE_NUMBER
                && expr->operator.left->type == EXPRESSION_TYPE_NUMBER) {
                SCALAR_DEFINE(result);

                switch (expr->operator.infix) {
                    case '+':
                        SCALAR_ADD(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '-':
                        SCALAR_SUB(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '/':
                        SCALAR_DIV(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '*':
                        SCALAR_MUL(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '^':
                        SCALAR_POW(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '=':
                        return 0;
                    default:
                        printf("ERROR: invalid operator '%c'", expr->operator.infix);
                        return 1;
                }
                expression_free(expr->operator.left);
                expression_free(expr->operator.right);

                expr->type = EXPRESSION_TYPE_NUMBER;
                SCALAR_SET(result, expr->number.value);
            }
            break;
        }
    }
    return 0;
}

int expression_simplify(expression_t* expr) {
    expression_t* variables[26 * 2];
    memset(&variables[0], 0, 26 * 2 *sizeof(expression_t*));

    return expression_simplify_vars(expr, &variables[0]);
}

int expression_isolate_variable(expression_t* expr, variable_t var) {
    // TODO(IanS5)
    return 1;
}