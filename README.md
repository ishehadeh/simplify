# Simplify

Simplify evaluates a mathematician expression, putting it in it's simplest terms.

For example:

`simplify --isolate x '2(x + 2) = 5'` results in `x = 0.5`

Simplify also works with multiple variables, for example:

`simplify --isolate y '2 + x * y^4 = 10` results in `y = 8 / x \ 4`.

## Building

Simplify depends on [GMP](https://www.gmplib.org) and [MPFR](https://www.mpfr.org)

1. `mkdir build; cd build`
2. `cmake ..`
3. `cmake --build .`

## Testing

_To run the tests ctest must be in your PATH_\
Follow build steps `1` and `2`, then run `make test`

## Regenerate Documentation

_To generate documentation cldoc must be in your PATH_\
Similar to making the tests do build steps `1` and `2`, but run `make doc` instead of `make test`.