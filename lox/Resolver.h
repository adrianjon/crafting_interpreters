//
// Created by adrian on 2025-10-05.
//

#ifndef LOX_RESOLVER_H
#define LOX_RESOLVER_H

#include "Interpreter.h"
#include "Stmt.h"

typedef struct resolver resolver_t;

// Public API
resolver_t * new_resolver(interpreter_t * p_interpreter);
void resolve_list(stmt_t ** pp_stmts, size_t n_stmts, resolver_t * p_resolver);

#endif //LOX_RESOLVER_H