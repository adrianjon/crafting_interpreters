//
// Created by adrian on 2025-10-06.
//

#include "Class.h"

#include "../extra/Memory.h"
#include "Callable.h"
#include "Function.h"
#include "Interpreter.h"
#include "Object.h"
#include "../extra/Map.h"

typedef struct instance {
    class_t * p_class;
    map_t * fields; // <char*, object_t*>
} instance_t;
// TODO needs to bind methods to instances
instance_t * new_instance(class_t * p_class) {
    instance_t * instance = memory_allocate(sizeof(instance_t));
    instance->p_class = p_class;
    return instance;
}

struct class {
    const char * name;
    callable_vtable_t * vtable;
    map_t * functions; // <char*, function_t*>
};
int class_arity(void * self) {
    return 0;
}
static function_t * find_method(const class_t * self, const char * name) {
    if (map_contains(self->functions, name)) {
        return map_get(self->functions, name);
    }
    return NULL;
}
static object_t * class_call(const void * self, interpreter_t * p_interpreter, object_t ** pp_arguments) {
    const instance_t * p_self = self;
    function_t * initializer = find_method(p_self->p_class, "init");
    if (initializer) {
        function_call(initializer, p_interpreter, pp_arguments);
    }
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