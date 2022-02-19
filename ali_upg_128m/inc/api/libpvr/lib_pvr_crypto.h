#ifndef __LIB_PVR_CRYPTO_H__
#define __LIB_PVR_CRYPTO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <api/libpvr/lib_pvr_eng.h>

/* definition of invalid crypto values */
#define INVALID_CRYPTO_KEY_POS            0xff
#define INVALID_CRYPTO_STREAM_ID        0xff
#define INVALID_CRYPTO_STREAM_HANDLE    0xffffffff
#define INVALID_CRYPTO_MODE                0xffffffff

typedef struct
{
    UINT32    dmx_id;
    void   *decrypt_dev;        // DSC parse device
    UINT32     decrypt_mode;         // AES, DES, TDES, CSA
    UINT32    decrypt_stream_id;    // DSC parse stream ID
} dsc_parse_resource;

extern int pvr_crypto_init_ext(PVR_CRYPTO_MODE re_encrypt_mode,
                               struct pvr_crypto_config * cfg);
extern void set_user_crypto_valid( UINT32  input_para[3]);
extern UINT8 get_cur_record_key_mode();
extern UINT8 get_current_record_key_mode();
extern UINT8 get_current_play_key_mode(void);
extern BOOL crypto_mode_change(de_encryp_key_mode new_mode);
extern int pvr_crypto_init(PVR_CRYPTO_MODE re_encrypt_mode);
extern int pvr_crypto_init2(PVR_CRYPTO_MODE re_encrypt_mode, UINT8 keyset);
extern int pvr_crypto_set_dsc_parse(dsc_parse_resource *dpr);

extern int pvr_crypto_rec_config(pvr_crypto_general_param *rec_param,
                                 dsc_parse_resource *dpr);
extern int pvr_crypto_playback_config(pvr_crypto_general_param *play_param);

extern int pvr_crypto_rec_stop(pvr_crypto_general_param *rec_param);
extern int pvr_crypto_playback_stop(pvr_crypto_general_param *play_param);

extern int pvr_crypto_generate_keys(pvr_crypto_key_param *key_param);
extern int pvr_crypto_data(pvr_crypto_data_param *cp);
extern BOOL crypto_mode_change(de_encryp_key_mode new_mode);
extern int get_pvr_user_key(UINT8 *key_value, UINT8 buf_len);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_PVR_CRYPTO_H__ */

