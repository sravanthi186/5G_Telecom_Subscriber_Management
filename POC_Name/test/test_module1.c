#include "module1.h"
#include "module2.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL: %s line %d\n",#x,__LINE__);exit(1);} }while(0)
static Record rec(uint64_t id,const char *plan){Record r={0};r.id=id;strcpy(r.name,"Test User");strcpy(r.plan,plan);strcpy(r.status,"Active");strcpy(r.network,"5G SA");return r;}
struct ThreadArg {Database *db;uint64_t start;int errors;};
static void *thread_work(void *ptr){struct ThreadArg *a=ptr;for(unsigned i=0;i<1000;i++){Record r=rec(a->start+i,"5G Shared");if(db_add(a->db,&r)!=TC_OK)a->errors++;Record found;if(db_find(a->db,r.id,&found)!=TC_OK)a->errors++;if(i%2==0){strcpy(r.status,"Suspended");if(db_update(a->db,&r)!=TC_OK)a->errors++;}if(i%4==0&&db_delete(a->db,r.id)!=TC_OK)a->errors++;}return NULL;}
int main(void){Database db;CHECK(db_init(&db)==TC_OK);Record a=rec(101,"Premium"),b=rec(102,"Premium"),out;CHECK(db_add(&db,&a)==TC_OK);CHECK(db_add(&db,&b)==TC_OK);CHECK(db_add(&db,&a)==TC_DUPLICATE);CHECK(db_find(&db,101,&out)==TC_OK);CHECK(strcmp(out.plan,"Premium")==0);CHECK(out.created_at>0);Stats s;CHECK(db_stats(&db,&s)==TC_OK);CHECK(s.subscribers==2&&s.unique_strings==3&&s.reference_total==6);strcpy(a.plan,"Basic");CHECK(db_update(&db,&a)==TC_OK);CHECK(db_stats(&db,&s)==TC_OK);CHECK(s.unique_strings==4);CHECK(db_delete(&db,102)==TC_OK);CHECK(db_find(&db,102,&out)==TC_MISSING);CHECK(db_stats(&db,&s)==TC_OK);CHECK(s.unique_strings==3);CHECK(db_check(&db)==TC_OK);Record bad=rec(0,"X");CHECK(db_add(&db,&bad)==TC_INVALID);bad=rec(201,"bad|value");CHECK(db_add(&db,&bad)==TC_INVALID);CHECK(db_save(&db,"data/test_subscribers.db")==TC_OK);CHECK(db_delete(&db,101)==TC_OK);size_t loaded=0;CHECK(db_load(&db,"data/test_subscribers.db",&loaded)==TC_OK&&loaded==1);CHECK(db_find(&db,101,&out)==TC_OK&&strcmp(out.plan,"Basic")==0);CHECK(db_check(&db)==TC_OK);
/* Corrupt input must not replace existing in-memory data. */
FILE *f=fopen("data/test_corrupt.db","wb");CHECK(f!=NULL);fputs("TC_SUBSCRIBERS_V1\n101|bad|record\n",f);fclose(f);CHECK(db_load(&db,"data/test_corrupt.db",&loaded)!=TC_OK);CHECK(db_find(&db,101,&out)==TC_OK);
/* Deliberately force hash bucket collisions. */
Record c=rec(101+TC_BUCKETS,"Collision");CHECK(db_add(&db,&c)==TC_OK);CHECK(db_find(&db,c.id,&out)==TC_OK);
struct ThreadArg jobs[4];pthread_t threads[4];for(unsigned i=0;i<4;i++){jobs[i]=(struct ThreadArg){&db,100000u+(uint64_t)i*1000u,0};CHECK(pthread_create(&threads[i],NULL,thread_work,&jobs[i])==0);}for(unsigned i=0;i<4;i++){pthread_join(threads[i],NULL);CHECK(jobs[i].errors==0);}CHECK(db_check(&db)==TC_OK);CHECK(db_stats(&db,&s)==TC_OK);printf("PASS: CRUD, duplicate, validation, shared strings, reference counts, persistence, malformed file, collisions, concurrency (%zu records)\n",s.subscribers);db_destroy(&db);remove("data/test_subscribers.db");remove("data/test_corrupt.db");puts("ALL CORE TESTS PASSED");return 0;}
