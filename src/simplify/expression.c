/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression.h"
#include "simplify/errors.h"

void expression_free(expression_t* expr) {
    expression_clean(expr);
    free(expr);
}

void expression_clean(expression_t* expr) {
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
        case EXPRESSION_TYPE_VARIABLE:
            free(expr->variable.value);
            break;
        default:
            break;
    }
}

expression_t* expression_copy(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_PREFIX:
            return new_prefix_expression(expr->prefix.prefix, expression_copy(expr->prefix.right));
        case EXPRESSION_TYPE_OPERATOR:
            return new_operator_expression(expression_copy(expr->operator.left),
                    expr->operator.infix, expression_copy(expr->operator.right));
        case EXPRESSION_TYPE_NUMBER:
            return new_number_expression(expr->number.value);
        case EXPRESSION_TYPE_VARIABLE:
            return new_variable_expression(expr->variable.value, strlen(expr->variable.value));
        default:
            return new_expression();
    }
}

error_t expression_print(expression_t* expr) {
    if (!expr)
        return ERROR_NULL_EXPRESSION;

    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
        {
            int len = SCALAR_REQUIRED_CHARS(expr->number.value);
            char* buf = malloc(len + 1);
            buf[len] = 0;

            SCALAR_TO_STRING(expr->number.value, buf);
            printf("%s", buf);
            free(buf);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            printf("%s", expr->variable.value);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            expression_print(expr->operator.left);
            printf(" %c ", expr->operator.infix);
            expression_print(expr->operator.right);
            break;
        case EXPRESSION_TYPE_PREFIX:
            printf("%c", expr->prefix.prefix);
            expression_print(expr->prefix.right);
            break;
        default:
            printf("(UNDEFINED TYPE %d)", expr->type);
            break;
    }

    return ERROR_NO_ERROR;
}


error_t expression_to_bool(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NULL:
        case EXPRESSION_TYPE_NUMBER:
            return ERROR_CANNOT_COMPARE;
        case EXPRESSION_TYPE_PREFIX:
            return ERROR_CANNOT_COMPARE;
        case EXPRESSION_TYPE_VARIABLE:
            return ERROR_CANNOT_COMPARE;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expr->operator.left->type != EXPRESSION_TYPE_NUMBER ||
                expr->operator.right->type != EXPRESSION_TYPE_NUMBER) {
                    return ERROR_CANNOT_COMPARE;
            }

            int len = SCALAR_REQUIRED_CHARS(expr->operator.left->number.value);
            int len1 = SCALAR_REQUIRED_CHARS(expr->operator.right->number.value);

            char* buf = malloc(len + 1);
            char* buf1 = malloc(len1 + 1);

            buf[len] = 0;

            SCALAR_TO_STRING(expr->operator.left->number.value, buf);
            SCALAR_TO_STRING(expr->operator.right->number.value, buf1);

            scalar_t l;
            scalar_t r;

            SCALAR_INIT(l);
            SCALAR_INIT(r);
            SCALAR_FROM_STRING(buf, l);
            SCALAR_FROM_STRING(buf1, r);

            int x = SCALAR_COMPARE(l, r);

            free(buf);
            free(buf1);
            SCALAR_CLEAN(r);
            SCALAR_CLEAN(l);
            expression_free(expr->operator.left);
            expression_free(expr->operator.right);

            expr->type = EXPRESSION_TYPE_NUMBER;
            switch (expr->operator.infix) {
                case '<':
                    SCALAR_INIT(expr->number.value);
                    SCALAR_SET_INT(x < 0, expr->number.value);
                    break;
                case '>':
                    SCALAR_INIT(expr->number.value);
                    SCALAR_SET_INT(x > 0, expr->number.value);
                    break;
                case '=':
                    SCALAR_INIT(expr->number.value);
                    SCALAR_SET_INT(x == 0, expr->number.value);
                    break;
                default:
                    return ERROR_INVALID_OPERATOR;
            }
        }
    }
    return ERROR_NO_ERROR;
}

