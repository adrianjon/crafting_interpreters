//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_PARSER_H
#define LOX_PARSER_H

#include "../extra/Arrays.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Expr.h"


extern const char* g_expr_type_names[];
extern const char* g_stmt_type_names[];
extern bool g_error_flag;
typedef struct parser parser_t;
struct parser {
    dynamic_array_t * tokens;
    size_t current;
    token_t * previous;
    token_t * current_token;
    bool * had_error;
    bool * panic_mode;
};

expr_t* parse_expression(parser_t* parser);
expr_t* parse_equality(parser_t* parser);
expr_t* parse_comparison(parser_t* parser);
expr_t* parse_term(parser_t* parser);
expr_t* parse_factor(parser_t* parser);
expr_t* parse_unary(parser_t* parser);
expr_t* parse_primary(parser_t* parser);



#endif //LOX_PARSER_H