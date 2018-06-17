/* Copyright Ian Shehadeh 2018 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "simplify/errors.h"
#include "simplify/parser.h"
error_t _expression_parser_parse_precedence_recursive(expression_parser_t* parser, expression_t* expression,
                                                      operator_precedence_t precedence);

error_t _expression_parser_parse_list_precedence(expression_parser_t* parser, expression_list_t* list,
                                                 operator_precedence_t precedence);

/* get the next token from the parser's internal lexer
 *
 * @parser
 * @out token output, this may be null
 * @return returns an error code
 */
static inline error_t _parser_next_token(expression_parser_t* parser, token_t* out) {
    if (out) *out = parser->previous;
    return lexer_next(parser->lexer, &parser->previous);
}

/* grab the parser's cached token
 *
 * @parser
 * @out token output
 */
static inline void _parser_peek_token(expression_parser_t* parser, token_t* out) { *out = parser->previous; }

error_t parse_string(char* source, expression_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_string(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = expression_parser_parse(&parser, result);
    expression_parser_clean(&parser);
    lexer_clean(&lexer);
    return err;
}

error_t parse_file(FILE* source, expression_list_t* result) {
    lexer_t lexer;
    expression_parser_t parser;

    lexer_init_from_file(&lexer, source);
    expression_parser_init(&parser, &lexer);

    error_t err = _expression_parser_parse_list_precedence(&parser, result, OPERATOR_PRECEDENCE_MINIMUM);
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
    expr->number.value = malloc(sizeof(mpc_t));
    mpc_init2(expr->number.value, 256);
    mpc_set_str(expr->number.value, &number_buffer[0], 10, MPC_RNDNN);

    return ERROR_NO_ERROR;
}

error_t _parser_parse_prefix_operator(expression_parser_t* parser, expression_t* expr) {
    expr->type = EXPRESSION_TYPE_PREFIX;
    token_t token;
    error_t err;

    err = _parser_next_token(parser, &token);
    if (err) return err;

    expr->prefix.prefix = *token.start;
    expr->prefix.right = malloc(sizeof(expression_t));

    return _expression_parser_parse_precedence_recursive(parser, expr->prefix.right, OPERATOR_PRECEDENCE_MAXIMUM);
}

error_t _expression_parser_parse_list_precedence(expression_parser_t* parser, expression_list_t* list,
                                                 operator_precedence_t precedence) {
    expression_t* next = malloc(sizeof(expression_t));
    error_t err = _expression_parser_parse_precedence_recursive(parser, next, precedence);
    if (err) return err;
    expression_list_append(list, next);

    while (parser->previous.type == TOKEN_TYPE_COMMA) {
        _parser_next_token(parser, NULL);
        expression_t* next = malloc(sizeof(expression_t));
        error_t err = _expression_parser_parse_precedence_recursive(parser, next, precedence);
        if (err) return err;
        expression_list_append(list, next);
    }
    return ERROR_NO_ERROR;
}

error_t _parser_parse_parentheses(expression_parser_t* parser, expression_t* expr) {
    token_t token;
    error_t err;

    err = expression_parser_parse(parser, expr);
    if (err) return err;

    err = _parser_next_token(parser, &token);
    if (err) return err;

    if (token.type != TOKEN_TYPE_RIGHT_PAREN) err = ERROR_STRAY_LEFT_PAREN;

    return err;
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

        token_t tok;
        err = _parser_next_token(parser, NULL);
        if (err) return err;
        _parser_peek_token(parser, &tok);

        if (tok.type != TOKEN_TYPE_RIGHT_PAREN)
            err =
                _expression_parser_parse_list_precedence(parser, expr->function.parameters, OPERATOR_PRECEDENCE_ASSIGN);
        _parser_next_token(parser, NULL);
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
error_t _expression_parser_parse_precedence_recursive(expression_parser_t* parser, expression_t* expression,
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
        case TOKEN_TYPE_LEFT_PAREN: {
            // when a left paren is hit, try to parse a sub-expression
            err = _parser_next_token(parser, NULL);
            if (err) goto cleanup;

            err = _parser_parse_parentheses(parser, left);
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
            expression_t* new_left = malloc(sizeof(expression_t));
            expression_t* subexpr = malloc(sizeof(expression_t));

            err = _parser_next_token(parser, NULL);
            if (err) goto cleanup;

            err = _parser_parse_parentheses(parser, subexpr);
            expression_init_operator(new_left, left, '*', subexpr);
            left = new_left;
            err = _parser_next_token(parser, &token);
            if (err) goto cleanup;
            if (token.type == TOKEN_TYPE_EOF) goto cleanup;

            infix = *token.start;
        } else if (token.type == TOKEN_TYPE_NUMBER || token.type == TOKEN_TYPE_IDENTIFIER) {
            infix = '*';
        } else if (token.type == TOKEN_TYPE_OPERATOR) {
            infix = *token.start;
            err = _parser_next_token(parser, &token);
        } else {
            return ERROR_INVALID_TOKEN;
        }
        if (err) goto cleanup;

        expression_t* right_operand = malloc(sizeof(expression_t));
        err = _expression_parser_parse_precedence_recursive(parser, right_operand, operator_prec);
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

cleanup:
    *expression = *left;
    free(left);
    return err;
}

error_t expression_parser_parse(expression_parser_t* parser, expression_t* result) {
    return _expression_parser_parse_precedence_recursive(parser, result, OPERATOR_PRECEDENCE_MINIMUM);
}
