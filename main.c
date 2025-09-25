
#include <stdio.h>
#include "extra/Arrays.h"

#include "lox/AstPrinter.h"
#include "lox/Parser.h"
#include "lox/Scanner.h"
#include "lox/Environment.h"
// #include "lox/Token.h"
// #include "lox/Object.h"
#include "lox/Expr.h"
// #include "lox/Stmt.h"
#include "extra/Memory.h"
#include <string.h>
#include "lox/ast_interpreter.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

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
