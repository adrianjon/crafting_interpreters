//
// Created by adrian on 2025-09-19.
//

#include "ast_interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "Parser.h"
#include "Token.h"
#include "../extra/Memory.h"


// ----- private helpers ---------
static void value_free(value_t * val) {
    memory_free((void**)&val);
}
bool value_is_truthy(const value_t * val) {
    if (val->type == VAL_NIL) return false;
    if (val->type == VAL_BOOL) return val->as.boolean;
    return true;
}
bool value_equals(const value_t * a, const value_t * b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case VAL_NUMBER: return a->as.number == b->as.number;
        case VAL_BOOL: return a->as.boolean == b->as.boolean;
        case VAL_STRING: return strcmp(a->as.string, b->as.string) == 0;
        case VAL_NIL: return true;
        default: return false;
    }
}
// -------------------------------
static void* visit_literal_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    (void)visitor, (void)context;
    const expr_literal_t* literal_p = &expr->as.literal_expr;
    value_t* val = memory_allocate(sizeof(value_t));

    switch (literal_p->kind->type) {
        case NIL:
            val->type = VAL_NIL; break;
        case KW_TRUE:
            val->type = VAL_BOOL;
            val->as.boolean = true;
            break;
        case KW_FALSE:
            val->type = VAL_BOOL;
            val->as.boolean = false;
            break;
        case NUMBER:
            val->type = VAL_NUMBER;
            val->as.number = strtod(literal_p->kind->lexeme, NULL);
            break;
        case STRING:
            val->type = VAL_STRING;
            val->as.string = literal_p->kind->lexeme;
            break;
        default:
            free(val);
            val = NULL;
            break;
    }
    return val;
}

static void* visit_unary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    const expr_unary_t * unary_p = &expr->as.unary_expr;
    value_t * right = expr_accept(unary_p->right, visitor, context);

    value_t * result = memory_allocate(sizeof(value_t));

    switch (unary_p->operator->type) {
        case MINUS: // Arithmetic negation
            if (right->type == VAL_NUMBER) {
                result->type = VAL_NUMBER;
                result->as.number = -right->as.number;
            } else {
                // TODO: handle error
                printf("visit_unary_expr error");
                result->type = VAL_NIL;
            }
            break;
        case BANG: // Logical not
            result->type = VAL_BOOL;
            result->as.boolean = !value_is_truthy(right);
            break;
        default:
            result->type = VAL_NIL;
            break;
    }
    value_free(right);
    return result;
}

static void* visit_binary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    const expr_binary_t * binary_p = &expr->as.binary_expr;

    value_t * left = expr_accept(binary_p->left, visitor, context);
    value_t * right = expr_accept(binary_p->right, visitor, context);

    value_t * result = memory_allocate(sizeof(value_t));

    switch (binary_p->operator->type) {
        case PLUS:
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER) {
                result->type = VAL_NUMBER;
                result->as.number = left->as.number + right->as.number;
            } else if (left->type == VAL_STRING && right->type == VAL_STRING) {
                printf("ERROR: Not implemented");
                result->type = VAL_NIL;
            } else {
                result->type = VAL_NIL;
            }
            break;
        case MINUS:
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER) {
                result->type = VAL_NUMBER;
                result->as.number = left->as.number - right->as.number;
            } else {
                result->type = VAL_NIL;
            }
            break;
        case STAR:
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER) {
                result->type = VAL_NUMBER;
                result->as.number = left->as.number * right->as.number;
            } else {
                result->type = VAL_NIL;
            }
            break;
        case SLASH:
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER) {
                result->type = VAL_NUMBER;
                result->as.number = left->as.number / right->as.number;
            } else {
                result->type = VAL_NIL;
            }
            break;
        case EQUAL_EQUAL:
            result->type = VAL_BOOL;
            result->as.boolean = value_equals(left, right);
            break;
        default:
            result->type = VAL_NIL;
            break;
    }
    value_free(left);
    value_free(right);
    return result;
}

static void* visit_grouping_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {

}

// ---------- Fallbacks (so missing visitors don't segfault) ----------
static void * eval_unimpl_expr(const expr_t * expr, const expr_visitor_t * v, void * ctx) {
    (void)expr; (void)v;
    printf("Unimplemented expression: %s", g_expr_type_names[expr->type]);
    return NULL;
}
static void * eval_unimpl_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx) {
    (void)stmt; (void)v;
    printf("Unimplemented statement: %s", g_stmt_type_names[stmt->type]);
    return NULL;
}

// Initialize and wire visitor function pointers.
void ast_evaluator_init(ast_evaluator_t * evaluator_p) {
    if (evaluator_p == NULL) {
        fprintf(stderr, "Error: ast_evaluator_init recieved NULL pointer\n");
        exit(EXIT_FAILURE);
    }
    // Initialize all visitor function pointers to unimplemented fallbacks
    evaluator_p->expr_visitor.visit_assign = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_binary = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_call = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_get = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_grouping = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_literal = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_logical = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_set = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_super = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_this = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_unary = eval_unimpl_expr;
    evaluator_p->expr_visitor.visit_variable = eval_unimpl_expr;

    evaluator_p->stmt_visitor.visit_block = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_function = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_class = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_expression = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_if = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_print = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_return = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_var = eval_unimpl_stmt;
    evaluator_p->stmt_visitor.visit_while = eval_unimpl_stmt;


    evaluator_p->expr_visitor.visit_binary = visit_binary_expr;
    evaluator_p->expr_visitor.visit_literal = visit_literal_expr;
    evaluator_p->expr_visitor.visit_unary = visit_unary_expr;

}

// Entry points
value_t * ast_evaluator_eval_expr(ast_evaluator_t * evaluator_p, const expr_t * expr_p) {
    return expr_accept(expr_p, &evaluator_p->expr_visitor, evaluator_p);
}

value_t * ast_evaluator_eval_stmt(ast_evaluator_t * evaluator_p, const stmt_t * stmt_p) {
    return stmt_accept(stmt_p, &evaluator_p->stmt_visitor, evaluator_p);
}