// Copyright Ian R. Shehadeh

#ifndef SIMPLIFY_PARSER_H_
#define SIMPLIFY_PARSER_H_

#include "simplify/expression.h"
#include "simplify/lexer.h"

#define parse_expression(PARSER, EXPRESSION) parse_expression_prec(PARSER, EXPRESSION, OPERATOR_PRECEDENCE_MINIMUM)

typedef struct expression_parser   expression_parser_t;
typedef enum   operator_precedence operator_precedence_t;

enum operator_precedence {
    OPERATOR_PRECEDENCE_MINIMUM,
    OPERATOR_PRECEDENCE_COMPARE,
    OPERATOR_PRECEDENCE_SUM,
    OPERATOR_PRECEDENCE_PRODUCT,
    OPERATOR_PRECEDENCE_EXPONENT,
    OPERATOR_PRECEDENCE_MAXIMUM,
};

struct expression_parser {
    lexer_t* lexer;
    token_t  previous;
    int      missing_right_parens;
};

static inline void expression_parser_init(expression_parser_t* parser, lexer_t* lexer) {
    parser->lexer = lexer,
    parser->missing_right_parens = 0;
}

error_t parse_expression_prec(expression_parser_t* parser, expression_t* expr, operator_precedence_t prec);

#endif  // SIMPLIFY_PARSER_H_
