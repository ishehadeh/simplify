# The Simplify Language

The simplify language is made of 9 main operators.
Additionally it supports variables and functions.

## Operators

- `+`: addition, or as a prefix.
- `-`: subtraction, or as a prefix.
- `*`: multiplication
- `/`: division
- `^`: exponent
- `\`: root
- `=`: equality
- `<`: less than
- `>`: greater than
- `:`: assignment

__NOTE__: the root operator `\` takes it's degree on the right hand side, contrary to how radicals are written in most expressions.

## Variables

Variables are made up of any combination of letters and underscores (`a-z`, `A-Z`, and `_`).
They may be assigned to with the `:` operator. A variable's value may not be a number, it can
also be an expression. e.g. if evaluated, `x : y + 2` would set `x` equal to `y + 2`.

## Functions

Functions are very similar to variables, the only difference is they take an argument list immediately after their name.
The argument list is written inside of parentheses (`(` and `)`). Individual arguments are separated by commas (`,`).
When declaring a function (if it is on the left side of the `:` operator) all arguments must name a variable. Functions can be
invoked later.