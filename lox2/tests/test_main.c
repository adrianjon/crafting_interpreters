//
// Created by adrian on 2025-10-12.
//


#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <math.h>

#include "../scanner.h"
#include "../parser.h"

extern void run_scanner_tests(scanner_t * p_scanner);
extern void run_parser_tests(parser_t * p_parser);
extern void run_map_tests(void);

int main() {
#ifdef WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // scanner_t scanner = { 0 };
    // run_scanner_tests(&scanner);
    // // The parser is dependent on a working scanner
    // parser_t parser = { 0 };
    // run_parser_tests(&parser);

    run_map_tests();



    return 0;
}
