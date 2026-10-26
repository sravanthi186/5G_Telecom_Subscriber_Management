#include "module1.h"
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#define WORKERS 4
#define OPERATIONS 2000u
struct Job { Database *db; unsigned index; unsigned errors; };
static void *worker(void *arg){struct Job *job=arg;for(unsigned i=0;i<OPERATIONS;i++){Record r={0};r.id=(uint64_t)job->index*OPERATIONS+i+1;snprintf(r.name,sizeof(r.name),"Worker_%u_User_%u",job->index,i);strcpy(r.plan,"5G Unlimited");strcpy(r.status,"Active");strcpy(r.network,"5G SA");if(db_add(job->db,&r)!=TC_OK)job->errors++;Record out;if(db_find(job->db,r.id,&out)!=TC_OK)job->errors++;if(i%2==0){strcpy(r.status,"Suspended");if(db_update(job->db,&r)!=TC_OK)job->errors++;}if(i%3==0&&db_delete(job->db,r.id)!=TC_OK)job->errors++;}return NULL;}
int concurrency_demo(void){Database db;if(db_init(&db)!=TC_OK)return 1;pthread_t threads[WORKERS];struct Job jobs[WORKERS];unsigned created=0;for(unsigned i=0;i<WORKERS;i++){jobs[i]=(struct Job){&db,i,0};if(pthread_create(&threads[i],NULL,worker,&jobs[i])!=0)break;created++;}for(unsigned i=0;i<created;i++)pthread_join(threads[i],NULL);unsigned errors=0;for(unsigned i=0;i<created;i++)errors+=jobs[i].errors;Stats s;db_stats(&db,&s);int rc=db_check(&db);printf("Concurrent test: %u threads x %u operations | records=%zu | errors=%u | invariant=%s\n",created,OPERATIONS,s.subscribers,errors,rc==TC_OK?"PASS":"FAIL");db_destroy(&db);return created==WORKERS&&errors==0&&rc==TC_OK?0:1;}
