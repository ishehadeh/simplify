// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_SCALAR_SCALAR_H_
#define SIMPLIFY_SCALAR_SCALAR_H_


static char* __g_nan_string = "NAN";
static char* __g_inf_string = "INF";


// Make the default float precision 32
#if !defined(FLOAT_PRECISION)
#   define FLOAT_PRECISION 32
#endif

// Make the default scalar type a float
#if !defined(SCALAR_FLOAT) && !defined(SCALAR_INTEGER)
#   define SCALAR_FLOAT 1
#endif

#define SCALAR_FEATURE_MULTIPRECISION           1
#define SCALAR_FEATURE_FLOATING_POINT           2
#define SCALAR_FEATURE_ROOT_N                   4
#define SCALAR_FEATURE_CUBE_ROOT                8
#define SCALAR_FEATURE_SQAURE_ROOT              16
#define SCALAR_FEATURE_FLOATING_POINT_EXPONENT  32
#define SCALAR_FEATURE_FROM_INT                 64
#define SCALAR_FEATURE_FROM_DOUBLE              128
#define SCALAR_FEATURE_BASIC_ARITHMETIC         256
#define SCALAR_FEATURE_REFERENCE                512
#define SCALAR_FEATURE_BIGNUM                   1024
#define SCALAR_FEATURE_INTEGER_COMPARE          2048

char* dtoa(double, char*);
char* ltoa(long, char*);

#if defined(HAVE_GMP)
#   if defined(SCALAR_INTEGER)
#       include "simplify/scalar/mpz.h"
#   elif defined(SCALAR_FLOAT)
#       if defined(HAVE_MPFR)
#           include "simplify/scalar/mpfr.h"
#       else
#           include "simplify/scalar/mpf.h"
#       endif
#   endif
#else
#   include "simplify/scalar/builtins.h"
#endif



#if !(SCALAR_FEATURE_INTEGER_COMPARE & SCALAR_FEATURES)
    static inline int __scalar_compare_int(scalar_t x, int y) {
        scalar_t sy;
        SCALAR_SET_INT(y, sy);
        int z = SCALAR_COMPARE(x, sy);
        SCALAR_CLEAN(sy);
        return z;
    }

    #define SCALAR_COMPARE_INT(X, Y) __scalar_compare_int(X, Y)
#endif


#endif  // SIMPLIFY_SCALAR_SCALAR_H_
