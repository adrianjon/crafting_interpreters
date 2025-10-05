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

#include "Token.h"

// Private functions

// Forward declarations

// new implementation
struct environment {
    environment_t * enclosing;
    map_t * values; // <char*,object_t*>
};

environment_t * new_environment(environment_t * p_enclosing) {
    environment_t * p_env = memory_allocate(sizeof(environment_t));
    p_env->enclosing = p_enclosing;
    // values is a map of type <char*, object_t*> keys are owned, values not
    p_env->values = map_create(4, NULL, NULL, NULL, NULL);
    return p_env;
}

object_t * environment_get(token_t * p_name, environment_t * p_env) {
    if (map_contains(p_env->values, p_name->lexeme)) {
         return map_get(p_env->values, p_env->values);
    }
    if (p_env->enclosing) return environment_get(p_name, p_env->enclosing);

    fprintf(stderr, "Undefined variable ''.");
    exit(EXIT_FAILURE);
}
void environment_assign(token_t * p_name, object_t * p_value, environment_t * p_env) {
    if (map_contains(p_env->values, p_name->lexeme)) {
        map_put(p_env->values, p_name->lexeme, p_value);
        return;
    }
    if (p_env->enclosing) {
        environment_assign(p_name, p_value, p_env->enclosing);
        return;
    }

    fprintf(stderr, "Undefined variable ''.");
    exit(EXIT_FAILURE);
}
void environment_define(const char * p_name, object_t * p_value,
                            const environment_t * p_env) {
    map_put(p_env->values, p_name, p_value);
}
environment_t * environment_ancestor(const int distance, environment_t * p_env) {

    for (int i = 0; i < distance; i++) {
        p_env = p_env->enclosing;
    }
    return p_env;
}
object_t * environment_get_at(const int distance, const char * name,
                                environment_t * p_env) {
    environment_t * p_ancestor = environment_ancestor(distance, p_env);
    if (!p_ancestor) return NULL;
    return map_get(p_ancestor->values, name);
}
void environment_assign_at(const int distance, const token_t * p_name, object_t * p_value,
                                environment_t * p_env) {
    environment_t * p_ancestor = environment_ancestor(distance, p_env);
    if (!p_ancestor) return;
    map_put(p_ancestor->values, p_name->lexeme, p_value);
}