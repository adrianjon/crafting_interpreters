//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_TOKEN_H
#define LOX_TOKEN_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    LEFT_PAREN,
    RIGHT_PAREN,
    COMMA,
    DOT,
    SEMICOLON,
    COLON,
    SLASH,
    LEFT_BRACE,
    RIGHT_BRACE,
    PLUS,
    MINUS,
    STAR,
    PERCENTAGE,
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    STRING,
    NUMBER,
    IDENTIFIER,
    IF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    PRINT,
    AND,
    OR,
    KW_TRUE,
    KW_FALSE,
    NIL,
    VAR,
    FUN,
    CLASS,
    SUPER,
    KW_THIS,
    END_OF_FILE,
} token_type_t;

 static char const * g_token_type_names[] = {
    "LEFT_PAREN",
    "RIGHT_PAREN",
    "COMMA",
    "DOT",
    "SEMICOLON",
    "COLON",
    "SLASH",
    "LEFT_BRACE",
    "RIGHT_BRACE",
    "PLUS",
    "MINUS",
    "STAR",
    "PERCENTAGE",
    "BANG",
    "BANG_EQUAL",
    "EQUAL",
    "EQUAL_EQUAL",
    "GREATER",
    "GREATER_EQUAL",
    "LESS",
    "LESS_EQUAL",
    "STRING",
    "NUMBER",
    "IDENTIFIER",
    "IF",
    "ELSE",
    "WHILE",
    "FOR",
    "RETURN",
    "PRINT",
    "AND",
    "OR",
    "TRUE",
    "FALSE",
    "NIL",
    "VAR",
    "FUN",
    "CLASS",
    "SUPER",
    "THIS",
    "EOF",
};

typedef struct {
    token_type_t    type;
    char *          lexeme;
    size_t          line;
} token_t;

static inline token_t * new_token(token_type_t const type, char const * lexeme, size_t const line) {
    token_t * token = malloc(sizeof(token_t));
    token->type = type;
    rsize_t const  lexeme_len = strlen(lexeme);
    token->lexeme = malloc(lexeme_len + 1);
    strcpy_s(token->lexeme, lexeme_len + 1, lexeme);
    token->line = line;
    return token;
}
static inline token_t * copy_token(token_t const * token) {
    return new_token(token->type, token->lexeme, token->line);
}
// TODO put stuff into .c file
static inline void token_free(void ** pp_token) {
    token_t * p_token = *(token_t **)pp_token;
    free(p_token->lexeme);
    p_token->lexeme = NULL;
    free(p_token);
    *pp_token = NULL;
}

#endif //LOX_TOKEN_H