//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_PARSER_H
#define LOX_PARSER_H
#include <stdbool.h>

#include "list.h"
#include "token.h"

typedef struct {
    list_t tokens; // List<stmt_t>
    token_t * p_previous;
    token_t * current_p;
    bool had_error;
} parser_t;

list_t parse(parser_t * p_parser);
#endif //LOX_PARSER_H