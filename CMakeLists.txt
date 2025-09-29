cmake_minimum_required(VERSION 4.0)
project(lox C)

set(CMAKE_C_STANDARD 17)


add_executable(lox main.c
        extra/Arrays.h
        extra/Memory.h
        extra/Windows.h
        extra/Arrays.c
        extra/Memory.c
        extra/Windows.c
        lox/Token.c
        lox/Token.h
        lox/Scanner.c
        lox/Scanner.h
        lox/Parser.c
        lox/Parser.h
        lox/Object.c
        lox/Object.h
        lox/Expr.c
        lox/Expr.h
        lox/Stmt.c
        lox/Stmt.h
        lox/Environment.c
        lox/Environment.h
        lox/Interpreter.c
        lox/Interpreter.h)

add_executable(GenerateAst tools/GenerateAst.c
        extra/Arrays.h
        extra/Memory.h
        extra/Windows.h
        extra/Arrays.c
        extra/Memory.c
        extra/Windows.c
        lox/Token.c
        lox/Token.h
        lox/Scanner.c
        lox/Scanner.h
        lox/Object.c
        lox/Object.h)