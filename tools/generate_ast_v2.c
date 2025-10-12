//
// Created by adrian on 2025-10-11.
//

#include "generate_ast_v2.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int string_to_uppercase(char * dest, size_t dest_size, char const * source);

bool generate_ast(char const * target, char const * name, char const * grammar[]) {
    // File initializations. Creating one source file and one header file.
    FILE * p_source_file = NULL;
    FILE * p_header_file = NULL;
    char source_file_name[1024] = {0};
    char header_file_name[1024] = {0};
    snprintf(source_file_name, 1024, "%s/%s.c", target, name);
    snprintf(header_file_name, 1024, "%s/%s.h", target, name);
    errno_t e = 0;
    if ( (e = fopen_s(&p_source_file, source_file_name, "w")) != 0 ||
        (e = fopen_s(&p_header_file, header_file_name, "w")) != 0) {
        if (p_source_file) fclose(p_source_file);
        char error_message[1024];
        strerror_s(error_message, 1024, e);
        fprintf(stderr, "Error: %s\n", error_message);
        return false;
        }

    // Files created. Writing grammar to them.
    // Typedefs and API in header file.
    // Implementation in source file.
    char buffer[1024] = {0}; // Generic buffer to be used for various things.

    fprintf(p_source_file, "// This file is auto-generated. Do not edit\n\n");
    fprintf(p_header_file, "// This file is auto-generated. Do not edit\n\n");
    fprintf(p_header_file, "#ifndef ");
    string_to_uppercase(buffer, 1024, name);
    fprintf(p_header_file, "%s_H\n", buffer);
    fprintf(p_header_file, "#define %s_H\n\n", buffer);
    fprintf(p_header_file, "#include \"token.h\"\n");
    // fprintf(p_header_file, "#include \"expr.h\"\n");
    // fprintf(p_header_file, "#include \"stmt.h\"\n");
    fprintf(p_source_file, "#include \"expr.h\"\n");
    fprintf(p_source_file, "#include \"stmt.h\"\n");
    fprintf(p_header_file, "\n");
    fprintf(p_source_file, "\n");
    fprintf(p_header_file, "// Forward declarations\n");
    fprintf(p_header_file, "typedef struct expr expr_t;\n");
    fprintf(p_header_file, "typedef struct stmt stmt_t;\n");
    // fprintf(p_header_file, "typedef struct %s %s_t;\n", name, name);
    fprintf(p_header_file, "typedef struct %s_visitor %s_visitor_t;\n\n", name, name);

    // visitor struct and accept implementation
    fprintf(p_header_file, "struct %s_visitor {\n", name);
    fprintf(p_source_file, "void * %s_accept(%s_t const * %s, %s_visitor_t const * visitor, void * context) {\n", name, name, name, name);
    fprintf(p_source_file, "\tswitch(%s->type) {\n", name);
    for (size_t i = 0; grammar[i]; i++) {
        char const * end = strchr(grammar[i], ' ');
        size_t const len = end - grammar[i];
        strncpy_s(buffer, 1024, grammar[i], len);
        fprintf(p_header_file,
        "\tvoid * (*visit_%s)(%s_t const * %s, void * context);\n",
            buffer, name, name);

        string_to_uppercase(buffer, 1024, name);
        fprintf(p_source_file, "\t\tcase %s_", buffer);
        for (size_t j = 0; j < len; j++) {
            fprintf(p_source_file, "%c", (char)toupper(grammar[i][j]));
        }
        fprintf(p_source_file, ":\n");
        strncpy_s(buffer, 1024, grammar[i], len);
        fprintf(p_source_file, "\t\t\tif (visitor->visit_%s)\n", buffer);
        fprintf(p_source_file, "\t\t\t\treturn visitor->visit_%s(%s, context);\n", buffer, name);
    }
    fprintf(p_header_file, "};\n\n");

    fprintf(p_source_file, "\t\tdefault: return NULL;\n\t}\n}\n\n");

    // typedef enum
    fprintf(p_header_file, "// %s types\n", name);
    fprintf(p_header_file, "typedef enum {\n");
    for (size_t i = 0; grammar[i]; i++) {
        string_to_uppercase(buffer, 1024, name);
        fprintf(p_header_file, "\t%s_", buffer);
        char const * end = strchr(grammar[i], ' ');
        size_t const len = end - grammar[i];
        for (size_t j = 0; j < len; j++) {
            fprintf(p_header_file, "%c", (char)toupper(grammar[i][j]));
        }
        fprintf(p_header_file, ",\n");
    }
    fprintf(p_header_file, "} %s_type_t;\n\n", name);

    // typedef structs
    for (size_t i = 0; grammar[i]; i++) {
        fprintf(p_header_file, "typedef struct {\n");

        char * start = strchr(grammar[i], ':');
        if (!start) return false;
        start++;
        char * context = NULL;
        char * token = strtok_s(start, ",", &context);
        while (token) {
            fprintf(p_header_file, "\t%s;\n", token);
            token = strtok_s(NULL, ",", &context);
        }

        char const * end = strchr(grammar[i], ' ');
        size_t const len = end - grammar[i];
        strncpy_s(buffer, 1024, grammar[i], len);
        fprintf(p_header_file, "} %s_%s_t;\n\n", name, buffer);
    }

    // main struct type
    fprintf(p_header_file, "struct %s {\n", name);
    fprintf(p_header_file, "\t%s_type_t type;\n", name);
    fprintf(p_header_file, "\tunion {\n");
    for (size_t i = 0; grammar[i]; i++) {
        char const * end = strchr(grammar[i], ' ');
        size_t const len = end - grammar[i];
        strncpy_s(buffer, 1024, grammar[i], len);
        fprintf(p_header_file, "\t\t%s_%s_t %s_%s;\n", name, buffer, buffer, name);
    }
    fprintf(p_header_file, "\t} as;\n};\n\n");

    // API
    fprintf(p_header_file, "void * %s_accept(%s_t const * %s, %s_visitor_t const * visitor, void * context);\n", name, name, name, name);

    fprintf(p_header_file, "\n");
    fprintf(p_header_file, "#endif\n");



    fclose(p_source_file);
    fclose(p_header_file);
    return true;
}
static int string_to_uppercase(char * dest, size_t const dest_size, char const * source) {
    if (!dest || !source || dest_size == 0) return 1;
    size_t i = 0;
    while (source[i] != '\0') {
        if (i + 1 >= dest_size) return 1;
        dest[i] = (char)toupper(source[i]);
        i++;
    }
    dest[i] = '\0'; // If buffer was not empty before use
    return 0;
}
static void build_ast(void) {
    char const * target_dir = "./lox2";
    if (!generate_ast(target_dir, "expr", g_ast_expr_grammar) ||
        !generate_ast(target_dir, "stmt", g_ast_stmt_grammar)) {
        fprintf(stderr, "Failed to generate ast\n");
        exit(EXIT_FAILURE);
        }
}
int main(void) {
    build_ast();
    return 0;
}