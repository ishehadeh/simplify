/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EVALUATE_H_
#define SIMPLIFY_EXPRESSION_EVALUATE_H_

#include <assert.h>

#include "simplify/expression/expression.h"

/* evaluate an expression as much as possible
 * 
 * @expr the expression to simplify
 * @scope the where variables should be assigned and looked up.
 * @return returns an error
 */
error_t expression_evaluate(expression_t* expr, scope_t* scope);

#endif  // SIMPLIFY_EXPRESSION_EVALUATE_H_
