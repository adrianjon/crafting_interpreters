//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_LIST_H
#define LOX_LIST_H

#include <stdlib.h>

typedef struct {
    void ** data;
    size_t count;
    size_t capacity;
    void (*free_fn)(void ** element);
} list_t;

inline void list_add(list_t * p_list, void * p_data) {
    if (p_list->data == NULL) {
        p_list->data = malloc(sizeof(void *));
        p_list->capacity = 1;
    }
    if (p_list->count == p_list->capacity) {
        p_list->capacity *= 2;
        p_list->data = realloc(p_list->data, sizeof(void *) * p_list->capacity);
    }
    p_list->data[p_list->count] = p_data;
    p_list->count++;
}

inline void list_free(list_t * p_list) {
    for (size_t i = 0; i < p_list->count; i++) {
        p_list->free_fn(&p_list->data[i]);
    }
    free(p_list->data);
    p_list->data = NULL;
    p_list->count = 0;
    p_list->capacity = 0;
}
#endif //LOX_LIST_H