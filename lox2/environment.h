//
// Created by adrian on 2025-10-12.
//

#ifndef LOX_ENVIRONMENT_H
#define LOX_ENVIRONMENT_H

#include <stdbool.h>
#include "value.h"
#include "../tests/map/map2.h"


/* value_t is your runtime value representation. Keep it opaque here. */
//typedef struct value value_t;

/* Environment represents a lexical environment (a map of name -> value)
 * and a link to an enclosing Environment.
 */
typedef struct environment {
    map_t * values;                 /* map from const char* -> value_t* (or boxed value) */
    struct environment * enclosing; /* NULL for global environment */
} environment_t;

/* Create an environment. If enclosing is non-NULL, this environment chains to it. */
environment_t * environment_create(environment_t * enclosing);
/* Destroy an environment (does not destroy enclosing). Frees the map. */
void environment_destroy(environment_t * env);

/* Define a name in this environment (creates/overwrites in this environment) */
void environment_define(environment_t *env,  char const * name, value_t * value);

/* Get a name in the current environment chain. Returns NULL if not found. */
value_t * environment_get(environment_t const * env,  char const * name);

/* Get a name at a lexical distance (0 = current env, 1 = immediate enclosing, ...).
 * Returns NULL if not found at that depth. */
value_t * environment_get_at(environment_t * env, int distance,  char const * name);

/* Assign to an existing name in the chain. Returns true on success, false if not found. */
bool environment_assign(environment_t const * env,  char const * name, value_t * value);

/* Assign at a lexical distance (0 = current env, ...). Returns true on success. */
bool environment_assign_at(environment_t * env, int distance,
     char const * name, value_t * value);


#endif //LOX_ENVIRONMENT_H