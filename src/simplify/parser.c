// Copyright Ian R. Shehadeh

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "simplify/parser.h"
#include "simplify/errors.h"

inline static operator_precedence_t operator_precedence(operator_t op) {
    switch (op) {
        case '=':
        case '>':
        case '<':
            return OPERATOR_PRECEDENCE_COMPARE;
        case '+':
        case '-':
            return OPERATOR_PRECEDENCE_SUM;
        case '*':
        case '/':
            return OPERATOR_PRECEDENCE_PRODUCT;
        case '^':
            return OPERATOR_PRECEDENCE_EXPONENT;
        case '(':
            return OPERATOR_PRECEDENCE_MAXIMUM;
        default:
            // TODO(IanS5): Somehow throw an error here
            return OPERATOR_PRECEDENCE_MINIMUM;
    }
}


error_t parse_expression_prec(expression_parser_t* parser, expression_t* expression, operator_precedence_t precedence) {
    token_t token;
    error_t err = lexer_next(parser->lexer, &token);
    if (err) return err;

    expression_t* left;

    switch (token.type) {
        case TOKEN_TYPE_OPERATOR:
        {
            operator_t operator = *token.start;
            expression_t* operand = new_expression();

            error_t err = parse_expression_prec(parser, operand, OPERATOR_PRECEDENCE_MAXIMUM);
            if (err) return err;

            left = new_prefix_expression(operator, operand);
            memmove(&token, &parser->previous, sizeof(token_t));
            break;
        }
        case TOKEN_TYPE_NUMBER:
        {
            scalar_t num;

            char* del = alloca(token.length + 1);
            del[token.length] = 0;
            strncpy(del, token.start, token.length);
            if (SCALAR_FROM_STRING(del, num))
                return ERROR_INVALID_NUMBER;

            left = new_number_expression(num);

            SCALAR_CLEAN(num);
            err = lexer_next(parser->lexer, &token);
            if (err) {
                expression_free(left);
                return err;
            }
            break;
        }
        case TOKEN_TYPE_IDENTIFIER:
            left = new_variable_expression(*token.start);
            break;
        case TOKEN_TYPE_LEFT_PAREN:
            ++parser->missing_right_parens;
            return parse_expression_prec(parser, expression, OPERATOR_PRECEDENCE_MINIMUM);
        case TOKEN_TYPE_EOF:
            return ERROR_NO_ERROR;
        default:
            return ERROR_INVALID_TOKEN;
    }

    operator_t infix = *token.start;
    while (left && token.type != TOKEN_TYPE_EOF && precedence < operator_precedence(infix)) {

        expression_t* right_operand = new_expression();
        if (token.type == TOKEN_TYPE_LEFT_PAREN) {
            ++parser->missing_right_parens;
            err = parse_expression_prec(parser, right_operand, OPERATOR_PRECEDENCE_MINIMUM);
        } else {
            err = parse_expression_prec(parser, right_operand, operator_precedence(infix));
        }
        if (err) return err;

        memmove(&token, &parser->previous, sizeof(token_t));
        if (token.type == TOKEN_TYPE_RIGHT_PAREN) {
            if (parser->missing_right_parens <= 0)
                return ERROR_STRAY_RIGHT_PAREN;

            err = lexer_next(parser->lexer, &token);
            if (err) return err;
            --parser->missing_right_parens;
            precedence = 50;
        }

        left = new_operator_expression(left, infix, right_operand);
        infix = *token.start;
    }

    memmove(&parser->previous, &token, sizeof(token_t));
    *expression = *left;

    return ERROR_NO_ERROR;
}
