//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_SCANNER_H
#define LOX_SCANNER_H

#include "../extra/Windows.h"
#include "../extra/Arrays.h"
#include "Token.h"

typedef struct {
    file_t *p_target_file;
    const char* p_start;
    const char* p_current;
    int line;
    //void (*init_scanner)(void);
} scanner_t;

// API
int scanner_main(dynamic_array_t* tokens);
void scanner_read_file(const char* filename);
bool scanner_is_at_end(void);
char scanner_advance(void);
char scanner_peek(void);
char scanner_peek_next(void);
const char* scanner_peek_ptr(void);
int scanner_get_line(void);
const char* scanner_previous(void);

void add_token(token_type_t type, const char* lexeme, int line, dynamic_array_t* tokens);
static const size_t MAX_SOURCE_LENGTH = 1024 * 1024; // 1MB
bool match(char expected, char actual);
void string();
bool is_digit(char c);
void number();
bool is_alpha(char c);
bool is_alphanumeric(char c);
void identifier();
token_type_t get_keyword_type(const char* identifier);

void print_tokens(dynamic_array_t* tokens);

#endif //LOX_SCANNER_H