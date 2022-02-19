#ifndef HEADER_RAND_LCL_H
#define HEADER_RAND_LCL_H

#define ENTROPY_NEEDED 32  /* require 256 bits = 32 bytes of randomness */


#if !defined(USE_MD5_RAND) && !defined(USE_SHA1_RAND) && !defined(USE_MDC2_RAND) && !defined(USE_MD2_RAND)
#if !defined(OPENSSL_NO_SHA) && !defined(OPENSSL_NO_SHA1)
#define USE_SHA1_RAND
#elif !defined(OPENSSL_NO_MD5)
#define USE_MD5_RAND
#elif !defined(OPENSSL_NO_MDC2) && !defined(OPENSSL_NO_DES)
#define USE_MDC2_RAND
#elif !defined(OPENSSL_NO_MD2)
#define USE_MD2_RAND
#else
#error No message digest algorithm available
#endif
#endif

#include <openssl/evp.h>
#define MD_Update(a,b,c)    EVP_DigestUpdate(a,b,c)
#define    MD_Final(a,b)        EVP_DigestFinal_ex(a,b,NULL)
#if defined(USE_MD5_RAND)
#include <openssl/md5.h>
#define MD_DIGEST_LENGTH    MD5_DIGEST_LENGTH
#define MD_Init(a)        EVP_DigestInit_ex(a,EVP_md5(), NULL)
#define    MD(a,b,c)        EVP_Digest(a,b,c,NULL,EVP_md5(), NULL)
#elif defined(USE_SHA1_RAND)
#include <openssl/sha.h>
#define MD_DIGEST_LENGTH    SHA_DIGEST_LENGTH
#define MD_Init(a)        EVP_DigestInit_ex(a,EVP_sha1(), NULL)
#define    MD(a,b,c)        EVP_Digest(a,b,c,NULL,EVP_sha1(), NULL)
#elif defined(USE_MDC2_RAND)
#include <openssl/mdc2.h>
#define MD_DIGEST_LENGTH    MDC2_DIGEST_LENGTH
#define MD_Init(a)        EVP_DigestInit_ex(a,EVP_mdc2(), NULL)
#define    MD(a,b,c)        EVP_Digest(a,b,c,NULL,EVP_mdc2(), NULL)
#elif defined(USE_MD2_RAND)
#include <openssl/md2.h>
#define MD_DIGEST_LENGTH    MD2_DIGEST_LENGTH
#define MD_Init(a)        EVP_DigestInit_ex(a,EVP_md2(), NULL)
#define    MD(a,b,c)        EVP_Digest(a,b,c,NULL,EVP_md2(), NULL)
#endif

#ifndef OPENSSL_NO_ENGINE
void int_RAND_set_callbacks(
    int (*set_rand_func)(const RAND_METHOD *meth,
                        const RAND_METHOD **pmeth),
    const RAND_METHOD *(*get_rand_func)
                        (const RAND_METHOD **pmeth));
int eng_RAND_set_rand_method(const RAND_METHOD *meth,
                const RAND_METHOD **pmeth);
const RAND_METHOD *eng_RAND_get_rand_method(const RAND_METHOD **pmeth);
#endif


#endif
