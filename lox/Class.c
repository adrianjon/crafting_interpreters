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

struct instance {
    class_t * p_class;
    map_t * fields; // <char*, object_t*>
};
// TODO needs to bind methods to instances
instance_t * new_instance(class_t * p_class) {
    instance_t * instance = memory_allocate(sizeof(instance_t));
    instance->p_class = p_class;
    instance->fields = map_create(8, (map_config_t){0});
    return instance;
}

struct class {
    const char * name;
    callable_vtable_t * vtable;
    map_t * functions; // <char*, function_t*>
};
const char * instance_get_class_name(const instance_t * p_instance) {
    return p_instance->p_class->name;
}
int class_arity(void * self) {
    return 0;
}
static function_t * find_method(const class_t * self, const char * name) {
    if (self->functions && map_contains(self->functions, name)) {
        return map_get(self->functions, name);
    }
    return NULL;
}
static object_t * class_call(const void * self, interpreter_t * p_interpreter, object_t ** pp_arguments) {
    const object_t * p_object = self;
    class_t * p_class = get_object_value(p_object);
    instance_t * p_instance = new_instance(p_class);
    //p_instance->fields
    const function_t * initializer = find_method(p_class, "init");
    if (initializer) {
        function_call(initializer, p_interpreter, pp_arguments);
    } else {
        // no initializer found, use generic by binding all methods to instance
        map_enumerator_t * it = map_get_enumerator(p_class->functions);
        while (map_enumerator_next(it)) {
            const map_entry_t * entry = map_enumerator_current(it);
            // this is probably wrong, creates new object function with the same function instance
            object_t * field = new_object(OBJECT_FUNCTION, map_entry_value(entry));
            map_put(p_instance->fields, map_entry_key(entry), field);
        }
    }
    object_t * instance_object = new_object(OBJECT_INSTANCE, p_instance);
    return instance_object;
}

static callable_vtable_t class_vtable = {
    .arity = class_arity,
    .call = class_call,
};
callable_vtable_t * get_class_vtable(void) {
    return &class_vtable;
}
class_t * new_class(const char * name) {
    class_t * p_class = memory_allocate(sizeof(class_t));
    p_class->name = name;
    p_class->vtable = &class_vtable;
    p_class->functions = map_create(8, (map_config_t){0});
    return p_class;
}
void class_add_method(class_t * p_class, const char * key, function_t * value) {
    if (!p_class) return;
    map_put(p_class->functions, key, value);
}