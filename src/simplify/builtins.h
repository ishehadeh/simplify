/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_BUILTINS_H_
#define SIMPLIFY_BUILTINS_H_

#include "simplify/errors.h"
#include "simplify/expression/expression.h"

/* this file includes utilities for creating builtin functions and constants */

#define DEFINE_MPFR_FUNCTION(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    mpfr_t num; \
    mpfr_init(num); \
    expression_t input; \
    scope_get_value(scope, "__arg0", &input); \
    if (!EXPRESSION_IS_NUMBER(&input)) \
        return ERROR_INVALID_NUMBER; \
    mpfr_## NAME(num, input.number.value, MPFR_RNDN); \
    *out = expression_new_number(num); \
    mpfr_clear(num); \
    return ERROR_NO_ERROR; \
}

#define DEFINE_MPFR_FUNCTION_NRND(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    mpfr_t num; \
    mpfr_init(num); \
    expression_t input; \
    scope_get_value(scope, "__arg0", &input); \
    if (!EXPRESSION_IS_NUMBER(&input)) \
        return ERROR_INVALID_NUMBER; \
    mpfr_## NAME(num, input.number.value); \
    *out = expression_new_number(num); \
    mpfr_clear(num); \
    return ERROR_NO_ERROR; \
}

#define DEFINE_MPFR_FUNCTION2(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    mpfr_t num; \
    mpfr_init(num); \
    expression_t input; \
    expression_t input2; \
    scope_get_value(scope, "__arg0", &input); \
    scope_get_value(scope, "__arg1", &input2); \
    if (!EXPRESSION_IS_NUMBER(&input) || !EXPRESSION_IS_NUMBER(&input2)) \
        return ERROR_INVALID_NUMBER; \
    mpfr_## NAME(num, input.number.value, input2.number.value, MPFR_RNDN); \
    *out = expression_new_number(num); \
    mpfr_clear(num); \
    return ERROR_NO_ERROR; \
}

#define EXPORT_MPFR_FUNCTION(SCOPE, NAME) \
    scope_define_internal_function((SCOPE), #NAME, builtin_func_ ## NAME, 1, "__arg0");

#define EXPORT_MPFR_FUNCTION2(SCOPE, NAME) \
    scope_define_internal_function((SCOPE), #NAME, builtin_func_ ## NAME, 2, "__arg0", "__arg1");

#define DEFINE_MPFR_CONST(NAME) \
error_t builtin_const_ ## NAME(scope_t* scope, expression_t** out) { \
    (void)scope; \
    mpfr_t num; \
    mpfr_init(num); \
    mpfr_const_ ## NAME(num, MPFR_RNDF); \
    *out = expression_new_number(num); \
    mpfr_clear(num); \
    return ERROR_NO_ERROR; \
}

#define EXPORT_MPFR_CONST(SCOPE, NAME) \
    scope_define_internal_const((SCOPE), #NAME, builtin_const_ ## NAME);

#define ALIAS(SCOPE, X, Y) scope_define_constant((SCOPE), #X, expression_new_variable(#Y))

#endif  // SIMPLIFY_BUILTINS_H_
