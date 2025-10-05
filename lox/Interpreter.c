//
// Created by adrian on 2025-09-29.
//

#include "Interpreter.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"
#include "Environment.h"
#include "Function.h"
#include "../extra/Memory.h"
#include "../extra/Map.h"

struct interpreter {
    environment_t * p_current_env;
    map_t * locals;
    expr_visitor_t expr_visitor;
    stmt_visitor_t stmt_visitor;
    bool had_runtime_error;
    char error_message[256];
};

// Forward declarations
static void check_runtime_error(interpreter_t * p_interpreter);
static void throw_error(interpreter_t * p_interpreter, const char * fmt, ...);

static void * visit_assign_expr         (const expr_t * p_expr, void * p_ctx);
static void * visit_binary_expr         (const expr_t * p_expr, void * p_ctx);
static void * visit_call_expr           (const expr_t * p_expr, void * p_ctx);
static void * visit_get_expr            (const expr_t * p_expr, void * p_ctx);
static void * visit_grouping_expr       (const expr_t * p_expr, void * p_ctx);
static void * visit_literal_expr        (const expr_t * p_expr, void * p_ctx);
static void * visit_logical_expr        (const expr_t * p_expr, void * p_ctx);
static void * visit_set_expr            (const expr_t * p_expr, void * p_ctx);
static void * visit_super_expr          (const expr_t * p_expr, void * p_ctx);
static void * visit_this_expr           (const expr_t * p_expr, void * p_ctx);
static void * visit_unary_expr          (const expr_t * p_expr, void * p_ctx);
static void * visit_variable_expr       (const expr_t * p_expr, void * p_ctx);

static void * visit_block_stmt          (const stmt_t * p_stmt, void * p_ctx);
static void * visit_class_stmt          (const stmt_t * p_stmt, void * p_ctx);
static void * visit_expression_stmt     (const stmt_t * p_stmt, void * p_ctx);
static void * visit_function_stmt       (const stmt_t * p_stmt, void * p_ctx);
static void * visit_if_stmt             (const stmt_t * p_stmt, void * p_ctx);
static void * visit_print_stmt          (const stmt_t * p_stmt, void * p_ctx);
static void * visit_return_stmt         (const stmt_t * p_stmt, void * p_ctx);
static void * visit_var_stmt            (const stmt_t * p_stmt, void * p_ctx);
static void * visit_while_stmt          (const stmt_t * p_stmt, void * p_ctx);

size_t hash_expr(const void * key, const size_t num_buckets) {
    return (size_t)key % num_buckets;
}
bool cmp_expr(const void * key1, const void * key2) {
    return key1 == key2;
}
void clean_expr(const void * key, const void * value) {
    (void)key, (void)value;
}
// Public API
interpreter_t * new_interpreter(void) {
    interpreter_t * p_interpreter = memory_allocate(sizeof(interpreter_t));
    if (!p_interpreter) {
        fprintf(stderr, "Failed to create Interpreter\n");
        return NULL;
    }
    p_interpreter->p_current_env = create_environment(NULL);
    p_interpreter->locals = map_create(8, hash_expr, cmp_expr, clean_expr);
    p_interpreter->expr_visitor.visit_assign        = visit_assign_expr;
    p_interpreter->expr_visitor.visit_binary        = visit_binary_expr;
    p_interpreter->expr_visitor.visit_call          = visit_call_expr;
    p_interpreter->expr_visitor.visit_get           = visit_get_expr;
    p_interpreter->expr_visitor.visit_grouping      = visit_grouping_expr;
    p_interpreter->expr_visitor.visit_literal       = visit_literal_expr;
    p_interpreter->expr_visitor.visit_logical       = visit_logical_expr;
    p_interpreter->expr_visitor.visit_set           = visit_set_expr;
    p_interpreter->expr_visitor.visit_super         = visit_super_expr;
    p_interpreter->expr_visitor.visit_this          = visit_this_expr;
    p_interpreter->expr_visitor.visit_unary         = visit_unary_expr;
    p_interpreter->expr_visitor.visit_variable      = visit_variable_expr;

    p_interpreter->stmt_visitor.visit_block         = visit_block_stmt;
    p_interpreter->stmt_visitor.visit_class         = visit_class_stmt;
    p_interpreter->stmt_visitor.visit_expression    = visit_expression_stmt;
    p_interpreter->stmt_visitor.visit_function      = visit_function_stmt;
    p_interpreter->stmt_visitor.visit_if            = visit_if_stmt;
    p_interpreter->stmt_visitor.visit_print         = visit_print_stmt;
    p_interpreter->stmt_visitor.visit_return        = visit_return_stmt;
    p_interpreter->stmt_visitor.visit_var           = visit_var_stmt;
    p_interpreter->stmt_visitor.visit_while         = visit_while_stmt;

    p_interpreter->had_runtime_error = false;
    p_interpreter->error_message[0] = '\0';
    return p_interpreter;
}
void interpret(const dynamic_array_t * statements, interpreter_t * p_interpreter) {
    stmt_t ** p_stmts = statements->data;
    const size_t stmts_count = statements->size / sizeof(stmt_t *);
    for (size_t i = 0; i < stmts_count; i++) {
        execute(p_stmts[i], p_interpreter);
        check_runtime_error(p_interpreter);
    }
}
object_t * evaluate(const expr_t * p_expr, interpreter_t * p_interpreter) {
    return expr_accept(p_expr, &p_interpreter->expr_visitor, p_interpreter);
}
void * execute(const stmt_t * p_stmt, interpreter_t * p_interpreter) {
    return stmt_accept(p_stmt, &p_interpreter->stmt_visitor, p_interpreter);
}
environment_t * get_interpreter_environment(const interpreter_t * p_interpreter) {
    return p_interpreter->p_current_env;
}
void set_interpreter_environment(interpreter_t * p_interpreter, environment_t * p_env) {
    p_interpreter->p_current_env = p_env;
}
void interpreter_resolve(const expr_t * p_expr, const int depth, const interpreter_t * p_interpreter) {
    map_put(p_interpreter->locals, p_expr, (void*)(intptr_t)depth);
}

