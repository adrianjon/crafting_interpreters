//
// Created by adrian on 2025-09-19.
//
#include "ast_interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "Parser.h"
#include "Token.h"
#include "Environment.h"
#include "../extra/Memory.h"
struct ast_evaluator {
    value_t last_result;

    bool had_runtime_error;
    char error_message[256];

    expr_visitor_t expr_visitor;
    stmt_visitor_t stmt_visitor;

    environment_t * current_env; // ?
};
// Externs TODO change this

extern environment_t * g_scope;
// extern void set_global(const char * name, value_t * value);
// value_t * get_global(const char * name);

// Forward declarations
static void* visit_literal_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
static void* visit_unary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
static void* visit_binary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
static void* visit_grouping_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
static void * visit_variable_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
static void * visit_assignment_expr(const expr_t * expr, const expr_visitor_t * visitor, void * context);
static void * eval_unimpl_expr(const expr_t * expr, const expr_visitor_t * v, void * ctx);
static void * visit_print_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx);
static void * visit_expression_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx);
static void * visit_variable_stmt(const stmt_t* stmt, const stmt_visitor_t* visitor, void* context);
static void * visit_block_stmt(const stmt_t * stmt, const stmt_visitor_t* visitor, void* ctx);
static void * eval_unimpl_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx);
// Public API
ast_evaluator_t * ast_evaluator_init(void) {
    ast_evaluator_t * p_evaluator = memory_allocate(sizeof(ast_evaluator_t));
    if (p_evaluator == NULL) {
        fprintf(stderr, "Error: ast_evaluator_init recieved NULL pointer\n");
        exit(EXIT_FAILURE);
    }
    p_evaluator->current_env = g_scope;
    // Initialize all visitor function pointers to unimplemented fallbacks
    p_evaluator->expr_visitor.visit_assign = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_binary = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_call = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_get = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_grouping = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_literal = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_logical = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_set = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_super = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_this = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_unary = eval_unimpl_expr;
    p_evaluator->expr_visitor.visit_variable = eval_unimpl_expr;

    p_evaluator->stmt_visitor.visit_block = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_function = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_class = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_expression = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_if = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_print = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_return = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_var = eval_unimpl_stmt;
    p_evaluator->stmt_visitor.visit_while = eval_unimpl_stmt;


    p_evaluator->expr_visitor.visit_binary = visit_binary_expr;
    p_evaluator->expr_visitor.visit_literal = visit_literal_expr;
    p_evaluator->expr_visitor.visit_unary = visit_unary_expr;
    p_evaluator->expr_visitor.visit_grouping = visit_grouping_expr;
    p_evaluator->expr_visitor.visit_variable = visit_variable_expr;
    p_evaluator->expr_visitor.visit_assign = visit_assignment_expr;

    p_evaluator->stmt_visitor.visit_print = visit_print_stmt;
    p_evaluator->stmt_visitor.visit_expression = visit_expression_stmt;
    p_evaluator->stmt_visitor.visit_var = visit_variable_stmt;
    p_evaluator->stmt_visitor.visit_block = visit_block_stmt;

    return p_evaluator;
}
value_t * ast_evaluator_evaluate_expression(ast_evaluator_t * p_evaluator, const expr_t * expr_p) {
    // TODO implement these
    return NULL;
}
void ast_evaluator_free(ast_evaluator_t * p_evaluator) {
    if  (!p_evaluator) return;
    memory_free((void**)&p_evaluator);
    p_evaluator = NULL;
}
value_t * ast_evaluator_eval_expr(ast_evaluator_t * p_evaluator, const expr_t * expr_p) {
    return expr_accept(expr_p, &p_evaluator->expr_visitor, p_evaluator);
}
value_t * ast_evaluator_eval_stmt(ast_evaluator_t * p_evaluator, const stmt_t * stmt_p) {
    return stmt_accept(stmt_p, &p_evaluator->stmt_visitor, p_evaluator);
}
// Private functions
static void value_free(value_t * val) {

    // if (val->type == VAL_STRING) {
    //     memory_free((void**)&val->as.string);
    // }
    if (!val) return;
    if (val->is_on_heap) {
        memory_free((void**)&val);
    }
}
static bool value_is_truthy(const value_t * val) {
    if (val->type == VAL_NIL) return false;
    if (val->type == VAL_BOOL) return val->as.boolean;
    return true;
}
static bool value_equals(const value_t * a, const value_t * b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case VAL_NUMBER: return a->as.number == b->as.number;
        case VAL_BOOL: return a->as.boolean == b->as.boolean;
        case VAL_STRING: return strcmp(a->as.string, b->as.string) == 0;
        case VAL_NIL: return true;
        default: return false;
    }
}
static void * visit_literal_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    (void)visitor, (void)context;
    const expr_literal_t* literal_p = &expr->as.literal_expr;
    value_t* val = memory_allocate(sizeof(value_t));
    val->is_on_heap = true;

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
static void * visit_unary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    const expr_unary_t * unary_p = &expr->as.unary_expr;
    value_t * right = expr_accept(unary_p->right, visitor, context);

    value_t * result = memory_allocate(sizeof(value_t));
    result->is_on_heap = true;

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
static void * visit_binary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    const expr_binary_t * binary_p = &expr->as.binary_expr;

    value_t * left = expr_accept(binary_p->left, visitor, context);
    value_t * right = expr_accept(binary_p->right, visitor, context);

    value_t * result = memory_allocate(sizeof(value_t));
    result->is_on_heap = true;

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
static void * visit_grouping_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    const expr_grouping_t * grouping_p = &expr->as.grouping_expr;
    return expr_accept(grouping_p->expression, visitor, context);
}
static void * visit_variable_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
    const ast_evaluator_t * p_evaluator = context;
    const expr_variable_t * p_expr = &expr->as.variable_expr;
    return env_lookup(p_evaluator->current_env, p_expr->name->lexeme);
}
static void * visit_assignment_expr(const expr_t * expr, const expr_visitor_t * visitor, void * context) {
    const expr_assign_t * p_expr = &expr->as.assign_expr;
    ast_evaluator_t * p_evaluator = context;

    value_t * p_val = ast_evaluator_eval_expr(p_evaluator, p_expr->value);
    assign_variable(p_evaluator->current_env, p_expr->name->lexeme, p_val);
    return p_val;
}
static void * eval_unimpl_expr(const expr_t * expr, const expr_visitor_t * v, void * ctx) {
    (void)expr; (void)ctx;
    printf("Unimplemented expression: %s\n", g_expr_type_names[expr->type]);
    return NULL;
}
static void * visit_variable_stmt(const stmt_t * stmt, const stmt_visitor_t * visitor, void * context) {

    const ast_evaluator_t * evaluator = context;
    value_t * p_val = expr_accept(stmt->as.var_stmt.initializer, &evaluator->expr_visitor, context);

    declare_variable(evaluator->current_env, stmt->as.var_stmt.name->lexeme, p_val);
    memory_free((void**)&p_val);
    return NULL;
}
static void * visit_expression_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx) {
    const ast_evaluator_t * evaluator = ctx;
    void * p = expr_accept(stmt->as.expression_stmt.expression, &evaluator->expr_visitor, ctx);
    memory_free(&p);
    return NULL;
}
const char * stringify(const value_t * p_value) {
    if (!p_value) return "stringify_error";
    static char num_buffer[32]; // static buffer for numbers (not thread-safe) TODO better method
    switch (p_value->type) {
        case VAL_STRING:
            return p_value->as.string;
        case VAL_NUMBER:
            snprintf(num_buffer, sizeof(num_buffer), "%g", p_value->as.number);
            return num_buffer;
        case VAL_BOOL:
            return p_value->as.boolean ? "true" : "false";
        case VAL_NIL:
            return "nil";
        default:
            return "stringify_error";
    }
}
static void * visit_print_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx) {
    const ast_evaluator_t * evaluator = ctx;
    const value_t * p_value = expr_accept(stmt->as.print_stmt.expression, &evaluator->expr_visitor, ctx);
    printf("%s\n", stringify(p_value));
    if (stmt->as.print_stmt.expression->type != EXPR_VARIABLE) {
        memory_free((void**)&p_value);
    }
    return NULL;
}
static void * visit_block_stmt(const stmt_t * stmt, const stmt_visitor_t* visitor, void* ctx) {
    environment_t * p_new_env = create_environment(((ast_evaluator_t*)ctx)->current_env);
    ((ast_evaluator_t*)ctx)->current_env = p_new_env;
    for (size_t i = 0; i < *stmt->as.block_stmt.count; i++) {
        ast_evaluator_eval_stmt(ctx, stmt->as.block_stmt.statements[i]);
    }
    ((ast_evaluator_t*)ctx)->current_env = get_parent_environment(p_new_env); // reset env context
    free_environment(p_new_env);
    return NULL;
}
static void * eval_unimpl_stmt(const stmt_t * stmt, const stmt_visitor_t * v, void * ctx) {
    (void)stmt; (void)ctx;
    printf("Unimplemented statement: %s\n", g_stmt_type_names[stmt->type]);
    return NULL;
}