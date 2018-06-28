/* Copyright Ian Shehadeh 2018 */

#include "simplify/simplify.h"
#include "test/test.h"

void Tsimplify_check_evaluate(const char* x, expression_t* end, expression_result_t result) {
    expression_t* xexpr = Tsimplify_parse(x);

    Tsimplify_evaluate(xexpr);
    expression_assert_eq(xexpr, end);

    expression_result_t cmp_result = Tsimplify_compare(xexpr);

    if (result != cmp_result) {
        FATAL("expected \"%s\" to be %s, but it was %s.", x, print_expression_result(cmp_result),
              print_expression_result(result));
    }
}

int main() {
    SIMPLIFY_CHECK(evaluate, "2 * 5.5", TSIMPLIFY_NUM(11), EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "2 * 9x", TSIMPLIFY_OP(TSIMPLIFY_NUM(18), '*', TSIMPLIFY_VAR("x")),
                   EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "4 \\ 2 = 5", TSIMPLIFY_OP(TSIMPLIFY_NUM(2), '=', TSIMPLIFY_NUM(5)),
                   EXPRESSION_RESULT_FALSE);
    SIMPLIFY_CHECK(evaluate, "2 < 11", TSIMPLIFY_OP(TSIMPLIFY_NUM(2), '<', TSIMPLIFY_NUM(11)), EXPRESSION_RESULT_TRUE);
    SIMPLIFY_CHECK(evaluate, "31.5 > 1", TSIMPLIFY_OP(TSIMPLIFY_NUM(31.5), '>', TSIMPLIFY_NUM(1)),
                   EXPRESSION_RESULT_TRUE);
    SIMPLIFY_CHECK(evaluate, "0.94 > 0.991", TSIMPLIFY_OP(TSIMPLIFY_NUM(0.94), '>', TSIMPLIFY_NUM(0.991)),
                   EXPRESSION_RESULT_FALSE);
    SIMPLIFY_CHECK(evaluate, "x ^ 3 * 5(6 + 0 \\ 1)",
                   TSIMPLIFY_OP(TSIMPLIFY_OP(TSIMPLIFY_VAR("x"), '^', TSIMPLIFY_NUM(3)), '*', TSIMPLIFY_NUM(30)),
                   EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "f(x, y): 10x^y",
                   TSIMPLIFY_OP(TSIMPLIFY_NUM(10), '*', TSIMPLIFY_OP(TSIMPLIFY_VAR("x"), '^', TSIMPLIFY_VAR("y"))),
                   EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "5 / x : 2 * 10", TSIMPLIFY_NUM(0.25), EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "+(-5 * 2)", TSIMPLIFY_NUM(-10), EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "-(3 ^ 4)", TSIMPLIFY_NUM(-81), EXPRESSION_RESULT_NONBINARY);
    SIMPLIFY_CHECK(evaluate, "cos(pi2)", TSIMPLIFY_NUM(1), EXPRESSION_RESULT_NONBINARY);
}