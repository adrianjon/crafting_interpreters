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

enum function_type {
    FUNCTION_TYPE_NONE,
    FUNCTION_TYPE_FUNCTION,
    FUNCTION_TYPE_METHOD,
};

struct resolver {
    interpreter_t * p_interpreter;
    stack_t * scopes;
    enum function_type current_function;
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
    p_resolver->current_function = FUNCTION_TYPE_NONE;

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
    stack_push(p_resolver->scopes, map_create(8, (map_config_t){0}));
}
static void end_scope(const resolver_t * p_resolver) {
    stack_pop(p_resolver->scopes);
}
static void declare(const token_t * p_name, resolver_t * p_resolver) {
    if (stack_is_empty(p_resolver->scopes)) return;
    map_t * scope = stack_peek(p_resolver->scopes);
    if (map_contains(scope, p_name->lexeme)) {
        throw_error(p_resolver, "[line %d] Already a variable named '%s' in this scope.",
            p_name->line, p_name->lexeme);
    }
    map_put(scope, p_name->lexeme, false);
}
static void define(const token_t * p_name, const  resolver_t * p_resolver) {
    if (stack_is_empty(p_resolver->scopes)) return;
    map_t * scope = stack_peek(p_resolver->scopes);
    map_put(scope, p_name->lexeme, (void*)true);
}
static void resolve_function(const stmt_function_t stmt, enum function_type type,
                                resolver_t * p_resolver) {
    const enum function_type enclosing_function = p_resolver->current_function;
    p_resolver->current_function = type;
    begin_scope(p_resolver);
    for (size_t i = 0; i < *stmt.params_count; i++) {
        declare(stmt.params[i], p_resolver);
        define(stmt.params[i], p_resolver);
    }
    for (size_t i = 0; i < *stmt.count; i++) {
        resolve_stmt(stmt.body[i], p_resolver);
    }
    end_scope(p_resolver);
    p_resolver->current_function = enclosing_function;
}
static void resolve_local( const expr_t * p_expr, const token_t * p_token, const resolver_t * p_resolver) {
    for (int i = (int)stack_size(p_resolver->scopes) - 1; i >= 0; i--) {
        const map_t * scope = (map_t*)p_resolver->scopes->data[i];
        if (map_contains(scope, p_token->lexeme)) {
            const int distance = (int)stack_size(p_resolver->scopes) - 1 - i;
            interpreter_resolve(p_expr, distance, p_resolver->p_interpreter);
            return;
        }
    }
    // global
}
// Visitor implementations
static void * visit_assign_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_assign_t expr = p_expr->as.assign_expr;
    resolve_expr(expr.value, p_ctx);
    resolve_local(expr.target, expr.target->as.variable_expr.name, p_ctx);
    return NULL;
}
static void * visit_binary_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_binary_t expr = p_expr->as.binary_expr;
    resolve_expr(expr.left, p_ctx);
    resolve_expr(expr.right, p_ctx);
    return NULL;
}
static void * visit_call_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_call_t expr = p_expr->as.call_expr;
    resolve_expr(expr.callee, p_ctx);
    for (size_t i = 0; i < *expr.count; i++) {
        resolve_expr(expr.arguments[i], p_ctx);
    }
    return NULL;
}
static void * visit_get_expr(const expr_t * p_expr, void * p_ctx) {
    throw_error(p_ctx, "Unimplemented expression: %s (%d)",
        g_expr_type_names[p_expr->type], p_expr->type);
    return NULL;
}
static void * visit_grouping_expr      (const expr_t * p_expr, void * p_ctx) {
    const expr_grouping_t expr = p_expr->as.grouping_expr;
    resolve_expr(expr.expression, p_ctx);
    return NULL;
}
static void * visit_literal_expr(const expr_t * p_expr, void * p_ctx) {
    (void)p_expr;
    return NULL;
}
static void * visit_logical_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_logical_t expr = p_expr->as.logical_expr;
    resolve_expr(expr.left, p_ctx);
    resolve_expr(expr.right, p_ctx);
    return NULL;
}
static void * visit_set_expr(const expr_t * p_expr, void * p_ctx) {
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
static void * visit_unary_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_unary_t expr = p_expr->as.unary_expr;
    resolve_expr(expr.right, p_ctx);
    return NULL;
}
static void * visit_variable_expr(const expr_t * p_expr, void * p_ctx) {
    const expr_variable_t expr = p_expr->as.variable_expr;
    resolver_t * p_resolver = p_ctx;
    const map_t * scope = stack_peek(p_resolver->scopes);
   // void * is_var_defined = map_get(scope, expr.name->lexeme);
    if (!stack_is_empty(p_resolver->scopes) && map_contains(scope, expr.name->lexeme)) {
        if ((bool)map_get(scope, expr.name->lexeme) == false)
            throw_error(p_resolver, "Can't read local variable in its own initializer.");
    }
    resolve_local(p_expr, expr.name, p_resolver);
    return NULL;
}

