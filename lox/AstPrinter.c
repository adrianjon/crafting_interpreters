// // DEPRECATED
// // Created by adrian on 2025-09-15.
// //
// #include "AstPrinter.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdarg.h>
// #include <string.h>
//
// #include "../extra/Arrays.h"
// #include "../extra/Memory.h"
// #include "Expr.h"
// #include "Stmt.h"
// #include "Token.h"
// struct ast_printer {
//     string_builder_t* sb_p;
//     expr_visitor_t expr_visitor;
//     stmt_visitor_t stmt_visitor;
// };
// // TODO: Split this into ast printer using push-sink (string builder) and
// // ast to string functions that return newly allocated strings.
// // Currently mixing both approaches.
//
// // Forward declarations
// static void* ap_unimpl_expr(const expr_t* expr, const expr_visitor_t* v, void* ctx);
// static void* ap_unimpl_stmt(const stmt_t* stmt, const stmt_visitor_t* v, void* ctx);
// static void* visit_literal_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
// static void* visit_print_stmt(const stmt_t* stmt, const stmt_visitor_t* visitor, void* context);
// static void* visit_unary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
// static void* visit_binary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
// static void* visit_grouping_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context);
// // Public API
// ast_printer_t *  ast_printer_init(string_builder_t* sb_p) {
//     ast_printer_t * printer_p = memory_allocate(sizeof(ast_printer_t));
//     // if (printer_p == NULL || sb_p == NULL) {
//     //     fprintf(stderr, "Error: ast_printer_init received NULL pointer\n");
//     //     exit(EXIT_FAILURE);
//     // }
//     if (printer_p == NULL) {
//         fprintf(stderr, "Error: ast_printer_init received NULL pointer\n");
//         exit(EXIT_FAILURE);
//     }
//     printer_p->sb_p = sb_p;
//
//     // Initialize all visitor function pointers to unimplemented fallbacks
//     printer_p->expr_visitor.visit_assign = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_binary = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_call = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_get = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_grouping = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_literal = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_logical = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_set = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_super = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_this = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_unary = ap_unimpl_expr;
//     printer_p->expr_visitor.visit_variable = ap_unimpl_expr;
//
//     printer_p->stmt_visitor.visit_block = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_function = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_class = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_expression = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_if = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_print = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_return = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_var = ap_unimpl_stmt;
//     printer_p->stmt_visitor.visit_while = ap_unimpl_stmt;
//
//     // Wire up the expression visitor function pointers
//     printer_p->expr_visitor.visit_binary = visit_binary_expr;
//     printer_p->expr_visitor.visit_literal = visit_literal_expr;
//     printer_p->expr_visitor.visit_unary = visit_unary_expr;
//     printer_p->expr_visitor.visit_grouping = visit_grouping_expr;
//
//     // Wire up the statement visitor function pointers
//     printer_p->stmt_visitor.visit_print = visit_print_stmt;
//
//     return printer_p;
// }
// char* ast_printer_print_expr(ast_printer_t* printer_p, const expr_t* expr_p) {
//     return expr_accept((expr_t*)expr_p, &printer_p->expr_visitor, NULL);
// }
// char* ast_printer_print_stmt(ast_printer_t* printer_p, const stmt_t* stmt_p) {
//     return stmt_accept((stmt_t*)stmt_p, &printer_p->stmt_visitor, NULL);
// }
// void ast_printer_free(ast_printer_t * printer_p) {
//     if (!printer_p) return;
//     if (printer_p->sb_p) {
//         memory_free((void**)&printer_p->sb_p->buffer);
//         printer_p->sb_p->buffer = NULL;
//         printer_p->sb_p->length = 0;
//         printer_p->sb_p->capacity = 0;
//     }
//     memory_free((void**)&printer_p);
//     printer_p = NULL;
// }
// char * ast_printer_print_expression(ast_printer_t * printer_p, const expr_t * expr_p) {
//     // TODO implement these
//     return NULL;
// }
// // Private functions
// static void print_token(string_builder_t* sb_p, const token_t* token_p) {
//     if (token_p->type == STRING) {
//         append_string(sb_p, "\"");
//         append_string(sb_p, token_p->lexeme);
//         append_string(sb_p, "\"");
//         return;
//     }
//     append_string(sb_p, token_p->lexeme);
// }
// static char* str_printf(const char* fmt, ...) {
//     va_list ap;
//     va_start(ap, fmt);
//     va_list ap2;
//     va_copy(ap2, ap);
//     const int n = vsnprintf(NULL, 0, fmt, ap);
//     va_end(ap);
//     if (n < 0) {
//         va_end(ap2);
//         return NULL;
//     }
//     char* buf = memory_allocate((size_t)n + 1);
//     if (!buf) {
//         va_end(ap2);
//         return NULL;
//     }
//     vsnprintf(buf, (size_t)n + 1, fmt, ap2);
//     va_end(ap2);
//     return buf;
// }
// static char* paren1(const char* name, char* e) {
//     char* result = str_printf("(%s %s)", name, e ? e : "(null)");
//     memory_free((void**)&e);
//     return result;
// }
// static void* ap_unimpl_expr(const expr_t* expr, const expr_visitor_t* v, void* ctx) {
//     // TODO implement this
//     return NULL;
// }
// static void* ap_unimpl_stmt(const stmt_t* stmt, const stmt_visitor_t* v, void* ctx) {
//     // TODO implement this
//     return NULL;
// }
// static void* visit_literal_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
//     (void)visitor; (void)context;
//     const token_t * token = expr->as.literal_expr.kind;
//
//     if (token && token->lexeme) {
//         char * result = _strdup(token->lexeme);
//         return result;
//     }
//
//     // for null or non-string literal, return "nil" or similar
//     char * result = _strdup("nil");
//     return result;
// }
// static void* visit_print_stmt(const stmt_t* stmt, const stmt_visitor_t* visitor, void* context) {
//     // TODO implement this
//     return NULL;
// }
// static void* visit_unary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
//     const char * operator = expr->as.unary_expr.operator->lexeme;
//     char * right = expr_accept(expr->as.unary_expr.right, visitor, context);
//     const size_t len = strlen(operator) + strlen(right) + 4;
//     char * result = memory_allocate(len);
//     if (!result) {
//         fprintf(stderr, "Error: string allocation failed in visit_unary_expr\n");
//         memory_free((void**)&right);
//         return NULL;
//     }
//     snprintf(result, len, "(%s %s)", operator, right);
//     memory_free((void**)&right);
//     return result;
// }
// static void* visit_binary_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
//
//     char * left = expr_accept(expr->as.binary_expr.left, visitor, context);
//     char * right = expr_accept(expr->as.binary_expr.right, visitor, context);
//
//     const char * operator = expr->as.binary_expr.operator->lexeme;
//
//     const size_t len = strlen(left) + strlen(operator) + strlen(right) + 5; //  5 for "(", " ", " ", ")" and null
//
//     char * result = memory_allocate(len);
//     if (!result) {
//         fprintf(stderr, "Error: string allocation failed in visit_binary_expr\n");
//         memory_free((void**)&left);
//         memory_free((void**)&right);
//         return NULL;
//     }
//     snprintf(result, len, "(%s %s %s)", left, operator, right);
//
//     memory_free((void**)&left);
//     memory_free((void**)&right);
//
//     return result;
// }
// static void* visit_grouping_expr(const expr_t* expr, const expr_visitor_t* visitor, void* context) {
//     char * inner = expr_accept(expr->as.grouping_expr.expression, visitor, context);
//     const size_t len = strlen(inner) + 3;
//     char * result = memory_allocate(len);
//     if (!result) {
//         fprintf(stderr, "Error: string allocation failed in visit_grouping_expr\n");
//         memory_free((void**)&inner);
//         return NULL;
//     }
//     snprintf(result, len, "(%s)", inner);
//     memory_free((void**)&inner);
//     return result;
// }