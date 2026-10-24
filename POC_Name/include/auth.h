#ifndef TELECOM_AUTH_H
#define TELECOM_AUTH_H
#include <stddef.h>
#define AUTH_ADMIN 2
#define AUTH_VIEWER 1
int auth_setup(const char *path,const char *username,const char *password);
int auth_add_viewer(const char *path,const char *username,const char *password);
int auth_login(const char *path,const char *username,const char *password,int *role);
int auth_read_password(char *out,size_t capacity);
#endif
