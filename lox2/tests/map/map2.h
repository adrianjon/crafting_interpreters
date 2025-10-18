//
// Created by adrian on 2025-10-13.
//

#ifndef LOX_MAP2_H
#define LOX_MAP2_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#ifndef __STDC_NO_THREADS__
//#include <threads.h>
#endif



// Function pointer types
typedef size_t (*hash_fn_t)(void const * key);
typedef bool (*equals_fn_t)(void const * a, void const * b);
typedef void * (*copy_fn_t)(void const * ptr);
typedef void (*free_fn_t)(void * data);

// Map entry structure
typedef struct map_entry map_entry_t;

// Main hashmap structure
typedef struct hashmap hashmap_t;
typedef hashmap_t map_t;
struct hashmap {
    map_entry_t ** buckets;
    size_t num_buckets;
    size_t size;  // current number of elements

    // Key info
    size_t key_size;
    hash_fn_t key_hash;
    equals_fn_t key_equals;
    copy_fn_t key_copy;
    free_fn_t key_free;

    // Value info
    size_t value_size;
    copy_fn_t value_copy;
    free_fn_t value_free;

#ifdef __STDC_NO_THREADS__
    mtx_t mutex;  // thread safety
#endif
};

typedef struct {

    // key handlers
    size_t key_size;
    hash_fn_t key_hash;
    equals_fn_t key_equals;
    copy_fn_t key_copy;
    free_fn_t key_free;

    // value handlers
    size_t value_size;
    copy_fn_t value_copy;
    free_fn_t value_free;
} map_config_t;

// API Functions

// Create a new map
hashmap_t * map_create(size_t num_buckets, map_config_t const * map_config);

// Destroy the map and free all memory
void map_destroy(hashmap_t * map);

// Insert or update a key-value pair
bool map_put(hashmap_t * map, void const * key, void const * value);

// Retrieve a value for a given key (returns 0 if found, -1 if not)
bool map_get(hashmap_t * map, void const * key, void ** out_value);

// Remove a key-value pair (returns 0 if removed, -1 if not found)
bool map_remove(hashmap_t * map,  void const * key);

// Check if a key exists
bool map_contains(hashmap_t const * map, void const * key);

// Get the number of elements in the map
size_t map_size(hashmap_t * map);

#endif //LOX_MAP2_H