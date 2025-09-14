#include "extra/Memory.h"
#include "extra/Arrays.h"
#include "extra/Windows.h"
#include "lox/Token.h"
#include "lox/Scanner.h"

// #include "tools/GenerateAst.c"

int main(void) {


    // TODO: This whole process should be in a function in scanner.c
    scanner_read_file("lox.txt");

    dynamic_array_t tokens = create_array(1024 * 1); // initial capacity for 1KB

    while(!scanner_is_at_end()) {
        char c = scanner_advance();
        bool is_equal;
        //print_char(c);
        switch (c) {
            case '(': add_token(LEFT_PAREN, "(", scanner_get_line(), &tokens); break;
            case ')': add_token(RIGHT_PAREN, ")", scanner_get_line(), &tokens); break;
            case '{': add_token(LEFT_BRACE, "{", scanner_get_line(), &tokens); break;
            case '}': add_token(RIGHT_BRACE, "}", scanner_get_line(), &tokens); break;
            case ',': add_token(COMMA, ",", scanner_get_line(), &tokens); break;
            case '.': add_token(DOT, ".", scanner_get_line(), &tokens); break;
            case '+': add_token(PLUS, "+", scanner_get_line(), &tokens); break;
            case '-': add_token(MINUS, "-", scanner_get_line(), &tokens); break;
            case ';': add_token(SEMICOLON, ";", scanner_get_line(), &tokens); break;
            case ':': add_token(COLON, ":", scanner_get_line(), &tokens); break;
            case '*': add_token(STAR, "*", scanner_get_line(), &tokens); break;
            case '!':
                is_equal = match('=', scanner_peek());
                add_token(is_equal ? BANG_EQUAL : BANG, is_equal ? "!=" : "!", scanner_get_line(), &tokens);
                break;
            // TODO: fix this bug here in case of == first match returns EQUAL (should be EQUAL_EQUAL) second returns ==
            case '=':
                is_equal = match('=', scanner_peek());
                add_token(is_equal ? EQUAL_EQUAL : EQUAL, is_equal ? "==" : "=", scanner_get_line(), &tokens);
                break;
            case '<':
                is_equal = match('=', scanner_peek());
                add_token(is_equal ? LESS_EQUAL : LESS, is_equal ? "<=" : "<", scanner_get_line(), &tokens);
                break;
            case '>':
                is_equal = match('=', scanner_peek());
                add_token(is_equal ? GREATER_EQUAL : GREATER, is_equal ? ">=" : ">", scanner_get_line(), &tokens);
                break;
            case '/':
                if (match('/', scanner_peek())) {
                    // Handle single-line comment
                    while (!scanner_is_at_end() && scanner_peek() != '\n') {
                        scanner_advance();
                    }
                } else if (match('*', scanner_peek())) {
                    // Handle multi-line comment
                    unsigned int counter = 1; // counter for nested multi-line comments
                    while (!scanner_is_at_end() && counter > 0) {
                        if (scanner_peek() == '/' && scanner_peek_next() == '*') {
                            counter++;
                        } else if (scanner_peek() == '*' && scanner_peek_next() == '/') {
                            counter--;
                        }
                        scanner_advance();
                    }
                    if (!scanner_is_at_end()) {
                        // Consume the closing */
                        scanner_advance();
                        scanner_advance();
                    } else {
                        print_error("Unterminated multi-line comment");
                    }
                } else {
                    add_token(SLASH, NULL, scanner_get_line(), &tokens);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                // Ignore whitespace and newlines
                break;
            case '"':
                const char* start = scanner_previous();
                string();
                size_t length = scanner_peek_ptr() - start;
                char buffer[256] = {0}; // max string length
                memory_copy(buffer, start + 1, length - 2); // exclude quotes
                buffer[length - 2] = '\0';

                add_token(STRING, (const char*)buffer, scanner_get_line(), &tokens); break;
            default: 
                if (is_digit(c)) {
                    const char* start = scanner_previous();
                    number();
                    size_t length = scanner_peek_ptr() - start;
                    char buffer[256] = {0}; // max number length
                    memory_copy(buffer, start, length);
                    buffer[length] = '\0';
                    // printf("number length: %zu\n", length);
                    // printf("start char: '%c'\n", *start);
                    // printf("Number: %s\n", buffer);
                    add_token(NUMBER, (const char*)buffer, scanner_get_line(), &tokens);
                } else if (is_alpha(c)) {
                    const char* start = scanner_previous();
                    identifier();
                    size_t length = scanner_peek_ptr() - start;
                    char buffer[256] = {0}; // max identifier length
                    memory_copy(buffer, start, length);
                    buffer[length] = '\0';
                    token_type_t type = get_keyword_type(buffer);
                    printf("buffer: '%s' length: %zu\n", buffer, length);
                    add_token(type, (const char*)buffer, scanner_get_line(), &tokens);
                } else {
                    print_error("Unexpected character"); 
                }
                break;
        }
    }
    add_token(EOF_, NULL, scanner_get_line(), &tokens);
    print_tokens(&tokens);


    // debug_memory_pool();
    // GenerateAst();
    // memory_free(&g_scanner.target_file->buffer);
    // memory_free(&tokens.data);
    // memory_free(&g_scanner.target_file);
    // debug_memory_pool();
    return 0;
}