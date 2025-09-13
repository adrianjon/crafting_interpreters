#ifndef HELPERS_H
#define HELPERS_H

#include <windows.h>

// MEMORY OPERATIONS

static char heap[1024 * 1024]; // 1MB heap
static size_t heap_size = 0;
#define NULL ((void *)0)
typedef enum {
    true = 1,
    false = 0
} bool;


char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return NULL;
}
char* strtok(char* str, const char* delim) { // ignore whitespace
    static char* last = NULL;
    if (str) last = str;
    if (!last) return NULL;

    char* token = last;
    while (*last && !strchr(delim, *last)) last++;
    if (*last) {
        *last = '\0';
        last++;
    } else {
        last = NULL;
    }
    return token;
}
void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}
bool memcmp(const void* ptr1, const void* ptr2, size_t n) {
    const char* p1 = (const char*)ptr1;
    const char* p2 = (const char*)ptr2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return false;
        }
    }
    return true;
}
void* malloc(size_t size) {
    if (heap_size + size > sizeof(heap)) {
        return NULL; // Out of memory
    }
    void* ptr = &heap[heap_size];
    heap_size += size;
    return ptr;
}
void* realloc(void* ptr, size_t new_size) {
    // Simple realloc implementation that always allocates new memory
    void* new_ptr = malloc(new_size);
    if (new_ptr && ptr) {
        // Copy old data to new location (not safe, just for demonstration)
        memcpy(new_ptr, ptr, new_size);
    }
    return new_ptr;
}

typedef struct {
    void* data;
    size_t size;
    size_t capacity;
} DynamicArray;

DynamicArray create_array(size_t initial_capacity) {
    DynamicArray array;
    array.data = malloc(initial_capacity);
    array.size = 0;
    array.capacity = initial_capacity;
    return array;
}

void array_push(DynamicArray* array, void* element, size_t element_size) {
    if (array->size + element_size > array->capacity) {
        array->capacity *= 2;
        array->data = realloc(array->data, array->capacity);
    }
    memcpy((char*)array->data + array->size, element, element_size);
    array->size += element_size;
}

void* array_get(DynamicArray* array, size_t index) {
    if (index < array->size) {
        return (char*)array->data + index;
    }
    return NULL;
}
void array_free(DynamicArray* array) {
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

typedef struct {
    char* buffer;
    size_t length;
    size_t capacity;
} StringBuilder;

StringBuilder create_string_builder(void) {
    StringBuilder sb;
    sb.buffer = (char*)malloc(1024);
    sb.length = 0;
    sb.capacity = 1024;
    return sb;
}

void append_string(StringBuilder* sb, const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    while (sb->length + len > sb->capacity) {
        sb->capacity *= 2;
        sb->buffer = (char*)realloc(sb->buffer, sb->capacity);
    }
    memcpy(sb->buffer + sb->length, str, len);
    sb->length += len;
}

// void free_string_builder(StringBuilder* sb) {
//     free(sb->buffer);
//     sb->buffer = NULL;
//     sb->length = 0;
//     sb->capacity = 0;
// }





// CUSTOM STRING TYPE
// typedef struct {
//     char* data;
//     size_t length;
// } String;


// WINDOWS IO OPERATIONS
void print(const char* str) {
    DWORD written;
    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), str, len, &written, NULL);
}
void print_int(int value) {
    // convert int to string without std functions
    char buffer[32];
    int i = 0;
    if (value == 0) {
        buffer[i++] = '0';
    } else {
        int is_negative = 0;
        if (value < 0) {
            is_negative = 1;
            value = -value;
        }
        while (value > 0) {
            buffer[i++] = '0' + (value % 10);
            value /= 10;
        }
        if (is_negative) {
            buffer[i++] = '-';
        }
    }
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    buffer[i] = '\0';
    print(buffer);
}
void print_char(char c) {
    char buffer[2];
    buffer[0] = c;
    buffer[1] = '\0';
    print(buffer);
}
void print_error(const char* str) {
    char buffer[256];
    memcpy(buffer, "ERROR: ", 7);
    memcpy(buffer + 7, str, 256 - 7);
    print(buffer);
    print("\n");
}

typedef struct {
    char* buffer;
    size_t size;
} File;
File* read_file(const char* filename) {
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    DWORD file_size = GetFileSize(file, NULL);
    if (file_size == INVALID_FILE_SIZE) {
        CloseHandle(file);
        return NULL;
    }
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        CloseHandle(file);
        return NULL;
    }
    DWORD read;
    if (!ReadFile(file, buffer, file_size, &read, NULL) || read != file_size) {
        CloseHandle(file);
        return NULL;
    }
    buffer[file_size] = '\0'; // Null-terminate the string
    CloseHandle(file);
    File* f = (File*)malloc(sizeof(File));
    f->buffer = buffer;
    f->size = file_size;
    return f;
}
File* write_file(const char* filename, const char* data, size_t size) {
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    DWORD written;
    if (!WriteFile(file, data, size, &written, NULL) || written != size) {
        CloseHandle(file);
        return NULL;
    }
    CloseHandle(file);
    File* f = (File*)malloc(sizeof(File));
    f->buffer = (char*)data;
    f->size = size;
    return f;
}
#endif // HELPERS_H