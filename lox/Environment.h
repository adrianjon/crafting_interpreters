//
// Created by adrian on 2025-09-25.
//

#ifndef LOX_ENVIRONMENT_H
#define LOX_ENVIRONMENT_H

#include "ast_interpreter.h"

typedef struct environment environment_t;
#define MAX_GLOBALS 128
#define MAX_VARS 128

void set_global(const char * name, value_t * value);
value_t * get_global(const char * name);
void free_globals(void);

#endif //LOX_ENVIRONMENT_H