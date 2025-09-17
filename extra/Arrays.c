//
// Created by adrian on 2025-09-15.
//

#include "Arrays.h"
#include "Memory.h"
#include "Windows.h"
//#include <stdlib.h>

dynamic_array_t create_array(size_t initial_capacity) {
    dynamic_array_t array;
    array.data = memory_allocate(initial_capacity);
    array.size = 0;
    array.capacity = initial_capacity;
    return array;
}

void array_push(dynamic_array_t* array, const void* element, const size_t element_size) {
    if (array->size + element_size > array->capacity) {
        array->capacity *= 2;
        array->data = memory_reallocate(array->data, array->capacity);
    }
    memory_copy((char*)array->data + array->size, element, element_size);
    array->size += element_size;
}

void* array_get(const dynamic_array_t* array, const size_t index) {
    if (index < array->size) {
        return (char*)array->data + index;
    }
    return NULL;
}
void array_free(dynamic_array_t* array) {
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}


string_builder_t create_string_builder(void) {
    string_builder_t sb;
    sb.buffer = (char*)memory_allocate(1024);
    if (!sb.buffer) {
        print("Failed to allocate memory for string builder\n");
    }
    sb.length = 0;
    sb.capacity = 1024;
    sb.buffer[0] = '\0';
    return sb;
}

void append_string(string_builder_t* sb, const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    while (sb->length + len > sb->capacity) {
        sb->capacity *= 2;
        sb->buffer = (char*)memory_reallocate(sb->buffer, sb->capacity);
        if (!sb->buffer) {
            print("Failed to reallocate memory for string builder\n");
            return;
        }
    }
    memory_copy(sb->buffer + sb->length, str, len);
    sb->length += len;
    sb->buffer[sb->length] = '\0'; // Null-terminate the string
}

char* string_to_uppercase(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    char* upper_str = memory_allocate(len + 1);
    for (size_t i = 0; i < len; i++) {
        const char c = str[i];
        upper_str[i] = (char)(c >= 'a' && c <= 'z' ? c - 32 : c);
    }
    upper_str[len] = '\0';
    return upper_str;
}
char* string_to_lowercase(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    char* lower_str = memory_allocate(len + 1);
    for (size_t i = 0; i < len; i++) {
        const char c = str[i];
        lower_str[i] = (char)(c >= 'A' && c <= 'Z' ? c + 32 : c);
    }
    lower_str[len] = '\0';
    return lower_str;
}

size_t split_string_copy(const char* str, const char delimiter, char tokens[][MAX_TOKEN_LEN], const size_t max_tokens, const size_t buf_size) {
    if (str == NULL || tokens == NULL || max_tokens == 0 || buf_size == 0) {
        return 0;
    }
    size_t count = 0;
    const char* start = str;
    const char* p = str;

    while (*p != '\0' && count < max_tokens) {
        if (*p == delimiter) {
            size_t len = p - start;
            if (len >= buf_size) {
                len = buf_size - 1;
            }
            memory_copy(tokens[count], start, len);
            tokens[count][len] = '\0';
            count++;
            start = p + 1;
        }
        p++;
    }

    // Capture the last token if there's no trailing delimiter
    if (count < max_tokens && *start != '\0') {
        size_t len = p - start;
        if (len >= buf_size) {
            len = buf_size - 1;
        }
        memory_copy(tokens[count], start, len);
        tokens[count][len] = '\0';
        count++;
    }

    return count;
}

size_t trim(char* str, const char delimiter) {
    //printf("Trimming string with delimiter '%c'\n", delimiter);
    if (str == NULL) {
        print("String is NULL\n");
        return 0;
    }
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    size_t start = 0;
    while (start < len && str[start] == delimiter) {
        start++;
    }
    size_t end = len;
    while (end > start && str[end - 1] == delimiter) {
        end--;
    }
    const size_t new_len = end - start;
    for (size_t i = 0; i < new_len; i++) {
        str[i] = str[start + i];
    }
    str[new_len] = '\0';
    return new_len;
}