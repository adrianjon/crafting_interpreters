//
// Created by adrian on 2025-10-11.
//

#include "interpreter.h"
#include <stdio.h>
#include <string.h>
#include "../tests/map/map2.h"
#include "environment.h"
#include "value.h"
#include "object.h"
#include "stmt.h"
#include "expr.h"

static void execute(interpreter_t * p_i, stmt_t const * p_s);
static value_t evaluate(interpreter_t * i, expr_t const * e);
static value_t * lookup(interpreter_t const * p_i, token_t const * p_t, expr_t const * p_e);

// int embedded in void * for map usage
// static void * copy_int(void const * value) {
//     int * copy = malloc(sizeof(int));
//     if (!copy) exit(1);
//     *copy = *(int *)value;
//     return copy;
// }
// static void free_int(void * value) {
//     free(value);
// }
// expr_t* functions for map usage, no deep copy, no ownership
// can use copy_int and free_int
// static size_t hash_expr(void const * value) {
//     return (size_t)value;
// }
// static bool cmp_expr(void const * key_a, void const * key_b) {
//     if (!key_a || !key_b) return false;
//     return(memcmp(key_a, key_b, sizeof(expr_t)) == 0);
// }
// static void expr_free(void * p_expr) {
//     free(p_expr);
// }
// static void * copy_expr(void const * ptr) {
//     expr_t * copy = malloc(sizeof(expr_t));
//     memcpy(copy, ptr, sizeof(expr_t));
//     return copy;
// }

void interpret(interpreter_t * p_interpreter, list_t * p_statements) {
    if (!p_interpreter || !p_statements) return;
    if (!p_interpreter->globals) {
        p_interpreter->globals = environment_create(NULL);
    }
    for (size_t i = 0; i < p_statements->count; i++) {
        execute(p_interpreter, p_statements->data[i]);
        // TODO error handling
    }
}

static void execute(interpreter_t * p_i, stmt_t const * p_s) {
    switch (p_s->type) {
        case STMT_BLOCK: {
            stmt_block_t const stmt = p_s->as.block_stmt;
            environment_t * p_prev = p_i->environment;
            environment_t * p_env = environment_create(p_prev);
            p_i->environment = p_env;
            for (size_t i = 0; i < stmt.count; i++) {
                execute(p_i, stmt.statements[i]);
                // TODO handle runtime error
            }
            break;
        }
        case STMT_FUNCTION:
        case STMT_CLASS:
            break;
        case STMT_EXPRESSION: {
            stmt_expression_t const stmt = p_s->as.expression_stmt;
            evaluate(p_i, stmt.expression);
            break;
        }
        case STMT_IF:
        case STMT_PRINT: {
            stmt_print_t const stmt = p_s->as.print_stmt;
            value_t const val = evaluate(p_i, stmt.expression);
            switch (val.type) {
                case VAL_NUMBER:
                    printf("%f\n", val.as.number);
                    break;
                case VAL_BOOL:
                    printf("%s\n", val.as.boolean ? "true" : "false");
                    break;
                case VAL_NIL:
                    printf("nil\n");
                    break;
                case VAL_OBJ:
                    break;
            }
            break;
        }
        case STMT_RETURN: break;
        case STMT_VAR: {
            stmt_var_t const stmt = p_s->as.var_stmt;
            value_t val = value_nil();
            if (stmt.initializer) val = evaluate(p_i, stmt.initializer);
            // TODO handle runtime error
            if (p_i->environment) {
                environment_define(p_i->environment, stmt.name->lexeme, &val);
            }
            // globals
            environment_define(p_i->globals, stmt.name->lexeme, &val);
            break;
        }
        case STMT_WHILE:
        default:
            fprintf(stderr, "Not implemented (%d)\n", p_s->type);
            exit(EXIT_FAILURE);
    }
}
static value_t evaluate(interpreter_t * p_i, expr_t const * p_e) {
    value_t val = value_nil();
    switch (p_e->type) {
        case EXPR_ASSIGN: {
            expr_assign_t const expr = p_e->as.assign_expr;
            if (expr.value) val = evaluate(p_i, expr.value);
            // TODO handle runtime error
            if (expr.target->type == EXPR_VARIABLE &&
                expr.target->as.variable_expr.depth >= 0) {
                environment_assign_at(p_i->environment,
                         expr.target->as.variable_expr.depth,
                         expr.target->as.variable_expr.name->lexeme, &val);
            } else {
                environment_assign(p_i->globals,
                    expr.target->as.variable_expr.name->lexeme, &val);
            }
            break;
        }
        case EXPR_BINARY:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_CALL:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_GET:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_GROUPING:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_LITERAL: {
            expr_literal_t const expr = p_e->as.literal_expr;
            switch (expr.kind->type) {
                case NUMBER:
                    val.type = VAL_NUMBER;
                    val.as.number = strtod(expr.kind->lexeme, NULL);
                    break;
                case STRING:
                    char const * str = expr.kind->lexeme;
                    obj_string_t * s = obj_string_new(str);
                    val = value_object((object_t*)s);
                    break;
                case NIL:
                    val = value_nil();
                    break;
                default:
                    fprintf(stderr, "Not implemented (%d)\n", p_e->type);
                    exit(EXIT_FAILURE);
            }
            break;
        }
        case EXPR_LOGICAL:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_SET:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_SUPER:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_THIS:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_UNARY:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
            break;
        case EXPR_VARIABLE:
            expr_variable_t const expr = p_e->as.variable_expr;
            val = *lookup(p_i, expr.name, p_e);
            break;
        default:
            fprintf(stderr, "Not implemented (%d)\n", p_e->type);
            exit(EXIT_FAILURE);
    }
    return val;
}
static value_t * lookup(interpreter_t const * p_i, token_t const * p_t, expr_t const * p_e) {
    int distance = -1;
    if (p_e->type == EXPR_VARIABLE) {
        distance = p_e->as.variable_expr.depth;
    }
    if (distance >= 0) {
        return environment_get_at(p_i->environment, distance, p_t->lexeme);
    }
    return environment_get(p_i->globals, p_t->lexeme);
}