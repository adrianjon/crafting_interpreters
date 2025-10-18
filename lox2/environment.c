//
// Created by adrian on 2025-10-12.
//

#include "environment.h"
#include "../tests/map/map2.h"
#include <stdlib.h>

#undef NULL
#define NULL nullptr

/* Map semantics assumed:
 * - map_create(num_buckets, config) returns a map_t*
 * - map_put(map, key, value) stores key->value (key is const void* here)
 * - map_get(map, key) returns value or NULL
 * - map_contains(map, key) returns bool
 * - map_destroy(map) destroys the map (and frees keys/values according to config)
 *
 * For environment keys we use const char* (token->lexeme). Ensure those strings
 * have stable lifetime (interned or strdup'd). If map_destroy frees keys and you
 * didn't strdup, adapt config to not free keys.
 */
static void * copy_value(void const * val) {
    value_t * copy = malloc(sizeof(value_t));
    memcpy(copy, val, sizeof(value_t));
    return copy;
}
static void free_value(void * val) {
    free(val);
}
static void * str_copy(void const * ptr) {
    size_t const len = strlen(ptr);
    char * copy = malloc(len + 1);
    memcpy(copy, ptr, len);
    copy[len] = '\0';
    return copy;
}
static bool str_equal(void const * a, void const * b) {
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}
static size_t str_hash(void const * key) {
    if (!key) return 0;
    unsigned char const * s = key;
    size_t hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}
environment_t * environment_create(environment_t * enclosing) {
    //<char*,value_t*> map
    map_config_t const cfg = {
        .key_copy = str_copy,
        .key_equals = str_equal,
        .key_hash = str_hash,
        .key_free = free,
        .key_size = sizeof(char*),
        .value_size = sizeof(value_t),
        .value_copy = copy_value,
        .value_free = free_value
    };
    /* choose initial bucket count conservatively */
    map_t * m = map_create(16, &cfg);
    if (!m) return NULL;
    environment_t * env = malloc(sizeof(environment_t));
    if (!env) {
        map_destroy(m);
        return NULL;
    }
    env->values = m;
    env->enclosing = enclosing;
    return env;
}

void environment_destroy(environment_t * env) {
    if (!env) return;
    if (env->values) map_destroy(env->values);
    free(env);
}

void environment_define(environment_t * env, char const * name, value_t * value) {
    if (!env) return;
    /* put overwrites any previous value in this environment */
    map_put(env->values, name, value);
}

value_t * environment_get(environment_t const * env, char const * name) {
    environment_t const * curr = env;
    while (curr) {
        if (map_contains(curr->values, name)) {
            value_t * ret;
            map_get(curr->values, name, (void**)&ret);
            return ret;
        }
        curr = curr->enclosing;
    }
    return NULL;
}

value_t * environment_get_at(environment_t * env, int const distance, char const * name) {
    environment_t * curr = env;
    for (int i = 0; i < distance; ++i) {
        if (!curr) return NULL;
        curr = curr->enclosing;
    }
    if (!curr) return NULL;
    if (map_contains(curr->values, name)) {
        value_t * ret;
        map_get(curr->values, name, (void**)&ret);
        return ret;
    }
    return NULL;
}

bool environment_assign(environment_t const * env, char const * name, value_t * value) {
    environment_t const * curr = env;
    while (curr) {
        if (map_contains(curr->values, name)) {
            map_put(curr->values, name, value);
            return true;
        }
        curr = curr->enclosing;
    }
    return false;
}

bool environment_assign_at(environment_t * env, int const distance,
    char const * name, value_t * value) {
    environment_t * curr = env;
    for (int i = 0; i < distance; ++i) {
        if (!curr) return false;
        curr = curr->enclosing;
    }
    if (!curr) return false;
    if (map_contains(curr->values, name)) {
        map_put(curr->values, name, value);
        return true;
    }
    /* If the binding isn't present at that depth, you can still choose to
     * set it (create new) or treat as error. Here we create/overwrite.
     */
    map_put(curr->values, name, value);
    return true;
}