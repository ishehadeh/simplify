/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"
#include "simplify/expression/stringify.h"


int index_of(char* x, char* y) {
    int i = 0;
    int yi = 0;
    while (x[i]) {
        if (!y[yi])
            return i;
        if (y[yi] == x[i])
            ++yi;
        else
            yi = 0;
        ++i;
    }

    return -1;
}

static inline unsigned long long _pow(unsigned long long x, unsigned long long y) {
    while (y-- > 0) x *= x;
    return x;
}

int strintcmp(char* x, char* y, int lenx, int leny, bool dec) {
    int xi = 0;
    int yi = 0;
    int cmp_mul = 1;

    unsigned long long xp;
    unsigned long long yp;

    if (!dec) {
        xp = _pow(10, lenx - 1);
        yp = _pow(10, leny - 1);
    } else {
        xp = 1;
        yp = 1;
    }

    if (y[0] == '-' || x[0] == '-') {
        if (y[0] != '-') {
            return -1;
        } else if (x[0] != '-') {
            return 1;
        } else {
            cmp_mul = -1;
            ++yi;
            ++xi;
        }
    }

    while (xi < lenx || yi < leny) {
        int cmp = cmp_mul * ((x[xi] * xp) - (y[yi] * yp));
        if (cmp != 0) {
            return cmp;
        }

        if (!dec) {
            xp /= 10;
            yp /= 10;
        } else {
            xp *= 10;
            yp *= 10;
        }

        if (xi < lenx)
            ++xi;
        else
            xp = 0;

        if (yi < leny)
            ++yi;
        else
            yp = 0;
    }
    return 0;
}

int strnumcmp(char* x, char* y, int lenx, int leny) {
    int decx = index_of(x, ".") - 1;
    int decy = index_of(y, ".") - 1;
    if (decx < 0) decx = lenx;
    if (decy < 0) decy = leny;

    int cmp = strintcmp(x, y, decx, decy, false);
    if (cmp != 0 || (decx == lenx && decy == leny))
        return cmp;

    return strintcmp(x + decx, y + decy, lenx - decx, leny - decy, true);
}

compare_result_t _expression_compare_numbers(expression_t* expr1, expression_t* expr2) {
    if (!mpfr_zero_p(mpc_imagref(expr1->number.value)) || !mpfr_zero_p(mpc_imagref(expr2->number.value))) {
        if (mpc_cmp(expr1->number.value, expr2->number.value) == 0) {
            return COMPARE_RESULT_EQUAL;
        } else {
            return COMPARE_RESULT_INCOMPARABLE;
        }
    }

    string_format_t sf = STRING_FORMAT_DEFAULT();
    string_t s;
    string_init(&s);
    write_expression(&s, &sf, expr1);

    char* expr1str = s.buffer;
    size_t expr1len = s.len;
    expr1str[expr1len] = 0;

    string_init(&s);
    write_expression(&s, &sf, expr2);

    char* expr2str = s.buffer;
    size_t expr2len = s.len;
    expr2str[expr2len] = 0;

    int result = strnumcmp(expr1str, expr2str, expr1len, expr2len);

    free(expr1str);
    free(expr2str);

    if (result < 0) {
        return COMPARE_RESULT_LESS;
    } else if (result > 0) {
        return COMPARE_RESULT_GREATER;
    } else if (result == 0) {
        return COMPARE_RESULT_EQUAL;
    }

    return COMPARE_RESULT_INCOMPARABLE;
}

