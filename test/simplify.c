#include "simplify/simplify.h"
#include "test/test.h"

void Tsimplify_check_simplify(const char* x, expression_t* result) {
    expression_t* xexpr = Tsimplify_parse(x);

    Tsimplify_evaluate(xexpr);
    Tsimplify_simplify(xexpr);

    expression_assert_eq(xexpr, result);

    expression_free(xexpr);
}

int main() {
    SIMPLIFY_CHECK(simplify, "x * x", TSIMPLIFY_OP(TSIMPLIFY_VAR("x"), '^', TSIMPLIFY_NUM(2)));
    SIMPLIFY_CHECK(
        simplify, "3 + x * x * x * 3",
        TSIMPLIFY_OP(TSIMPLIFY_NUM(3), '+',
                     TSIMPLIFY_OP(TSIMPLIFY_OP(TSIMPLIFY_VAR("x"), '^', TSIMPLIFY_NUM(3)), '*', TSIMPLIFY_NUM(3))));
    SIMPLIFY_CHECK(
        simplify, "3 * x * x * x * 3",
        TSIMPLIFY_OP(TSIMPLIFY_NUM(3), '*',
                     TSIMPLIFY_OP(TSIMPLIFY_OP(TSIMPLIFY_VAR("x"), '^', TSIMPLIFY_NUM(3)), '*', TSIMPLIFY_NUM(3))));
    SIMPLIFY_CHECK(
        simplify, "100 ^ x * x * x * 3",
        TSIMPLIFY_OP(TSIMPLIFY_OP(TSIMPLIFY_NUM(100), '^', TSIMPLIFY_VAR("x")), '*',
                     TSIMPLIFY_OP(TSIMPLIFY_OP(TSIMPLIFY_VAR("x"), '^', TSIMPLIFY_NUM(2)), '*', TSIMPLIFY_NUM(3))));
}

