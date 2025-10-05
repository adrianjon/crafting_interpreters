//
// Created by adrian on 2025-10-05.
//

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

typedef struct {
    map_entry_t **buckets;
    size_t num_buckets;
    size_t size;
    map_hash_fn_t hash;
    map_cmp_fn_t cmp;
} map_t;

//map_t *map_create(size_t num_buckets);
map_t *map_create(size_t num_buckets, map_hash_fn_t hash, map_cmp_fn_t cmp);

void map_destroy(map_t *map);

bool map_put(map_t * map, const void * key, void * value);
void * map_get(map_t * map, const void * key);
bool map_remove(map_t * map, const void * key);
size_t map_size(const map_t * map);
bool map_contains(const map_t * map, const void * key);

#endif //LOX_MAP_H