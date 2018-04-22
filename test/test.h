/* Copyright Ian Shehadeh 2018 */

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

/* file: test/test.h
 * Utilities and data to help create tests
 * 
 * Macros:
 *  - FATAL(MSG, ...FMT) : print an error and exit
 *  - ERROR(MSG, ...FMT) : print an error, but do not exit
 *  - INFO(MSG, ...FMT) : print an info message
 *  - WARN(MSG, ...FMT) : print a warning message (goes to stderr)
 */

#include <stdlib.h>
#include <stdio.h>

#include "simplify/expression/expression.h"

#define FATAL(MSG, ...) { \
    fprintf(stderr, __FILE__ ":%d FATAL: " MSG "\n", __LINE__, ##__VA_ARGS__); \
    exit(1); \
}
#define ERROR(MSG, ...) fprintf(stderr, __FILE__ ":%d ERROR: " MSG "\n", __LINE__, ##__VA_ARGS__);
#define INFO(MSG, ...) fprintf(stdout, __FILE__ ":%d INFO: " MSG "\n", __LINE__, ##__VA_ARGS__);
#define WARN(MSG, ...) fprintf(stderr, __FILE__ ":%d WARN: " MSG "\n", __LINE__, ##__VA_ARGS__);


void expression_assert_eq(expression_t* expr1, expression_t* expr2) {
    if (expr1->type != expr2->type) {
        expression_print(expr2);
        FATAL("ASSERT FAILED (%d != %d): expression types don't match", expr1->type, expr2->type);
    }

    switch (expr1->type) {
        case EXPRESSION_TYPE_NUMBER:
            if (mpfr_cmp(expr1->number.value, expr2->number.value) != 0) {
                char x1[25];
                char x2[25];
                x1[24] = 0;
                x2[24] = 0;
                number_to_buffer(expr1->number.value, x1, 24);
                number_to_buffer(expr2->number.value, x2, 24);
                FATAL("ASSERT FAILED ('%s' != '%s'): numeric expressions don't match", x1, x2);
            }
            break;
        case EXPRESSION_TYPE_PREFIX:
        case EXPRESSION_TYPE_OPERATOR:
        {
            if (EXPRESSION_OPERATOR(expr1) != EXPRESSION_OPERATOR(expr2))
                FATAL("ASSERT FAILED ('%c' != '%c'): expression's operators don't match",
                    EXPRESSION_OPERATOR(expr1), EXPRESSION_OPERATOR(expr2));
            if (EXPRESSION_LEFT(expr1))
                expression_assert_eq(EXPRESSION_LEFT(expr1), EXPRESSION_LEFT(expr2));
            expression_assert_eq(EXPRESSION_RIGHT(expr1), EXPRESSION_RIGHT(expr2));
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            if (strcmp(expr1->variable.value, expr2->variable.value) != 0)
                FATAL("ASSERT FAILED ('%s' != '%s'): variable names don't match",
                    expr1->variable.value, expr2->variable.value);
            break;
        case EXPRESSION_TYPE_FUNCTION:
        {
            if (strcmp(expr1->function.name, expr2->function.name) != 0)
                FATAL("ASSERT FAILED ('%s' != '%s'): function names don't match",
                    expr1->function.name, expr2->function.name);

            expression_t*      param1;
            expression_list_t* other = expr2->function.parameters;
            EXPRESSION_LIST_FOREACH(param1, expr1->function.parameters) {
                if (!other)
                    FATAL("ASSERT FAILED: argument count doesn't match");
                expression_assert_eq(param1, other->value);
                other = other->next;
            }
            if (other)
                FATAL("ASSERT FAILED: argument count doesn't match");
        }
    }
}

#endif  // TEST_TEST_H_