error_t _expression_simplify_recursive(expression_t* expr, scope_t* scope) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NULL:
            return ERROR_NULL_EXPRESSION;
        case EXPRESSION_TYPE_VARIABLE:
        {
            expression_t* var_value;
            error_t err = rbtree_search(&scope->variables, expr->variable.value, (void**)&var_value);
            if (!err) {
                var_value = expression_copy(var_value);
                err = _expression_simplify_recursive(var_value, scope);
                if (err) return err;
                memmove(expr, var_value, sizeof(expression_t));
            }
            break;
        }
        case EXPRESSION_TYPE_NUMBER:
            break;
        case EXPRESSION_TYPE_PREFIX:
        {
            error_t err = _expression_simplify_recursive(expr->prefix.right, scope);
            if (err) return err;

            if (expr->prefix.right->type == EXPRESSION_TYPE_NUMBER) {
                scalar_t result;
                SCALAR_INIT(result);

                switch (expr->prefix.prefix) {
                    case '+':
                        SCALAR_SET(expr->prefix.right->number.value, result);
                        break;
                    case '-':
                        SCALAR_NEGATE(expr->prefix.right->number.value, result);
                        break;
                    default:
                        return ERROR_INVALID_PREFIX;
                }
                expression_free(expr->prefix.right);

                expr->type = EXPRESSION_TYPE_NUMBER;
                SCALAR_INIT(expr->number.value);
                SCALAR_SWAP(result, expr->number.value);
                SCALAR_CLEAN(result);
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
                scalar_t result;
                SCALAR_INIT(result);

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
                    case '(':
                        SCALAR_MUL(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '^':
                        SCALAR_POW(expr->operator.left->number.value, expr->operator.right->number.value, result);
                        break;
                    case '\\':
#                       if (SCALAR_FEATURES & SCALAR_FEATURE_SQAURE_ROOT)
                            if (SCALAR_COMPARE_INT(expr->operator.left->number.value, 2) == 0) {
                                SCALAR_SQUARE_ROOT(expr->operator.left->number.value,
                                    expr->operator.right->number.value, result);
                                break;
                            }
#                       endif
#                       if (SCALAR_FEATURES & SCALAR_FEATURE_CUBE_ROOT)
                            if (SCALAR_COMPARE_INT(expr->operator.left->number.value, 3) == 0) {
                                SCALAR_CUBE_ROOT(expr->operator.left->number.value,
                                    expr->operator.right->number.value, result);
                                break;
                            }
#                       endif
#                       if (SCALAR_FEATURES & SCALAR_FEATURE_ROOT_N)
                            SCALAR_ROOT(expr->operator.left->number.value, expr->operator.right->number.value, result);
#                       else
                            return ERROR_CANNOT_PERFORM_VARIABLE_ROOTS;
#                       endif
                        break;
                    case '=':
                    case '>':
                    case '<':
                        SCALAR_CLEAN(result);
                        return expression_to_bool(expr);
                    default:
                        SCALAR_CLEAN(result);
                        return ERROR_INVALID_OPERATOR;
                }
                expression_free(expr->operator.left);
                expression_free(expr->operator.right);

                expr->type = EXPRESSION_TYPE_NUMBER;
                SCALAR_SWAP(result, expr->number.value);
            }
            break;
        }
    }
    return ERROR_NO_ERROR;
}

int _expression_has_variable_recursive(expression_t* expr, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NULL:
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
        case EXPRESSION_TYPE_NULL:
        case EXPRESSION_TYPE_NUMBER:
            return ERROR_VARIABLE_NOT_PRESENT;
        case EXPRESSION_TYPE_OPERATOR:
        {
            operator_t my_op;
            if (expr->operator.infix == '=') {
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

            expression_t* new_target = new_expression();
            new_target->type = EXPRESSION_TYPE_OPERATOR;
            if (_expression_has_variable_recursive(expr->operator.left, var)) {
                new_target->operator.left = *target;
                new_target->operator.right = expr->operator.right;
            } else {
                new_target->operator.right = expr->operator.left;
                new_target->operator.left  = *target;
            }

            switch (expr->operator.infix) {
                case '+':
                    my_op = '-';
                    break;
                case '-':
                    my_op = '+';
                    break;
                case '/':
                    my_op = '*';
                    break;
                case '*':
                case '(':
                    my_op = '/';
                    break;
                case '^':
                    my_op = '\\';
                    break;
                case '\\':
                    my_op = '^';
                    break;
                default:
                    break;
            }

            new_target->operator.infix = my_op;

            *target = new_target;
            error_t err = _expression_isolate_variable_recursive(expr->operator.right, target, var);
            if (err != ERROR_VARIABLE_NOT_PRESENT)
                return err;

            err =  _expression_isolate_variable_recursive(expr->operator.left, target, var);
            variable_t nv;
            if (expr->operator.left->type == EXPRESSION_TYPE_VARIABLE) {
                nv = expr->operator.left->variable.value;
            } else {
                nv = expr->operator.right->variable.value;
            }

            expr->type = EXPRESSION_TYPE_VARIABLE;
            expr->variable.value = nv;
            return err;
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
    if ( !(expr->type == EXPRESSION_TYPE_OPERATOR && expr->operator.infix == '=') ) {
            expression_t* left = new_expression();
            memmove(left, expr, sizeof(expression_t));
            expr->type = EXPRESSION_TYPE_OPERATOR;
            expr->operator.left = left;
            scalar_t zero;
            SCALAR_SET_INT(0, zero);
            expr->operator.right = new_number_expression(zero);
            expr->operator.infix = '=';
    }

    return _expression_isolate_variable_recursive(expr, NULL, var);
}
