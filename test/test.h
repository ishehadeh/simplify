/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_TEST_TEST_H_
#define SIMPLIFY_TEST_TEST_H_

#include <stdio.h>
#include <stdlib.h>

#include "simplify/simplify.h"

static unsigned long long _g_Tsimplify_check;

#define FATAL(MSG, ...)                                                            \
    {                                                                              \
        fprintf(stdout, __FILE__ ":%d FATAL: " MSG "\n", __LINE__, ##__VA_ARGS__); \
        exit(1);                                                                   \
    }
#define ERROR(MSG, ...) fprintf(stdout, __FILE__ ":%d ERROR: " MSG "\n", __LINE__, ##__VA_ARGS__);
#define INFO(MSG, ...) fprintf(stdout, __FILE__ ":%d INFO: " MSG "\n", __LINE__, ##__VA_ARGS__);
#define WARN(MSG, ...) fprintf(stdout, __FILE__ ":%d WARN: " MSG "\n", __LINE__, ##__VA_ARGS__);

#define TSIMPLIFY_OP(X, OP, Y) expression_new_operator(X, OP, Y)
#define TSIMPLIFY_NUM(X) expression_new_number_d(X)
#define TSIMPLIFY_VAR(X) expression_new_variable(X)
#define TSIMPLIFY_FUN(NAME, ARGS, ...) expression_new_function(NAME, ARGS, __VA_ARGS__)
#define TSIMPLIFY_PREFIX(PRE, X) expression_new_prefix(PRE, X)

#define SIMPLIFY_CHECK(CHECK, ...)        \
    TSIMPLIFY_START_CHECK(CHECK)          \
    Tsimplify_check_##CHECK(__VA_ARGS__); \
    TSIMPLIFY_END_CHECK()

#define TSIMPLIFY_START_CHECK(CHECK) INFO("starting check %s #%llu", #CHECK, ++_g_Tsimplify_check);
#define TSIMPLIFY_END_CHECK() INFO("finished check #%llu", _g_Tsimplify_check)
#define TSIMPLIFY_RESET_CHECK_COUNT() _g_Tsimplify_check = 0

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

static inline const char* print_compare_result(compare_result_t t) {
    switch (t) {
        case COMPARE_RESULT_LESS:
            return "COMPARE_RESULT_LESS";
        case COMPARE_RESULT_GREATER:
            return "COMPARE_RESULT_GREATER";
        case COMPARE_RESULT_EQUAL:
            return "COMPARE_RESULT_EQUAL";
        case COMPARE_RESULT_INCOMPARABLE:
            return "COMPARE_RESULT_INCOMPARABLE";
    }
    return "UNKOWN";
}

static inline const char* print_expression_result(expression_result_t t) {
    switch (t) {
        case EXPRESSION_RESULT_FALSE:
            return "EXPRESSION_RESULT_FALSE";
        case EXPRESSION_RESULT_TRUE:
            return "EXPRESSION_RESULT_TRUE";
        case EXPRESSION_RESULT_NONBINARY:
            return "EXPRESSION_RESULT_NONBINARY";
    }
    return "UNKOWN";
}

expression_t* Tsimplify_parse(const char* str) {
    expression_t* expr = expression_new_uninialized();
    error_t err = parse_string((char*)str, expr);
    if (err) FATAL("failed to parse \"%s\" (reason: %s)", str, error_string(err));
    return expr;
}

void Tsimplify_evaluate(expression_t* expr) {
    scope_t scope;
    scope_init(&scope);
    simplify_export_builtins(&scope);
    error_t err = expression_evaluate(expr, &scope);
    if (err) FATAL("failed to evaluate \"%s\" (reason: %s)", stringify(expr), error_string(err));
    scope_clean(&scope);
}

compare_result_t Tsimplify_compare(expression_t* expr) { return expression_evaluate_comparisons(expr); }

void Tsimplify_simplify(expression_t* expr) {
    error_t err = expression_simplify(expr);
    if (err) FATAL("failed to simplify \"%s\" (reason: %s)", stringify(expr), error_string(err));
}

void Tsimplify_isolate(expression_t* expr, char* x) {
    error_t err = expression_isolate_variable(expr, x);
    if (err) FATAL("failed to simplify \"%s\" (reason: %s)", stringify(expr), error_string(err));
}

void expression_assert_eq(expression_t* expr1, expression_t* expr2) {
    if (expr1->type != expr2->type) {
        printf("EXPECTED:\n");
        expression_print(expr2);
        printf("\n\nGOT:\n");
        expression_print(expr1);
        puts("");
        FATAL("ASSERT FAILED (%s != %s): expression types don't match", print_type(expr1->type),
              print_type(expr2->type));
    }

    switch (expr1->type) {
        case EXPRESSION_TYPE_NUMBER:
            if (expression_compare(expr1, expr2) != COMPARE_RESULT_EQUAL) {
                char* expr1num = stringify(expr1);
                char* expr2num = stringify(expr2);
                FATAL("ASSERT FAILED ('%s' != '%s'): numeric expressions don't match", expr1num, expr2num);
                free(expr1num);
                free(expr2num);
            }
            break;
        case EXPRESSION_TYPE_PREFIX:
        case EXPRESSION_TYPE_OPERATOR: {
            if (EXPRESSION_OPERATOR(expr1) != EXPRESSION_OPERATOR(expr2))
                FATAL("ASSERT FAILED ('%c' != '%c'): expression's operators don't match", EXPRESSION_OPERATOR(expr1),
                      EXPRESSION_OPERATOR(expr2));
            if (EXPRESSION_LEFT(expr1)) expression_assert_eq(EXPRESSION_LEFT(expr1), EXPRESSION_LEFT(expr2));
            expression_assert_eq(EXPRESSION_RIGHT(expr1), EXPRESSION_RIGHT(expr2));
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            if (strcmp(expr1->variable.value, expr2->variable.value) != 0)
                FATAL("ASSERT FAILED ('%s' != '%s'): variable names don't match", expr1->variable.value,
                      expr2->variable.value);
            break;
        case EXPRESSION_TYPE_FUNCTION: {
            if (strcmp(expr1->function.name, expr2->function.name) != 0)
                FATAL("ASSERT FAILED ('%s' != '%s'): function names don't match", expr1->function.name,
                      expr2->function.name);

            expression_t* param1;
            expression_list_t* other = expr2->function.parameters;
            EXPRESSION_LIST_FOREACH(param1, expr1->function.parameters) {
                if (!other) FATAL("%s", "ASSERT FAILED: arguments don't match");
                expression_assert_eq(param1, other->value);
                other = other->next;
            }
            if (other) FATAL("%s", "ASSERT FAILED: arguments don't match");
        }
    }
}

void assert_token_eq(token_t* tok1, token_t* tok2) {
    if (tok1->length != tok2->length || strncmp(tok1->start, tok2->start, tok1->length) != 0) {
        FATAL("token strings don't match! ('%.*s' != '%.*s')", (int)tok1->length, tok1->start, (int)tok2->length,
              tok2->start);
    }

    if (tok1->type != tok2->type) {
        FATAL("tokens are not of the same type! (%d != %d)", tok1->type, tok2->type);
    }
}

#endif  // SIMPLIFY_TEST_TEST_H_
