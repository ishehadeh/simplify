// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_SCALAR_H_
#define SIMPLIFY_SCALAR_H_

// Input Macros:
// - HAVE_GMP: if defined we attempt to import `gmp.h`
// - HAVE_MPFR: use mpfr type for floating point values
// - SCALAR_INTEGER: Use a integral numeric type for `scalar_t`
// - SCALAR_FLOAT: Use a floating point numeric type for `scalar_t`

// Types:
// - scalar_t: a single numeric value

// Macros:
// - SCALAR_DEFINE(NAME): define a scalar variable `NAME`
// - SCALAR_ADD(X, Y, Z): Set Z to X + Y
// - SCALAR_SUB(X, Y, Z): Set Z to X - Y
// - SCALAR_MUL(X, Y, Z): Set Z to X * Y
// - SCALAR_DIV(X, Y, Z): Set Z to X / Y
// - SCALAR_POW(X, Y, Z): Set Z to X to the power of Y
// - SCALAR_NEGATE(X, Z): Set Z to -X
// - SCALAR_EQ(X, Y): Check X and Y for equality
// - SCALAR_FROM_STRING(X, Z): Convert X to a scalar value stored in Z, assuming X is a string
// - SCALAR_TO_STRING(X, Z): Convert scalar X to a string and store it in string buffer Z
// - SCALAR_REQUIRED_CHARS(X): how many chars it would take to represent X as a string (not counting a  null terminator)

#include <math.h>
#include <stdlib.h>

#if !defined(SCALAR_INTEGER) && !defined(SCALAR_FLOAT)
#   define SCALAR_FLOAT 1
#endif

#if !defined(FLOAT_PRECISION)
#   define FLOAT_PRECISION 47
#endif

#if !defined(HAVE_GMP)

#   define _DOUBLE_PRECISION .00000000000000000000000000000000000000000000001

    char* __ltoa(long, char*);
    char* __dtoa(double, char*);
#endif
char* _approximate_number(char* str, int min_reps);



#if defined(HAVE_GMP)
#   include "gmp.h"
#   if defined(SCALAR_INTEGER)
        typedef mpz_t scalar_t;
#       define SCALAR_ADD(X, Y, Z)      mpz_add((Z), (X), (Y))
#       define SCALAR_SUB(X, Y, Z)      mpz_sub((Z), (X), (Y))
#       define SCALAR_MUL(X, Y, Z)      mpz_mul((Z), (X), (Y))
#       define SCALAR_DIV(X, Y, Z)      mpz_div((Z), (X), (Y))
#       define SCALAR_PRINT(X)          mpz_out_str(NULL, 10, (X))
#       define SCALAR_NEGATE(X, Z)      mpz_neg(Z, X)
#       define SCALAR_FROM_STRING(X, Z) mpz_init(Z), mpz_set_str((Z), (X), 10)
#       define SCALAR_POW(X, Y, Z)      mpz_pow_ui((Z), (X), mpz_get_ui(Y))
#       define SCALAR_SET(X, Z)         mpz_init(Z), mpz_set((Z), (X))
#       define SCALAR_DEFINE(X)         scalar_t X; mpz_init(X)
#       define SCALAR_CLEAN(X)          mpz_clear(X)
#       define SCALAR_TO_STRING(X, Z)   mpz_get_str(Z, X, 10)
#       define SCALAR_REQUIRED_CHARS(X) (mpz_sizeinbase ((X), 10) + 1)
#   elif defined(SCALAR_FLOAT)
#       if defined(HAVE_MPFR)
#           include "mpfr.h"
            typedef mpfr_t scalar_t;
            static inline char* _scalar_to_string(scalar_t val, char* buf) {
                mpfr_sprintf(buf, "%.*RNf", FLOAT_PRECISION, val);
                return _approximate_number(buf, 5);
            }
            static inline int _scalar_get_size(scalar_t val) {
                if  (mpfr_fits_uint_p(val, MPFR_RNDN)) {
                    int i = mpfr_get_ui(val, MPFR_RNDN);
                    return i == 0 ? FLOAT_PRECISION + 3 : floor(log10(i)) + FLOAT_PRECISION + 2;
                } else {
                    mpz_t tmp;
                    mpfr_get_z(tmp, val, MPFR_RNDN);
                    int retval = mpz_sizeinbase(tmp, 10);
                    mpz_clear(tmp);
                    return retval + FLOAT_PRECISION + 2;
                }
            }
