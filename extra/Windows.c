//
// Created by adrian on 2025-09-15.
//

#pragma warning(disable: 5105)
#include <windows.h>
#pragma warning(default: 5105)
#include "Windows.h"
#include "Memory.h"

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
            buffer[i++] = (char)('0' + value % 10);
            value /= 10;
        }
        if (is_negative) {
            buffer[i++] = '-';
        }
    }
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        const char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    buffer[i] = '\0';
    print(buffer);
}
void print_char(const char c) {
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


file_t* read_file(const char* filename) {
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    DWORD file_size = GetFileSize(file, NULL);
    if (file_size == INVALID_FILE_SIZE) {
        CloseHandle(file);
        return NULL;
    }
    char* buffer = memory_allocate(file_size + 1);
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
    file_t* f = memory_allocate(sizeof(file_t));
    f->buffer = buffer;
    f->size = file_size;
    return f;
}
file_t* write_file(const char* filename, const char* data, size_t size) {
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        print_error("Failed to create file");
        return NULL;
    }
    DWORD written;
    if (!WriteFile(file, data, size, &written, NULL) || written != size) {
        print_error("Failed to write file");
        CloseHandle(file);
        return NULL;
    }
    // debug written
    print("Wrote ");
    print_int(written);
    print(" bytes to ");
    print(filename);
        print("\n");
    CloseHandle(file);
    file_t* f = memory_allocate(sizeof(file_t));
    f->buffer = (char*)data;
    f->size = size;
    return f;
}
