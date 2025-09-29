//
// Created by adrian on 2025-09-15.
//

#include "Object.h"

#include <stdlib.h>

#include "../extra/Memory.h"

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
            void* function;
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
        case OBJECT_FUNCTION:
            p_object->as.function.function = value;
            break;
        default:
            break;
    }
    return p_object;
}
void object_free(object_t ** p_object) {
    switch ((*p_object)->type) {
        case OBJECT_STRING:
            free((*p_object)->as.string.value);
            break;
        case OBJECT_NUMBER:
        case OBJECT_BOOLEAN:
            break;
        case OBJECT_FUNCTION:
            free((*p_object)->as.function.function);
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
    return p_object->as.function.function;
}