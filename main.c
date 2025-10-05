
#include "extra/Arrays.h"
#include "extra/Memory.h"
#include "lox/Scanner.h"
#include "lox/Parser.h"
#include "lox/Interpreter.h"
#include "lox/Resolver.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdio.h>


#include "extra/Stack.h"
#include "extra/Map.h"


int main(void) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // TESTING
    // region_t * r = new_region();
    // activate_region(r);
    // stack_t * p_stack = stack_create(8);
    // map_t * p_map = map_create(10);
    // stack_push(p_stack, p_map);
    // int * x = memory_allocate(sizeof(int));
    // *x = 40;
    // bool success = map_put(p_map, "hello", x);
    // const int * result = map_get(p_map, "hello");
    // printf("Stack size: %zu\n", stack_size(p_stack));
    // printf("Result: %d\n", *result);
    // map_t * a = stack_peek(p_stack);
    // int y = 5;
    // map_put(a, "hello", &y);
    // printf("Stack size: %zu\n", stack_size(p_stack));
    // result = map_get(p_map, "hello");
    // printf("Result: %d\n", *result);
    //
    // map_destroy(p_map);
    // stack_destroy(p_stack);
    // region_free(r);

    // allocate memory
    region_t * scanner_region = new_region();
    region_t * parser_region = new_region();
    region_t * interpreter_region = new_region();

    activate_region(scanner_region);
    scanner_t * p_scanner = scanner_init("lox.txt");
    scanner_scan(p_scanner);
    scanner_print_tokens(p_scanner);

    activate_region(parser_region);
    // Transition from scanner_region to parser_region
    parser_t * p_parser = parser_init(scanner_get_tokens(p_scanner));
    region_free(scanner_region);

    activate_region(interpreter_region);
    // Transition from parser_region to interpreter_region
    const dynamic_array_t * statements = parse_statements(p_parser);
    region_free(parser_region);

    interpreter_t * p_interpreter = new_interpreter();
    resolver_t * p_resolver = new_resolver(p_interpreter);
    resolve_list(statements->data, statements->size / sizeof(stmt_t*) , p_resolver);
    interpret(statements, p_interpreter);
    region_free(interpreter_region);

    return 0;
}
