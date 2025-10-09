//
// Created by adrian on 2025-10-01.
//

#include "Function.h"

#include <stdio.h>

#include "Environment.h"
#include "Callable.h"
#include "../extra/Memory.h"
#include "../extra/Map.h"

struct function {
    stmt_function_t * p_declaration;
    environment_t * closure;
    callable_vtable_t * vtable;
};
static int arity(void * self) {
    return 0;
}
static object_t * call(const void * self, interpreter_t * p_interpreter, object_t ** pp_arguments) {
    const function_t * p_function = self;
    environment_t * p_env = new_environment(p_function->closure);
    for (size_t i = 0; i < *p_function->p_declaration->params_count; i++) {
        environment_define(p_function->p_declaration->params[i]->lexeme,
            pp_arguments[i], p_env);
    }
    //    environment_t * p_parent_env = get_interpreter_environment(p_interpreter);

    environment_t * previous = get_interpreter_environment(p_interpreter);
    set_interpreter_environment(p_interpreter, p_env);
    object_t * p_ret = NULL;
    for (size_t i = 0; i < *p_function->p_declaration->count; i++) {
        p_ret = execute(p_function->p_declaration->body[i], p_interpreter);
        if (p_ret) {
            set_interpreter_environment(p_interpreter, previous);
            return p_ret;
        }
    }
    set_interpreter_environment(p_interpreter, previous);
    return NULL;
}
static callable_vtable_t function_vtable = {
    .arity = arity,
    .call = call
};
callable_vtable_t * get_function_vtable(void) {
    return &function_vtable;
}
function_t * new_function(stmt_function_t * p_declaration, environment_t * closure) {
    function_t * p_new = memory_allocate(sizeof(function_t));
    p_new->p_declaration = p_declaration;
    // should clone the environment as the funciton closure
    p_new->closure = closure;
    p_new->vtable = &function_vtable;
    return p_new;
}
size_t function_arity(const function_t * p_function) {
    return *p_function->p_declaration->params_count;
}
stmt_function_t * get_function_declaration(const function_t * p_function) {
    return p_function->p_declaration;
}
object_t * function_call(const function_t * p_function, interpreter_t * p_interpreter,
    object_t ** pp_arguments) {
    return p_function->vtable->call(p_function, p_interpreter, pp_arguments);
}