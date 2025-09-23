//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_SCANNER_H
#define LOX_SCANNER_H

#include "../extra/Arrays.h"

typedef struct scanner scanner_t;

// new scanner API
scanner_t * scanner_init(const char * filename);
void scanner_scan(scanner_t * p_scanner);
const dynamic_array_t * scanner_get_tokens(const scanner_t * p_scanner);
void scanner_print_tokens(const scanner_t * p_scanner);
void scanner_free(scanner_t * p_scanner);

#endif //LOX_SCANNER_H