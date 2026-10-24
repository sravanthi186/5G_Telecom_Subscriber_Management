# 5G Telecom Subscriber Management System

## Windows (VS Code PowerShell)
Open POC_Name. Run `./build.bat` then `./build/telecom_poc.exe` (PowerShell accepts `.\\build.bat` and `.\\build\\telecom_poc.exe`). Run `./test.bat` for the C test suites.

## Menu
1 Add, 2 Search, 3 Update, 4 Activate, 5 Deactivate, 6 Recharge, 7 Check status, 8 Delete, 9 View all, 0 Save and exit.

## Business rule
90 days since the most recent recharge, or since registration if no recharge exists, triggers Inactive on a status check/search. A valid recharge reactivates eligible subscribers. Suspended and Terminated accounts cannot recharge. The threshold is a simplified demonstration policy, not a carrier-wide regulatory rule.

## Files
- `data/subscribers.txt`: current records, status, created and updated times.
- `data/operations.txt`: timestamped audit history.
- `data/recharges.txt`: recharge history in `ID|UNIX_TIMESTAMP|AMOUNT` format.
- `data/subscribers.db`: persistent subscriber data.
- `data/users.auth`: credential storage (do not share contents).

Changes to subscriber records auto-save to subscriber text and DB. Recharge history is appended at recharge time. For demonstration, use dummy data.

## Unit tests
`test/test_all_gtest.cpp` contains Google Test cases for subscriber CRUD, validation, hash lookup, shared-string reference counts, persistence, status changes, recharge, 90-day deactivation, admin/viewer authentication and diagnostics. These call the real project functions, not sample stand-ins.

**GTest requires a Linux environment with Google Test installed**. On Ubuntu/WSL with approved installation: `sudo apt install libgtest-dev build-essential libssl-dev`, then `make gtest`. On Windows without Google Test, use `./test.bat` to run the included C unit tests. `make coverage` uses gcov on Linux.

## Valgrind
Valgrind needs a Linux executable (WSL/Ubuntu or Linux server), not the MinGW `.exe`. In Ubuntu, build using `make`, then run `valgrind --leak-check=full --show-leak-kinds=all ./build/telecom_poc`.

## Limits
Automatic inactivity is evaluated on search/status check; it does not run as a background service. The sample recharge ledger is append-only and is not transactionally coupled with the subscriber DB. This is a training POC, not a production telecom billing system. Google Test execution and Windows MinGW compilation have not been verified here.
