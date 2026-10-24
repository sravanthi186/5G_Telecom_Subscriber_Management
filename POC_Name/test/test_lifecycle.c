#include "lifecycle.h"
#include "module2.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
int main(void){
 Database db;assert(db_init(&db)==TC_OK);
 Record r={0};r.id=101;strcpy(r.name,"Akhila");strcpy(r.plan,"5G Unlimited");strcpy(r.status,"Registered");strcpy(r.network,"Airtel");
 assert(db_add(&db,&r)==TC_OK);
 assert(lifecycle_change_status(&db,101,"Active")==TC_OK);
 assert(lifecycle_change_status(&db,101,"WrongStatus")==TC_INVALID);
 assert(lifecycle_change_status(&db,999,"Active")==TC_MISSING);
 assert(lifecycle_days(1000,1000+89*86400)==89);
 assert(!lifecycle_expired(1000,1000+89*86400));
 assert(lifecycle_expired(1000,1000+90*86400));
 assert(lifecycle_check(&db,101,"data/test_recharges_missing.txt",time(NULL),&r)==TC_OK);
 assert(lifecycle_recharge(&db,101,199.0,time(NULL),"data/test_recharges.txt")==TC_OK);
 assert(lifecycle_last_recharge(101,"data/test_recharges.txt",&r.created_at,(double[]){0})==TC_OK);
 assert(lifecycle_recharge(&db,101,-1,time(NULL),"data/test_recharges.txt")==TC_INVALID);
 assert(lifecycle_change_status(&db,101,"Terminated")==TC_OK);
 assert(lifecycle_change_status(&db,101,"Active")==TC_DENIED);
 assert(lifecycle_recharge(&db,101,199,time(NULL),"data/test_recharges.txt")==TC_DENIED);
 assert(db_delete(&db,101)==TC_OK);db_destroy(&db);remove("data/test_recharges.txt");
 puts("PASS: lifecycle activation, deactivation rule, recharge, validation, terminated protection");return 0;
}
