#include <stdio.h>
#include <stdbool.h>
// #include <stdlib.h>
#include <stdint.h>
#include "Memory.h"
#include "Windows.h"


#ifdef CUSTOM_MEMORY_ALLOCATOR
struct memory_chunk {
    struct memory_chunk* next;
    size_t size;
    bool free; // is 8 bytes, could be reduced to 1 byte using bit fields
    //uint8_t padding[7]; // unused padding
};
#define MAX_MEMORY_POOL_SIZE (1024 * 1024) // 1MB
static struct {
    char data[MAX_MEMORY_POOL_SIZE]; // 1MB pool
    struct memory_chunk* head;
    size_t total_size;
    size_t used_size;
} global_memory_pool = { .head = NULL, .total_size = 0, .used_size = 0 };

bool memory_allocate_new(size_t size, struct memory_chunk** new_chunk) {
    struct memory_chunk* current = global_memory_pool.head;
    struct memory_chunk* prev = NULL;

    while (current) {

        if (current->free && current->size >= size) {
            // Found a free chunk, no need to allocate a new one
            // Try to split chunk
            if (current->size > size + sizeof(struct memory_chunk)) {
                struct memory_chunk* split_chunk = (struct memory_chunk*)((char*)current + sizeof(struct memory_chunk) + size);
                split_chunk->size = current->size - size - sizeof(struct memory_chunk);
                split_chunk->next = current->next;
                split_chunk->free = true;
                current->next = split_chunk;
                current->size = size;
            }
            current->free = false;
            *new_chunk = current;
            return false;
        }
        prev = current;
        current = current->next;
    }

    // If we reached here, it means we need to allocate a new chunk
    *new_chunk = (struct memory_chunk*)(global_memory_pool.data + global_memory_pool.total_size);
    if (!*new_chunk || (global_memory_pool.total_size + sizeof(struct memory_chunk) + size > MAX_MEMORY_POOL_SIZE)) {
        return false; // Allocation failed
    }
    (*new_chunk)->size = size;
    (*new_chunk)->free = false;
    (*new_chunk)->next = NULL;
    if (prev) {
        prev->next = *new_chunk;
    } else {
        global_memory_pool.head = *new_chunk;
    }
    return true;
}
void* memory_allocate(size_t size) {
    if (size == 0 || size > 1024 * 1024) {
        return NULL; // Invalid size
    }

    struct memory_chunk* new_chunk = NULL;

    if (memory_allocate_new(size, &new_chunk)) {
        global_memory_pool.used_size += size;
        global_memory_pool.total_size += sizeof(struct memory_chunk) + size;
    }
    return (void*)((char*)new_chunk + sizeof(struct memory_chunk));
}

void memory_free(void** ptr) {
    if (!*ptr) return;

    // printf("Freeing memory at %p\n", ptr);
    struct memory_chunk* chunk = (struct memory_chunk*)((char*)*ptr - sizeof(struct memory_chunk));
    chunk->free = true;
    *ptr = NULL;
}


void debug_memory_pool() {
    printf("Debugging memory pool:\n");
    // printf("Chunk size: %zu\n", sizeof(struct memory_chunk));
    struct memory_chunk* current = global_memory_pool.head;
    size_t total_chunks = 0;
    size_t free_chunks = 0;

    while (current) {
        total_chunks++;
        if (current->free) {
            free_chunks++;
        }
        printf("\tChunk at %p: size=%zu, %s\n", (void*)current, current->size, current->free ? "free" : "");
        current = current->next;
    }

    printf("\tTotal chunks: %zu\n", total_chunks);
    printf("\tFree chunks: %zu\n", free_chunks);
    printf("\tTotal allocated memory: %zu\n", global_memory_pool.used_size);
    printf("\tTotal free memory: %d\n", MAX_MEMORY_POOL_SIZE - (int)global_memory_pool.used_size);
}

