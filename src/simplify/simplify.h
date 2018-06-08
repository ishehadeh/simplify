/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_SIMPLIFY_H_
#define SIMPLIFY_SIMPLIFY_H_

/* errors */
#include "simplify/errors.h"

/* language */
#include "simplify/lexer.h"
#include "simplify/parser.h"
#include "simplify/builtins.h"

/* math */
#include "simplify/math/math.h"
#include "simplify/math/algebra.h"

/* red-black tree */
#include "simplify/rbtree/rbtree.h"

/* strings */
#include "simplify/string/string.h"
#include "simplify/string/format.h"

/* expressions */
#include "simplify/expression/expr_types.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/evaluate.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/stringify.h"
#include "simplify/expression/simplify.h"

#endif  // SIMPLIFY_SIMPLIFY_H_
