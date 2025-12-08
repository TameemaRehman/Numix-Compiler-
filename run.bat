@echo off
REM Helper batch file to run the compiler easily
REM Usage: run [source-file] [options]

set COMPILER=MathScript-Compiler-main\bin\mathseqc.exe

if not exist "%COMPILER%" (
    echo Error: Compiler not found at %COMPILER%
    echo Please build it first with: make release
    exit /b 1
)

"%COMPILER%" %*

