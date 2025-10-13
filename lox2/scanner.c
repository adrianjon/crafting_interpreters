//
// Created by adrian on 2025-10-11.
//

#include "scanner.h"
#include "token.h"

#include <stdbool.h>

static bool scanner_is_at_end(scanner_t const * p_scanner);
static token_t * scan_token(scanner_t * p_scanner);
list_t scan_tokens(scanner_t * p_scanner) {
    if (!p_scanner || !p_scanner->start) {
        fprintf(stderr, "Error: No source to scan through.");
        exit(EXIT_FAILURE);
    }
    list_t tokens = {0};
    tokens.free_fn = token_free;
    p_scanner->p_current = p_scanner->start;
    p_scanner->p_previous = NULL;
    p_scanner->line = 1;
    while (!scanner_is_at_end(p_scanner)) {
        token_t * p_token = scan_token(p_scanner);
        if (p_token) list_add(&tokens, p_token);
        p_scanner->p_previous = p_scanner->p_current;
    }

    list_add(&tokens, new_token(END_OF_FILE, "", p_scanner->line));
    return tokens;
}

static bool scanner_is_at_end(scanner_t const * p_scanner) {
    return *p_scanner->p_current == '\0';
}
static token_t * scan_token(scanner_t * p_scanner) {
    switch (*p_scanner->p_current) {
        case '(':
            p_scanner->p_current++;
            return new_token(LEFT_PAREN, "(", p_scanner->line);
        case ')':
            p_scanner->p_current++;
            return new_token(RIGHT_PAREN, ")", p_scanner->line);
        case '{':
            p_scanner->p_current++;
            return new_token(LEFT_BRACE, "{", p_scanner->line);
        case '}':
            p_scanner->p_current++;
            return new_token(RIGHT_BRACE, "}", p_scanner->line);
        case ',':
            p_scanner->p_current++;
            return new_token(COMMA, ",", p_scanner->line);
        case '.':
            p_scanner->p_current++;
            return new_token(DOT, ".", p_scanner->line);
        case '+':
            p_scanner->p_current++;
            return new_token(PLUS, "+", p_scanner->line);
        case '-':
            p_scanner->p_current++;
            return new_token(MINUS, "-", p_scanner->line);
        case ';':
            p_scanner->p_current++;
            return new_token(SEMICOLON, ";", p_scanner->line);
        case ':':
            p_scanner->p_current++;
            return new_token(COLON, ":", p_scanner->line);
        case '*':
            p_scanner->p_current++;
            return new_token(STAR, "*", p_scanner->line);
        case '%':
            p_scanner->p_current++;
            return new_token(PERCENTAGE, "%", p_scanner->line);
        case '!':
            bool is_equal = *(p_scanner->p_current + 1) == '=';
            if (is_equal) p_scanner->p_current+=2;
            else p_scanner->p_current+=1;
            return new_token(is_equal ? BANG_EQUAL : BANG,
                is_equal ? "!=" : "!", p_scanner->line);
        case '=':
            is_equal = *(p_scanner->p_current + 1) == '=';
            if (is_equal) p_scanner->p_current+=2;
            else p_scanner->p_current+=1;
            return new_token(is_equal ? EQUAL_EQUAL : EQUAL,
                is_equal ? "==" : "=", p_scanner->line);
        case '<':
            is_equal = *++p_scanner->p_current == '=';
            return new_token(is_equal ? LESS_EQUAL : LESS,
                is_equal ? "<=" : "<", p_scanner->line);
        case '>':
            is_equal = *++p_scanner->p_current == '=';
            return new_token(is_equal ? GREATER_EQUAL : GREATER,
                is_equal ? ">=" : ">", p_scanner->line);
        case '/':
            if (*(p_scanner->p_current + 1) == '/') { // single line comment

                while (!scanner_is_at_end(p_scanner) && *++p_scanner->p_current != '\n') ;
                if (*p_scanner->p_current == '\n') p_scanner->line++;
                // TODO: possibility to save comments as tokens
                // size_t const lexeme_length = p_scanner->p_current - p_scanner->p_previous;
                // char * lexeme = malloc(lexeme_length + 1);
                // memcpy(lexeme, p_scanner->p_previous, lexeme_length);
                // lexeme[lexeme_length] = '\0';
                // token_t * t = new_token(COMMENT, lexeme, p_scanner->line);
                // free(lexeme);
                // return t;

            } else if (*++p_scanner->p_current == '*') { // multi line comment
                p_scanner->p_current++;
                int counter = 1;
                while (!scanner_is_at_end(p_scanner) && counter > 0) {
                    if (*p_scanner->p_current == '\n') p_scanner->line++;
                    if (*(p_scanner->p_current) == '/' && *(p_scanner->p_current + 1) == '*') {
                        counter++;
                        p_scanner->p_current+=2;
                    } else if (*(p_scanner->p_current) == '*' && *(p_scanner->p_current + 1) == '/') {
                        counter--;
                        p_scanner->p_current+=2;
                    }
                }
                if (scanner_is_at_end(p_scanner) && counter != 0) {
                    fprintf(stderr, "Error: Unterminated mult-line comment");
                    exit(EXIT_FAILURE);
                }
            } else {
                return new_token(SLASH, "/", p_scanner->line);
            }
        case ' ':
        case '\r':
        case '\t':
            p_scanner->p_current++;
            break;
        case '\n':
            p_scanner->p_current++;
            p_scanner->line++;
            break;
        case '"':
            p_scanner->p_previous = p_scanner->p_current;
            while (!scanner_is_at_end(p_scanner) && *++p_scanner->p_current != '"')
                if (*p_scanner->p_current == '\\' && *(p_scanner->p_current + 1) == '"')
                    p_scanner->p_current+=1;
            if (scanner_is_at_end(p_scanner) && *p_scanner->p_current != '"') {
                fprintf(stderr, "Error: Unterminated string");
                exit(EXIT_FAILURE);
            }
            p_scanner->p_current++;
            size_t const lexeme_length = p_scanner->p_current - p_scanner->p_previous;
            char * lexeme = malloc(lexeme_length + 1);
            memcpy(lexeme, p_scanner->p_previous, lexeme_length);
            lexeme[lexeme_length] = '\0';
            token_t * t = new_token(STRING, lexeme, p_scanner->line);
            free(lexeme);
            return t;

        default:
            if (*p_scanner->p_current >= '0' && *p_scanner->p_current <= '9') {
                p_scanner->p_previous = p_scanner->p_current;
                while (*p_scanner->p_current >= '0' && *p_scanner->p_current <= '9') {
                    p_scanner->p_current++;
                }
                if (*p_scanner->p_current == '.' &&
                    (*(p_scanner->p_current + 1) >= '0' && *(p_scanner->p_current + 1) <= '9')) {
                    p_scanner->p_current++;
                    while (*p_scanner->p_current >= '0' && *p_scanner->p_current <= '9') {
                        p_scanner->p_current++;
                    }
                }
                size_t const lexeme_length = p_scanner->p_current - p_scanner->p_previous;
                char * lexeme = malloc(lexeme_length + 1);
                memcpy(lexeme, p_scanner->p_previous, lexeme_length);
                lexeme[lexeme_length] = '\0';
                token_t * t = new_token(NUMBER, lexeme, p_scanner->line);
                free(lexeme);
                return t;
            }
            if ((*p_scanner->p_current >= 'A' && *p_scanner->p_current <= 'Z') ||
                (*p_scanner->p_current >= 'a' && *p_scanner->p_current <= 'z')) {
                p_scanner->p_previous = p_scanner->p_current;
                p_scanner->p_current++;
                while ((*p_scanner->p_current >= 'A' && *p_scanner->p_current <= 'Z') ||
                    (*p_scanner->p_current >= 'a' && *p_scanner->p_current <= 'z') ||
                        (*p_scanner->p_current >= '0' && *p_scanner->p_current <= '9')) {
                    p_scanner->p_current++;
                }
                size_t const lexeme_length = p_scanner->p_current - p_scanner->p_previous;
                char * lexeme = malloc(lexeme_length + 1);
                memcpy(lexeme, p_scanner->p_previous, lexeme_length);
                lexeme[lexeme_length] = '\0';
                token_type_t type = IDENTIFIER;
                if (strcmp(lexeme, "if") == 0) type = IF;
                if (strcmp(lexeme, "else") == 0) type = ELSE;
                if (strcmp(lexeme, "while") == 0) type = WHILE;
                if (strcmp(lexeme, "for") == 0) type = FOR;
                if (strcmp(lexeme, "return") == 0) type = RETURN;
                if (strcmp(lexeme, "print") == 0) type = PRINT;
                if (strcmp(lexeme, "and") == 0) type = AND;
                if (strcmp(lexeme, "or") == 0) type = OR;
                if (strcmp(lexeme, "true") == 0) type = KW_TRUE;
                if (strcmp(lexeme, "false") == 0) type = KW_FALSE;
                if (strcmp(lexeme, "nil") == 0) type = NIL;
                if (strcmp(lexeme, "var") == 0) type = VAR;
                if (strcmp(lexeme, "fun") == 0) type = FUN;
                if (strcmp(lexeme, "class") == 0) type = CLASS;
                if (strcmp(lexeme, "super") == 0) type = SUPER;
                if (strcmp(lexeme, "this") == 0) type = KW_THIS;

                token_t * t = new_token(type, lexeme, p_scanner->line);
                free(lexeme);
                return t;
            }
            fprintf(stderr, "Error: Unterminated string");
            exit(EXIT_FAILURE);
    }
    return NULL;
}