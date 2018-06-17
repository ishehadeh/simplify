
#ifndef SIMPLIFY_MATH_MATH_H_
#define SIMPLIFY_MATH_MATH_H_

#include <mpc.h>
#include <stdlib.h>

#define SIMPLIFY_DEFAULT_PRECISION ((mp_prec_t)0)

/* Init several variables with precision PREC */
#define INITS(PREC, ...) __math_multi_init(PREC, (mpc_ptr[]){__VA_ARGS__, NULL});

/* Clear several variables (this doesn't free them if they were allocated with simplify_new_number) */
#define CLEARS(...) __math_multi_clean((mpc_ptr[]){__VA_ARGS__, NULL})

/* get the maximum precision in a list of mpc_ptrs,
 each mpfr_t in the mpc_t struct is counted individually */
#define GET_MAX_PREC(...) __math_max_prec((mpc_ptr[]){__VA_ARGS__, NULL})

static inline void __math_multi_init(mp_prec_t prec, mpc_ptr* inits) {
    prec = prec > SIMPLIFY_DEFAULT_PRECISION ? prec : SIMPLIFY_DEFAULT_PRECISION;
    while (*inits != NULL) {
        mpc_init2(*inits, prec);
        ++inits;
    }
}

static inline void __math_multi_clean(mpc_ptr* nums) {
    while (*nums != NULL) {
        mpc_clear(*nums);
        ++nums;
    }
}

static inline mp_prec_t __math_max_prec(mpc_ptr* nums) {
    mp_prec_t real;
    mp_prec_t imag;
    mp_prec_t max = 0;
    while (*nums != NULL) {
        mpc_get_prec2(&real, &imag, *nums);
        if (real > max) {
            max = real;
        }
        if (imag > max) {
            max = imag;
        }
        ++nums;
    }
    return max;
}

mpc_ptr simplify_new_number(mp_prec_t);
mp_prec_t simplify_get_default_precision();
void simplify_set_default_precision(mp_prec_t);

void simplify_const_e(mpc_ptr);
void simplify_const_pi(mpc_ptr);
void simplify_const_catalan(mpc_ptr);
void simplify_const_euler(mpc_ptr);
void simplify_const_i(mpc_ptr);

#endif  // SIMPLIFY_MATH_MATH_H_
