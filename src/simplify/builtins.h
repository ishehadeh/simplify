/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_BUILTINS_H_
#define SIMPLIFY_BUILTINS_H_

/* this file includes utilities for creating builtin functions and constants */

#define DEFINE_MPFR_FUNCTION(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    expression_t input; \
    scope_get_value(scope, "__arg0", &input); \
    if (!EXPRESSION_IS_NUMBER(&input)) { \
        return ERROR_NO_ERROR; \
        expression_clean(&input); \
    } \
    mpfr_ptr num = malloc(sizeof(mpfr_t)); \
    mpfr_init(num); \
    mpfr_## NAME(num, input.number.value, MPFR_RNDN); \
    *out = expression_new_number(num); \
    expression_clean(&input); \
    return ERROR_NO_ERROR; \
}

#define DEFINE_MPFR_FUNCTION_NRND(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    expression_t input; \
    scope_get_value(scope, "__arg0", &input); \
    if (!EXPRESSION_IS_NUMBER(&input)) { \
        return ERROR_NO_ERROR; \
        expression_clean(&input); \
    } \
    mpfr_ptr num = malloc(sizeof(mpfr_t)); \
    mpfr_init(num); \
    mpfr_## NAME(num, input.number.value); \
    *out = expression_new_number(num); \
    expression_clean(&input); \
    return ERROR_NO_ERROR; \
}

#define DEFINE_MPFR_FUNCTION2(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    expression_t input; \
    expression_t input2; \
    scope_get_value(scope, "__arg0", &input); \
    scope_get_value(scope, "__arg1", &input2); \
    if (!EXPRESSION_IS_NUMBER(&input) || !EXPRESSION_IS_NUMBER(&input2)) { \
        expression_clean(&input); \
        expression_clean(&input2); \
        return ERROR_NO_ERROR; \
    } \
    mpfr_ptr num = malloc(sizeof(mpfr_t)); \
    mpfr_init(num); \
    mpfr_## NAME(num, input.number.value, input2.number.value, MPFR_RNDN); \
    *out = expression_new_number(num); \
    expression_clean(&input); \
    expression_clean(&input2); \
    return ERROR_NO_ERROR; \
}

#define EXPORT_BUILTIN_FUNCTION(SCOPE, NAME) \
    scope_define_internal_function((SCOPE), #NAME, builtin_func_ ## NAME, 1, "__arg0");

#define EXPORT_BUILTIN_FUNCTION2(SCOPE, NAME) \
    scope_define_internal_function((SCOPE), #NAME, builtin_func_ ## NAME, 2, "__arg0", "__arg1");

#define DEFINE_MPFR_CONST(NAME) \
error_t builtin_const_ ## NAME(scope_t* scope, expression_t** out) { \
    (void)scope; \
    mpfr_ptr num = malloc(sizeof(mpfr_t)); \
    mpfr_init(num); \
    mpfr_const_ ## NAME(num, MPFR_RNDF); \
    *out = expression_new_number(num); \
    return ERROR_NO_ERROR; \
}

#define EXPORT_BUILTIN_CONST(SCOPE, NAME) \
    scope_define_internal_const((SCOPE), #NAME, builtin_const_ ## NAME);

#define ALIAS(SCOPE, X, Y) scope_define_constant((SCOPE), #X, expression_new_variable(#Y))

#endif  // SIMPLIFY_BUILTINS_H_
