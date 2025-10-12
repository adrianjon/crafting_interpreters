//
// Created by adrian on 2025-10-12.
//

#ifndef LOX_STACK_H
#define LOX_STACK_H
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    void **data;
    size_t capacity;
    size_t size;
} stack_t;

stack_t *stack_create(size_t initial_capacity);
void stack_destroy(stack_t *stack);
bool stack_push(stack_t *stack, void *value);
void *stack_pop(stack_t *stack);
void *stack_peek(stack_t *stack);
bool stack_is_empty(const stack_t *stack);
size_t stack_size(const stack_t *stack);

#endif //LOX_STACK_H