//
// Created by adrian on 2025-10-13.
//

#include "map2.h"

#include <stddef.h>
#include <stdlib.h>

inline void * default_copy(void const * src);
inline void default_free(void * ptr);
inline size_t default_hash(void const *key);
inline bool default_equals(void const *a, void const *b);

static const map_config_t DEFAULT_MAP_CONFIG = {
    // key handlers
    sizeof(intptr_t), // might make this void*
    default_hash,
    default_equals,
    default_copy,
    default_free,

    // value handlers
    sizeof(intptr_t), // might make this void*
    default_copy,
    default_free
};

hashmap_t * map_create(size_t const num_buckets, map_config_t const * map_config) {
    if (!map_config) {
        return map_create(num_buckets, &DEFAULT_MAP_CONFIG);
    }
    hashmap_t * map = malloc(sizeof(hashmap_t));
    map->num_buckets = num_buckets;
    map->buckets = calloc(num_buckets, sizeof(map_entry_t*));
    if (!map->buckets) { free(map); exit(-1); }
    map->size = 0;

    map->key_size = map_config->key_size;
    map->key_hash = map_config->key_hash;
    map->key_equals = map_config->key_equals;
    map->key_copy = map_config->key_copy;
    map->key_free = map_config->key_free;

    map->value_size = map_config->value_size;
    map->value_copy = map_config->value_copy;
    map->value_free = map_config->value_free;

    return map;
}

void map_destroy(hashmap_t * map) {}

int map_put(hashmap_t * map, void const * key, void const * value) {}

int map_get(hashmap_t * map, void const * key, void * out_value) {}

int map_remove(hashmap_t * map,  void const * key) {}

int map_contains(hashmap_t * map, void const * key) {}

size_t map_size(hashmap_t * map) {}


// Default copy/free functions for simple types
inline void * default_copy(void const * src) {
    return (void*)src;
}

inline void default_free(void * ptr) {
    // nothing to free for plain data
}
inline size_t default_hash(void const * key) {
    return 0;
}
inline bool default_equals(void const * a, void const * b) {
    return a == b;
}