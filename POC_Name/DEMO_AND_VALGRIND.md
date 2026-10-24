# 5G Telecom Subscriber Management System

The main menu matches the subscriber lifecycle shown in the state diagram: register, search, update status, delete, exit. Status examples: Registered, Pending Activation, Active, Suspended, Inactive/Terminated. Update status according to the state diagram. TXT autosaving and timestamps remain enabled.

## Windows VS Code
Open POC_Name in VS Code PowerShell:

```powershell
.\build.bat
.\build\telecom_poc.exe
Get-Content .\data\subscribers.txt
Get-Content .\data\operations.txt
```

## Valgrind using VS Code on Windows
Valgrind cannot analyze the Windows MinGW .exe. Use VS Code connected to WSL Ubuntu, or an approved Linux server. WSL requires Windows features and permission to install a distro.

In PowerShell, check WSL: `wsl --list --verbose`. In Ubuntu: `sudo apt update && sudo apt install -y build-essential valgrind libssl-dev`. Open this source directory in WSL (Windows C: drive mounts under /mnt/c/).

```bash
make clean && make
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind-report.txt ./build/telecom_poc
cat valgrind-report.txt
```

Note: The Linux build has not been verified on the user's environment. Linux OpenSSL and pthread dependencies must be available. Always exit the app normally for the final leak report. Do not run Valgrind on build/telecom_poc.exe.
