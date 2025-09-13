#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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

void* memory_allocate(size_t size);
void memory_free(void* ptr);

void debug_memory_pool();
int main(void) {
    void* p1 = memory_allocate(100);
    void* p2 = memory_allocate(300);
    void* p3 = memory_allocate(100);
    memory_free(&p2);
    void* p4 = memory_allocate(50);
    void* p5 = memory_allocate(100);
    void* p6 = memory_allocate(100);
    debug_memory_pool();

    return 0;
}
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
    printf("Chunk size: %zu\n", sizeof(struct memory_chunk));
    struct memory_chunk* current = global_memory_pool.head;
    size_t total_chunks = 0;
    size_t free_chunks = 0;

    while (current) {
        total_chunks++;
        if (current->free) {
            free_chunks++;
        }
        printf("Chunk at %p: size=%zu, %s\n", (void*)current, current->size, current->free ? "free" : "");
        current = current->next;
    }

    printf("Total chunks: %zu\n", total_chunks);
    printf("Free chunks: %zu\n", free_chunks);
    printf("Total allocated memory: %zu\n", global_memory_pool.used_size);
    printf("Total free memory: %d\n", MAX_MEMORY_POOL_SIZE - global_memory_pool.used_size);
}