// Private helper functions
static void check_runtime_error(interpreter_t * p_interpreter) {
    if (p_interpreter->had_runtime_error) {
        fprintf(stderr, "RuntimeError: %s\n", p_interpreter->error_message);
        exit(EXIT_FAILURE);
    }
}
static void throw_error(interpreter_t * p_interpreter, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(p_interpreter->error_message,
        sizeof(p_interpreter->error_message),
        fmt, args);
    p_interpreter->error_message[sizeof(p_interpreter->error_message) - 1] = '\0';
    va_end(args);
    p_interpreter->had_runtime_error = true;
}
const char * stringify(const object_t * p_object) {
    if (!p_object) {
        return NULL;
    }
    static char buffer[32]; // static buffer for numbers (not thread-safe) TODO better method
    switch (get_object_type(p_object)) {
        case OBJECT_STRING:
            return get_object_string(p_object);
        case OBJECT_NUMBER:
            snprintf(buffer, sizeof(buffer), "%g", get_object_number(p_object));
            break;
        case OBJECT_BOOLEAN:
        case OBJECT_FUNCTION:
        case OBJECT_NIL:
        default:
            buffer[0] = '\0';
            break;
    }
    return buffer;
}
bool is_truthy(const object_t * p_obj) {
    switch (get_object_type(p_obj)) {
        case OBJECT_BOOLEAN: return get_object_boolean(p_obj);
        default: return false;
    }
}

