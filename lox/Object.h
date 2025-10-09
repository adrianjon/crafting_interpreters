//
// Created by adrian on 2025-09-15.
//

#ifndef LOX_OBJECT_H
#define LOX_OBJECT_H
#include <stdbool.h>

// Forward declarations
typedef struct object object_t;

extern const char* g_object_type_names[];
// Object types
typedef enum {
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_BOOLEAN,
    OBJECT_FUNCTION,
    OBJECT_CLASS,
    OBJECT_CALLABLE,
    OBJECT_NIL,
} object_type_t;

// API
object_t * new_object(object_type_t p_object_type, void * value);
object_type_t get_object_type (const object_t * p_object);
void set_object_type (object_t * p_object, object_type_t p_object_type);
char * get_object_string (const object_t * p_object);
double get_object_number (const object_t * p_object);
bool get_object_boolean (const object_t * p_object);
void * get_object_value( const object_t * p_object);
void * copy_object_value(const object_t * p_object);
void * call_object(const object_t * p_object, void * p_ctx, object_t ** arguments);
#endif //LOX_OBJECT_H