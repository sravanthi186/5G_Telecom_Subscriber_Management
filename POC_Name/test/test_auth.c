#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"AUTH FAIL line %d\n",__LINE__);exit(1);}}while(0)
int main(void){const char *path="data/test_auth.auth";remove(path);int role=0;CHECK(auth_setup(path,"testadmin","CorrectPassword123!")==1);CHECK(auth_setup(path,"testadmin","CorrectPassword123!")==0);CHECK(auth_login(path,"testadmin","wrongPassword123!",&role)==0);CHECK(auth_login(path,"testadmin","CorrectPassword123!",&role)==1&&role==AUTH_ADMIN);CHECK(auth_add_viewer(path,"testviewer","ViewerPassword123!")==1);CHECK(auth_add_viewer(path,"testviewer","ViewerPassword123!")==0);CHECK(auth_login(path,"testviewer","ViewerPassword123!",&role)==1&&role==AUTH_VIEWER);CHECK(auth_login(path,"testviewer","CorrectPassword123!",&role)==0);remove(path);puts("PASS: password hashing, login, admin/viewer RBAC roles");return 0;}
