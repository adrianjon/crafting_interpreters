//
// Created by adrian on 2025-10-05.
//
/**
 * Generic hash-map implementation.
 *
 * Keys and values are stored as void pointers. For string keys, provide
 * appropriate hash and compare functions (e.g., hash_string, cmp_string),
 *
 * For pointer keys (e.g., structs, AST nodes), use pointer hash/compare
 *
 * To store integer values, cast them to and from void* using (void*)(intptr_t)val
 * and (int)(intptr_t)ptr.
 *
 * Example usage:
 *   map_t *m = map_create(16, hash_string, cmp_string, clean_string);
 *   map_put(m, "foo", (void*)true);
 *   ...
 *   map_t *m2 = map_create(8, hash_ptr, cmp_ptr, clean_ptr);
 *   map_put(m2, (void*)expr_node, (void*)(intptr_t)depth);
 */

#ifndef LOX_MAP_H
#define LOX_MAP_H

#include <stddef.h>
#include <stdbool.h>


typedef struct map_entry {
    const void * key;
    void * value;
    struct map_entry * next;
} map_entry_t;

typedef size_t (*map_hash_fn_t)(const void * key, size_t num_buckets);
typedef bool (*map_cmp_fn_t)(const void * key1, const void * key2);
typedef void (*map_clean_fn_t)(const void * key, const void * value);

typedef struct {
    map_entry_t **buckets;
    size_t num_buckets;
    size_t size;
    map_hash_fn_t hash;
    map_cmp_fn_t cmp;
    map_clean_fn_t clean;
} map_t;

//map_t *map_create(size_t num_buckets);
map_t *map_create(size_t num_buckets, map_hash_fn_t hash, map_cmp_fn_t cmp, map_clean_fn_t clean);

void map_destroy(map_t *map);

bool map_put(map_t * map, const void * key, void * value);
void * map_get(const map_t * map, const void * key);
bool map_remove(map_t * map, const void * key);
size_t map_size(const map_t * map);
bool map_contains(const map_t * map, const void * key);

#endif //LOX_MAP_H