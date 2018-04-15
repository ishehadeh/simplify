/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_SCALAR_MPZ_H_
#define SIMPLIFY_SCALAR_MPZ_H_

#include <gmp.h>

typedef mpz_t scalar_t;

#define SCALAR_FEATURES                                                                      \
(                                                                                            \
    SCALAR_FEATURE_BIGNUM   | SCALAR_FEATURE_ROOT_N           | SCALAR_FEATURE_FROM_DOUBLE | \
    SCALAR_FEATURE_FROM_INT | SCALAR_FEATURE_BASIC_ARITHMETIC | SCALAR_FEATURE_REFERENCE     \
)

#define SCALAR_INFO_STRING \
"scalar_t is a integral bignum type, `mpz', from GNU MP v" #__GNU_MP_VERSION

/* Basic Arithmetic */
#define SCALAR_ADD(X, Y, Z)      mpz_add((Z), (X), (Y))
#define SCALAR_SUB(X, Y, Z)      mpz_sub((Z), (X), (Y))
#define SCALAR_MUL(X, Y, Z)      mpz_mul((Z), (X), (Y))
#define SCALAR_DIV(X, Y, Z)      mpz_div((Z), (X), (Y))
#define SCALAR_NEGATE(X, Z)      mpz_neg(Z, X)

/* Conversions */
#define SCALAR_FROM_STRING(X, Z) mpz_set_str((Z), (X), 10)
#define SCALAR_TO_STRING(X, Z)   mpz_get_str(Z, X, 10)
#define SCALAR_REQUIRED_CHARS(X) (mpz_sizeinbase ((X), 10) + 1)
#define SCALAR_SET_INT(X, Z)     mpz_set_si(Z, X)
#define SCALAR_SET_DOUBLE(X, Z)  mpz_set_d(Z, X)

/* Radicals & Exponents */
#define SCALAR_POW(X, Y, Z)      mpz_pow((Z), (X), (Y))
#define SCALAR_ROOT(X, Y, Z)     mpz_root((Z), (X), (Y))

/* Construction, Deconstruction & Assignment */
#define SCALAR_INIT(X)           mpz_init(X)
#define SCALAR_COPY(X, Z)        mpz_set((Z), (X))
#define SCALAR_CLEAN(X)          mpz_clear(X)
#define SCALAR_SWAP(X, Z)        mpz_swap(Z, X)

#define SCALAR_REQUIRED_CHARS(X) (mpz_sizeinbase ((X), 10) + 1)
#define SCALAR_COMPARE(X, Y)     mpz_cmp(X, Y)

#endif  // SIMPLIFY_SCALAR_MPZ_H_
