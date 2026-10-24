#include "module1.h"
#include "module2.h"
#include "auth.h"
#include "benchmark.h"
#include "concurrency.h"
#include "lifecycle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define CLEAN_PASSWORD(buffer) SecureZeroMemory((buffer), sizeof(buffer))
#else
#include <openssl/crypto.h>
#define CLEAN_PASSWORD(buffer) OPENSSL_cleanse((buffer), sizeof(buffer))
#endif
#define DATA_PATH "data/subscribers.db"
#define AUTH_PATH "data/users.auth"
#define LOG_PATH "data/operations.txt"
#define TXT_PATH "data/subscribers.txt"
#define RECHARGE_PATH "data/recharges.txt"
static int line(char *s,size_t n){if(!fgets(s,(int)n,stdin))return 0;size_t len=strcspn(s,"\r\n");if(s[len])s[len]=0;else if(len==n-1){int c;while((c=getchar())!='\n'&&c!=EOF){}return 0;}return 1;}
static int prompt(const char *label,char *s,size_t n){printf("%s: ",label);fflush(stdout);return line(s,n);}
static int number(const char *label,uint64_t *value){char s[80],*end;printf("%s: ",label);if(!line(s,sizeof(s))||!s[0]||s[0]=='-')return 0;errno=0;unsigned long long x=strtoull(s,&end,10);if(errno||*end||!x)return 0;*value=(uint64_t)x;return 1;}
static int input_record(Record *r,int existing){memset(r,0,sizeof(*r));if(!number("Subscriber ID",&r->id))return 0;if(!prompt("Name",r->name,sizeof(r->name)))return 0;if(!prompt("Plan",r->plan,sizeof(r->plan)))return 0;if(!prompt("Status (Registered/Pending Activation/Active/Suspended/Inactive/Terminated)",r->status,sizeof(r->status)))return 0;if(!prompt("Network",r->network,sizeof(r->network)))return 0;(void)existing;return 1;}
static void stamp(time_t t,char *out,size_t n){struct tm *tmv=localtime(&t);if(!tmv||!strftime(out,n,"%Y-%m-%d %H:%M:%S",tmv))snprintf(out,n,"unknown");}
static void show_record(const Record *r){char a[32],b[32];stamp(r->created_at,a,sizeof(a));stamp(r->updated_at,b,sizeof(b));printf("\nSubscriber ID : %" PRIu64 "\nName          : %s\nPlan          : %s\nStatus        : %s\nNetwork       : %s\nCreated At    : %s\nUpdated At    : %s\n",r->id,r->name,r->plan,r->status,r->network,a,b);}
static void audit_record(const char *username,const char *operation,int rc,const Record *r){FILE *f=fopen(LOG_PATH,"ab");char now[32];stamp(time(NULL),now,sizeof(now));printf("Operation Time: %s\n",now);if(!f){puts("WARNING: Unable to write data/operations.txt");return;}fprintf(f,"[%s] User=%s | Action=%s | Result=%s",now,username,operation,tc_error(rc));if(r)fprintf(f," | ID=%" PRIu64 " | Name=%s | Plan=%s | Status=%s | Network=%s",r->id,r->name,r->plan,r->status,r->network);fputc('\n',f);if(fclose(f)!=0)puts("WARNING: Failed to close operation log");}
static void audit(const char *username,const char *operation,int rc){audit_record(username,operation,rc,NULL);}
static int write_txt(const Record *r,void *ctx){FILE *f=ctx;char a[32],b[32];stamp(r->created_at,a,sizeof(a));stamp(r->updated_at,b,sizeof(b));return fprintf(f,"ID: %" PRIu64 " | Name: %s | Plan: %s | Status: %s | Network: %s | Created: %s | Updated: %s\n",r->id,r->name,r->plan,r->status,r->network,a,b)<0;}
static int save_all(Database *db){int rc=db_save(db,DATA_PATH);if(rc!=TC_OK)return rc;FILE *f=fopen(TXT_PATH,"wb");if(!f)return TC_IO;if(fputs("5G TELECOM SUBSCRIBERS - CURRENT RECORDS\n",f)==EOF||db_foreach(db,write_txt,f)!=TC_OK)rc=TC_IO;if(fclose(f)!=0)rc=TC_IO;return rc;}
static void autosave(Database *db){int rc=save_all(db);printf("Auto-save: %s | Files: %s, %s\n",tc_error(rc),DATA_PATH,TXT_PATH);if(rc!=TC_OK)puts("WARNING: Changes may not be saved completely. Please check file permissions.");}
static void recharge_details(uint64_t id){time_t last=0;double amount=0;char date[32];if(lifecycle_last_recharge(id,RECHARGE_PATH,&last,&amount)==TC_OK){stamp(last,date,sizeof(date));printf("Last Recharge: %s | Amount: %.2f | Days Ago: %d\n",date,amount,lifecycle_days(last,time(NULL)));}else puts("Last Recharge: Not recorded");}
static int show_all(const Record *r,void *ctx){(void)ctx;show_record(r);recharge_details(r->id);return 0;}
int main(void){Database db;if(db_init(&db)!=TC_OK){fputs("Failed to initialize database.\n",stderr);return 1;}char username[40]={0},password[160]={0};int role=0;FILE *f=fopen(AUTH_PATH,"rb");if(!f){puts("FIRST-RUN ADMIN SETUP (minimum 12-character password)");if(!prompt("Admin username",username,sizeof(username))||!auth_read_password(password,sizeof(password))||!auth_setup(AUTH_PATH,username,password)){puts("Setup failed. Check data/ folder and password length.");db_destroy(&db);return 1;}puts("Admin account created. Please sign in.");}else fclose(f);for(int attempt=0;attempt<3;attempt++){if(!prompt("Username",username,sizeof(username))||!auth_read_password(password,sizeof(password)))break;role=0;int ok=auth_login(AUTH_PATH,username,password,&role);CLEAN_PASSWORD(password);if(ok)break;puts("Invalid credentials.");}if(!role){puts("Login failed.");db_destroy(&db);return 1;}size_t loaded=0;int load_rc=db_load(&db,DATA_PATH,&loaded);if(load_rc!=TC_OK){puts("Data file could not be loaded. No changes will be saved until the issue is resolved.");db_destroy(&db);return 1;}printf("Welcome %s | role=%s | loaded=%zu\n",username,role==AUTH_ADMIN?"admin":"viewer",loaded);audit(username,"LOGIN",TC_OK);
for(;;){
puts("\n=========================================\n  5G TELECOM SUBSCRIBER MANAGEMENT SYSTEM\n=========================================");
puts("1. Add Subscriber\n2. Search Subscriber\n3. Update Subscriber Details\n4. Activate Subscriber\n5. Deactivate Subscriber\n6. Recharge Subscriber\n7. Check Subscriber Status\n8. Delete Subscriber\n9. View All Subscribers\n0. Save and Exit");
char choice[24];if(!prompt("Select option",choice,sizeof(choice)))break;
char *endptr=NULL;long option=strtol(choice,&endptr,10);
if(!choice[0]||*endptr||option<0||option>9){puts("Invalid menu choice.");continue;}
if(option==0){int rc=save_all(&db);audit(username,"EXIT_SAVE",rc);printf("Save: %s\n",tc_error(rc));break;}
if(option!=2&&option!=7&&option!=9&&role!=AUTH_ADMIN){puts("Permission denied: admin role required.");audit(username,"DENIED",TC_DENIED);continue;}
int rc=TC_OK;Record r={0};uint64_t id=0;
switch(option){
case 1:
 if(!input_record(&r,0))rc=TC_INVALID;else rc=db_add(&db,&r);
 printf("Add: %s\n",tc_error(rc));if(rc==TC_OK){db_find(&db,r.id,&r);show_record(&r);autosave(&db);}
 audit_record(username,"ADD",rc,rc==TC_OK?&r:NULL);break;
case 2:
 if(!number("Subscriber ID",&id))rc=TC_INVALID;else rc=lifecycle_check(&db,id,RECHARGE_PATH,time(NULL),&r);
 if(rc==TC_OK){show_record(&r);recharge_details(id);}else puts(tc_error(rc));
 audit_record(username,"SEARCH",rc,rc==TC_OK?&r:NULL);if(rc==TC_OK)autosave(&db);break;
case 3:
 if(!input_record(&r,1))rc=TC_INVALID;else rc=db_update(&db,&r);
 printf("Update: %s\n",tc_error(rc));if(rc==TC_OK){db_find(&db,r.id,&r);show_record(&r);autosave(&db);}
 audit_record(username,"UPDATE",rc,rc==TC_OK?&r:NULL);break;
case 4:case 5:
 if(!number("Subscriber ID",&id))rc=TC_INVALID;
 else rc=lifecycle_change_status(&db,id,option==4?"Active":"Inactive");
 printf("Status change: %s\n",tc_error(rc));if(rc==TC_OK){db_find(&db,id,&r);show_record(&r);autosave(&db);}
 audit_record(username,option==4?"ACTIVATE":"DEACTIVATE",rc,rc==TC_OK?&r:NULL);break;
case 6:{
 char amt[80],*ep;double value=0;
 if(!number("Subscriber ID",&id)||!prompt("Recharge Amount",amt,sizeof(amt)))rc=TC_INVALID;
 else{errno=0;value=strtod(amt,&ep);if(errno||ep==amt||*ep)rc=TC_INVALID;else rc=lifecycle_recharge(&db,id,value,time(NULL),RECHARGE_PATH);}
 printf("Recharge: %s\n",tc_error(rc));if(rc==TC_OK){db_find(&db,id,&r);show_record(&r);recharge_details(id);autosave(&db);}
 audit_record(username,"RECHARGE",rc,rc==TC_OK?&r:NULL);break;}
case 7:
 if(!number("Subscriber ID",&id))rc=TC_INVALID;else rc=lifecycle_check(&db,id,RECHARGE_PATH,time(NULL),&r);
 if(rc==TC_OK){show_record(&r);recharge_details(id);autosave(&db);}else puts(tc_error(rc));
 audit_record(username,"STATUS_CHECK",rc,rc==TC_OK?&r:NULL);break;
case 8:
 if(!number("Subscriber ID",&id))rc=TC_INVALID;
 else{rc=db_find(&db,id,&r);if(rc==TC_OK)rc=db_delete(&db,id);}
 printf("Delete: %s\n",tc_error(rc));if(rc==TC_OK){show_record(&r);autosave(&db);}
 audit_record(username,"DELETE",rc,rc==TC_OK?&r:NULL);break;
case 9:
 rc=db_foreach(&db,show_all,NULL);printf("View all: %s\n",tc_error(rc));audit(username,"VIEW_ALL",rc);break;
}
}
db_destroy(&db);return 0;}
