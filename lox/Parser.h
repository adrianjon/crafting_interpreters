//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_PARSER_H
#define LOX_PARSER_H

#include "../extra/Arrays.h"
#include <stdbool.h>
#include "Expr.h"
#include "Stmt.h"

extern const char* g_expr_type_names[];
extern const char* g_stmt_type_names[];
extern bool g_error_flag;
typedef struct parser parser_t;

// new API
parser_t * parser_init(const dynamic_array_t * tokens);
expr_t * parser_parse_expression(parser_t * p_parser);
stmt_t * parser_parse_statement(parser_t * p_parser);
dynamic_array_t * parse_statements(parser_t * p_parser);
token_type_t parser_get_current_token_type(const parser_t * p_parser);
void parser_free(parser_t * p_parser);
void free_expression(expr_t* expr);
void free_statement(stmt_t* stmt);

#endif //LOX_PARSER_H