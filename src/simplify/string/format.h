/* Copyright Ian Shehadeh 2018 */

#ifndef SIMPLIFY_STRING_FORMAT_H_
#define SIMPLIFY_STRING_FORMAT_H_

#include <stdbool.h>
#include <stdlib.h>

/* a string format holds information about how to transform an expression into a string */
typedef struct string_format string_format_t;

typedef enum string_format_type string_format_type_t;

enum string_format_type {
    STRING_FORMAT_ASCII,
    STRING_FORMAT_LATEX,
    STRING_FORMAT_HTML,
};

struct string_format {
    char* whitespace;
    char* infinity;
    char* not_a_number;
    char* imaginary;

    size_t approximate_tolerance; /* zero to disable approximation */

    bool brief_multiplication; /* x * 1 => x1 */
    bool force_brief_multiplication; /* 1 * 2 => 1(2) */
    bool omit_parmeter_parentheses; /* f(x, y, z) => f x,y,z */
    bool omit_parameter_commas; /* f(x, y, z) => f(xyz) */
    string_format_type_t format;
};

/* get a string format with the default settings. */
#define STRING_FORMAT_DEFAULT()          \
{                                        \
    .approximate_tolerance = 5,          \
    .not_a_number = "NaN",               \
    .infinity = "Inf",                   \
    .imaginary = "i",                    \
    .whitespace = " ",                   \
    .omit_parameter_commas = false,      \
    .omit_parmeter_parentheses = false,  \
    .force_brief_multiplication = true,  \
    .brief_multiplication = true,        \
    .format = STRING_FORMAT_ASCII        \
}

#endif  // SIMPLIFY_STRING_FORMAT_H_
