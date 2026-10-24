# HCL POC requirement traceability (implementation restructured v3)

| Requirement | Implementation | Evidence / status |
|---|---|---|
| Subscriber CRUD | `src/module1.c` | `test/test_module1.c` PASS on Linux |
| ID, plan, status, network | `include/module1.h` | Records stored and loaded |
| Hash table lookup and collisions | `src/module1.c` | Collision test PASS |
| Shared strings and reference counts | `src/module1.c` | Shared string and update/delete tests PASS |
| File handling | `src/module2.c` | Save/load + corrupt-file tests PASS |
| Concurrent operations | `pthread_rwlock_t`, `src/concurrency.c` | 4-thread stress test; not proof of all race freedom |
| Memory monitoring and allocation failures | `db_stats` | Allocation counters; forced failure test pending |
| Large synthetic datasets | `src/benchmark.c` | Bench up to 1,000,000; no SLA established |
| Secure coding | Input bounds, staging load, auth | More audit needed |
| Authentication / RBAC | `src/auth.c` | Admin/viewer demo; not enterprise-ready |
| Static analysis | Cppcheck command in README | Pending approved environment |
| Coverage | gcov in Makefile | Core tests measured; coverage target not fully established |
| MISRA C | Coding review required | NOT verified/certified |
| Valgrind / GDB | Linux tool commands in README | Pending approved Linux server |

Never label a pending item as completed in the final HCL demo.
