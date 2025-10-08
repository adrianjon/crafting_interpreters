//
// Created by adrian on 2025-10-07.
//

#ifndef LOX_ENUMERABLE_H
#define LOX_ENUMERABLE_H

#include <stdbool.h>

typedef struct {
    void (*reset)(void * self);
    bool (*next)(void * self);
    void * (*current)(const void * self);
    void (*destroy)(void * self);
} enumerable_vtable_t;

#endif //LOX_ENUMERABLE_H