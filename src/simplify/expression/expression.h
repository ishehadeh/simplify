/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_EXPRESSION_H_

#include "simplify/errors.h"
#include "simplify/rbtree/rbtree.h"
#include "simplify/expression/expr_types.h"
#include "simplify/expression/stringify.h"

/* A scope holds information about an expression's local variables and functions
 */
typedef struct scope          scope_t;

struct scope {
    rbtree_t variables;
    rbtree_t functions;
    int      boolean;   /* The result of all boolean operations so far */
};

/* Initialize a scope
 *
 * @scope the scope to initialize
 */
static inline void scope_init(scope_t* scope) {
    rbtree_init(&scope->variables);
    rbtree_init(&scope->functions);
    scope->boolean = -1;
}

/* define a variable in the scope
 *
 * @scope the scope to define the variable in
 * @variable the variable's name
 * @value the variable's value
 * @return return an error code
 */
static inline error_t scope_define(scope_t* scope, char* variable, expression_t* value) {
    return rbtree_insert(&scope->variables, variable, value);
}

/* get the value of a variable
 *
 * @scope the scope to search for the variable in
 * @variable the variable's name
 * @value the location where the value of the variable will be copied.
 * @return return an error code
 */
static inline error_t scope_value_of(scope_t* scope, char* variable, expression_t* value) {
    expression_t* temp_value;
    error_t err = rbtree_search(&scope->variables, variable, (void**)&temp_value);
    if (err) return err;

    expression_copy(temp_value, value);
    return ERROR_NO_ERROR;
}

/* clean all resources associated with a scope
 *
 * @param scope
 */
static inline void scope_clean(scope_t* scope) {
    rbtree_clean(&scope->variables, (void(*)(void*))&expression_free);
    rbtree_clean(&scope->functions, (void(*)(void*))&expression_free);
}

/* evaluate an expression as much as possible
 * @expr the expression to simplify
 * @scope the where variables should be assigned and looked up.
 * @return returns an error
 */
error_t expression_simplify(expression_t* expr, scope_t* scope);

/* isolate a variable on one side of an comparison operator.
 * If no comparison operator is present, append `= 0`
 * 
 * @expr the expression to work with
 * @var the variable to isolate
 * @return returns an error code
 */
error_t expression_isolate_variable(expression_t* expr, variable_t var);


#endif  // SIMPLIFY_EXPRESSION_EXPRESSION_H_
