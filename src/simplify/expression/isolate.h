/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_ISOLATE_H_
#define SIMPLIFY_EXPRESSION_ISOLATE_H_

#include "simplify/expression/expression.h"

// Expressions like `e ^ X = Y` may be replaced by the NUTURAL_LOG_BUILTIN(Y)
// #define NATURAL_LOG_BUILTIN "ln"

#define E_BUILTIN "e"

/* isolate a variable on one side of an comparison operator.
 * If no comparison operator is present, append `= 0`
 * 
 * @expr the expression to work with
 * @var the variable to isolate
 * @return returns an error code
 */
error_t expression_isolate_variable(expression_t* expr, variable_t var);


#endif  // SIMPLIFY_EXPRESSION_ISOLATE_H_
