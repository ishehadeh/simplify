/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_BUILTINS_H_
#define SIMPLIFY_BUILTINS_H_

#include "simplify/errors.h"
#include "simplify/expression/expression.h"

#define ADD_BUILTIN(SCOPE, NAME) scope_define_internal_function((SCOPE), #NAME, builtin_func_ ## NAME, 1, "__arg0")

#define _DEFINE_MPFR_BUILTIN(NAME) \
error_t builtin_func_ ## NAME(expression_list_t* args, scope_t* scope, expression_t** out) { \
    if (!args) \
        return ERROR_IS_A_FUNCTION; \
    (void)args; \
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
}

_DEFINE_MPFR_BUILTIN(cos)
_DEFINE_MPFR_BUILTIN(sin)
_DEFINE_MPFR_BUILTIN(tan)

_DEFINE_MPFR_BUILTIN(acos)
_DEFINE_MPFR_BUILTIN(atan)
_DEFINE_MPFR_BUILTIN(asin)


#endif  // SIMPLIFY_BUILTINS_H_
