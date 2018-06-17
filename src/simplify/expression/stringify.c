/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/stringify.h"

void _write_expression_prec(string_t* string, string_format_t* fmt, expression_t* expr, operator_precedence_t prec);

static inline void _reverse(char* x, size_t i, size_t n) {
    while (i < n) {
        x[i] ^= x[n];
        x[n] ^= x[i];
        x[i] ^= x[n];
        ++i;
        --n;
    }
}

size_t _stringifier_round_number(char* str, size_t start) {
    /* round direction is `1` to round up or `-1` to round down */
    int round_direction = str[start] >= '5' ? 1 : -1;

    /* round the digits before the decimal point */
    size_t i;
    for (i = start; i > 0 && isdigit(str[i]); --i) {
        if (str[i] + round_direction < '0' || str[i] + round_direction > '9') {
            /* if the current character can't safely be rounded than zero it and continue */
            str[i] = '0';
        } else {
            /* apply the the round direction, the exit. */
            str[i] += round_direction;
            ++i;
            goto finished;
        }
    }

    /* if the earlier loop ran out of room
        than that means we need to move past the decimal point */
    if (i == 0) {
        if (round_direction > 0)
            str[i] = '1';
        else
            str[i] = '0';
        goto finished;
    } else if (str[i] == '.') {
        --i;
    } else {
        goto finished;
    }

    size_t d = i;
    /* try to round everything before the decimal */
    for (; d > 0 && isdigit(str[d]); --d) {
        if (str[d] + round_direction < '0' || str[d] + round_direction > '9') {
            str[d] = '0';
        } else {
            str[d] += round_direction;
            i = d;
            goto finished;
        }
    }

    if (i == 0) {
        /* push back the string to make room for a new digit */
        memmove(str + d + 1, str + d, start - d);
        if (round_direction > 0)
            str[d] = '1';
        else
            str[d] = '0';
    }

finished:
    return i;
}

size_t _approximate_number(char* str, size_t tolerance, size_t length) {
    size_t decimal_index = 0;

    for (; str[decimal_index] != '.'; ++decimal_index) {
        /* if there is no decimal point then exit, there is nothing left to do */
        if (decimal_index >= length) return length;
    }

    size_t chain = 0;
    size_t idx = length;
    char last = 0;

    for (size_t i = decimal_index + 1; i < length && isdigit(str[i]); ++i) {
        if (str[i] == last) {
            ++chain;
        } else {
            if (chain >= tolerance) {
                if (last == '9') {
                    idx = _stringifier_round_number(str, i - 1);
                } else if (last == '0') {
                    idx = i - chain - 1;
                    goto finished;
                }
            }
            last = str[i];
            chain = 0;
        }
    }

    if (chain >= tolerance) {
        if (last == '9') {
            idx = _stringifier_round_number(str, length - 1);
        } else if (last == '0') {
            idx = length - chain - 1;
        }
    }

finished:
    if (str[idx - 1] == '.') --idx;
    return idx;
}

void _write_mpfr(string_t* string, string_format_t* format, mpfr_ptr num) {
    mpfr_t int_part;
    mpfr_t int_part_frac;

    if (mpfr_sgn(num) < 0) {
        string_append_char(string, '-');
        mpfr_neg(num, num, MPFR_RNDN);
    }

    if (!mpfr_regular_p(num)) {
        if (mpfr_inf_p(num)) {
            string_append_cstring(string, format->infinity);
        } else if (mpfr_nan_p(num)) {
            string_append_cstring(string, format->not_a_number);
        } else if (mpfr_zero_p(num)) {
            string_append_char(string, '0');
        }
        return;
    }

    mpfr_inits(int_part, int_part_frac, NULL);
    mpfr_modf(int_part, num, num, MPFR_RNDF);

    size_t start = string->len;
    if (!mpfr_zero_p(int_part)) {
        while (!mpfr_zero_p(int_part)) {
            mpfr_div_ui(int_part_frac, int_part, 10, MPFR_RNDF);
            mpfr_modf(int_part, int_part_frac, int_part_frac, MPFR_RNDF);
            mpfr_add_d(int_part_frac, int_part_frac, 0.03, MPFR_RNDF);
            mpfr_mul_ui(int_part_frac, int_part_frac, 10, MPFR_RNDF);
            string_append_char(string, mpfr_get_si(int_part_frac, MPFR_RNDN) + '0');
        }
        _reverse(string->buffer, start, string->len - 1);
    }

    if (!mpfr_zero_p(num)) {
        string_append_char(string, '.');
        while (!mpfr_zero_p(num)) {
            mpfr_mul_ui(num, num, 10, MPFR_RNDF);
            mpfr_modf(int_part_frac, num, num, MPFR_RNDF);
            string_append_char(string, mpfr_get_si(int_part_frac, MPFR_RNDN) + '0');
        }
        if (format->approximate_tolerance > 0)
            string->len =
                start + _approximate_number(string->buffer + start, format->approximate_tolerance, string->len - start);
    }

    mpfr_clear(int_part);
    mpfr_clear(int_part_frac);
}

