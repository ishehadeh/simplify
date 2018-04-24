simplify(7) -- expression syntax for simplify
=============================================

## SYNOPSIS

The simplify expression syntax contains 10 built-in operators.
Simplify also supports variable's and functions, so expressions can be reused.

### Operator List

1. `+` addition, or as a prefix to make a number positive.
2. `-` subtraction, or as a prefix to make a number negative.
3. `*` multiplication
4. `/` division
5. `^` exponent
6. `\` root
7. `=` equality
8. `<` less than
9. `>` greater than
10. `:` assignment

### Variable syntax:

Variable's are a sequence at least one uppercase letter, lowercase letter, or underscores.
Variables can be assigned to with the `:` operator.

### Function syntax

Function's names are the same as variables. Function's parameters come after the name, surrounded by `(` and `)`.
Individual arguments must be separated by a comma.

## DESCRIPTION

### Basic Operations

The first 4 operators (`+`, `-`, `*` and `/`) perform basic arithmetic operations.
For the sake of brevity in expressions, it's possible to omit them in some cases.

When a number and a variable are adjacent, in any order, a multiplication operator is implied.
For instance `2x` and `x2` are equivalent to `2 * x`. Similarly, when a number is used as a prefix
to left parentheses, the multiplication operator is implied. For example `2(x + 5)` is equivalent to
`2 * (x + 5)`.

### Exponents and Roots

Operators 5 and 6 (`^` and `\`) perform power and root operations.

The `^` operator's left operand is the number to be operated on. The right operand
is the exponent. For instance `2^4` is equivalent to `2 * 2 * 2 * 2`.

The `\` operator's left operand is the number to be operated on. The right operand
is the root. The root __must__ be an integer, if it is not it will be rounded,
For example `27 \ 3.34` is equivalent to `27 \ 3`.

### Equality Operators

Operators 7 to 9 (`=`, `<`, and `>`) are the comparison operators. They imply
that the user is comparing their left and right operands. Depending on the
how the `simplify` command was invoked they may perform different operations.

By default, these operators check if their left operand is
_equal_, _less than_, or _greater than_, their right operand, respectively.
If all equality operations in an expression are true then the expression's result
is `true` otherwise the expression's result is `false`

Alternatively, if the `-i` flag was specified with a variable the expression will not
evaluate to `true` or `false`. Instead the variable specified will be isolated on one side
of an equality operator. Note the value of the expression does not change, this operation only makes the
variable's value more clear.

### Assignment

Operator 10 (`:`) is the assignment operator. It assumes its left operand is a variable or function.
If it's left operand is variable than the right operand is simplified and then assigned to it's left operand.
Otherwise, the left operand is immediately assigned to the function named in the right operand.

### Parentheses

Parentheses serve two purposes: to encase functions parameters or to indicate an expression should have a higher
priority. The latter case is assumed if the parentheses are not immediately preceded by an identifier. If `(` occurs,
then a `)` must follow eventually, otherwise `simplify` throws an error.

### Variables

Variables associate a name with an expression. The name consists of lowercase letters, capital letters, and underscores.
Variables can be assigned to using the `:` operator. The expression being assigned will be simplified before assignment.

### Functions

Functions are similar to variables, the only difference is they take a parameter list when defined or called, and their definition
is not evaluated. Function's argument list is surrounded by parentheses (`(` and `)`). When defining a function every argument must
contain a variable. Arguments are separated by commas (`,`). A function may not have zero arguments.

Function parameters can be defined as an expression. When the function is called the variable in the expression will be isolated
automatically.