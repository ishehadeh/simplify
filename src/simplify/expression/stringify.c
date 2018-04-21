/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/stringify.h"


error_t __round_string(char* str, int len) {
    // round direction is `1` to round up or `-1` to round down
    int round_direction = str[len - 1] >= '5' ? 1 : -1;
    int new_len = len;

    // round the first set of digits
    int i;
    for (i = len - 2; i > -1 && isdigit(str[i]); --i) {
        if (round_direction < 0) {
            if (str[i] + round_direction <= '0' ||  str[i] + round_direction >= '9') {
                // if the current character can't safely be rounded than zero it and continue
                str[i] = '0';
            } else {
                // apply the the round direction, the exit.
                str[i] += round_direction;
                return ERROR_NO_ERROR;
            }
        }
    }

    // if the earlier loop ran out of room, than the string can just be set to zero or one
    if (i == 0) {
        if (round_direction > 0)
            str[i] = '1';
        else
            str[i] = '0';
        str[i + 1] = 0;
        return ERROR_NO_ERROR;
    } else if (str[i] == '.') {
        // truncate the string at the decimal if possible
        str[i] = 0;
        new_len = len - i;
        ++i;
    } else {
        return ERROR_INVALID_NUMBER;
    }

    // if there's more keep trying to round the second half of the number
    for (; i > -1 && isdigit(str[i]); --i) {
        if (round_direction < 0) {
            if (str[i] + round_direction <= '0' ||  str[i] + round_direction >= '9') {
                str[i] = '0';
            } else {
                str[i] += round_direction;
                return ERROR_NO_ERROR;
            }
        }
    }

    if (i == 0) {
        // push back the string, there should be space, because the decimal point can be ignored
        memmove(str + 1, str, new_len);
        if (round_direction > 0)
            str[i] = '1';
        else
            str[i] = '0';
        str[i + 1] = 0;
    } else {
        return ERROR_INVALID_NUMBER;
    }

    return ERROR_NO_ERROR;
}

error_t approximate_number(char* str, int tolerance) {
    int     decimal_index;
    error_t err;

    // Find the decimal point
    for (decimal_index = 0; str[decimal_index] != '.'; ++decimal_index) {
        // if there is no decimal point then exit
        if (!str[decimal_index]) return ERROR_NO_ERROR;
    }

    int  chain = 0;
    char last = 0;
    for (int i = decimal_index + 1; isdigit(str[i]) || str[i] == 0; ++i) {
        if (str[i] == last) {
            ++chain;
        } else {
            if (chain >= tolerance && (last == '9' || last == '0')) {
                err = __round_string(str, i);
                if (err) return err;
            }
            last = str[i];
        }

        if (str[i] == 0) {
            break;
        }
    }

    return ERROR_NO_ERROR;
}

error_t number_to_buffer(mpfr_t val, char* buffer, size_t len) {
    if (len) {
        char x = buffer[len + 1];
        mpfr_snprintf(buffer, len + 1, "%.*RUg", mpfr_get_prec(val), val);
        buffer[len + 1] = x;
    } else {
        mpfr_sprintf(buffer, "%.*RUg", mpfr_get_prec(val), val);
    }

    return ERROR_NO_ERROR;
}

error_t operator_to_buffer(operator_t op, char* buffer, size_t len) {
    (void)len;

    buffer[0] = op;
    return ERROR_NO_ERROR;
}

error_t variable_to_buffer(variable_t var, char* buffer, size_t len) {
    for (size_t i = 0; (i < len || len == 0) && var[i]; ++i)
        buffer[i] = var[i];

    return ERROR_NO_ERROR;
}


error_t _expression_fprint_precedence_recursive(expression_t* expr, FILE* f, operator_precedence_t precedence) {
    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
            mpfr_fprintf(f, "%.*RUg", mpfr_get_prec(expr->number.value), expr->number.value);
            break;
        case EXPRESSION_TYPE_OPERATOR:
        {
            operator_precedence_t my_prec = operator_precedence(expr->operator.infix);

            if (my_prec < precedence)
                fputc('(', f);
            _expression_fprint_precedence_recursive(expr->operator.left, f, my_prec);
            fputc(' ', f);
            fputc(expr->operator.infix, f);
            fputc(' ', f);
            _expression_fprint_precedence_recursive(expr->operator.right, f, my_prec);
            if (my_prec < precedence)
                fputc(')', f);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            fprintf(f, "%s", expr->variable.value);
            break;
        case EXPRESSION_TYPE_PREFIX:
            fputc(expr->prefix.prefix, f);
            expression_fprint(expr->prefix.right, f);
            break;
        case EXPRESSION_TYPE_FUNCTION:
        {
            fprintf(f, "%s(", expr->function.name);
            expression_t* arg;
            EXPRESSION_LIST_FOREACH(arg, expr->function.parameters) {
                expression_fprint(arg, f);
                if (__item->next)
                    fprintf(f, ", ");
            }
            fputc(')', f);
            break;
        }
    }
    return ERROR_NO_ERROR;
}

error_t expression_fprint(expression_t* expr, FILE* f) {
    return _expression_fprint_precedence_recursive(expr, f, OPERATOR_PRECEDENCE_MINIMUM);
}
