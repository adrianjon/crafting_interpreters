//
// Created by adrian on 2025-10-13.
//

#include <stdio.h>
#include "value.h"
#include "expr.h"
#include "map/map2.h"
#include <string.h>
#include <assert.h>

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
    //expr_t * expr_a = *(expr_t **) a;

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
void expr_free(void * ptr) {
    free(ptr);
}
void * int_copy(void const * ptr) {
    return (void*)ptr;
}
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void int_free(void * ptr) { (void)ptr; }
static map_config_t const EXPRPTR_INT_CONFIG = {
    sizeof(expr_t),
    expr_hash,
    expr_equal,
    expr_copy,
    expr_free,
    sizeof(int),
    int_copy,
    int_free
};

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
void str_free(void * ptr) {
    free(ptr);
}
void * bool_copy(void const * ptr) {
    bool * copy = malloc(sizeof(bool));
    if (!copy) return NULL;
    memcpy(copy, ptr, sizeof(bool));
    return copy;
}
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void bool_free(void * ptr) { free(ptr); }
static map_config_t const CHARPTR_BOOL_CONFIG = {
    sizeof(char*),
    str_hash,
    str_equal,
    str_copy,
    str_free,
    sizeof(bool),
    bool_copy,
    bool_free
};

// <char*,value_t*>
void * value_copy2(void const * ptr) {
    value_t * copy = malloc(sizeof(value_t));
    memcpy(copy, ptr, sizeof(value_t));
    return copy;
}
void value_free2(void * ptr) {
    free(ptr);
}

static map_config_t const CHARPTR_VALUEPTR_CONFIG = {
    sizeof(char*),
    str_hash,
    str_equal,
    str_copy,
    str_free,
    sizeof(value_t),
    value_copy2,
    value_free2
};
#define CHECK_BOOL(val, expected) assert((val) == (expected))
void run_map_tests(void) {



    printf("=== Test 1: <char*, bool*> ===\n");
    hashmap_t * m1 = map_create(3, &CHARPTR_BOOL_CONFIG);
    char const * key1 = "key1";
    char const * key2 = "key2";
    bool constexpr val1 = true;
    bool constexpr val2 = false;

    assert(map_put(m1, key1, &val1));
    assert(map_put(m1, key2, &val2));

    bool * res1;
    assert(map_get(m1, key1, (void**)&res1));
    bool * res2;
    assert(map_get(m1, key2, (void**)&res2));
    bool * res3;
    assert(map_get(m1, "nonexistent", (void**)&res3) == false);

    CHECK_BOOL(*res1, true);
    CHECK_BOOL(*res2, false);
    printf("Passed <char*, bool*> tests.\n");

    printf("=== Test 2: <expr_t*, int> ===\n");
    hashmap_t * m2 = map_create(3, &EXPRPTR_INT_CONFIG);
    expr_t const key_a = {
        .type = EXPR_LITERAL,
        .as.literal_expr = {
            .kind = &(token_t){.type = STRING, .lexeme = "hello", .line = 1}
        }
    };
    int constexpr aa = 2;
    expr_t const key_b = {
        .type = EXPR_LITERAL,
        .as.literal_expr = {
            .kind = &(token_t){.type = NUMBER, .lexeme = "10", .line = 1}
        }
    };
    int constexpr bb = 5;

    assert(map_put(m2, &key_a, &aa));
    assert(map_put(m2, &key_b, &bb));

    int * int_res1;
    assert(map_get(m2, &key_a, (void**)&int_res1));
    int * int_res2;
    assert(map_get(m2, &key_b, (void**)&int_res2));
    assert(aa == *int_res1);
    assert(bb == *int_res2);
    printf("Passed <expr_t*, int> test.\n");
    // // Ensure proper put and get for pointers to data
    //
    //
    printf("=== Test 3: <char*, value_t*> ===\n");
    hashmap_t * m3 = map_create(3, &CHARPTR_VALUEPTR_CONFIG);
    char const * key_c = "var1";
    value_t val_c = {0};
    val_c.type = VAL_NUMBER;
    val_c.as.number = 2.5;
    char const * key_d = "var2";
    value_t val_d = {0};
    val_d.type = VAL_NUMBER;
    val_d.as.number = 3.5;

    assert(map_put(m3, key_c, &val_c));
    assert(map_put(m3, key_d, &val_d));

    value_t * val_res_c = nullptr;
    assert(map_get(m3, key_c, (void**)&val_res_c));
    value_t * val_res_d = nullptr;
    assert(map_get(m3, key_d, (void**)&val_res_d));

    assert(memcmp((void*)&val_c, (void*)val_res_c, sizeof(value_t)) == 0);
    assert(memcmp((void*)&val_d, (void*)val_res_d, sizeof(value_t)) == 0);

    printf("Passed <char*, value_t*> test.\n");
    // Ensure proper put and get for pointers to data

    map_destroy(m3);
    map_destroy(m2);
    map_destroy(m1);




    // This program will need these maps
    // <char*,bool>
    // <char*,value_t>
    // <expr_t*,int>
}