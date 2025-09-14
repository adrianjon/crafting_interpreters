#include <stdio.h>
#include "../extra/Arrays.h"
#include "../lox/Expr.h"
#include "../lox/Stmt.h"
#include "../lox/Token.h"
#include "../lox/Object.h"
#include "../lox/AstPrinter.h"

int main(void) {
    string_builder_t sb = create_string_builder();
    ast_printer_t printer = {0};
    ast_printer_init(&printer, &sb);

    expr_t expr = {
        .type = EXPR_LITERAL,
        .as.literal_expr = {
            .value = &(object_t){
                .type = OBJECT_STRING,
                .as.string = { .value = "Hello, World!" }
            }
        }
    };
    expr_t expr2 = {
        .type = EXPR_UNARY,
        .as.unary_expr = {
            .operator = &(token_t){
                .type = MINUS,
                .lexeme = "-",
                .line = 1
            },
            .right = &(expr_t){
                .type = EXPR_BINARY,
                .as.binary_expr = {
                    .left = &(expr_t){
                        .type = EXPR_LITERAL,
                        .as.literal_expr = {
                            .value = &(object_t){
                                .type = OBJECT_NUMBER,
                                .as.number = { .value = 123.45 }
                            }
                        }
                    },
                    .operator = &(token_t){
                        .type = PLUS,
                        .lexeme = "+",
                        .line = 1
                    },
                    .right = &(expr_t){
                        .type = EXPR_LITERAL,
                        .as.literal_expr = {
                            .value = &(object_t){
                                .type = OBJECT_NUMBER,
                                .as.number = { .value = 678.90 }
                            }
                        }
                    }
                }
            }
        }
    };
    stmt_t stmt = {
        .type = STMT_PRINT,
        .as.print_stmt = {
            .expression = &expr
        }
    };

    //printf("\nAST: %s\n", ast_printer_print_stmt(&printer, &stmt));
    ast_printer_print(&printer, &expr2);
    printf("\nAST: %s\n", sb.buffer);
}