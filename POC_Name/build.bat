@echo off
if not exist build mkdir build
if not exist data mkdir data
gcc -std=c11 -O2 -g -Wall -Wextra -Wpedantic -D_XOPEN_SOURCE=700 -Iinclude src\module1.c src\module2.c src\lifecycle.c src\utils.c src\auth.c src\benchmark.c src\concurrency.c src\main.c -o build\telecom_poc.exe -pthread -lbcrypt
if errorlevel 1 (echo BUILD FAILED. & exit /b 1)
echo BUILD SUCCESSFUL. Run: .\build\telecom_poc.exe
