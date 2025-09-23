//
// Created by adrian on 2025-09-15.
//
#include "Parser.h"
#include <stdio.h>
#include <stdarg.h>
#include "../extra/Arrays.h"
#include "../extra/Memory.h"
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
/* new grammar
    expression     → equality ;
    equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    term           → factor ( ( "-" | "+" ) factor )* ;
    factor         → unary ( ( "/" | "*" ) unary )* ;
    unary          → ( "!" | "-" ) unary
                   | primary ;
    primary        → NUMBER | STRING | "true" | "false" | "nil"
                   | "(" expression ")" ;

*/
struct parser {
    const dynamic_array_t * tokens;
    size_t current;
    token_t * previous;
    token_t * current_token;
    bool * had_error;
    bool * panic_mode;
};
// Forward declarations
static token_t token_advance(parser_t* parser);
static token_t token_peek(const parser_t* parser);
static bool token_match(parser_t* parser, int count, ...);
static bool token_check(const parser_t* parser, token_type_t expected);
static bool token_is_at_end(const parser_t* parser);
static token_t token_previous(const parser_t* parser);
static token_t* token_previous_ptr(const parser_t* parser);
static expr_t* parse_expression(parser_t* parser);
static expr_t* parse_equality(parser_t* parser);
static expr_t* parse_comparison(parser_t* parser);
static expr_t* parse_term(parser_t* parser);
static expr_t* parse_factor(parser_t* parser);
static expr_t* parse_unary(parser_t* parser);
static expr_t* parse_primary(parser_t* parser);
// Public API
const char* g_expr_type_names[] = {
    "EXPR_ASSIGN",
    "EXPR_BINARY",
    "EXPR_CALL",
    "EXPR_GET",
    "EXPR_GROUPING",
    "EXPR_LITERAL",
    "EXPR_LOGICAL",
    "EXPR_SET",
    "EXPR_SUPER",
    "EXPR_THIS",
    "EXPR_UNARY",
    "EXPR_VARIABLE"
};
const char* g_stmt_type_names[] = {
    "STMT_BLOCK",
    "STMT_FUNCTION",
    "STMT_CLASS",
    "STMT_EXPRESSION",
    "STMT_IF",
    "STMT_PRINT",
    "STMT_RETURN",
    "STMT_VAR",
    "STMT_WHILE"
};
bool g_error_flag = false;
parser_t * parser_init(const dynamic_array_t * tokens) {
    // does not own tokens array so should not free
    parser_t * parser = memory_allocate(sizeof(parser_t));
    parser->tokens = tokens;
    parser->current = 0;
    parser->current_token = NULL;
    parser->had_error = false;
    parser->panic_mode = false;
    parser->previous = NULL;
    return parser;
}
expr_t * parser_parse_expression(parser_t * p_parser) {
    return parse_expression(p_parser);
}
void parser_free(parser_t * p_parser) {
    if (!p_parser) return;
    if (p_parser->tokens) {
        // does not own tokens so should not free
        // TODO change owneship by copying tokens array
    }
    memory_free((void**)&p_parser);
    p_parser = NULL;
}
// token_t* should point to tokens in the shared parser token buffer. Should NOT be freed here
void free_expression(expr_t* expr) {
    if (!expr) return;

    switch (expr->type) {
        case EXPR_ASSIGN:
            free_expression(expr->as.assign_expr.value);
            break;
        case EXPR_BINARY:
            free_expression(expr->as.binary_expr.left);
            free_expression(expr->as.binary_expr.right);
            break;
        case EXPR_CALL:
            // call expression
            free_expression(expr->as.call_expr.callee);
            // assuming argument dynamic array consists of expressions
            const size_t number_of_arguments = expr->as.call_expr.arguments->size / sizeof(expr_call_t);
            for (size_t i = 0; i < number_of_arguments; i++) {
                free_expression((expr_t*)expr->as.call_expr.arguments->data + i);
            }
            break;
        case EXPR_GET:
            free_expression(expr->as.get_expr.object);
            break;
        case EXPR_GROUPING:
            free_expression(expr->as.grouping_expr.expression);
            break;
        case EXPR_LITERAL:
            break;
        case EXPR_LOGICAL:
            free_expression(expr->as.logical_expr.left);
            free_expression(expr->as.logical_expr.right);
            break;
        case EXPR_SET:
            free_expression(expr->as.set_expr.object);
            free_expression(expr->as.set_expr.value);
            break;
        case EXPR_SUPER:
            //break;
        case EXPR_THIS:
            break;
        case EXPR_UNARY:
            free_expression(expr->as.unary_expr.right);
            break;
        case EXPR_VARIABLE:
            //break;
        default:
            break;
    }
}
// Private functions
static expr_t* parse_expression(parser_t* parser) {
    //    expression     → equality
    return parse_equality(parser);
}
static expr_t* parse_equality(parser_t* parser) {
    //    equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    expr_t* expr = parse_comparison(parser);
    while (token_match(parser, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_comparison(parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = operator, .right = right };
        expr_t* new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}
static expr_t* parse_comparison(parser_t* parser) {
    //    comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    expr_t* expr = parse_term(parser);
    while (token_match(parser, 4, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_term(parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = operator, .right = right };
        expr_t* new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}
static expr_t* parse_term(parser_t* parser) {
    // term           → factor ( ( "-" | "+" ) factor )* ;
    expr_t* expr = parse_factor(parser);
    while (token_match(parser, 2, MINUS, PLUS)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_factor(parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = operator, .right = right };
        expr_t* new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}
static expr_t* parse_factor(parser_t* parser) {
    // factor         → unary ( ( "/" | "*" ) unary )* ;
    expr_t* expr = parse_unary(parser);
    while (token_match(parser, 2, SLASH, STAR)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_unary(parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = operator, .right = right };
        expr_t* new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}
static expr_t* parse_unary(parser_t* parser) {
    // unary          → ( "!" | "-" ) unary
    //                | primary ;
    if (token_match(parser, 2, BANG, MINUS)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_unary(parser);
        const expr_unary_t unary_expr = { .operator = operator, .right = right };
        expr_t* new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_UNARY;
        new_expr->as.unary_expr = unary_expr;
        return new_expr;
    }
    return parse_primary(parser);
}
static expr_t* parse_primary(parser_t* parser) {
    // primary        → NUMBER | STRING | "true" | "false" | "nil"
    //                | "(" expression ")" ;
    if (token_match(parser, 5, NUMBER, STRING, KW_TRUE, KW_FALSE, NIL)) {
        token_t* number_token = token_previous_ptr(parser);
        expr_t* expr = memory_allocate(sizeof(expr_t));
        expr->type = EXPR_LITERAL;
        expr->as.literal_expr.kind = number_token;
        return expr;
    }
    if (token_match(parser, 1, LEFT_PAREN)) {
        expr_t* expr = parse_expression(parser);
        if (!token_match(parser, 1, RIGHT_PAREN)) {
            // Error: expected ')'
            token_t * test = (token_t*)parser->tokens->data + parser->current - 1;
            printf("Error: token: %s\tline:%d\n", test->lexeme, test->line);
            printf("Error: Expected ')' after expression.\n");
            g_error_flag = true;
            return NULL;
        }
        expr_t* group = memory_allocate(sizeof(expr_t));
        group->type = EXPR_GROUPING;
        group->as.grouping_expr.expression = expr;
        return group;
    }
    // If none matched, error
    printf("Error: Expected expression.\n");
    // TODO: fix some other way
    g_error_flag = true; // sets global error flag
    return NULL;
}
static token_t token_advance(parser_t* parser) {
    if (!token_is_at_end(parser)) {
        parser->current++;
    }
    return token_previous(parser);
}
static token_t token_peek(const parser_t* parser) {
    return *(token_t*)array_get(parser->tokens, parser->current * sizeof(token_t));
}
static bool token_match(parser_t* parser, const int count, ...) {
    va_list args;
    va_start(args, count);
    bool matched = false;
    for (int i = 0; i < count; i++) {
        const token_type_t expected = va_arg(args, token_type_t);
        if (token_check(parser, expected)) {
            token_advance(parser);
            matched = true;
            break;
        }
    }
    va_end(args);
    return matched;
}
static bool token_check(const parser_t* parser, const token_type_t expected) {
    if (token_is_at_end(parser)) return false;
    return token_peek(parser).type == expected;
}
static bool token_is_at_end(const parser_t* parser) {
    return token_peek(parser).type == END_OF_FILE;
}
static token_t token_previous(const parser_t* parser) {
    if (parser->current == 0) return (token_t){ .type = END_OF_FILE };
    return *(token_t*)array_get(parser->tokens, (parser->current - 1) * sizeof(token_t));
}
static token_t* token_previous_ptr(const parser_t* parser) {
    if (parser->current == 0) return NULL;
    return array_get(parser->tokens, (parser->current - 1) * sizeof(token_t));
}