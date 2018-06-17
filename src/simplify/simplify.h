/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_SIMPLIFY_H_
#define SIMPLIFY_SIMPLIFY_H_

/* errors */
#include "simplify/errors.h"

/* language */
#include "simplify/builtins.h"
#include "simplify/lexer.h"
#include "simplify/parser.h"

/* math */
#include "simplify/math/algebra.h"
#include "simplify/math/math.h"

/* red-black tree */
#include "simplify/rbtree/rbtree.h"

/* strings */
#include "simplify/string/format.h"
#include "simplify/string/string.h"

/* expressions */
#include "simplify/expression/evaluate.h"
#include "simplify/expression/expr_types.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/simplify.h"
#include "simplify/expression/stringify.h"

#endif  // SIMPLIFY_SIMPLIFY_H_
