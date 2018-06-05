
#ifndef SIMPLIFY_MATH_MATH_H_
#define SIMPLIFY_MATH_MATH_H_

#include <mpc.h>

static inline void _multi_init(mp_prec_t prec, mpc_ptr* inits) {
    while (*inits != NULL) {
        mpc_init2(*inits, prec);
        ++inits;
    }
}

static inline void _multi_clean(mpc_ptr* nums) {
    while (*nums != NULL) {
        mpc_clear(*nums);
        ++nums;
    }
}

static inline mp_prec_t _max_prec(mpc_ptr* nums) {
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

#define DECLARE_NUMBERS(PREC, ...)                 \
mpc_t __VA_ARGS__;                                 \
_multi_init(PREC, (mpc_ptr[]){__VA_ARGS__, NULL}); \

#define CLEAN(...) _multi_clean((mpc_ptr[]){__VA_ARGS__, NULL})

#define MAX_PRECISION(...) _max_prec((mpc_ptr[]){__VA_ARGS__, NULL})

void simplify_const_e(mpc_ptr);
void simplify_const_pi(mpc_ptr);
void simplify_const_catalan(mpc_ptr);
void simplify_const_euler(mpc_ptr);
void simplify_const_i(mpc_ptr);

#endif  // SIMPLIFY_MATH_MATH_H_
