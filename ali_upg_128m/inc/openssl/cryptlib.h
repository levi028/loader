#ifndef HEADER_CRYPTLIB_H
#define HEADER_CRYPTLIB_H

#include <stdlib.h>
#include <string.h>

#include <openssl/e_os.h>

#ifdef OPENSSL_USE_APPLINK
#define BIO_FLAGS_UPLINK 0x8000
#include "ms/uplink.h"
#endif

#include <openssl/crypto.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/opensslconf.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef OPENSSL_SYS_VMS
#define X509_CERT_AREA        OPENSSLDIR
#define X509_CERT_DIR        OPENSSLDIR "/certs"
#define X509_CERT_FILE        OPENSSLDIR "/cert.pem"
#define X509_PRIVATE_DIR    OPENSSLDIR "/private"
#else
#define X509_CERT_AREA        "SSLROOT:[000000]"
#define X509_CERT_DIR        "SSLCERTS:"
#define X509_CERT_FILE        "SSLCERTS:cert.pem"
#define X509_PRIVATE_DIR        "SSLPRIVATE:"
#endif

#define X509_CERT_DIR_EVP        "SSL_CERT_DIR"
#define X509_CERT_FILE_EVP       "SSL_CERT_FILE"

/* size of string representations */
#define DECIMAL_SIZE(type)    ((sizeof(type)*8+2)/3+1)
#define HEX_SIZE(type)        (sizeof(type)*2)

void OPENSSL_cpuid_setup(void);
extern unsigned long OPENSSL_ia32cap_P;
void OPENSSL_showfatal(const char *,...);
void *OPENSSL_stderr(void);
extern int OPENSSL_NONPIC_relocated;

#ifdef  __cplusplus
}
#endif

#endif
