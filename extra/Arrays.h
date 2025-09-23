//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_ARRAYS_H
#define LOX_ARRAYS_H

#define NULL ((void *)0)
typedef unsigned long long size_t;

// typedef struct {
//     void* data;
// } Object;

typedef struct {
    void* data;
    size_t size;
    size_t capacity; // in
} dynamic_array_t;

dynamic_array_t create_array(size_t initial_capacity);
void array_push(dynamic_array_t* array, const void* element, size_t element_size);
void* array_get(const dynamic_array_t* array, size_t index);
void array_free(dynamic_array_t* array);

typedef struct {
    char* buffer;
    size_t length;
    size_t capacity;
} string_builder_t;

string_builder_t create_string_builder(void);
void append_string(string_builder_t* sb, const char* str);
char* string_to_uppercase(const char* str);
char* string_to_lowercase(const char* str);


#define MAX_TOKEN_LEN 256
size_t split_string_copy(const char* str, char delimiter, char tokens[][MAX_TOKEN_LEN], size_t max_tokens, size_t buf_size);

size_t trim(char* str, char delimiter);

#endif //LOX_ARRAYS_H