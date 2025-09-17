//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_ASTPRINTER_H
#define LOX_ASTPRINTER_H

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

#endif //LOX_ASTPRINTER_H