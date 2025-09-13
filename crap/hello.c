#include <stdio.h>
#include "../extra/Memory.h"

// void* memory_allocate(size_t size) {
//     if (size == 0) return NULL;
//     void* ptr = malloc(size);
//     if (!ptr) {
//         printf("Memory allocation failed\n");
//     }
//     return ptr;
// }
// void memory_free(void** ptr) {
//     if (!ptr || !*ptr) {
//         printf("Attempted to free a NULL pointer\n");
//         return;
//     }
//     printf("Freeing memory at %p\n", *ptr);
//     free(*ptr);
//     printf("Freed memory at %p\n", *ptr);
//     *ptr = NULL;
// }
int main(void) {
    printf("Hello, World!\n");
    int *x = (int*)memory_allocate(10 * sizeof(int));
    memory_free(&x);
    printf("Freed malloced memory at %p\n", x);
    return 0;
}