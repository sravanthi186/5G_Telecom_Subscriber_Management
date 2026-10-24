#ifndef TELECOM_CORE_H
#define TELECOM_CORE_H
#include "common.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>

typedef struct Interned { char *text; size_t refs; struct Interned *next; } Interned;
typedef struct { Interned *buckets[TC_POOL_BUCKETS]; size_t unique, bytes; } StringPool;
typedef struct Subscriber { uint64_t id; char name[TC_NAME]; time_t created_at, updated_at; Interned *plan,*status,*network; struct Subscriber *next; } Subscriber;
typedef struct {
    Subscriber **buckets; size_t count, capacity, collisions;
    StringPool pool; pthread_rwlock_t lock;
    size_t allocations, frees, failed_allocations;
} Database;
typedef struct { uint64_t id; char name[TC_NAME],plan[TC_TEXT],status[TC_TEXT],network[TC_TEXT]; time_t created_at, updated_at; } Record;
typedef struct { size_t subscribers, unique_strings, reference_total, string_bytes, approximate_bytes, collisions, allocations, frees, failed_allocations; } Stats;
int db_init(Database *db);
void db_destroy(Database *db);
int db_add(Database *db,const Record *record);
int db_update(Database *db,const Record *record);
int db_delete(Database *db,uint64_t id);
int db_find(Database *db,uint64_t id,Record *out);
int db_stats(Database *db,Stats *out);
int db_foreach(Database *db,int (*visit)(const Record *,void *),void *ctx);
int db_check(Database *db);
const char *tc_error(int code);
#endif
