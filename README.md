# Notice

Simplify was a project I worked on while taking algebra 2 to teach myself the concepts and improve my C skills.
I don't plan on working on it anymore, even though it's buggy and incomplete. 

I'm leaving the repo up and archiving it because I think it will be fun to look back on. :)

# Simplify

[![Build Status](https://travis-ci.org/IanS5/simplify.svg?branch=master)](https://travis-ci.org/IanS5/simplify)
[![Coverage Status](https://coveralls.io/repos/github/IanS5/simplify/badge.svg?branch=master)](https://coveralls.io/github/IanS5/simplify?branch=master)

Simplify evaluates a mathematical expression, putting it in its simplest terms.

For example:

`simplify --isolate x '2(x + 2) = 5'`\
`x = 0.5`

Simplify also works with multiple variables:

`simplify --isolate y '2 + x * y^4 = 10`\
`y = (8 / x) \ 4`.

Variables can also be assigned in an expression:

`simplify 'x : 2' 'x - 5'`\
`2`\
`-3`

For a more detailed explanation check out the [wiki](https://github.com/IanS5/simplify/wiki).

## Building

Simplify depends on [GMP](https://www.gmplib.org) and [MPFR](https://www.mpfr.org)
You can use the `get-deps.sh` script to download them, but most package managers will
have up-to-date prebuilt versions.

1. `./get-deps.sh` _[optional]_
2. `mkdir build; cd build`
3. `cmake ..`
4. `cmake --build .`

## Testing

_To run the tests ctest must be in your PATH_\
Follow build steps `1` and `2`, then run `make test`

## Regenerate Documentation

_To generate documentation cldoc and ronn must be in your PATH_\
Similar to making the tests do build steps `1` and `2`, but run `make docs` instead of `make test`.
