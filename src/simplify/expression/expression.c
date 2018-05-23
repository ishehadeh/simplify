/* Copyright Ian Shehadeh 2018 */

#include "simplify/expression/expression.h"
#include "simplify/expression/stringify.h"


compare_result_t expression_compare(expression_t* expr1, expression_t* expr2) {
    compare_result_t retval = COMPARE_RESULT_INCOMPARABLE;

    stringifier_t st;
    st.buffer = malloc(sizeof(4096));
    st.length = 4096;
    st.index = 0;
    /* incase theses expressions aren't numbers, use invalid variable names for NAN and INF */
    st.nan_string = "@";
    st.inf_string = "#";
    st.whitespace = "";
    st.current_precedence = OPERATOR_PRECEDENCE_MINIMUM;
    st.approximate_numbers = true;
    st.approximate_tolerance = 5;

    stringifier_write_expression(&st, expr1);
    char* expr1str = st.buffer;
    st.buffer = malloc(sizeof(4096));
    st.length = 4096;
    st.index = 0;
    stringifier_write_expression(&st, expr2);
    char* expr2str = st.buffer;

    if (!strcmp(expr1str, expr2str)) {
        retval = COMPARE_RESULT_EQUAL;
        goto cleanup;
    }

    if (EXPRESSION_IS_NUMBER(expr1) && EXPRESSION_IS_NUMBER(expr2)) {
        /* if either number is NAN than they can not be compared */
        if (mpfr_unordered_p(expr1->number.value, expr2->number.value)) goto cleanup;

        int x = mpfr_cmp(expr1->number.value, expr2->number.value);
        if (!x)
            retval = COMPARE_RESULT_EQUAL;
        else if (x < 0)
            retval = COMPARE_RESULT_LESS;
        else if (x > 0)
            retval = COMPARE_RESULT_GREATER;
    }

cleanup:
    free(expr1str);
    free(expr2str);
    return retval;
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