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
    VAL_FUNCTION,
    VAL_NIL,
} value_type_t;
typedef struct value value_t;
struct value {
    value_type_t type;
    union {
        double number;
        char* string;
        bool boolean;
        value_t (*function)(int argc, value_t * args);
    } as;
    bool is_on_heap;
};
typedef struct ast_evaluator ast_evaluator_t;

// new API
ast_evaluator_t * ast_evaluator_init(void);
value_t * ast_evaluator_evaluate_expression(ast_evaluator_t * p_evaluator, const expr_t * expr_p);
void ast_evaluator_free(ast_evaluator_t * p_evaluator);
value_t * ast_evaluator_eval_expr(ast_evaluator_t * p_evaluator, const expr_t * expr_p);
value_t * ast_evaluator_eval_stmt(ast_evaluator_t * p_evaluator, const stmt_t * stmt_p);

#endif //LOX_AST_INTERPRETER_H
