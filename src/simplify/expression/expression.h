/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_EXPRESSION_H_

#include "simplify/errors.h"
#include "simplify/rbtree/rbtree.h"
#include "simplify/expression/expr_types.h"
#include "simplify/expression/stringify.h"

/* evaluate an expression as much as possible
 * @expr the expression to simplify
 * @scope the where variables should be assigned and looked up.
 * @return returns an error
 */
error_t expression_evaluate(expression_t* expr, scope_t* scope);

/* isolate a variable on one side of an comparison operator.
 * If no comparison operator is present, append `= 0`
 * 
 * @expr the expression to work with
 * @var the variable to isolate
 * @return returns an error code
 */
error_t expression_isolate_variable(expression_t* expr, variable_t var);


#endif  // SIMPLIFY_EXPRESSION_EXPRESSION_H_
