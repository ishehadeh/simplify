# Simplify

Simplify evaluates a mathematician expression, putting it in it's simplest terms.

For example:

`simplify --isolate x '2(x + 2) = 5'`\
`x = 0.5`

Simplify also works with multiple variables:

`simplify --isolate y '2 + x * y^4 = 10`\
`y = 8 / x \ 4`.

Variables can also be assigned in an expression:

`simplify 'x : 2' 'x - 5'`\
`2`\
`-3`

## Building

Simplify depends on [GMP](https://www.gmplib.org) and [MPFR](https://www.mpfr.org)

1. `mkdir build; cd build`
2. `cmake ..`
3. `cmake --build .`

## Getting Started

The simplify CLI takes a series of expressions, it tries to shorten each one as much as possible,
assigns any variables or functions it can, then prints the result.

To demonstrate lets do a few unit conversions.

Define a series of variables, `B: 1`, `Kb: B * 1024`, `Mb: Kb * 1024` `Gb: Mb * 1024`.
Using these variables lets try converting 20 Megabytes into bytes: this could be written as `x = 20 * Mb` or `x = 20Mb`.

Lets try a more complicated expression. We have a 100Kb file, on a 120Gb disk.
How much of the disk is left in Megabytes? `x = (120Gb - 100Kib) / Mb`

assuming your in the build directory created in __Building__, the command to execute
this may look a bit like `./simplify 'B: 1' 'Kb: B * 1024' 'Mb: Kb * 1024' 'Gb: Mb * 1024' 'x = (120Gb - 100Kb) / Mb'`

For more information on the CLI see [simplify.1.md](docs/simplify.1.md).
For more information on the expression syntax see [simplify.7.md](docs/simplify.7.md)

## Testing

_To run the tests ctest must be in your PATH_\
Follow build steps `1` and `2`, then run `make test`

## Regenerate Documentation

_To generate documentation cldoc and ronn must be in your PATH_\
Similar to making the tests do build steps `1` and `2`, but run `make docs` instead of `make test`.