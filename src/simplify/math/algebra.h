/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_MATH_ALGEBRA_H_
#define SIMPLIFY_MATH_ALGEBRA_H_

#include <mpc.h>
#include "simplify/math/math.h"

void perform_quadratic_equation(mpc_ptr C1, mpc_ptr C2, mpc_ptr a, mpc_ptr b, mpc_ptr c, mpc_rnd_t);
void perform_cubic_equation(mpc_ptr C1, mpc_ptr C2, mpc_ptr a, mpc_ptr b, mpc_ptr c, mpc_ptr d, mpc_rnd_t);

#endif  // SIMPLIFY_MATH_ALGEBRA_H_
