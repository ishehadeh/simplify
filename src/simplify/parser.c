// Copyright Ian R. Shehadeh

#include "simplify/parser.h"

inline static int operator_precedence(operator_t op) {
    switch (op) {
        case '=':
        case '>':
        case '<':
            return 10;
        case '+':
        case '-':
            return 20;
        case '*':
        case '/':
            return 30;
        case '^':
            return 40;
        default:
            return -1;
    }
}


expression_t* parse_expression_prec(lexer_t* stream, token_t* last, int precedence) {
    token_t tok = lexer_next(stream);

    expression_t* left;

    switch (tok.type) {
        case TOKEN_TYPE_OPERATOR:
        {
            operator_t operator = *tok.start;
            expression_t* operand = parse_expression_prec(stream, &tok, 50);
            if (!operand) return NULL;
            left = new_prefix_expression(operator, operand);
            break;
        }
        case TOKEN_TYPE_NUMBER:
        {
            scalar_t num;

            char* del = alloca(tok.length + 1);
            del[tok.length] = 0;
            strncpy(del, tok.start, tok.length);
            if (SCALAR_FROM_STRING(del, num)) {
                printf("ERROR: invalid number: %s\n", del);
                return NULL;
            }

            left = new_number_expression(num);
            SCALAR_CLEAN(num);
            tok = lexer_next(stream);
            break;
        }
        case TOKEN_TYPE_IDENTIFIER:
            left = new_variable_expression(*tok.start);
            break;
        case TOKEN_TYPE_EOF:
            return NULL;
        default:
            printf("ERROR: invalid token '%s', expeected operator number or variable", tok.start);
            return NULL;
    }

    operator_t infix = *tok.start;
    while (left && tok.type != TOKEN_TYPE_EOF && precedence < operator_precedence(infix)) {
        expression_t* op = parse_expression_prec(stream, &tok, operator_precedence(infix));
        if (!op) break;

        left = new_operator_expression(left, infix, op);

        infix = *tok.start;
    }

    if (last) *last = tok;

    return left;
}

expression_t* parse_expression(lexer_t* stream) {
    return parse_expression_prec(stream, NULL, 0);
}
