//
// Created by adrian on 2025-09-25.
//

#include "Environment.h"

#include "../extra/Memory.h"
#include "../lox/Object.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Private functions

// Forward declarations

// new implementation
struct environment {
    environment_t * parent_environment;
    size_t variable_count;
    struct {
        char * name;
        object_t * object;
    } variables[MAX_VARS];
};
bool assign_variable(environment_t * p_env, const char * p_name, object_t * p_object) {

    for (size_t i = 0; i < p_env->variable_count; i++) {
        if (memory_compare(p_name, p_env->variables[i].name, strlen(p_name))) {
            p_env->variables[i].object = p_object;
            return true;
        }
    }
    if (p_env->parent_environment) {
        return assign_variable(p_env->parent_environment, p_name, p_object);
    }
    return false;
}
bool declare_variable(environment_t * p_env, const char * p_name,
    const object_t * p_object) {
    if (p_env->variable_count < MAX_VARS) {
        p_env->variables[p_env->variable_count].name = memory_allocate(strlen(p_name) + 1);
        if (!memory_copy(p_env->variables[p_env->variable_count].name, p_name, strlen(p_name) + 1)) {
            return false;
        }
        object_t * copy = new_object(get_object_type(p_object), copy_object_value(p_object));
        p_env->variables[p_env->variable_count].object = copy; // deep copy

        p_env->variable_count++;
    } else {
        return false;
    }
    return true;
}

object_t * env_lookup(environment_t* p_env, const char* p_name) {
    for (size_t i = 0; i < p_env->variable_count; ++i) {
        if (p_env->variables[i].name && strcmp(p_env->variables[i].name, p_name) == 0) {
            return p_env->variables[i].object;
        }
    }
    if (p_env->parent_environment) {
        return env_lookup(p_env->parent_environment, p_name);
    }
    return NULL; // Not found
}
environment_t * create_environment(environment_t * p_parent_env) {
    environment_t * p_env = memory_allocate(sizeof(environment_t));
    p_env->parent_environment = p_parent_env;
    p_env->variable_count = 0;
    for (size_t i = 0; i < MAX_VARS; ++i) {
        p_env->variables[i].name = NULL;
        p_env->variables[i].object = NULL;
    }
    return p_env;
}
environment_t * get_parent_environment(const environment_t * p_env) {
    return p_env->parent_environment;
}