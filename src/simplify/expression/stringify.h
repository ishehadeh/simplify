/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_EXPRESSION_STRINGIFY_H_
#define SIMPLIFY_EXPRESSION_STRINGIFY_H_

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "simplify/expression/expression.h"

#ifndef STRINGIFIER_DEFAULT_SIZE
#   define STRINGIFIER_DEFAULT_SIZE 256
#endif

#ifndef NAN_STRING
#   define NAN_STRING "[Not a Number]"
#endif

#ifndef INF_STRING
#   define INF_STRING "[Infinity]"
#endif

#define _STRINGIFIER_FIT(ST, X) \
    while (st->index + (X) >= st->length - 1) { \
        stringifier_grow(st); \
    }

/* A stringifier writes an expression to a variable length buffer
 *
 * The stringifier type keeps information about the buffer, and stores the buffer itself.
 */
typedef struct stringifier stringifier_t;


struct stringifier {
    char* buffer;
    size_t length;
    size_t index;
};


char* stringify(expression_t* expr);
static inline size_t stringifier_write_expression(stringifier_t* st, expression_t* expr);
size_t stringifier_write_function(stringifier_t* st, expression_t* func);
size_t stringifier_write_number(stringifier_t* st, expression_t* number);

/* grow the stringifier's internal buffer.
 *
 * This method should be called automatically when writing, there is no need to call it by hand
 * 
 * @st the stringifier to grow
 */
static inline void stringifier_grow(stringifier_t* st) {
    st->length = st->length ? st->length * 2 : STRINGIFIER_DEFAULT_SIZE;
    st->buffer = realloc(st->buffer, st->length);
}

/* write a single byte to the stringifier's buffer
 *
 * @st the stringifier to write to
 * @b the byte to write
 * @return returns the number of bytes written
 */
static inline size_t stringifier_write_byte(stringifier_t* st, char b) {
    _STRINGIFIER_FIT(st, 1)

    st->buffer[st->index++] = b;
    return 1;
}

/* write a string to the stringifier's buffer
 *
 * @st the stringifier to write to
 * @str the base of the string to begin writing
 * @len the number of bytes to write
 * @return returns the number of bytes written
 */
static inline size_t stringifier_write_len(stringifier_t* st, char* str, size_t len) {
    _STRINGIFIER_FIT(st, len)

    strncpy(st->buffer + st->index, str, len);
    st->index += len;
    return len;
}

/* write a null terminated string to the stringifier's buffer
 *
 * @st the stringifier to write to
 * @str the string to write
 * @return returns the number of bytes written
 */
static inline size_t stringifier_write(stringifier_t* st, char* str) {
    size_t len = strlen(str);
    if (len > 0)
        len = stringifier_write_len(st, str, len);
    return len;
}

static inline size_t stringifier_write_variable(stringifier_t* st, expression_t* variable) {
    assert(EXPRESSION_IS_VARIABLE(variable));
    return stringifier_write(st, variable->variable.value);
}

static inline size_t stringifier_write_operator(stringifier_t* st, expression_t* op) {
    assert(EXPRESSION_IS_OPERATOR(op));
    size_t written = stringifier_write_expression(st, op->operator.left);
    written += stringifier_write_byte(st, op->operator.infix);
    written += stringifier_write_expression(st, op->operator.right);
    return written;
}

static inline size_t stringifier_write_prefix(stringifier_t* st, expression_t* pre) {
    assert(EXPRESSION_IS_PREFIX(pre));
    size_t written = stringifier_write_byte(st, pre->prefix.prefix);
    written += stringifier_write_expression(st, pre->prefix.right);
    return written;
}

static inline size_t stringifier_write_expression(stringifier_t* st, expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_FUNCTION:
            return stringifier_write_function(st, expr);
        case EXPRESSION_TYPE_PREFIX:
            return stringifier_write_prefix(st, expr);
        case EXPRESSION_TYPE_OPERATOR:
            return stringifier_write_operator(st, expr);
        case EXPRESSION_TYPE_VARIABLE:
            return stringifier_write_variable(st, expr);
        case EXPRESSION_TYPE_NUMBER:
            return stringifier_write_number(st, expr);
    }
    return 0;
}



/* print an expression to `file`
 * @file the file to write to
 * @expr the expression to print
 * @return an error code
 */
error_t expression_fprint(expression_t* expr, FILE* file);

/* print an expression to stdout
 * @expr the expression to print
 * @return an error code
 */
static inline error_t expression_print(expression_t* expr) {
    return expression_fprint(expr, stdout);
}

#endif  // SIMPLIFY_EXPRESSION_STRINGIFY_H_
