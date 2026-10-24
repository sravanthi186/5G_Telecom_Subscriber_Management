#include "module2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %s at line %d\n", #x, __LINE__); exit(1); } } while (0)
int main(void) {
    Database db;
    Record in = {0}, out = {0};
    size_t loaded = 0;
    CHECK(db_init(&db) == TC_OK);
    in.id = 4501;
    strcpy(in.name, "Storage Tester");
    strcpy(in.plan, "5G Premium");
    strcpy(in.status, "Active");
    strcpy(in.network, "5G SA");
    CHECK(db_add(&db, &in) == TC_OK);
    CHECK(db_save(&db, "data/test_module2.db") == TC_OK);
    CHECK(db_delete(&db, in.id) == TC_OK);
    CHECK(db_load(&db, "data/test_module2.db", &loaded) == TC_OK);
    CHECK(loaded == 1);
    CHECK(db_find(&db, in.id, &out) == TC_OK);
    CHECK(strcmp(out.plan, "5G Premium") == 0);
    CHECK(out.created_at > 0 && out.updated_at >= out.created_at);
    CHECK(db_check(&db) == TC_OK);
    db_destroy(&db);
    remove("data/test_module2.db");
    puts("PASS: module2 file save, reload, record integrity");
    return 0;
}
