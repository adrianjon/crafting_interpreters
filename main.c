#include "extra/Memory.h"
#include "extra/Arrays.h"
#include "extra/Windows.h"
#include "lox/Scanner.h"
#include "lox/Parser.c"
// #include "tools/GenerateAst.c"

int main(void) {

    scanner_read_file("lox.txt");

    dynamic_array_t tokens = create_array(1024 * 1); // initial capacity for 1KB

    while(!scanner_is_at_end()) {
        char c = scanner_advance();
        //print_char(c);
        switch (c) {
            case '(': add_token(LEFT_PAREN, scanner_get_line(), &tokens); break;
            case ')': add_token(RIGHT_PAREN, scanner_get_line(), &tokens); break;
            case '{': add_token(LEFT_BRACE, scanner_get_line(), &tokens); break;
            case '}': add_token(RIGHT_BRACE, scanner_get_line(), &tokens); break;
            case ',': add_token(COMMA, scanner_get_line(), &tokens); break;
            case '.': add_token(DOT, scanner_get_line(), &tokens); break;
            case '+': add_token(PLUS, scanner_get_line(), &tokens); break;
            case '-': add_token(MINUS, scanner_get_line(), &tokens); break;
            case ';': add_token(SEMICOLON, scanner_get_line(), &tokens); break;
            case ':': add_token(COLON, scanner_get_line(), &tokens); break;
            case '*': add_token(STAR, scanner_get_line(), &tokens); break;
            case '!': add_token(match('=', scanner_peek()) ? BANG_EQUAL : BANG, scanner_get_line(), &tokens); break;
            case '=': add_token(match('=', scanner_peek()) ? EQUAL_EQUAL : EQUAL, scanner_get_line(), &tokens); break;
            case '<': add_token(match('=', scanner_peek()) ? LESS_EQUAL : LESS, scanner_get_line(), &tokens); break;
            case '>': add_token(match('=', scanner_peek()) ? GREATER_EQUAL : GREATER, scanner_get_line(), &tokens); break;
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
                    add_token(SLASH, scanner_get_line(), &tokens);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                // Ignore whitespace and newlines
                break;
            case '"': string(); add_token(STRING, scanner_get_line(), &tokens); break;
            default: 
                if (is_digit(c)) {
                    number();
                    add_token(NUMBER, scanner_get_line(), &tokens);
                } else if (is_alpha(c)) {
                    const char* start = scanner_previous();
                    identifier();
                    size_t length = scanner_peek_ptr() - start; //change scanner_peek() to return a ptr
                    char buffer[256] = {0}; // max identifier length
                    memory_copy(buffer, start, length);
                    buffer[length] = '\0';
                    add_token(get_keyword_type(buffer), scanner_get_line(), &tokens);
                } else {
                    print_error("Unexpected character"); 
                }
                break;
        }
    }
    add_token(EOF, scanner_get_line(), &tokens);
    print_tokens(&tokens);


    // debug_memory_pool();
    // GenerateAst();
    // memory_free(&g_scanner.target_file->buffer);
    // memory_free(&tokens.data);
    // memory_free(&g_scanner.target_file);
    // debug_memory_pool();
    return 0;
}