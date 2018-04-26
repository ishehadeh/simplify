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
#include <stdarg.h>

#include "simplify/expression/expression.h"
#include "simplify/expression/stringify.h"
#include "simplify/parser.h"

#define FATAL(MSG, ...) { \
    fprintf(stdout, __FILE__ ":%d FATAL: " MSG "\n", __LINE__, ##__VA_ARGS__); \
    exit(1); \
}
#define ERROR(MSG, ...) fprintf(stdout, __FILE__ ":%d ERROR: " MSG "\n", __LINE__, ##__VA_ARGS__);
#define INFO(MSG, ...) fprintf(stdout, __FILE__ ":%d INFO: " MSG "\n", __LINE__, ##__VA_ARGS__);
#define WARN(MSG, ...) fprintf(stdout, __FILE__ ":%d WARN: " MSG "\n", __LINE__, ##__VA_ARGS__);

static inline const char* print_type(expression_type_t t) {
    switch (t) {
        case EXPRESSION_TYPE_FUNCTION:
            return "EXPRESSION_TYPE_FUNCTION";
        case EXPRESSION_TYPE_NUMBER:
            return "EXPRESSION_TYPE_NUMBER";
        case EXPRESSION_TYPE_OPERATOR:
            return "EXPRESSION_TYPE_OPERATOR";
        case EXPRESSION_TYPE_PREFIX:
            return "EXPRESSION_TYPE_PREFIX";
        case EXPRESSION_TYPE_VARIABLE:
            return "EXPRESSION_TYPE_VARIABLE";
    }
    return "UNKOWN";
}

static inline expression_t* expression_new_operator(expression_t* left, operator_t op, expression_t* right) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_operator(x, left, op, right);
    return x;
}

static inline expression_t* expression_new_prefix(operator_t op, expression_t* right) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_prefix(x, op, right);
    return x;
}

static inline expression_t* expression_new_number(double num) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_number_d(x, num);
    return x;
}

static inline expression_t* expression_new_variable(variable_t var) {
    expression_t* x = malloc(sizeof(expression_t));
    expression_init_variable(x, var, strlen(var));
    return x;
}

static inline expression_t* expression_new_function(variable_t name, int param_count, ...) {
    expression_t* x = malloc(sizeof(expression_t));
    va_list args;
    va_start(args, param_count);
    expression_list_t* params = malloc(sizeof(expression_list_t));
    expression_list_init(params);

    for (int i = 0; i < param_count; ++i) {
        expression_list_append(params, va_arg(args, expression_t*));
    }
    va_end(args);
    expression_init_function(x, name, strlen(name), params);
    return x;
}

void expression_assert_eq(expression_t* expr1, expression_t* expr2) {
    if (expr1->type != expr2->type) {
        printf("EXPECTED:\n");
        expression_print(expr2);
        printf("\n\nGOT:\n");
        expression_print(expr1);
        puts("");
        FATAL("ASSERT FAILED (%s != %s): expression types don't match",
            print_type(expr1->type), print_type(expr2->type));
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

void assert_token_eq(token_t* tok1, token_t* tok2) {
    if (tok1->length != tok2->length || strncmp(tok1->start, tok2->start, tok1->length) != 0) {
        FATAL("token strings don't match! ('%.*s' != '%.*s')",
            (int)tok1->length, tok1->start, (int)tok2->length, tok2->start);
    }

    if (tok1->type != tok2->type) {
        FATAL("tokens are not of the same type! (%d != %d)", tok1->type, tok2->type);
    }
}

#endif  // TEST_TEST_H_
