# Simplify

Simplify an expression.

## Building

For optimal precision install both __GMP__ and __MPFR__  (see optional dependencies)

1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. `cmake --build .`

## Integer only build

To disable floating point numbers compile with `cmake .. -DONLY_INTEGERS=1`
for build step _2_. In integer only mode numbers with a decimal point will not
throw a parser error, however everything after the decimal will be truncated (including exponents).

### Optional Dependencies

Optional dependencies are automatically detected. To stop this behavior config cmake with
`-DDISABLE_XXX=ON` where XXX is is the dependency's name.

#### GMP

GMP is the _GNU_ milti-precision library, it adds bignum support for both floating point and integral numbers.
If gmp is enabled, the floating point precision can be set with `-DFLOAT_PRECISION=XX`. By default `FLOAT_PRECISION` is 32.

__limitations__: GMP _cannot_ put a number to the power of another floating point number. If this operation is performed the exponent will be rounded.

#### MPFR

MPFR can only be enabled if __GMP__ is also enabled. MPFR makes floating point numbers more precise, with better rounding. It also removes
__GMP__'s limitation on floating-point exponents.

## Tests

At the moment the only tests are in the form of a shell script, `./test/test.sh`. Running
it will compile `simplify` with various features, and run several equations through each build.