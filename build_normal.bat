@echo off
REM Compile and link all source files in one step
cl /std:c17 /GS- %*

REM cl /GS- main.c extra/Windows.c extra/Memory.c extra/Arrays.c lox/Token.c lox/Scanner.c lox/Parser.c /Fe:main.exe

REM Run the executable (named after the first source file)
%~n1.exe