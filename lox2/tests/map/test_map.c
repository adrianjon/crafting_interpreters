//
// Created by adrian on 2025-10-13.
//

#include <stdio.h>
#include "value.h"
#include "expr.h"
#include <stdbool.h>
#include <string.h>
#include "map2.h"

// typedef struct map map_t;
//
// #define DEFINE_MAP(NAME, KEY_T, VAL_T)                          \
//     struct NAME {                                               \
//         int x;                                                  \
//         struct { KEY_T key; VAL_T val; void * next; } * buckets;\
//     };                                                          \
//     static inline map_t * new_##NAME(size_t n)                  \
//     {                                                           \
//         map_t * m = malloc(sizeof(map_t));                      \
//         m->implemented =                                        \
//             (struct NAME*)malloc(sizeof(struct NAME));          \
//         return m;                                               \
//     }
// // ReSharper disable once CppInconsistentNaming
// #define new_map(type, size) \
//     new_##type(size)
//
// struct map {
//     void * implemented;
// };
//
// DEFINE_MAP(map_str_bool, char*, bool);
// DEFINE_MAP(map_str_value, char*, value_t);
// DEFINE_MAP(map_expr_p_int, expr_t*,int);

// <expr_t*,int>
size_t expr_hash(void const * ptr) {
    size_t constexpr len = sizeof(expr_t);
    const unsigned char * bytes = ptr;
    size_t hash = 1469598103934665603ULL;  // FNV offset basis (64-bit)
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;          // FNV prime
    }
    return hash;
}
bool expr_equal(void const * a, void const * b) {
    if (!a || !b) return false;
    // if (expr_hash(a) != expr_hash(b)) return false;
    // // compare hash might give false positives so we only check if no match
    // expr_t const * e_a = a;
    // expr_t const * e_b = b;
    // if (e_a->type != e_b->type) return false;
    if (memcmp(a, b, sizeof(expr_t)) != 0) return false;
    return true;
}
void * expr_copy(void const * ptr) {
    expr_t * copy = malloc(sizeof(expr_t));
    memcpy(copy, ptr, sizeof(expr_t));
    return copy;
}
void expr_free(void const * ptr) {
    free((void*)ptr);
}
void * int_copy(void const * ptr) {
    return (void*)ptr;
}
void int_free(void const * ptr) { (void)ptr; }

// <char*,bool>
size_t str_hash(void const * key) {
    if (!key) return 0;
    unsigned char const * s = key;
    size_t hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}
bool str_equal(void const * a, void const * b) {
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}
void * str_copy(void const * ptr) {
    size_t const len = strlen(ptr);
    char * copy = malloc(len + 1);
    memcpy(copy, ptr, len);
    copy[len] = '\0';
    return copy;
}
void str_free(void const * ptr) {
    free((void*)ptr);
}
void * bool_copy(void const * ptr) {
    return (void*)ptr;
}
void bool_free(void const * ptr) { (void)ptr; }

// <char*,value_t*>
// void * value_copy(void const * ptr) {}
// void value_free(void const * ptr) {}

static hashmap_t * test_string_to_bool_map() {
    // printf("Running test_string_to_bool_map...\n");
    //
    // void * copy(void const * ptr) {
    //
    // }
    // void free(void * ptr) {
    //
    // }
    // size_t hash(void const * key) {
    //     printf("hello\n");
    //     if (!key) return 0;
    //     unsigned char const * s = key;
    //     size_t hash = 5381;
    //     int c;
    //     while ((c = *s++))
    //         hash = ((hash << 5) + hash) + c;
    //     return hash;
    // }
    // bool str_equals(void const * a, void const * b) {
    //     if (!a || !b) return false;
    //     return strcmp(a,b) == 0;
    // }
    // map_config_t const cfg = {
    //     sizeof(char*), hash, str_equals, copy, free,
    //     sizeof(bool)
    // };
    // hashmap_t * map = map_create(1, &cfg);
    // map->key_free(nullptr);
    // map->value_free(nullptr);
    // map->key_copy(nullptr);
    // map->value_copy(nullptr);
    // map->key_equals(nullptr, nullptr);
    // map->key_hash(nullptr);
    // return map;
}
void run_map_tests(void) {

    hashmap_t * m1 = map_create(2,nullptr);

    hashmap_t * m = test_string_to_bool_map();
    map_put(m, "hello", "hello");
    m->key_hash(NULL);
    //map_t * map = new_map(map_str_bool, 10);
    // This program will need these maps
    // <char*,bool>
    // <char*,value_t>
    // <expr_t*,int>
}