// Private visitor functions
static void * visit_assign_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_assign_t expr = p_expr->as.assign_expr;
    interpreter_t * p_interpreter = p_ctx;
    object_t * p_value  = evaluate(expr.value, p_interpreter);
    if (!assign_variable(get_interpreter_environment(p_interpreter),
                            expr.target->as.variable_expr.name->lexeme,
                            p_value)) {
        throw_error(p_interpreter, "Failed to assign value '%s' to variable '%s'",
            stringify(p_value), expr.target->as.variable_expr.name->lexeme);
    }
    return p_value;
}
static void * visit_binary_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_binary_t expr = p_expr->as.binary_expr;

    // TODO change this to objects
    const object_t * left  = evaluate(expr.left, p_ctx);
    check_runtime_error(p_ctx);
    const object_t * right = evaluate(expr.right, p_ctx);
    check_runtime_error(p_ctx);
    object_t * result = NULL;
    //result->is_on_heap = true;

    switch (expr.operator->type) {
        case PLUS:
            if (get_object_type(left) == OBJECT_NUMBER &&
                    get_object_type(right) == OBJECT_NUMBER) {
                double sum = get_object_number(left) + get_object_number(right);
                result = new_object(OBJECT_NUMBER, &sum);
            } else {
                result = NULL;
                throw_error(p_ctx, "Invalid operand types for operator: %s",
                    expr.operator->lexeme);
            }
            break;
        case MINUS:
            if (get_object_type(left) == OBJECT_NUMBER &&
                    get_object_type(right) == OBJECT_NUMBER) {
                double sum = get_object_number(left) - get_object_number(right);
                result = new_object(OBJECT_NUMBER, &sum);
                    } else {
                        result = NULL;
                        throw_error(p_ctx, "Invalid operand types for operator: %s",
                            expr.operator->lexeme);
                    }
            break;
        case STAR:
            if (get_object_type(left) == OBJECT_NUMBER &&
                    get_object_type(right) == OBJECT_NUMBER) {
                double product = get_object_number(left) * get_object_number(right);
                result = new_object(OBJECT_NUMBER, &product);
            } else {
                result = NULL;
                throw_error(p_ctx, "Invalid operand types for operator: %s",
                    expr.operator->lexeme);
            }
            break;
        case SLASH:
            if (get_object_type(left) == OBJECT_NUMBER &&
                     get_object_type(right) == OBJECT_NUMBER) {
                const double numerator = get_object_number(left);
                const double denominator = get_object_number(right);
                if (denominator == 0) {
                    throw_error(p_ctx, "Division by zero at line %d", expr.operator->line);
                    result = NULL;
                    break;
                }
                double product = get_object_number(left) / get_object_number(right);
                result = new_object(OBJECT_NUMBER, &product);
                     } else {
                         result = NULL;
                         throw_error(p_ctx, "Invalid operand types for operator: %s",
                             expr.operator->lexeme);
                     }
            break;
        case GREATER:
            if (get_object_type(left) == OBJECT_NUMBER &&
                    get_object_type(right) == OBJECT_NUMBER) {
                bool value = get_object_number(left) > get_object_number(right);
                result = new_object(OBJECT_BOOLEAN, &value);
                    } else {
                        throw_error(p_ctx, "Expected two numbers as operands of operator '%s'.",
                            expr.operator->lexeme);
                    }
            break;
        case LESS:
            if (get_object_type(left) == OBJECT_NUMBER &&
                    get_object_type(right) == OBJECT_NUMBER) {
                bool value = get_object_number(left) < get_object_number(right);
                result = new_object(OBJECT_BOOLEAN, &value);
            } else {
                throw_error(p_ctx, "Expected two numbers as operands of operator '%s'.",
                    expr.operator->lexeme);
            }
            break;
        default:
            throw_error(p_ctx, "Unknown binary operator: %s",
                expr.operator->lexeme);
            return NULL;
    }
    return result;

}
static void * visit_call_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_call_t expr = p_expr->as.call_expr;
    if (expr.callee->type != EXPR_VARIABLE) {
        throw_error(p_ctx, "BIG ERROR");
        return NULL;
    }
    const object_t * callee = evaluate(expr.callee, p_ctx);
    const object_type_t type = get_object_type(callee);

    if (type != OBJECT_FUNCTION &&
        type != OBJECT_NATIVE) {
        throw_error(p_ctx, "Expected '%s' at line %d to be function object",
            expr.callee->as.variable_expr.name->lexeme,
            expr.callee->as.variable_expr.name->line);
        return NULL;
    }
    check_runtime_error(p_ctx);
    object_t ** arguments = memory_allocate(*expr.count * sizeof(object_t*));
    for (size_t i = 0; i < *expr.count; i++) {
        arguments[i] = evaluate(expr.arguments[i], p_ctx);
        check_runtime_error(p_ctx);
    }
    //object_t * p_return =
    void * p_return = call_function(get_object_function(callee), p_ctx, arguments);
    return p_return;
}
static void * visit_get_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
// static void * visit_grouping_expr       (const expr_t * p_expr, void * p_ctx) {
//     const expr_grouping_t * grouping_p = &p_expr->as.grouping_expr;
//     return expr_accept(grouping_p->expression, visitor, p_ctx);
// }
static void * visit_grouping_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_grouping_t expr = p_expr->as.grouping_expr;
    return evaluate(expr.expression, p_ctx);
}
static void * visit_literal_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_literal_t expr = p_expr->as.literal_expr;
    object_t * p_object;
    switch (expr.kind->type) {
        case NUMBER:
            double value = strtod(expr.kind->lexeme, NULL);
            return new_object(OBJECT_NUMBER, &value);
        case STRING://
            char * str = memory_allocate(sizeof(expr.kind->lexeme) + 1);
            if (!memory_copy(str, expr.kind->lexeme, sizeof(expr.kind->lexeme))) {
                throw_error(p_ctx, "Memory copy failed");
                return NULL;
            }
            str[sizeof(expr.kind->lexeme) - 1] = '\0';
            return new_object(OBJECT_STRING, str);
        default:
            throw_error(p_ctx, "Unknown literal type: %s",
                g_token_type_names[expr.kind->type]);
            return new_object(OBJECT_NIL, NULL);
    }

}
static void * visit_logical_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_set_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_super_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_this_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_unary_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_variable_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_variable_t expr = p_expr->as.variable_expr;
    object_t * result = env_lookup(((interpreter_t*)p_ctx)->p_current_env, expr.name->lexeme);
    if (result) {

    } else {
        throw_error(p_ctx, "Undefined variable '%s' at line %d",
        expr.name->lexeme, expr.name->line);
    }
    return result;
}
static void * visit_block_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_block_t stmt = p_stmt->as.block_stmt;
    interpreter_t * p_interpreter = p_ctx;
    environment_t * p_new_env = create_environment(p_interpreter->p_current_env);
    set_interpreter_environment(p_interpreter, p_new_env);
    for (size_t i = 0; i < *stmt.count; i++) {
        execute(stmt.statements[i], p_interpreter);
    }
    set_interpreter_environment(p_interpreter, get_parent_environment(p_new_env));
    return NULL;
}
static void * visit_class_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_expression_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_expression_t stmt = p_stmt->as.expression_stmt;
    evaluate(stmt.expression, p_ctx);
    check_runtime_error(p_ctx);
    return NULL;
}
static void * visit_function_stmt(const stmt_t * p_stmt, void * p_ctx) {
    stmt_function_t * stmt = memory_allocate(sizeof(stmt_function_t));
    stmt->name          = p_stmt->as.function_stmt.name;
    stmt->body          = p_stmt->as.function_stmt.body;
    stmt->count         = p_stmt->as.function_stmt.count;
    stmt->params        = p_stmt->as.function_stmt.params;
    stmt->params_count  = p_stmt->as.function_stmt.params_count;


    function_t * p_function = new_function(stmt, ((interpreter_t*)p_ctx)->p_current_env);
    const object_t * p_object = new_object(OBJECT_FUNCTION, p_function);
    declare_variable(((interpreter_t*)p_ctx)->p_current_env, stmt->name->lexeme, p_object);
    return NULL;
}
static void * visit_if_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_if_t stmt = p_stmt->as.if_stmt;
    const object_t * p_condition_obj = evaluate(stmt.condition, p_ctx);
    if (is_truthy(p_condition_obj)) {
        execute(stmt.then_branch, p_ctx);
    } else if (stmt.else_branch) {
        execute(stmt.else_branch, p_ctx);
    }
    return NULL;
}
// ReSharper disable once CppDFAConstantFunctionResult
static void * visit_print_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_print_t stmt = p_stmt->as.print_stmt;
    const object_t * p_object = evaluate(stmt.expression, p_ctx);
    check_runtime_error(p_ctx);
    const char * str = stringify(p_object);
    if (!str) {
        throw_error(p_ctx, "print error: object is null.");
        return NULL;
    }
    printf("%s\n", str);
    return NULL;
}
static void * visit_return_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_return_t stmt = p_stmt->as.return_stmt;
    object_t * p_object = evaluate(stmt.value, p_ctx);
    check_runtime_error(p_ctx);
    return p_object;

}
static void * visit_var_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_var_t stmt = p_stmt->as.var_stmt;
    const object_t * p_value = evaluate(stmt.initializer, p_ctx);
    check_runtime_error(p_ctx);
    declare_variable(((interpreter_t*)p_ctx)->p_current_env, stmt.name->lexeme, p_value);
    return NULL;
}
static void * visit_while_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_while_t stmt = p_stmt->as.while_stmt;
    while (true) {
        const object_t * cond_obj = evaluate(stmt.condition, p_ctx);
        if (!is_truthy(cond_obj)) {
            break;
        }
        execute(stmt.body, p_ctx);
    }
    return NULL;
}
