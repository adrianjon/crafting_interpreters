//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_GENERATE_AST_V2_H
#define LOX_GENERATE_AST_V2_H
#include <stdbool.h>
#include <stddef.h>

static char const * g_ast_expr_grammar[] = {
    "assign   : token_t * target, expr_t * value",
    "binary   : expr_t * left, token_t * operator, expr_t * right",
    "call     : expr_t * callee, token_t * paren, expr_t ** arguments, size_t count",
    "get      : expr_t * object, token_t * name",
    "grouping : expr_t * expression",
    "literal  : token_t * kind",
    "logical  : expr_t * left, token_t * operator, expr_t * right",
    "set      : expr_t * object, token_t * name, expr_t * value",
    "super    : token_t * keyword, token_t * method",
    "this     : token_t * keyword",
    "unary    : token_t * operator, expr_t * right",
    "variable : token_t * name",
    NULL
};

static char const * g_ast_stmt_grammar[] = {
    "block      : stmt_t ** statements, size_t count",
    "function   : token_t * name, token_t ** params, size_t params_count, stmt_t ** body, size_t count",
    "class      : token_t * name, expr_t ** superclass, size_t superclass_count, stmt_t ** methods, size_t methods_count",
    "expression : expr_t * expression",
    "if         : expr_t * condition, stmt_t * then_branch, stmt_t * else_branch",
    "print      : expr_t * expression",
    "return     : token_t * keyword, expr_t * value",
    "var        : token_t * name, expr_t * initializer",
    "while      : expr_t * condition, stmt_t * body",
    NULL
};

bool generate_ast(char const * target, char const * name, char const * grammar[]);

#endif //LOX_GENERATE_AST_V2_H