/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EVALUATE_H_
#define SIMPLIFY_EXPRESSION_EVALUATE_H_

#include <assert.h>

#include "simplify/expression/expression.h"

/* The boolean result of an expression, No Result (if there was no condition operator), true or false */
typedef enum expression_result expression_result_t;

enum expression_result {
    EXPRESSION_RESULT_NONBINARY,
    EXPRESSION_RESULT_TRUE,
    EXPRESSION_RESULT_FALSE,
};

/* evaluate an expression as much as possible
 * 
 * @expr the expression to simplify
 * @scope the where variables should be assigned and looked up.
 * @return returns an error
 */
error_t expression_evaluate(expression_t* expr, scope_t* scope);

expression_result_t expression_evaluate_comparisons(expression_t* expr);

#endif  // SIMPLIFY_EXPRESSION_EVALUATE_H_
