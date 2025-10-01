//
// Created by adrian on 2025-10-01.
//

#ifndef LOX_FUNCTION_H
#define LOX_FUNCTION_H

#include "Stmt.h"
#include "Object.h"
#include "Interpreter.h"

typedef struct function function_t;

function_t * new_function(stmt_function_t * p_declaration);
stmt_function_t * get_function_declaration(const function_t * p_function);
object_t * call_function(const function_t * p_function, interpreter_t * p_interpreter,
    object_t ** pp_arguments);

#endif //LOX_FUNCTION_H