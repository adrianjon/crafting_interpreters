//
// Created by adrian on 2025-09-15.
//
#include "Scanner.h"
#include "../extra/Arrays.h"
#include "../extra/Windows.h"
#include "Token.h"
#include <stdio.h>
#include <stdlib.h>

struct scanner{
    file_t *p_target_file;
    const char* p_start;
    const char* p_current;
    int line;
    dynamic_array_t * tokens;
};
// Forward declarations
static void add_token(const token_type_t type, const char * lexeme, const int line, dynamic_array_t * tokens);
static bool scanner_is_at_end(const scanner_t * p_scanner);
static int scanner_get_line(const scanner_t * p_scanner);
static char scanner_advance(scanner_t * p_scanner);
static const char * scanner_previous(const scanner_t * p_scanner);
static char scanner_peek(const scanner_t * p_scanner);
static char scanner_peek_next(const scanner_t * p_scanner);
static const char * scanner_peek_ptr(const scanner_t * p_scanner);
static bool match(scanner_t * p_scanner, const char expected, const char actual);
static void string(scanner_t * p_scanner);
static bool is_digit(const char c);
static void number(scanner_t * p_scanner);
static bool is_alpha(const char c);
static bool is_alphanumeric(const char c);
static void identifier(scanner_t * p_scanner);
static token_type_t get_keyword_type(const char * identifier);
// Public API
scanner_t * scanner_init(const char * filename) {
    scanner_t * p_scanner = memory_allocate(sizeof(scanner_t));
    p_scanner->p_target_file = read_file(filename);
    if (!p_scanner->p_target_file) {
        print_error("Unable to open file");
        return NULL;
    }
    p_scanner->p_start = p_scanner->p_target_file->buffer;
    p_scanner->p_current = p_scanner->p_start;
    p_scanner->line = 1;
    p_scanner->tokens = create_array(8 * sizeof(token_t));
    return p_scanner;
}
void scanner_scan(scanner_t * p_scanner) {
    while (!scanner_is_at_end(p_scanner)) {
        const char c = scanner_advance(p_scanner);
        bool is_equal;
        const char * start;
        size_t length;
        char buffer[256] = {0}; // TODO
        switch (c) {
            case '(': add_token(LEFT_PAREN, "(", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case ')': add_token(RIGHT_PAREN, ")", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '{': add_token(LEFT_BRACE, "{", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '}': add_token(RIGHT_BRACE, "}", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case ',': add_token(COMMA, ",", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '.': add_token(DOT, ".", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '+': add_token(PLUS, "+", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '-': add_token(MINUS, "-", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case ';': add_token(SEMICOLON, ";", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case ':': add_token(COLON, ":", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '*': add_token(STAR, "*", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '%': add_token(PERCENTAGE, "%", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '!':
                is_equal = match(p_scanner, '=', scanner_peek(p_scanner));
                add_token(is_equal ? BANG_EQUAL : BANG, is_equal ? "!=" : "!", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '=':
                is_equal = match(p_scanner, '=', scanner_peek(p_scanner));
                add_token(is_equal ? EQUAL_EQUAL : EQUAL, is_equal ? "==" : "=", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '<':
                is_equal = match(p_scanner, '=', scanner_peek(p_scanner));
                add_token(is_equal ? LESS_EQUAL : LESS, is_equal ? "<=" : "<", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '>':
                is_equal = match(p_scanner, '=', scanner_peek(p_scanner));
                add_token(is_equal ? GREATER_EQUAL : GREATER, is_equal ? ">=" : ">", scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            case '/':
                if (match(p_scanner, '/', scanner_peek(p_scanner))) {
                    // Handle single-line comment
                    while (!scanner_is_at_end(p_scanner) && scanner_peek(p_scanner) != '\n') {
                        scanner_advance(p_scanner);
                    }
                } else if (match(p_scanner, '*', scanner_peek(p_scanner))) {
                    // Handle multi-line comment
                    unsigned int counter = 1; // counter for nested multi-line comments
                    while (!scanner_is_at_end(p_scanner) && counter > 0) {
                        if (scanner_peek(p_scanner) == '/' && scanner_peek_next(p_scanner) == '*') {
                            counter++;
                        } else if (scanner_peek(p_scanner) == '*' && scanner_peek_next(p_scanner) == '/') {
                            counter--;
                        }
                        scanner_advance(p_scanner);
                    }
                    if (!scanner_is_at_end(p_scanner)) {

                        scanner_advance(p_scanner);
                        scanner_advance(p_scanner);
                    } else {
                        print_error("Unterminated multi-line comment");
                    }
                } else {
                    add_token(SLASH, "/", scanner_get_line(p_scanner), p_scanner->tokens);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                // Ignore whitespace and newlines
                break;
            case '"':
                start = scanner_previous(p_scanner);
                string(p_scanner);

                if (scanner_peek_ptr(p_scanner) == NULL) {
                    print_error("Unterminated string");
                    break;
                }
                length = scanner_peek_ptr(p_scanner) - start;

                memory_copy(buffer, start + 1, length - 2); // exclude quotes bug here
                buffer[length - 2] = '\0';

                add_token(STRING, buffer, scanner_get_line(p_scanner), p_scanner->tokens);
                break;
            default:
                if (is_digit(c)) {
                    start = scanner_previous(p_scanner);
                    number(p_scanner);
                    length = scanner_peek_ptr(p_scanner) - start;

                    memory_copy(buffer, start, length);
                    buffer[length] = '\0';
                    add_token(NUMBER, buffer, scanner_get_line(p_scanner), p_scanner->tokens);
                } else if (is_alpha(c)) {
                    start = scanner_previous(p_scanner);
                    identifier(p_scanner);
                    length = scanner_peek_ptr(p_scanner) - start;
                    memory_copy(buffer, start, length);
                    buffer[length] = '\0';
                    const token_type_t type = get_keyword_type(buffer);
                    add_token(type, buffer, scanner_get_line(p_scanner), p_scanner->tokens);
                } else {
                    print_error("Unexpected character");
                }
                break;
        }
    }
    add_token(END_OF_FILE, NULL, scanner_get_line(p_scanner), p_scanner->tokens);
    printf("Scanning complete. Total tokens: %zu\n", p_scanner->tokens->size / sizeof(token_t));

}
const dynamic_array_t * scanner_get_tokens(const scanner_t * p_scanner) {
    return p_scanner->tokens;
}
void scanner_print_tokens(const scanner_t * p_scanner) {
    const dynamic_array_t * tokens = p_scanner->tokens;
    for (size_t i = 0; i < tokens->size / sizeof(token_t); i++) {
        const token_t * token = (token_t *) array_get(tokens, i * sizeof(token_t));
        print("Token: ");
        print(g_token_type_names[token->type]);
        print(" at line ");
        print_int(token->line);
        print(" with lexeme '");
        print(token->lexeme);
        print("'");
        print("\n");
    }
}
void scanner_free(scanner_t * p_scanner) {
    if (!p_scanner) return;
    if (p_scanner->tokens->data) {
        array_free(p_scanner->tokens);
        p_scanner->tokens->data = NULL;
        p_scanner->tokens->capacity = 0;
        p_scanner->tokens->size = 0;
    }
    free_file(p_scanner->p_target_file);
    memory_free((void**)&p_scanner);
    p_scanner = NULL;
}
// Private functions
static void add_token(const token_type_t type, const char * lexeme, const int line, dynamic_array_t * tokens) {
    token_t token = {.type = type, .line = line};

    if (lexeme) {

        // TODO: implement some kind of strlen function and replace with this manual method
        size_t lex_len = 0;
        while (lexeme[lex_len] != '\0' && lex_len < TOKEN_LEXEME_MAX - 1) {
            lex_len++;
        }
        memory_copy(token.lexeme, lexeme, lex_len);
        token.lexeme[lex_len] = '\0';
    } else {
        token.lexeme[0] = '\0';
    }
    array_push(tokens, &token, sizeof(token_t));
}
static bool scanner_is_at_end(const scanner_t * p_scanner) {
    return *p_scanner->p_current == '\0' || p_scanner->p_current >= p_scanner->p_start + p_scanner->p_target_file->size;
}
static int scanner_get_line(const scanner_t * p_scanner) {
    return p_scanner->line;
}
static char scanner_advance(scanner_t * p_scanner) {
    if (*p_scanner->p_current == '\n') p_scanner->line++;
    return *p_scanner->p_current++;
}
static const char * scanner_previous(const scanner_t * p_scanner) {
    if (p_scanner->p_current > p_scanner->p_start) {
        return p_scanner->p_current - 1;
    }
    return NULL;
}
static char scanner_peek(const scanner_t * p_scanner) {
    if (scanner_is_at_end(p_scanner)) return '\0';
    return *p_scanner->p_current;
}
static char scanner_peek_next(const scanner_t * p_scanner) {
    if (scanner_is_at_end(p_scanner)) return '\0';
    if (*(p_scanner->p_current + 1) == '\0') return '\0';
    return *(p_scanner->p_current + 1);
}
static const char * scanner_peek_ptr(const scanner_t * p_scanner) {
    return p_scanner->p_current;
}
static bool match(scanner_t * p_scanner, const char expected, const char actual) {
    (void) actual;
    //printf("match: expected '%c', actual '%c'\n", expected, actual);
    if (scanner_is_at_end(p_scanner)) return false;
    if (*p_scanner->p_current != expected) return false;
    p_scanner->p_current++;
    return true;
}
static void string(scanner_t * p_scanner) {
    while (scanner_peek(p_scanner) != '"' && !scanner_is_at_end(p_scanner)) {
        scanner_advance(p_scanner);
    }
    if (scanner_is_at_end(p_scanner)) {
        // Unterminated string
        print_error("Unterminated string");
        return;
    }
    // Consume the closing quote
    scanner_advance(p_scanner);
}
static bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}
static void number(scanner_t * p_scanner) {
    while (is_digit(scanner_peek(p_scanner))) scanner_advance(p_scanner);

    if (scanner_peek(p_scanner) == '.' && is_digit(scanner_peek_next(p_scanner))) {
        // Consume the "."
        scanner_advance(p_scanner);
        while (is_digit(scanner_peek(p_scanner))) scanner_advance(p_scanner);
    }
}
static bool is_alpha(const char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}
static bool is_alphanumeric(const char c) {
    return is_alpha(c) || is_digit(c);
}
static void identifier(scanner_t * p_scanner) {
    while (is_alphanumeric(scanner_peek(p_scanner))) scanner_advance(p_scanner);
}
static token_type_t get_keyword_type(const char * identifier) {
    if (memory_compare(identifier, "if", 2)) return IF;
    if (memory_compare(identifier, "else", 4)) return ELSE;
    if (memory_compare(identifier, "while", 5)) return WHILE;
    if (memory_compare(identifier, "for", 3)) return FOR;
    if (memory_compare(identifier, "return", 6)) return RETURN;
    if (memory_compare(identifier, "print", 5)) return PRINT;
    if (memory_compare(identifier, "and", 3)) return AND;
    if (memory_compare(identifier, "or", 2)) return OR;
    if (memory_compare(identifier, "true", 4)) return KW_TRUE;
    if (memory_compare(identifier, "false", 5)) return KW_FALSE;
    if (memory_compare(identifier, "nil", 3)) return NIL;
    if (memory_compare(identifier, "var", 3)) return VAR;
    if (memory_compare(identifier, "fun", 3)) return FUN;
    if (memory_compare(identifier, "class", 5)) return CLASS;
    if (memory_compare(identifier, "super", 5)) return SUPER;
    if (memory_compare(identifier, "this", 4)) return KW_THIS;
    return IDENTIFIER;
}