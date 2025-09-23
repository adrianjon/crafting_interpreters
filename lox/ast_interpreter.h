//
// Created by adrian on 2025-09-19.
//

#ifndef LOX_AST_INTERPRETER_H
#define LOX_AST_INTERPRETER_H

#include <stdbool.h>
#include "Expr.h"
#include "Stmt.h"

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_NIL,
} value_type_t;

typedef struct {
    value_type_t type;
    union {
        double number;
        char* string;
        bool boolean;
    } as;
} value_t;

typedef struct ast_evaluator {
    value_t last_result;

    bool had_runtime_error;
    char error_message[256];

    expr_visitor_t expr_visitor;
    stmt_visitor_t stmt_visitor;
} ast_evaluator_t;

// Initialize and wire visitor function pointers.
void ast_evaluator_init(ast_evaluator_t * evaluator_p);

// Entry points
value_t * ast_evaluator_eval_expr(ast_evaluator_t * evaluator_p, const expr_t * expr_p);

value_t * ast_evaluator_eval_stmt(ast_evaluator_t * evaluator_p, const stmt_t * stmt_p);

#endif //LOX_AST_INTERPRETER_H
