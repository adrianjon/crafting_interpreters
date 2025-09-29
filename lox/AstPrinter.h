// // DEPRECATED
// // Created by adrian on 2025-09-15.
// //
//
// #ifndef LOX_ASTPRINTER_H
// #define LOX_ASTPRINTER_H
//
// #include "../extra/Arrays.h"
// #include "Expr.h"
// #include "Stmt.h"
//
// typedef struct ast_printer ast_printer_t;
//
// // new API
// ast_printer_t * ast_printer_init(string_builder_t * sb_p);
// char * ast_printer_print_expression(ast_printer_t * printer_p, const expr_t * expr_p);
// void ast_printer_free(ast_printer_t * printer_p);
// // Entry points
// char* ast_printer_print_expr(ast_printer_t* printer_p, const expr_t* expr_p);
// char* ast_printer_print_stmt(ast_printer_t* printer_p, const stmt_t* stmt_p);
//
// #endif //LOX_ASTPRINTER_H