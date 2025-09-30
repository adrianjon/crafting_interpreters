
#include "extra/Arrays.h"
#include "lox/Parser.h"
#include "lox/Scanner.h"
#include "lox/Interpreter.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

int main(void) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    scanner_t * p_scanner = scanner_init("lox.txt");
    scanner_scan(p_scanner);
    scanner_print_tokens(p_scanner);

    parser_t * p_parser = parser_init(scanner_get_tokens(p_scanner));
    dynamic_array_t * statements = parse_statements(p_parser);

    interpreter_t * p_interpreter = new_interpreter();
    interpret(statements, p_interpreter);
    free_statements(statements);

    interpreter_free(&p_interpreter);
    parser_free(p_parser);
    scanner_free(p_scanner);
    return 0;
}
