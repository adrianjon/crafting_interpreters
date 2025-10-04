//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_TOKEN_H
#define LOX_TOKEN_H
#include <stdlib.h>

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

extern const char* g_token_type_names[];

#define TOKEN_LEXEME_MAX 256
typedef struct {
    token_type_t type;
    char lexeme[TOKEN_LEXEME_MAX]; // max lexeme length
    int line; // TODO make this size_t
} token_t;

token_t * new_token(token_type_t type, char lexeme[], int line);

#endif //LOX_TOKEN_H