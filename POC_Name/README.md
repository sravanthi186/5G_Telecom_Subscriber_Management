# 5G Telecom Subscriber Management — C Capstone POC (restructured v3)

**Purpose:** Demonstrate memory-efficient storage of synthetic telecom subscriber metadata using hash lookup, string interning, reference counting, concurrency and evidence-based performance testing. This is **not** a real 5G core, SIM provisioning system, or production-ready subscriber database.

## What's implemented

- Modular C code: `module1` (subscriber/hash/string pool), `module2` (file persistence), `auth`, `benchmark`, `concurrency`, CLI.
- Add, find, update, delete subscriber ID/name/plan/status/network.
- Hash table with separate chaining and fixed 65,537 buckets; full ID comparison and collision metrics.
- Second hash table for interned plan/status/network strings, reference counting, and release at zero.
- Transactional field updates on allocation failure (old values remain until all new interned strings acquired).
- Multi-reader/single-writer lock (`pthread_rwlock_t`) protecting both hash tables, records and reference counts. Four-worker stress demonstration and tests.
- Versioned, pipe-delimited file persistence; rejects malformed input and duplicate IDs, loads via a staging DB so failures do not overwrite the current DB. Save uses a temporary file, flush/sync, and replacement; this is not a full transactional database.
- First-run administrator setup; PBKDF2-HMAC-SHA256 salted password hash via OpenSSL; admin/viewer roles; admin can create a read-only viewer; hidden password input on terminals; 3 login attempts per run. **Basic demonstration RBAC only, not enterprise authentication.**
- Local append-only *demo* audit log (not tamper-proof).
- Synthetic benchmark up to 1,000,000 records (subject to machine memory). Reports timings and **text-payload-only** savings, plus approximate total DB memory. No invented numbers.
- GCC warnings, automated CRUD/validation/persistence/collision/concurrency tests, sanitizer and gcov instructions.

## Windows setup (standalone MinGW-w64 GCC 15.2.0)

This version supports the user's **standalone MinGW-w64 x86_64-msvcrt-posix-seh** compiler, without MSYS2, `pacman`, or OpenSSL on Windows. Authentication uses the Windows CNG `bcrypt` system library (PBKDF2-HMAC-SHA256, salted). The Windows build links `-lbcrypt` and `-pthread`. It requires the compiler's Windows SDK-compatible `bcrypt.h` and import library `libbcrypt.a`, normally included in current MinGW-w64 distributions.

In VS Code **PowerShell**, from the directory containing `build.bat`:

```powershell
gcc --version
.\build.bat
.\bin\telecom_poc.exe
.\test.bat
```

The Windows benchmark uses `QueryPerformanceCounter`, not `timespec_get`, for wall-clock timing. Keep this entire extracted folder intact; it creates `data/` for local database and credentials. Do not copy older `data/users.auth` unless you intentionally want to keep those credentials; the stored PBKDF2 hash format is interoperable with the Linux version.

**Important:** The Windows binary cannot be compiled or executed in this Linux workspace, so Windows compatibility is implemented but must be confirmed on your MinGW64 machine. If the Windows compiler reports a new error, share its exact output.

## Linux setup

Install or use the approved GCC, pthread support, OpenSSL development headers, `make`, and optional Valgrind, Cppcheck, gcov. In the project folder:

```bash
make
make test
./build/telecom_poc
make coverage
valgrind --leak-check=full --show-leak-kinds=all ./build/test_core
cppcheck --enable=all --inconclusive -Iinclude src tests
```

The Valgrind/C++check results must be collected on your actual environment. MISRA C **has not** been formally certified; GCC/Cppcheck do not prove MISRA compliance.

## First run

1. Keep the `data/` folder next to the executable's working directory (run from project root).
2. Start the application; create an administrator username and a password at least 12 characters long.
3. Sign in; use menu option 1 to add a subscriber, 2 to search, 5 to view analytics, 9 to run a synthetic benchmark, and 10 for concurrent stress.
4. Use menu 12 as admin to create a read-only viewer account. Viewer can search and view reports but cannot change stored records.
5. Use option 0 to save and exit. Run again and search for a previously saved record.

