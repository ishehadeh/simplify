/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_PARSER_H_
#define SIMPLIFY_PARSER_H_

#include "simplify/expression/expression.h"
#include "simplify/lexer.h"

/* transforms a stream of tokens into an expression.
 */
typedef struct expression_parser   expression_parser_t;

struct expression_parser {
    lexer_t* lexer;
    token_t  previous;
    int      missing_right_parens;
};

static inline void expression_parser_init(expression_parser_t* parser, lexer_t* lexer) {
    parser->lexer = lexer,
    parser->missing_right_parens = 0;
    lexer_next(lexer, &parser->previous);
}

/* clean all resources associated with a parser
 * @parser the parser to clean
 */
static inline void expression_parser_clean(expression_parser_t* parser) {
    (void)parser;
}

/* parse an expression 
 * NOTE: most of the time it's safer and easier to use the `parse_string` or `parse_file` functions
 *
 * @parser The parser used to draw the expression.
 * @result The expression that should be filled.
 * @return returns an error code
 */
error_t parser_parse_expression(expression_parser_t* parser, expression_t* result);

/* parses an expression from a string
 *
 * @source the string to parse
 * @result the expression to be filled
 * @return returns an error code
 */
error_t parse_string(char* source, expression_t* result);

/* parses an expression from a file
 *
 * @source the file to parse
 * @result the expression to be filled
 * @return returns an error code
 */
error_t parse_file(FILE* source, expression_list_t* result);


#endif  // SIMPLIFY_PARSER_H_
