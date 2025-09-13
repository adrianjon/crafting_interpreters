#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "Object.h"

// Simple string builder for demonstration
typedef struct {
    char* buf;
    size_t cap;
    size_t len;
} string_builder_tt;

void sb_init(string_builder_tt* sb) {
    sb->cap = 256;
    sb->len = 0;
    sb->buf = malloc(sb->cap);
    sb->buf[0] = '\0';
}

void sb_append(string_builder_tt* sb, const char* str) {
    size_t slen = strlen(str);
    if (sb->len + slen + 1 > sb->cap) {
        sb->cap *= 2;
        sb->buf = realloc(sb->buf, sb->cap);
    }
    strcpy(sb->buf + sb->len, str);
    sb->len += slen;
    sb->buf[sb->len] = '\0';
}

void sb_free(string_builder_tt* sb) {
    free(sb->buf);
}

// Forward declaration for recursive printing
void* print_expr(expr_t* expr, void* context);
const char* parenthesize(const char* name, expr_visitor_t* visitor, void* context, int count, ...);


// Visitor functions
void* visit_assign(expr_t* expr, void* context) {
    // string_builder_tt* sb = (string_builder_tt*)context;
    // sb_append(sb, "(= ");
    // //sb_append(sb, expr->as.assign_expr.name.lexeme);
    // sb_append(sb, expr->as.assign_expr.name->lexeme);
    // sb_append(sb, " ");
    // // print_expr(expr->as.assign_expr.value, sb);
    // sb_append(sb, ")");
    return NULL;
}

void* visit_binary(expr_t* expr, expr_visitor_t* visitor, void* context) {
    //printf("Visiting binary expression\n");
    const char* result = parenthesize(expr->as.binary_expr.operator->lexeme, visitor, context, 2, expr->as.binary_expr.left, expr->as.binary_expr.right);
    return (void*)result;
}
const char* parenthesize(const char* name, expr_visitor_t* visitor, void* context, int count, ...) {
    string_builder_tt* sb = (string_builder_tt*)context;
    sb_append(sb, "(");
    sb_append(sb, name);
    printf("(%s", name);
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        sb_append(sb, " ");
        printf(" ");
        expr_t* expr = va_arg(args, expr_t*);
        // printf("expr: %d\n", expr->type);
        expr_accept(expr, visitor, context);
    }
    sb_append(sb, ")");
    printf(")");
    va_end(args);

    return NULL;
}
void *visit_literal(expr_t* expr, expr_visitor_t* visitor, void* context) {
    string_builder_tt* sb = (string_builder_tt*)context;
    if (expr->as.literal_expr.value == NULL) {
        sb_append(sb, "nil");
        printf("nil");
    } else {
        // Here you would convert the object to string based on its type
        if (expr->as.literal_expr.value->type == OBJECT_NUMBER) {
            char num_buf[32];
            snprintf(num_buf, sizeof(num_buf), "%g", expr->as.literal_expr.value->as.number.value);
            sb_append(sb, num_buf);
            printf("%s", num_buf);
        }
        else {
            sb_append(sb, "literal");
            printf("literal");
        }
    }
    return NULL;
}
int main(void) {
    printf("\tAstPrinter\n");
    string_builder_tt sb;
    sb_init(&sb);

    // Example usage with a binary expression
    token_t plus = { .type = PLUS, .lexeme = "+", .line = 1 };

    expr_visitor_t visitor = {
       // .visit_assign = visit_assign,
        .visit_binary = visit_binary,
        .visit_literal = visit_literal,
        // ... assign other function pointers
    };

    expr_t expr = {
        .type = EXPR_BINARY,
        .as.binary_expr = {
            .left = &(expr_t){
                .type = EXPR_LITERAL,
                .as.literal_expr = {
                    .value = &(object_t){
                        .type = OBJECT_NUMBER,
                        .as.number = { .value = 1.0 }
                    }
                }
            },
            .operator = &plus,
            .right = &(expr_t){
                .type = EXPR_LITERAL,
                .as.literal_expr = {
                    .value = &(object_t){
                        .type = OBJECT_NUMBER,
                        .as.number = { .value = 2.0 }
                    }
                }
            }
        }
    };

    expr_t expr2 = {
        .type = EXPR_BINARY,
        .as.binary_expr = {
            .left = &(expr_t){
                .type = EXPR_BINARY,
                .as.binary_expr = {
                    .left = &(expr_t){
                        .type = EXPR_LITERAL,
                        .as.literal_expr = {
                            .value = &(object_t){
                                .type = OBJECT_NUMBER,
                                .as.number = { .value = 1.0 }
                            }
                        }
                    },
                    .operator = &plus,
                    .right = &(expr_t){
                        .type = EXPR_LITERAL,
                        .as.literal_expr = {
                            .value = &(object_t){
                                .type = OBJECT_NUMBER,
                                .as.number = { .value = 2.0 }
                            }
                        }
                    }
                }
            },
            .operator = &plus,
            .right = &(expr_t){
                .type = EXPR_LITERAL,
                .as.literal_expr = {
                    .value = &(object_t){
                        .type = OBJECT_NUMBER,
                        .as.number = { .value = 2.0 }
                    }
                }
            }
        }
    };

    expr_accept(&expr2, &visitor, &sb);
    // printf("\n");

    // printf("AST: %s\n", sb.buf);
    // printf("string builder capacity: %zu\n", sb.cap);
    // printf("string builder length: %zu\n", sb.len);
    // print_expr(&expr, &sb);
    // printf("string builder length after print: %zu\n", sb.len);
    // printf("%s\n", sb.buf);

    sb_free(&sb);
    return 0;
}