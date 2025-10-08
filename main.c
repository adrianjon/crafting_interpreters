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
#include "lox/Callable.h"

int main(void) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // TESTING
    // region_t * r = new_region();
    // activate_region(r);
    //
    // map_t * map = map_create(10, map_default_config());
    // map_put(map, "apple", "red");
    // map_put(map, "banana", "yellow");
    // map_put(map, "grape", "purple");
    //
    // map_enumerator_t * it = map_get_enumerator(map);
    // printf("Map contents:\n");
    // while (map_enumerator_next(it)) {
    //     const map_entry_t * entry = map_enumerator_current(it);
    //     printf("    %s => %s\n",
    //         (const char*)map_entry_key(entry), (const char*)map_entry_value(entry));
    // }
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
    resolve_list(statements->data, statements->size / sizeof(stmt_t *), p_resolver);
    interpret(statements, p_interpreter);
    region_free(interpreter_region);

    return 0;
}
