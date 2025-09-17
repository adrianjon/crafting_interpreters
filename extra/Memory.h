//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_MEMORY_H
#define LOX_MEMORY_H

#include <stdbool.h>

// Define memory allocation functions API

// Allocate a block of memory of given size
void* memory_allocate(size_t size);

// Reallocate a previously allocated block of memory
void* memory_reallocate(void* ptr, size_t new_size);

// Returns a pointer to the first occurrence of the character c in the memory block
const void* memory_character(const void* ptr, char c, size_t n);

// Copy a block of memory from one location to another
bool memory_copy(void* dest, const void* src, size_t n);

// Compare two blocks of memory, return true if they are equal
bool memory_compare(const void* ptr1, const void* ptr2, size_t n);


// only when dealing with c-strings we can assume null-termination. This is bad.
void memory_replace(char* ptr, char old_char, char new_char);

// Free a previously allocated block of memory
void memory_free(void** ptr);


void memory_free_all(void);

// debug functions TODO: remove api access
void debug_memory_pool();

#endif //LOX_MEMORY_H