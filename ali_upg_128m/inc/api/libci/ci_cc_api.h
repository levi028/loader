/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2011 Copyright (C)
*
*    File:    cic.h
*
*    Description:    This file contains all functions definition
*                     of CI+ CC device.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.15th, Jue. 2011     Owen Zhao       Ver 0.1    Create file.
*
*****************************************************************************/

#ifndef __CI_CC_API_H__
#define __CI_CC_API_H__

#include <sys_config.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <hld/dsc/dsc.h>
#include <hld/dmx/dmx.h>
#include <hld/crypto/crypto.h>
#include <api/libci/ci_plus.h>
#include <api/libsi/si_tdt.h>

#define DUMP_HEX            0
#define DUMP_CHAR            1

/*
 *  Crypto Engine MACRO
 */
#define DES_56_ECB        0
#define AES_128_CBC        1


/*
 *  Scrambler capabilities
 *  Table 9.7: Capabilities Supported
 */
#define SCAP_DES                0
#define SCAP_DES_AES            1
#define SCAP_NONE               0xFFFF

/*
 *  Table H.1
 */
#define KEY_EVEN            0
#define KEY_ODD                1
#define KEY_ODD_EVEN        2

/*
 *  cc_system_id_bitmask = 0x01  version 1
 */
#define CC_SYSTEM_ID_BITMASK        0x01

/*
 *  status_field
 *  Table 11.17
 *  00: OK
 *  01: No CC Support
 *  02: Host Busy
 *  03: Authentication failed
 */
#define STATUS_OK                    0
#define STATUS_NO_CC_SUPPORT        1
#define STATUS_HOST_BUSY            2
#define STATUS_AUTH_FAILED            3
/*
 *  Table 11.24
 *  Not Required  03
 */
#define STATUS_NOT_REQUIRED            3
#define STATUS_RESERVED             0xFF

/*
 *  Certificate buffer len
 */
#define CI_CERT_LEN                2048
#define SIGNATURE_LEN           256
#define AKH_LEN                 32
#define HOST_ID_LEN             8
#define CICAM_ID_LEN            8
#define NS_LEN                  8
#define URI_CONFIRM_LEN         32
#define URI_VERSIONS_LEN        32
#define AUTH_NONCE_LEN          32
#define DH_BITS_LEN             2048
#define DH_BYTES_LEN            (DH_BITS_LEN/8)
#define DH_Q_LEN                32
#define kp_len                  32
#define ks_len                  32
#define URI_MESSAGE_DATA_LEN    8
#define UCK_LEN                 32
#define SAK_LEN                 16
#define SLK_LEN                 16
#define CLK_LEN                 16

//ERROR code for CI+
enum ciplus_error_code
{
    ERR_INVALID_NONCE = 1,
    ERR_INVALID_DHPH = 2,
    ERR_INVALID_CERT = 3,
    ERR_CERT_VERIFY_FAIL = 4,
    ERR_CERT_EXPIRED = 5,
    ERR_SIG_B_VERIFY_FAIL = 6,
    ERR_INVALID_DHPM = 7,
    ERR_INVALID_AKH = 8,
    ERR_INVALID_DHSK = 9,
    ERR_CAL_CCK_FAIL = 10,
    ERR_CICAM_ID_NOT_MATCH = 11,

    ERR_SAC_MSG_COUNTER_INVALID = 12,
    ERR_SAC_PROTOCOL_INVALID = 13,
    ERR_SAC_VERIFY_FAIL= 14,
    err_ns_module_invalid = 15
};


/******************************************************************/

/*
 *  @cicam_id     8 byte
 *  @akh        32 byte
 *  @dhsk        256 byte
 *  @scramble_type    1 byte
 *  @slot            1 byte
 *  @brand_id        2 byte
 */
#define AKH_ARRAY_SIZE (8+32+256+8)

/*
 *  We need register a timer handler for the CC device
 *  This handler should be register when init the device
 *  The timer mechanism of this timer handler should be processed
 *  by CI stack. In the CC lib, the handler function and para
 *  will be set.
 */
typedef void (* ci_cc_timer_handler)(void *arg1, void *arg2);

/*
 *  Key header structure stored in flash
 */
typedef struct {
    UINT32 type;
    UINT32 len;
    UINT32 size;
    UINT32 reserved;
} CIPLUS_KEY_HDR;

/*
 *  Structure cic_device, the basic structure between HLD and LLD of CI
 *    controler device.
 */
struct ci_cc_device
{
    /* Device name */
    char        ci_cc_name[HLD_MAX_NAME_SIZE];

    /* Hardware privative structure */
    void        *priv;                /* pointer to private data */

    /* Current used DSC para */
    UINT8       is_dsc_stream_created; /* 1 DSC stream created; 0 No created */
    UINT8       running_scrambler;  /* -1 None; 0 DES_56_ECB; 1 AES_128_CBC */

    /* Current slot parameter */
    UINT8       slot;               /* -1 None; 0 Slot 1; 1 Slot 2 */

    /* Current akh index in flash */
    UINT8       akh_idx;            /* <= 5 */

    /* Varible for crypt engine */
    UINT32      last_handle[CI_MAX_ES_NUM];

    /* Record the content control SC */
    void        *sc;

    /* Record current prog number */
    UINT16      program_number;

    /* Descrambler we used */
    p_aes_dev    p_aes_dev;
    p_des_dev    p_des_dev;
    p_dsc_dev    p_dsc_dev;

