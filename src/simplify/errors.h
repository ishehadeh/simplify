// Copyright Ian R. Shehadeh 2018

#ifndef SIMPLIFY_ERRORS_H_
#define SIMPLIFY_ERRORS_H_

#include <string.h>
#include <inttypes.h>

typedef enum error error_t;

enum error {
    ERROR_NO_ERROR,
    ERROR_INVALID_TOKEN,
    ERROR_INVALID_CHARACTER,
    ERROR_INVALID_PREFIX,
    ERROR_INVALID_OPERATOR,
    ERROR_INVALID_NUMBER,
    ERROR_INVALID_IDENTIFIER,

    ERROR_FAILED_TO_ALLOCATE,
    ERROR_FAILED_TO_REALLOCATE,
    ERROR_UNABLE_TO_OPEN_FILE,
    ERROR_FILE_CLOSED,
    ERROR_NUMBER_IS_NAN,
    ERROR_NUMBER_IS_INFINITY,

    ERROR_NULL_EXPRESSION,
    ERROR_CANNOT_COMPARE,
    ERROR_UNEXPECTED_EOF,
    ERROR_STRAY_RIGHT_PAREN,
    ERROR_STRAY_LEFT_PAREN,
    ERROR_NONEXISTANT_KEY,
    ERROR_VARIABLE_NOT_PRESENT,
};

static inline const char* error_string(error_t err) {
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
            return "random right parentheses in expression";
        case ERROR_STRAY_LEFT_PAREN:
            return "random left parentheses in expression";
        case ERROR_NONEXISTANT_KEY:
            return "no value associated with key";
        case ERROR_VARIABLE_NOT_PRESENT:
            return "that variable was not found in this expression";
    }
}


#endif  // SIMPLIFY_ERRORS_H_
