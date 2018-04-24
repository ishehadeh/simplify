simplify(1) -- rewrite a mathematical expression in it's simplest terms
========================================================================

## SYNOPSIS

`simplify` [__-q__] [__-v__] [__-f__ _FILE_] [__-i__ _VARIABLE_] [__-d__ _VARIABLE_=_EXPR_] ...EXPRESSION

## DESCIPTION

Simplify takes a series of expression and tries to reduce them to their simplest form.
Simplify may just evaluate the expression, if that is possible, for example, `2 + 2` will be
`simplify`'d to `4`.

Simplify can also work with unknowns. By default, it will try to evaluate as much as possible around any unknowns.
Simplify can also solve for an unknown, using the `-i` flag. `-i` will try to isolate that variable on one side of
an equality operator (`:`, `=`, `<`, or `>`), it appends `= 0` to the equation if no equality operator is present.
For more information on how to write `simplify` expressions see simplify(7)

## OPTIONS

* `-q`, `--quiet`:
   Only print errors, ignore all the expression's results

* `-v`, `--verbose`:
   Print status updates while running

* `-f`, `--file`=[__FILE__]:
   Evaluate `FILE` before any other expressions.

* `-i`, `--isolate`=[__VARIABLE__]:
   Try to isolate __VARIABLE__ on one side of an equality operator.

* `-d`, `--define`=[__VARIABLE__=__EXPRESSION__]:
   Define __VARIABLE__ as __EXPRESSION__, do try to evaluate either side simplify

## SEE ALSO

simplify(7)