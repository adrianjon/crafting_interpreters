//
// Created by adrian on 2025-10-11.
//

#include "parser.h"

#include <stdarg.h>

#include "expr.h"
#include "stmt.h"

static expr_t * parse_expression(parser_t * p_parser);
static expr_t * assignment(parser_t * p_parser);
static expr_t * logical_or(parser_t * p_parser);
static expr_t * logical_and(parser_t * p_parser);
static expr_t * equality(parser_t * p_parser);
static expr_t * comparison(parser_t * p_parser);
static expr_t * term(parser_t * p_parser);
static expr_t * factor(parser_t * p_parser);
static expr_t * unary(parser_t * p_parser);
static expr_t * call(parser_t * p_parser);
static expr_t * primary(parser_t * p_parser);

static stmt_t * parse_statement(parser_t * p_parser);
static stmt_t * declaration(parser_t * p_parser);
static stmt_t * class_declaration(parser_t * p_parser);
static stmt_t * function_declaration(parser_t * p_parser);
static stmt_t * variable_declaration(parser_t * p_parser);
static stmt_t * statement(parser_t * p_parser);
static stmt_t * expression_statement(parser_t * p_parser);
static stmt_t * for_statement(parser_t * p_parser);
static stmt_t * if_statement(parser_t * p_parser);
static stmt_t * print_statement(parser_t * p_parser);
static stmt_t * return_statement(parser_t * p_parser);
static stmt_t * while_statement(parser_t * p_parser);
static stmt_t * block_statement(parser_t * p_parser);

// helpers
static token_t consume(parser_t * p_parser, token_type_t type, char const * p_msg);
static bool token_match(parser_t * p_parser, int count, ...);
static bool token_is_at_end(parser_t const * p_parser);
static bool token_check(parser_t const * p_parser, token_type_t type);

static expr_t * finish_call(parser_t * p_parser, expr_t * callee);

static void free_expr(void ** pp_expr);
static void free_stmt(void ** pp_stmt);

list_t parse(parser_t * p_parser) {
    if (!p_parser || !p_parser->tokens.data) {
        fprintf(stderr, "Expected at least one token\n");
        exit(EXIT_FAILURE);
    }
    p_parser->had_error = false;
    p_parser->p_previous = NULL;
    p_parser->current_index = 0;
    p_parser->p_current = p_parser->tokens.data[p_parser->current_index];
    list_t statements = { .free_fn = free_stmt};

    // First part of the grammar
    // program          -> declaration* END_OF_FILE ;
    while (p_parser->p_current->type != END_OF_FILE ) {
        stmt_t * p_stmt = parse_statement(p_parser);
        list_add(&statements, p_stmt);
    }
    return statements;
}


// statement            -> declaration
static stmt_t * parse_statement(parser_t * p_parser) {
    return declaration(p_parser);
}

// expression           -> assignment ;
static expr_t * parse_expression(parser_t * p_parser) {
    return assignment(p_parser);
}

// assignment           -> ( call "." )? IDENTIFIER "=" assignment
//                          | logical_or ;
static expr_t * assignment(parser_t * p_parser) {
    expr_t * p_expr = logical_or(p_parser);
    if (token_match(p_parser, 1, EQUAL)) {
        //token_t * equals = p_parser->p_previous;
        expr_t * value = assignment(p_parser);

        expr_t * assign = malloc(sizeof(expr_t));
        if (p_expr->type == EXPR_VARIABLE) {
            token_t * name = copy_token(p_expr->as.variable_expr.name);
            free_expr((void**)&p_expr); // discard expr, only using copy of its name
            assign->type = EXPR_ASSIGN;
            assign->as.assign_expr.target = name;
            assign->as.assign_expr.value = value;
            return assign;
        }
        if (p_expr->type == EXPR_GET) {
            assign->type = EXPR_SET;
            assign->as.set_expr.object = p_expr; // TODO .object should probaby be object_t*
            assign->as.set_expr.name = p_expr->as.get_expr.name;
            assign->as.set_expr.value = value;
            return assign;
        }
        fprintf(stderr, "ParserError: Invalid assignment target.\n"); // no throw
    }
    return p_expr;
}

