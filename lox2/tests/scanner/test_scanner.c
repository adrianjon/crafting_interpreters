//
// Created by adrian on 2025-10-11.
//


#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "list.h"

int run_scanner_tests(scanner_t * p_scanner);

static bool token_equal(token_t const * e, token_t const * a);
static bool compare_tokens(list_t const * actual, list_t const * expected);

//
// Test case 1: simple arithmetic expression
//
static char const * g_test_source_1 =
    "1 + 2 * (3 - 4) / 5;";

static list_t const EXPECTED_TOKENS_1 = {
    .data = (void *[]) {
        /* 1 */ &(token_t){ .type = NUMBER,      .lexeme = "1", .line = 1 },
        /* + */ &(token_t){ .type = PLUS,        .lexeme = "+", .line = 1 },
        /* 2 */ &(token_t){ .type = NUMBER,      .lexeme = "2", .line = 1 },
        /* * */ &(token_t){ .type = STAR,        .lexeme = "*", .line = 1 },
        /* ( */ &(token_t){ .type = LEFT_PAREN,  .lexeme = "(", .line = 1 },
        /* 3 */ &(token_t){ .type = NUMBER,      .lexeme = "3", .line = 1 },
        /* - */ &(token_t){ .type = MINUS,       .lexeme = "-", .line = 1 },
        /* 4 */ &(token_t){ .type = NUMBER,      .lexeme = "4", .line = 1 },
        /* ) */ &(token_t){ .type = RIGHT_PAREN, .lexeme = ")", .line = 1 },
        /* / */ &(token_t){ .type = SLASH,       .lexeme = "/", .line = 1 },
        /* 5 */ &(token_t){ .type = NUMBER,      .lexeme = "5", .line = 1 },
        /* ; */ &(token_t){ .type = SEMICOLON,   .lexeme = ";", .line = 1 },
        /* EOF */ &(token_t){ .type = END_OF_FILE, .lexeme = "", .line = 1 },
        NULL
    },
    .count = 13,
    .capacity = 16,
    .free_fn = NULL
};

//
// Test case 2: string literals, comparisons, keywords, and boolean operators
//
static char const * g_test_source_2 =
    "\"hello\" != \"\\\"world\" and true or false; var x = 42;";

static list_t const EXPECTED_TOKENS_2 = {
    .data = (void *[]) {
        /* "hello" */    &(token_t){ .type = STRING,     .lexeme = "\"hello\"",  .line = 1 },
        /* != */        &(token_t){ .type = BANG_EQUAL, .lexeme = "!=",         .line = 1 },
        /* "world" */   &(token_t){ .type = STRING,     .lexeme = "\"\\\"world\"",  .line = 1 },
        /* and */       &(token_t){ .type = AND,        .lexeme = "and",        .line = 1 },
        /* true */      &(token_t){ .type = KW_TRUE,    .lexeme = "true",       .line = 1 },
        /* or */        &(token_t){ .type = OR,         .lexeme = "or",         .line = 1 },
        /* false */     &(token_t){ .type = KW_FALSE,   .lexeme = "false",      .line = 1 },
        /* ; */         &(token_t){ .type = SEMICOLON,  .lexeme = ";",          .line = 1 },
        /* var */       &(token_t){ .type = VAR,        .lexeme = "var",        .line = 1 },
        /* x */         &(token_t){ .type = IDENTIFIER, .lexeme = "x",          .line = 1 },
        /* = */         &(token_t){ .type = EQUAL,      .lexeme = "=",          .line = 1 },
        /* 42 */        &(token_t){ .type = NUMBER,     .lexeme = "42",         .line = 1 },
        /* ; */         &(token_t){ .type = SEMICOLON,  .lexeme = ";",          .line = 1 },
        /* EOF */       &(token_t){ .type = END_OF_FILE, .lexeme = "",          .line = 1 },
        NULL
    },
    .count = 14,
    .capacity = 16,
    .free_fn = NULL
};

//
// Test case 3: variable declarations with floating-point literals
//
static char const * g_test_source_3 =
    "var radius = 10.5; var area = 3.14 * radius * radius;";

static list_t const EXPECTED_TOKENS_3 = {
    .data = (void *[]) {
        &(token_t){ .type = VAR,          .lexeme = "var",        .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "radius",     .line = 1 },
        &(token_t){ .type = EQUAL,        .lexeme = "=",          .line = 1 },
        &(token_t){ .type = NUMBER,       .lexeme = "10.5",       .line = 1 },
        &(token_t){ .type = SEMICOLON,    .lexeme = ";",          .line = 1 },
        &(token_t){ .type = VAR,          .lexeme = "var",        .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "area",       .line = 1 },
        &(token_t){ .type = EQUAL,        .lexeme = "=",          .line = 1 },
        &(token_t){ .type = NUMBER,       .lexeme = "3.14",       .line = 1 },
        &(token_t){ .type = STAR,         .lexeme = "*",          .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "radius",     .line = 1 },
        &(token_t){ .type = STAR,         .lexeme = "*",          .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "radius",     .line = 1 },
        &(token_t){ .type = SEMICOLON,    .lexeme = ";",          .line = 1 },
        &(token_t){ .type = END_OF_FILE,  .lexeme = "",           .line = 1 },
        NULL
    },
    .count = 15,
    .capacity = 16,
    .free_fn = NULL
};

