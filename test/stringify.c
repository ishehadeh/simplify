#include "test.h"

void Tsimplify_check_stringify(expression_t* expr, char* str) {
    char* rstr = stringify(expr);
    if (strcmp(str, rstr) != 0) FATAL("expecting string \"%s\" got \"%s\"", str, rstr); 
}

int main() {
    mpc_t bad_number;
    mpc_t infinity;
    mpc_t neg_infinity;

    mpc_init2(bad_number, 8);
    mpc_init2(infinity, 8);
    mpc_init2(neg_infinity, 8);
    mpc_set_si_si(bad_number, 0, 0, MPC_RNDNN);
    mpc_set_si_si(infinity, 0, 0, MPC_RNDNN);
    mpc_set_si_si(neg_infinity, 0, 0, MPC_RNDNN);

    mpfr_set_nan(mpc_realref(bad_number));
    mpfr_set_inf(mpc_realref(infinity), 1);
    mpfr_set_inf(mpc_realref(neg_infinity), -1);

    SIMPLIFY_CHECK(stringify, TSIMPLIFY_NUM(5), "5");
    SIMPLIFY_CHECK(stringify, TSIMPLIFY_NUM(2.25), "2.25");
    SIMPLIFY_CHECK(stringify,
                   TSIMPLIFY_OP(TSIMPLIFY_FUN("f", 2, TSIMPLIFY_NUM(5), TSIMPLIFY_VAR("x")), '*', TSIMPLIFY_VAR("y")),
                   "f(5, x) * y");
    SIMPLIFY_CHECK(stringify, TSIMPLIFY_OP(TSIMPLIFY_PREFIX('+', TSIMPLIFY_VAR("OwO")), '-', TSIMPLIFY_NUM(3)),
                   "+OwO - 3");
    SIMPLIFY_CHECK(stringify, TSIMPLIFY_NUM(3.9), "3.9");
    SIMPLIFY_CHECK(stringify, expression_new_number(infinity), "Inf");
    SIMPLIFY_CHECK(stringify, expression_new_number(bad_number), "NaN");
    SIMPLIFY_CHECK(stringify, expression_new_number(neg_infinity), "-Inf");
}
