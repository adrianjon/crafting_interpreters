//
// Created by adrian on 2025-10-12.
//

#include "resolver.h"

int run_resolver_tests(resolver_t * p_resolver) {
    char * test_strings[] = {
        "var a = \"outer\"; { print a; var a = \"inner\"; print a; } print a;",
        "var a = a; // should report \"can't read local variable in its own initializer\"",
        "class A { init() { return 1; } } // init returning a value should be error",
        "class B < A { method() { super.method(); } } // test super resolution",
    };
    return 0;
}
