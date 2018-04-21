/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_EXPRESSION_H_
#define SIMPLIFY_EXPRESSION_EXPRESSION_H_

#include "simplify/errors.h"
#include "simplify/rbtree/rbtree.h"
#include "simplify/expression/expr_types.h"
#include "simplify/expression/stringify.h"

/* A scope holds information about an expression's local variables and functions
 */
typedef struct scope scope_t;

/* Holds information about a variable, constant, or function
 */
typedef struct variable_info variable_info_t;

struct scope {
    scope_t* parent;
    rbtree_t variables;
    int      boolean;   /* The result of all boolean operations so far */
};

struct variable_info {
    int                constant;
    expression_list_t* named_inputs;
    expression_t*      value;
};

/* Initialize a scope
 *
 * @scope the scope to initialize
 */
static inline void scope_init(scope_t* scope) {
    rbtree_init(&scope->variables);
    scope->parent = NULL;
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
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value = value;
    info->named_inputs = NULL;
    info->constant = 0;
    return rbtree_insert(&scope->variables, variable, info);
}

/* define a constant in the scope
 *
 * @scope the scope to define the constant in
 * @name the constant's name
 * @value the constant's value
 * @return return an error code
 */
static inline error_t scope_define_constant(scope_t* scope, char* variable, expression_t* value) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value = value;
    info->named_inputs = NULL;
    info->constant = 0;
    return rbtree_insert(&scope->variables, variable, info);
}

/* define a function in the scope
 *
 * @scope the scope to define the function in
 * @name the function's name
 * @body the functions's body
 * @args the function's arguments
 * @return return an error code
 */
static inline error_t scope_define_function(scope_t* scope,
                                            char* name,
                                            expression_t* value,
                                            expression_list_t* args) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value = value;
    info->named_inputs = args;
    info->constant = 0;
    return rbtree_insert(&scope->variables, name, info);
}


/* get information about a variable
 *
 * @scope the scope to search for the variable
 * @variable the variable's name
 * @value location to store the variable info 
 * @return returns an error code
 */
static inline error_t scope_get_variable_info(scope_t* scope, char* variable, variable_info_t** value) {
    error_t err = rbtree_search(&scope->variables, variable, (void**)value);

    /* check the parent scope(s) for the variable */
    scope_t* parent = scope->parent;
    while (err && parent != NULL) {
        err = rbtree_search(&parent->variables, variable, (void**)value);
        parent = parent->parent;
    }

    return err;
}

/* get the value of a variable or constant
 * @scope the scope to search
 * @name the name to look for
 * @expr location for result
 * @return returns an error code
 */
static inline error_t scope_get_value(scope_t* scope, char* name, expression_t* expr) {
    variable_info_t* info;
    error_t err = scope_get_variable_info(scope, name, &info);
    if (err) return err;
    if (info->named_inputs)
        return ERROR_IS_A_FUNCTION;

    expression_copy(info->value, expr);
    return ERROR_NO_ERROR;
}

/* search a scope for a function
 * @scope the scope to search
 * @name the name to look for
 * @body location for function's body
 * @args location for argument list
 * @return returns an error code
 */
static inline error_t scope_get_function(scope_t* scope, char* name, expression_t* body, expression_list_t* args) {
    variable_info_t* info;
    error_t err = scope_get_variable_info(scope, name, &info);
    if (err) return err;
    if (!info->named_inputs)
        return ERROR_IS_A_VARIABLE;

    expression_list_init(args);
    expression_list_copy(info->named_inputs, args);

    expression_copy(info->value, body);
    return ERROR_NO_ERROR;
}

static void variable_info_free(variable_info_t* info) {
    if (info->named_inputs)
        expression_list_free(info->named_inputs);
    expression_free(info->value);
    free(info);
}

/* clean all resources associated with a scope
 *
 * @scope the scope to clean
 */
static inline void scope_clean(scope_t* scope) {
    rbtree_clean(&scope->variables, (void(*)(void*))&variable_info_free);
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
