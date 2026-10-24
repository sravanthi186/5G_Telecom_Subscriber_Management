#include "module1.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <inttypes.h>
#include <string.h>
static double seconds(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter))
        return 0.0;
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return 0.0;
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
#endif
}
int benchmark_run(size_t n){if(n<1||n>1000000){puts("Dataset size must be between 1 and 1000000.");return 1;}Database db;if(db_init(&db)!=TC_OK)return 1;static const char *plans[]={"5G Unlimited","5G Basic","5G Premium","5G Family"};static const char *statuses[]={"Active","Suspended","Inactive"};static const char *networks[]={"5G SA","5G NSA","4G LTE"};double start=seconds();for(size_t i=0;i<n;i++){Record r={0};r.id=(uint64_t)i+1;snprintf(r.name,sizeof(r.name),"Synthetic_User_%zu",i+1);strcpy(r.plan,plans[i%4]);strcpy(r.status,statuses[i%3]);strcpy(r.network,networks[i%3]);if(db_add(&db,&r)!=TC_OK){fprintf(stderr,"Insertion failed at %zu\n",i);db_destroy(&db);return 1;}}double insert=seconds()-start;Record result;start=seconds();size_t hits=0;for(size_t i=0;i<n;i++)if(db_find(&db,(uint64_t)((i*7919u)%n)+1,&result)==TC_OK)hits++;double lookup=seconds()-start;Stats s;db_stats(&db,&s);size_t baseline=0;/* Compare only repeated text storage, not whole subscriber struct. */
for(size_t i=0;i<n;i++){baseline+=strlen(plans[i%4])+1+strlen(statuses[i%3])+1+strlen(networks[i%3])+1;}
printf("\n=== SYNTHETIC PERFORMANCE BENCHMARK ===\nRecords: %zu | Search hits: %zu\n",n,hits);
printf("Insert total: %.3f s | Lookup total: %.3f s\n",insert,lookup);
printf("Average lookup: %.3f microseconds\n",lookup*1e6/n);
printf("Conventional repeated-string bytes (estimated): %zu\n",baseline);
printf("Interned text bytes (measured): %zu\n",s.string_bytes);
printf("Unique interned strings: %zu | Hash collisions on insert: %zu\n",s.unique_strings,s.collisions);
if(baseline)printf("Text payload reduction: %.4f%% (excluding pool metadata)\n",100.0*(1.0-(double)s.string_bytes/(double)baseline));
printf("Total estimated application-owned bytes: %zu (includes pool overhead)\n",s.approximate_bytes);
int rc=db_check(&db);db_destroy(&db);return rc==TC_OK?0:1;}
