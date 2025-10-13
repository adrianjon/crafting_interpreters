//
// Created by adrian on 2025-10-12.
//

#ifndef LOX_OBJECT_H
#define LOX_OBJECT_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
    OBJ_NATIVE
} object_type_t;

typedef struct object {
    object_type_t type;
    int refcount;
} object_t;

typedef struct {
    object_t header;
    size_t length;
    char * chars;
    uint32_t hash;
} obj_string_t;

static inline void free_object(object_t * o) {
    if (!o) return;
    switch (o->type) {
        case OBJ_STRING:
            obj_string_t * s = (obj_string_t*)o;
            if (s->chars) free(s->chars);
            free(s);
            break;
        default:
            free(o);
            break;
    }
}

static inline void obj_inc_ref(object_t * o) {
    if (!o) return;
    o->refcount++;
}

static inline void obj_dec_ref(object_t * o) {
    if (!o) return;
    o->refcount--;
    if (o->refcount <= 0) {
        free_object(o);
    }
}

static inline uint32_t obj_string_hash(char const * key, size_t const len) {
    if (!key || !len) return 0;
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; ++i) {
        h ^= (unsigned char)key[i];
        h *= 16777619u;
    }
    return h;
}

static inline obj_string_t *obj_string_new(char const * chars) { /* convenience: strdup */
    obj_string_t * p_str = malloc(sizeof(obj_string_t));
    if (!p_str) return NULL;
    p_str->header.type = OBJ_STRING;
    p_str->header.refcount = 0;
    p_str->length = strlen(chars);
    p_str->chars = malloc(p_str->length + 1);
    if (!p_str->chars) return NULL;
    memcpy(p_str->chars, chars, p_str->length + 1);
    p_str->hash = obj_string_hash(p_str->chars, p_str->length);
    return p_str;
}
#endif //LOX_OBJECT_H