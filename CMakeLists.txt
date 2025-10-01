cmake_minimum_required(VERSION 4.0)
project(lox C)

set(CMAKE_C_STANDARD 17)


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
)

add_executable(GenerateAst tools/GenerateAst.c
        extra/Arrays.c
        extra/Memory.c
        extra/Windows.c
)