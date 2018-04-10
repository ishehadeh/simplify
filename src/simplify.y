%{
    #include "simplify.h"

    token_stream_t __token_stream;
%}

DIGIT    [0-9]
VARIABLE [a-zA-Z]
OPERATOR [*\-+/=^]
WHITESPACE [\n\t\v ]

%%

{DIGIT}+ {
    token_stream_push(&__token_stream, TOKEN_TYPE_NUMBER, yytext, yyleng);
}

{DIGIT}*"."{DIGIT}+ {
    token_stream_push(&__token_stream, TOKEN_TYPE_NUMBER, yytext, yyleng);
}

{OPERATOR} {
    token_stream_push(&__token_stream, TOKEN_TYPE_OPERATOR, yytext, yyleng);
}

{VARIABLE} {
    token_stream_push(&__token_stream, TOKEN_TYPE_IDENTIFIER, yytext, yyleng);
}

{WHITESPACE} {

}

%%

int main(int argc, char** argv) {
    (void)input;
    (void)yyunput;

#if defined(SCALAR_FLOAT)
#   if defined(HAVE_MPFR)
        mpfr_set_default_prec(FLOAT_PRECISION);
#   elif defined(HAVE_GMP)
        mpf_set_default_prec(FLOAT_PRECISION);
#   endif
#endif

    if (argc > 1) {
        yy_scan_string(argv[1]);
    } else {
        yyin = stdin;
    }

    if (token_stream_init(&__token_stream)) {
        printf("ERROR: failed to initialize parser.\n");
        return 1;
    }

    yylex();

    expression_t* expr = parse_expression(&__token_stream);

    if (!expr) {
        return 1;
    }

    int retcode = expression_simplify(expr);
    if(!retcode)
        expression_print(expr);

    puts("");

    token_stream_clean(&__token_stream);
    expression_free(expr);
    yylex_destroy();

#if defined(HAVE_MPFR) && !defined(SCALAR_INTEGER)
    mpfr_free_cache();
#endif

    return retcode;
}
