

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../extra/Arrays.h"
#include "../extra/Memory.h"
#include "Scanner.h"
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
#include "AstPrinter.h"

#include "Parser.h"

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

const char* expr_type_names[] = {
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

typedef struct parser parser_t;
struct parser {
    dynamic_array_t * tokens;
    size_t current;
};
expr_t* parse_expression(parser_t* parser);
expr_t* parse_equality(parser_t* parser);
expr_t* parse_comparison(parser_t* parser);
expr_t* parse_term(parser_t* parser);
expr_t* parse_factor(parser_t* parser);
expr_t* parse_unary(parser_t* parser);
expr_t* parse_primary(parser_t* parser);

token_t token_advance(parser_t* parser);
token_t token_peek(parser_t* parser);
bool token_match(parser_t* parser, int count, ...);
bool token_check(parser_t* parser, token_type_t expected);
bool token_is_at_end(parser_t* parser);
token_t token_previous(parser_t* parser);

int main(void) {

    dynamic_array_t tokens;
    parser_t parser = { .tokens = &tokens, .current = 0 };
    scanner_main(&tokens);
    // while(!token_is_at_end(&parser)) {
    //     //expr_t* expr = parse_expression(&parser);
    //     token_t token = token_advance(&parser);
    //     printf("Token: %s at line %d", token_type_names[token.type], token.line);
    //     if (token.lexeme) {
    //         printf(" with lexeme '%s'", token.lexeme);
    //     }
    //     printf("\n");
    // }
    print_tokens(&tokens);
    expr_t* expr = parse_expression(&parser);
    if (!expr) {
        printf("Failed to parse expression\n");
        return 1;
    }
    printf("Parsed expression of type %s\n", expr_type_names[expr->type]);
    
    string_builder_t sb = create_string_builder();
    ast_printer_t printer = {0};
    ast_printer_init(&printer, &sb);

    char* result = ast_printer_print_expr(&printer, expr);
    printf("AST Printer Result: %s\n", result);
    printf("String Builder Content: %s\n", sb.buffer);
    memory_free(&result);

    return 0;
}

token_t token_advance(parser_t* parser) {
    if (!token_is_at_end(parser)) {
        parser->current++;
    }
    return token_previous(parser);
}
token_t token_peek(parser_t* parser) {
    return *(token_t*)array_get(parser->tokens, parser->current * sizeof(token_t));
}
bool token_match(parser_t* parser, int count, ...) {
    va_list args;
    va_start(args, count);
    bool matched = false;
    for (int i = 0; i < count; i++) {
        token_type_t expected = va_arg(args, token_type_t);
        if (token_check(parser, expected)) {
            token_advance(parser);
            matched = true;
            break;
        }
    }
    va_end(args);
    return matched;
}
bool token_check(parser_t* parser, token_type_t expected) {
    if (token_is_at_end(parser)) return false;
    return token_peek(parser).type == expected;
}
bool token_is_at_end(parser_t* parser) {
    return token_peek(parser).type == EOF_;
}
token_t token_previous(parser_t* parser) {
    if (parser->current == 0) return (token_t){ .type = EOF_ };
    return *(token_t*)array_get(parser->tokens, (parser->current - 1) * sizeof(token_t));
}
token_t* token_previous_ptr(parser_t* parser) {
    if (parser->current == 0) return NULL;
    return (token_t*)array_get(parser->tokens, (parser->current - 1) * sizeof(token_t));
}
expr_t* parse_expression(parser_t* parser) {
    return parse_equality(parser);
}
expr_t* parse_equality(parser_t* parser) {
    expr_t* expr = parse_comparison(parser);
    while (token_match(parser, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        token_t* operator = token_previous_ptr(parser);
        expr_t* right = parse_comparison(parser);
        expr_binary_t binary_expr = { .left = expr, .operator = operator, .right = right };
        expr_t* new_expr = (expr_t*)memory_allocate(sizeof(expr_t));
        new_expr->type = EXPR_BINARY;
        new_expr->as.binary_expr = binary_expr;
        expr = new_expr;
    }
    return expr;
}
expr_t* parse_comparison(parser_t* parser) {
    // unimplemented
    return parse_term(parser);
}
expr_t* parse_term(parser_t* parser) {
    // unimplemented
    return parse_factor(parser);
}
expr_t* parse_factor(parser_t* parser) {
    // unimplemented
    return parse_unary(parser);
}
expr_t* parse_unary(parser_t* parser) {
    // unimplemented
    return parse_primary(parser);
}

// helpers move somewhere else
static object_t* object_new_string(const char* value) {
    object_t* obj = (object_t*)memory_allocate(sizeof(object_t));
    if (!obj) return NULL;
    obj->type = OBJECT_STRING;
    obj->as.string.value = (char*)value; // assume value is heap allocated
    if (!obj->as.string.value) {
        memory_free(obj);
        return NULL;
    }
    return obj;
}
static object_t* object_new_number(double value) {
    object_t* obj = (object_t*)memory_allocate(sizeof(object_t));
    if (!obj) return NULL;
    obj->type = OBJECT_NUMBER;
    obj->as.number.value = value;
    return obj;
}

expr_t* parse_primary(parser_t* parser) {
    // unimplemented
    printf("parse_primary called\n");
    if (token_match(parser, 2, NUMBER, STRING)) {
        token_t* value_token = token_previous_ptr(parser);
        object_t* obj = NULL;
        if (value_token->type == STRING) {
            obj = object_new_string(value_token->lexeme);
        }
        else if (value_token->type == NUMBER) {
            obj = object_new_number(atof(value_token->lexeme));
        }
        if (!obj) {
            printf("Failed to create object for literal\n");
            return NULL;
        }
        expr_t* new_expr = (expr_t*)memory_allocate(sizeof(expr_t));
        if (!new_expr) {
            memory_free(&obj);
            printf("Failed to allocate memory for literal expression\n");
            return NULL;
        }
        new_expr->type = EXPR_LITERAL;
        new_expr->as.literal_expr.value = obj;
        return new_expr;
    }
    return NULL;
}