compare_result_t _expression_compare_recursive(expression_t* expr1, expression_t* expr2) {
    if (expr1->type != expr2->type)
        return COMPARE_RESULT_INCOMPARABLE;

    switch (expr1->type) {
        case EXPRESSION_TYPE_NUMBER:
            return _expression_compare_numbers(expr1, expr2);
        case EXPRESSION_TYPE_VARIABLE:
            if (!strcmp(expr1->variable.value, expr2->variable.value))
                return COMPARE_RESULT_EQUAL;
            else
                return COMPARE_RESULT_INCOMPARABLE;
        case EXPRESSION_TYPE_FUNCTION:
            if (!strcmp(expr1->function.name, expr2->function.name)) {
                expression_t* arg1;
                expression_t* arg2;
                compare_result_t current = COMPARE_RESULT_EQUAL;
                EXPRESSION_LIST_FOREACH2(arg1, arg2, expr1->function.parameters, expr2->function.parameters) {
                    compare_result_t next = _expression_compare_recursive(arg1, arg2);
                    if (current == COMPARE_RESULT_EQUAL && next == COMPARE_RESULT_EQUAL) {
                            current = next;
                    } else {
                        return COMPARE_RESULT_INCOMPARABLE;
                    }
                }
                return current;
            } else {
                return COMPARE_RESULT_INCOMPARABLE;
            }
        case EXPRESSION_TYPE_PREFIX:
            if (expr1->prefix.prefix != expr2->prefix.prefix)
                return COMPARE_RESULT_INCOMPARABLE;
            else
                return _expression_compare_recursive(expr1->prefix.right, expr2->prefix.right);
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (expr1->operator.infix != expr2->operator.infix)
                return COMPARE_RESULT_INCOMPARABLE;
            compare_result_t result_left = _expression_compare_recursive(expr1->operator.left, expr2->operator.left);
            compare_result_t result_right = _expression_compare_recursive(expr1->operator.right, expr2->operator.right);
            if (result_left == result_right && result_left != COMPARE_RESULT_INCOMPARABLE) {
                return result_left;
            } else if (expr1->operator.infix == '*' || expr1->operator.infix == '+') {
                if (result_left == COMPARE_RESULT_INCOMPARABLE || result_right == COMPARE_RESULT_INCOMPARABLE) {
                    result_left = _expression_compare_recursive(expr1->operator.left, expr2->operator.right);
                    result_right = _expression_compare_recursive(expr1->operator.right, expr2->operator.left);
                    if (result_left == result_right && result_left != COMPARE_RESULT_INCOMPARABLE) {
                        return result_left;
                    }
                }
            }
            return COMPARE_RESULT_INCOMPARABLE;
        }
    }
    return COMPARE_RESULT_INCOMPARABLE;
}

compare_result_t expression_compare(expression_t* expr1, expression_t* expr2) {
    return _expression_compare_recursive(expr1, expr2);
}


variable_t _expression_find_variable_recursive(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
        case EXPRESSION_TYPE_NUMBER:
            return NULL;
        case EXPRESSION_TYPE_OPERATOR:
        {
            variable_t var = _expression_find_variable_recursive(expr->operator.left);
            if (var) return var;
            var = _expression_find_variable_recursive(expr->operator.right);
            return var;
        }
        case EXPRESSION_TYPE_PREFIX:
            return _expression_find_variable_recursive(expr->operator.right);
        case EXPRESSION_TYPE_VARIABLE:
            return expr->variable.value;
    }
    return NULL;
}


int _expression_has_variable_or_function_recursive(expression_t* expr, variable_t var) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            return 0;
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (_expression_has_variable_or_function_recursive(expr->operator.left, var))
                return 1;
            if (_expression_has_variable_or_function_recursive(expr->operator.right, var))
                return 1;
            return 0;
        }
        case EXPRESSION_TYPE_PREFIX:
        {
            if (_expression_has_variable_or_function_recursive(expr->prefix.right, var))
                return 1;
            return 0;
        }
        case EXPRESSION_TYPE_VARIABLE:
            return strcmp(var, expr->variable.value) == 0;
        case EXPRESSION_TYPE_FUNCTION:
            return strcmp(var, expr->function.name) == 0;
    }
    return 0;
}


int expression_has_variable_or_function(expression_t* expr, variable_t var) {
    return _expression_has_variable_or_function_recursive(expr, var);
}

variable_t expression_find_variable(expression_t* expr) {
    return _expression_find_variable_recursive(expr);
}