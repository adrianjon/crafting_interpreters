//
// Created by adrian on 2025-09-15.
//

//#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "Windows.h"


// Allocate a block of memory of given size
void* memory_allocate(const size_t size) {
    if (size == 0) return NULL;
    void* ptr = malloc(size);
    if (!ptr) {
        print("Memory allocation failed\n");
    }
    return ptr;
}

// Reallocate a previously allocated block of memory
void* memory_reallocate(void* ptr, const size_t new_size) {
    if (!ptr) return memory_allocate(new_size);
    if (new_size == 0) {
        memory_free(&ptr);
        return NULL;
    }
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr) {
        print("Memory reallocation failed\n");
    }
    return new_ptr;
}

// Returns a pointer to the first occurrence of the character c in the memory block
const void* memory_character(const void* ptr, const char c, size_t n)
{
    if (!ptr || n == 0) return NULL;

    const char* p = ptr;
    while (n--) {
        if (*p == (char)c){
            //printf("Found character '%c'.\n", c);
            return p;
        }
        p++;
    }
    return NULL;
}

// Copy a block of memory from one location to another
bool memory_copy(void* dest, const void* src, size_t n)
{
    if (!dest || !src || n == 0) return false;
    char* d = dest;
    const char* s = src;

    // printf("Copying %zu bytes from %p to %p\n", n, src, dest);
    // printf("%c\n", ((char*)dest)[0]);
    // *d = *s;
    // printf("%c\n", ((char*)dest)[0]);
    while (n-- != 0) {

        *d++ = *s++;
    }
    return true;
}

// Compare two blocks of memory, return true if they are equal
bool memory_compare(const void* ptr1, const void* ptr2, size_t n)
{
    if (!ptr1 || !ptr2 || n == 0) return false;

    const char* p1 = ptr1;
    const char* p2 = ptr2;

    while (n--) {
        if (*p1++ != *p2++) return false;
    }
    return true;
}

// only when dealing with c-strings we can assume null-termination. This is bad.
void memory_replace(char* ptr, const char old_char, const char new_char)
{
    if (!ptr) return;
    while (*ptr) {
        if (*ptr == old_char) {
            *ptr = new_char;
        }
        ptr++;
    }
}

// Free a previously allocated block of memory
void memory_free(void** ptr)
{
    if (!ptr || !*ptr) {
        print("Attempted to free a NULL pointer\n");
        return;
    }
    //printf("Freeing memory at %p\n", *ptr);
    free(*ptr);
    //printf("Freed memory at %p\n", *ptr);
    *ptr = NULL;
}