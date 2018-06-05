/* Copyright Ian Shehadeh 2018 */

#include "simplify/errors.h"
#include "simplify/expression/expr_types.h"
#include "simplify/expression/evaluate.h"

void expression_init_operator(expression_t* expr,  expression_t* left, operator_t op, expression_t* right) {
    expr->type = EXPRESSION_TYPE_OPERATOR;
    expr->operator.infix = op;
    expr->operator.right = right;
    expr->operator.left  = left;
}

void expression_init_prefix(expression_t* expr, operator_t op, expression_t* right) {
    expr->type = EXPRESSION_TYPE_PREFIX;
    expr->prefix.prefix = op;
    expr->prefix.right  = right;
}

void expression_init_variable(expression_t* expr, char* name, size_t len) {
    expr->type = EXPRESSION_TYPE_VARIABLE;
    expr->variable.value = malloc(len + 1);
    expr->variable.value[len] = 0;
    expr->variable.binding = NULL;
    strncpy(expr->variable.value, name, len);
}

void expression_init_number(expression_t* expr, mpc_ptr value) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    expr->number.value = value;
}

void expression_init_number_d(expression_t* expr, double value) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    expr->number.value = malloc(sizeof(mpc_t));
    mpc_init2(expr->number.value, 52);
    mpc_set_d(expr->number.value, value, MPC_RNDNN);
}

void expression_init_number_si(expression_t* expr, long value) {
    expr->type = EXPRESSION_TYPE_NUMBER;
    expr->number.value = malloc(sizeof(mpc_t));
    mpc_init2(expr->number.value, 52);
    mpc_set_si(expr->number.value, value, MPC_RNDNN);
}

void expression_init_function(expression_t* expr, char* name, size_t len, expression_list_t* params) {
    expr->type = EXPRESSION_TYPE_FUNCTION;
    expr->function.name = malloc(len + 1);
    expr->function.name[len] = 0;
    strncpy(expr->function.name, name, len);
    expr->function.parameters = params;
}

void expression_clean(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_TYPE_PREFIX:
            expression_clean(expr->prefix.right);
            free(expr->prefix.right);
            break;
        case EXPRESSION_TYPE_OPERATOR:
            expression_clean(expr->operator.left);
            expression_clean(expr->operator.right);
            free(expr->operator.left);
            free(expr->operator.right);
            break;
        case EXPRESSION_TYPE_NUMBER:
            mpc_clear(expr->number.value);
            free(expr->number.value);
            break;
        case EXPRESSION_TYPE_VARIABLE:
            free(expr->variable.value);
            break;
        case EXPRESSION_TYPE_FUNCTION:
            free(expr->function.name);
            expression_list_free(expr->function.parameters);
            break;
        default:
            break;
    }
}

void expression_free(expression_t* expr) {
    expression_clean(expr);
    free(expr);
}

