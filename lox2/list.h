//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_LIST_H
#define LOX_LIST_H

#include <stdlib.h>
#include <stdio.h>

/*
 * list_t:
 *   A generic list that stores pointers to custom data.
 *   User provides a free_fn suitable to their data.
 */
typedef struct {
    void ** data;
    size_t count;
    size_t capacity;
    void (*free_fn)(void ** element);
} list_t;

static inline void fallback_free_fn(void ** element) {
    free(*element);
    *element = NULL;
}

/*
 * list_add:
 *   Append p_data to p_list, growing the internal array as needed.
 *
 * Precondition: p_list must either be zero-initialized (e.g. list_t L = {0};)
 * or you must manually set its fields to zero/NULL before first use.
 */
static inline void list_add(list_t * p_list, void * p_data) {
    // Install fallback function if none is provided.
    if (p_list->free_fn == NULL) {
        p_list->free_fn = fallback_free_fn;
    }

    // First allocation?
    if (p_list->data == NULL) {
        p_list->data = malloc(sizeof(void *));
        if (p_list->data == NULL) { fprintf(stderr, "Malloc error"); exit(1); }
        p_list->capacity = 1;
    }

    // Grow list if full.
    if (p_list->count == p_list->capacity) {
        p_list->capacity *= 2;
        p_list->data = realloc(p_list->data, sizeof(void *) * p_list->capacity);
        if (p_list->data == NULL) { fprintf(stderr, "Malloc error"); exit(1); }
    }
    p_list->data[p_list->count] = p_data;
    p_list->count++;
}

static inline void list_free(list_t * p_list) {
    if (!p_list || !p_list->data) return;
    for (size_t i = 0; i < p_list->count; i++) {
        if (p_list->data[i] && p_list->free_fn)
            p_list->free_fn(&p_list->data[i]);
    }
    free(p_list->data);
    p_list->data = NULL;
    p_list->count = 0;
    p_list->capacity = 0;
}
#endif //LOX_LIST_H