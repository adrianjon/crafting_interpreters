//
// Created by adrian on 2025-09-25.
//

#ifndef LOX_ENVIRONMENT_H
#define LOX_ENVIRONMENT_H

#include "Object.h"
#include "Token.h"

typedef struct environment environment_t;
#define MAX_GLOBALS 128
#define MAX_VARS 128

environment_t * new_environment(environment_t * p_enclosing);
object_t * environment_get(token_t * p_name, environment_t * p_env);
void environment_assign(token_t * p_name, object_t * p_value, environment_t * p_env);
void environment_define(const char * p_name, object_t * p_value, const environment_t * p_env);
environment_t * environment_ancestor(int distance, environment_t * p_env);
object_t * environment_get_at(int distance, const char * name, environment_t * p_env);
void environment_assign_at(int distance, const token_t * p_name, object_t * p_value, environment_t * p_env);
#endif //LOX_ENVIRONMENT_H