//
// Test case 4: for-loop with initialization, condition, increment, and block
//
static char const * g_test_source_4 =
    "for (var i = 0; i < 10; i = i + 1) { print i; }";

static list_t const EXPECTED_TOKENS_4 = {
    .data = (void *[]) {
        &(token_t){ .type = FOR,          .lexeme = "for",        .line = 1 },
        &(token_t){ .type = LEFT_PAREN,   .lexeme = "(",          .line = 1 },
        &(token_t){ .type = VAR,          .lexeme = "var",        .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "i",          .line = 1 },
        &(token_t){ .type = EQUAL,        .lexeme = "=",          .line = 1 },
        &(token_t){ .type = NUMBER,       .lexeme = "0",          .line = 1 },
        &(token_t){ .type = SEMICOLON,    .lexeme = ";",          .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "i",          .line = 1 },
        &(token_t){ .type = LESS,         .lexeme = "<",          .line = 1 },
        &(token_t){ .type = NUMBER,       .lexeme = "10",         .line = 1 },
        &(token_t){ .type = SEMICOLON,    .lexeme = ";",          .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "i",          .line = 1 },
        &(token_t){ .type = EQUAL,        .lexeme = "=",          .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "i",          .line = 1 },
        &(token_t){ .type = PLUS,         .lexeme = "+",          .line = 1 },
        &(token_t){ .type = NUMBER,       .lexeme = "1",          .line = 1 },
        &(token_t){ .type = RIGHT_PAREN,  .lexeme = ")",          .line = 1 },
        &(token_t){ .type = LEFT_BRACE,   .lexeme = "{",          .line = 1 },
        &(token_t){ .type = PRINT,        .lexeme = "print",      .line = 1 },
        &(token_t){ .type = IDENTIFIER,   .lexeme = "i",          .line = 1 },
        &(token_t){ .type = SEMICOLON,    .lexeme = ";",          .line = 1 },
        &(token_t){ .type = RIGHT_BRACE,  .lexeme = "}",          .line = 1 },
        &(token_t){ .type = END_OF_FILE,  .lexeme = "",           .line = 1 },
        NULL
    },
    .count = 23,
    .capacity = 32,
    .free_fn = NULL
};


static bool token_equal(token_t const * e, token_t const * a) {
    return e->type == a->type && e->line == a->line && strcmp(e->lexeme, a->lexeme) == 0;
}
static bool compare_tokens(list_t const * actual, list_t const * expected) {
    /* count expected entries by NULL sentinel */
    size_t exp_count = 0;
    while (expected->data[exp_count] != NULL) {
        exp_count++;
    }

    if (actual->count != exp_count) {
        printf("  token count mismatch: expected %zu, got %zu\n",
               exp_count, actual->count);
        return false;
    }

    for (size_t i = 0; i < exp_count; i++) {
        const token_t *e = expected->data[i];
        const token_t *a = actual->data[i];
        if (!token_equal(e, a)) {
            printf("  token #%zu mismatch:\n", i);
            printf("    expected: { type=%s, lexeme=\"%s\", line=%zu }\n",
                   g_token_type_names[e->type], e->lexeme, e->line);
            printf("         got: { type=%s, lexeme=\"%s\", line=%zu }\n",
                   g_token_type_names[a->type], a->lexeme, a->line);
            return false;
        }
    }

    return true;
}

int run_scanner_tests(scanner_t * p_scanner) {
    printf("SCANNER TESTS:\n");
    struct {
        char const * source;
        list_t const * expected; //<token_t*>
        char const * name;
    } const tests[] = {
        {g_test_source_1, &EXPECTED_TOKENS_1, "Test 1 (arithmetic)"},
        {g_test_source_2, &EXPECTED_TOKENS_2, "Test 2 (strings & keywords)" },
        { g_test_source_3, &EXPECTED_TOKENS_3, "Test 3 (floats & vars)" },
        { g_test_source_4, &EXPECTED_TOKENS_4, "Test 4 (for-loop)" },
        {NULL, NULL, NULL }
    };
    bool all_passed = true;
    for (int ti = 0; tests[ti].source; ti++) {
        printf("%s:\n", tests[ti].name);

        p_scanner->start = tests[ti].source;

        // Scanner manages list
        list_t actual = scan_tokens(p_scanner);

        /* compare against expected */
        if (compare_tokens(&actual, tests[ti].expected)) {
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
            all_passed = false;
        }

        /* free the actual tokens and list */
        list_free(&actual);
    }
    return all_passed ? 0 : 1;
}