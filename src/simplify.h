#ifndef VALUE_OF_H_
#define VALUE_OF_H_

#include "scalar.h"
#include "expression.h"
#include "parser.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int yywrap(void) {
    return 1;
}

void expression_print(expression_t* expr) {
    if (!expr) {
        printf("(NULL)");
        return;
    }

    switch (expr->type) {
        case EXPRESSION_TYPE_NUMBER:
        {
            char* buf = alloca(256);
            buf[255] = 0;
            SCALAR_TO_STRING(expr->number.value, buf);
            printf("%s", buf);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            printf("%c", expr->variable.value);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            printf("( ");
            expression_print(expr->operator.left);
            printf(" %c ", expr->operator.infix);
            expression_print(expr->operator.right);
            printf(" )");
            break;
        case EXPRESSION_TYPE_PREFIX:
            printf("%c", expr->prefix.prefix);
            expression_print(expr->prefix.right);
            break;
        default:
            printf("(UNDEFINED TYPE %d)", expr->type);
            break;
    }
}


#endif  // VALUE_OF_H_