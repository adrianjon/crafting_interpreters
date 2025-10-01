//
// Created by adrian on 2025-10-01.
//

#include "Function.h"
#include "Environment.h"
#include "../extra/Memory.h"

struct function {
    stmt_function_t * p_declaration;
};

function_t * new_function(stmt_function_t * p_declaration) {
    function_t * p_new = memory_allocate(sizeof(function_t));
    p_new->p_declaration = p_declaration;
    return p_new;
}
stmt_function_t * get_function_declaration(const function_t * p_function) {
    return p_function->p_declaration;
}
object_t * call_function(const function_t * p_function, interpreter_t * p_interpreter,
    object_t ** pp_arguments) {
    environment_t * p_parent_env = get_interpreter_environment(p_interpreter);
    environment_t * p_env = create_environment(p_parent_env);
    set_interpreter_environment(p_interpreter, p_env);

    for (size_t i = 0; i < *p_function->p_declaration->params_count; i++) {
        declare_variable(p_env, p_function->p_declaration->params[i]->lexeme, pp_arguments[i]);
    }

    // TODO fix this, p_function->p_declaration->body pointer is garbage
    for (size_t i = 0; i < *p_function->p_declaration->count; i++) {
        execute(p_function->p_declaration->body[i], p_interpreter);
    }
    set_interpreter_environment(p_interpreter, p_parent_env);
    free_environment(p_env);
    return NULL;
}