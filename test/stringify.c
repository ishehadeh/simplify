#include "test.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/stringify.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/simplify.h"
#include "simplify/expression/evaluate.h"


int main() {
    struct {
        expression_t* expr;
        char*         string;
    } __expr_string_pairs[] = {
        { expression_new_number_si(5), "5"},
        { expression_new_number_d(2.25), "2.25"},
        { expression_new_operator(
            expression_new_function("f", 2, expression_new_number_d(5), expression_new_variable("x")),
            '*',
            expression_new_variable("y")),
        "f(5, x) * y"
        },
        { expression_new_operator(
            expression_new_prefix('+', expression_new_variable("OwO")),
            '-',
            expression_new_number_si(3)),
        "+OwO - 3"
        },
    };

    for (int i = 0; i < (int) (sizeof(__expr_string_pairs) / sizeof(__expr_string_pairs[0])); ++i) {
        printf("starting test #%d...", i + 1);
        char* str = stringify(__expr_string_pairs[i].expr);
        if (strcmp(str, __expr_string_pairs[i].string) != 0) {
            FATAL("strings do not match! expecting string '%s' got '%s'", __expr_string_pairs[i].string, str);
        }
        free(str);
        printf("done\n");
    }
}