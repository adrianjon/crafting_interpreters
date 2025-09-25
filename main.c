
#include <stdio.h>
#include "extra/Arrays.h"

#include "lox/AstPrinter.h"
#include "lox/Parser.h"
#include "lox/Scanner.h"
// #include "lox/Token.h"
// #include "lox/Object.h"
#include "lox/Expr.h"
// #include "lox/Stmt.h"
#include "extra/Memory.h"
#include <string.h>
#include "lox/ast_interpreter.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define MAX_GLOBALS 128
typedef struct {
    char * name;
    value_t value;
} global_var_t;
global_var_t g_globals[MAX_GLOBALS];
size_t g_globals_count = 0;
void set_global(const char * name, value_t * value) {
    for (size_t i = 0; i < g_globals_count; ++i) {
        if (strcmp(g_globals[i].name, name) == 0) {
            if (value->type == VAL_STRING) {
                g_globals[i].value.type = VAL_STRING;
                g_globals[i].value.as.string = strdup(value->as.string); // TODO need to free
                g_globals[i].value.is_on_heap = false;
            } else {
                g_globals[i].value = *value;
                g_globals[i].value.is_on_heap = false;
            }

            return;
        }
    }
    if (g_globals_count < MAX_GLOBALS) {
        g_globals[g_globals_count].name = strdup(name);
        if (value->type == VAL_STRING) {
            g_globals[g_globals_count].value.type = VAL_STRING;
            g_globals[g_globals_count].value.as.string = strdup(value->as.string);
            g_globals[g_globals_count].value.is_on_heap = false;
        } else {
            g_globals[g_globals_count].value = *value;
            g_globals[g_globals_count].value.is_on_heap = false;
        }
        ++g_globals_count;
    } else {
        fprintf(stderr, "Global variable limit reached!\n");
    }
}
value_t * get_global(const char * name) {
    for (size_t i = 0; i < g_globals_count; ++i) {
        if (strcmp(g_globals[i].name, name) == 0) {
            return &g_globals[i].value;
        }
    }
    return NULL;
}
void free_globals(void) {
    for (size_t i = 0; i < g_globals_count; ++i) {
        memory_free((void**)&g_globals[i].name);
        if (g_globals[i].value.type == VAL_STRING) {
            memory_free((void**)&g_globals[i].value.as.string);
        }
        g_globals[i].value.type = 0;
        // TODO if value_t contains heap allocations, free those too
    }
    g_globals_count = 0;
}
// TODO both variable names and values are leaking
int main(void) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    scanner_t * p_scanner = scanner_init("lox.txt");
    scanner_scan(p_scanner);
    scanner_print_tokens(p_scanner);
    parser_t * p_parser = parser_init(scanner_get_tokens(p_scanner));

    ast_evaluator_t * p_evaluator = ast_evaluator_init();

    // TODO this should be a function that takes a list of statements as input
    while (parser_get_current_token_type(p_parser) != END_OF_FILE) {
        // printf("Parsing statement...\n");
        stmt_t * stmt = parser_parse_statement(p_parser);
        // printf("Evaluating statement...\n");
        ast_evaluator_eval_stmt(p_evaluator, stmt);
        free_statement(stmt);
    }
    parser_free(p_parser);
    scanner_free(p_scanner);

    ast_evaluator_free(p_evaluator);

    free_globals();
    return 0;
}
