#ifndef WINDOWS_H
#define WINDOWS_H

#include <windows.h>
#include "Memory.h"

typedef struct {
    char* buffer;
    size_t size;
} file_t;

// WINDOWS IO OPERATIONS
void print(const char* str);
void print_int(int value);
void print_char(char c);
void print_error(const char* str);

file_t* read_file(const char* filename);
file_t* write_file(const char* filename, const char* data, size_t size);

#endif // WINDOWS_H