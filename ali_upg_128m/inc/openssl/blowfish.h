#ifndef HEADER_BLOWFISH_H
#define HEADER_BLOWFISH_H

#include <openssl/e_os2.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef OPENSSL_NO_BF
#error BF is disabled.
#endif

#define BF_ENCRYPT    1
#define BF_DECRYPT    0

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! BF_LONG has to be at least 32 bits wide. If it's wider, then !
 * ! BF_LONG_LOG2 has to be defined along.                        !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#if defined(OPENSSL_SYS_WIN16) || defined(__LP32__)
#define BF_LONG unsigned long
#elif defined(OPENSSL_SYS_CRAY) || defined(__ILP64__)
#define BF_LONG unsigned long
#define BF_LONG_LOG2 3
/*
 * _CRAY note. I could declare short, but I have no idea what impact
 * does it have on performance on none-T3E machines. I could declare
 * int, but at least on C90 sizeof(int) can be chosen at compile time.
 * So I've chosen long...
 *                    <appro@fy.chalmers.se>
 */
#else
#define BF_LONG unsigned int
#endif

#define BF_ROUNDS    16
#define BF_BLOCK    8

typedef struct bf_key_st
    {
    BF_LONG P[BF_ROUNDS+2];
    BF_LONG S[4*256];
    } BF_KEY;

#ifdef OPENSSL_FIPS
void private_BF_set_key(BF_KEY *key, int len, const unsigned char *data);
#endif
void BF_set_key(BF_KEY *key, int len, const unsigned char *data);

void BF_encrypt(BF_LONG *data,const BF_KEY *key);
void BF_decrypt(BF_LONG *data,const BF_KEY *key);

void BF_ecb_encrypt(const unsigned char *in, unsigned char *out,
    const BF_KEY *key, int enc);
void BF_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
    const BF_KEY *schedule, unsigned char *ivec, int enc);
void BF_cfb64_encrypt(const unsigned char *in, unsigned char *out, long length,
    const BF_KEY *schedule, unsigned char *ivec, int *num, int enc);
void BF_ofb64_encrypt(const unsigned char *in, unsigned char *out, long length,
    const BF_KEY *schedule, unsigned char *ivec, int *num);
const char *BF_options(void);

#ifdef  __cplusplus
}
#endif

#endif
