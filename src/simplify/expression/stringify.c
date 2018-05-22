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

/* try to trim a floating point number that was convert to a string, to improve accuracy
 * @str the string to edit
 * @tolerance how easily the string should be rounded off. (five is probably a good default for this parameter)
 * @length the string's length
 * @return an error code
 */
error_t approximate_number(char* str, int tolerance, size_t length) {
    int     decimal_index;
    error_t err;

    // Find the decimal point
    for (decimal_index = 0; str[decimal_index] != '.'; ++decimal_index) {
        // if there is no decimal point then exit
        if (!str[decimal_index]) return ERROR_NO_ERROR;
    }

    int  chain = 0;
    char last = 0;
    for (size_t i = decimal_index + 1; isdigit(str[i]) || str[i] == 0; ++i) {
        if (str[i] == last) {
            ++chain;
        } else {
            if (chain >= tolerance && (last == '9' || last == '0')) {
                err = __round_string(str, i);
                if (err) return err;
            }
            last = str[i];
        }

        if (i >= length) {
            break;
        }
    }

    return ERROR_NO_ERROR;
}

size_t stringifier_write_number(stringifier_t* st, expression_t* number) {
    assert(EXPRESSION_IS_NUMBER(number));

    mpfr_ptr num = number->number.value;
    static const int base = 10;
    const bool neg = mpfr_sgn(num) < 0;

    if (!mpfr_regular_p(num)) {
        if (mpfr_nan_p(num)) {
            return stringifier_write(st, NAN_STRING);
        } else if (mpfr_inf_p(num)) {
            return (neg ? stringifier_write_byte(st, '-') : 0)
                + stringifier_write(st, INF_STRING);
        } else {
            assert(mpfr_zero_p(num));
            return (neg ? stringifier_write_byte(st, '-') : 0)
                + stringifier_write_byte(st, '0');
        }
    }

    size_t written = 0;
    mpfr_prec_t prec = mpfr_get_prec(num);
    mpfr_exp_t exponent;

    if (neg) {
        written += stringifier_write_byte(st, '-');
        mpfr_abs(num, num, MPFR_RNDN);
    }
    char* numstart = st->buffer + st->index;
    size_t numlen = ceil(prec * log(2)/log(base)) + 1;

    _STRINGIFIER_FIT(st, numlen);
    mpfr_get_str(numstart, &exponent, base, numlen, number->number.value, MPFR_RNDN);
    st->index += numlen;
    if (exponent > 0) {
        memmove(numstart + exponent + 1, numstart + exponent, numlen - exponent);
        numstart[exponent] = '.';
        ++st->index;
    }
    approximate_number(numstart, 5, numlen);
    written += numlen;
    return written;
}

char* stringify(expression_t* expr) {
    stringifier_t st;
    st.buffer = malloc(STRINGIFIER_DEFAULT_SIZE);
    st.index = 0;
    st.length = 0;
    stringifier_write_expression(&st, expr);
    stringifier_write_byte(&st, 0);
    return st.buffer;
}

size_t stringifier_write_function(stringifier_t* st, expression_t* func) {
    assert(EXPRESSION_IS_FUNCTION(func));
    size_t written = stringifier_write(st, func->function.name);
    written += stringifier_write_byte(st, '(');
    expression_t* arg;
    EXPRESSION_LIST_FOREACH(arg, func->function.parameters) {
        if (__item->next)
            written += stringifier_write_byte(st, ',');
        written += stringifier_write_expression(st, arg);
    }
    written += stringifier_write_byte(st, ')');
    return written;
}

error_t expression_fprint(expression_t* expr, FILE* f) {
    char* str = stringify(expr);
    fputs(str, f);
    free(str);
    return ERROR_NO_ERROR;
}
