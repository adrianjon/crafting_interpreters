//
// Created by adrian on 2025-09-15.
//

#include "Object.h"

#include <stdlib.h>
#include <string.h>

#include "../extra/Memory.h"
#include "Function.h"

struct object {
    object_type_t type;
    union {
        struct {
            char* value;
        } string;
        struct {
            double value;
        } number;
        struct {
            bool value;
        } boolean;
        struct {
            object_t * (*fn)(int argc, object_t ** argv);
            int arity;
            char * name;
        } native;
        struct {
            function_t * p_function;
        } function;
    } as;
};

// Public API
object_t * new_object(const object_type_t p_object_type, void * value) {
    object_t * p_object = memory_allocate(sizeof(object_t));
    p_object->type = p_object_type;
    switch (p_object_type) {
        case OBJECT_STRING:
            p_object->as.string.value = value;
            break;
        case OBJECT_NUMBER:
            p_object->as.number.value = *(double*)value;
            break;
        case OBJECT_BOOLEAN:
            p_object->as.boolean.value = *(bool*)value;
            break;
        case OBJECT_NATIVE:
            break;
        case OBJECT_FUNCTION:
            p_object->as.function.p_function = value;
            break;
        case OBJECT_NIL:
        default:
            break;
    }
    return p_object;
}
void object_free(object_t ** p_object) {
    switch ((*p_object)->type) {
        case OBJECT_STRING:
            memory_free((void**)&(*p_object)->as.string.value);
            break;
        case OBJECT_NUMBER:
        case OBJECT_BOOLEAN:
            break;
        case OBJECT_NATIVE:
            memory_free((void**)&(*p_object)->as.native.name);
            break;
        case OBJECT_FUNCTION:
            memory_free(&(*p_object)->as.function.p_function);
            break;
        default:
            break;
    }
    memory_free((void**)p_object);
}
object_type_t get_object_type (const object_t * p_object) {
    return p_object->type;
}
void set_object_type (object_t * p_object, const object_type_t p_object_type) {
    p_object->type = p_object_type;
}
// Not copied string
char * get_object_string (const object_t * p_object) {
    return p_object->as.string.value;
}
double get_object_number (const object_t * p_object) {
    return p_object->as.number.value;
}
bool get_object_boolean (const object_t * p_object) {
    return p_object->as.boolean.value;
}
void * get_object_function (const object_t * p_object) {
    return p_object->as.function.p_function;
}
void * copy_object_value( const object_t * p_object) {
    void * p_value = NULL;
    switch (p_object->type) {
        case OBJECT_STRING:
            const size_t len = strlen(p_object->as.string.value);
            p_value = memory_allocate((len + 1) * sizeof(char));
            memory_copy(p_value, p_object->as.string.value, len);
            char * str = p_value;
            str[len] = '\0';
            break;
        case OBJECT_NUMBER:
            p_value = memory_allocate(sizeof(double));
            *(double*)p_value = p_object->as.number.value;
            break;
        case OBJECT_BOOLEAN:
            p_value = memory_allocate(sizeof(bool));
            *(bool*)p_value = p_object->as.boolean.value;
            break;
        case OBJECT_NATIVE:
            break;
        case OBJECT_FUNCTION:
            p_value = new_function(get_function_declaration(p_object->as.function.p_function));
            break;
        default:
            break;
    }
    return p_value;
}