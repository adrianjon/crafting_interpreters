//
// Created by adrian on 2025-09-15.
//

#include "Object.h"
#include "Callable.h"
#include <stdlib.h>
#include <string.h>

#include "Class.h"
#include "../extra/Memory.h"
#include "Function.h"

const char* g_object_type_names[] = {
    "OBJECT_STRING",
    "OBJECT_NUMBER",
    "OBJECT_BOOLEAN",
    "OBJECT_FUNCTION",
    "OBJECT_CLASS",
    "OBJECT_CALLABLE",
    "OBJECT_INSTANCE"
    "OBJECT_NIL",
};
// all callable types must start with a callable_vtable_t
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
            callable_vtable_t * vtable;
            function_t * function;
        } function;
        struct {
            callable_vtable_t * vtable;
            class_t * class;
        } class;
        struct {
            callable_vtable_t * vtable;
        } callable;
        struct {
            instance_t * instance;
        } instance;
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
        case OBJECT_FUNCTION:
            p_object->as.callable.vtable = get_function_vtable();
            p_object->as.function.function = value;
            break;
        case OBJECT_CLASS:
            p_object->as.callable.vtable = get_class_vtable();
            p_object->as.class.class = value;
            break;
        case OBJECT_CALLABLE:
            p_object->as.callable.vtable = NULL;
            break;
        case OBJECT_INSTANCE:
            p_object->as.instance.instance = value;
            break;
        case OBJECT_NIL:
        default:
            break;
    }
    return p_object;
}

object_type_t get_object_type (const object_t * p_object) {
    if (!p_object) return OBJECT_NIL;
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
void * get_object_value(const object_t * p_object) {
    switch (p_object->type) {
        case OBJECT_STRING:
            return p_object->as.string.value;
        case OBJECT_NUMBER:
            return (void*)(intptr_t)p_object->as.number.value;
        case OBJECT_BOOLEAN:
            return (void*)p_object->as.boolean.value;
        case OBJECT_FUNCTION:
            return p_object->as.function.function;
        case OBJECT_CLASS:
            return p_object->as.class.class;
        case OBJECT_CALLABLE:
            return NULL;
        case OBJECT_INSTANCE:
            return p_object->as.instance.instance;
        case OBJECT_NIL:
        default:
            break;
    }
    return NULL;
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
        default:
            break;
    }
    return p_value;
}

void * call_object(const object_t * p_object, void * p_ctx, object_t ** arguments) {
    if (p_object && p_object->type == OBJECT_CALLABLE ||
        p_object && p_object->type == OBJECT_FUNCTION ||
        p_object && p_object->type == OBJECT_CLASS) {
        return p_object->as.callable.vtable->call(p_object, p_ctx, arguments);
    }
    return NULL;
}