#include "auth.h"
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef _WIN32
#include <sys/stat.h>
#endif
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
/* Use built-in Windows CNG on MinGW64; OpenSSL remains for Linux. */
#ifdef _WIN32
static int secure_random(unsigned char *buffer, size_t length) {
    return length <= 0xffffffffu &&
           BCRYPT_SUCCESS(BCryptGenRandom(NULL, buffer, (ULONG)length,
                                           BCRYPT_USE_SYSTEM_PREFERRED_RNG));
}
static int derive_password(const char *password, const unsigned char *salt,
                           size_t salt_length, int iterations,
                           unsigned char *key, size_t key_length) {
    BCRYPT_ALG_HANDLE algorithm = NULL;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM,
                                                   NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (!BCRYPT_SUCCESS(status)) return 0;
    size_t pass_length = strlen(password);
    if (pass_length > 0xffffffffu || salt_length > 0xffffffffu ||
        key_length > 0xffffffffu) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return 0;
    }
    status = BCryptDeriveKeyPBKDF2(algorithm, (PUCHAR)password, (ULONG)pass_length,
                                    (PUCHAR)salt, (ULONG)salt_length,
                                    (ULONGLONG)iterations, key, (ULONG)key_length, 0);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    return BCRYPT_SUCCESS(status);
}
static void wipe_secret(void *buffer, size_t length) { SecureZeroMemory(buffer, length); }
static int equal_secret(const unsigned char *a, const unsigned char *b, size_t n) {
    unsigned char diff = 0;
    for (size_t i = 0; i < n; ++i) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}
