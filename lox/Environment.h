//
// Created by adrian on 2025-09-25.
//

#ifndef LOX_ENVIRONMENT_H
#define LOX_ENVIRONMENT_H

#include "Object.h"

typedef struct environment environment_t;
#define MAX_GLOBALS 128
#define MAX_VARS 128

// void set_global(const char * name, value_t * value);
// value_t * get_global(const char * name);
// void free_globals(void);

environment_t * init_global_scope(void);
environment_t * create_environment(environment_t * p_parent_env);
environment_t * get_parent_environment(const environment_t * p_env);
bool assign_variable(environment_t * p_env, const char * p_name, object_t * p_object);
bool declare_variable(environment_t * p_env, const char * p_name, const object_t * p_object);
object_t * env_lookup(environment_t * p_env, const char * p_name);
environment_t * copy_environment(environment_t * p_env);

#endif //LOX_ENVIRONMENT_H