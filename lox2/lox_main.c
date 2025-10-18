//
// Created by adrian on 2025-10-11.
//

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdint.h>

#include "scanner.h"
#include "parser.h"
#include "interpreter.h"
#include "resolver.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L)
#undef NULL
#define NULL nullptr
#endif

#ifndef WIN32
// ReSharper disable once CppInconsistentNaming
#define fread_s(buffer, bufferSize, elementSize, count, stream) \
fread(buffer, elementSize, count, stream)
// ReSharper disable once CppInconsistentNaming
#define fopen_s(pFile, filename, mode) \
    0; *pFile = fopen(filename, mode)
// ReSharper disable once CppInconsistentNaming
#define strerror_s(buffer, sizeInBytes, errnum) \
    strerror(errnum)

// ReSharper disable once CppInconsistentNaming
#define errno_t int
#endif
char * file_to_string(char const * path, char * buffer, size_t buffer_size);

int main() {
#ifdef WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    char source[4096] = {0};
    char * p_source =
        file_to_string("./lox2/source/main.lox", source, sizeof(source));

    interpreter_t interpreter = {0};

    scanner_t scanner = { .start = p_source };

    list_t tokens = scan_tokens(&scanner); // List<token_t*>
    for (int i = 0; i < tokens.count; i++)
        printf("Token %d: %s\n", i, ((token_t*)tokens.data[i])->lexeme);

    parser_t parser = { .tokens = tokens };
    list_t statements = parse(&parser); // List<stmt_t*>

    resolver_t resolver = {.interpreter = &interpreter, .scopes = NULL};
    resolve(&resolver, &statements);
    interpret(&interpreter, &statements);

    //free_interpreter(&interpreter);
    //free_resolver(&resolver);
    list_free(&tokens);
    list_free(&statements);
    return 0;
}

char * file_to_string(char const * path,
    char * buffer, size_t const buffer_size) {
    if (!path || !buffer || buffer_size == 0) return NULL;
    FILE * source_file = NULL;
    errno_t const e = fopen_s(&source_file, path, "rb");
    if (e) {
        char error_message[1024];
        strerror_s(error_message, 1024, e);
        fprintf(stderr, "Error: %s\n", error_message);
        exit(EXIT_FAILURE);
    }

    if (fseek(source_file, 0, SEEK_END)) {
        perror("fseek");
        fclose(source_file);
        exit(EXIT_FAILURE);
    }
    int64_t const source_len = ftell(source_file);
    if (source_len < 0) {
        perror("ftell");
        fclose(source_file);
        exit(EXIT_FAILURE);;
    }
    rewind(source_file);
    if (buffer_size < source_len + 1) {
        perror("buffer to small");
        fclose(source_file);
        exit(EXIT_FAILURE);
    }

    char * source = buffer;
    size_t const nread = fread_s(source, (size_t)source_len, 1, source_len, source_file);
    if (nread != (size_t)source_len) {
        fprintf(stderr, "Read error: expected %lld bytes, got %zu\n", source_len, nread);
        free(source);
        fclose(source_file);
        exit(EXIT_FAILURE);
    }
    source[source_len] = '\0';
    fclose(source_file);
    return source;
}