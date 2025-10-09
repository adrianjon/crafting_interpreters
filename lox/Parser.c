//
// Created by adrian on 2025-09-15.
//
#include "Parser.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../extra/Arrays.h"
#include "../extra/Memory.h"
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
/* new grammar
    expression     → assignment ;
    assignment     → IDENTIFIER "=" assignment | logic_or ;
    logic_or       → logic_and ( "or" logic_and )* ;
    logic_and      → equality ( "and" equality )* ;
    equality       → comparison ( ( "!=" | "==" ) comparison )* ;
    comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
    term           → factor ( ( "-" | "+" ) factor )* ;
    factor         → unary ( ( "/" | "*" ) unary )* ;
    unary          → ( "!" | "-" ) unary
                   | call ;
    call           → primary ( "(" arguments? ")" ) ;
    arguments      → expression ( "," expression )* ;
    primary        → NUMBER | STRING | "true" | "false" | "nil"
                   | "(" expression ")" | IDENTIFIER ;

    program         -> declaration* EOF ;
    declaration     -> classDecl | funDecl | varDecl | statement ;
    statement       -> exprStmt | ifStmt | whileStmt | printStmt | returnStmt | block ;
    exprStmt        -> expression ";" ;
    ifStmt          -> "if" "(" expression ")" statement
                        ( "else" statement )? ;
    whileStmt       -> "while" "(" expression ")" statement ;
    printStmt       -> "print" expression ";" ;
    returnStmt      -> "return" expression? ";" ;
    varDecl         -> "var" IDENTIFIER ( "=" expression )? ";" ;
    funDecl         -> "fun" function ;
    classDecl       -> "class" IDENTIFIER "{" function* "}" ;
    function        -> IDENTIFIER "(" parameters? ")" block ;
    parameters      -> IDENTIFIER ( "," IDENTIFIER )* ;
    block           -> "{" declaration* "}" ;

*/


struct parser {
    dynamic_array_t * tokens;
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

static token_t consume(parser_t * p_parser, token_type_t type, const char * message);
static expr_t * finish_call(parser_t * p_parser, expr_t * callee);

static expr_t* parse_expression(parser_t* parser);
static expr_t * parse_assignment(parser_t * p_parser);
static expr_t * parse_logical_or(parser_t * p_parser);
static expr_t * parse_logical_and(parser_t * p_parser);
static expr_t* parse_equality(parser_t* parser);
static expr_t* parse_comparison(parser_t* parser);
static expr_t* parse_term(parser_t* parser);
static expr_t* parse_factor(parser_t* parser);
static expr_t* parse_unary(parser_t* parser);
static expr_t * parse_call(parser_t * p_parser);
static expr_t* parse_primary(parser_t* parser);

static stmt_t * statement(parser_t * p_parser);
static stmt_t * declaration(parser_t * p_parser);
static stmt_t * parse_block_statement(parser_t * p_parser);
static stmt_t * parse_if_statement(parser_t * p_parser);
static stmt_t * parse_while_statement(parser_t * p_parser);
static stmt_t * parse_return_statement(parser_t * p_parser);

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