#           define SCALAR_ADD(X, Y, Z)      mpfr_add((Z), (X), (Y), MPFR_RNDN)
#           define SCALAR_SUB(X, Y, Z)      mpfr_sub((Z), (X), (Y), MPFR_RNDN)
#           define SCALAR_MUL(X, Y, Z)      mpfr_mul((Z), (X), (Y), MPFR_RNDN)
#           define SCALAR_DIV(X, Y, Z)      mpfr_div((Z), (X), (Y), MPFR_RNDN)
#           define SCALAR_PRINT(X)          mpfr_out_str(NULL, 10, 0, (X), MPFR_RNDN)
#           define SCALAR_NEGATE(X, Z)      mpfr_neg(Z, X, MPFR_RNDN)
#           define SCALAR_FROM_STRING(X, Z) mpfr_init_set_str((Z), (X), 10, MPFR_RNDN)
#           define SCALAR_POW(X, Y, Z)      mpfr_pow((Z), (X), (Y), MPFR_RNDN)
#           define SCALAR_SET(X, Z)         mpfr_init_set((Z), (X), MPFR_RNDN)
#           define SCALAR_DEFINE(X)         MPFR_DECL_INIT(X, FLOAT_PRECISION)
#           define SCALAR_CLEAN(X)          mpfr_clear(X)
#           define SCALAR_TO_STRING(X, Z)   _scalar_to_string(X, Z)
#           define SCALAR_REQUIRED_CHARS(X) _scalar_get_size(X)
#       else
            typedef mpf_t scalar_t;
            static inline char* _scalar_to_string(scalar_t val, char* buf) {
                gmp_sprintf(buf, "%.*Ff", FLOAT_PRECISION, val);
                return _approximate_number(buf, 5);
            }
            static inline int _scalar_get_size(scalar_t val) {
                if  (mpf_fits_uint_p(val)) {
                    int i = mpf_get_ui(val);
                    return i == 0 ? FLOAT_PRECISION + 2 : floor(log10(i)) + FLOAT_PRECISION + 2;
                } else {
                    mpz_t tmp;
                    mpz_set_f(tmp, val);
                    int retval = mpz_sizeinbase(tmp, 10);
                    mpz_clear(tmp);
                    return retval;
                }
            }
#           define SCALAR_ADD(X, Y, Z)      mpf_add((Z), (X), (Y))
#           define SCALAR_SUB(X, Y, Z)      mpf_sub((Z), (X), (Y))
#           define SCALAR_MUL(X, Y, Z)      mpf_mul((Z), (X), (Y))
#           define SCALAR_DIV(X, Y, Z)      mpf_div((Z), (X), (Y))
#           define SCALAR_PRINT(X)          mpf_out_str(NULL, 10, 1000, (X))
#           define SCALAR_NEGATE(X, Z)      mpf_neg(Z, X)
#           define SCALAR_FROM_STRING(X, Z) mpf_init_set_str((Z), (X), 10)
#           define SCALAR_POW(X, Y, Z)      mpf_pow_ui((Z), (X), mpf_get_ui(Y))
#           define SCALAR_SET(X, Z)         mpf_init(Z), mpf_set((Z), (X))
#           define SCALAR_DEFINE(X)         scalar_t X; mpf_init2(X, FLOAT_PRECISION)
#           define SCALAR_CLEAN(X)          mpf_clear(X)
#           define SCALAR_TO_STRING(X, Z)   _scalar_to_string(X, Z)
#           define SCALAR_REQUIRED_CHARS(X) _scalar_get_size(X)
#       endif
#   endif
#else
#   define SCALAR_ADD(X, Y, Z) ((Z) = (X) + (Y))
#   define SCALAR_SUB(X, Y, Z) ((Z) = (X) - (Y))
#   define SCALAR_DIV(X, Y, Z) ((Z) = (X) / (Y))
#   define SCALAR_MUL(X, Y, Z) ((Z) = (X) * (Y))
#   define SCALAR_NEGATE(X, Z) ((Z) = -(X))
#   define SCALAR_DEFINE(X) scalar_t X
#   define SCALAR_CLEAN(X)
#   define SCALAR_SET(X, Z) ((Z) = (X))
#   if defined(SCALAR_INTEGER)
        typedef long scalar_t;
#       define SCALAR_FROM_STRING(X, Z) (_scalar_from_str(X, &(Z)))
#       define SCALAR_POW(X, Y, Z) ((Z) = powl(X, Y))
#       define SCALAR_TO_STRING(X, Z)   __ltoa(X, Z)
#       define SCALAR_REQUIRED_CHARS(X) ( (X) == 0 ? 1 : floor(log10(abs(X))) + 1)
#   elif defined(SCALAR_FLOAT)
        typedef double scalar_t;
#       define SCALAR_FROM_STRING(X, Z) (_scalar_from_str(X, &(Z)))
#       define SCALAR_POW(X, Y, Z)      ((Z) = pow(X, Y))
#       define SCALAR_TO_STRING(X, Z)   __dtoa(X, Z), _approximate_number(Z, 5)
#       define SCALAR_REQUIRED_CHARS(X) ((X) == 0 ? 1 : 18 + FLOAT_PRECISION)
#   endif

static inline int _scalar_from_str(char* str, scalar_t* out) {
    char* end;
#   if defined(SCALAR_INTEGER)
    *out = strtol(str, &end, 10);
#   elif defined(SCALAR_FLOAT)
    *out = strtod(str, &end);
#endif
    return end == str;
}
#endif

#endif  // SIMPLIFY_SCALAR_H_
