/* Copyright Ian Shehadeh 2018 */

#include "simplify/errors.h"
#include "simplify/expression/expr_types.h"

void expression_init_operator(expression_t* expr,  expression_t* left, operator_t op, expression_t* right) {
    expr->type = EXPRESSION_TYPE_OPERATOR;
    expr->operator.infix = op;
    expr->operator.right = right;
    expr->operator.left  = left;
}

void expression_init_prefix(expression_t* expr, operator_t op, expression_t* right) {
    expr->type = EXPRESSION_TYPE_PREFIX;
    expr->prefix.prefix = op;
    expr->prefix.right  = right;
}

void expression_init_variable(expression_t* expr, char* name, size_t len) {
    expr->type = EXPRESSION_TYPE_VARIABLE;
    expr->variable.value = malloc(len + 1);
    expr->variable.value[len] = 0;
    expr->variable.binding = NULL;
    strncpy(expr->variable.value, name, len);
}

void expression_init_number(expression_t* expr, mpfr_t value) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    mpfr_init_set(expr->number.value, value, MPFR_RNDF);
}

void expression_init_number_d(expression_t* expr, double value) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    mpfr_init_set_d(expr->number.value, value, MPFR_RNDF);
}

void expression_init_number_si(expression_t* expr, int value) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    mpfr_init_set_si(expr->number.value, value, MPFR_RNDF);
}

void expression_init_function(expression_t* expr, char* name, size_t len, expression_list_t* params) {
    expr->type = EXPRESSION_TYPE_FUNCTION;
    expr->function.name = malloc(len + 1);
    expr->function.name[len] = 0;
    strncpy(expr->function.name, name, len);
    expr->function.parameters = params;
}

void expression_clean(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_PREFIX:
            expression_clean(expr->operator.right);
            free(expr->operator.right);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            expression_clean(expr->operator.left);
            expression_clean(expr->operator.right);
            free(expr->operator.left);
            free(expr->operator.right);
            break;
        case EXPRESSION_TYPE_NUMBER:
            mpfr_clear(expr->number.value);
            break;
        case EXPRESSION_TYPE_VARIABLE:
            free(expr->variable.value);
            break;
        case EXPRESSION_TYPE_FUNCTION:
            free(expr->function.name);
            expression_list_free(expr->function.parameters);
        default:
            break;
    }
}

void expression_free(expression_t* expr) {
    expression_clean(expr);
    free(expr);
}

void expression_copy(expression_t* expr, expression_t* out) {
    switch (expr->type) {
        case EXPRESSION_TYPE_PREFIX:
        {
            expression_t* right = (expression_t*)malloc(sizeof(expression_t));
            expression_copy(expr->prefix.right, right);
            expression_init_prefix(out, expr->prefix.prefix, right);
            break;
        }
        case EXPRESSION_TYPE_OPERATOR:
        {
            expression_t* right = (expression_t*)malloc(sizeof(expression_t));
            expression_t* left = (expression_t*)malloc(sizeof(expression_t));
            expression_copy(expr->operator.right, right);
            expression_copy(expr->operator.left,  left);
            expression_init_operator(out, left, expr->operator.infix, right);
            break;
        }
        case EXPRESSION_TYPE_NUMBER:
            expression_init_number(out, expr->number.value);
            break;
        case EXPRESSION_TYPE_VARIABLE:
            expression_init_variable(out, expr->variable.value, strlen(expr->variable.value));
            out->variable.binding = expr->variable.binding;
            break;
        case EXPRESSION_TYPE_FUNCTION:
        {
            expression_list_t* params = malloc(sizeof(expression_list_t));
            expression_list_init(params);
            expression_list_copy(expr->function.parameters, params);
            expression_init_function(out, expr->function.name, strlen(expr->function.name), params);
            break;
        }

        default:
            memcpy(expr, out, sizeof(expression_t));
            break;
    }
}

inline operator_precedence_t operator_precedence(operator_t op) {
    switch (op) {
        case '=':
        case '>':
        case '<':
            return OPERATOR_PRECEDENCE_COMPARE;
        case '+':
        case '-':
            return OPERATOR_PRECEDENCE_SUM;
        case '*':
        case '/':
            return OPERATOR_PRECEDENCE_PRODUCT;
        case '^':
        case '\\':
            return OPERATOR_PRECEDENCE_EXPONENT;
        case '(':
            return OPERATOR_PRECEDENCE_MAXIMUM;
        case ')':
            return OPERATOR_PRECEDENCE_MINIMUM;
        case ':':
            return OPERATOR_PRECEDENCE_ASSIGN;
        default:
            return OPERATOR_PRECEDENCE_MINIMUM;
    }
}