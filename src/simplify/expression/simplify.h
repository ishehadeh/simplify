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

#endif  // SIMPLIFY_EXPRESSION_SIMPLIFY_H_