// logical_or           -> logical_and ( "or" logical_and )* ;
static expr_t * logical_or(parser_t * p_parser) {
    expr_t * p_expr = logical_and(p_parser);

    while (token_match(p_parser, 1, OR)) {
        token_t * op = copy_token(p_parser->p_previous);
        expr_t * right = logical_and(p_parser);
        expr_t * logical_or = malloc(sizeof(expr_t));
        if (!logical_or) exit(EXIT_FAILURE);
        logical_or->type = EXPR_LOGICAL;
        logical_or->as.logical_expr.left = p_expr;
        logical_or->as.logical_expr.right = right;
        logical_or->as.logical_expr.operator = op;

        p_expr = logical_or;
    }
    return p_expr;
}

// logical_and          -> equality ( "and" equality )* ;
static expr_t * logical_and(parser_t * p_parser) {
    expr_t * p_expr = equality(p_parser);

    while (token_match(p_parser, 1, AND)) {
        token_t * op = copy_token(p_parser->p_previous);
        expr_t * right = equality(p_parser);
        expr_t * logical_and = malloc(sizeof(expr_t));
        if (!logical_and) exit(EXIT_FAILURE);
        logical_and->type = EXPR_LOGICAL;
        logical_and->as.logical_expr.left = p_expr;
        logical_and->as.logical_expr.right = right;
        logical_and->as.logical_expr.operator = op;

        p_expr = logical_and;
    }
    return p_expr;
}

