//
// Created by adrian on 2025-10-05.
//

#include "Resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "Interpreter.h"
#include "Stmt.h"
#include "Expr.h"
#include "Parser.h"
#include "../extra/Memory.h"
#include "../extra/Stack.h"
#include "../extra/Map.h"

struct resolver {
    interpreter_t * p_interpreter;
    stack_t * scopes;
    expr_visitor_t expr_visitor;
    stmt_visitor_t stmt_visitor;
    bool had_runtime_error;
    char error_message[256];
};

// Forward declarations
static void check_runtime_error(resolver_t * p_resolver);
static void throw_error(resolver_t * p_resolver, const char * fmt, ...);

static void * visit_assign_expr        (const expr_t * p_expr, void * p_ctx);
static void * visit_binary_expr        (const expr_t * p_expr, void * p_ctx);
static void * visit_call_expr          (const expr_t * p_expr, void * p_ctx);
static void * visit_get_expr           (const expr_t * p_expr, void * p_ctx);
static void * visit_grouping_expr      (const expr_t * p_expr, void * p_ctx);
static void * visit_literal_expr       (const expr_t * p_expr, void * p_ctx);
static void * visit_logical_expr       (const expr_t * p_expr, void * p_ctx);
static void * visit_set_expr           (const expr_t * p_expr, void * p_ctx);
static void * visit_super_expr         (const expr_t * p_expr, void * p_ctx);
static void * visit_this_expr          (const expr_t * p_expr, void * p_ctx);
static void * visit_unary_expr         (const expr_t * p_expr, void * p_ctx);
static void * visit_variable_expr      (const expr_t * p_expr, void * p_ctx);

static void * visit_block_stmt         (const stmt_t * p_stmt, void * p_ctx);
static void * visit_class_stmt         (const stmt_t * p_stmt, void * p_ctx);
static void * visit_expression_stmt    (const stmt_t * p_stmt, void * p_ctx);
static void * visit_function_stmt      (const stmt_t * p_stmt, void * p_ctx);
static void * visit_if_stmt            (const stmt_t * p_stmt, void * p_ctx);
static void * visit_print_stmt         (const stmt_t * p_stmt, void * p_ctx);
static void * visit_return_stmt        (const stmt_t * p_stmt, void * p_ctx);
static void * visit_var_stmt           (const stmt_t * p_stmt, void * p_ctx);
static void * visit_while_stmt         (const stmt_t * p_stmt, void * p_ctx);

static void resolve_stmt(const stmt_t * p_stmt, resolver_t * p_resolver);
static void resolve_expr(const expr_t * p_expr, resolver_t * p_resolver);

// Public API
resolver_t * new_resolver(interpreter_t * p_interpreter) {
    resolver_t * p_resolver = memory_allocate(sizeof(resolver_t));
    if (!p_resolver) {
        fprintf(stderr, "Failed to create Resolver\n");
        return NULL;
    }
    p_resolver->expr_visitor.visit_assign        = visit_assign_expr;
    p_resolver->expr_visitor.visit_binary        = visit_binary_expr;
    p_resolver->expr_visitor.visit_call          = visit_call_expr;
    p_resolver->expr_visitor.visit_get           = visit_get_expr;
    p_resolver->expr_visitor.visit_grouping      = visit_grouping_expr;
    p_resolver->expr_visitor.visit_literal       = visit_literal_expr;
    p_resolver->expr_visitor.visit_logical       = visit_logical_expr;
    p_resolver->expr_visitor.visit_set           = visit_set_expr;
    p_resolver->expr_visitor.visit_super         = visit_super_expr;
    p_resolver->expr_visitor.visit_this          = visit_this_expr;
    p_resolver->expr_visitor.visit_unary         = visit_unary_expr;
    p_resolver->expr_visitor.visit_variable      = visit_variable_expr;

    p_resolver->stmt_visitor.visit_block         = visit_block_stmt;
    p_resolver->stmt_visitor.visit_class         = visit_class_stmt;
    p_resolver->stmt_visitor.visit_expression    = visit_expression_stmt;
    p_resolver->stmt_visitor.visit_function      = visit_function_stmt;
    p_resolver->stmt_visitor.visit_if            = visit_if_stmt;
    p_resolver->stmt_visitor.visit_print         = visit_print_stmt;
    p_resolver->stmt_visitor.visit_return        = visit_return_stmt;
    p_resolver->stmt_visitor.visit_var           = visit_var_stmt;
    p_resolver->stmt_visitor.visit_while         = visit_while_stmt;

    p_resolver->p_interpreter = p_interpreter;
    p_resolver->scopes = stack_create(8);

    return p_resolver;
}
void resolve_list(stmt_t ** pp_stmts, const size_t n_stmts, resolver_t * p_resolver) {
    for (size_t i = 0; i < n_stmts; i++) {
        resolve_stmt(pp_stmts[i], p_resolver);
        check_runtime_error(p_resolver);
    }
}

// Private helper functions
static void check_runtime_error(resolver_t * p_resolver) {
    if (p_resolver->had_runtime_error) {
        fprintf(stderr, "RuntimeError: %s\n", p_resolver->error_message);
        exit(EXIT_FAILURE);
    }
}
static void throw_error(resolver_t * p_resolver, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(p_resolver->error_message,
        sizeof(p_resolver->error_message),
        fmt, args);
    p_resolver->error_message[sizeof(p_resolver->error_message) - 1] = '\0';
    va_end(args);
    p_resolver->had_runtime_error = true;
}
static void resolve_stmt(const stmt_t * p_stmt, resolver_t * p_resolver) {
    stmt_accept(p_stmt, &p_resolver->stmt_visitor, p_resolver);
}
static void resolve_expr(const expr_t * p_expr, resolver_t * p_resolver) {
    expr_accept(p_expr, &p_resolver->expr_visitor, p_resolver);
}
static void begin_scope(const resolver_t * p_resolver) {
    stack_push(p_resolver->scopes, map_create(8));
}
static void end_scope(const resolver_t * p_resolver) {
    stack_pop(p_resolver->scopes);
}
// Visitor implementations
static void * visit_assign_expr        (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_binary_expr        (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_call_expr          (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_get_expr           (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_grouping_expr      (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_literal_expr       (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_logical_expr       (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_set_expr           (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_super_expr         (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_this_expr          (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_unary_expr         (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_variable_expr      (const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}

static void * visit_block_stmt(const stmt_t * p_stmt, void * p_ctx) {
    begin_scope(p_ctx);
    resolve_list(p_stmt->as.block_stmt.statements, *p_stmt->as.block_stmt.count, p_ctx);
    end_scope(p_ctx);
    return NULL;
}
static void * visit_class_stmt         (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_expression_stmt    (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_function_stmt      (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_if_stmt            (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_print_stmt         (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_return_stmt        (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_var_stmt           (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}
static void * visit_while_stmt         (const stmt_t * p_stmt, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented statement: %s (%d)",
        g_stmt_type_names[p_stmt->type], p_stmt->type);
    return NULL;
}