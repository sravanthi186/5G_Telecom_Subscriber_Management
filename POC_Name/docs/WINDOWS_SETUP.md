# Standalone MinGW64 quick start

Tested Linux build; Windows changes prepared for MinGW-w64 GCC 15.2.0 (x86_64-msvcrt-posix-seh) but Windows execution is **not** verified here.

In VS Code PowerShell open this folder (`telecom_capstone_pro`) and run:

```powershell
where.exe gcc
gcc --version
.\build.bat
.\bin\telecom_poc.exe
.\test.bat
```

The Windows build uses built-in BCrypt/CNG and QueryPerformanceCounter. No OpenSSL installation or MSYS2 pacman required. GCC's POSIX threading distribution should provide `pthread.h` and `libwinpthread`.

If the build fails, send the full compiler output. Never run the program if BUILD FAILED.

**Security limitation:** local account storage and audit logging are for a training demonstration, not production-grade telecom deployment. Do not use actual subscriber personal data.
