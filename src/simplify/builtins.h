/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_BUILTINS_H_
#define SIMPLIFY_BUILTINS_H_

#include <time.h>

#include "simplify/errors.h"
#include "simplify/expression/evaluate.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/simplify.h"
#include "simplify/math/math.h"

/* export all builtin variables & functions into a scope
 * @scope the target scope
 */
void simplify_export_builtins(scope_t* scope);

#define ALIAS(SCOPE, X, Y) scope_define_constant((SCOPE), #X, expression_new_variable(#Y))

#endif  // SIMPLIFY_BUILTINS_H_

