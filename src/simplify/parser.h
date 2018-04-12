// Copyright Ian R. Shehadeh

#ifndef SIMPLIFY_PARSER_H_
#define SIMPLIFY_PARSER_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "simplify/expression.h"
#include "simplify/lexer.h"

#ifndef TOKEN_STREAM_BUFFER_INITIAL_CAPACITY
#   define TOKEN_STREAM_BUFFER_INITIAL_CAPACITY 50
#endif


expression_t* parse_expression(lexer_t*);

#endif  // SIMPLIFY_PARSER_H_
