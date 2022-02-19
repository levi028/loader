#ifndef HEADER_RC2_H
#define HEADER_RC2_H

#include <openssl/opensslconf.h> /* OPENSSL_NO_RC2, RC2_INT */
#ifdef OPENSSL_NO_RC2
#error RC2 is disabled.
#endif

#define RC2_ENCRYPT    1
#define RC2_DECRYPT    0

#define RC2_BLOCK    8
#define RC2_KEY_LENGTH    16

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct rc2_key_st
    {
    RC2_INT data[64];
    } RC2_KEY;

#ifdef OPENSSL_FIPS
void private_RC2_set_key(RC2_KEY *key, int len, const unsigned char *data,int bits);
#endif
void RC2_set_key(RC2_KEY *key, int len, const unsigned char *data,int bits);
void RC2_ecb_encrypt(const unsigned char *in,unsigned char *out,RC2_KEY *key,
             int enc);
void RC2_encrypt(unsigned long *data,RC2_KEY *key);
void RC2_decrypt(unsigned long *data,RC2_KEY *key);
void RC2_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
    RC2_KEY *ks, unsigned char *iv, int enc);
void RC2_cfb64_encrypt(const unsigned char *in, unsigned char *out,
               long length, RC2_KEY *schedule, unsigned char *ivec,
               int *num, int enc);
void RC2_ofb64_encrypt(const unsigned char *in, unsigned char *out,
               long length, RC2_KEY *schedule, unsigned char *ivec,
               int *num);

#ifdef  __cplusplus
}
#endif

#endif
