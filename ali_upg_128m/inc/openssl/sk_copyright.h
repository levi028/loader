#include <basic_types.h>
#include <openssl/rsa.h>

#define P_MAX_EVENTPROT_NUM 16
#define P_MAX_NASP_NUM 16

typedef struct
{
    UINT16 event_id;
    UINT8 digital_copy_protection;
    UINT8 analog_copy_protection;
}EVENTPROT_INFO;

typedef struct
{
    EVENTPROT_INFO EVENTPROT_info[P_MAX_EVENTPROT_NUM];
    UINT8 EVENTPROT_count;
}COPYPROT_INFO;

//COPYPROT_INFO *cp_info;

typedef struct
{
    UINT8 right_type;
    UINT16 PPID;
    UINT16 NASP_SID;
    UINT8 level;
    UINT8 unscrambled;
    UINT8 free_access;
}NASP_INFO;

typedef struct
{
    NASP_INFO NASP_info[P_MAX_NASP_NUM];
    UINT8 NASP_count;
}NASPCA_INFO;

//NASPCA_INFO *naspca_info;

struct descriptor {
    UINT8 tag;
    UINT8 len;
    UINT8 data[0];
}__attribute__((packed));

void hdplus_eit_callback(UINT8 *, INT32, COPYPROT_INFO *);
void hdplus_pmt_callback(UINT8 *, INT32, NASPCA_INFO *);

void RSA_key_bin2bn(BIGNUM *sk_n,BIGNUM *sk_d,BIGNUM *sk_p,BIGNUM *sk_q);

