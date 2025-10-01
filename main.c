
#include "extra/Arrays.h"
#include "extra/Memory.h"
#include "lox/Scanner.h"
#include "lox/Parser.h"
#include "lox/Interpreter.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>



int main(void) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

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
    interpret(statements, p_interpreter);
    region_free(interpreter_region);

    return 0;
}
