
#include <stdio.h>
#include "extra/Arrays.h"

#include "lox/AstPrinter.h"
#include "lox/Parser.h"
#include "lox/Scanner.h"
// #include "lox/Token.h"
// #include "lox/Object.h"
#include "lox/Expr.h"
// #include "lox/Stmt.h"
#include "lox/ast_interpreter.h"

int main(void) {
    dynamic_array_t tokens;
    parser_t parser = {.tokens = &tokens, .current = 0};
    scanner_main(&tokens);
    print_tokens(&tokens);

    expr_t* expr = parse_expression(&parser);
    if (!expr) {
        printf("Failed to parse expression\n");
        return 1;
    }
    if (g_error_flag) {
        printf("Error parsing expression\n");
        return 1;
    }
    printf("Parsed expression of type %s\n", g_expr_type_names[expr->type]);

    string_builder_t sb = create_string_builder();
    ast_printer_t printer = {0};
    ast_printer_init(&printer, &sb);

    char* result = ast_printer_print_expr(&printer, expr);
    // if (!result)
    //     printf("result string is null\n");
    // printf("AST Printer Result: %s\n", result);
    printf("String Builder Content: %s\n", sb.buffer);

    if (result)
        memory_free((void**)&result);


    ast_evaluator_t evaluator = {0};
    ast_evaluator_init(&evaluator);

    value_t* val = ast_evaluator_eval_expr(&evaluator, expr);
    if (val) {
        switch (val->type) {
            case VAL_NUMBER:
                printf("RESULT: %f", val->as.number);
                break;
            default:
                printf("Error");
                break;
        }
        memory_free((void**)&val);
    }
    return 0;
}
