/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_STRINGIFY_H_
#define SIMPLIFY_EXPRESSION_STRINGIFY_H_

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "simplify/expression/expr_types.h"

/* convert a number to a string
 * @number the number to convert
 * @buf the buffer that the number should be stored in
 * @len the max length of the number, 0 for no maximum
 * @return an error code
 */
error_t number_to_buffer(mpfr_t number,    char* buf, size_t len);

/* convert an operator to a string
 * @op the operator to convert
 * @buf the buffer that the operator should be stored in
 * @len the max length of the operator, 0 for no maximum
 * @return an error code
 */
error_t operator_to_buffer(operator_t op,  char* buf, size_t len);


/* convert a variable to a string
 * @var the variable to convert
 * @buf the buffer that the variable should be stored in
 * @len the max length of the variable, 0 for no maximum
 * @return an error code
 */
error_t variable_to_buffer(variable_t var, char* buf, size_t len);

/* try to trim a floating point number that was convert to a string, to improve accuracy
 * @str the string to edit
 * @tolerance how easily the string should be rounded off. (five is probably a good default for this parameter)
 * @return an error code
 */
error_t approximate_number(char* str, int tolerance);

/* print an expression to `file`
 * @file the file to write to
 * @expr the expression to print
 * @return an error code
 */
error_t expression_fprint(expression_t* expr, FILE* file);

/* print an expression to stdout
 * @expr the expression to print
 * @return an error code
 */
static inline error_t expression_print(expression_t* expr) {
    return expression_fprint(expr, stdout);
}

#endif  // SIMPLIFY_EXPRESSION_STRINGIFY_H_
