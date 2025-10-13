cmake_minimum_required(VERSION 4.0)
project(lox C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
        ${PROJECT_SOURCE_DIR}/lox2
        ${PROJECT_SOURCE_DIR}/lox2/tests
)

add_executable(lox main.c
        extra/Arrays.c
        extra/Memory.c
        extra/Windows.c
        lox/Token.c
        lox/Scanner.c
        lox/Parser.c
        lox/Object.c
        lox/Expr.c
        lox/Stmt.c
        lox/Environment.c
        lox/Interpreter.c
        lox/Function.c
        lox/Resolver.c
        extra/Map.c
        extra/Stack.c
        lox/Class.c
        lox/Callable.c
        lox/Enumerable.h
        extra/Interfaces.h
)

add_executable(ast_generator tools/generate_ast_v2.c)

add_executable(lox2 lox2/lox_main.c
        lox2/expr.c
        lox2/stmt.c
        lox2/token.h
        lox2/scanner.c
        lox2/list.h
        lox2/parser.c
        lox2/interpreter.c
        lox2/resolver.c
        lox2/utils/map.c
        lox2/utils/stack.c
        lox2/environment.c
        lox2/value.h
        lox2/object.h
)

add_executable(test lox2/tests/test_main.c
#        lox2/tests/scanner/test_scanner.c
#        lox2/tests/parser/test_parser.c
#        lox2/tests/resolver/test_resolver.c
#        lox2/expr.c
#        lox2/stmt.c
#        lox2/token.h
#        lox2/scanner.c
#        lox2/list.h
#        lox2/parser.c
#        lox2/interpreter.c
#        lox2/resolver.c
#        lox2/utils/map.c
#        lox2/utils/stack.c
#        lox2/environment.c
        lox2/tests/map/test_map.c
        lox2/tests/map/map2.c
        lox2/tests/map/map2.h
)
