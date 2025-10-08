//
// Created by adrian on 2025-10-08.
//

#ifndef LOX_INTERFACES_H
#define LOX_INTERFACES_H

typedef size_t (*hash_fn_t)(const void * object);
typedef bool (*cmp_fn_t)(const void * object1, const void * object2);
typedef void * (*cpy_fn_t)(const void * object);

#endif //LOX_INTERFACES_H