// Reallocate a previously allocated block of memory
void* memory_reallocate(void* ptr, size_t new_size) {
    if (!ptr) return memory_allocate(new_size);
    if (new_size == 0) {
        memory_free(&ptr);
        return NULL;
    }
    print("Reallocating memory\n");
    struct memory_chunk* chunk = (struct memory_chunk*)((char*)ptr - sizeof(struct memory_chunk));
    if (chunk->size >= new_size) {
        print("Realloc: existing chunk is already large enough\n");
        return ptr; // No need to reallocate, but we might want to shrink/split the chunk
    }

    // Allocate a new chunk and copy the old data
    void* new_ptr = memory_allocate(new_size);
    if (!new_ptr) return NULL;

    memory_copy(new_ptr, ptr, chunk->size);
    memory_free(&ptr);
    return new_ptr;
}

// Returns a pointer to the first occurrence of the character c in the memory block
void* memory_character(const void* ptr, int c, size_t n) {
    if (!ptr || n == 0) return NULL;

    char* p = (char*)ptr;
    while (n--) {
        if (*p == (char)c) return p;
        p++;
    }
    return NULL;
}

// Copy a block of memory from one location to another
bool memory_copy(void* dest, const void* src, size_t n) {
    if (!dest || !src || n == 0) return false;
    // ensure allocation in global memory pool
    // if (dest < global_memory_pool.data || dest >= global_memory_pool.data + MAX_MEMORY_POOL_SIZE) return false;
    // if (src < global_memory_pool.data || src >= global_memory_pool.data + MAX_MEMORY_POOL_SIZE) return false;

    char* d = (char*)dest;
    const char* s = (const char*)src;

    // ensure sufficient space for copy
    // if (d + n > global_memory_pool.data + MAX_MEMORY_POOL_SIZE) return false;

    while (n--) {
        *d++ = *s++;
    }
    return true;
}

// Compare two blocks of memory, return true if they are equal
bool memory_compare(const void* ptr1, const void* ptr2, size_t n) {
    if (!ptr1 || !ptr2 || n == 0) return false;

    const char* p1 = (const char*)ptr1;
    const char* p2 = (const char*)ptr2;

    while (n--) {
        if (*p1++ != *p2++) return false;
    }
    return true;
}

void memory_replace(char* ptr, char old_char, char new_char) {
    if (!ptr) return;
    while (*ptr) {
        if (*ptr == old_char) {
            *ptr = new_char;
        }
        ptr++;
    }
}
void memory_free_all(void) {
    struct memory_chunk* current = global_memory_pool.head;
    while (current) {
        void* data = (void*)((char*)current + sizeof(struct memory_chunk));
        memory_free(&data);
        current = current->next;
    }
    global_memory_pool.head = NULL;
    global_memory_pool.used_size = 0;
    global_memory_pool.total_size = 0;
}
#else
// Allocate a block of memory of given size
void* memory_allocate(size_t size) {
    if (size == 0) return NULL;
    void* ptr = malloc(size);
    if (!ptr) {
        print("Memory allocation failed\n");
    }
    return ptr;
}

// Reallocate a previously allocated block of memory
void* memory_reallocate(void* ptr, size_t new_size) {
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
const void* memory_character(const void* ptr, char c, size_t n)
{
    if (!ptr || n == 0) return NULL;

    const char* p = (const char*)ptr;
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
    char* d = (char*)dest;
    const char* s = (const char*)src;

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

    const char* p1 = (const char*)ptr1;
    const char* p2 = (const char*)ptr2;

    while (n--) {
        if (*p1++ != *p2++) return false;
    }
    return true;
}

// only when dealing with c-strings we can assume null-termination. This is bad.
void memory_replace(char* ptr, char old_char, char new_char)
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
        printf("Attempted to free a NULL pointer\n");
        return;
    }
    //printf("Freeing memory at %p\n", *ptr);
    free(*ptr);
    //printf("Freed memory at %p\n", *ptr);
    *ptr = NULL;
}

// debug functions TODO: remove api access
//void debug_memory_pool();
#endif