    /* Currently descrambling PID */
    UINT16      pid_lst[CI_MAX_ES_NUM];
    UINT8       pid_num;

/*
 *  Functions of this device
 */

/*
 *  Init
 */
    void (*init)(UINT8 slot);
/*
 *  Sometimes CC lib need set a timer, which should be supplied by CI stack
 *  The delete function also should be supplied by CI stack
 */
    INT32 (*ci_set_timer)(UINT32 time, ci_cc_timer_handler h, void *arg1, void *arg2);
    INT32 (*ci_del_timer)(ci_cc_timer_handler h, void *arg1, void *arg2);

/*
 *  URI function won't be included in CC lib, it's supplied by CI stack
 *  to CC lib, CC lib use those functions to notice URI changing
 */
    void (*uri_message)(UINT8 *data, int len, void *uri_msg);
    void (*ci_uri_monitor)(void *appdata, UINT32 prog/*@??*/);
    void (*ci_notify_uri)(UINT32 prog/*@??*/, void *uri);
    /*@??*//* Means paras may need to be modified */

/*
 *  When matching, we need to write AKH, DHSK, etc to flash
 *  Write function should be supplied by application layer
 */
    INT32 (*akh_dhsk_writer)(UINT8 *buf);
    INT32 (*get_hdr)(UINT32 type, CIPLUS_KEY_HDR *hdr);
    INT32 (*host_key_writer)(UINT8 *key, UINT32 len, UINT32 type);

/*
 *  CC lib need get seed from flash
 */
    INT32 (*get_aes_seed)(UINT8 *aes_seed, UINT32 seed_len);

/*
 *  CC lib need parse srm information, this should be done in ci_stack
 *  CC lib just pass srm information to CI stack
 *  CI stack then parse the information
 */
    void (*srm_parser)(UINT8 *srm_file, UINT32 srm_file_len);

/*
 *  CC lib need to operate DSC
 */
    void (*dsc_start)(UINT slot);
    void (*dsc_stop)(UINT slot);
    void (*dsc_close)(UINT slot);
    INT32 (*aes_update_key)(void *priv, p_aes_dev p_aes_dev, UINT16 *pid_lst,
                            UINT8 pid_num, UINT32 last_handle, UINT8 key_index);
    INT32 (*des_update_key)(void *priv, p_des_dev p_des_dev, UINT16 *pid_lst,
                            UINT8 pid_num, UINT32 last_handle, UINT8 key_index);

/*
 *  Calculate key, the function should be maintained by manufacturer
 */
    int (*ci_f_cc)(unsigned char *kp, unsigned long ul_kp_len, unsigned char *clk, unsigned long clk_len,
            unsigned char *cck, unsigned char *civ, unsigned long ul_len, int scrambler);
    int (*ci_f_sac)(unsigned char *ks, unsigned long ul_ks_len, unsigned char *slk, unsigned long slk_len,
            unsigned char *sek, unsigned char *sak, unsigned long ul_len);

/*
 *  Crypto engine must generate key for CC lib
 */
    INT32 (*ce_key_generate)(p_ce_device pcedev,p_ce_data_info cedata_info);

/*
 *  We need to get stream time information for debug
 */
    void (*get_stream_time)(date_time *tmc);
    INT32 (*tdt_status)();
};

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  CC device operation
 */
struct ci_cc_device *ci_cc_attach(UINT8 slot, void (*init)(UINT8 slot));
void ci_cc_dettach(UINT8 slot);
struct ci_cc_device *ci_cc_get_dev(UINT8 slot);
void ci_cc_set_cur_slot(void *priv, UINT8 slot);

/*
 *  Get buf used to store keys
 */
void ci_cc_get_cert_buf(UINT8 **buf, UINT32 **len, UINT32 type);
void ci_cc_get_akhetc_buf(UINT8 **buf, UINT8 idx);

/*
 *  CC authentication
 */
INT32 cc_data_req(void *priv, int len, UINT8 *data, void *data_conf);
UINT8 *cc_data_cnf(void *priv, void *cck_data, UINT32 *len);
INT32 cc_sac_data_req(void *priv, int len, UINT8 *data, void *data_conf);
UINT8 *cc_sac_data_cnf(void *priv, void *cck_data, UINT32 *sac_len);
UINT8 *cc_sac_sync_cnf(void *priv, UINT32 *sac_len);
INT32 cc_sac_sync_req(void *priv, int len, UINT8 *data);

/*
 *  Encrypt interface
 */
RET_CODE decrypt_akh_dhsk();
RET_CODE decrypt_cert(UINT32 type);
RET_CODE encrypt_key(UINT8 *in, UINT8 *out, UINT8 *seed, UINT32 length);

RET_CODE ci_cc_aes_create_stream(void *priv, p_aes_dev p_aes_dev, p_dsc_dev p_dsc_dev,
                                    UINT32 *last_handle, UINT16 *pid_lst, UINT8 pid_num,
                                    UINT8 stream_id, UINT8 key_index);
RET_CODE ci_cc_des_create_stream(void *priv, p_des_dev p_des_dev, p_dsc_dev p_dsc_dev,
                                    UINT32 *last_handle, UINT16 *pid_lst, UINT8 pid_num,
                                    UINT8 stream_id, UINT8 key_index);

/*
 *  Any error happen when authentication
 */
UINT32 ci_auth_get_err_code();

#ifdef __cplusplus
}
#endif

#endif /* __CI_CC_API_H__ */

