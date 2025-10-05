//
// Created by adrian on 2025-09-25.
//

#include "Environment.h"

#include "../extra/Memory.h"
#include "../extra/Map.h"
#include "Object.h"
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
    map_t * values;
    struct {
        char * name;
        object_t * object;
    } variables[MAX_VARS];
};

static size_t hash_expr(const void * key, const size_t num_buckets) {
    return (size_t)key % num_buckets;
}
static bool cmp_expr(const void * key1, const void * key2) {
    return key1 == key2;
}
static void clean_expr(const void * key, const void * value) {
    (void)key, (void)value;
}
static const void * copy_expr(const void * key) {
    return key;
}

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
    // values is a map of type <char*, object_t*> keys are owned, values not
    p_env->values = map_create(4, NULL, NULL, NULL, NULL);
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
environment_t * copy_environment(environment_t * p_env) {
    if (!p_env) return NULL;
    environment_t * p_env_copy = memory_allocate(sizeof(environment_t));
    memory_copy(p_env_copy, p_env, sizeof(environment_t));
    p_env_copy->parent_environment = copy_environment(p_env->parent_environment);
    return p_env_copy;
}
environment_t *ancestor(const int distance, environment_t * p_env) {
    for (int i = 0; i < distance; i++) {
        p_env = p_env->parent_environment;
    }
    return p_env;
}
object_t * get_at(const int distance, const char * name, environment_t * p_env) {
    environment_t * env = ancestor(distance, p_env);
    if (!env) return NULL;
    return map_get(env->values, name);
}
