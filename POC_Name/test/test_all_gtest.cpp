#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <ctime>
extern "C" {
#include "module1.h"
#include "module2.h"
#include "lifecycle.h"
#include "auth.h"
}
class SubscriberTest : public ::testing::Test {
protected:
 Database db{};
 void SetUp() override { ASSERT_EQ(db_init(&db),TC_OK); }
 void TearDown() override { db_destroy(&db); }
 Record sample(uint64_t id=101) {
  Record r{};r.id=id;
  std::strcpy(r.name,"Vidya");std::strcpy(r.plan,"5G Premium");
  std::strcpy(r.status,"Registered");std::strcpy(r.network,"Airtel");return r;
 }
};
TEST_F(SubscriberTest, AddValidSubscriber){auto r=sample();EXPECT_EQ(db_add(&db,&r),TC_OK);}
TEST_F(SubscriberTest, RejectDuplicateID){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);EXPECT_EQ(db_add(&db,&r),TC_DUPLICATE);}
TEST_F(SubscriberTest, RejectInvalidSubscriber){auto r=sample(0);EXPECT_EQ(db_add(&db,&r),TC_INVALID);}
TEST_F(SubscriberTest, FindExistingSubscriber){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);Record out{};EXPECT_EQ(db_find(&db,101,&out),TC_OK);EXPECT_STREQ(out.name,"Vidya");}
TEST_F(SubscriberTest, SubscriberNotFound){Record out{};EXPECT_EQ(db_find(&db,999,&out),TC_MISSING);}
TEST_F(SubscriberTest, UpdateSubscriber){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);std::strcpy(r.plan,"5G Unlimited");ASSERT_EQ(db_update(&db,&r),TC_OK);Record out{};ASSERT_EQ(db_find(&db,101,&out),TC_OK);EXPECT_STREQ(out.plan,"5G Unlimited");}
TEST_F(SubscriberTest, DeleteSubscriber){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);EXPECT_EQ(db_delete(&db,101),TC_OK);Record out{};EXPECT_EQ(db_find(&db,101,&out),TC_MISSING);}
TEST_F(SubscriberTest, SharedStringReferenceCount){auto a=sample(101),b=sample(102);ASSERT_EQ(db_add(&db,&a),TC_OK);ASSERT_EQ(db_add(&db,&b),TC_OK);Stats stats{};ASSERT_EQ(db_stats(&db,&stats),TC_OK);EXPECT_EQ(stats.unique_strings,3u);EXPECT_EQ(stats.reference_total,6u);ASSERT_EQ(db_delete(&db,102),TC_OK);ASSERT_EQ(db_stats(&db,&stats),TC_OK);EXPECT_EQ(stats.reference_total,3u);}
static int count_record(const Record*,void *ctx){++*static_cast<int*>(ctx);return 0;}
TEST_F(SubscriberTest, ListAllSubscribers){auto a=sample(101),b=sample(102);ASSERT_EQ(db_add(&db,&a),TC_OK);ASSERT_EQ(db_add(&db,&b),TC_OK);int count=0;EXPECT_EQ(db_foreach(&db,count_record,&count),TC_OK);EXPECT_EQ(count,2);}
TEST_F(SubscriberTest, ConsistencyCheck){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);EXPECT_EQ(db_check(&db),TC_OK);}
TEST_F(SubscriberTest, SaveAndLoad){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);ASSERT_EQ(db_save(&db,"data/gtest_subscribers.db"),TC_OK);ASSERT_EQ(db_delete(&db,101),TC_OK);size_t loaded=0;ASSERT_EQ(db_load(&db,"data/gtest_subscribers.db",&loaded),TC_OK);EXPECT_EQ(loaded,1u);std::remove("data/gtest_subscribers.db");}
TEST_F(SubscriberTest, ActivateAndDeactivate){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);EXPECT_EQ(lifecycle_change_status(&db,101,"Active"),TC_OK);EXPECT_EQ(lifecycle_change_status(&db,101,"Inactive"),TC_OK);Record out{};ASSERT_EQ(db_find(&db,101,&out),TC_OK);EXPECT_STREQ(out.status,"Inactive");}
TEST_F(SubscriberTest, InvalidStatusRejected){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);EXPECT_EQ(lifecycle_change_status(&db,101,"Unknown"),TC_INVALID);}
TEST_F(SubscriberTest, RechargeAndReactivate){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);ASSERT_EQ(lifecycle_change_status(&db,101,"Inactive"),TC_OK);EXPECT_EQ(lifecycle_recharge(&db,101,199.0,std::time(nullptr),"data/gtest_recharges.txt"),TC_OK);Record out{};ASSERT_EQ(db_find(&db,101,&out),TC_OK);EXPECT_STREQ(out.status,"Active");std::remove("data/gtest_recharges.txt");}
TEST_F(SubscriberTest, RejectNegativeRecharge){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);EXPECT_EQ(lifecycle_recharge(&db,101,-1,std::time(nullptr),"data/gtest_recharges.txt"),TC_INVALID);}
TEST_F(SubscriberTest, NinetyDayInactivity){EXPECT_FALSE(lifecycle_expired(100000,100000+89*86400));EXPECT_TRUE(lifecycle_expired(100000,100000+90*86400));}
TEST_F(SubscriberTest, AutomaticDeactivation){auto r=sample();r.created_at=std::time(nullptr)-91*86400;std::strcpy(r.status,"Active");ASSERT_EQ(db_add(&db,&r),TC_OK);Record out{};ASSERT_EQ(lifecycle_check(&db,101,"data/gtest_nonexistent_recharge.txt",std::time(nullptr),&out),TC_OK);EXPECT_STREQ(out.status,"Inactive");}
TEST_F(SubscriberTest, TerminatedCannotReactivate){auto r=sample();ASSERT_EQ(db_add(&db,&r),TC_OK);ASSERT_EQ(lifecycle_change_status(&db,101,"Terminated"),TC_OK);EXPECT_EQ(lifecycle_change_status(&db,101,"Active"),TC_DENIED);}
TEST(AuthTest, AdminLoginAndViewerRole){const char *path="data/gtest_users.auth";std::remove(path);ASSERT_EQ(auth_setup(path,"demo_admin","DemoPassword123!"),1);int role=0;EXPECT_EQ(auth_login(path,"demo_admin","DemoPassword123!",&role),1);EXPECT_EQ(role,AUTH_ADMIN);EXPECT_EQ(auth_login(path,"demo_admin","wrongpassword",&role),0);ASSERT_EQ(auth_add_viewer(path,"viewer","ViewerPassword123!"),1);EXPECT_EQ(auth_login(path,"viewer","ViewerPassword123!",&role),1);EXPECT_EQ(role,AUTH_VIEWER);std::remove(path);}
