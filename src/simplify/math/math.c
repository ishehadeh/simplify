#include "simplify/math/math.h"

mp_prec_t _g_simplify_default_precision = 256;

mp_prec_t simplify_get_default_precision() {
    return _g_simplify_default_precision;
}

void simplify_set_default_precision(mp_prec_t prec) {
    _g_simplify_default_precision = prec;
}

mpc_ptr simplify_new_number(mp_prec_t prec) {
    mpc_ptr x = (mpc_ptr)malloc(sizeof(mpc_t));
    mpc_init2(x, prec > SIMPLIFY_DEFAULT_PRECISION ? prec : simplify_get_default_precision() );
    return x;
}

void simplify_const_e(mpc_ptr x) {
    mpc_t one;
    mpc_init2(one, 8);
    mpc_set_si(one, 1, MPC_RNDNN);
    mpc_exp(x, one, MPC_RNDNN);
    mpc_clear(one);
}

void simplify_const_pi(mpc_ptr x) {
    mpc_set_si_si(x, 0, 0, MPC_RNDNN);
    mpfr_const_pi(mpc_realref(x), MPFR_RNDN);
}

void simplify_const_catalan(mpc_ptr x) {
    mpc_set_si_si(x, 0, 0, MPC_RNDNN);
    mpfr_const_catalan(mpc_realref(x), MPFR_RNDN);
}

void simplify_const_euler(mpc_ptr x) {
    mpc_set_si_si(x, 0, 0, MPC_RNDNN);
    mpfr_const_euler(mpc_realref(x), MPFR_RNDN);
}

void simplify_const_i(mpc_ptr x) {
    mpc_set_si_si(x, 0, 1, MPC_RNDNN);
}