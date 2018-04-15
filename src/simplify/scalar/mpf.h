// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_SCALAR_MPF_H_
#define SIMPLIFY_SCALAR_MPF_H_

#include <math.h>
#include <gmp.h>

#define SCALAR_FEATURES                                                                                \
(                                                                                                      \
    SCALAR_FEATURE_BIGNUM         | SCALAR_FEATURE_MULTIPRECISION   | SCALAR_FEATURE_FROM_DOUBLE     | \
    SCALAR_FEATURE_FROM_INT       | SCALAR_FEATURE_BASIC_ARITHMETIC | SCALAR_FEATURE_REFERENCE       | \
    SCALAR_FEATURE_FLOATING_POINT | SCALAR_FEATURE_SQUARE_ROOT      | SCALAR_FEATURE_INTEGER_COMPARE   \
)

#define SCALAR_INFO_STRING \
"scalar_t is a floating point, multi-precision bignum, `mpf', from GNU MP v" #__GNU_MP_VERSION

typedef mpf_t scalar_t;


// Basic Arithmetic
#define SCALAR_ADD(X, Y, Z)      mpf_add((Z), (X), (Y))
#define SCALAR_SUB(X, Y, Z)      mpf_sub((Z), (X), (Y))
#define SCALAR_MUL(X, Y, Z)      mpf_mul((Z), (X), (Y))
#define SCALAR_DIV(X, Y, Z)      mpf_div((Z), (X), (Y))
#define SCALAR_NEGATE(X, Z)      mpf_neg(Z, X)

// Initialization, deconstruction, and setting
#define SCALAR_INIT(X)           mpf_init2(X, FLOAT_PRECISION)
#define SCALAR_CLEAN(X)          mpf_clear(X)
#define SCALAR_SET(X, Z)         mpf_set((Z), (X))
#define SCALAR_SWAP(X, Z)        mpf_swap(Z, X)

// Compare
#define SCALAR_COMPARE(X, Y)     mpf_cmp(X, Y)
#define SCALAR_COMPARE_INT(X, Y) mpf_cmp_ui(X, Y)

// Radicals & Exponents
#define SCALAR_POW(X, Y, Z)          _scalar_pow(X, Y, Z)
#define SCALAR_SQUARE_ROOT(X, Y, Z)  mpf_sqrt((Z), (X))

// Casting
#define SCALAR_REQUIRED_CHARS(X) _scalar_get_size(X)
#define SCALAR_TO_STRING(X, Z)   _scalar_to_string(X, Z)
#define SCALAR_FROM_STRING(X, Z) mpf_set_str((Z), (X), 10)
#define SCALAR_SET_INT(X, Z)     mpf_init_set_si(Z, X)
#define SCALAR_SET_DOUBLE(X, Z)  mpf_init_set_d(Z, X)


static inline void _scalar_pow(scalar_t x, scalar_t y, scalar_t z) {
    mpf_pow_ui(z, x, mpf_get_ui(y));

    if (SCALAR_COMPARE_INT(y, 0) < 0) {
        mpf_ui_div(z, 1, z);
    }
}

static inline char* _scalar_to_string(scalar_t val, char* buf) {
    gmp_sprintf(buf, "%.*Ff", FLOAT_PRECISION, val);

    char* _approximate_number(char* str, int min_reps);

    return _approximate_number(buf, 5);
}


static inline int _scalar_get_size(scalar_t val) {
    if  (mpf_fits_uint_p(val)) {
        int i = mpf_get_ui(val);
        return i == 0 ? FLOAT_PRECISION + 2 : floor(log10(i)) + FLOAT_PRECISION + 2;
    } else {
        mpz_t tmp;
        mpz_init(tmp);
        mpz_set_f(tmp, val);
        int retval = mpz_sizeinbase(tmp, 10);
        mpz_clear(tmp);
        return retval + FLOAT_PRECISION + 2;
    }
}

#endif  // SIMPLIFY_SCALAR_MPF_H_
