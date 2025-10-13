//
// Created by adrian on 2025-10-13.
//

#include <stdio.h>
#include "value.h"
#include "expr.h"
#include <stdbool.h>

typedef struct map map_t;

#define DEFINE_MAP(NAME, KEY_T, VAL_T)                          \
    struct NAME {                                               \
        int x;                                                  \
        struct { KEY_T key; VAL_T val; void * next; } * buckets;\
    };                                                          \
    static inline map_t * new_##NAME(size_t n)                  \
    {                                                           \
        map_t * m = malloc(sizeof(map_t));                      \
        m->implemented =                                        \
            (struct NAME*)malloc(sizeof(struct NAME));          \
        return m;                                               \
    }
// ReSharper disable once CppInconsistentNaming
#define new_map(type, size) \
    new_##type(size)

struct map {
    void * implemented;
};

DEFINE_MAP(map_str_bool, char*, bool);
DEFINE_MAP(map_str_value, char*, value_t);
DEFINE_MAP(map_expr_p_int, expr_t*,int);

struct A {
    union {
        int x;
        char s;
    };
};
void test(int, int, bool) {
    printf("hello\n");
}
void run_map_tests(void) {
    struct A map1;
    int * m = malloc(100);
    auto constexpr x = 5;
    printf("hello%d\n", x);
    auto mapp = (map_t){.implemented = NULL};
    map_t mapoo = {};
    map_t * map = new_map(map_str_bool, 10);
    // ((struct map_str_bool*)map->implemented)->buckets->key
    void * xs = nullptr;
    // This program will need these maps
    // <char*,bool>
    // <char*,value_t>
    // <expr_t*,int>
}