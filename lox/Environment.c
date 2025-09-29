//
// Created by adrian on 2025-09-25.
//

#include "Environment.h"

#include "ast_interpreter.h"
#include "../extra/Memory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// static struct {
//     char * name;
//     value_t value;
// } g_globals[MAX_GLOBALS];
// static size_t g_globals_count = 0;
// Forward declarations
// static void assign_variable(environment_t * p_env, const char * p_name, const value_t * p_value);
// static bool try_update_variable(environment_t * p_env, const char * p_name, const value_t * p_value);
// value_t * env_lookup(environment_t* p_env, const char* p_name);
// Public API
// void set_global(const char * name, value_t * value) {
//     for (size_t i = 0; i < g_globals_count; ++i) {
//         if (strcmp(g_globals[i].name, name) == 0) {
//             if (value->type == VAL_STRING) {
//                 g_globals[i].value.type = VAL_STRING;
//                 g_globals[i].value.as.string = strdup(value->as.string); // TODO need to free
//                 g_globals[i].value.is_on_heap = false;
//             } else {
//                 g_globals[i].value = *value;
//                 g_globals[i].value.is_on_heap = false;
//             }
//
//             return;
//         }
//     }
//     if (g_globals_count < MAX_GLOBALS) {
//         g_globals[g_globals_count].name = strdup(name);
//         if (value->type == VAL_STRING) {
//             g_globals[g_globals_count].value.type = VAL_STRING;
//             g_globals[g_globals_count].value.as.string = strdup(value->as.string);
//             g_globals[g_globals_count].value.is_on_heap = false;
//         } else {
//             g_globals[g_globals_count].value = *value;
//             g_globals[g_globals_count].value.is_on_heap = false;
//         }
//         ++g_globals_count;
//     } else {
//         fprintf(stderr, "Global variable limit reached!\n");
//     }
// }
// value_t * get_global(const char * name) {
//     for (size_t i = 0; i < g_globals_count; ++i) {
//         if (strcmp(g_globals[i].name, name) == 0) {
//             return &g_globals[i].value;
//         }
//     }
//     return NULL;
// }
// void free_globals(void) {
//     for (size_t i = 0; i < g_globals_count; ++i) {
//         memory_free((void**)&g_globals[i].name);
//         if (g_globals[i].value.type == VAL_STRING) {
//             memory_free((void**)&g_globals[i].value.as.string);
//         }
//         g_globals[i].value.type = 0;
//         // TODO if value_t contains heap allocations, free those too
//     }
//     g_globals_count = 0;
// }
// Private functions


// Forward declarations
value_t lox_clock(int argc, value_t * args);



// new implementation
struct environment {
    environment_t * parent_environment;
    size_t variable_count;
    struct {
        char * name;
        value_t value;
    } variables[MAX_VARS];
};
// static void assign_variable(environment_t * p_env, const char * p_name, const value_t * p_value) {
//     if (!try_update_variable(p_env, p_name, p_value)) {
//         if ( < MAX_GLOBALS) {
//             g_globals[g_globals_count].name = strdup(name);
//             if (value->type == VAL_STRING) {
//                 g_globals[g_globals_count].value.type = VAL_STRING;
//                 g_globals[g_globals_count].value.as.string = strdup(value->as.string);
//                 g_globals[g_globals_count].value.is_on_heap = false;
//             } else {
//                 g_globals[g_globals_count].value = *value;
//                 g_globals[g_globals_count].value.is_on_heap = false;
//             }
//             ++g_globals_count;
//         } else {
//             fprintf(stderr, "Global variable limit reached!\n");
//         }
//     }
// }

