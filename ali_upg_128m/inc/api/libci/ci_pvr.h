#ifndef __CI_PVR_H__
#define __CI_PVR_H__

#include <sys_config.h>

typedef struct
{
    UINT32    pvr_hnd;
    UINT16 *pid_list;
    UINT16    pid_num;
    UINT8    dmx_id;
    UINT8    ca_mode;

    UINT32    key_num;            // total number of keys
    UINT32    key_len;            // the real length of one key, measured in bit
    UINT8  *keys_ptr;            // keys pointer
    UINT8    crypto_mode;        // crypto mode TDES/AES (CBC/ECB)
//    UINT8    rec_special_mode;
    UINT8    resv[3];
    UINT32    resv2;

    // return values
    UINT32    crypto_stream_hnd;
    UINT32    crypto_stream_id;
    UINT32    crypto_key_pos;
} ciplus_crypto_general_param;

extern int ciplus_pvr_rec_config(ciplus_crypto_general_param *rec_param);
extern int ciplus_pvr_playback_config(ciplus_crypto_general_param *play_param);

extern int ciplus_pvr_rec_stop(ciplus_crypto_general_param *rec_param);
extern int ciplus_pvr_playback_stop(ciplus_crypto_general_param *play_param);

extern void ciplus_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id);

extern int ciplus_pvr_init(void);

//extern int ciplus_crypto_data(pvr_crypto_data_param *cp);

#endif /* __CI_PVR_H__ */

