//
// Created by adrian on 2025-10-05.
//

#include "Map.h"
#include <stdlib.h>
#include <string.h>
static size_t hash_string(const void * key, const size_t num_buckets) {
    const unsigned char * str = key;
    size_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % num_buckets;
}
static bool cmp_string(const void * key1, const void * key2) {
    return strcmp(key1, key2) == 0;
}
static void clean_string(const void * key, const void * value) {
    (void)value;
    free((void *)key);
}
map_t *map_create(const size_t num_buckets, const map_hash_fn_t hash,
                    const map_cmp_fn_t cmp, const map_clean_fn_t clean) {
    map_t *map = malloc(sizeof(map_t));
    if (!map) return NULL;
    map->buckets = calloc(num_buckets, sizeof(map_entry_t*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    map->num_buckets = num_buckets;
    map->size = 0;
    map->hash = hash ? hash : hash_string;
    map->cmp = cmp ? cmp : cmp_string;
    map->clean = clean ? clean : clean_string;
    return map;
}

void map_destroy(map_t * map) {
    if (!map) return;
    for (size_t i = 0; i < map->num_buckets; i++) {
        map_entry_t *entry = map->buckets[i];
        while (entry) {
            map_entry_t *next = entry->next;
            map->clean(entry->key, entry->value);
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}

bool map_put(map_t * map, const void * key, void * value) {
    const size_t index = map->hash(key, map->num_buckets);
    map_entry_t * entry = map->buckets[index];
    while (entry) {
        if (map->cmp(entry->key, key)) {
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }
    // Not found: add new entry
    entry = malloc(sizeof(map_entry_t));
    if (!entry) return false;
    entry->key = key;
    entry->value = value;
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;
    return true;
}

void * map_get(const map_t * map, const void * key) {
    const size_t index = map->hash(key, map->num_buckets);
    const map_entry_t * entry = map->buckets[index];
    while (entry) {
        if (map->cmp(entry->key, key))
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}

bool map_remove(map_t * map, const void * key) {
    const size_t index = map->hash(key, map->num_buckets);
    map_entry_t * entry = map->buckets[index];
    map_entry_t * prev = NULL;
    while (entry) {
        if (map->cmp(entry->key, key)) {
            if (prev)
                prev->next = entry->next;
            else
                map->buckets[index] = entry->next;
            map->clean(entry->key, entry->value);
            free(entry);
            map->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    return false;
}

size_t map_size(const map_t * map) {
    return map->size;
}