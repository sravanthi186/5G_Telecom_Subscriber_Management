#include "lifecycle.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
int lifecycle_days(time_t last,time_t now){if(last<=0||now<last)return 0;return (int)(difftime(now,last)/86400.0);}
int lifecycle_expired(time_t last,time_t now){return last>0&&lifecycle_days(last,now)>=TC_INACTIVE_DAYS;}
int lifecycle_last_recharge(uint64_t id,const char *path,time_t *when,double *amount){
 if(!id||!path||!when||!amount)return TC_INVALID;
 FILE *f=fopen(path,"r");if(!f)return TC_MISSING;
 char line[256];int found=0;time_t latest=0;double latest_amount=0;
 while(fgets(line,sizeof(line),f)){uint64_t rid;long long ts;double amt;
  if(sscanf(line,"%" SCNu64 "|%lld|%lf",&rid,&ts,&amt)==3&&rid==id&&ts>0&&isfinite(amt)&&amt>0&&(!found||(time_t)ts>=latest)){
   latest=(time_t)ts;latest_amount=amt;found=1;
  }
 }
 fclose(f);if(!found)return TC_MISSING;*when=latest;*amount=latest_amount;return TC_OK;
}
int lifecycle_change_status(Database *db,uint64_t id,const char *status){
 if(!db||!status)return TC_INVALID;
 const char *allowed[]={"Registered","Pending Activation","Active","Suspended","Inactive","Terminated"};int ok=0;
 for(size_t i=0;i<sizeof(allowed)/sizeof(allowed[0]);i++)if(strcmp(status,allowed[i])==0)ok=1;
 if(!ok)return TC_INVALID;
 Record r;int rc=db_find(db,id,&r);if(rc)return rc;
 if(strcmp(r.status,"Terminated")==0&&strcmp(status,"Terminated")!=0)return TC_DENIED;
 snprintf(r.status,sizeof(r.status),"%s",status);return db_update(db,&r);
}
int lifecycle_recharge(Database *db,uint64_t id,double amount,time_t when,const char *path){
 if(!db||!path||!isfinite(amount)||amount<=0||when<=0||when>time(NULL)+86400)return TC_INVALID;
 Record r;int rc=db_find(db,id,&r);if(rc)return rc;
 if(strcmp(r.status,"Terminated")==0||strcmp(r.status,"Suspended")==0)return TC_DENIED;
 FILE *f=fopen(path,"a");if(!f)return TC_IO;
 int written=fprintf(f,"%" PRIu64 "|%lld|%.2f\n",id,(long long)when,amount);
 if(written<0||fflush(f)!=0||fclose(f)!=0)return TC_IO;
 return lifecycle_change_status(db,id,"Active");
}
int lifecycle_check(Database *db,uint64_t id,const char *path,time_t now,Record *out){
 if(!db||!path||!out)return TC_INVALID;
 int rc=db_find(db,id,out);if(rc)return rc;
 if(strcmp(out->status,"Active")==0){time_t last=0;double amt=0;
  if(lifecycle_last_recharge(id,path,&last,&amt)!=TC_OK)last=out->created_at;
  if(lifecycle_expired(last,now)){
   rc=lifecycle_change_status(db,id,"Inactive");if(rc)return rc;return db_find(db,id,out);
  }
 }
 return TC_OK;
}
