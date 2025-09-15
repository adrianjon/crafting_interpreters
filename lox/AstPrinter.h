#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "../extra/Arrays.h"
#include "Expr.h"
#include "Stmt.h"

typedef struct ast_printer {
    string_builder_t* sb_p;
    expr_visitor_t expr_visitor;
    stmt_visitor_t stmt_visitor;
} ast_printer_t;

// Initialize and wire visitor function pointers.
void ast_printer_init(ast_printer_t* printer_p, string_builder_t* sb_p);

// Entry points
char* ast_printer_print_expr(ast_printer_t* printer_p, const expr_t* expr_p);
char* ast_printer_print_stmt(ast_printer_t* printer_p, const stmt_t* stmt_p);

// One-call, type-agnostic entry (compile-time dispatch with _Generic in C11+).
// #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
// #define ast_printer_print(p, node) \
//     _Generic((node),               \
//         expr_t*:       ast_printer_print_expr,       \
//         const expr_t*: ast_printer_print_expr,       \
//         stmt_t*:       ast_printer_print_stmt,       \
//         const stmt_t*: ast_printer_print_stmt        \
//     )(p, node)
// #else
// void ast_printer_print(ast_printer_t* p, const void* node) {
//     fprintf(stderr, "Error: ast_printer_print requires C11 or later for _Generic support.\n");
//     exit(EXIT_FAILURE);
// }
// // Fallback for pre-C11: require an explicit kind.
// typedef enum { AST_NODE_EXPR, AST_NODE_STMT } ast_node_kind_t;
// void ast_printer_print_kind(AstPrinter* p, ast_node_kind_t kind, const void* node) {
//     switch (kind) {
//         case AST_NODE_EXPR:
//             ast_printer_print_expr(p, (const expr_t*)node);
//             break;
//         case AST_NODE_STMT:
//             ast_printer_print_stmt(p, (const stmt_t*)node);
//             break;
//     }
// }
// #endif

#endif // AST_PRINTER_H