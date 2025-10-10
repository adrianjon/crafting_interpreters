//
// Created by adrian on 2025-10-06.
//

#ifndef LOX_CLASS_H
#define LOX_CLASS_H
#include "Function.h"
#include "Interpreter.h"
#include "Object.h"

typedef struct class class_t;

class_t * new_class(const char * name);
callable_vtable_t * get_class_vtable(void);
void class_add_method(class_t * p_class, const char * key, function_t * value);
#endif //LOX_CLASS_H