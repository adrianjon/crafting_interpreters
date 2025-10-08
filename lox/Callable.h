//
// Created by adrian on 2025-10-06.
//

#ifndef LOX_CALLABLE_H
#define LOX_CALLABLE_H
#include "Interpreter.h"
#include "Object.h"

typedef struct {
    int (*arity)(void *self);
    object_t * (*call)(const void *self,
                        interpreter_t * p_interpreter, object_t ** pp_arguments);
} callable_vtable_t;

#endif //LOX_CALLABLE_H