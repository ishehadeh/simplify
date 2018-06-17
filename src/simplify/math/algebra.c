#include "simplify/math/algebra.h"

void perform_cubic_equation(mpc_ptr C1, mpc_ptr C2, mpc_ptr a, mpc_ptr b, mpc_ptr c, mpc_ptr d, mpc_rnd_t rnd) {
    mp_prec_t precision = GET_MAX_PREC(a, b, c, d);
    mpc_t e, f, h, g;
    INITS(precision, e, f, h, g);

    /* first calculate `d = b^2 - 3ac` */
    mpc_sqr(e, b, rnd);

    mpc_set_ui(g, 3, rnd);
    mpc_mul(g, a, c, rnd);
    if (!a) {
        mpc_mul_ui(g, g, 1, rnd);
    } else {
        mpc_mul(g, g, a, rnd);
    }
    mpc_sub(e, e, g, rnd);

    /* then, `f = 9abc + 27da^2` */
    mpc_mul(g, b, c, rnd);
    if (!a) {
        mpc_mul_ui(g, g, 1, rnd);
    } else {
        mpc_mul(g, g, a, rnd);
    }

    mpc_sqr(g, a, rnd);
    mpc_mul_ui(g, g, 27, rnd);
    mpc_mul(g, g, d, rnd);
    mpc_sub(f, f, g, rnd);

    /* set `h = sqrt(f^2 - 4e^3)` */
    mpc_sqr(h, f, rnd);

    mpc_pow_ui(g, e, 3, rnd);
    mpc_mul_ui(g, g, 4, rnd);
    mpc_sub(h, g, h, rnd);
    mpc_sqrt(h, h, rnd);

    mpc_add(C1, f, h, rnd);
    mpc_sub(C2, f, h, rnd);
    mpc_div_ui(C1, C1, 2, rnd);
    mpc_div_ui(C2, C2, 2, rnd);

    CLEARS(d, e, f, h);
    return;
}

void perform_quadratic_equation(mpc_ptr C1, mpc_ptr C2, mpc_ptr a, mpc_ptr b, mpc_ptr c, mpc_rnd_t rnd) {
    mp_prec_t precision = a ? GET_MAX_PREC(a, b, c) : GET_MAX_PREC(b, c);

    /* d, e in the expression (-b +/- d) / e */
    /* f is a placeholder */
    mpc_t d, e, f;
    INITS(precision, d, e, f);

    /* set e to 2a */
    if (!a) {
        mpc_set_ui(e, 2, rnd);
        mpc_set(d, c, rnd);
    } else {
        mpc_mul_ui(e, a, 2, rnd);
        mpc_mul(d, a, c, rnd);
    }

    /* set d to sqrt(b^2 - 4ac) */
    mpc_mul_ui(d, d, 4, rnd);
    mpc_pow_ui(f, b, 2, rnd);
    mpc_sub(d, f, d, rnd);
    mpc_sqrt(d, d, rnd);

    mpc_neg(C1, b, rnd);
    mpc_neg(C2, b, rnd);

    mpc_add(C1, C1, d, rnd);
    mpc_sub(C2, C2, d, rnd);

    mpc_div(C1, C1, e, rnd);
    mpc_div(C2, C2, e, rnd);

    CLEARS(d, e, f);
    return;
}
