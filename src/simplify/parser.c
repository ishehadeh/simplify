/* Copyright Ian Shehadeh 2018 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "simplify/parser.h"
#include "simplify/errors.h"
error_t _parser_parse_expression_precedence_recursive(expression_parser_t* parser,
                                                      expression_t* expression,
                                                      operator_precedence_t precedence);

error_t _parser_parse_expression_list_precedence(expression_parser_t* parser,
                                                 expression_list_t* list,
                                                 operator_precedence_t precedence);

/* get the next token from the parser's internal lexer
 *
 * @parser
 * @out token output, this may be null
 * @return returns an error code
 */
inline error_t _parser_next_token(expression_parser_t* parser, token_t* out) {
    if (out)
        *out = parser->previous;
    return lexer_next(parser->lexer, &parser->previous);
}

/* grab the parser's cached token
 *
 * @parser
 * @out token output
 */
inline void _parser_peek_token(expression_parser_t* parser, token_t* out) {
    *out = parser->previous;
}

error_t parse_string(char* source, expression_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_string(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = parser_parse_expression(&parser, result);
    expression_parser_clean(&parser);
    lexer_clean(&lexer);
    return err;
}

error_t parse_file(FILE* source, expression_list_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_file(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = _parser_parse_expression_list_precedence(&parser, result, OPERATOR_PRECEDENCE_MINIMUM);
    expression_parser_clean(&parser);
    lexer_clean(&lexer);
    return err;
}

error_t _parser_parse_number(expression_parser_t* parser, expression_t* expr) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    token_t token;
    error_t err;

    err = _parser_next_token(parser, &token);
    if (err) return err;

    char number_buffer[token.length + 1];
    number_buffer[token.length] = 0;

    strncpy(&number_buffer[0], token.start, token.length);
    mpfr_init2(expr->number.value, token.length + (1 << 6));
    mpfr_set_str(expr->number.value, &number_buffer[0], 10, MPFR_RNDF);

    return ERROR_NO_ERROR;
}

error_t _parser_parse_prefix_operator(expression_parser_t* parser, expression_t* expr) {
    expr->type = EXPRESSION_TYPE_PREFIX;
    token_t       token;
    error_t       err;

    err = _parser_next_token(parser, &token);
    if (err) return err;

    expr->prefix.prefix = *token.start;
    expr->prefix.right = malloc(sizeof(expression_t));

    return _parser_parse_expression_precedence_recursive(parser, expr->prefix.right, OPERATOR_PRECEDENCE_MAXIMUM);
}


error_t _parser_parse_expression_list_precedence(expression_parser_t* parser,
                                                 expression_list_t* list,
                                                 operator_precedence_t precedence) {
    expression_t* next = malloc(sizeof(expression_t));
    error_t err = _parser_parse_expression_precedence_recursive(parser, next, precedence);
    if (err) return err;
    expression_list_append(list, next);
    while (parser->previous.type == TOKEN_TYPE_COMMA) {

        _parser_next_token(parser, NULL);
        expression_t* next = malloc(sizeof(expression_t));
        error_t err = _parser_parse_expression_precedence_recursive(parser, next, precedence);
        if (err) return err;
        expression_list_append(list, next);
    }
    return ERROR_NO_ERROR;
}


error_t _parser_parse_identifier(expression_parser_t* parser, expression_t* expr) {
    token_t identifier;
    token_t maybe_left_paren;
    error_t err;

    err = _parser_next_token(parser, &identifier);
    if (err) return err;

    _parser_peek_token(parser, &maybe_left_paren);
    // if the identifier is followed by a left-paren then assume it's a function
    if (maybe_left_paren.type == TOKEN_TYPE_LEFT_PAREN) {
        expr->type = EXPRESSION_TYPE_FUNCTION;
        expr->function.parameters = malloc(sizeof(expression_list_t));

        expression_list_init(expr->function.parameters);

        // since a left paren was hit let the parser know there should be a right paren coming up
        ++parser->missing_right_parens;
        err = _parser_next_token(parser, NULL);
        if (err) return err;

        error_t err = _parser_parse_expression_list_precedence(parser,
                                                               expr->function.parameters,
                                                               OPERATOR_PRECEDENCE_ASSIGN);
        if (err) {
            expression_list_free(expr->function.parameters);
            return err;
        }

        // copy the function's name from the underlying buffer
        expr->function.name = malloc(identifier.length + 1);
        expr->function.name[identifier.length] = 0;
        strncpy(expr->function.name, identifier.start, identifier.length);
    } else {
        expr->type = EXPRESSION_TYPE_VARIABLE;

        // copy the variable's name from the underlying buffer
        expr->variable.value = malloc(identifier.length + 1);
        expr->variable.value[identifier.length] = 0;
        expr->variable.binding = NULL;
        strncpy(expr->variable.value, identifier.start, identifier.length);
    }

    return err;
}


/* _parse_expression_precedence_recursive parses an expression recursively.
 * 
 * @parser the parser to draw from
 * @expression the parse result
 * @precedence The minimum operator precedence where this function will recurse.
 * @return returns an error code
 */
error_t _parser_parse_expression_precedence_recursive(expression_parser_t* parser,
                                                      expression_t* expression,
                                                      operator_precedence_t precedence) {
    token_t token;
    error_t err;

    _parser_peek_token(parser, &token);
    expression_t* left = malloc(sizeof(expression_t));
    switch (token.type) {
        case TOKEN_TYPE_OPERATOR:
            err = _parser_parse_prefix_operator(parser, left);
            break;
        case TOKEN_TYPE_NUMBER:
            err = _parser_parse_number(parser, left);
            break;
        case TOKEN_TYPE_IDENTIFIER:
            err = _parser_parse_identifier(parser, left);
            break;
        case TOKEN_TYPE_LEFT_PAREN:
        {
            // when a left paren is hit, try parse a sub-expression
            ++parser->missing_right_parens;
            err = _parser_next_token(parser, NULL);
            if (err) goto cleanup;
            err = parser_parse_expression(parser, left);
            break;
        }
        case TOKEN_TYPE_EOF:
            err = ERROR_UNEXPECTED_EOF;
            goto cleanup;
        case TOKEN_TYPE_COMMA:
            err = ERROR_UNEXPECTED_EOL;
            goto cleanup;
        default:
            err = ERROR_INVALID_TOKEN;
            goto cleanup;
    }
    if (err) goto cleanup;

    _parser_peek_token(parser, &token);
    if (token.type == TOKEN_TYPE_EOF) {
        goto cleanup;
    }

    operator_precedence_t operator_prec = operator_precedence(*token.start);
    while (left && precedence < operator_prec) {
        operator_t infix;

        // comma's are the end-of-expression delemiter, so if the parser hits one exit immediately
        if (token.type == TOKEN_TYPE_COMMA) {
            goto cleanup;
        }

        // left parens can be used as a multiplication operator
        if (token.type == TOKEN_TYPE_LEFT_PAREN) {
            ++parser->missing_right_parens;
            infix = '*';
        } else {
            infix = *token.start;
        }

        err = _parser_next_token(parser, &token);
        if (err) goto cleanup;

        expression_t* right_operand = malloc(sizeof(expression_t));
        err = _parser_parse_expression_precedence_recursive(parser, right_operand, operator_prec);
        if (err) {
            free(right_operand);
            goto cleanup;
        }

        _parser_peek_token(parser, &token);

        expression_t* new_left = malloc(sizeof(expression_t));
        expression_init_operator(new_left, left, infix, right_operand);
        left = new_left;

        if (token.type == TOKEN_TYPE_EOF) goto cleanup;
        operator_prec = operator_precedence(*token.start);
    }

    if (token.type == TOKEN_TYPE_RIGHT_PAREN) {
        --parser->missing_right_parens;
        _parser_next_token(parser, &token);
        goto cleanup;
    }

cleanup:
    *expression = *left;
    free(left);
    return err;
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
