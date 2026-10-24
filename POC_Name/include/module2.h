#ifndef TELECOM_MODULE2_H
#define TELECOM_MODULE2_H
#include "module1.h"
/* Persistent storage: versioned file save and transactional reload. */
int db_save(Database *db, const char *path);
int db_load(Database *db, const char *path, size_t *loaded);
#endif
