// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_SCALAR_BUILTINS_H_
#define SIMPLIFY_SCALAR_BUILTINS_H_

#include <math.h>
#include <gmp.h>

#if defined(SCALAR_INTEGER)
#   define SCALAR_FEATURES                                             \
    (                                                                  \
        SCALAR_FEATURE_FROM_DOUBLE | SCALAR_FEATURE_ROOT_N           | \
        SCALAR_FEATURE_FROM_INT    | SCALAR_FEATURE_BASIC_ARITHMETIC   \
    )

#   define SCALAR_INFO_STRING \
    "scalar_t is a integral fixed-size number, `long', from the C Language Standard v" __STDC_VERSION__

    typedef long scalar_t;

#   define SCALAR_POW(X, Y, Z) ((Z) = powl(X, Y))
#   define SCALAR_TO_STRING(X, Z)   ltoa(X, Z)
#   define SCALAR_COMPARE(X, Y)     (X > Y ? 1 : X < Y ? -1 : 0)
#   define SCALAR_FROM_STRING(X, Z) ((Z) = strtol(X, NULL, 10), 0)

#elif defined(SCALAR_FLOAT)
#   define SCALAR_FEATURES                                                                                     \
    (                                                                                                          \
        SCALAR_FEATURE_FROM_DOUBLE | SCALAR_FEATURE_ROOT_N           | SCALAR_FEATURE_FLOATING_POINT         | \
        SCALAR_FEATURE_FROM_INT    | SCALAR_FEATURE_BASIC_ARITHMETIC | SCALAR_FEATURE_FLOATING_POINT_EXPONENT  \
    )

#   define SCALAR_INFO_STRING \
    "scalar_t is a floating-point fixed-size number, `double', from the C Language Standard v" __STDC_VERSION__

    typedef double scalar_t;

#   define SCALAR_FROM_STRING(X, Z) ((Z) = strtod(X, NULL), 0)
#   define SCALAR_POW(X, Y, Z)      ((Z) = pow(X, Y))
#   define SCALAR_TO_STRING(X, Z)   dtoa(X, Z)
#   define SCALAR_REQUIRED_CHARS(X) ((X) == 0 ? 3 : 64 + FLOAT_PRECISION)
#   define SCALAR_COMPARE(X, Y)     ((long)((X) * FLOAT_PRECISION) - (long)((Y) * FLOAT_PRECISION))

#endif

// Basic Arithmetic
// becuase of c's operators these operations are the same for doubles and longs
#define SCALAR_ADD(X, Y, Z) ((Z) = (X) + (Y))
#define SCALAR_SUB(X, Y, Z) ((Z) = (X) - (Y))
#define SCALAR_DIV(X, Y, Z) ((Z) = (X) / (Y))
#define SCALAR_MUL(X, Y, Z) ((Z) = (X) * (Y))
#define SCALAR_NEGATE(X, Z) ((Z) = -(X))

// Initialization, cleanup
// Both longs and doubles can only get so big, so they are always stack allocated
#define SCALAR_INIT(Z) ((Z) = 0)
#define SCALAR_CLEAN(X)
#define SCALAR_SET(X, Z) ((Z) = (X))
#define SCALAR_SWAP(X, Z) scalar_t __Z = (Z); (Z) = (X), (X) = __Z

// Casting
#define SCALAR_SET_INT(X, Z)    SCALAR_SET((scalar_t)X, Z)
#define SCALAR_SET_DOUBLE(X, Z) SCALAR_SET((scalar_t)X, Z)

// Fulfill the ROOT_N feature
#define SCALAR_ROOT(X, Y, Z) ((Z) = __scalar_root(X, Y))

static inline scalar_t __scalar_root(scalar_t x, scalar_t y) {
    if (x == 0) {
        return 1;
    } else if (x == 1) {
        return x;
    } else if (x == 2) {
        return (scalar_t)sqrt((double)x);
    } else if (x == 3) {
        return (scalar_t)cbrt((double)x);
    }

    return (scalar_t)pow((double)x, 1.0/(double)y);
}

#endif  // SIMPLIFY_SCALAR_BUILTINS_H_
