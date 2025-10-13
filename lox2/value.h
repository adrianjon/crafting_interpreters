//
// Created by adrian on 2025-10-12.
//

#ifndef LOX_VALUE_H
#define LOX_VALUE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "object.h"

typedef struct object object_t;

typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_OBJ
} value_type_t;

typedef struct value {
    value_type_t type;
    union {
        bool boolean;
        double number;
        object_t * object; // owned via obj_inc_ref/obj_dec_ref
    } as;
} value_t;

static inline value_t value_nil(void) {
    value_t v; v.type = VAL_NIL; v.as.number = 0.0; return v;
}
inline value_t value_bool(bool const b) {
    value_t v; v.type = VAL_BOOL; v.as.boolean = b; return v;
}
inline value_t value_number(double const n) {
    value_t v; v.type = VAL_NUMBER; v.as.number = n; return v;
}
static inline value_t value_object(object_t * o) {
    value_t v; v.type = VAL_OBJ; v.as.object = o;
    if (o) obj_inc_ref(o); return v;
}
static inline void value_free(value_t * v) {
    if (!v) return;
    if (v->type == VAL_OBJ && v->as.object) {
        obj_dec_ref(v->as.object);
        v->as.object = NULL;
    }
    v->type = VAL_NIL;
}
static inline bool value_is_truthy(value_t const * v) {
    switch (v->type) {
        case VAL_NIL:       return false;
        case VAL_BOOL:      return v->as.boolean;
        case VAL_NUMBER:    return v->as.number != 0.0;
        case VAL_OBJ:       return true;
    }
    return false;
}
static inline bool value_equals(value_t const * a, value_t const * b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case VAL_NIL:       return true;
        case VAL_BOOL:      return a->as.boolean == b->as.boolean;
        case VAL_NUMBER:    return a->as.number == b->as.number;
        case VAL_OBJ:       return a->as.object == b->as.object;
    }
    return false;
}
static inline void value_print(value_t const * v) {
    switch (v->type) {
        case VAL_NIL:       printf("nil"); break;
        case VAL_BOOL:      printf(v->as.boolean ? "true" : "false"); break;
        case VAL_NUMBER:    printf("%g", v->as.number); break;
        case VAL_OBJ:
            if (!v->as.object) { printf("nil"); break;}
            printf("<object:%d>", (int)v->as.object->type);
        default:
            break;
    }
}
#endif //LOX_VALUE_H