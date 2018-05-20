/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_SIMPLIFY_H_
#define SIMPLIFY_EXPRESSION_SIMPLIFY_H_

#include "simplify/expression/expression.h"

/* try to make `expr` as short as possible by combining child expressions
 *
 * @expr the expression to shorten
 * @return returns an error code
 */
error_t expression_simplify(expression_t* expr);


/* find the value of x in the expression `b^x = y`
 * @b
 * @y
 * @out out will be filled with the value of `x`, or NULL if an error is returned 
 * @returns a error code
 */
error_t expression_do_logarithm(expression_t* b, expression_t* y, expression_t** out);

#endif  // SIMPLIFY_EXPRESSION_SIMPLIFY_H_
