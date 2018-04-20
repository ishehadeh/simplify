/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_ERRORS_H_
#define SIMPLIFY_ERRORS_H_

#include <string.h>
#include <inttypes.h>

/* Enumerates all errors that may occur.
 * Error also has the value `ERROR_NO_ERROR`, meaning no error occurred. This value
 * will always be zero, so it's safe to check if an error occurred with `if (error) { ... }`.
 */
typedef enum error error_t;

enum error {
    ERROR_NO_ERROR = 0,
    ERROR_INVALID_TOKEN,
    ERROR_INVALID_CHARACTER,
    ERROR_INVALID_PREFIX,
    ERROR_INVALID_OPERATOR,
    ERROR_INVALID_NUMBER,
    ERROR_INVALID_IDENTIFIER,

    ERROR_FAILED_TO_ALLOCATE,
    ERROR_FAILED_TO_REALLOCATE,

    ERROR_UNABLE_TO_OPEN_FILE,
    ERROR_FILE_CLOSED_UNEXPECTEDLY,

    ERROR_NUMBER_IS_NAN,
    ERROR_NUMBER_IS_INFINITY,

    ERROR_CANNOT_COMPARE,

    ERROR_UNEXPECTED_EOF,
    ERROR_UNEXPECTED_EOL,
    ERROR_STRAY_RIGHT_PAREN,
    ERROR_STRAY_LEFT_PAREN,

    ERROR_NONEXISTANT_KEY,
    ERROR_VARIABLE_NOT_PRESENT,

    ERROR_INVALID_ASSIGNMENT_EXPRESSION,
    ERROR_UNRECOGNIZED_ARGUMENT,
};

/* get a description of the error
 *
 * @err the error to describe
 * @return returns a static string describing the error
 */
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
        case ERROR_FILE_CLOSED_UNEXPECTEDLY:
            return "failed to close file";
        case ERROR_NUMBER_IS_NAN:
            return "number is NaN";
        case ERROR_NUMBER_IS_INFINITY:
            return "number is Infinity";
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
        case ERROR_INVALID_ASSIGNMENT_EXPRESSION:
            return "expected an expression in the form of VARIABLE=EXPRESSION";
        case ERROR_UNRECOGNIZED_ARGUMENT:
            return "invalid command line argument";
        case ERROR_UNEXPECTED_EOL:
            return "line ended prematurely";
    }
    return "unkown error type";
}


#endif  // SIMPLIFY_ERRORS_H_
