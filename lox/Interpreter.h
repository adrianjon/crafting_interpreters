//
// Created by adrian on 2025-09-29.
//

#ifndef LOX_INTERPRETER_H
#define LOX_INTERPRETER_H

#include "Expr.h"
#include "Stmt.h"
#include "Object.h"

// Interpreter implements the Visitors for Expressions and Statements
typedef struct interpreter interpreter_t;

// API
interpreter_t * new_interpreter (void);
void interpreter_free           (interpreter_t ** p_interpreter);
void interpret                  (const dynamic_array_t * statements, interpreter_t * p_interpreter);
object_t * evaluate             (const expr_t * p_expr, interpreter_t * p_interpreter);
void * execute                  (const stmt_t * p_stmt, interpreter_t * p_interpreter);

#endif //LOX_INTERPRETER_H