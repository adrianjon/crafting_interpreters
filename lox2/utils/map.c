//
// Created by adrian on 2025-10-12.
//

#include "map.h"
#include <string.h>
#include <stdlib.h>

struct map_entry {
    const void * key;
    void * value;
    struct map_entry * next;
};
struct map {
    struct map_entry **buckets;
    size_t num_buckets;
    size_t size;
    hash_fn_t hash;
    cmp_fn_t cmp;
    free_fn_t kfree;
    free_fn_t vfree;
    cpy_fn_t kcopy;
    cpy_fn_t vcopy;
};
struct map_enumerator {
    enumerable_vtable_t vtable;
    struct map * map;
    size_t bucket_index;
    struct map_entry * entry;
};

static void free_map_entry(map_entry_t * p_entry, free_fn_t kfree, free_fn_t vfree);

static void free_wrapper(void const ** ptr);
static size_t hash_string(const void * key) {
    const unsigned char * str = key;
    size_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = (hash << 5) + hash + c; // hash * 33 + c
    return hash;
}
static bool cmp_string(const void * key1, const void * key2) {
    return strcmp(key1, key2) == 0;
}
static void * copy_string(const void * key) {
    if (!key) return NULL;
    char * str = malloc(strlen(key) + 1);
    if (!str) exit(EXIT_FAILURE);
    memcpy(str, key, strlen(key));
    str[strlen(key)] = '\0';
    return str;
}

map_config_t map_default_config(void) {
    return (map_config_t){ .hash = hash_string, .cmp = cmp_string,
                            .kcopy = copy_string, .vcopy = copy_string,
                            .kfree = free_wrapper, .vfree = free_wrapper
    };
}
//123456789012345678901234567890
static void free_wrapper(void const ** ptr) {

    free((void*)*ptr);
    *ptr = NULL;
}

// API
map_t *map_create(const size_t num_buckets, const map_config_t config) {
    //map_t *map = malloc(sizeof(map_t));
    map_t *map = malloc(sizeof(map_t));
    if (!map) exit(EXIT_FAILURE);
    //map->buckets = calloc(num_buckets, sizeof(struct map_entry*));
    map->buckets = malloc(num_buckets * sizeof(struct map_entry*));
    if (!map->buckets) exit(EXIT_FAILURE);
    memset(map->buckets, 0, num_buckets * sizeof(struct map_entry*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    map->num_buckets = num_buckets;
    map->size = 0;
    map->hash = config.hash ? config.hash : hash_string;
    map->cmp = config.cmp ? config.cmp : cmp_string;
    map->kfree = config.kfree ? config.kfree : free_wrapper;
    map->vfree = config.vfree ? config.vfree : free_wrapper;
    map->kcopy = config.kcopy ? config.kcopy : copy_string;
    map->vcopy = config.vcopy ? config.vcopy : copy_string;

    return map;
}

void map_destroy(map_t * map) {
    if (!map) return;
    for (size_t i = 0; i < map->num_buckets; i++) {
        struct map_entry * entry = map->buckets[i];
        while (entry) {
            struct map_entry *next = entry->next;
            free_map_entry(entry, map->kfree, map->vfree);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}

bool map_put(map_t * map, const void * key, void * value) {
    if (!map) return false;
    const size_t index = map->hash(key) % map->num_buckets;
    struct map_entry * entry = map->buckets[index];
    while (entry) {
        if (map->cmp(entry->key, key)) {
            map->vfree((void const **)&entry->value); // free old value
            entry->value = map->vcopy(value);
            return true;
        }
        entry = entry->next;
    }
    // Not found: add new entry
    //entry = malloc(sizeof(struct map_entry));
    entry = malloc(sizeof(struct map_entry));
    if (!entry) return false;
    entry->key = map->kcopy(key);
    entry->value = map->vcopy(value);
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;
    return true;
}

void * map_get(const map_t * map, const void * key) {
    const size_t index = map->hash(key) % map->num_buckets;
    const struct map_entry * entry = map->buckets[index];
    while (entry) {
        if (map->cmp(entry->key, key))
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}

bool map_remove(map_t * map, const void * key) {
    const size_t index = map->hash(key) % map->num_buckets;
    struct map_entry * entry = map->buckets[index];
    struct map_entry * prev = NULL;
    while (entry) {
        if (map->cmp(entry->key, key)) {
            if (prev)
                prev->next = entry->next;
            else
                map->buckets[index] = entry->next;
            // map->clean(entry->key, entry->value);
            free_map_entry(entry, NULL, NULL);
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
bool map_contains(const map_t * map, const void * key) {
    const size_t index = map->hash(key) % map->num_buckets;
    const struct map_entry *entry = map->buckets[index];
    while (entry) {
        if (map->cmp(entry->key, key)) {
            return true;
        }
        entry = entry->next;
    }
    return false;
}


const void * map_entry_key(const map_entry_t * entry) {
    return entry->key;
}
void * map_entry_value(const map_entry_t * entry) {
    return entry->value;
}


// Enumerable interface implementations
static void map_enum_reset(void * self) {
    map_enumerator_t * e = self;
    e->bucket_index = 0;
    e->entry = NULL;
}
static bool map_enum_next(void * self) {
    map_enumerator_t * e = self;
    map_t * map = e->map;
    if (!map) return false;

    if (e->entry && e->entry->next) {
        e->entry = e->entry->next;
        return true;
    }
    while (e->bucket_index < map->num_buckets) {
        struct map_entry * entry = map->buckets[e->bucket_index++];
        if (entry) {
            e->entry = entry;
            return true;
        }
    }
    e->entry = NULL;
    return false;
}
static void * map_enum_current(const void * self) {
    const map_enumerator_t * e = self;
    return e->entry;
}
static void map_enum_destroy(void * self) {
    (void)self; // not implemented due to not using malloc to allocate
}
map_enumerator_t * map_get_enumerator(map_t * map) {
    map_enumerator_t * e = malloc(sizeof(map_enumerator_t));
    if (!e) return NULL;
    e->vtable = (enumerable_vtable_t){
        .reset = map_enum_reset,
        .next = map_enum_next,
        .current = map_enum_current,
        .destroy = map_enum_destroy,
    };
    e->map = map;
    e->bucket_index = 0;
    e->entry = NULL;
    e->vtable.reset(e);
    return e;
}

bool map_enumerator_next(map_enumerator_t * it) {
    return it->vtable.next(it);
}
void *map_enumerator_current(map_enumerator_t * it) {
    return it->vtable.current(it);
}
void map_enumerator_reset(map_enumerator_t * it) {
    it->vtable.reset(it);
}
void map_enumerator_destroy(map_enumerator_t * it) {
    it->vtable.destroy(it);
}

static void free_map_entry(map_entry_t * p_entry, free_fn_t const kfree, free_fn_t const vfree) {
    if (p_entry && kfree && vfree) {
        kfree((void const **)&p_entry->key);
        vfree((void const **)&p_entry->value);
    }
}

void map_entry_free(void** ptr) {

}