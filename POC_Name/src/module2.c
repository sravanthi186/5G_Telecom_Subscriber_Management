#include "module2.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define TC_SYNC _commit
#define TC_FD _fileno
#else
#include <unistd.h>
#define TC_SYNC fsync
#define TC_FD fileno
#endif
/* Strict pipe-delimited v1 format; pipes, backslashes and controls rejected by core. */
static int write_record(const Record *r,void *ctx){FILE *f=ctx;return fprintf(f,"%" PRIu64 "|%s|%s|%s|%s|%lld|%lld\n",r->id,r->name,r->plan,r->status,r->network,(long long)r->created_at,(long long)r->updated_at)<0;}
int db_save(Database *db,const char *path){if(!db||!path||!*path||strlen(path)>900)return TC_INVALID;char tmp[1024];snprintf(tmp,sizeof(tmp),"%s.tmp",path);FILE *f=fopen(tmp,"wb");if(!f)return TC_IO;int rc=TC_OK;if(fputs("TC_SUBSCRIBERS_V2\n",f)==EOF||db_foreach(db,write_record,f)!=TC_OK)rc=TC_IO;if(rc==TC_OK&&(fflush(f)!=0||TC_SYNC(TC_FD(f))!=0))rc=TC_IO;if(fclose(f)!=0)rc=TC_IO;if(rc==TC_OK){
#ifdef _WIN32
if(!MoveFileExA(tmp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))rc=TC_IO;
#else
if(rename(tmp,path)!=0)rc=TC_IO;
#endif
}
if(rc!=TC_OK)remove(tmp);
return rc;}
static int parse_line(char *line,Record *r,int version){char *parts[7];size_t n=strlen(line);if(n&&line[n-1]=='\n')line[--n]=0;if(n&&line[n-1]=='\r')line[--n]=0;parts[0]=line;size_t count=1;for(char *p=line;*p;p++)if(*p=='|'){if(count==(version==2?7u:5u))return 0;*p=0;parts[count++]=p+1;}if(count!=(version==2?7u:5u)||!parts[0][0]||parts[0][0]=='-')return 0;char *end=NULL;errno=0;unsigned long long id=strtoull(parts[0],&end,10);if(errno||!end||*end||!id)return 0;memset(r,0,sizeof(*r));r->id=(uint64_t)id;char *dest[]={r->name,r->plan,r->status,r->network};const size_t sizes[]={TC_NAME,TC_TEXT,TC_TEXT,TC_TEXT};for(size_t i=0;i<4;i++){size_t len=strlen(parts[i+1]);if(!len||len>=sizes[i])return 0;memcpy(dest[i],parts[i+1],len+1);}if(version==2){char *end1=NULL,*end2=NULL;errno=0;long long a=strtoll(parts[5],&end1,10),b=strtoll(parts[6],&end2,10);if(errno||!end1||*end1||!end2||*end2||a<0||b<0)return 0;r->created_at=(time_t)a;r->updated_at=(time_t)b;}return 1;}
int db_load(Database *db,const char *path,size_t *loaded){if(loaded)*loaded=0;if(!db||!path)return TC_INVALID;FILE *f=fopen(path,"rb");if(!f)return errno==ENOENT?TC_OK:TC_IO;char line[512];if(!fgets(line,sizeof(line),f)){fclose(f);return TC_IO;}int version=strcmp(line,"TC_SUBSCRIBERS_V2\n")==0?2:(strcmp(line,"TC_SUBSCRIBERS_V1\n")==0?1:0);if(!version){fclose(f);return TC_IO;}Database staging;if(db_init(&staging)!=TC_OK){fclose(f);return TC_NOMEM;}int rc=TC_OK;size_t count=0;while(fgets(line,sizeof(line),f)){if(!strchr(line,'\n')&&!feof(f)){rc=TC_IO;break;}Record r;if(!parse_line(line,&r,version)||(rc=db_add(&staging,&r))!=TC_OK){if(rc==TC_OK)rc=TC_IO;break;}count++;}if(ferror(f))rc=TC_IO;if(fclose(f)!=0)rc=TC_IO;if(rc==TC_OK){/* Replace records only after entire file validated; preserve DB lock. */
pthread_rwlock_wrlock(&db->lock);
/* Swap data under lock; staging now owns old records, freed after unlock. */
Subscriber **old_buckets=db->buckets;db->buckets=staging.buckets;staging.buckets=old_buckets;
StringPool old_pool=db->pool;db->pool=staging.pool;staging.pool=old_pool;
size_t old_count=db->count;db->count=staging.count;staging.count=old_count;
size_t old_col=db->collisions;db->collisions=staging.collisions;staging.collisions=old_col;
size_t old_alloc=db->allocations;db->allocations=staging.allocations;staging.allocations=old_alloc;
size_t old_free=db->frees;db->frees=staging.frees;staging.frees=old_free;
size_t old_fail=db->failed_allocations;db->failed_allocations=staging.failed_allocations;staging.failed_allocations=old_fail;
pthread_rwlock_unlock(&db->lock);if(loaded)*loaded=count;}
db_destroy(&staging);return rc;}
