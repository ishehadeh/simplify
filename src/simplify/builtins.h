/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_BUILTINS_H_
#define SIMPLIFY_BUILTINS_H_

#include "simplify/errors.h"
#include "simplify/expression/expression.h"

#define EXPORT_MPFR_FUNCTION(SCOPE, NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    mpfr_t num; \
    mpfr_init(num); \
    expression_t input; \
    scope_get_value(scope, "__arg0", &input); \
    if (!EXPRESSION_IS_NUMBER(&input)) \
        return ERROR_INVALID_NUMBER; \
    mpfr_## NAME(num, input.number.value, MPFR_RNDF); \
    *out = expression_new_number(num); \
    mpfr_clear(num); \
    return ERROR_NO_ERROR; \
} \
scope_define_internal_function((SCOPE), #NAME, builtin_func_ ## NAME, 1, "__arg0");

#define EXPORT_MPFR_CONST(SCOPE, NAME) \
error_t builtin_const_ ## NAME(scope_t* scope, expression_t** out) { \
    (void)scope; \
    mpfr_t num; \
    mpfr_init(num); \
    mpfr_const_ ## NAME(num, MPFR_RNDF); \
    *out = expression_new_number(num); \
    mpfr_clear(num); \
    return ERROR_NO_ERROR; \
} \
scope_define_internal_const((SCOPE), #NAME, builtin_const_ ## NAME);

#endif  // SIMPLIFY_BUILTINS_H_
