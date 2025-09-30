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
#include "../extra/Memory.h"

struct interpreter {
    environment_t * p_current_env;
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

// Public API
interpreter_t * new_interpreter(void) {
    interpreter_t * p_interpreter = memory_allocate(sizeof(interpreter_t));
    if (!p_interpreter) {
        fprintf(stderr, "Failed to create Interpreter\n");
        return NULL;
    }
    p_interpreter->p_current_env = create_environment(NULL);
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
void interpreter_free(interpreter_t ** p_interpreter) {
    if (!p_interpreter) return;
    free_environment((*p_interpreter)->p_current_env);
    memory_free((void**)p_interpreter);
    *p_interpreter = NULL;
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
    stmt_accept(p_stmt, &p_interpreter->stmt_visitor, p_interpreter);
    return NULL;
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
            buffer[0] = '\0';
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

// Private visitor functions
static void * visit_assign_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_binary_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_binary_t expr = p_expr->as.binary_expr;

    // TODO change this to objects
    object_t * left  = evaluate(expr.left, p_ctx);
    check_runtime_error(p_ctx);
    object_t * right = evaluate(expr.right, p_ctx);
    check_runtime_error(p_ctx);
    object_t * result;
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
        default:
            throw_error(p_ctx, "Unknown binary operator: %s",
                expr.operator->lexeme);
            return NULL;
    }
    object_free(&left);
    object_free(&right);
    return result;

}
static void * visit_call_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_get_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_grouping_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_literal_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_literal_t expr = p_expr->as.literal_expr;
    object_t * p_object;
    switch (expr.kind->type) {
        case NUMBER:
            double value = strtod(expr.kind->lexeme, NULL);
            return new_object(OBJECT_NUMBER, &value);
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
    return env_lookup(((interpreter_t*)p_ctx)->p_current_env, expr.name->lexeme);
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}

static void * visit_block_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_class_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_expression_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_expression_t stmt = p_stmt->as.expression_stmt;
    void * p_expr_val = evaluate(stmt.expression, p_ctx);
    memory_free(&p_expr_val);
    return NULL;
}
static void * visit_function_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_if_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
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
    if (p_object) {
        memory_free((void**)&p_object);
    }
    printf("%s\n", str);
    return NULL;
}
static void * visit_return_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_var_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_var_t stmt = p_stmt->as.var_stmt;
    object_t * p_value = evaluate(stmt.initializer, p_ctx);
    declare_variable(((interpreter_t*)p_ctx)->p_current_env, stmt.name->lexeme, p_value);
    return NULL;
}
static void * visit_while_stmt(const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}