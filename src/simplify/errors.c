// Copyright Ian R. Shehadeh 2018

#include <stdio.h>

#include "simplify/errors.h"

const char* error_string(error_t err) {
    switch (err) {
        case ERROR_NO_ERROR:
            return "no error";
        case ERROR_INVALID_TOKEN:
            return "invalid token";
        case ERROR_INVALID_CHARACTER:
            return "invalid character";
        case ERROR_INVALID_PREFIX:
            return "invalid prefix operator";
        case ERROR_INVALID_OPERATOR:
            return "invalid operator";
        case ERROR_INVALID_NUMBER:
            return "invalid number";
        case ERROR_INVALID_IDENTIFIER:
            return "invalid identifier";
        case ERROR_FAILED_TO_ALLOCATE:
            return "failed to allocate memory";
        case ERROR_FAILED_TO_REALLOCATE:
            return "failed to reallocate buffer";
        case ERROR_UNABLE_TO_OPEN_FILE:
            return "failed to open file";
        case  ERROR_FILE_CLOSED:
            return "failed to close file";
        case ERROR_NUMBER_IS_NAN:
            return "number is NaN";
        case ERROR_NUMBER_IS_INFINITY:
            return "number is Infinity";
        case ERROR_NULL_EXPRESSION:
            return "invalid (null) expression";
        case ERROR_CANNOT_COMPARE:
            return "cannot compare values";
        case ERROR_UNEXPECTED_EOF:
            return "unexpected end of equation";
        case ERROR_STRAY_RIGHT_PAREN:
            return "random right parenthese in expression";
        case ERROR_STRAY_LEFT_PAREN:
            return "random left parenthese in expression";
    }
}
