
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
#include "lox/ast_interpreter.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// TODO fix free functions. Exception occurs during scanner_free()
int main(void) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    scanner_t * p_scanner = scanner_init("lox.txt");
    scanner_scan(p_scanner);
    scanner_print_tokens(p_scanner);
    parser_t * p_parser = parser_init(scanner_get_tokens(p_scanner));


    expr_t* expr = parser_parse_expression(p_parser);
    // expression tree stores lexemes, should be free to free scanner and parser here

    parser_free(p_parser);
    scanner_free(p_scanner);

    if (!expr) {
        printf("Failed to parse expression\n");
        return 1;
    }
    if (g_error_flag) {
        printf("Error parsing expression\n");
        return 1;
    }
    printf("Parsed expression of type %s\n", g_expr_type_names[expr->type]);

    ast_printer_t * printer = ast_printer_init(NULL);
    char* result = ast_printer_print_expr(printer, expr);
    if (result) {
        printf("AST Printer Result: %s\n", result);
        memory_free((void**)&result);
    }
    ast_printer_free(printer);

    ast_evaluator_t * evaluator_p = ast_evaluator_init();
    value_t* val = ast_evaluator_eval_expr(evaluator_p, expr);
    if (val) {
        switch (val->type) {
            case VAL_NUMBER:
                printf("RESULT: %f\n", val->as.number);
                break;
            case VAL_BOOL:
                printf("RESULT: %s\n", val->as.boolean == true ? "true" : "false");
                break;
            default:
                printf("Error");
                break;
        }
        memory_free((void**)&val);
    }

    ast_evaluator_free(evaluator_p);
    free_expression(expr);

    return 0;
}
