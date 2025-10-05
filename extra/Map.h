//
// Created by adrian on 2025-10-05.
//

#ifndef LOX_MAP_H
#define LOX_MAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct map_entry {
    char *key;
    void *value;
    struct map_entry *next;
} map_entry_t;

typedef struct {
    map_entry_t **buckets;
    size_t num_buckets;
    size_t size;
} map_t;

map_t *map_create(size_t num_buckets);
void map_destroy(map_t *map);

bool map_put(map_t *map, const char *key, void *value);
void *map_get(map_t *map, const char *key);
bool map_remove(map_t *map, const char *key);
size_t map_size(const map_t *map);

#endif //LOX_MAP_H