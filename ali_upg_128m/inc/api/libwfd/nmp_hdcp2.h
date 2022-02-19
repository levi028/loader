/**
 * Header file NMP HDCP2.
 *
 */


#ifndef __NMP_HDCP2_H__
#define __NMP_HDCP2_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  HDCP2 Callback Type enumeration
 *  This enumeration indicates possible responses from hdcp2 via callback function 
 *  #HDCP2_CALLBACK.
 */
typedef enum
{
        eHDCP2_CBT_INIT, /**< Init state. */
        eHDCP2_CBT_AKE, /**< AKE state. */
        eHDCP2_CBT_LC, /**< Locality Check state. */
        eHDCP2_CBT_SKE, /**< SKE state. */
        eHDCP2_CBT_KEYSET, /**< Got Key Set (aes key+riv). */
        eHDCP2_CBT_AUTHENTICATED, /**< HDCP2 Authentication success. */
        eHDCP2_CBT_ERR, /**< Error response from hdcp2. */
        eHDCP2_CBT_MAX, /**< Just mark the end of response enumeration. */
}eHDCP2_CALLBACK_TYPE;


typedef struct
{
    unsigned char aes_key[16];
    unsigned char riv[8];
}gHDCP2_KEY_SET;


#define RXID_LEN (40/8)
#define KPUBRX_LEN (1048/8)
#define CERT_RESERVED_LEN (16/8)
#define CERT_SIGN_LEN (3072/8)
#define LC128_LEN (128/8)
#define KPRIVRX_LEN (2560/8)

typedef struct
{
  unsigned char receiverId[RXID_LEN];
  unsigned char kpubRx[KPUBRX_LEN];
  unsigned char reserved[CERT_RESERVED_LEN];
  unsigned char certRx[CERT_SIGN_LEN];
} H2_ReceiverPubCert;


typedef struct {
   unsigned char lc[LC128_LEN];
   H2_ReceiverPubCert rxPubKeyCert;
   unsigned char kPrivRx[KPRIVRX_LEN];
} tReceiverKeySets;

typedef void(* HDCP2_CALLBACK)(eHDCP2_CALLBACK_TYPE type, void *data);

int hdcp2_open(HDCP2_CALLBACK hdcp2_cbk, unsigned int port);

int hdcp2_close(int plugout );

int hdcp2_load_KeySets(tReceiverKeySets *h2_KeySets);

#ifdef __cplusplus
}
#endif

#endif
