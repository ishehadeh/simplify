#include "test.h"

int main() {
    mpc_t bad_number;
    mpc_t infinity;
    mpc_t neg_infinity;

    mpc_init(bad_number);
    mpc_init(infinity);
    mpc_init(neg_infinity);
    mpc_set_nan(bad_number);
    mpc_set_inf(infinity, 1);
    mpc_set_inf(neg_infinity, -1);

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
        { expression_new_number_d(3.9), "3.9" },
        { expression_new_number(infinity), "Inf" },
        { expression_new_number(bad_number), "NaN" },
        { expression_new_number(neg_infinity), "-Inf" },
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