// equality             -> comparison ( ( "!=" | "==" ) comparison )* ;
static expr_t * equality(parser_t * p_parser) {
    expr_t * expr = comparison(p_parser);
    while (token_match(p_parser, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        token_t * op = copy_token(p_parser->p_previous);
        expr_t * right = comparison(p_parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = op, .right = right };
        expr_t * new_expr = malloc(sizeof(expr_t));
        if (!new_expr) exit(EXIT_FAILURE);
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}

// comparison           -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
static expr_t * comparison(parser_t * p_parser) {
    expr_t* expr = term(p_parser);
    while (token_match(p_parser, 4, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL)) {
        token_t * op = copy_token(p_parser->p_previous);
        expr_t * right = term(p_parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = op, .right = right };
        expr_t * new_expr = malloc(sizeof(expr_t));
        if (!new_expr) exit(EXIT_FAILURE);
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}

// term                 -> factor ( ( "-" | "+" ) factor )* ;
static expr_t * term(parser_t * p_parser) {
    expr_t* expr = factor(p_parser);
    while (token_match(p_parser, 2, MINUS, PLUS)) {
        token_t * op = copy_token(p_parser->p_previous);
        expr_t * right = factor(p_parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = op, .right = right };
        expr_t * new_expr = malloc(sizeof(expr_t));
        if (!new_expr) exit(EXIT_FAILURE);
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}

// factor               -> unary ( ( "/" | "*" ) unary )* ;
static expr_t * factor(parser_t * p_parser) {
    expr_t* expr = unary(p_parser);
    while (token_match(p_parser, 3, SLASH, STAR, PERCENTAGE)) {
        token_t * op = copy_token(p_parser->p_previous);
        expr_t * right = unary(p_parser);
        const expr_binary_t binary_expr = { .left = expr, .operator = op, .right = right };
        expr_t * new_expr = malloc(sizeof(expr_t));
        if (!new_expr) exit(EXIT_FAILURE);
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}

// unary                -> ( "!" | "-" ) unary | call ;
static expr_t * unary(parser_t * p_parser) {
    if (token_match(p_parser, 2, BANG, MINUS)) {
        token_t* op = copy_token(p_parser->p_previous);
        expr_t * right = unary(p_parser);
        const expr_unary_t unary_expr = { .operator = op, .right = right };
        expr_t * new_expr = malloc(sizeof(expr_t));
        if (!new_expr) exit(EXIT_FAILURE);
        new_expr->type = EXPR_UNARY;
        new_expr->as.unary_expr = unary_expr;
        return new_expr;
    }
    return call(p_parser);
}

// call                 -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
static expr_t * call(parser_t * p_parser) {
    expr_t * expr = primary(p_parser);
    while (true) {
        if (token_match(p_parser, 1, LEFT_PAREN)) {
            expr = finish_call(p_parser, expr);
        } else if (token_match(p_parser, 1, DOT)) {
            token_t name = consume(p_parser, IDENTIFIER,
                "Expected property name after '.'.");
            expr_t * temp = expr;
            expr = malloc(sizeof(expr_t));
            if (!expr) exit(EXIT_FAILURE);
            expr->type = EXPR_GET;
            expr->as.get_expr.name = copy_token(&name);
            expr->as.get_expr.object = temp;
        } else {
            break;
        }
    }
    return expr;
}

// primary              -> "true" | "false" | "nil" | "this" | NUMBER | STRING
//                          | IDENTIFIER | "(" expression ")" | "super" "." IDENTIFIER ;
static expr_t * primary(parser_t * p_parser) {
    if (token_match(p_parser, 5, NUMBER, STRING, KW_TRUE, KW_FALSE, NIL)) {
        token_t* number_token = copy_token(p_parser->p_previous);
        expr_t* expr = malloc(sizeof(expr_t));
        if (!expr) exit(EXIT_FAILURE);
        expr->type = EXPR_LITERAL;
        expr->as.literal_expr.kind = number_token;
        return expr;
    }
    if (token_match(p_parser, 1, IDENTIFIER)) {
        expr_t* expr = malloc(sizeof(expr_t));
        if (!expr) exit(EXIT_FAILURE);
        expr->type = EXPR_VARIABLE;
        expr->as.variable_expr.name = copy_token(p_parser->p_previous);
        return expr;
    }
    if (token_match(p_parser, 1, LEFT_PAREN)) {
        expr_t* expr = parse_expression(p_parser);
        if (!token_match(p_parser, 1, RIGHT_PAREN)) {
            // Error: expected ')'
            fprintf(stderr, "ParserError: Expected ')' after expression.\n");
            return NULL;
        }
        expr_t* group = malloc(sizeof(expr_t));
        if (!expr) exit(EXIT_FAILURE);
        group->type = EXPR_GROUPING;
        group->as.grouping_expr.expression = expr;
        return group;
    }
    fprintf(stderr, "ParserError: Expected expression.\n");
    exit(EXIT_FAILURE);
}

//  declaration         -> class_declaration
//                          | function declaration
//                          | variable_declaration
//                          | statement ;
static stmt_t * declaration(parser_t * p_parser) {
    if (p_parser->p_current->type == CLASS)
        return class_declaration(p_parser);
    if (p_parser->p_current->type == FUN)
        return function_declaration(p_parser);
    if (p_parser->p_current->type == VAR)
        return variable_declaration(p_parser);
    return statement(p_parser);
}

static stmt_t * class_declaration(parser_t * p_parser)  {
    p_parser->had_error = true;
    fprintf(stderr, "Not implemented\n");
    exit(EXIT_FAILURE);
}
static stmt_t * function_declaration(parser_t * p_parser)  {
    p_parser->had_error = true;
    fprintf(stderr, "Not implemented\n");
    exit(EXIT_FAILURE);
}
static stmt_t * variable_declaration(parser_t * p_parser)  {
    p_parser->had_error = true;
    fprintf(stderr, "Not implemented\n");
    exit(EXIT_FAILURE);
}

// statement            -> expression_statement
//                          | for_statement
//                          | if_statement
//                          | print_statement
//                          | return_statement
//                          | while_statement
//                          | block_statement ;
static stmt_t * statement(parser_t * p_parser)  {
    if (token_match(p_parser, 1, FOR))
        return for_statement(p_parser);
    if (token_match(p_parser, 1, IF))
        return if_statement(p_parser);
    if (token_match(p_parser, 1, PRINT))
        return print_statement(p_parser);
    if (token_match(p_parser, 1, RETURN))
        return return_statement(p_parser);
    if (token_match(p_parser, 1, WHILE))
        return while_statement(p_parser);
    if (token_match(p_parser, 1, LEFT_BRACE))
        return block_statement(p_parser);
    return expression_statement(p_parser);
}

// expression_statement -> expression ";" ;
static stmt_t * expression_statement(parser_t * p_parser) {
    expr_t * p_expr = parse_expression(p_parser);
    consume(p_parser, SEMICOLON, "Expected ';' after expression.");
    stmt_t * expr_stmt = malloc(sizeof(stmt_t));
    expr_stmt->type = STMT_EXPRESSION;
    expr_stmt->as.expression_stmt.expression = p_expr;
    return expr_stmt;
}

// for_statement        -> "for" "(" ( variable_declaration | expression_statement | ";" )
//                                  expression? ";" expression? ")" statement ;
static stmt_t * for_statement(parser_t * p_parser) {
    consume(p_parser, LEFT_PAREN, "Expected '(' after 'for'.");
    stmt_t * p_initializer;
    if (token_match(p_parser, 1, SEMICOLON)) {
        p_initializer = NULL;
    } else if (token_match(p_parser, 1, VAR)) {
        p_initializer = variable_declaration(p_parser);
    } else {
        p_initializer = expression_statement(p_parser);
    }

    expr_t * p_condition = NULL;
    if (!token_check(p_parser, SEMICOLON))
        p_condition = parse_expression(p_parser);
    consume(p_parser, SEMICOLON, "Expected ';' after loop condition.");

    expr_t * p_increment = NULL;
    if (!token_check(p_parser, RIGHT_PAREN)) {
        p_increment = parse_expression(p_parser);
    }
    consume(p_parser, RIGHT_PAREN, "Expected ')' after for clauses.");

    stmt_t * p_body = statement(p_parser);
    stmt_t * p_new_body = NULL;
    if (p_increment != NULL) {
        p_new_body = malloc(sizeof(stmt_t));
        if (!p_new_body) exit(EXIT_FAILURE);
        p_new_body->type = STMT_BLOCK;
        p_new_body->as.block_stmt.count = 2;
        p_new_body->as.block_stmt.statements = malloc(sizeof(stmt_t*) * 2);
        p_new_body->as.block_stmt.statements[0] = p_body;
        stmt_t * p_stmt_expr_temp = malloc(sizeof(stmt_t));
        if (!p_stmt_expr_temp) exit(EXIT_FAILURE);
        p_stmt_expr_temp->type = STMT_EXPRESSION;
        p_stmt_expr_temp->as.expression_stmt.expression = p_increment;
        p_new_body->as.block_stmt.statements[1] = p_stmt_expr_temp;
        p_body = p_new_body;
    }

    if (p_condition == NULL) {
        p_condition = malloc(sizeof(expr_t));
        if (!p_condition) exit(EXIT_FAILURE);
        p_condition->type = EXPR_LITERAL;
        token_t * p_true = new_token(KW_TRUE, "true", p_parser->p_previous->line);
        p_condition->as.literal_expr.kind = p_true;
    }
    stmt_t * p_new_new_body = malloc(sizeof(stmt_t));
    if (!p_new_new_body) exit(EXIT_FAILURE);
    p_new_new_body->type = STMT_WHILE;
    p_new_new_body->as.while_stmt.condition = p_condition;
    p_new_new_body->as.while_stmt.body = p_body;
    p_body = p_new_new_body;

    if (p_initializer != NULL) {
        stmt_t * p_new_new_new_body = malloc(sizeof(stmt_t));
        if (!p_new_new_new_body) exit(EXIT_FAILURE);
        p_new_new_new_body->type = STMT_BLOCK;
        p_new_new_new_body->as.block_stmt.count = 2;
        p_new_new_new_body->as.block_stmt.statements = malloc(sizeof(stmt_t*) * 2);
        if (!p_new_new_new_body->as.block_stmt.statements)
            exit(EXIT_FAILURE);
        p_new_new_new_body->as.block_stmt.statements[0] = p_initializer;
        p_new_new_new_body->as.block_stmt.statements[1] = p_body;
    }
    return p_body;
}

// if_statement         -> "if" "(" expression ")" statement
//                                  ( "else" statement )? ;
static stmt_t * if_statement(parser_t * p_parser) {
    stmt_t * if_stmt = malloc(sizeof(stmt_t));
    if (!if_stmt) exit(EXIT_FAILURE);
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

// print_statement      -> "print" expression ";" ;
static stmt_t * print_statement(parser_t * p_parser) {
    expr_t* expr = parse_expression(p_parser);
    consume(p_parser, SEMICOLON, "Expected ';' after value.");
    stmt_t * print_stmt = malloc(sizeof(stmt_t));
    if (!print_stmt) exit(EXIT_FAILURE);
    print_stmt->type = STMT_PRINT;
    print_stmt->as.print_stmt.expression = expr;
    return print_stmt;
}

// return_statement     -> "return" expression? ";" ;
static stmt_t * return_statement(parser_t * p_parser) {
    stmt_t * return_stmt = malloc(sizeof(stmt_t));
    if (!return_stmt) exit(EXIT_FAILURE);
    return_stmt->type = STMT_RETURN;
    token_t * p_keyword = copy_token(p_parser->p_previous);
    expr_t * p_expression = NULL;
    if (!token_check(p_parser, ';')) {
        p_expression = parse_expression(p_parser);
    }
    consume(p_parser, SEMICOLON, "Expected ';' after return value.");
    return_stmt->as.return_stmt.value = p_expression;
    return_stmt->as.return_stmt.keyword = p_keyword;
    return return_stmt;
}

// while_statement      -> "while" "(" expression ")" statement ;
static stmt_t * while_statement(parser_t * p_parser) {
    consume(p_parser, LEFT_PAREN, "Expected '(' after 'while'.");
    expr_t * p_condition = parse_expression(p_parser);
    consume(p_parser, RIGHT_PAREN, "Expected ')' after condition.");
    stmt_t * p_body = statement(p_parser);

    stmt_t * p_while_stmt = malloc(sizeof(stmt_t));
    if (!p_while_stmt) exit(EXIT_FAILURE);
    p_while_stmt->type = STMT_WHILE;
    p_while_stmt->as.while_stmt.condition = p_condition;
    p_while_stmt->as.while_stmt.body = p_body;
    return p_while_stmt;
}

// block_statement      -> "{" declaration* "}" ;
static stmt_t * block_statement(parser_t * p_parser) {

    stmt_t * p_stmt = malloc(sizeof(stmt_t));
    if (!p_stmt) exit(EXIT_FAILURE);
    p_stmt->type = STMT_BLOCK;

    size_t capacity = 1;
    p_stmt->as.block_stmt.count = 0;
    p_stmt->as.block_stmt.statements = malloc(sizeof(stmt_t*) * capacity);
    if (!p_stmt->as.block_stmt.statements) exit(EXIT_FAILURE);
    while (!token_check(p_parser, RIGHT_BRACE) && !token_is_at_end(p_parser)) {
        stmt_t * p = declaration(p_parser);
        if (p_stmt->as.block_stmt.count < capacity) {
            p_stmt->as.block_stmt.statements[p_stmt->as.block_stmt.count++] = p;
        } else if (p_stmt->as.block_stmt.count == capacity) {
            capacity *= 2;
            void * p_temp = realloc(p_stmt->as.block_stmt.statements, sizeof(stmt_t*) * capacity);
            if (!p_temp) exit(EXIT_FAILURE);
            p_stmt->as.block_stmt.statements = p_temp;
            p_stmt->as.block_stmt.statements[p_stmt->as.block_stmt.count++] = p;
        } else {
            fprintf(stderr, "Array size larger than capacity.\n");
            exit(EXIT_FAILURE);
        }
    }
    consume(p_parser, RIGHT_BRACE, "Expected '}' after block.");
    return p_stmt;
}


static token_t consume(parser_t * p_parser, token_type_t const type, char const * p_msg) {
    if (p_parser->p_current->type == type) {
        p_parser->p_previous = p_parser->p_current;
        p_parser->p_current = p_parser->tokens.data[++p_parser->current_index];
        return *p_parser->p_current;
    }
    fprintf(stderr, "ParserError: %s\n", p_msg);
    exit(EXIT_FAILURE);
}
static bool token_is_at_end(parser_t const * p_parser) {
    return p_parser->p_current->type == END_OF_FILE;
}
static bool token_check(parser_t const * p_parser, token_type_t const type) {
    if (token_is_at_end(p_parser)) return false;
    return p_parser->p_current->type == type;
}
static bool token_match(parser_t* p_parser, const int count, ...) {
    va_list args;
    va_start(args, count);
    bool matched = false;
    for (int i = 0; i < count; i++) {
        const token_type_t expected = va_arg(args, token_type_t);
        if (token_check(p_parser, expected)) {
            p_parser->p_previous = p_parser->p_current;
            p_parser->p_current = p_parser->tokens.data[++p_parser->current_index];
            matched = true;
            break;
        }
    }
    va_end(args);
    return matched;
}

static expr_t * finish_call(parser_t * p_parser, expr_t * callee) {
    expr_t ** pp_args = NULL;

    size_t count = 0;
    if (!token_check(p_parser, RIGHT_PAREN)) {
        size_t capacity = 1;
        pp_args = malloc(sizeof(expr_t*) * capacity);
        if (!pp_args) exit(EXIT_FAILURE);
        do {
            expr_t * p_arg = parse_expression(p_parser);
            if (count < capacity)
                pp_args[count++] = p_arg;
            else if (count == capacity) {
                capacity *= 2;
                void * p_temp = realloc(pp_args, sizeof(expr_t*) * capacity);
                if (!p_temp) exit(EXIT_FAILURE);
                pp_args = p_temp;
                pp_args[count++] = p_arg;
            } else {
                fprintf(stderr, "Array size larger than capacity.\n");
                exit(EXIT_FAILURE);
            }
        } while (token_match(p_parser, 1, COMMA));
    }
    const token_t token = consume(p_parser, RIGHT_PAREN, "Expected ')' after arguments.");
    token_t * p_token = copy_token(&token);
    expr_t * p_expr = malloc(sizeof(expr_t));
    if (!p_expr) exit(EXIT_FAILURE);
    p_expr->type = EXPR_CALL;
    p_expr->as.call_expr.callee = callee;
    p_expr->as.call_expr.paren = p_token;

    p_expr->as.call_expr.count = count;
    p_expr->as.call_expr.arguments = pp_args;
    return p_expr;
}

static void free_expr(void ** pp_expr) {
    if (!pp_expr || !*pp_expr) exit(EXIT_FAILURE);
    expr_t * p_expr = *pp_expr;
     switch (p_expr->type) {
         case EXPR_ASSIGN:
             free_expr((void**)&p_expr->as.assign_expr.value);
             token_free((void**)&p_expr->as.assign_expr.target);
             break;
         case EXPR_BINARY:
             free_expr((void**)&p_expr->as.binary_expr.left);
             token_free((void**)&p_expr->as.binary_expr.operator);
             free_expr((void**)&p_expr->as.binary_expr.right);
             break;
         case EXPR_CALL:
             free_expr((void**)&p_expr->as.call_expr.callee);
             token_free((void**)&p_expr->as.call_expr.paren);
             for (size_t i = 0; i < p_expr->as.call_expr.count; i++) {
                 free_expr((void**)&p_expr->as.call_expr.arguments[i]);
             }
             free(p_expr->as.call_expr.arguments);
         case EXPR_GET:
             free_expr((void**)&p_expr->as.get_expr.object);
             token_free((void**)&p_expr->as.get_expr.name);
             break;
         case EXPR_GROUPING:
             free_expr((void**)&p_expr->as.grouping_expr.expression);
             break;
         case EXPR_LITERAL:
             token_free((void**)&p_expr->as.literal_expr.kind);
             break;
         case EXPR_LOGICAL:
             free_expr((void**)&p_expr->as.logical_expr.left);
             token_free((void**)&p_expr->as.logical_expr.operator);
             free_expr((void**)&p_expr->as.logical_expr.right);
             break;
         case EXPR_SET:
             free_expr((void**)&p_expr->as.set_expr.object);
             token_free((void**)&p_expr->as.set_expr.name);
             free_expr((void**)&p_expr->as.set_expr.value);
             break;
         case EXPR_SUPER:
             token_free((void**)&p_expr->as.super_expr.keyword);
             token_free((void**)&p_expr->as.super_expr.method);
             break;
         case EXPR_THIS:
             token_free((void**)&p_expr->as.this_expr.keyword);
             break;
         case EXPR_UNARY:
             token_free((void**)&p_expr->as.unary_expr.operator);
             free_expr((void**)&p_expr->as.unary_expr.right);
             break;
         case EXPR_VARIABLE:
             token_free((void**)&p_expr->as.variable_expr.name);
             break;
         default:
             break;
    }
    free(p_expr);
}
static void free_stmt(void ** pp_stmt) {
    if (!pp_stmt || !*pp_stmt) exit(EXIT_FAILURE);
    stmt_t * p_stmt = *pp_stmt;
    switch (p_stmt->type) {
        case STMT_BLOCK:
            for (size_t i = 0; i < p_stmt->as.block_stmt.count; i++) {
                free_stmt((void**)&p_stmt->as.block_stmt.statements[i]);
            }
            free(p_stmt->as.block_stmt.statements);
            break;
        case STMT_FUNCTION:
            token_free((void**)&p_stmt->as.function_stmt.name);
            for (size_t i = 0; i < p_stmt->as.function_stmt.params_count; i++) {
                token_free((void**)&p_stmt->as.function_stmt.params[i]);
            }
            free(p_stmt->as.function_stmt.params);
            for (size_t i = 0; i < p_stmt->as.function_stmt.count; i++) {
                free_stmt((void**)&p_stmt->as.function_stmt.body[i]);
            }
            free(p_stmt->as.function_stmt.body);
            break;
        case STMT_CLASS:
            token_free((void**)&p_stmt->as.class_stmt.name);
            for (size_t i = 0; i < p_stmt->as.class_stmt.superclass_count; i++) {
                free_expr((void**)&p_stmt->as.class_stmt.superclass[i]);
            }
            free(p_stmt->as.class_stmt.superclass);
            for (size_t i = 0; i < p_stmt->as.class_stmt.methods_count; i++) {
                free_stmt((void**)&p_stmt->as.class_stmt.methods[i]);
            }
            free(p_stmt->as.class_stmt.methods);
            break;
        case STMT_EXPRESSION:
            free_expr((void**)&p_stmt->as.expression_stmt.expression);
            break;
        case STMT_IF:
            free_expr((void**)&p_stmt->as.if_stmt.condition);
            free_stmt((void**)&p_stmt->as.if_stmt.then_branch);
            free_stmt((void**)&p_stmt->as.if_stmt.else_branch);
            break;
        case STMT_PRINT:
            free_expr((void**)&p_stmt->as.print_stmt.expression);
            break;
        case STMT_RETURN:
            token_free((void**)&p_stmt->as.return_stmt.keyword);
            free_expr((void**)&p_stmt->as.return_stmt.value);
            break;
        case STMT_VAR:
            free_expr((void**)&p_stmt->as.var_stmt.initializer);
            token_free((void**)&p_stmt->as.var_stmt.name);
            break;
        case STMT_WHILE:
            free_expr((void**)&p_stmt->as.while_stmt.condition);
            free_stmt((void**)&p_stmt->as.while_stmt.body);
            break;
    }
    free(p_stmt);
}
