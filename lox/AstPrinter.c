//
// Created by adrian on 2025-09-15.
//

#include "AstPrinter.h"

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <stdarg.h>
#include "../extra/Arrays.h"
#include "../extra/Memory.h"
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Object.h"

// TODO: Split this into ast printer using push-sink (string builder) and
// ast to string functions that return newly allocated strings.
// Currently mixing both approaches.

// ------------ Small private helper functions ------------
static void print_token(string_builder_t* sb_p, const token_t* token_p) {
    if (token_p->type == STRING) {
        append_string(sb_p, "\"");
        append_string(sb_p, token_p->lexeme);
        append_string(sb_p, "\"");
        return;
    }
    append_string(sb_p, token_p->lexeme);
}
static void print_object(string_builder_t* sb_p, const object_t* obj_p) {
    switch (obj_p->type) {
        case OBJECT_STRING:
            append_string(sb_p, "(\"");
            append_string(sb_p, obj_p->as.string.value);
            append_string(sb_p, "\")");
            break;
        case OBJECT_NUMBER:
            {
                char num_buf[32] = {0};
                snprintf(num_buf, sizeof(num_buf), "%g", obj_p->as.number.value);
                append_string(sb_p, num_buf);
            }
            break;
        default:
            append_string(sb_p, "literal");
            break;
    }
}
static char* str_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    const int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return NULL;
    }
    char* buf = memory_allocate((size_t)n + 1);
    if (!buf) {
        va_end(ap2);
        return NULL;
    }
    vsnprintf(buf, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    return buf;
}
static char* obj_to_string(const object_t* obj_p) {
    if (!obj_p) return "nil";
    switch (obj_p->type) {
        case OBJECT_STRING: return str_printf("\"%s\"", obj_p->as.string.value ? obj_p->as.string.value : "");
        case OBJECT_NUMBER: return str_printf("%g", obj_p->as.number.value);
        // TODO: handle bool, function, class, instance, native
        default: return "<object>";
    }
}
static char* paren1(const char* name, char* e) {
    char* result = str_printf("(%s %s)", name, e ? e : "(null)");
    memory_free((void**)&e);
    return result;
}
// ---------- Fallbacks (so missing visitors don't segfault) ----------

static void* ap_unimpl_expr(const expr_t* expr, const expr_visitor_t* v, void* ctx) {
    (void)expr; (void)v;
    const ast_printer_t* p = (ast_printer_t*)ctx;
    append_string(p->sb_p, "<unimpl-expr>");
    return NULL;
}

static void* ap_unimpl_stmt(const stmt_t* stmt, const stmt_visitor_t* v, void* ctx) {
    (void)stmt; (void)v;
    const ast_printer_t* p = (ast_printer_t*)ctx;
    append_string(p->sb_p, "<unimpl-stmt>");
    return NULL;
}

// ---------------------------------------------------------------------

static void* visit_literal_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    (void)visitor;
    const ast_printer_t* printer_p = (ast_printer_t*)context;
    print_token(printer_p->sb_p, expr->as.literal_expr.kind);
    return NULL; // TODO: return string instead
}
static void* visit_print_stmt(const stmt_t* stmt, const stmt_visitor_t* visitor, void* context) {
    (void)visitor;
    const ast_printer_t* printer_p = (ast_printer_t*)context;
    append_string(printer_p->sb_p, "(print ");
    char* e = expr_accept(stmt->as.print_stmt.expression, &printer_p->expr_visitor, context);
    append_string(printer_p->sb_p, ")");
    return paren1("print", e);
}

static void* visit_unary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    (void)visitor;
    const ast_printer_t* printer_p = (ast_printer_t*)context;
    append_string(printer_p->sb_p, "(");
    append_string(printer_p->sb_p, expr->as.unary_expr.operator->lexeme);
    append_string(printer_p->sb_p, " ");
    expr_accept(expr->as.unary_expr.right, &printer_p->expr_visitor, context);
    append_string(printer_p->sb_p, ")");
    return NULL; // TODO: return string instead
}

static void* visit_binary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    (void)visitor;
    const ast_printer_t* printer_p = (ast_printer_t*)context;
    append_string(printer_p->sb_p, "(");
    expr_accept(expr->as.binary_expr.left, &printer_p->expr_visitor, context);
    append_string(printer_p->sb_p, " ");
    append_string(printer_p->sb_p, expr->as.binary_expr.operator->lexeme);
    append_string(printer_p->sb_p, " ");
    expr_accept(expr->as.binary_expr.right, &printer_p->expr_visitor, context);
    append_string(printer_p->sb_p, ")");
    return NULL; // TODO: return string instead
}

static void* visit_grouping_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    (void)visitor;
    const ast_printer_t* printer_p = (ast_printer_t*)context;
    append_string(printer_p->sb_p, "(");
    expr_accept(expr->as.grouping_expr.expression, &printer_p->expr_visitor, context);
    append_string(printer_p->sb_p, ")");
    return NULL; // TODO: return string instead
}

// --------------- Public API implementations ----------------

void ast_printer_init(ast_printer_t* printer_p, string_builder_t* sb_p) {
    if (printer_p == NULL || sb_p == NULL) {
        fprintf(stderr, "Error: ast_printer_init received NULL pointer\n");
        exit(EXIT_FAILURE);
    }
    printer_p->sb_p = sb_p;

    // Initialize all visitor function pointers to unimplemented fallbacks
    printer_p->expr_visitor.visit_assign = ap_unimpl_expr;
    printer_p->expr_visitor.visit_binary = ap_unimpl_expr;
    printer_p->expr_visitor.visit_call = ap_unimpl_expr;
    printer_p->expr_visitor.visit_get = ap_unimpl_expr;
    printer_p->expr_visitor.visit_grouping = ap_unimpl_expr;
    printer_p->expr_visitor.visit_literal = ap_unimpl_expr;
    printer_p->expr_visitor.visit_logical = ap_unimpl_expr;
    printer_p->expr_visitor.visit_set = ap_unimpl_expr;
    printer_p->expr_visitor.visit_super = ap_unimpl_expr;
    printer_p->expr_visitor.visit_this = ap_unimpl_expr;
    printer_p->expr_visitor.visit_unary = ap_unimpl_expr;
    printer_p->expr_visitor.visit_variable = ap_unimpl_expr;

    printer_p->stmt_visitor.visit_block = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_function = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_class = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_expression = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_if = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_print = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_return = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_var = ap_unimpl_stmt;
    printer_p->stmt_visitor.visit_while = ap_unimpl_stmt;

    // Wire up the expression visitor function pointers
    printer_p->expr_visitor.visit_binary = visit_binary_expr;
    printer_p->expr_visitor.visit_literal = visit_literal_expr;
    printer_p->expr_visitor.visit_unary = visit_unary_expr;
    printer_p->expr_visitor.visit_grouping = visit_grouping_expr;

    // Wire up the statement visitor function pointers
    printer_p->stmt_visitor.visit_print = visit_print_stmt;
}

char* ast_printer_print_expr(ast_printer_t* printer_p, const expr_t* expr_p) {
    return expr_accept((expr_t*)expr_p, &printer_p->expr_visitor, printer_p);
}

char* ast_printer_print_stmt(ast_printer_t* printer_p, const stmt_t* stmt_p) {
    return stmt_accept((stmt_t*)stmt_p, &printer_p->stmt_visitor, printer_p);
}

