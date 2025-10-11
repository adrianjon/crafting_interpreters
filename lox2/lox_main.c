//
// Created by adrian on 2025-10-11.
//

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include "../tools/generate_ast_v2.h"

#include <stdio.h>
#include <stdlib.h>

void build_ast(void) {
    char const * target_dir = "./lox2";
    if (!generate_ast(target_dir, "expr", g_ast_expr_grammar) ||
        !generate_ast(target_dir, "stmt", g_ast_stmt_grammar)) {
        fprintf(stderr, "Failed to generate ast\n");
        exit(EXIT_FAILURE);
    }
}

int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    build_ast();

    return 0;
}

