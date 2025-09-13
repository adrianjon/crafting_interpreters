#ifndef LOX_TOKEN_H
#define LOX_TOKEN_H

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
    EOF_,
} token_type_t;

extern const char* token_type_names[];

typedef struct {
    token_type_t type;
    char* lexeme;
    int line;
} token_t;
#endif // LOX_TOKEN_H