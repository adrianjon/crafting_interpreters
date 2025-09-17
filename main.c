
#include <stdio.h>
#include "extra/Arrays.h"

#include "lox/AstPrinter.h"
#include "lox/Parser.h"
#include "lox/Scanner.h"
// #include "lox/Token.h"
// #include "lox/Object.h"
#include "lox/Expr.h"
// #include "lox/Stmt.h"

int main(void) {
    dynamic_array_t tokens;
    parser_t parser = {.tokens = &tokens, .current = 0};
    scanner_main(&tokens);
    // while(!token_is_at_end(&parser)) {
    //     //expr_t* expr = parse_expression(&parser);
    //     token_t token = token_advance(&parser);
    //     printf("Token: %s at line %d", token_type_names[token.type], token.line);
    //     if (token.lexeme) {
    //         printf(" with lexeme '%s'", token.lexeme);
    //     }
    //     printf("\n");
    // }
    print_tokens(&tokens);
    expr_t* expr = parse_expression(&parser);
    if (!expr) {
        printf("Failed to parse expression\n");
        return 1;
    }
    printf("Parsed expression of type %s\n", expr_type_names[expr->type]);

    string_builder_t sb = create_string_builder();
    ast_printer_t printer = {0};
    ast_printer_init(&printer, &sb);

    char* result = ast_printer_print_expr(&printer, expr);
    printf("AST Printer Result: %s\n", result);
    printf("String Builder Content: %s\n", sb.buffer);
    memory_free((void**)&result);

    return 0;
}