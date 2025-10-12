//
// Created by adrian on 2025-10-11.
//

#ifndef LOX_RESOLVER_H
#define LOX_RESOLVER_H
#include "interpreter.h"
#include "list.h"
#include "../extra/Stack.h"

//////////////////////////////////////////////////////////
typedef enum {
    FUNCTION_TYPE_NONE,
    FUNCTION_TYPE_FUNCTION,
    FUNCTION_TYPE_INITIALIZER,
    FUNCTION_TYPE_METHOD
} function_type_t;

typedef enum {
    CLASS_TYPE_NONE,
    CLASS_TYPE_CLASS,
    CLASS_TYPE_SUBCLASS
} class_type_t;
//////////////////////////////////////////////////////////

typedef struct {
    interpreter_t * interpreter;
    stack_t * scopes; // Stack<map_t*>

    function_type_t current_function;
    class_type_t current_class;
} resolver_t;

void resolve(resolver_t * p_resolver, list_t * p_statements);
void free_resolver(resolver_t * p_resolver);
#endif //LOX_RESOLVER_H