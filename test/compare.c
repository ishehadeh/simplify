
#define SIMPLIFY_TEST "compare"

#include "simplify/simplify.h"
#include "test.h"

void Tsimplify_check_compare(const char* x, const char* y, compare_result_t result) {
    expression_t* xexpr = Tsimplify_parse(x);
    expression_t* yexpr = Tsimplify_parse(y);

    Tsimplify_evaluate(xexpr);
    Tsimplify_evaluate(yexpr);

    Tsimplify_simplify(xexpr);
    Tsimplify_simplify(yexpr);

    compare_result_t cmp = expression_compare(xexpr, yexpr);
    if (cmp != result) {
        FATAL("expected the result of \"%s <=> %s\" to be %s, but it was %s.", x, y, print_compare_result(result),
              print_compare_result(cmp));
    }

    expression_free(xexpr);
    expression_free(yexpr);
}

int main() {
    SIMPLIFY_CHECK(compare, "3", "4", COMPARE_RESULT_LESS);
    SIMPLIFY_CHECK(compare, "10", "10", COMPARE_RESULT_EQUAL);
    SIMPLIFY_CHECK(compare, "10", "-10", COMPARE_RESULT_GREATER);
    SIMPLIFY_CHECK(compare, "-0.2", "0.2", COMPARE_RESULT_LESS);
    SIMPLIFY_CHECK(compare, "-0.0002", "-1", COMPARE_RESULT_GREATER);
    SIMPLIFY_CHECK(compare, "-5", "-6", COMPARE_RESULT_GREATER);
    SIMPLIFY_CHECK(compare, "10", "1", COMPARE_RESULT_GREATER);
    SIMPLIFY_CHECK(compare, "-2", "-2", COMPARE_RESULT_EQUAL);
    SIMPLIFY_CHECK(compare, "x + 5", "1", COMPARE_RESULT_INCOMPARABLE);
    SIMPLIFY_CHECK(compare, "2 * x", "2x", COMPARE_RESULT_EQUAL);
    SIMPLIFY_CHECK(compare, "2.25", "2.2", COMPARE_RESULT_GREATER);
    SIMPLIFY_CHECK(compare, "2.0003", "2.2", COMPARE_RESULT_LESS);
    SIMPLIFY_CHECK(compare, ".0001", "0.00002", COMPARE_RESULT_GREATER);
    SIMPLIFY_CHECK(compare, "1.000001", "1.00002", COMPARE_RESULT_LESS);
    SIMPLIFY_CHECK(compare, "5e2", "5e3", COMPARE_RESULT_LESS);
    SIMPLIFY_CHECK(compare, "y * (x2) + x^4", "x^4 + y * (2 * x)", COMPARE_RESULT_EQUAL);
    SIMPLIFY_CHECK(compare, "(y)f(x + 5)", "f(x + 5) * y", COMPARE_RESULT_EQUAL);
    SIMPLIFY_CHECK(compare, "x * x * x", "x ^ 2 * x", COMPARE_RESULT_EQUAL);
}
