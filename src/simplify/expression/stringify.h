/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_STRINGIFY_H_
#define SIMPLIFY_EXPRESSION_STRINGIFY_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simplify/expression/expression.h"
#include "simplify/string/string.h"
#include "simplify/string/format.h"

/* Write the expression `expr` as a c string
 *
 * The `stringify` function stringifies an expression usinging the
 * default stringifier settings.
 * 
 * @expr the expression to stringify
 * @return returns a null-terminated string. This string must be freed using `free` by the user.
 */
char* stringify(expression_t* expr);

void write_expression(string_t* string, string_format_t* format, expression_t* expr);
void expression_fprint(expression_t* expr, FILE* f);

static inline void expression_print(expression_t* expr) {
    expression_fprint(expr, stdout);
}

#endif  // SIMPLIFY_EXPRESSION_STRINGIFY_H_
