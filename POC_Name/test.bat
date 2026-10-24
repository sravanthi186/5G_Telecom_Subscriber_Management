@echo off
if not exist build mkdir build
if not exist data mkdir data
gcc -std=c11 -O0 -g -Wall -Wextra -Wpedantic -D_XOPEN_SOURCE=700 -Iinclude src\module1.c src\module2.c src\lifecycle.c src\utils.c test\test_module1.c -o build\test_module1.exe -pthread
if errorlevel 1 exit /b 1
build\test_module1.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O0 -g -Wall -Wextra -Wpedantic -D_XOPEN_SOURCE=700 -Iinclude src\module1.c src\module2.c src\lifecycle.c src\utils.c test\test_module2.c -o build\test_module2.exe -pthread
if errorlevel 1 exit /b 1
build\test_module2.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O0 -g -Wall -Wextra -Iinclude src\auth.c test\test_auth.c -o build\test_auth.exe -lbcrypt
if errorlevel 1 exit /b 1
build\test_auth.exe

gcc -std=c11 -O0 -g -Wall -Wextra -D_XOPEN_SOURCE=700 -Iinclude src\module1.c src\module2.c src\lifecycle.c src\utils.c test\test_lifecycle.c -o build\test_lifecycle.exe -pthread
if errorlevel 1 exit /b 1
build\test_lifecycle.exe
if errorlevel 1 exit /b 1