**Do not use real subscriber/customer data or reuse company passwords.** The application does not provide secure multiuser provisioning, encryption at rest, durable audit guarantees, or recovery from every power-loss scenario.

## Suggested demonstration

1. Create subscribers 1001 and 1002 with identical plan/status/network; analytics shows 2 subscribers, 3 unique strings, 6 references.
2. Update 1001's plan and show reference counts are correctly adjusted via tests.
3. Run benchmark with 10,000 then 100,000 synthetic records. Show the measured numbers; clarify that string payload reduction excludes intern pool overhead.
4. Run concurrent operations test, then integrity diagnostics.
5. Run `make test`, gcov, Valgrind and Cppcheck on the Linux server when available. Save authentic output/screenshots.

## Current limitations and remaining work before HCL final review

- No dynamic hash-table resizing. With one million records, fixed bucket count gives more collisions; benchmark the actual workload.
- No failure-injection harness for `malloc` and disk errors; no extensive fuzz tests or cross-platform Windows CI yet.
- Password account file and demo audit log are local files. Role management is minimal, audit logging isn't tamper-resistant, and Linux access permissions depend on deployment setup.
- Windows build has **not** been executed in this environment. Linux GCC build, core tests and sanitizer test have been executed.
- No actual multi-million record run or formal performance SLA. The benchmark supports up to 1 million synthetic records, not an assertion of telecom-scale readiness.
- MISRA C formal compliance and HCL-specific acceptance must be verified separately.
- The console interface intentionally follows the HLD. GUI not required by the POC screenshots.

## Source layout

`include/` public interfaces; `src/module1.c` subscriber and string-pool internals; `src/module2.c` persistence; `src/auth.c` login/RBAC; `src/benchmark.c` measurements; `src/concurrency.c` stress; `src/main.c` console UI; `test/test_module1.c` regression tests; `docs/` notes; `build.bat`, `test.bat`, `Makefile` build helpers.

## Required HCL directory layout (restructured)

- `include/common.h`: shared constants and status codes.
- `include/module1.h`, `src/module1.c`: subscriber CRUD, hash lookup, shared-string interning, reference counting, statistics, concurrency locks.
- `include/module2.h`, `src/module2.c`: file persistence and reload.
- `src/utils.c`: error message helper.
- `src/main.c`: console UI.
- `src/auth.c`, `src/benchmark.c`, `src/concurrency.c`: extra source modules required to preserve the full POC features.
- `test/test_module1.c`, `test/test_module2.c`, `test/test_auth.c`: unit and integration tests.
- `build/`: compiled executables and coverage artifacts.

Windows PowerShell (standalone MinGW64): `./build.bat`, `./build/telecom_poc.exe`, `./test.bat`.
Linux with approved OpenSSL development headers: `make`, `make test`, `./build/telecom_poc`.
Valgrind only works with the Linux binary; for example: `valgrind --leak-check=full --show-leak-kinds=all ./build/test_module1`.


## Version 5: immediate timestamps and text files

Every successful Add, Update, or Delete prints the record details and operation timestamp immediately, then automatically writes `data/subscribers.db` (machine-readable persistence) and `data/subscribers.txt` (human-readable current subscriber snapshot). Searches print record details and the current operation timestamp. `data/operations.txt` appends a timestamped audit trail including deleted record details. Passwords are never written to these text reports.

The database format is `TC_SUBSCRIBERS_V2` with creation and update epochs; older `TC_SUBSCRIBERS_V1` data can still be loaded, with timestamps initialized when imported. Files are relative to the working directory; run the application from the `POC_Name` folder. Because `.txt` files contain subscriber data, use sample/test data only unless your organization explicitly approves this storage approach.
