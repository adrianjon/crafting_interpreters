//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_RESOLVER_H
#define LOX_RESOLVER_H
#include "interpreter.h"
#include "list.h"

typedef struct {
    interpreter_t interpreter;
} resolver_t;

void resolve(resolver_t * p_resolver, list_t * p_statements);
#endif //LOX_RESOLVER_H