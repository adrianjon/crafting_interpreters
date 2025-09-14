
#include "../extra/Arrays.h"
#include "../extra/Windows.h"
#include "Scanner.h"
#include "Token.h"

// Static variables
static scanner_t g_scanner = {0};

// Public functions

void scanner_read_file(const char* filename) {
    g_scanner.p_target_file = read_file(filename);
    if(!g_scanner.p_target_file) {
        print_error("Failed to read file");
        return;
    }
    g_scanner.p_start = g_scanner.p_target_file->buffer;
    g_scanner.p_current = g_scanner.p_target_file->buffer;
    g_scanner.line = 1;
}

/*
typedef struct {
    token_type_t type;
    char lexeme[256]; // max lexeme length
    int line;
} token_t;
 */
void add_token(token_type_t type, const char* lexeme, int line, dynamic_array_t* tokens) {
    token_t token = { .type = type, .line = line };
    
    if (lexeme) {
        size_t lex_len = 0;
        while(lexeme[lex_len] != '\0' && lex_len < 255) {
            lex_len++;
        }
        memory_copy(token.lexeme, lexeme, lex_len);
        token.lexeme[lex_len] = '\0';
    }
    array_push(tokens, &token, sizeof(token_t));
}

bool scanner_is_at_end(void) {
    return *g_scanner.p_current == '\0' || g_scanner.p_current >= g_scanner.p_start + g_scanner.p_target_file->size;
}
int scanner_get_line(void) {
    return g_scanner.line;
}
char scanner_advance(void) {
    if (*g_scanner.p_current == '\n') g_scanner.line++;
    return *g_scanner.p_current++;
}
const char* scanner_previous(void) {
    if (g_scanner.p_current > g_scanner.p_start) {
        return (g_scanner.p_current - 1);
    }
    return NULL;
}
char scanner_peek(void) {
    if (scanner_is_at_end()) return '\0';
    return *g_scanner.p_current;
}
char scanner_peek_next(void) {
    if (scanner_is_at_end()) return '\0';
    if (*(g_scanner.p_current + 1) == '\0') return '\0';
    return *(g_scanner.p_current + 1);
}
const char* scanner_peek_ptr(void) {
    if (scanner_is_at_end()) return NULL;
    return g_scanner.p_current;
}

bool match(char expected, char actual) {
    //printf("match: expected '%c', actual '%c'\n", expected, actual);
    if (scanner_is_at_end()) return false;
    if (*g_scanner.p_current != expected) return false;
    g_scanner.p_current++;
    return true;
}
void string() {
    while (scanner_peek() != '"' && !scanner_is_at_end()) {
        scanner_advance();
    }
    if (scanner_is_at_end()) {
        // Unterminated string
        print_error("Unterminated string");
        return;
    }
    // Consume the closing quote
    scanner_advance();
    // Trim the surrounding quotes
    // TODO: Implement trimming logic
}
bool is_digit(char c) {
    return c >= '0' && c <= '9';
}
void number() {
    while (is_digit(scanner_peek())) scanner_advance();

    if (scanner_peek() == '.' && is_digit(scanner_peek_next())) {
        // Consume the "."
        scanner_advance();
        while (is_digit(scanner_peek())) scanner_advance();
    }
    // Convert the number string to an integer
    // TODO: Implement conversion logic
}
bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}
bool is_alphanumeric(char c) {
    return is_alpha(c) || is_digit(c);
}
void identifier() {
    while (is_alphanumeric(scanner_peek())) scanner_advance();
}
/*
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
*/
token_type_t get_keyword_type(const char* identifier) {
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

void print_tokens(dynamic_array_t* tokens) {
    for(size_t i = 0; i < tokens->size / sizeof(token_t); i++) {
        token_t* token = (token_t*)array_get(tokens, i * sizeof(token_t));
        print("Token: ");
        print(token_type_names[token->type]);
        print(" at line ");
        print_int(token->line);
            if (token->lexeme) {
                print(" with lexeme '");
                print(token->lexeme);
                print("'");
            }
        print("\n");
    }
}

