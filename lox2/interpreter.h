//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_INTERPRETER_H
#define LOX_INTERPRETER_H
#include "list.h"

typedef struct {
    void * globals;
} interpreter_t;

void interpret(interpreter_t * p_interpreter, list_t * p_statements);
#endif //LOX_INTERPRETER_H