/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_ISOLATE_H_
#define SIMPLIFY_EXPRESSION_ISOLATE_H_

#include "simplify/expression/expression.h"

/* isolate a variable on one side of an comparison operator.
 * If no comparison operator is present, append `= 0`
 * 
 * @expr the expression to work with
 * @var the variable to isolate
 * @return returns an error code
 */
error_t expression_isolate_variable(expression_t* expr, variable_t var);

#endif  // SIMPLIFY_EXPRESSION_ISOLATE_H_
