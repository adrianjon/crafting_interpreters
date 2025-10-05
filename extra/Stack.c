//
// Created by adrian on 2025-10-05.
//

#include "Stack.h"
#include <stdlib.h>
#include <string.h>

#define STACK_GROWTH_FACTOR 2

stack_t *stack_create(size_t initial_capacity) {
    stack_t *stack = malloc(sizeof(stack_t));
    if (!stack) return NULL;
    stack->data = malloc(initial_capacity * sizeof(void*));
    if (!stack->data) {
        free(stack);
        return NULL;
    }
    stack->capacity = initial_capacity;
    stack->size = 0;
    return stack;
}

void stack_destroy(stack_t *stack) {
    if (stack) {
        free(stack->data);
        free(stack);
    }
}

bool stack_push(stack_t *stack, void *value) {
    if (stack->size >= stack->capacity) {
        size_t new_capacity = stack->capacity * STACK_GROWTH_FACTOR;
        void **new_data = realloc(stack->data, new_capacity * sizeof(void*));
        if (!new_data) return false;
        stack->data = new_data;
        stack->capacity = new_capacity;
    }
    stack->data[stack->size++] = value;
    return true;
}

void *stack_pop(stack_t *stack) {
    if (stack->size == 0) return NULL;
    return stack->data[--stack->size];
}

void *stack_peek(stack_t *stack) {
    if (stack->size == 0) return NULL;
    return stack->data[stack->size - 1];
}

bool stack_is_empty(const stack_t *stack) {
    return stack->size == 0;
}

size_t stack_size(const stack_t *stack) {
    return stack->size;
}