//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_MEMORY_H
#define LOX_MEMORY_H

#include <stdbool.h>

typedef struct region region_t;

void * memory_allocate(size_t size);
void * memory_reallocate(void * p, size_t old_size, size_t new_size);
// const void * memory_character(const void * p, char c, size_t n);
bool memory_copy(void * p_dest, const void * p_src, size_t n);
bool memory_compare(const void * p_1, const void * p_2, size_t n);
// void memory_replace(char * p, char old_char, char new_char);
void memory_free(void ** pp);
// void memory_free_all(void);

region_t * new_region(void);
void * region_allocate(size_t size);
void region_free(region_t * p_region);

// debug functions TODO: remove api access
void debug_memory_pool();

#endif //LOX_MEMORY_H