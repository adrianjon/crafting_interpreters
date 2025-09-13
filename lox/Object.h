#ifndef OBJECT_H
#define OBJECT_H

#include "Memory.h"

// Forward declarations
typedef struct object object_t;

// Object types
typedef enum {
	OBJECT_STRING,
	OBJECT_NUMBER,
	OBJECT_FUNCTION,
	OBJECT_NATIVE,
	OBJECT_CLASS,
	OBJECT_INSTANCE
} object_type_t;

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
			void* function;
		} function;
		struct {
			void* native;
		} native;
		struct {
			char* name;
			void* methods;
		} class;
		struct {
			void* class;
			void* fields;
		} instance;
	} as;
};

#endif // OBJECT_H
