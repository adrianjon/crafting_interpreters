//
// Created by adrian on 2025-09-15.
//

//#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Windows.h"

#define REGION_SIZE (1<<20) // 1MB
struct region {
    unsigned char buffer[REGION_SIZE];
    size_t capacity;
    size_t used;
};
//region_t p_region = {.buffer = {0}, REGION_SIZE, 0};
static region_t * g_p_current_region = NULL;
void memory_free(void ** pp)
{

}
size_t align_up(const size_t n, const size_t align) {
    return (n + align - 1) & ~(align - 1);
}
// Allocate a block of memory of given size
void* memory_allocate(const size_t size) {
    if (size == 0) return NULL;
    return region_allocate(size);
    // void* ptr = malloc(size);
    // if (!ptr) {
    //     fprintf(stderr, "Memory allocation failed\n");
    //     exit(EXIT_FAILURE);
    // }
    // return ptr;
}

// Reallocate a previously allocated block of memory
void* memory_reallocate(void * p, const size_t old_size, const size_t new_size) {
    if (!p) return memory_allocate(new_size);
    if (new_size == 0) {
        memory_free(&p);
        return NULL;
    }
    //void* new_ptr = realloc(p, new_size);
    void* new_ptr = memory_allocate(new_size);
    memory_copy(new_ptr, p, old_size);
    if (!new_ptr) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}
// searches n characters ahead for c and returns pointer to first occurence
const void * memory_character(const void * p, char c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (*(char*)p == c) return p;
        p = (char*)p + 1;
    }
    return NULL;
}
// Copy a block of memory from one location to another
bool memory_copy(void * p_dest, const void * p_src, size_t n)
{
    if (!p_dest || !p_src || n == 0) return false;
    char* d = p_dest;
    const char* s = p_src;

    while (n-- != 0) {

        *d++ = *s++;
    }
    return true;
}

// Compare two blocks of memory, return true if they are equal
bool memory_compare(const void * p_1, const void * p_2, size_t n)
{
    if (!p_1 || !p_2 || n == 0) return false;

    const char* p1 = p_1;
    const char* p2 = p_2;

    while (n--) {
        if (*p1++ != *p2++) return false;
    }
    return true;
}

region_t * new_region(void) {
    region_t * p_region = malloc(sizeof(region_t));
    memset(p_region, 0, sizeof(region_t));
    p_region->capacity = REGION_SIZE;
    p_region->used = 0;
    return p_region;
}
#define REGION_ALIGNMENT 8
void * region_allocate(const size_t size) {
    if (!g_p_current_region) {
        fprintf(stderr, "Region used uninitialized.\n");
        exit(EXIT_FAILURE);
    }
    if (g_p_current_region->used + size > g_p_current_region->capacity) {
        fprintf(stderr, "Memory region full.\n");
        exit(EXIT_FAILURE);
    }
    const size_t offset = align_up(g_p_current_region->used, REGION_ALIGNMENT);
    void * p = g_p_current_region->buffer + offset;
    g_p_current_region->used = offset + size;
    return p;
}
void region_free(region_t * p_region) {
    memset(p_region->buffer, 0, REGION_SIZE);
    free(p_region);
}
void activate_region(region_t * p_region) {
    g_p_current_region = p_region;
}