void expression_copy(expression_t* expr, expression_t* out) {
    switch (expr->type) {
        case EXPRESSION_TYPE_PREFIX:
        {
            expression_t* right = (expression_t*)malloc(sizeof(expression_t));
            expression_copy(expr->prefix.right, right);
            expression_init_prefix(out, expr->prefix.prefix, right);
            break;
        }
        case EXPRESSION_TYPE_OPERATOR:
        {
            expression_t* right = (expression_t*)malloc(sizeof(expression_t));
            expression_t* left = (expression_t*)malloc(sizeof(expression_t));
            expression_copy(expr->operator.right, right);
            expression_copy(expr->operator.left,  left);
            expression_init_operator(out, left, expr->operator.infix, right);
            break;
        }
        case EXPRESSION_TYPE_NUMBER:
        {
            mpc_ptr copy = malloc(sizeof(mpc_t));
            mp_prec_t real;
            mp_prec_t imag;
            mpc_get_prec2(&real, &imag, expr->number.value);
            mpc_init3(copy, real, imag);
            mpc_set(copy, expr->number.value, MPC_RNDNN);
            expression_init_number(out, copy);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
            expression_init_variable(out, expr->variable.value, strlen(expr->variable.value));
            out->variable.binding = expr->variable.binding;
            break;
        case EXPRESSION_TYPE_FUNCTION:
        {
            expression_list_t* params = malloc(sizeof(expression_list_t));
            expression_list_init(params);
            expression_list_copy(expr->function.parameters, params);
            expression_init_function(out, expr->function.name, strlen(expr->function.name), params);
            break;
        }

        default:
            memcpy(expr, out, sizeof(expression_t));
            break;
    }
}

inline operator_precedence_t operator_precedence(operator_t op) {
    switch (op) {
        case '=':
        case '>':
        case '<':
            return OPERATOR_PRECEDENCE_COMPARE;
        case '+':
        case '-':
            return OPERATOR_PRECEDENCE_SUM;
        case '*':
        case '/':
            return OPERATOR_PRECEDENCE_PRODUCT;
        case '^':
        case '\\':
            return OPERATOR_PRECEDENCE_EXPONENT;
        case '(':
            return OPERATOR_PRECEDENCE_MAXIMUM;
        case ')':
            return OPERATOR_PRECEDENCE_MINIMUM;
        case ':':
            return OPERATOR_PRECEDENCE_ASSIGN;
        // numbers and identifiers aren't operators, but it's legal to omit the '*' operator, so we pretend they are.
        case '0'...'9':
        case 'a'...'z':
        case 'A'...'Z':
        case '_':
        case '.':
            return OPERATOR_PRECEDENCE_PRODUCT;
        default:
            return OPERATOR_PRECEDENCE_MINIMUM;
    }
}

void expression_list_free(expression_list_t* list) {
    expression_list_t* last = NULL;
    while (list) {
        if (list->value) expression_free(list->value);
        last = list;
        list = list->next;
        free(last);
    }
    free(list);
}

void expression_list_append(expression_list_t* list, expression_t* expr) {
    if (!list->value) {
        list->value = expr;
        return;
    }

    expression_list_t* next = malloc(sizeof(expression_list_t));
    next->value = expr;
    next->next  = NULL;
    while (list->next) list = list->next;

    list->next = next;
}

void expression_list_copy(expression_list_t* list1, expression_list_t* list2) {
    expression_t* expr;
    EXPRESSION_LIST_FOREACH(expr, list1) {
        expression_t* copy = malloc(sizeof(expression_t));
        expression_copy(expr, copy);
        expression_list_append(list2, copy);
    }
}

void variable_info_free(variable_info_t* info) {
    if (info->named_inputs)
        expression_list_free(info->named_inputs);
    if (!info->is_internal)
        expression_free(info->value.expression);
    free(info);
}


error_t scope_define(scope_t* scope, char* variable, expression_t* value) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value.expression = value;
    info->is_internal = 0;
    info->named_inputs = NULL;
    info->constant = 0;
    return rbtree_insert(&scope->variables, variable, info);
}

error_t scope_define_constant(scope_t* scope, char* variable, expression_t* value) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value.expression = value;
    info->is_internal = 0;
    info->named_inputs = NULL;
    info->constant = 0;
    return rbtree_insert(&scope->variables, variable, info);
}

error_t scope_get_variable_info(scope_t* scope, char* variable, variable_info_t** value) {
    error_t err = rbtree_search(&scope->variables, variable, (void**)value);

    /* check the parent scope(s) for the variable */
    scope_t* parent = scope->parent;
    while (err && parent != NULL) {
        err = rbtree_search(&parent->variables, variable, (void**)value);
        parent = parent->parent;
    }
    return err;
}

error_t scope_define_internal_variable(scope_t* scope, char* name, simplify_func_t callback) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value.internal = callback;
    info->is_internal = 1;
    info->named_inputs = NULL;
    info->constant = 0;
    return rbtree_insert(&scope->variables, name, info);
}

error_t scope_define_internal_const(scope_t* scope, char* name, simplify_func_t callback) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value.internal = callback;
    info->is_internal = 1;
    info->named_inputs = NULL;
    info->constant = 1;
    return rbtree_insert(&scope->variables, name, info);
}

error_t scope_define_function(scope_t* scope, char* name, expression_t* body, expression_list_t* args) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    info->value.expression = body;
    info->is_internal = 0;
    info->named_inputs = args;
    info->constant = 0;
    return rbtree_insert(&scope->variables, name, info);
}