// only in local scope
void assign_variable(environment_t * p_env, const char * p_name, const value_t * p_val) {
    value_t * x = env_lookup(p_env, p_name);
    if (x) {
        // overwriting exisitng value
        if (x->type == VAL_STRING) {
            memory_free((void**)&x->as.string);
        }
        // free old value TODO fix bug here
        // memory_free((void**)&x);

        if (p_val->type == VAL_STRING) {
            x->type = VAL_STRING;
            x->as.string = memory_allocate(strlen(p_val->as.string) + 1);
            if (!memory_copy(x->as.string, p_val->as.string, strlen(p_val->as.string) + 1)) {
                fprintf(stderr, "Memory allocation error");
                exit(EXIT_FAILURE);
            }
        } else {
            *x = *p_val;
        }

    } else {
        declare_variable(p_env, p_name, p_val);
    }
}
void declare_variable(environment_t * p_env, const char * p_name, const value_t * p_value) {
    if (p_env->variable_count < MAX_VARS) {
        p_env->variables[p_env->variable_count].name = memory_allocate(strlen(p_name) + 1);
        if (!memory_copy(p_env->variables[p_env->variable_count].name, p_name, strlen(p_name) + 1)) {
            fprintf(stderr, "Memory allocation error!\n");
            exit(EXIT_FAILURE);
        }
        if (p_value->type == VAL_STRING) {
            p_env->variables[p_env->variable_count].value.type = VAL_STRING;
            p_env->variables[p_env->variable_count].value.as.string = memory_allocate(strlen(p_value->as.string) + 1);
            if (!memory_copy(p_env->variables[p_env->variable_count].value.as.string, p_value->as.string, strlen(p_value->as.string) + 1)) {
                fprintf(stderr, "Memory allocation error!\n");
                exit(EXIT_FAILURE);
            }
            p_env->variables[p_env->variable_count].value.is_on_heap = false;
        } else {
            p_env->variables[p_env->variable_count].value = *p_value;
            p_env->variables[p_env->variable_count].value.is_on_heap = false;
        }
        p_env->variable_count++;
    } else {
        fprintf(stderr, "Variable variable limit reached!\n");
    }
}
static bool try_update_variable(
    environment_t * p_env,
    const char * p_name,
    const value_t * p_value) {
    // if (!p_env) {
    //     return false;
    // }
    // for (size_t i = 0; i < p_env->variable_count; ++i) {
    //     if (strcmp(p_env->variables[i].name, p_name) == 0) {
    //         if (p_value->type == VAL_STRING) {
    //             p_env->variables[i].value.type = VAL_STRING;
    //             p_env->variables[i].value.as.string = memory_allocate(strlen(p_value->as.string) + 1);
    //             if (!memory_copy(p_env->variables[i].value.as.string, p_value->as.string, strlen(p_value->as.string) + 1)) {
    //                 printf("error memory_copy\n");
    //                 exit(EXIT_FAILURE);
    //             }
    //             p_env->variables[i].value.is_on_heap = false;
    //         } else {
    //             g_globals[i].value = *p_value;
    //             g_globals[i].value.is_on_heap = false;
    //         }
    //         return true;
    //     }
    // }
    // try_update_variable(p_env->parent_environment, p_name, p_value);
    return false;
}

value_t * env_lookup(environment_t* p_env, const char* p_name) {
    for (size_t i = 0; i < p_env->variable_count; ++i) {
        if (p_env->variables[i].name && strcmp(p_env->variables[i].name, p_name) == 0) {
            return &p_env->variables[i].value;
        }
    }
    if (p_env->parent_environment) {
        return env_lookup(p_env->parent_environment, p_name);
    }
    return NULL; // Not found
}
void free_environment(environment_t * p_env) {
    for (size_t i = 0; i < p_env->variable_count; ++i) {
        memory_free((void**)(&p_env->variables[i].name));
        if (p_env->variables[i].value.type == VAL_STRING) {
            memory_free((void**)&p_env->variables[i].value.as.string);
        }
    }
    p_env->variable_count = 0;
    memory_free((void**)&p_env);
}
environment_t * create_environment(environment_t * p_parent_env) {
    environment_t * p_env = memory_allocate(sizeof(environment_t));
    p_env->parent_environment = p_parent_env;
    p_env->variable_count = 0;
    for (size_t i = 0; i < MAX_VARS; ++i) {
        p_env->variables[i].name = NULL;
        p_env->variables[i].value = (value_t){0};
    }
    return p_env;
}
environment_t * get_parent_environment(const environment_t * p_env) {
    return p_env->parent_environment;
}
static value_t make_native(value_t (*native_fn)(int argc, value_t * args)) {
    value_t v;
    v.type = VAL_FUNCTION;
    v.as.function = native_fn;
    v.is_on_heap = false;
    return v;
}
struct {
    const char * name;
    value_t (*native_fn)(int argc, value_t * args);
} g_native_functions[] = {
    { .name = "lox_clock", lox_clock },
};
environment_t * init_global_scope(void) {
    environment_t * p_env = memory_allocate(sizeof(environment_t));
    p_env->parent_environment = NULL;
    p_env->variable_count = 0;
    for (size_t i = 0; i < MAX_VARS; ++i) {
        p_env->variables[i].name = NULL;
        p_env->variables[i].value = (value_t){0};
    }
    for (size_t i = 0; i < sizeof(g_native_functions) / sizeof(g_native_functions[0]); i++) {
        value_t v = make_native(g_native_functions[i].native_fn);
        declare_variable(p_env, g_native_functions[i].name, &v);
    }
    return p_env;
}

// Global lox native functions
value_t lox_clock(int argc, value_t * args) {
    (void)argc; (void)args;
    value_t v;
    v.type = VAL_NUMBER;
    v.as.number = (double)time(NULL);
    v.is_on_heap = true;
    return v;
}