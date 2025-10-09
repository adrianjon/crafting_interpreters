cmake_minimum_required(VERSION 4.0)
project(lox C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)


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

add_executable(GenerateAst tools/GenerateAst.c
        extra/Arrays.c
        extra/Memory.c
        extra/Windows.c
)