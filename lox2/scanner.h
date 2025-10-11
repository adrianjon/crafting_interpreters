//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_SCANNER_H
#define LOX_SCANNER_H
#include <stdio.h>

#include "list.h"
#include "token.h"

typedef struct {
    char const * start;
    // token_t ** pp_tokens;
    char const * p_previous;
    char const * p_current;
    size_t line;
    // size_t token_count;
} scanner_t;

list_t scan_tokens(scanner_t * p_scanner);

#endif //LOX_SCANNER_H