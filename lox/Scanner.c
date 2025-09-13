
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

void add_token(token_type_t type, int line, dynamic_array_t* tokens) {
    token_t token = { type, line };
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
token_type_t get_keyword_type(const char* identifier) {
    if (memory_compare(identifier, "if", 2)) return IF;
    if (memory_compare(identifier, "else", 4)) return ELSE;
    if (memory_compare(identifier, "while", 5)) return WHILE;
    if (memory_compare(identifier, "for", 3)) return FOR;
    if (memory_compare(identifier, "return", 6)) return RETURN;
    if (memory_compare(identifier, "print", 5)) return PRINT;
    return IDENTIFIER;
}

void print_tokens(dynamic_array_t* tokens) {
    for(size_t i = 0; i < tokens->size / sizeof(token_t); i++) {
        token_t* token = (token_t*)array_get(tokens, i * sizeof(token_t));
        print("Token: ");
        print(token_type_names[token->type]);
        print(" at line ");
        print_int(token->line);
        print("\n");
    }
}

// Private functions

// int main(void) {

//     Scanner.target_file = read_file("lox");
//     if(!Scanner.target_file) {
//         print_error("Failed to read file");
//         return 1;
//     }
//     Scanner.start = Scanner.target_file->buffer;
//     Scanner.current = Scanner.target_file->buffer;
//     Scanner.line = 1;

//     print("file size: ");
//     print_int(Scanner.target_file->size);
//     print("\n");

//     DynamicArray tokens = create_array(1024 * 1); // initial capacity for 1KB
//     while(!isAtEnd() && Scanner.current < Scanner.start + Scanner.target_file->size) {
//         char c = advance();
//         //print_char(c);
//         switch (c) {
//             case '(': addToken(LEFT_PAREN, Scanner.line, &tokens); break;
//             case ')': addToken(RIGHT_PAREN, Scanner.line, &tokens); break;
//             case '{': addToken(LEFT_BRACE, Scanner.line, &tokens); break;
//             case '}': addToken(RIGHT_BRACE, Scanner.line, &tokens); break;
//             case ',': addToken(COMMA, Scanner.line, &tokens); break;
//             case '.': addToken(DOT, Scanner.line, &tokens); break;
//             case '+': addToken(PLUS, Scanner.line, &tokens); break;
//             case '-': addToken(MINUS, Scanner.line, &tokens); break;
//             case ';': addToken(SEMICOLON, Scanner.line, &tokens); break;
//             case ':': addToken(COLON, Scanner.line, &tokens); break;
//             case '*': addToken(STAR, Scanner.line, &tokens); break;
//             case '!': addToken(match('=', peek()) ? BANG_EQUAL : BANG, Scanner.line, &tokens); break;
//             case '=': addToken(match('=', peek()) ? EQUAL_EQUAL : EQUAL, Scanner.line, &tokens); break;
//             case '<': addToken(match('=', peek()) ? LESS_EQUAL : LESS, Scanner.line, &tokens); break;
//             case '>': addToken(match('=', peek()) ? GREATER_EQUAL : GREATER, Scanner.line, &tokens); break;
//             case '/':
//                 if (match('/', peek())) {
//                     // Handle single-line comment
//                     while (!isAtEnd() && *Scanner.current != '\n') {
//                         Scanner.current++;
//                     }
//                 } else if (match('*', peek())) {
//                     // Handle multi-line comment
//                     unsigned int counter = 1; // counter for nested multi-line comments
//                     while (!isAtEnd() && counter > 0) {
//                         if (*Scanner.current == '\n') Scanner.line++;
//                         if (*Scanner.current == '/' && peekNext() == '*') {
//                             counter++;
//                         } else if (*Scanner.current == '*' && peekNext() == '/') {
//                             counter--;
//                         }
//                         Scanner.current++;
//                     }
//                     if (!isAtEnd()) {
//                         // Consume the closing */
//                         Scanner.current += 2;
//                     } else {
//                         print_error("Unterminated multi-line comment");
//                     }
//                 } else {
//                     addToken(SLASH, Scanner.line, &tokens);
//                 }
//                 break;
//             case ' ':
//             case '\r':
//             case '\t':
//                 // Ignore whitespace and newlines
//                 break;
//             case '\n':
//                 Scanner.line++;
//                 break;
//             case '"': string(); addToken(STRING, Scanner.line, &tokens); break;
//             default: 
//                 if (isDigit(c)) {
//                     number();
//                     addToken(NUMBER, Scanner.line, &tokens);
//                 } else if (isAlpha(c)) {
//                     const char* start = Scanner.current - 1;
//                     identifier();
//                     size_t length = Scanner.current - start;
//                     char buffer[256] = {0}; // max identifier length
//                     memcpy(buffer, start, length);
//                     buffer[length] = '\0';
//                     addToken(get_keyword_type(buffer), Scanner.line, &tokens);
//                 } else {
//                     print_error("Unexpected character"); 
//                 }
//                 break;
//         }
//     }
//     addToken(EOF, Scanner.line, &tokens);
//     // iterate through added tokens
//     for(size_t i = 0; i < tokens.size / sizeof(Token); i++) {
//         Token* token = (Token*)array_get(&tokens, i * sizeof(Token));
//         print("Token: ");
//         print(token_type_names[token->type]);
//         print(" at line ");
//         print_int(token->line);
//         print("\n");
//     }

//     // GenerateAst();

//     //write_file("test", "Hej\nhej", 7);
//     // test_visitor_pattern();
//     return 0;
// }