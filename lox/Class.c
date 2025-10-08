//
// Created by adrian on 2025-10-06.
//

#include "Class.h"

#include "../extra/Memory.h"
#include "Callable.h"
#include "Interpreter.h"
#include "Object.h"

struct class {
    const char * name;
    callable_vtable_t * vtable;
};
int class_arity(void * self) {
    return 0;
}
static object_t * class_call(const void * self, interpreter_t * p_interpreter, object_t ** pp_arguments) {
    return NULL;
}

static callable_vtable_t class_vtable = {
    .arity = class_arity,
    .call = class_call,
};

class_t * new_class(const char * name) {
    class_t * p_class = memory_allocate(sizeof(class_t));
    p_class->name = name;
    p_class->vtable = &class_vtable;
    return p_class;
}