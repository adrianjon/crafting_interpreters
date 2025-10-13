//
// Created by adrian on 2025-10-13.
//

#include "map2.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void * default_copy(void const * src);
static void default_free(void * ptr);
static size_t default_hash(void const * key);
static bool default_equals(void const * a, void const * b);

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

static void free_map_entry(map_entry_t * entry, free_fn_t kfree, free_fn_t vfree);

struct map_entry {
    void * key;
    void * value;
    struct map_entry * next;
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

    map->key_size   =  map_config -> key_size;
    map->key_hash   =  map_config ->   key_hash ? map_config -> key_hash   : default_hash;
    map->key_equals =  map_config -> key_equals ? map_config -> key_equals : default_equals;
    map->key_copy   =  map_config ->   key_copy ? map_config -> key_copy   : default_copy;
    map->key_free   =  map_config ->   key_free ? map_config -> key_free   : default_free;

    map->value_size =  map_config -> value_size;
    map->value_copy =  map_config -> value_copy ? map_config -> value_copy : default_copy;
    map->value_free =  map_config -> value_free ? map_config -> value_free : default_free;

    return map;
}

void map_destroy(hashmap_t * map) {
    if (!map || !map->buckets) return;
    for (size_t i = 0; i < map->num_buckets; i++) {
        map_entry_t * entry = map->buckets[i];
        while (entry) {
            map_entry_t * next = entry->next;
            free_map_entry(entry, map->key_free, map->value_free);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}


/**
 *
 * @param map pointer to an existing hashmap
 * @param key key of custom type, make sure of matching function pointers
 * @param value value of custom type, make sure of matching function pointers
 * @return returns false if any error, true if success
 */
bool map_put(hashmap_t * map, void const * key, void const * value) {
    if (!map || !map->buckets || !key) return false;
    size_t const index = map->key_hash(key) % map->num_buckets;
    map_entry_t * entry = map->buckets[index];
    while (entry) {
        if (map->key_equals(key, entry->key)) {
            map->value_free(entry->value);
            entry->value = map->value_copy(value);
            return true;
        }
        entry = entry->next;
    }
    // Not found: add new entry
    entry = malloc(sizeof(map_entry_t));
    if (!entry) return false;
    entry->key = map->key_copy(key);
    entry->value = map->value_copy(value);
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;
    return true;
}

bool map_get(hashmap_t * map, void const * key, void ** out_value) {
    if (!map || !map->buckets || !key) return false;
    size_t const index = map->key_hash(key) % map->num_buckets;
    map_entry_t const * entry = map->buckets[index];
    while (entry) {
        if (map->key_equals(key, entry->key)) {
            *out_value = entry->value;
            return true;
        }
        entry = entry->next;
    }
    return false;
}

bool map_remove(hashmap_t * map,  void const * key) {
    fprintf(stderr, "map_remove is not implemented");
    exit(EXIT_FAILURE);
}

bool map_contains(hashmap_t * map, void const * key) {
    fprintf(stderr, "map_contains is not implemented");
    exit(EXIT_FAILURE);
}

size_t map_size(hashmap_t * map) {
    if (!map) return 0;
    return map->size;
}


// Default copy/free functions for simple types
static inline void * default_copy(void const * src) {
    return (void*)src;
}

static inline void default_free(void * ptr) {
    // nothing to free for plain data
}
static inline size_t default_hash(void const * key) {
    return 34789324832l * (intptr_t)key;
}
static inline bool default_equals(void const * a, void const * b) {
    return a == b;
}

static void free_map_entry(map_entry_t * entry,
        free_fn_t const kfree, free_fn_t const vfree) {
    if (!entry || !kfree || !vfree) return;
    kfree(entry->key);
    kfree(entry->value);
    free(entry);
}