/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/stringify.h"


void _stringifier_round_number(stringifier_t* st, size_t start, size_t numstart) {
    /* round direction is `1` to round up or `-1` to round down */
    int round_direction = st->buffer[start - 1] >= '5' ? 1 : -1;

    /* round the digits before the decimal point */
    size_t i;
    for (i = start; i > numstart && isdigit(st->buffer[i]); --i) {
        if (st->buffer[i] + round_direction <= '0' ||  st->buffer[i] + round_direction >= '9') {
            /* if the current character can't safely be rounded than zero it and continue */
            st->buffer[i] = '0';
        } else {
            /* apply the the round direction, the exit. */
            st->buffer[i] += round_direction;
            ++i;
            goto finished;
        }
    }

    /* if the earlier loop ran out of room
        than that means we need to move past the decimal point */
    if (i == numstart) {
        if (round_direction > 0)
            st->buffer[i] = '1';
        else
            st->buffer[i] = '0';
        goto finished;
    } else if (st->buffer[i] == '.') {
        --i;
    } else {
        goto finished;
    }

    size_t d = i;
    /* try to round everything before the decimal */
    for (; d >= numstart && isdigit(st->buffer[d]); --d) {
        if (st->buffer[d] + round_direction <= '0' ||  st->buffer[d] + round_direction >= '9') {
            st->buffer[d] = '0';
        } else {
            st->buffer[d] += round_direction;
            ++i;
            goto finished;
        }
    }

    if (i == numstart) {
        /* push back the string to make room for a new digit */
        memmove(st->buffer + d + 1, st->buffer + d, start - d);
        if (round_direction > 0)
            st->buffer[d] = '1';
        else
            st->buffer[d] = '0';
    }

finished:
    st->index = i;
    return;
}

/* try to trim a floating point number that was convert to a string, to improve accuracy
 * @st the stringifier that the number was written to, assume that the number was the last thing written to the stringifier
 * @tolerance how easily the string should be rounded off. (five is probably a good default for this parameter)
 * @length the numbers's length
 */
void _stringifier_approximate_number(stringifier_t* st, size_t length) {
    size_t decimal_index = st->index - 1;

    for (; st->buffer[decimal_index] != '.'; --decimal_index) {
        /* if there is no decimal point then exit, there is nothing left to do */
        if (decimal_index < st->index - length) return;
    }

    size_t chain = 0;
    char last = 0;

    for (size_t i = decimal_index + 1; i < st->index && isdigit(st->buffer[i]); ++i) {
        if (st->buffer[i] == last) {
            ++chain;
        } else {
            if (chain >= st->approximate_tolerance) {
                if (last == '9') {
                    _stringifier_round_number(st, i - 2, st->index - length - 1);
                } else if (last == '0') {
                    st->index = i - chain - 2;
                    return;
                }
            }
            last = st->buffer[i];
            chain = 0;
        }
    }

    if (chain >= st->approximate_tolerance) {
        if (last == '9') {
            _stringifier_round_number(st, st->index - 2, st->index - length - 1);
        } else if (last == '0') {
            st->index = st->index - chain - 2;
        }
    }
    return;
}

size_t stringifier_write_variable(stringifier_t* st, expression_t* variable) {
    assert(EXPRESSION_IS_VARIABLE(variable));
    return stringifier_write(st, variable->variable.value);
}

size_t stringifier_write_operator(stringifier_t* st, expression_t* op) {
    assert(EXPRESSION_IS_OPERATOR(op));
    operator_precedence_t oldprec = st->current_precedence;
    st->current_precedence = operator_precedence(op->operator.infix);
    size_t written = 0;
    if (st->current_precedence < oldprec)
        written += stringifier_write_byte(st, '(');

    written += stringifier_write_expression(st, op->operator.left);

    written +=stringifier_write_whitespace(st);
    written += stringifier_write_byte(st, op->operator.infix);
    written += stringifier_write_whitespace(st);

    written += stringifier_write_expression(st, op->operator.right);

    if (st->current_precedence < oldprec)
        written += stringifier_write_byte(st, ')');

    st->current_precedence = oldprec;
    return written;
}

size_t stringifier_write_prefix(stringifier_t* st, expression_t* pre) {
    assert(EXPRESSION_IS_PREFIX(pre));
    size_t written = stringifier_write_byte(st, pre->prefix.prefix);
    written += stringifier_write_expression(st, pre->prefix.right);
    return written;
}

size_t stringifier_write_number(stringifier_t* st, expression_t* number) {
    assert(EXPRESSION_IS_NUMBER(number));

    mpfr_ptr num = number->number.value;
    static const int base = 10;
    const bool neg = mpfr_sgn(num) < 0;

    if (!mpfr_regular_p(num)) {
        if (mpfr_nan_p(num)) {
            return stringifier_write(st, st->nan_string);
        } else if (mpfr_inf_p(num)) {
            return (neg ? stringifier_write_byte(st, '-') : 0)
                + stringifier_write(st, st->inf_string);
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
    size_t numlen = ceil(prec * log(2)/log(base)) + 3;

    _STRINGIFIER_FIT(st, numlen);
    char* numstart = st->buffer + st->index;
    mpfr_get_str(numstart, &exponent, base, numlen, number->number.value, MPFR_RNDN);
    st->index += numlen;

    if (exponent > 0) {
        memmove(numstart + exponent + 1, numstart + exponent, numlen - exponent);
        numstart[exponent] = '.';
        ++st->index;
    } else {
        exponent = abs(exponent);

        memmove(numstart + exponent + 2, numstart, numlen - exponent);
        numstart[0] = '0';
        numstart[1] = '.';
        for (int i = 2; i < exponent + 2; ++i) {
            numstart[i] = '0';
        }
        ++st->index;
    }

    if (st->approximate_numbers)
        _stringifier_approximate_number(st, numlen);

    return written + st->index - (size_t)numstart;
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

size_t stringifier_write_expression(stringifier_t* st, expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
            return stringifier_write_function(st, expr);
        case EXPRESSION_TYPE_PREFIX:
            return stringifier_write_prefix(st, expr);
        case EXPRESSION_TYPE_OPERATOR:
            return stringifier_write_operator(st, expr);
        case EXPRESSION_TYPE_VARIABLE:
            return stringifier_write_variable(st, expr);
        case EXPRESSION_TYPE_NUMBER:
            return stringifier_write_number(st, expr);
    }
    return 0;
}

char* stringify(expression_t* expr) {
    stringifier_t st = STRINGIFIER_DEFAULT();
    stringifier_write_expression(&st, expr);
    stringifier_write_byte(&st, 0);
    return st.buffer;
}

error_t expression_fprint(expression_t* expr, FILE* f) {
    char* str = stringify(expr);
    fputs(str, f);
    free(str);
    return ERROR_NO_ERROR;
}
