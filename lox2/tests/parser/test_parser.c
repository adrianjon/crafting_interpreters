//
// Created by adrian on 2025-10-11.
//

#include "../../parser.h"
#include "../../scanner.h"
#include "../../stmt.h"
#include "../../expr.h"

int run_parser_tests(parser_t * p_parser);

static bool token_equal(token_t const * e, token_t const * a);
static bool expr_equal(expr_t const * a, expr_t const * b);
static bool stmt_equal(stmt_t * a, stmt_t * b);
static bool compare_statements(list_t const * actual, list_t const * expected);

static token_t token_one = { NUMBER, "1", 1 };
static token_t token_two = { NUMBER, "2", 1 };
static token_t token_three = { NUMBER, "3", 1 };
static token_t token_plus = { PLUS, "+", 1 };
static token_t token_minus = { MINUS, "-", 1 };
static token_t token_multiply = { STAR, "*", 1 };
static expr_t literal_one = {
    .type = EXPR_LITERAL,
    .as.literal_expr = { &token_one }
};
static expr_t literal_two = {
    .type = EXPR_LITERAL,
    .as.literal_expr = { & token_two }
};
static expr_t literal_three = {
    .type = EXPR_LITERAL,
    .as.literal_expr = { &token_three }
};
static expr_t multiply_one = {
    .type = EXPR_BINARY,
    .as.binary_expr = {
        &literal_two,
        &token_multiply,
        &literal_three
    }
};
static expr_t add_one = {
    .type = EXPR_BINARY,
    .as.binary_expr = {
        &literal_one,
        &token_plus,
        &multiply_one,
    }
};
static const list_t expected_statements_1 = {
    (void*[]){
        &(stmt_t){
            .type = STMT_EXPRESSION,
            .as.expression_stmt = {
                .expression = &add_one
            }
        },
        NULL,
    }
};

static bool token_equal(token_t const * e, token_t const * a) {
    return e->type == a->type && e->line == a->line && strcmp(e->lexeme, a->lexeme) == 0;
}
static bool expr_equal(expr_t const * a, expr_t const * b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case EXPR_LITERAL:
            return token_equal(a->as.literal_expr.kind, b->as.literal_expr.kind);
        case EXPR_BINARY:
            if (a->as.binary_expr.operator->type != b->as.binary_expr.operator->type) return false;
            return expr_equal(a->as.binary_expr.left, b->as.binary_expr.left) &&
                expr_equal(a->as.binary_expr.right, b->as.binary_expr.right);;
        case EXPR_UNARY:
            if (a->as.unary_expr.operator->type != b->as.unary_expr.operator->type) return false;
            return expr_equal(a->as.unary_expr.right, b->as.unary_expr.right);
        case EXPR_GROUPING:
            return expr_equal(a->as.grouping_expr.expression,
                              b->as.grouping_expr.expression);
        case EXPR_VARIABLE:
            return a->as.variable_expr.name->type == b->as.variable_expr.name->type
                && strcmp(a->as.variable_expr.name->lexeme,
                          b->as.variable_expr.name->lexeme) == 0;
        case EXPR_ASSIGN:
            if (a->as.assign_expr.target->type != b->as.assign_expr.target->type) return false;
            if (strcmp(a->as.assign_expr.target->as.variable_expr.name->lexeme,
                       b->as.assign_expr.target->as.variable_expr.name->lexeme) != 0) return false;
            return expr_equal(a->as.assign_expr.value, b->as.assign_expr.value);
        default:
            fprintf(stderr, "Unhandled expression type (%d)\n", a->type);
            return false;
    }
}
static bool stmt_equal(stmt_t * a, stmt_t * b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case STMT_EXPRESSION:
            return expr_equal(a->as.expression_stmt.expression, b->as.expression_stmt.expression);
        case STMT_PRINT:
            return expr_equal(a->as.print_stmt.expression, b->as.print_stmt.expression);
        case STMT_VAR:
            if (a->as.var_stmt.name->type != b->as.var_stmt.name->type ||
                strcmp(a->as.var_stmt.name->lexeme, b->as.var_stmt.name->lexeme) != 0)
                return false;
            if (a->as.var_stmt.initializer == NULL && b->as.var_stmt.initializer == NULL)
                return true;
            if (a->as.var_stmt.initializer == NULL || b->as.var_stmt.initializer == NULL)
                return false;
            return expr_equal(a->as.var_stmt.initializer, b->as.var_stmt.initializer);
        case STMT_BLOCK:
            list_t const a_stmts = {
                .data = (void**)a->as.block_stmt.statements,
                .count = a->as.block_stmt.count,
                .capacity = a->as.block_stmt.count,
                .free_fn = NULL // TODO
            };
            list_t const b_stmts = {
                .data =  (void**)b->as.block_stmt.statements,
                .count = b->as.block_stmt.count,
                .capacity = b->as.block_stmt.count,
                .free_fn = NULL // TODO
            };
            return compare_statements(&a_stmts, &b_stmts);
        default:
            fprintf(stderr, "Unhandled statement type (%d)\n", a->type);
            return false;
    }
}
static bool compare_statements(list_t const * actual, list_t const * expected) {
    /* count expected entries by NULL sentinel */
    size_t exp_count = 0;
    while (expected->data[exp_count] != NULL) {
        exp_count++;
    }

    if (actual->count != exp_count) {
        printf("  statement count mismatch: expected %zu, got %zu\n",
               exp_count, actual->count);
        return false;
    }

    for (size_t i = 0; i < exp_count; i++) {
        stmt_t * e = expected->data[i];
        stmt_t * a = actual->data[i];
        if (!stmt_equal(e, a)) {
            printf("  statement #%zu mismatch:\n", i);
            return false;
        }
    }

    return true;
}
int run_parser_tests(parser_t * p_parser) {
    printf("PARSER TESTS:\n");
    const struct {
         char const * source;
         list_t const expected; //<token_t*>
         char const * name;
    } tests[] = {
        // Test 1: 1 + 2 * 3;
        { "1 + 2 * 3;", expected_statements_1, "test 1", },
        NULL
    };
    bool all_passed = true;

    // create a new scanner
    scanner_t scanner = {0};

    for (int ti = 0; tests[ti].source; ti++) {
        printf("%s:\n", tests[ti].name);

        scanner.start = tests[ti].source;
        list_t const tokens = scan_tokens(&scanner);
        p_parser->tokens = tokens;

        list_t actual = parse(p_parser); // List<stmt_t*>

        /* compare against expected */
        if (compare_statements(&actual, &tests[ti].expected)) {
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
            all_passed = false;
        }

        /* free the actual tokens and list */
        //list_free(&actual);
    }
    return all_passed ? 0 : 1;
}