static void * visit_block_stmt(const stmt_t * p_stmt, void * p_ctx) {
    begin_scope(p_ctx);
    resolve_list(p_stmt->as.block_stmt.statements, *p_stmt->as.block_stmt.count, p_ctx);
    end_scope(p_ctx);
    return NULL;
}
static void * visit_class_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_class_t stmt = p_stmt->as.class_stmt;
    declare(stmt.name, p_ctx);
    define(stmt.name, p_ctx);
    for (size_t i = 0; i < stmt.methods_count; i++) {
        const enum function_type declaration = FUNCTION_TYPE_METHOD;
        resolve_function(stmt.methods[i]->as.function_stmt, declaration, p_ctx);
    }
    return NULL;
}
static void * visit_expression_stmt    (const stmt_t * p_stmt, void * p_ctx) {
    const stmt_expression_t stmt = p_stmt->as.expression_stmt;
    resolve_expr(stmt.expression, p_ctx);
    return NULL;
}
static void * visit_function_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_function_t stmt = p_stmt->as.function_stmt;
    declare(stmt.name, p_ctx);
    define(stmt.name, p_ctx);
    resolve_function(stmt, FUNCTION_TYPE_FUNCTION, p_ctx);
    return NULL;
}
static void * visit_if_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_if_t stmt = p_stmt->as.if_stmt;
    resolve_expr(stmt.condition, p_ctx);
    resolve_stmt(stmt.then_branch, p_ctx);
    if (stmt.else_branch) resolve_stmt(stmt.else_branch, p_ctx);
    return NULL;
}
static void * visit_print_stmt         (const stmt_t * p_stmt, void * p_ctx) {
    const stmt_print_t stmt = p_stmt->as.print_stmt;
    resolve_expr(stmt.expression, p_ctx);
    return NULL;
}
static void * visit_return_stmt        (const stmt_t * p_stmt, void * p_ctx) {
    const stmt_return_t stmt = p_stmt->as.return_stmt;
    if (((resolver_t*)p_ctx)->current_function == FUNCTION_TYPE_NONE) {
        throw_error(p_ctx, "Can't return from top level code.");
    }
    if (stmt.value) resolve_expr(stmt.value, p_ctx);
    return NULL;
}
static void * visit_var_stmt(const stmt_t * p_stmt, void * p_ctx) {
    const stmt_var_t stmt = p_stmt->as.var_stmt;
    declare(stmt.name, p_ctx);
    if (stmt.initializer) {
        resolve_expr(stmt.initializer, p_ctx);
    }
    define(stmt.name, p_ctx);
    return NULL;
}
static void * visit_while_stmt         (const stmt_t * p_stmt, void * p_ctx) {
    const stmt_while_t stmt = p_stmt->as.while_stmt;
    resolve_expr(stmt.condition, p_ctx);
    resolve_stmt(stmt.body, p_ctx);
    return NULL;
}