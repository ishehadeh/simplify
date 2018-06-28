#include "simplify/simplify.h"
#include "test/test.h"

void Tsimplify_check_isolate(const char* x, expression_t* var, expression_t* value_of) {
    expression_t* xexpr = Tsimplify_parse(x);

    Tsimplify_evaluate(xexpr);
    Tsimplify_isolate(xexpr, "x");
    Tsimplify_evaluate(xexpr);

    expression_assert_eq(EXPRESSION_RIGHT(xexpr), value_of);
    expression_assert_eq(EXPRESSION_LEFT(xexpr), var);

    expression_free(xexpr);
}

int main() {
    SIMPLIFY_CHECK(isolate, "(0-x) = 1", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(-1));
    SIMPLIFY_CHECK(isolate, "5 / x = 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(2.5));
    SIMPLIFY_CHECK(isolate, "4 \\ x = 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(2));
    SIMPLIFY_CHECK(isolate, "-x = 4", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(-4));
    SIMPLIFY_CHECK(isolate, "99 = -x * 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(-49.5));
    SIMPLIFY_CHECK(isolate, "5 + x", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(-5));
    SIMPLIFY_CHECK(isolate, "2 ^ x= 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(1));
    SIMPLIFY_CHECK(isolate, "2 ^ 4 = 4", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(2));
    SIMPLIFY_CHECK(isolate, "log(2, x + 5) = 3", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(3));
    SIMPLIFY_CHECK(isolate, "8 = 2 * x ^ 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(2));
    SIMPLIFY_CHECK(isolate, "11 = x \\ 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(121));
    SIMPLIFY_CHECK(isolate, "0 = x - 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(2));
    SIMPLIFY_CHECK(isolate, "-8 = +x + 2", TSIMPLIFY_VAR("x"), TSIMPLIFY_NUM(-10));
    SIMPLIFY_CHECK(isolate, "-8 = x(y) + 2", TSIMPLIFY_FUN("x", 1, TSIMPLIFY_VAR("y")), TSIMPLIFY_NUM(-10));
    SIMPLIFY_CHECK(isolate, "x(y, x) = -10", TSIMPLIFY_FUN("x", 2, TSIMPLIFY_VAR("y"), TSIMPLIFY_VAR("x")),
                   TSIMPLIFY_NUM(-10));
}