@echo off
REM Compile the source code
cl /GS- /c %1

REM Link the object file
REM Replace main and CONSOLE as needed for your entry point and subsystem
link %~n1.obj /NODEFAULTLIB /ENTRY:main /SUBSYSTEM:CONSOLE kernel32.lib
REM link %~n1.obj /ENTRY:main /SUBSYSTEM:CONSOLE kernel32.lib msvcrt.lib ucrt.lib vcruntime.lib

REM Clean up the .obj file (optional)
del %~n1.obj

REM Run the executable
%~n1.exe