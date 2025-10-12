//
// Created by adrian on 2025-10-12.
//

#ifndef LOX_MAP_H
#define LOX_MAP_H
#include <stdbool.h>
#include <stddef.h>

typedef size_t (*hash_fn_t)(const void * object);
typedef bool (*cmp_fn_t)(const void * object1, const void * object2);
typedef void * (*cpy_fn_t)(const void * object);

typedef struct {
    void (*reset)(void * self);
    bool (*next)(void * self);
    void * (*current)(const void * self);
    void (*destroy)(void * self);
} enumerable_vtable_t;

typedef struct map map_t;
typedef struct map_entry map_entry_t;
typedef struct map_enumerator map_enumerator_t;

typedef void (*map_clean_fn_t)(const void * key, const void * value);
typedef void (*free_fn_t)(const void ** value);

typedef struct {
    const hash_fn_t hash;
    const cmp_fn_t cmp;
    const cpy_fn_t kcopy;
    const cpy_fn_t vcopy;
    const free_fn_t kfree;
    const free_fn_t vfree;
} map_config_t;

map_t *map_create(size_t num_buckets, map_config_t config);
map_config_t map_default_config(void);
void map_destroy(map_t *map);

bool map_put(map_t * map, const void * key, void * value);
void * map_get(const map_t * map, const void * key);
bool map_remove(map_t * map, const void * key);
size_t map_size(const map_t * map);
bool map_contains(const map_t * map, const void * key);

const void * map_entry_key(const map_entry_t * entry);
void * map_entry_value(const map_entry_t * entry);

map_enumerator_t * map_get_enumerator(map_t * map);
bool map_enumerator_next(map_enumerator_t * it);
void * map_enumerator_current(map_enumerator_t * it);
void map_enumerator_reset(map_enumerator_t * it);
void map_enumerator_destroy(map_enumerator_t * it);

void map_entry_free(void** ptr);

#endif //LOX_MAP_H