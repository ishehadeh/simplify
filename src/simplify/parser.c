/* Copyright Ian Shehadeh 2018 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "simplify/parser.h"
#include "simplify/errors.h"
error_t _parser_parse_expression_precedence_recursive(expression_parser_t* parser,
                                                      expression_t* expression,
                                                      operator_precedence_t precedence);
error_t parse_string(char* source, expression_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_string(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = parser_parse_expression(&parser, result);
    lexer_clean(&lexer);
    expression_parser_clean(&parser);
    return err;
}

error_t parse_file(FILE* source, expression_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_file(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = parser_parse_expression(&parser, result);
    lexer_clean(&lexer);
    expression_parser_clean(&parser);
    return err;
}

error_t _parser_parse_expression_list(expression_parser_t* parser, expression_list_t* list) {
    expression_t* next = malloc(sizeof(expression_t));
    error_t err = _parser_parse_expression_precedence_recursive(parser, next, OPERATOR_PRECEDENCE_ASSIGN);
    if (err) return err;
    expression_list_append(list, next);

    while (parser->previous.type == TOKEN_TYPE_COMMA) {
        lexer_next(parser->lexer, &parser->previous);
        expression_t* next = malloc(sizeof(expression_t));
        error_t err = _parser_parse_expression_precedence_recursive(parser, next, OPERATOR_PRECEDENCE_ASSIGN);
        if (err) return err;
        expression_list_append(list, next);
    }
    return ERROR_NO_ERROR;
}


/* _parse_expression_precedence_recursive parses an expression recursively.
 * it expects a `precedence` parameter, which should only be used internally while recursing.
 */
error_t _parser_parse_expression_precedence_recursive(expression_parser_t* parser,
                                                      expression_t* expression,
                                                      operator_precedence_t precedence) {
    token_t token;
    error_t err = lexer_next(parser->lexer, &token);
    if (err) return err;
    expression_t* left = malloc(sizeof(expression_t));

    switch (token.type) {
        case TOKEN_TYPE_OPERATOR:
        {
            operator_t operator = *token.start;
            expression_t* operand = malloc(sizeof(expression_t));

            error_t err = _parser_parse_expression_precedence_recursive(parser, operand, OPERATOR_PRECEDENCE_MAXIMUM);
            if (err) return err;

            expression_init_prefix(left, operator, operand);
            memmove(&token, &parser->previous, sizeof(token_t));
            break;
        }
        case TOKEN_TYPE_NUMBER:
        {
            mpfr_t num;

            char del[token.length + 1];
            del[token.length] = 0;
            strncpy(del, token.start, token.length);
            mpfr_init2(num, token.length + (1 << 6));
            mpfr_set_str(num, del, 10, MPFR_RNDF);

            expression_init_number(left, num);

            mpfr_clear(num);
            err = lexer_next(parser->lexer, &token);
            if (err) {
                expression_free(left);
                return err;
            }
            break;
        }
        case TOKEN_TYPE_IDENTIFIER:
        {
            char* name = token.start;
            size_t len = token.length;

            err = lexer_next(parser->lexer, &token);
            if (token.type == TOKEN_TYPE_LEFT_PAREN) {
                expression_list_t* args = malloc(sizeof(expression_list_t));
                expression_list_init(args);
                error_t err = _parser_parse_expression_list(parser, args);
                ++parser->missing_right_parens;
                if (err) {
                    expression_list_free(args);
                    return err;
                }
                expression_init_function(left, name, len, args);
                token = parser->previous;
            } else {
                expression_init_variable(left, name, len);
            }
            if (err) {
                expression_free(left);
                return err;
            }
            break;
        }
        case TOKEN_TYPE_LEFT_PAREN:
            ++parser->missing_right_parens;
            err = _parser_parse_expression_precedence_recursive(parser, left, OPERATOR_PRECEDENCE_MINIMUM);
            if (err) return err;
            memmove(&token, &parser->previous, sizeof(token_t));
            break;
        case TOKEN_TYPE_EOF:
            return ERROR_NO_ERROR;
        default:
            return ERROR_INVALID_TOKEN;
    }

    if (token.type == TOKEN_TYPE_EOF)
        goto cleanup;

    operator_t infix = *token.start;

    if (token.type == TOKEN_TYPE_RIGHT_PAREN && precedence >= operator_precedence(infix)) {
        --parser->missing_right_parens;
        parser->previous = token;
        lexer_next(parser->lexer, &token);
        goto cleanup;
    }

    while (left && precedence < operator_precedence(infix)) {
        expression_t* right_operand = malloc(sizeof(expression_t));
        if (token.type == TOKEN_TYPE_LEFT_PAREN) {
            ++parser->missing_right_parens;
            err = _parser_parse_expression_precedence_recursive(parser, right_operand, OPERATOR_PRECEDENCE_MINIMUM);
        } else if (token.type == TOKEN_TYPE_COMMA) {
            goto cleanup;
        } else {
            err = _parser_parse_expression_precedence_recursive(parser, right_operand, operator_precedence(infix));
        }
        if (err) return err;

        token = parser->previous;

        expression_t* new_left = malloc(sizeof(expression_t));
        expression_init_operator(new_left, left, infix, right_operand);
        left = new_left;

        if (token.type == TOKEN_TYPE_RIGHT_PAREN) {
            --parser->missing_right_parens;
            parser->previous = token;
            lexer_next(parser->lexer, &token);
            goto cleanup;
        } else if (token.type == TOKEN_TYPE_EOF) {
            goto cleanup;
        }

        infix = *token.start;
    }

cleanup:
    memmove(&parser->previous, &token, sizeof(token_t));
    memmove(expression, left, sizeof(expression_t));
    free(left);

    return ERROR_NO_ERROR;
}

error_t parser_parse_expression(expression_parser_t* parser, expression_t* result) {
    error_t err = _parser_parse_expression_precedence_recursive(parser, result, OPERATOR_PRECEDENCE_MINIMUM);
    if (err) return err;

    if (parser->missing_right_parens > 0)
        return ERROR_STRAY_LEFT_PAREN;
    else if (parser->missing_right_parens < 0)
        return ERROR_STRAY_RIGHT_PAREN;
    return ERROR_NO_ERROR;
}