error_t scope_define_internal_function(scope_t* scope, char* name, simplify_func_t callback, int args, ...) {
    variable_info_t* info = malloc(sizeof(variable_info_t));
    expression_list_t* arg_list = malloc(sizeof(expression_list_t));
    expression_list_init(arg_list);

    va_list ap;
    va_start(ap, args);
    for (int i = 0; i < args; ++i) {
        expression_t* expr = malloc(sizeof(expression_t));
        char* argname = va_arg(ap, char*);
        expression_init_variable(expr, argname, strlen(argname));
        expression_list_append(arg_list, expr);
    }
    va_end(ap);

    info->value.internal = callback;
    info->is_internal = 1;
    info->named_inputs = arg_list;
    info->constant = 0;
    return rbtree_insert(&scope->variables, name, info);
}


error_t _scope_run_function(scope_t* scope, variable_t name, expression_list_t* arg_values, expression_t* out) {
    scope_t fn_scope;
    variable_info_t* func_info;
    expression_list_t arg_defs;
    error_t err;

    scope_init(&fn_scope);

    err = scope_get_variable_info(scope, name, &func_info);
    if (err)
        goto cleanup;

    if (err) goto cleanup;

    expression_list_init(&arg_defs);
    expression_list_copy(func_info->named_inputs, &arg_defs);
    expression_t* arg_def;
    expression_t* arg_value;

    EXPRESSION_LIST_FOREACH2(arg_def, arg_value, &arg_defs, arg_values) {
        expression_t* op_expr = malloc(sizeof(expression_t));

        expression_evaluate(arg_value, scope);
        expression_init_operator(op_expr, arg_def, ':', arg_value);

        err = expression_evaluate(op_expr, &fn_scope);
        if (err) goto cleanup;
        expression_free(op_expr);
    }
    expression_t* body = NULL;

    fn_scope.parent = scope;
    if (!func_info->is_internal) {
        body = malloc(sizeof(expression_t));
        expression_copy(func_info->value.expression, body);

        err = expression_evaluate(body, &fn_scope);
    } else {
        err = func_info->value.internal(&fn_scope, &body);
    }

    if (body && !err) {
        *out = *body;
        free(body);
    }
cleanup:
    scope_clean(&fn_scope);
    return err;
}

error_t scope_call(scope_t* scope, char* name, expression_list_t* args, expression_t* out) {
    variable_info_t* info;
    error_t err = scope_get_variable_info(scope, name, &info);
    if (err) return err;
    if (!info->named_inputs)
        return ERROR_IS_A_VARIABLE;

    expression_list_t args_copy;
    expression_list_init(&args_copy);
    expression_list_copy(args, &args_copy);

    err = _scope_run_function(scope, name, &args_copy, out);
    return err;
}

error_t scope_get_value(scope_t* scope, char* name, expression_t* expr) {
    variable_info_t* info;
    error_t err = scope_get_variable_info(scope, name, &info);
    if (err) return err;
    if (info->named_inputs)
        return ERROR_IS_A_FUNCTION;

    if (info->is_internal) {
        expression_t* new_expr = NULL;
        err = info->value.internal(scope, &new_expr);
        if (err) return err;
        if (new_expr) {
            *expr = *new_expr;
            free(new_expr);
        }
    } else {
        expression_copy(info->value.expression, expr);
    }
    return ERROR_NO_ERROR;
}

void expression_collapse_right(expression_t* expr) {
    assert(EXPRESSION_IS_OPERATOR(expr));

    expression_t* left = expr->operator.left;
    expression_free(expr->operator.right);
    *expr = *left;
    free(left);
}

void expression_collapse_left(expression_t* expr) {
    assert(EXPRESSION_IS_OPERATOR(expr) || EXPRESSION_IS_PREFIX(expr));

    expression_t* right = EXPRESSION_RIGHT(expr);
    if (EXPRESSION_LEFT(expr))
        expression_free(EXPRESSION_LEFT(expr));

    *expr = *right;
    free(right);
}
