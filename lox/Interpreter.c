//
// Created by adrian on 2025-09-29.
//

#include "Interpreter.h"

#include <stdio.h>

#include "Environment.h"
#include "../extra/Memory.h"

struct interpreter {
    environment_t * p_current_env;
    expr_visitor_t expr_visitor;
    stmt_visitor_t stmt_visitor;
    bool had_runtime_error;
    char error_message[256];
};

static void * unimplemented_expr(const expr_t * p_expr, void * p_ctx);
static void * unimplemented_stmt(const stmt_t * p_stmt, void * p_ctx);
// Public API
interpreter_t * new_interpreter(void) {
    interpreter_t * p_interpreter = memory_allocate(sizeof(interpreter_t));
    if (!p_interpreter) {
        fprintf(stderr, "Failed to create Interpreter\n");
        return NULL;
    }
    p_interpreter->p_current_env = create_environment(NULL);
    p_interpreter->expr_visitor.visit_assign        = unimplemented_expr;
    p_interpreter->expr_visitor.visit_binary        = unimplemented_expr;
    p_interpreter->expr_visitor.visit_call          = unimplemented_expr;
    p_interpreter->expr_visitor.visit_get           = unimplemented_expr;
    p_interpreter->expr_visitor.visit_grouping      = unimplemented_expr;
    p_interpreter->expr_visitor.visit_literal       = unimplemented_expr;
    p_interpreter->expr_visitor.visit_logical       = unimplemented_expr;
    p_interpreter->expr_visitor.visit_set           = unimplemented_expr;
    p_interpreter->expr_visitor.visit_super         = unimplemented_expr;
    p_interpreter->expr_visitor.visit_this          = unimplemented_expr;
    p_interpreter->expr_visitor.visit_unary         = unimplemented_expr;
    p_interpreter->expr_visitor.visit_variable      = unimplemented_expr;

    p_interpreter->stmt_visitor.visit_block         = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_class         = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_expression    = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_function      = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_if            = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_print         = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_return        = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_var           = unimplemented_stmt;
    p_interpreter->stmt_visitor.visit_while         = unimplemented_stmt;

    p_interpreter->had_runtime_error = false;
    p_interpreter->error_message[0] = '\0';
    return p_interpreter;
}
void interpreter_free       (interpreter_t ** p_interpreter) {
    if (!p_interpreter) return;
    free_environment((*p_interpreter)->p_current_env);
    memory_free((void**)p_interpreter);
    *p_interpreter = NULL;
}
void interpret              (const dynamic_array_t * statements, interpreter_t * p_interpreter) {
    stmt_t ** p_stmts = statements->data;
    const size_t stmts_count = statements->size / sizeof(stmt_t *);
    for (size_t i = 0; i < stmts_count; i++) {
        execute(p_stmts[i], p_interpreter);
        if (p_interpreter->had_runtime_error) {
            fprintf(stderr, "RuntimeError: %s\n", p_interpreter->error_message);
        }
    }
}
object_t * evaluate         (const expr_t * p_expr, interpreter_t * p_interpreter) {

    return expr_accept(p_expr, &p_interpreter->expr_visitor, p_interpreter);
}
void * execute              (const stmt_t * p_stmt, interpreter_t * p_interpreter) {
    stmt_accept(p_stmt, &p_interpreter->stmt_visitor, p_interpreter);
    return NULL;
}

static void * unimplemented_expr(const expr_t * p_expr, void * p_ctx) {
    fprintf(stderr, "Unimplemented expression\n");
    return NULL;
}
static void * unimplemented_stmt(const stmt_t * p_stmt, void * p_ctx) {
    fprintf(stderr, "Unimplemented statement\n");
    return NULL;
}