#else
static int secure_random(unsigned char *buffer, size_t length) {
    return length <= 0x7fffffffu && RAND_bytes(buffer, (int)length) == 1;
}
static int derive_password(const char *password, const unsigned char *salt,
                           size_t salt_length, int iterations,
                           unsigned char *key, size_t key_length) {
    return salt_length <= 0x7fffffffu && key_length <= 0x7fffffffu &&
           PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, (int)salt_length,
                             iterations, EVP_sha256(), (int)key_length, key) == 1;
}
static void wipe_secret(void *buffer, size_t length) { OPENSSL_cleanse(buffer, length); }
static int equal_secret(const unsigned char *a, const unsigned char *b, size_t n) {
    return CRYPTO_memcmp(a, b, n) == 0;
}
#endif
#define ITERATIONS 210000
#define KEY_BYTES 32
#define SALT_BYTES 16
static void hex(const unsigned char *in,size_t n,char *out){static const char *digits="0123456789abcdef";for(size_t i=0;i<n;i++){out[i*2]=digits[in[i]>>4];out[i*2+1]=digits[in[i]&15];}out[n*2]=0;}
static int unhex(const char *s,unsigned char *out,size_t n){if(strlen(s)!=n*2)return 0;for(size_t i=0;i<n;i++){unsigned a,b;char x=s[2*i],y=s[2*i+1];if(!isxdigit((unsigned char)x)||!isxdigit((unsigned char)y))return 0;a=(unsigned)(isdigit((unsigned char)x)?x-'0':tolower((unsigned char)x)-'a'+10);b=(unsigned)(isdigit((unsigned char)y)?y-'0':tolower((unsigned char)y)-'a'+10);out[i]=(unsigned char)((a<<4)|b);}return 1;}
static int good_user(const char *s){size_t n=strlen(s);if(n<3||n>31)return 0;for(size_t i=0;i<n;i++)if(!isalnum((unsigned char)s[i])&&s[i]!='_')return 0;return 1;}
int auth_setup(const char *path,const char *username,const char *password){if(!path||!username||!password||!good_user(username)||strlen(password)<12||strlen(password)>128)return 0;FILE *check=fopen(path,"rb");if(check){fclose(check);return 0;}unsigned char salt[SALT_BYTES],key[KEY_BYTES];if(!secure_random(salt,sizeof(salt)))return 0;if(!derive_password(password,salt,sizeof(salt),ITERATIONS,key,sizeof(key)))return 0;char s_hex[2*SALT_BYTES+1],k_hex[2*KEY_BYTES+1];hex(salt,sizeof(salt),s_hex);hex(key,sizeof(key),k_hex);wipe_secret(key,sizeof(key));FILE *f=fopen(path,"wb");if(!f)return 0;int ok=fprintf(f,"TC_AUTH_V1\n%s|admin|%d|%s|%s\n",username,ITERATIONS,s_hex,k_hex)>0;if(fclose(f)!=0)ok=0;if(!ok)remove(path);
#ifndef _WIN32
if(ok)chmod(path,S_IRUSR|S_IWUSR);
#endif
return ok;}
int auth_add_viewer(const char *path,const char *username,const char *password){
if(!path||!username||!password||!good_user(username)||strlen(password)<12||strlen(password)>128)return 0;
FILE *f=fopen(path,"rb");if(!f)return 0;char line[256];int valid=fgets(line,sizeof(line),f)&&strcmp(line,"TC_AUTH_V1\n")==0;
while(valid&&fgets(line,sizeof(line),f)){char existing[32];if(sscanf(line,"%31[^|]",existing)==1&&strcmp(existing,username)==0){fclose(f);return 0;}}
fclose(f);if(!valid)return 0;
unsigned char salt[SALT_BYTES],key[KEY_BYTES];if(!secure_random(salt,sizeof(salt)))return 0;
if(!derive_password(password,salt,sizeof(salt),ITERATIONS,key,sizeof(key)))return 0;
char s_hex[2*SALT_BYTES+1],k_hex[2*KEY_BYTES+1];hex(salt,sizeof(salt),s_hex);hex(key,sizeof(key),k_hex);wipe_secret(key,sizeof(key));
f=fopen(path,"ab");if(!f)return 0;int ok=fprintf(f,"%s|viewer|%d|%s|%s\n",username,ITERATIONS,s_hex,k_hex)>0;if(fclose(f)!=0)ok=0;return ok;
}
int auth_login(const char *path,const char *username,const char *password,int *role){if(role)*role=0;if(!path||!username||!password||!role)return 0;FILE *f=fopen(path,"rb");if(!f)return 0;char header[32],line[256];int ok=fgets(header,sizeof(header),f)&&strcmp(header,"TC_AUTH_V1\n")==0;while(ok&&fgets(line,sizeof(line),f)){char user[32],r[16],salt_hex[33],key_hex[65];int iter=0;if(sscanf(line,"%31[^|]|%15[^|]|%d|%32[^|]|%64s",user,r,&iter,salt_hex,key_hex)!=5)continue;if(strcmp(user,username)!=0||iter<100000||iter>1000000)continue;unsigned char salt[SALT_BYTES],expected[KEY_BYTES],actual[KEY_BYTES];if(!unhex(salt_hex,salt,sizeof(salt))||!unhex(key_hex,expected,sizeof(expected)))continue;if(!derive_password(password,salt,sizeof(salt),iter,actual,sizeof(actual)))continue;int match=equal_secret(expected,actual,sizeof(actual));wipe_secret(actual,sizeof(actual));wipe_secret(expected,sizeof(expected));if(match)*role=strcmp(r,"admin")==0?AUTH_ADMIN:strcmp(r,"viewer")==0?AUTH_VIEWER:0;break;}fclose(f);return *role!=0;}
int auth_read_password(char *out,size_t capacity){if(!out||capacity<2)return 0;printf("Password (hidden): ");fflush(stdout);
#ifdef _WIN32
size_t n=0;int c;while((c=_getch())!='\r'&&c!='\n'){if(c==8&&n){n--;continue;}if(c>=32&&c<127&&n+1<capacity)out[n++]=(char)c;}out[n]=0;putchar('\n');return n>0;
#else
struct termios old,hidden;int tty=isatty(STDIN_FILENO);if(tty&&tcgetattr(STDIN_FILENO,&old)==0){hidden=old;hidden.c_lflag &= ~(tcflag_t)ECHO;tcsetattr(STDIN_FILENO,TCSANOW,&hidden);}int ok=fgets(out,(int)capacity,stdin)!=NULL;if(tty)tcsetattr(STDIN_FILENO,TCSANOW,&old);putchar('\n');if(!ok)return 0;size_t n=strcspn(out,"\r\n");if(out[n]==0&&n==capacity-1)return 0;out[n]=0;return n>0;
#endif
}