    // owns copy of token array, need to free
    parser_t * parser = memory_allocate(sizeof(parser_t));
    parser->tokens = memory_allocate(sizeof(dynamic_array_t));
    parser->tokens->data = memory_allocate(tokens->capacity);
    memory_copy(parser->tokens->data, tokens->data, tokens->capacity);
    parser->tokens->size = tokens->size;
    parser->tokens->capacity = tokens->capacity;

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
stmt_t * parser_parse_statement(parser_t * p_parser) {
    return declaration(p_parser);
}
dynamic_array_t * parse_statements(parser_t * p_parser) {
    dynamic_array_t * statements = create_array(sizeof(stmt_t*));
    while (parser_get_current_token_type(p_parser) != END_OF_FILE) {
        stmt_t * p_stmt = parser_parse_statement(p_parser);
        array_push(statements, &p_stmt, sizeof(stmt_t*));
    }
    return statements;
}
token_type_t parser_get_current_token_type(const parser_t * p_parser) {
    const token_t * p_token = (token_t*)p_parser->tokens->data + p_parser->current;
    return p_token->type;
}
// Private functions
static expr_t* parse_expression(parser_t* parser) {
    return parse_assignment(parser);
}
static expr_t * parse_assignment(parser_t * p_parser) {
    expr_t * p_expr = parse_logical_or(p_parser);
    if (token_match(p_parser, 1, EQUAL)) {

        expr_t * value = parse_assignment(p_parser);
        if (p_expr->type == EXPR_VARIABLE) {
            expr_t * new_expr = memory_allocate(sizeof(expr_t));
            new_expr->type = EXPR_ASSIGN;
            new_expr->as.assign_expr.target = p_expr;
            new_expr->as.assign_expr.value = value;
            return new_expr;
        }
        fprintf(stderr, "ParserError: Invalid assignment target.\n");
    }
    return p_expr;
}
static expr_t * parse_logical_or(parser_t * p_parser) {
    expr_t * p_expr = parse_logical_and(p_parser);

    while (token_match(p_parser, 1, OR)) {
        token_t * operator = memory_allocate(sizeof(token_t));
        if (!memory_copy(operator, token_previous_ptr(p_parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
        expr_t * right = parse_logical_and(p_parser);
        expr_t * new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_LOGICAL;
        new_expr->as.logical_expr.left = p_expr;
        new_expr->as.logical_expr.operator = operator;
        new_expr->as.logical_expr.right = right;

        p_expr = new_expr;
    }
    return p_expr;
}
static expr_t * parse_logical_and(parser_t * p_parser) {
    expr_t * p_expr = parse_equality(p_parser);

    while (token_match(p_parser, 1, AND)) {
        token_t * operator = memory_allocate(sizeof(token_t));
        if (!memory_copy(operator, token_previous_ptr(p_parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
        expr_t * right = parse_equality(p_parser);
        expr_t * new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_LOGICAL;
        new_expr->as.logical_expr.left = p_expr;
        new_expr->as.logical_expr.operator = operator;
        new_expr->as.logical_expr.right = right;

        p_expr = new_expr;
    }
    return p_expr;
}
static expr_t* parse_equality(parser_t* parser) {
    expr_t* expr = parse_comparison(parser);
    while (token_match(parser, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        token_t * operator = memory_allocate(sizeof(token_t));
        if (!memory_copy(operator, token_previous_ptr(parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
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
    expr_t* expr = parse_term(parser);
    while (token_match(parser, 4, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL)) {
        token_t * operator = memory_allocate(sizeof(token_t));
        if (!memory_copy(operator, token_previous_ptr(parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
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
    expr_t* expr = parse_factor(parser);
    while (token_match(parser, 2, MINUS, PLUS)) {
        token_t * operator = memory_allocate(sizeof(token_t));
        if (!memory_copy(operator, token_previous_ptr(parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
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
    expr_t* expr = parse_unary(parser);
    while (token_match(parser, 3, SLASH, STAR, PERCENTAGE)) {
        token_t * operator = memory_allocate(sizeof(token_t));
        if (!memory_copy(operator, token_previous_ptr(parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
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
    if (token_match(parser, 2, BANG, MINUS)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_unary(parser);
        const expr_unary_t unary_expr = { .operator = operator, .right = right };
        expr_t* new_expr = memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_UNARY;
        new_expr->as.unary_expr = unary_expr;
        return new_expr;
    }
    return parse_call(parser);
}
// function calling helping function
static expr_t * finish_call(parser_t * p_parser, expr_t * callee) {
    dynamic_array_t * arguments = create_array(sizeof(expr_t*));
    if (!token_check(p_parser, RIGHT_PAREN)) {
        do {
            expr_t * arg = parse_expression(p_parser);
            array_push(arguments, &arg, sizeof(expr_t*));
        } while (token_match(p_parser, 1, COMMA));
    }
    const token_t token = consume(p_parser, RIGHT_PAREN, "Expect ')' after arguments.");
    token_t * p_token = memory_allocate(sizeof(token_t));
    if (!memory_copy(p_token, &token, sizeof(token_t))) {
        fprintf(stderr, "ParserError: Memory copy failed\n");
        exit(EXIT_FAILURE);
    }
    expr_t * expr = memory_allocate(sizeof(expr_t));
    expr->type = EXPR_CALL;
    expr->as.call_expr.callee = callee;
    expr->as.call_expr.paren = p_token;
    expr->as.call_expr.arguments = arguments->data;
    size_t * count = memory_allocate(sizeof(size_t));
    *count = arguments->size / sizeof(expr_t*);
    expr->as.call_expr.count = count;
    return expr;
}
static expr_t * parse_call(parser_t * p_parser) {
    expr_t * expr = parse_primary(p_parser);

    if (token_match(p_parser, 1, LEFT_PAREN)) {
        expr = finish_call(p_parser, expr);
    }
    return expr;
}
static expr_t* parse_primary(parser_t* parser) {
    if (token_match(parser, 5, NUMBER, STRING, KW_TRUE, KW_FALSE, NIL)) {
        const token_t* number_token = token_previous_ptr(parser);
        expr_t* expr = memory_allocate(sizeof(expr_t));
        expr->type = EXPR_LITERAL;
        expr->as.literal_expr.kind = memory_allocate(sizeof(token_t));
        if (!memory_copy(expr->as.literal_expr.kind, number_token, sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
        }
        return expr;
    }
    if (token_match(parser, 1, IDENTIFIER)) {
        expr_t* expr = memory_allocate(sizeof(expr_t));
        expr->type = EXPR_VARIABLE;
        expr->as.variable_expr.name = memory_allocate(sizeof(token_t));
        if (!memory_copy(expr->as.variable_expr.name, token_previous_ptr(parser), sizeof(token_t))) {
            fprintf(stderr, "ParserError: Memory copy failed\n");
            exit(EXIT_FAILURE);
        }
        return expr;
    }
    if (token_match(parser, 1, LEFT_PAREN)) {
        expr_t* expr = parse_expression(parser);
        if (!token_match(parser, 1, RIGHT_PAREN)) {
            // Error: expected ')'
            token_t * test = (token_t*)parser->tokens->data + parser->current - 1;
            fprintf(stderr, "ParserError: Token: %s\tline:%d\n", test->lexeme, test->line);
            fprintf(stderr, "ParserError: Expected ')' after expression.\n");
            g_error_flag = true;
            return NULL;
        }
        expr_t* group = memory_allocate(sizeof(expr_t));
        group->type = EXPR_GROUPING;
        group->as.grouping_expr.expression = expr;
        return group;
    }
    // If none matched, error
    fprintf(stderr, "ParserError: Expected expression.\n");
    // TODO fix some other way
    g_error_flag = true; // sets global error flag
    return NULL;
}
static token_t token_advance(parser_t* parser) {
    if (!token_is_at_end(parser)) {
        parser->previous = (token_t*)parser->tokens->data + parser->current;
        parser->current++;
        parser->current_token = (token_t*)parser->tokens->data + parser->current;
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
static token_t consume(parser_t * p_parser, token_type_t type, const char * message) {
    if (token_check(p_parser, type)) {
        return token_advance(p_parser);
    }
    fprintf(stderr, "ParserError: %s\n", message);
    exit(EXIT_FAILURE);
}
static stmt_t * print_statement(parser_t * p_parser) {
    expr_t* expr = parse_expression(p_parser);
    consume(p_parser, SEMICOLON, "Expected ';' after value.");
    stmt_t * print_stmt = memory_allocate(sizeof(stmt_t));
    print_stmt->type = STMT_PRINT;
    print_stmt->as.print_stmt.expression = expr;
    return print_stmt;
}

static stmt_t * expression_statement(parser_t * p_parser) {
    expr_t* expr = parse_expression(p_parser);
    consume(p_parser, SEMICOLON, "Expected ';' after expression.");
    stmt_t * expr_stmt = memory_allocate(sizeof(stmt_t));
    expr_stmt->type = STMT_EXPRESSION;
    expr_stmt->as.expression_stmt.expression = expr;
    return expr_stmt;
}
static stmt_t * var_declaration(parser_t * p_parser) {
    // declaration     -> varDecl | statement ;

    const token_t token = consume(p_parser, IDENTIFIER, "Expect variable name.");
    expr_t * initializer = NULL;

    if (token_match(p_parser, 1, EQUAL)) {
        initializer = parse_expression(p_parser);
    }
    consume(p_parser, SEMICOLON, "Expected ';' after variable declaration.");
    stmt_t * var_decl_stmt = memory_allocate(sizeof(stmt_t));
    var_decl_stmt->type = STMT_VAR;
    var_decl_stmt->as.var_stmt.initializer = initializer;
    var_decl_stmt->as.var_stmt.name = memory_allocate(sizeof(token_t));
    if (!memory_copy(var_decl_stmt->as.var_stmt.name, &token, sizeof(token_t))) {
        fprintf(stderr, "ParserError: Memory copy failed\n");
        exit(EXIT_FAILURE);
    }
    return var_decl_stmt;
}
static stmt_t * fun_declaration(parser_t * p_parser, const char * kind) {
    char buf[256] = {0};
    snprintf(buf, 256, "Expect %s name.", kind);
    token_t * p_name = memory_allocate(sizeof(token_t));
    *p_name = consume(p_parser, IDENTIFIER, buf);

    snprintf(buf, 256, "Expect '(' after %s name.", kind);
    consume(p_parser, LEFT_PAREN, buf);
    dynamic_array_t * parameters = create_array(8 * sizeof(token_t*));
    if (!token_check(p_parser, RIGHT_PAREN)) {
        do {
            if (parameters->size >= 255 * sizeof(token_t*)) { // limited at 255 parameters
                fprintf(stderr, "ParseError: Can't have more than 255 parameters\n");
                exit(EXIT_FAILURE);
            }
            token_t * p_token = memory_allocate(sizeof(token_t));
            *p_token = consume(p_parser, IDENTIFIER, "Expected parameter name.");
            array_push(parameters, &p_token, sizeof(token_t*));
        } while (token_match(p_parser, 1, COMMA));
    }
    consume(p_parser, RIGHT_PAREN, "Expect ')' after parameters.");
    snprintf(buf, 256, "Expect '{' before %s body.", kind);
    consume(p_parser, LEFT_BRACE, buf);
    const stmt_t * body_stmt = parse_block_statement(p_parser);
    stmt_t * fun_decl_stmt = memory_allocate(sizeof(stmt_t));
    fun_decl_stmt->type = STMT_FUNCTION;
    fun_decl_stmt->as.function_stmt.name = p_name;

    size_t * p_n = memory_allocate(sizeof(size_t));
    *p_n = *body_stmt->as.block_stmt.count;
    fun_decl_stmt->as.function_stmt.body = memory_allocate(*p_n * sizeof(stmt_t*));
    for (size_t i = 0; i < *p_n; i++) {
        fun_decl_stmt->as.function_stmt.body[i] =
            body_stmt->as.block_stmt.statements[i];
    }
    fun_decl_stmt->as.function_stmt.count = p_n;

    // fun_decl_stmt->as.function_stmt.body = body_stmt->as.block_stmt.statements;
    // fun_decl_stmt->as.function_stmt.count = body_stmt->as.block_stmt.count;

    fun_decl_stmt->as.function_stmt.params = parameters->data;
    size_t * params_count = memory_allocate(sizeof(size_t));
    *params_count = parameters->size / sizeof(token_t*);
    fun_decl_stmt->as.function_stmt.params_count = params_count;

    return fun_decl_stmt;
}
static stmt_t * class_declaration(parser_t * p_parser) {
    token_t * p_name = memory_allocate(sizeof(token_t));
    *p_name = consume(p_parser, IDENTIFIER, "Expect class name.");
    consume(p_parser, LEFT_BRACE, "Expected '{' before class body.");

    dynamic_array_t * methods = create_array(2 * sizeof(stmt_t*));
    size_t n = 0;
    while (!token_check(p_parser, RIGHT_BRACE) && !token_is_at_end(p_parser)) {
        stmt_t * f = fun_declaration(p_parser, "method");
        array_push(methods, &f,
            sizeof(stmt_t*));
        n++;
    }
    consume(p_parser, RIGHT_BRACE, "Expected '}' after class body.");
    stmt_t * class_decl_stmt = memory_allocate(sizeof(stmt_t));
    class_decl_stmt->type = STMT_CLASS;
    class_decl_stmt->as.class_stmt.name = p_name;
    class_decl_stmt->as.class_stmt.superclass = NULL; // no inheritance for now
    class_decl_stmt->as.class_stmt.methods = methods->data;
    class_decl_stmt->as.class_stmt.methods_count = n;

    return class_decl_stmt;
}
static stmt_t * declaration(parser_t * p_parser) {
    if (token_match(p_parser, 1, CLASS)) {
        return class_declaration(p_parser);
    }
    if (token_match(p_parser, 1, FUN)) {
        return fun_declaration(p_parser, "function");
    }
    if (token_match(p_parser, 1, VAR)) {
        return var_declaration(p_parser);
    }
    return statement(p_parser);
}
static stmt_t * statement(parser_t * p_parser) {
    if (token_match(p_parser, 1, IF)) {
        return parse_if_statement(p_parser);
    }
    if (token_match(p_parser, 1, WHILE)) {
        return parse_while_statement(p_parser);
    }
    if (token_match(p_parser, 1, PRINT)) {
        return print_statement(p_parser);
    }
    if (token_match(p_parser, 1, RETURN)) {
        return parse_return_statement(p_parser);
    }
    if (token_match(p_parser, 1, LEFT_BRACE)) {
        return parse_block_statement(p_parser);
    }
    return expression_statement(p_parser);
}
static stmt_t * parse_block_statement(parser_t * p_parser) {
    stmt_t * p_stmt = memory_allocate(sizeof(stmt_t));
    p_stmt->type = STMT_BLOCK;
    p_stmt->as.block_stmt.count = memory_allocate(sizeof(size_t));
    p_stmt->as.block_stmt.count[0] = 0;
    dynamic_array_t * statements = create_array(8 * sizeof(stmt_t*));
    while (!token_check(p_parser, RIGHT_BRACE) && !token_is_at_end(p_parser)) {
        stmt_t * stmt = declaration(p_parser);
        array_push(statements, &stmt, sizeof(stmt_t*));
    }
    consume(p_parser, RIGHT_BRACE, "Expected '}' after block.");

    *p_stmt->as.block_stmt.count = statements->size / sizeof(stmt_t*);
    p_stmt->as.block_stmt.statements = memory_allocate(statements->size);
    memory_copy(p_stmt->as.block_stmt.statements, statements->data, statements->size);

    array_free(statements);

    return p_stmt;
}
static stmt_t * parse_if_statement(parser_t * p_parser) {
    stmt_t * if_stmt = memory_allocate(sizeof(stmt_t));
    if_stmt->type = STMT_IF;

    consume(p_parser, LEFT_PAREN, "Expected '(' after 'if'.");
    expr_t * condition = parse_expression(p_parser);
    consume(p_parser, RIGHT_PAREN, "Expected ')' after if condition.");

    stmt_t * then_branch = statement(p_parser);
    stmt_t * else_branch = NULL;
    if (token_match(p_parser, 1, ELSE)) {
        else_branch = statement(p_parser);
    }
    if_stmt->as.if_stmt.condition = condition;
    if_stmt->as.if_stmt.then_branch = then_branch;
    if_stmt->as.if_stmt.else_branch = else_branch;
    return if_stmt;
}
static stmt_t * parse_while_statement(parser_t * p_parser) {
    stmt_t * while_stmt = memory_allocate(sizeof(stmt_t));
    while_stmt->type = STMT_WHILE;

    consume(p_parser, LEFT_PAREN, "Expected '(' after 'while'.");
    expr_t * condition = parse_expression(p_parser);
    consume(p_parser, RIGHT_PAREN, "Expected ')' after while condition.");

    stmt_t * body = statement(p_parser);

    while_stmt->as.while_stmt.condition = condition;
    while_stmt->as.while_stmt.body = body;

    return while_stmt;
}
static stmt_t * parse_return_statement(parser_t * p_parser) {
    stmt_t * return_stmt = memory_allocate(sizeof(stmt_t));
    return_stmt->type = STMT_RETURN;
    token_t * p_keyword = token_previous_ptr(p_parser);
    expr_t * p_expression = NULL;
    if (!token_check(p_parser, ';')) {
        p_expression = parse_expression(p_parser);
    }
    consume(p_parser, SEMICOLON, "Expected ';' after return value.");
    return_stmt->as.return_stmt.value = p_expression;
    return_stmt->as.return_stmt.keyword = p_keyword;
    return return_stmt;
}
