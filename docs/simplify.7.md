simplify(7) -- expression format for simplify
=============================================

## SYNSOPSIS

The simplify language is made of 10 builtin operators. It also supports variables and functions.

The opetators, and the operations they can perform:

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

## DESCRIPTION

### Basic Operations

The first 4 operators (`+`, `-`, `*` and `/`) perform basic arithmetic operations.
For the sake of brevity in expressions it's possible to imply a few of these basic operators.

The `(` token can double as a `*` operator. For instance the expression `2(4)` is equivalent to `2 * 4`.
When using `(` for multiplication it still acts as any other occurrence of parentheses would. (see [Parentheses][] for more info),
for instance `2 (4` is invalid, and the expression `2(2 + 2)` would evaluate to `8`, as opposed `6`.
Left parentheses may not be used as a multiplication operator when their left hand operand is a variable, because it may be confused as a
function call (See [Functions][]).

When adjacent to a number variables imply a `*` operator. For instance `2x` is equivalent to `2 * x`.
However when two variables are adjacent, `*` is not implied, becuase it may be confused as another variable name.
For instance `xy` will be read as the variable `xy` not as `x * y`.

### Exponents and Roots

Operators 5 and 6 (`^` and `\`) perform power and root operations.

The `^` operator's left operand is the number to be operated on. The right operand
is the exponent. For instance `2^4` is equivalent to `2 * 2 * 2 * 2`.

The `\` operator's left operand is the number to be operated on. The right operand
is the root. The root __must__ be an integer, it it is not it will be rounded.
For example `27 \ 3.34` is equivalent to `27 \ 3`.

### Equality Operators

Operators 7-9 (`=`, `<`, and `>`) are the comparison operators. They imply
that the user is comparing their left and right operands. Depending on the
how the `simplify` command was invoked they may perform different operations.

By default these operators check check if their left is
_equal_, _less than_, or _greater than_, their right operand, respectively.
If all equality operations in an expression are true, than the expression's result
is `true` otherwise the expression's result is `false`

Alternatively if the `-i` flag was specified with a variable the expression will not
evaluate to `true` or `false`. Instead the variable specified will be isolated on one side
of an equality operator, the expressions result will not change, this operation only makes the
variable's value more clear.

### Assignment

Operator 10 (`:`) is the assignment operator. It assumes it's left operand is a variable or function.
If it's left operand is variable than the right operand is simplified and then assigned to it's left operand.
Otherwise the left operand is immediately assigned to the function named in the right operand.

### Parentheses

Parentheses serve two purposes: to encase a functions parameters, or to indicate an expression should have a higher
priority. The later case is assumed if the parentheses are not immediately preceded by an identifier. If `(` occurs,
than a `)` must follow eventually, otherwise `simplify` throws an error.

### Variables

Variables associate a name with an expression. The name consists of lowercase letters, capital letters, and underscores.
Variables can be assigned to using the `:` operator. The expression being assigned will be simplified before assignment.

### Functions

Functions are similar to variables, the only difference is they take a parameter list when defined or called, and their definition
is not evaluated. Function's argument list is surrounded by parentheses (`(` and `)`). When defining a function every argument must
contain a variable. Arguments are separated by commas (`,`). A function may not have zero arguments.

Function parameters can be defined as an expression. When the function is called the variable in the expression will be isolated
automatically.