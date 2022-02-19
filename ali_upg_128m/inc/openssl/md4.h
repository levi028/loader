#ifndef HEADER_MD4_H
#define HEADER_MD4_H

#include <openssl/e_os2.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef OPENSSL_NO_MD4
#error MD4 is disabled.
#endif

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! MD4_LONG has to be at least 32 bits wide. If it's wider, then !
 * ! MD4_LONG_LOG2 has to be defined along.               !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#if defined(OPENSSL_SYS_WIN16) || defined(__LP32__)
#define MD4_LONG unsigned long
#elif defined(OPENSSL_SYS_CRAY) || defined(__ILP64__)
#define MD4_LONG unsigned long
#define MD4_LONG_LOG2 3
/*
 * _CRAY note. I could declare short, but I have no idea what impact
 * does it have on performance on none-T3E machines. I could declare
 * int, but at least on C90 sizeof(int) can be chosen at compile time.
 * So I've chosen long...
 *                    <appro@fy.chalmers.se>
 */
#else
#define MD4_LONG unsigned int
#endif

#define MD4_CBLOCK    64
#define MD4_LBLOCK    (MD4_CBLOCK/4)
#define MD4_DIGEST_LENGTH 16

typedef struct MD4state_st
    {
    MD4_LONG A,B,C,D;
    MD4_LONG Nl,Nh;
    MD4_LONG data[MD4_LBLOCK];
    unsigned int num;
    } MD4_CTX;

#ifdef OPENSSL_FIPS
int private_MD4_Init(MD4_CTX *c);
#endif
int MD4_Init(MD4_CTX *c);
int MD4_Update(MD4_CTX *c, const void *data, size_t len);
int MD4_Final(unsigned char *md, MD4_CTX *c);
unsigned char *MD4(const unsigned char *d, size_t n, unsigned char *md);
void MD4_Transform(MD4_CTX *c, const unsigned char *b);
#ifdef  __cplusplus
}
#endif

#endif
