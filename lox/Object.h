//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_OBJECT_H
#define LOX_OBJECT_H
#include <stdbool.h>

// Forward declarations
typedef struct object object_t;

// Object types
typedef enum {
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_BOOLEAN,
    OBJECT_NATIVE,
    OBJECT_FUNCTION,
    OBJECT_NIL,
} object_type_t;

// API
object_t * new_object(object_type_t p_object_type, void * value);
void object_free(object_t ** p_object);
object_type_t get_object_type (const object_t * p_object);
void set_object_type (object_t * p_object, object_type_t p_object_type);
char * get_object_string (const object_t * p_object);
double get_object_number (const object_t * p_object);
bool get_object_boolean (const object_t * p_object);
void * get_object_function (const object_t * p_object);
void * get_object_value( const object_t * p_object);
void * copy_object_value(const object_t * p_object);

#endif //LOX_OBJECT_H