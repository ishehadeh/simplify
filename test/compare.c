#include "simplify/expression/evaluate.h"
#include "simplify/expression/expression.h"
#include "simplify/expression/isolate.h"
#include "simplify/expression/simplify.h"
#include "simplify/expression/stringify.h"
#include "test.h"

static struct {
    char* expr1;
    char* expr2;
    compare_result_t result;
} __expr_result_pairs[] = {
    {"2", "4", COMPARE_RESULT_LESS},
    {"10", "10", COMPARE_RESULT_EQUAL},
    {"10", "-10", COMPARE_RESULT_GREATER},
    {"-0.2", "0.2", COMPARE_RESULT_LESS},
    {"-0.0002", "-1", COMPARE_RESULT_GREATER},
    {"-5", "-6", COMPARE_RESULT_GREATER},
    {"10", "1", COMPARE_RESULT_GREATER},
    {"-2", "-2", COMPARE_RESULT_EQUAL},
    {"x + 5", "1", COMPARE_RESULT_INCOMPARABLE},
    {"2 * x", "2x", COMPARE_RESULT_EQUAL},
    {"2.25", "2.2", COMPARE_RESULT_GREATER},
    {"2.0003", "2.2", COMPARE_RESULT_LESS},
    {".0001", "0.00002", COMPARE_RESULT_GREATER},
    {"1.000001", "1.00002", COMPARE_RESULT_LESS},
    {"5e2", "5e3", COMPARE_RESULT_LESS},
    {"y * (x2) + x^4", "x^4 + y * (2 * x)", COMPARE_RESULT_EQUAL},
    {"(y)f(x + 5)", "f(x + 5) * y", COMPARE_RESULT_EQUAL},
    {"x * x * x", "x ^ 2 * x", COMPARE_RESULT_EQUAL},
};

int main() {
    error_t err;
    for (int i = 0; i < (int)(sizeof(__expr_result_pairs) / sizeof(__expr_result_pairs[0])); ++i) {
        expression_t expr1;
        expression_t expr2;
        scope_t scope;

        printf("starting test #%d...", i + 1);
        err = parse_string(__expr_result_pairs[i].expr1, &expr1);
        if (err) FATAL("failed to parse expression 1/2 \"%s\": %s", __expr_result_pairs[i].expr1, error_string(err));

        err = parse_string(__expr_result_pairs[i].expr2, &expr2);
        if (err) FATAL("failed to parse expression 2/2 \"%s\": %s", __expr_result_pairs[i].expr2, error_string(err));

        scope_init(&scope);

        err = expression_evaluate(&expr1, &scope);
        if (err) FATAL("failed to evaluate 1/2 \"%s\": %s", __expr_result_pairs[i].expr1, error_string(err));

        err = expression_evaluate(&expr2, &scope);
        if (err) FATAL("failed to evaluate 2/2 \"%s\": %s", __expr_result_pairs[i].expr2, error_string(err));

        err = expression_simplify(&expr1);
        if (err) FATAL("failed to simplify expression 1/2 \"%s\": %s", __expr_result_pairs[i].expr1, error_string(err));

        err = expression_simplify(&expr2);
        if (err) FATAL("failed to simplify expression 2/2 \"%s\": %s", __expr_result_pairs[i].expr2, error_string(err));

        compare_result_t cmp = expression_compare(&expr1, &expr2);
        if (cmp != __expr_result_pairs[i].result) {
            FATAL("Test failed, expected the relationship of '%s' and '%s' to be %s, got %s",
                  __expr_result_pairs[i].expr1, __expr_result_pairs[i].expr2,
                  print_compare_result(__expr_result_pairs[i].result), print_compare_result(cmp));
        }
        scope_clean(&scope);
        expression_clean(&expr1);
        expression_clean(&expr2);
        printf("done\n");
    }
}
