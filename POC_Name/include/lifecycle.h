#ifndef TC_LIFECYCLE_H
#define TC_LIFECYCLE_H
#include "module1.h"
#define TC_INACTIVE_DAYS 90
int lifecycle_days(time_t last, time_t now);
int lifecycle_expired(time_t last, time_t now);
int lifecycle_recharge(Database *db, uint64_t id, double amount, time_t when, const char *path);
int lifecycle_last_recharge(uint64_t id, const char *path, time_t *when, double *amount);
int lifecycle_change_status(Database *db, uint64_t id, const char *status);
int lifecycle_check(Database *db, uint64_t id, const char *path, time_t now, Record *out);
#endif
