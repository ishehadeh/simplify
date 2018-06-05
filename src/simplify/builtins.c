#include "src/simplify/builtins.h"

bool _g_rand_state_initialized;
gmp_randstate_t _g_rand_state;

#define _DEFINE_MPFR_BUILTIN(NAME) \
error_t builtin_func_ ## NAME(scope_t* scope, expression_t** out) { \
    expression_t input; \
    scope_get_value(scope, "__arg0", &input); \
    if (!EXPRESSION_IS_NUMBER(&input) || !mpfr_zero_p(mpc_imagref(input.number.value))) { \
        expression_clean(&input); \
        return ERROR_NO_ERROR; \
    } \
    mpc_ptr num = malloc(sizeof(mpc_t)); \
    mpc_init2(num, MAX_PRECISION(input.number.value)); \
    mpfr_set_si(mpc_imagref(num), 0, MPFR_RNDN); \
    mpfr_## NAME(mpc_realref(num), mpc_realref(input.number.value), MPC_RNDNN); \
    *out = expression_new_number(num); \
    expression_clean(&input); \
    return ERROR_NO_ERROR; \
}

#define _DEFINE_CONST(NAME) \
error_t builtin_const_ ## NAME(scope_t* scope, expression_t** out) { \
    (void)scope; \
    mpc_ptr num = malloc(sizeof(mpc_t)); \
    mpc_init2(num, 256); \
    simplify_const_## NAME(num); \
    *out = expression_new_number(num); \
    return ERROR_NO_ERROR; \
}

#define _EXPORT_BUILTIN_FUNC(SCOPE, NAME) \
    scope_define_internal_function(SCOPE, #NAME, builtin_func_ ## NAME, 1, "__arg0")

#define _EXPORT_BUILTIN_CONST(SCOPE, NAME) scope_define_internal_const(SCOPE, #NAME, builtin_const_ ## NAME)

_DEFINE_MPFR_BUILTIN(cos)
_DEFINE_MPFR_BUILTIN(sin)
_DEFINE_MPFR_BUILTIN(tan)
_DEFINE_MPFR_BUILTIN(acos)
_DEFINE_MPFR_BUILTIN(asin)
_DEFINE_MPFR_BUILTIN(atan)
_DEFINE_MPFR_BUILTIN(sec)
_DEFINE_MPFR_BUILTIN(csc)
_DEFINE_MPFR_BUILTIN(cot)
_DEFINE_MPFR_BUILTIN(cosh)
_DEFINE_MPFR_BUILTIN(sinh)
_DEFINE_MPFR_BUILTIN(tanh)
_DEFINE_MPFR_BUILTIN(acosh)
_DEFINE_MPFR_BUILTIN(asinh)
_DEFINE_MPFR_BUILTIN(atanh)
_DEFINE_MPFR_BUILTIN(sech)
_DEFINE_MPFR_BUILTIN(csch)
_DEFINE_MPFR_BUILTIN(coth)

_DEFINE_CONST(e)
_DEFINE_CONST(i)
_DEFINE_CONST(euler)
_DEFINE_CONST(catalan)
_DEFINE_CONST(pi)

error_t builtin_func_random_imaginary(scope_t* scope, expression_t** out) {
    (void)scope;

    if (!_g_rand_state_initialized) {
        _g_rand_state_initialized = true;
        gmp_randinit_default(_g_rand_state);
        gmp_randseed_ui(_g_rand_state, (unsigned long)time(NULL));
    }

    mpc_ptr num = malloc(sizeof(mpc_t));
    mpc_init2(num, 256);

    mpc_urandom(num, _g_rand_state);
    *out = expression_new_number(num);

    return ERROR_NO_ERROR;
}


error_t builtin_func_random(scope_t* scope, expression_t** out) {
    (void)scope;

    if (!_g_rand_state_initialized) {
        _g_rand_state_initialized = true;
        gmp_randinit_default(_g_rand_state);
        gmp_randseed_ui(_g_rand_state, (unsigned long)time(NULL));
    }

    mpc_ptr num = malloc(sizeof(mpc_t));
    mpc_init2(num, 256);
    mpfr_set_si(mpc_imagref(num), 0, MPFR_RNDN);
    mpfr_urandom(mpc_realref(num), _g_rand_state, MPFR_RNDN);
    *out = expression_new_number(num);

    return ERROR_NO_ERROR;
}


error_t builtin_func_ln(scope_t* scope, expression_t** out) {
    expression_t input;
    scope_get_value(scope, "__arg0", &input);
    if (!EXPRESSION_IS_NUMBER(&input)) {
        return ERROR_NO_ERROR;
        expression_clean(&input);
    }
    mpc_ptr num = malloc(sizeof(mpc_t));
    mpc_init2(num, 256);
    mpc_log(num, input.number.value, MPC_RNDNN);
    *out = expression_new_number(num);
    mpc_clear(num);
    expression_clean(&input);
    return ERROR_NO_ERROR;
}

error_t builtin_func_log(scope_t* scope, expression_t** out) {
    error_t err;
    expression_t* b = malloc(sizeof(expression_t));
    expression_t* y = malloc(sizeof(expression_t));

    scope_get_value(scope, "__arg0", b);
    scope_get_value(scope, "__arg1", y);

    err = expression_do_logarithm(b, y, out);
    if (err) return err;

    return expression_evaluate(*out, scope);
}

void simplify_export_builtins(scope_t* scope) {
    _EXPORT_BUILTIN_FUNC(scope, cos);
    _EXPORT_BUILTIN_FUNC(scope, sin);
    _EXPORT_BUILTIN_FUNC(scope, tan);
    _EXPORT_BUILTIN_FUNC(scope, acos);
    _EXPORT_BUILTIN_FUNC(scope, asin);
    _EXPORT_BUILTIN_FUNC(scope, atan);
    _EXPORT_BUILTIN_FUNC(scope, sec);
    _EXPORT_BUILTIN_FUNC(scope, csc);
    _EXPORT_BUILTIN_FUNC(scope, cot);
    _EXPORT_BUILTIN_FUNC(scope, cosh);
    _EXPORT_BUILTIN_FUNC(scope, sinh);
    _EXPORT_BUILTIN_FUNC(scope, tanh);
    _EXPORT_BUILTIN_FUNC(scope, acosh);
    _EXPORT_BUILTIN_FUNC(scope, asinh);
    _EXPORT_BUILTIN_FUNC(scope, atanh);
    _EXPORT_BUILTIN_FUNC(scope, sech);
    _EXPORT_BUILTIN_FUNC(scope, csch);
    _EXPORT_BUILTIN_FUNC(scope, coth);

    _EXPORT_BUILTIN_FUNC(scope, random);
    _EXPORT_BUILTIN_FUNC(scope, random_imaginary);
    _EXPORT_BUILTIN_FUNC(scope, log);

    _EXPORT_BUILTIN_CONST(scope, pi);
    _EXPORT_BUILTIN_CONST(scope, i);
    _EXPORT_BUILTIN_CONST(scope, e);
    _EXPORT_BUILTIN_CONST(scope, euler);
    _EXPORT_BUILTIN_CONST(scope, catalan);
}