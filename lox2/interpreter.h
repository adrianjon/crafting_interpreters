//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_INTERPRETER_H
#define LOX_INTERPRETER_H
#include "environment.h"
#include "list.h"
#include "expr.h"
#include "utils/map.h"

typedef struct {
    environment_t * globals;
    environment_t * environment;
    map_t * locals; // <expr_t*,int>
} interpreter_t;

void interpret(interpreter_t * p_interpreter, list_t * p_statements);
void interpreter_resolve(interpreter_t * p_interpreter, expr_t * p_expr, int depth);
void free_interpreter(interpreter_t * p_interpreter);


#endif //LOX_INTERPRETER_H