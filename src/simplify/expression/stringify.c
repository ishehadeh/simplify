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
    }

    mpfr_clear(int_part);
    mpfr_clear(int_part_frac);

    // if (format->approximate_tolerance > 0)
    //     string->len = start + _approximate_number(string->buffer + start,
    //                                               format->approximate_tolerance,
    //                                               string->len - start);
}

void _write_variable(string_t* string, expression_t* variable) {
    assert(EXPRESSION_IS_VARIABLE(variable));
    string_append_cstring(string, variable->variable.value);
}

void _write_operator_prec_recursive(string_t* string, string_format_t* fmt, expression_t* op, operator_precedence_t prec) {
    assert(EXPRESSION_IS_OPERATOR(op));

    operator_precedence_t myprec = operator_precedence(op->operator.infix);
    if (myprec < prec)
        string_append_char(string, '(');

    _write_expression_prec(string, fmt, op->operator.left, myprec);

    string_append_cstring(string, fmt->whitespace);
    string_append_char(string, op->operator.infix);
    string_append_cstring(string, fmt->whitespace);

    _write_expression_prec(string, fmt, op->operator.right, myprec);

    if (myprec < prec)
        string_append_char(string, ')');
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

    if (!mpfr_zero_p(real)) {
        _write_mpfr(string, fmt, real);
        if (!mpfr_zero_p(imag)) {
            string_append_cstring(string, fmt->whitespace);
            string_append_char(string, '+');
            string_append_cstring(string, fmt->whitespace);
        }
    }

        if (mpfr_zero_p(imag)) {
        return;
    }

    if (mpfr_cmp_ui(imag, 1) != 0)
        _write_mpfr(string, fmt, imag);
    string_append_cstring(string, fmt->imaginary);
    return;
}

void _write_function(string_t* string, string_format_t* fmt, expression_t* func)  {
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
            if (!fmt->omit_parmeter_parentheses)
                string_append_cstring(string, fmt->whitespace);
        }
    }

    if (!fmt->omit_parmeter_parentheses)
        string_append_char(string, ')');
}

void _write_expression_prec(string_t* string, string_format_t* fmt, expression_t* expr, operator_precedence_t prec) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
            return _write_function(string, fmt, expr);
        case EXPRESSION_TYPE_PREFIX:
            return _write_prefix(string, fmt, expr, prec);
        case EXPRESSION_TYPE_OPERATOR:
            return _write_operator_prec_recursive(string, fmt, expr, prec);
        case EXPRESSION_TYPE_VARIABLE:
            return _write_variable(string, expr);
        case EXPRESSION_TYPE_NUMBER:
            return _write_number(string, fmt, expr);
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