void _write_variable(string_t* string, expression_t* variable) {
    assert(EXPRESSION_IS_VARIABLE(variable));
    string_append_cstring(string, variable->variable.value);
}

void _write_operator_prec_recursive(string_t* string, string_format_t* fmt, expression_t* op,
                                    operator_precedence_t prec) {
    assert(EXPRESSION_IS_OPERATOR(op));

    operator_precedence_t myprec = operator_precedence(op->operator.infix);
    if (myprec < prec) string_append_char(string, '(');

    _write_expression_prec(string, fmt, op->operator.left, myprec);

    if (!(op->operator.infix == '*' && fmt->brief_multiplication && !(EXPRESSION_IS_VARIABLE(EXPRESSION_LEFT(op)) &&
                                                                      EXPRESSION_IS_VARIABLE(EXPRESSION_RIGHT(op))))) {
        string_append_cstring(string, fmt->whitespace);
        string_append_char(string, op->operator.infix);
        string_append_cstring(string, fmt->whitespace);
    }

    _write_expression_prec(string, fmt, op->operator.right, myprec);

    if (myprec < prec) string_append_char(string, ')');
}

void _write_prefix(string_t* string, string_format_t* fmt, expression_t* pre, operator_precedence_t prec) {
    assert(EXPRESSION_IS_PREFIX(pre));
    string_append_char(string, pre->prefix.prefix);
    _write_expression_prec(string, fmt, pre->prefix.right, prec);
}

void _write_number(string_t* string, string_format_t* fmt, expression_t* number) {
    assert(EXPRESSION_IS_NUMBER(number));
    mpfr_t real;
    mpfr_t imag;
    mpfr_init(real);
    mpfr_init(imag);
    mpc_real(real, number->number.value, MPFR_RNDN);
    mpc_imag(imag, number->number.value, MPFR_RNDN);

    bool hasreal = !mpfr_zero_p(real);
    bool hasimag = !mpfr_zero_p(imag);
    if (hasreal) {
        _write_mpfr(string, fmt, real);
    } else if (!hasimag) {
        string_append_char(string, '0');
        goto cleanup;
    }

    if (!hasimag) goto cleanup;

    if (mpfr_sgn(imag) < 0) {
        if (!hasreal) {
            string_append_char(string, '-');
        } else {
            string_append_cstring(string, fmt->whitespace);
            string_append_char(string, '-');
            string_append_cstring(string, fmt->whitespace);
        }
        mpfr_neg(imag, imag, MPFR_RNDNA);
    } else if (hasreal) {
        string_append_cstring(string, fmt->whitespace);
        string_append_char(string, '+');
        string_append_cstring(string, fmt->whitespace);
    }

    if (mpfr_cmp_ui(imag, 1) != 0) _write_mpfr(string, fmt, imag);
    string_append_cstring(string, fmt->imaginary);

cleanup:
    mpfr_clear(real);
    mpfr_clear(imag);
    return;
}

void _write_function(string_t* string, string_format_t* fmt, expression_t* func) {
    assert(EXPRESSION_IS_FUNCTION(func));

    string_append_cstring(string, func->function.name);

    if (!fmt->omit_parmeter_parentheses)
        string_append_char(string, '(');
    else
        string_append_cstring(string, fmt->whitespace);

    expression_t* arg;
    EXPRESSION_LIST_FOREACH(arg, func->function.parameters) {
        _write_expression_prec(string, fmt, arg, OPERATOR_PRECEDENCE_MINIMUM);
        if (__item->next && !fmt->omit_parameter_commas) {
            string_append_char(string, ',');
            if (!fmt->omit_parmeter_parentheses) string_append_cstring(string, fmt->whitespace);
        }
    }

    if (!fmt->omit_parmeter_parentheses) string_append_char(string, ')');
}

void _write_expression_prec(string_t* string, string_format_t* fmt, expression_t* expr, operator_precedence_t prec) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
            _write_function(string, fmt, expr);
            break;
        case EXPRESSION_TYPE_PREFIX:
            _write_prefix(string, fmt, expr, prec);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            _write_operator_prec_recursive(string, fmt, expr, prec);
            break;
        case EXPRESSION_TYPE_VARIABLE:
            _write_variable(string, expr);
            break;
        case EXPRESSION_TYPE_NUMBER:
            _write_number(string, fmt, expr);
            break;
    }
}

void write_expression(string_t* string, string_format_t* format, expression_t* expr) {
    _write_expression_prec(string, format, expr, OPERATOR_PRECEDENCE_MINIMUM);
}

char* stringify(expression_t* expr) {
    string_format_t fmt = STRING_FORMAT_DEFAULT();
    string_t s;
    string_init(&s);
    write_expression(&s, &fmt, expr);
    string_append_char(&s, 0);
    return s.buffer;
}

void expression_fprint(expression_t* expr, FILE* f) {
    char* str = stringify(expr);
    fputs(str, f);
    free(str);
}
