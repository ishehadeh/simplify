/* Copyright Ian Shehadeh 2018 

* @file expr_types.h
* expression type definitions and utilities.
*/

#ifndef SIMPLIFY_EXPRESSION_EXPR_TYPES_H_
#define SIMPLIFY_EXPRESSION_EXPR_TYPES_H_

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <gmp.h>
#include <mpfr.h>
#include "simplify/errors.h"

#define EXPRESSION_IS_OPERATOR(EXPR) ((EXPR)->type == (EXPRESSION_TYPE_OPERATOR))
#define EXPRESSION_IS_VARIABLE(EXPR) ((EXPR)->type == (EXPRESSION_TYPE_VARIABLE))
#define EXPRESSION_IS_NUMBER(EXPR)   ((EXPR)->type == (EXPRESSION_TYPE_NUMBER))
#define EXPRESSION_IS_PREFIX(EXPR)   ((EXPR)->type == (EXPRESSION_TYPE_PREFIX))

#define EXPRESSION_RIGHT(EXPR) \
    (EXPRESSION_IS_PREFIX(EXPR) ? \
    ((EXPR)->prefix.right) : (EXPRESSION_IS_OPERATOR(EXPR) ? \
    (EXPR->operator.right) : (expression_t*)NULL))

#define EXPRESSION_LEFT(EXPR) (EXPRESSION_IS_OPERATOR(EXPR) ? (EXPR->operator.left) : (expression_t*)NULL)


#define EXPRESSION_OPERATOR(EXPR)  \
    (EXPRESSION_IS_PREFIX(EXPR) ? \
    ((EXPR)->prefix.prefix) : (EXPRESSION_IS_PREFIX(EXPR)   ? \
    (EXPR->operator.infix)  : '\0'))

/* A parsed mathmatical expression.
 *
 * An expression is a union of four structures:
 * 
 * - number
 * 
 * - prefix
 * 
 * - variable
 * 
 * - operator
 * 
 * Which one of these structures is currently in use is stored in the `type` option.
 * The EXPRESSION_IS_XXX macros can be used to conveniently check the type of an expression.
 * See the Expression Structures category for more information on each structure.
 */
typedef union expression      expression_t;

/* Specifies an operator's type.
 * An operator represents a specific operation performed on one or more numbers.
 * At the moment an operator is just the literal token stored in a `char`. This is liable to change in the future
 */
typedef char                  operator_t;

/* A variable is a null terminated string.
 */
typedef char*                 variable_t;

/* Enumerates the values that can be held in `expression_t`.
 */
typedef enum  expression_type expression_type_t;


/* Operator precedence enumerates the different precedence levels for groups of operators.
 */
typedef enum operator_precedence operator_precedence_t;

enum operator_precedence {
    OPERATOR_PRECEDENCE_MINIMUM,
    OPERATOR_PRECEDENCE_COMPARE,
    OPERATOR_PRECEDENCE_ASSIGN,
    OPERATOR_PRECEDENCE_SUM,
    OPERATOR_PRECEDENCE_PRODUCT,
    OPERATOR_PRECEDENCE_EXPONENT,
    OPERATOR_PRECEDENCE_MAXIMUM,
};

enum expression_type {
    EXPRESSION_TYPE_NUMBER,
    EXPRESSION_TYPE_PREFIX,
    EXPRESSION_TYPE_OPERATOR,
    EXPRESSION_TYPE_VARIABLE,
};

/* cldoc:begin-category(Expression Structures) */

struct expression_number {
    expression_type_t type;
    mpfr_t value;
};

struct expression_prefix {
    expression_type_t type;
    operator_t    prefix;
    expression_t* right;
};


struct expression_variable {
    expression_type_t type;
    variable_t value;
};

struct expression_operator {
    expression_type_t type;

    expression_t* left;
    operator_t    infix;
    expression_t* right;
};

union expression {
    expression_type_t type;
    struct expression_number   number;
    struct expression_variable variable;
    struct expression_prefix   prefix;
    struct expression_operator operator;
};

/* cldoc:end-category() */


/* initialize an operator expression
 * @expression the expression to initialize
 * @left the left side of the operator expression
 * @operator the infix operator in the expression
 * @right the right side of the operator expression
 */
void expression_init_operator(expression_t* expression,  expression_t* left, operator_t operator, expression_t* right);

/* initialize a prefix expression
 * @expression the expression to initialize
 * @operator the prefix operator in the expression
 * @right the expression to the right of the prefix
 */
void expression_init_prefix(expression_t* expression, operator_t operator, expression_t* right);

/* initialize a variable expression
 * @expression the expression to initialize
 * @name the variable's name
 * @length the length of the variable's name
 * 
 * It should be noted that this function copies the value `name` is pointing to up through `length`.
 */
void expression_init_variable(expression_t* expression, char* name, size_t length);


/* initialize a new number expression
 * @expression the expression to initialize
 * @number the number to use as the expression's initial value
 */
void expression_init_number(expression_t* expression, mpfr_t number);

/* initialize a new number expression with a double
 * @expression the expression to initialize
 * @number the number to use as the expression initial value
 */
void expression_init_number_d(expression_t* expression, double number);


/* initialize a new number expression with an integer
 * @expression the expression to initialize
 * @number the number to use as the expression initial value
 */
void expression_init_number_si(expression_t* expression, int number);

/* free all memory referenced by expression recursively, but not expression itself.
 * @expression the expression to clean
 */
void expression_clean(expression_t* expression);

/* free all memory referenced by the expression and the expression.
 * @expression the expression to free
 */
void expression_free(expression_t* expression);

/* deep copy expression into expression2.
 * @expression the source expression
 * @expression2 the destination expression
 */
void expression_copy(expression_t* expression, expression_t* expression2);

/* get the precedence of a given operator (higher = should be executed first)
 * @op the operator to check
 * 
 * @returns `op's` precedence
 */
extern operator_precedence_t operator_precedence(operator_t op);

/* check if the expression is an operator expression that compares it's left & right branches.
 * @expression the expression to check
 * @returns a boolean integer, true if the expression has a comparison operator.
 */
static inline int expression_is_comparison(expression_t* expr) {
    return EXPRESSION_IS_OPERATOR(expr) &&
        (expr->operator.infix == '<' ||
        expr->operator.infix == '>' ||
        expr->operator.infix == '=');
}

#endif  // SIMPLIFY_EXPRESSION_EXPR_TYPES_H_
