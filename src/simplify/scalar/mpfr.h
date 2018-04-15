/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_SCALAR_MPFR_H_
#define SIMPLIFY_SCALAR_MPFR_H_

#include <math.h>
#include <gmp.h>
#include <mpfr.h>

typedef mpfr_t scalar_t;

#define SCALAR_FEATURES                                                                                        \
(                                                                                                              \
    SCALAR_FEATURE_BIGNUM         | SCALAR_FEATURE_MULTIPRECISION   | SCALAR_FEATURE_FROM_DOUBLE             | \
    SCALAR_FEATURE_FROM_INT       | SCALAR_FEATURE_BASIC_ARITHMETIC | SCALAR_FEATURE_REFERENCE               | \
    SCALAR_FEATURE_FLOATING_POINT | SCALAR_FEATURE_ROOT_N           | SCALAR_FEATURE_FLOATING_POINT_EXPONENT   \
)

#define SCALAR_INFO_STRING \
"scalar_t is a floating point, multi-precision bignum, `mpfr'," \
"from MPFR v" __MPFR_VERSION ", extenidng GNU MP v" __GNU_MP_VERSION

// Basic Arithmetic
#define SCALAR_ADD(X, Y, Z)      mpfr_add((Z), (X), (Y), MPFR_RNDN)
#define SCALAR_SUB(X, Y, Z)      mpfr_sub((Z), (X), (Y), MPFR_RNDN)
#define SCALAR_MUL(X, Y, Z)      mpfr_mul((Z), (X), (Y), MPFR_RNDN)
#define SCALAR_DIV(X, Y, Z)      mpfr_div((Z), (X), (Y), MPFR_RNDN)
#define SCALAR_NEGATE(X, Z)      mpfr_neg(Z, X, MPFR_RNDN)

// Initialization, deconstruction, and setting
#define SCALAR_INIT(X)           mpfr_init2(X, FLOAT_PRECISION)
#define SCALAR_CLEAN(X)          mpfr_clear(X)
#define SCALAR_SET(X, Z)         mpfr_set((Z), (X), MPFR_RNDN)
#define SCALAR_SWAP(X, Z)        mpfr_swap(Z, X)

// Compare
#define SCALAR_COMPARE(X, Y)     mpfr_cmp(X, Y)

// Radicals & Exponents
#define SCALAR_POW(X, Y, Z)      mpfr_pow((Z), (X), (Y), MPFR_RNDN)
#define SCALAR_ROOT(X, Y, Z)     mpfr_rootn_ui((Z), (X), mpfr_get_ui(Y, MPFR_RNDN), MPFR_RNDN)

// Casting
#define SCALAR_REQUIRED_CHARS(X) _scalar_get_size(X)
#define SCALAR_TO_STRING(X, Z)   _scalar_to_string(X, Z)
#define SCALAR_FROM_STRING(X, Z) mpfr_set_str((Z), (X), 10, MPFR_RNDN)
#define SCALAR_SET_INT(X, Z)     mpfr_set_si(Z, X, MPFR_RNDN)
#define SCALAR_SET_DOUBLE(X, Z)  mpfr_set_d(Z, X, MPFR_RNDN)



static inline char* _scalar_to_string(scalar_t val, char* buf) {
    mpfr_sprintf(buf, "%.*RNf", FLOAT_PRECISION, val);

    char* _approximate_number(char* str, int min_reps);

    return _approximate_number(buf, 5);
}

static inline int _scalar_get_size(scalar_t val) {
    if  (mpfr_fits_uint_p(val, MPFR_RNDN)) {
        int i = mpfr_get_ui(val, MPFR_RNDN);
        return i == 0 ? FLOAT_PRECISION + 3 : floor(log10(i)) + FLOAT_PRECISION + 2;
    } else {
        mpz_t tmp;
        mpz_init(tmp);
        mpfr_get_z(tmp, val, MPFR_RNDN);
        int retval = mpz_sizeinbase(tmp, 10);
        mpz_clear(tmp);
        return retval + FLOAT_PRECISION + 2;
    }
}


#endif  // SIMPLIFY_SCALAR_MPFR_H_
