//
// Created by adrian on 2025-10-05.
//

#include "Map.h"
#include <stdlib.h>
#include <string.h>

static size_t hash_string(const char *str, size_t num_buckets) {
    size_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % num_buckets;
}

map_t *map_create(size_t num_buckets) {
    map_t *map = malloc(sizeof(map_t));
    if (!map) return NULL;
    map->buckets = calloc(num_buckets, sizeof(map_entry_t*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    map->num_buckets = num_buckets;
    map->size = 0;
    return map;
}

void map_destroy(map_t *map) {
    if (!map) return;
    for (size_t i = 0; i < map->num_buckets; i++) {
        map_entry_t *entry = map->buckets[i];
        while (entry) {
            map_entry_t *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}

bool map_put(map_t *map, const char *key, void *value) {
    size_t index = hash_string(key, map->num_buckets);
    map_entry_t *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }
    // Not found: add new entry
    entry = malloc(sizeof(map_entry_t));
    if (!entry) return false;
    entry->key = strdup(key);
    entry->value = value;
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;
    return true;
}

void *map_get(map_t *map, const char *key) {
    size_t index = hash_string(key, map->num_buckets);
    map_entry_t *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0)
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}

bool map_remove(map_t *map, const char *key) {
    size_t index = hash_string(key, map->num_buckets);
    map_entry_t *entry = map->buckets[index];
    map_entry_t *prev = NULL;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev)
                prev->next = entry->next;
            else
                map->buckets[index] = entry->next;
            free(entry->key);
            free(entry);
            map->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    return false;
}

size_t map_size(const map_t *map) {
    return map->size;
}