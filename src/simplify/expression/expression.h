/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_EXPRESSION_H_

#include "simplify/errors.h"
#include "simplify/rbtree/rbtree.h"
#include "simplify/expression/expr_types.h"

/* check for a variable or function in the expression
 *
 * @expr the expression to search
 * @var the variable to search for
 */
int expression_has_variable_or_function(expression_t* expr, variable_t var);

/* get the name of the first variable that appears in an expression
 *
 * @expr the expression to search
 * @return returns the variable's name, or NULL if it was not found
 */ 
variable_t expression_find_variable(expression_t* expr);

/* get the name of the first function that appears in an expression
 *
 * @expr the expression to search
 * @return returns the function's name, or NULL if it was not found
 */ 
variable_t expression_find_function(expression_t* expr);

#endif  // SIMPLIFY_EXPRESSION_EXPRESSION_H_
