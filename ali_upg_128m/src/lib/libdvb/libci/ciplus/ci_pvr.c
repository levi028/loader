#include <sys_config.h>
#include <api/libci/ci_plus.h>
#include <api/libci/ci_pvr.h>
#include <openssl/fips_rand.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/dmx/dmx.h>
#include <openssl/aes.h>

#include <basic_types.h>
#include <osal/osal.h>
#include <bus/otp/otp.h>
#include <retcode.h>
#include <hal/hal_gpio.h>
#include <hld/crypto/crypto.h>
#include <api/libc/alloc.h>
#include <hld/dmx/dmx_dev.h>


#if 0
#define CIPLUS_PVR_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#define CIPLUS_PVR_ERROR(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#else
#define CIPLUS_PVR_DEBUG(...)            do{} while(0)
#define CIPLUS_PVR_ERROR(...)            do{} while(0)
#endif

#define CIPLUS_PVR_KEY_DEBUG_ENABLE        0
#define CIPLUS_PVR_URI_DEBUG_ENABLE        0


#define INVALID_DSC_SUB_DEV_ID            0xff
#define INVALID_DSC_STREAM_ID            0xff

#define INVALID_CE_KEY_POS                0xff

#define CIPLUS_M2M2_KEY_POS                KEY_0_1
#define CIPLUS_M2M2_KEY_OTP_ADDR        OTP_ADDESS_2

#define CIPLUS_PVR_KEY_POS                KEY_1_1
#define CIPLUS_PVR_KEY_OTP_ADDR            0x70
#define CIPLUS_PVR_KEY_LEN                16        // measured in byte

#define CIPLUS_CRYPTO_KEY_LEVEL            TWO_LEVEL

#define INVALID_CRYPTO_STREAM_ID        INVALID_DSC_STREAM_ID
#define INVALID_CRYPTO_STREAM_HANDLE    0xffffffff

#define INVALID_CRYPTO_DERYPT_TYPE      0xffffffff

#define CIPLUS_PVR_RSC_TYPE_REC            0
#define CIPLUS_PVR_RSC_TYPE_PLAY        1

#define CIPLUS_PVR_RSC_REC_NUM            3
#define CIPLUS_PVR_RSC_PLAY_NUM            1

typedef struct
{
    UINT32    pvr_hnd;
    UINT32    crypto_stream_hnd;    // crypto stream handle used by PVR to record or playback
    UINT32    crypto_stream_id;    // crypto stream id used by PVR to record or playback
    UINT32    crypto_key_pos;        // crypto stream key pos used by PVR to record or playback
} ciplus_pvr_resource;

typedef struct
{
    ID                    mutex_id;
    ciplus_pvr_resource rec[CIPLUS_PVR_RSC_REC_NUM];
    ciplus_pvr_resource play[CIPLUS_PVR_RSC_PLAY_NUM];
} ciplus_pvr_mgr;

#ifdef CI_PLUS_SUPPORT
/* CIPLUS PVR Manager */
static ciplus_pvr_mgr g_cpm;

// crypto related devices
static p_ce_device pvr_ce_dev = NULL;
static p_dsc_dev pvr_dsc_dev  = NULL;
static p_aes_dev pvr_aes_dev  = NULL; // used to encrypt or decrypt TS data

// pvrDeEnconfig is used by other module, MUST defined as static or global
static DEEN_CONFIG pvr_de_enconfig;

#define ciplus_pvr_lock()    osal_mutex_lock(g_cpm.mutex_id, OSAL_WAIT_FOREVER_TIME)
#define ciplus_pvr_un_lock()    osal_mutex_unlock(g_cpm.mutex_id)

extern void *ciplus_get_decrypt_dev(UINT8 slot, int *mode);


static ciplus_pvr_resource *ciplus_pvr_resource_occupy(int rsc_type, UINT32 pvr_hnd)
{
    int i;
    ciplus_pvr_resource *rsc = NULL;

    ciplus_pvr_lock();
    if (rsc_type == CIPLUS_PVR_RSC_TYPE_REC)
    {
        for (i = 0; i < CIPLUS_PVR_RSC_REC_NUM; ++i)
        {
            if (g_cpm.rec[i].pvr_hnd == 0)
            {
                rsc = &g_cpm.rec[i];
                rsc->pvr_hnd = pvr_hnd;
                break;
            }
        }
    }
    else if (rsc_type == CIPLUS_PVR_RSC_TYPE_PLAY)
    {
        for (i = 0; i < CIPLUS_PVR_RSC_PLAY_NUM; ++i)
        {
            if (g_cpm.play[i].pvr_hnd == 0)
            {
                rsc = &g_cpm.play[i];
                rsc->pvr_hnd = pvr_hnd;
                break;
            }
        }
    }
    ciplus_pvr_un_lock();
    return rsc;
}

static ciplus_pvr_resource *ciplus_pvr_resource_find(int rsc_type, UINT32 pvr_hnd)
{
    int i;
    ciplus_pvr_resource *rsc = NULL;

    ciplus_pvr_lock();
    if (rsc_type == CIPLUS_PVR_RSC_TYPE_REC)
    {
        for (i = 0; i < CIPLUS_PVR_RSC_REC_NUM; ++i)
        {
            if (g_cpm.rec[i].pvr_hnd == pvr_hnd)
            {
                rsc = &g_cpm.rec[i];
                break;
            }
        }
    }
    else if (rsc_type == CIPLUS_PVR_RSC_TYPE_PLAY)
    {
        for (i = 0; i < CIPLUS_PVR_RSC_PLAY_NUM; ++i)
        {
            if (g_cpm.play[i].pvr_hnd == pvr_hnd)
            {
                rsc = &g_cpm.play[i];
                break;
            }
        }
    }
    ciplus_pvr_un_lock();
    return rsc;
}


static int ciplus_pvr_resource_release(ciplus_pvr_resource *rsc)
{
    ciplus_pvr_lock();
    rsc->pvr_hnd = 0;
    rsc->crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
    rsc->crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
    rsc->crypto_key_pos = INVALID_CE_KEY_POS;
    ciplus_pvr_un_lock();
    return 0;
}


// @return crypto stream handle
static UINT32 ciplus_aes_key_map_ex(UINT32 key_pos, UINT16 *pid_list, UINT16 pid_num, UINT32 stream_id)
{
    AES_INIT_PARAM aes_param;
    KEY_PARAM key_param;
    RET_CODE ret;

    MEMSET(&aes_param, 0, sizeof(AES_INIT_PARAM));
    aes_param.dma_mode = TS_MODE ;
    aes_param.key_from = KEY_FROM_CRYPTO;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = AUTO_PARITY_MODE0 ;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_ECB;
    aes_param.cbc_cts_enable = 0;
    ret = aes_ioctl(pvr_aes_dev, IO_INIT_CMD, (UINT32)&aes_param);
    if (ret != RET_SUCCESS)
    {
       CIPLUS_PVR_ERROR("AES IO_INIT_CMD fail\n");
       return INVALID_CRYPTO_STREAM_HANDLE;
    }
    
    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.ctr_counter = NULL ;
    key_param.init_vector = NULL ;
    key_param.key_length = 128;
    key_param.pid_len = pid_num;
    key_param.pid_list = pid_list;
    key_param.p_aes_iv_info = NULL ;
    key_param.stream_id = stream_id;
    key_param.force_mode = 1;
    key_param.pos = (UINT8)key_pos;
    ret = aes_ioctl(pvr_aes_dev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("AES IO_CREAT_CRYPT_STREAM_CMD fail\n");
        return INVALID_CRYPTO_STREAM_HANDLE;
    }

    return key_param.handle;
}

// @return crypto stream handle and key pos
static UINT32 ciplus_load_kreci_to_ce(ciplus_crypto_general_param *param, UINT32 stream_id)
{
    UINT32 crypto_stream_hnd;
    RET_CODE ret;

#if (CIPLUS_PVR_KEY_DEBUG_ENABLE)
    CIPLUS_PVR_DEBUG("%s() key: ", __FUNCTION__);
    int i;
    for (i = 0; i < param->key_len / 8; ++i)
    {
        CIPLUS_PVR_DEBUG("%02x ", param->keys_ptr[i]);
    }
    CIPLUS_PVR_DEBUG("\n");
#endif

    param->crypto_key_pos = INVALID_CE_KEY_POS;

    CE_FOUND_FREE_POS_PARAM key_pos_param;
    MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    key_pos_param.ce_key_level = CIPLUS_CRYPTO_KEY_LEVEL;
    key_pos_param.pos = INVALID_CE_KEY_POS;
    key_pos_param.root = CIPLUS_M2M2_KEY_POS;
    ret = ce_ioctl(pvr_ce_dev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("find free key pos fail!\n");
        return INVALID_CRYPTO_STREAM_HANDLE;
    }

    param->crypto_key_pos = key_pos_param.pos;

    ret = ce_generate_cw_key(param->keys_ptr, AES, CIPLUS_PVR_KEY_POS, (UINT8)param->crypto_key_pos);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("ce_generate_cw_key() fail\n");
        ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, param->crypto_key_pos);
        param->crypto_key_pos = INVALID_CE_KEY_POS;
        return INVALID_CRYPTO_STREAM_HANDLE;
    }

    crypto_stream_hnd = ciplus_aes_key_map_ex(param->crypto_key_pos, param->pid_list,
                                              param->pid_num, stream_id);
    if (crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
    {
        ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, param->crypto_key_pos);
        param->crypto_key_pos = INVALID_CE_KEY_POS;
    }

    return crypto_stream_hnd;
}

// configure crypto for re-encrypt ts, and encrypt key
int ciplus_pvr_rec_config(ciplus_crypto_general_param *rec_param)
{
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, rec_param->dmx_id);
    ciplus_pvr_resource *rec_rsc = NULL;
    UINT32 decrypt_stream_id = ciplus_get_ts_stream_id();
    int decrypt_mode = 0;
    void *decrypt_dev =  ciplus_get_decrypt_dev(ci_get_cur_dsc_slot(), &decrypt_mode);

    if (decrypt_stream_id == INVALID_CRYPTO_STREAM_ID)
    {
        CIPLUS_PVR_ERROR("Invalid CIPLUS decrypt stream id: %d\n", decrypt_stream_id);
        return -1;
    }

    if (decrypt_dev == NULL)
    {
        CIPLUS_PVR_ERROR("Invalid CIPLUS decrypt dev: NULL\n");
        return -1;
    }

    rec_rsc = ciplus_pvr_resource_occupy(CIPLUS_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
    if (rec_rsc == NULL)
    {
        CIPLUS_PVR_ERROR("No free CIPLUS PVR record resource\n");
        return -1;
    }

    rec_rsc->crypto_stream_id = dsc_get_free_stream_id(TS_MODE);
    if (rec_rsc->crypto_stream_id == INVALID_CRYPTO_STREAM_ID)
    {
        CIPLUS_PVR_ERROR("No free crypto stream id\n");
        ciplus_pvr_resource_release(rec_rsc);
        return -1;
    }

    rec_rsc->crypto_stream_hnd = ciplus_load_kreci_to_ce(rec_param, rec_rsc->crypto_stream_id);
    if (rec_rsc->crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
    {
        CIPLUS_PVR_ERROR("%s() failed! encrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
            __FUNCTION__, rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id,
            rec_param->crypto_key_pos);
        dsc_set_stream_id_idle(rec_rsc->crypto_stream_id);
        ciplus_pvr_resource_release(rec_rsc);
        return -1;
    }
    rec_rsc->crypto_key_pos = rec_param->crypto_key_pos;

    CIPLUS_PVR_DEBUG("%s()\n\tpvr handle: 0x%X, pid num: %d, dmx_id: %d, decrypt stream id: %d\n"
        "\tencrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
        __FUNCTION__, rec_param->pvr_hnd, rec_param->pid_num, rec_param->dmx_id,
        decrypt_stream_id, rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id,
        rec_rsc->crypto_key_pos);

    pvr_de_enconfig.do_decrypt = 1; // rec_param->ca_mode;
    pvr_de_enconfig.dec_dev = decrypt_dev;
    pvr_de_enconfig.decrypt_mode = decrypt_mode;
    pvr_de_enconfig.dec_dmx_id = decrypt_stream_id;

    pvr_de_enconfig.do_encrypt = 1;
    pvr_de_enconfig.enc_dev = pvr_aes_dev;
    pvr_de_enconfig.encrypt_mode = AES;
    pvr_de_enconfig.enc_dmx_id = rec_rsc->crypto_stream_id;

    dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig);

    /* set other return values */
    rec_param->crypto_stream_hnd = rec_rsc->crypto_stream_hnd;
    rec_param->crypto_stream_id = rec_rsc->crypto_stream_id;
    return 0;
}

// decrypt key and configure crypto for decrypt ts
int ciplus_pvr_playback_config(ciplus_crypto_general_param *play_param)
{
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id);
    ciplus_pvr_resource *play_rsc = NULL;
    struct dec_parse_param p_param;

#if 1 // may be no need to do this
    play_rsc = ciplus_pvr_resource_find(CIPLUS_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (play_rsc != NULL)
    {
        CIPLUS_PVR_ERROR("playback crypto stream resource is not released!\n");
        ciplus_pvr_playback_stop(play_param);
    }
#endif

    play_rsc = ciplus_pvr_resource_occupy(CIPLUS_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (play_rsc == NULL)
    {
        CIPLUS_PVR_ERROR("No free CIPLUS PVR play resource\n");
        return -1;
    }

    play_rsc->crypto_stream_id = dsc_get_free_stream_id(TS_MODE);
    if (play_rsc->crypto_stream_id == INVALID_CRYPTO_STREAM_ID)
    {
        CIPLUS_PVR_ERROR("No free crypto stream id\n");
        ciplus_pvr_resource_release(play_rsc);
        return -1;
    }

    play_rsc->crypto_stream_hnd = ciplus_load_kreci_to_ce(play_param, play_rsc->crypto_stream_id);
    if (play_rsc->crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
    {
        CIPLUS_PVR_ERROR("%s() failed! decrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
            __FUNCTION__, play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id,
            play_rsc->crypto_key_pos);
        dsc_set_stream_id_idle(play_rsc->crypto_stream_id);
        ciplus_pvr_resource_release(play_rsc);
        return -1;
    }
    play_rsc->crypto_key_pos = play_param->crypto_key_pos;

    CIPLUS_PVR_DEBUG("%s()\n\tpvr handle: 0x%X, pid num: %d, dmx_id: %d\n"
        "\tdecrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
        __FUNCTION__, play_param->pvr_hnd, play_param->pid_num, play_param->dmx_id,
        play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id,
        play_rsc->crypto_key_pos);

    // configure crypto for playback
    MEMSET(&p_param, 0, sizeof(struct dec_parse_param));
    p_param.dec_dev = pvr_aes_dev;
    p_param.type = AES;
    dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&p_param);
    dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
    dsc_ioctl(pvr_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, play_rsc->crypto_stream_id);

    /* set other return values */
    play_param->crypto_stream_hnd = play_rsc->crypto_stream_hnd;
    play_param->crypto_stream_id = play_rsc->crypto_stream_id;
    return 0;
}

// If pvr is playback with AES, set DSC again
// @dmx_id: playback dmx ID
// @stream_id: playback crypto stream ID
static void ciplus_set_dsc_for_playback(UINT16 dmx_id, UINT32 stream_id)
{
    struct dec_parse_param param;
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);

    CIPLUS_PVR_DEBUG("%s() dmx_id: %d, stream_id: %d\n", __FUNCTION__, dmx_id, stream_id);

    MEMSET(&param, 0, sizeof(struct dec_parse_param));
    param.dec_dev = pvr_aes_dev;
    param.type = AES;
    dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&param);
    dmx_io_control(dmx, IO_SET_DEC_STATUS, (UINT32)1);
    dsc_ioctl(pvr_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, stream_id);
}

// when pvr stop record or timeshift to live, need to set dsc again
void ciplus_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id)
{
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    struct dec_parse_param param;
    int decrypt_mode = 0;
    void *decrypt_dev = ciplus_get_decrypt_dev(ci_get_cur_dsc_slot(), &decrypt_mode);

    if (decrypt_dev == NULL)
    {
        CIPLUS_PVR_ERROR("Invalid decrypt dev: NULL\n");
        return;
    }

    CIPLUS_PVR_DEBUG("%s() dmx_id: %d, stream_id: %d, decrypte dev: 0x%X, decrypt mode: %d\n",
        __FUNCTION__, dmx_id, stream_id, decrypt_dev, decrypt_mode);

    MEMSET(&param, 0, sizeof(struct dec_parse_param));
    param.dec_dev = decrypt_dev;
    param.type = decrypt_mode;
    dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&param);
    dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
    dsc_ioctl(pvr_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, stream_id);
}

// reset crypto for FTA TS recording
static int ciplus_pvr_reset_rec_config(UINT16 dmx_id)
{
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);

    pvr_de_enconfig.do_decrypt = 0; // for FTA, no need to open decrypt device
    pvr_de_enconfig.dec_dev = NULL;
    pvr_de_enconfig.do_encrypt = 0;
    pvr_de_enconfig.enc_dev = NULL;

    dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig);
    return 0;
}

// when stop record, need delete encrypt stream.
int ciplus_pvr_rec_stop(ciplus_crypto_general_param *rec_param)
{
    RET_CODE ret = RET_SUCCESS;
    ciplus_pvr_resource *rec_rsc = NULL;

    rec_rsc = ciplus_pvr_resource_find(CIPLUS_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
    if (rec_rsc == NULL)
    {
        CIPLUS_PVR_ERROR("Cannot find record crypto stream resource, pvr handle: 0x%X\n", rec_param->pvr_hnd);
        return -1;
    }

    CIPLUS_PVR_DEBUG("%s()\n\tpvr handle: 0x%X, dmx_id: %d\n"
        "\tencrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
        __FUNCTION__, rec_param->pvr_hnd, rec_param->dmx_id,
        rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id, rec_rsc->crypto_key_pos);

    if (rec_rsc->crypto_stream_hnd != INVALID_CRYPTO_STREAM_HANDLE)
        ret = aes_ioctl(pvr_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD, rec_rsc->crypto_stream_hnd);

    if (rec_rsc->crypto_key_pos != INVALID_CE_KEY_POS)
        ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, rec_rsc->crypto_key_pos);

    if (rec_rsc->crypto_stream_id != INVALID_CRYPTO_STREAM_ID)
        dsc_set_stream_id_idle(rec_rsc->crypto_stream_id);

    ciplus_pvr_resource_release(rec_rsc);
    ciplus_pvr_reset_rec_config(rec_param->dmx_id);

#if 0
    if ((g_cpm.play[0].pvr_hnd != NULL) &&
        (g_cpm.play[0].crypto_stream_hnd != INVALID_CRYPTO_STREAM_HANDLE) &&
        (g_cpm.play[0].crypto_stream_id != INVALID_CRYPTO_STREAM_ID))
    {
        ciplus_set_dsc_for_playback(2, g_cpm.play[0].crypto_stream_id);
    }
#endif

    return (ret == RET_SUCCESS) ? 0 : -1;
}

int ciplus_pvr_playback_stop(ciplus_crypto_general_param *play_param)
{
    RET_CODE ret = RET_SUCCESS;

    ciplus_pvr_resource *play_rsc = NULL;

    struct dec_parse_param param;

    play_rsc = ciplus_pvr_resource_find(CIPLUS_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (play_rsc == NULL)
    {
        CIPLUS_PVR_ERROR("Cannot find play crypto stream resource, pvr handle: 0x%X\n", play_param->pvr_hnd);
        return -1;
    }

    CIPLUS_PVR_DEBUG("%s()\n\tpvr handle: 0x%X, dmx_id: %d\n"
        "\tdecrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
        __FUNCTION__, play_param->pvr_hnd, play_param->dmx_id,
        play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id, play_rsc->crypto_key_pos);

    if (play_rsc->crypto_stream_hnd != INVALID_CRYPTO_STREAM_HANDLE)
        ret = aes_ioctl(pvr_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD, play_rsc->crypto_stream_hnd);

    if (play_rsc->crypto_key_pos != INVALID_CE_KEY_POS)
        ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, play_rsc->crypto_key_pos);

    if (play_rsc->crypto_stream_id != INVALID_CRYPTO_STREAM_ID)
        dsc_set_stream_id_idle(play_rsc->crypto_stream_id);

    MEMSET(&param, 0, sizeof(struct dec_parse_param));
    param.dec_dev = NULL;
    param.type = INVALID_CRYPTO_DERYPT_TYPE;

    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id),
                   IO_GET_DEC_HANDLE, (UINT32)&param);

    CIPLUS_PVR_DEBUG("%s: decrypt type = 0x%x, decrypt dev = 0x%x\n",
                    __FUNCTION__, param.type, param.dec_dev);
    CIPLUS_PVR_DEBUG("%s: cur decrypt type = %s, cur decrypt dev = 0x%x\n",
                    __FUNCTION__, "AES", pvr_aes_dev);

    if ((AES != param.type) || (param.dec_dev != pvr_aes_dev))
    {
        CIPLUS_PVR_ERROR("Wrong decrypt type for PVR\n");
    }
    else
    {
        param.dec_dev = NULL;
        param.type = INVALID_CRYPTO_DERYPT_TYPE;
        dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id),
                       IO_SET_DEC_HANDLE, (UINT32)&param);
        dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id),
                        IO_SET_DEC_STATUS, 0);
    }

    ciplus_pvr_resource_release(play_rsc);

//  dmx_io_control(dmx, IO_SET_DEC_STATUS, 0); // don't set this
    return (ret == RET_SUCCESS) ? 0 : -1;
}

static int ciplus_load_m2m2_key(void)
{
    p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    OTP_PARAM opt_info;

    MEMSET(&opt_info, 0, sizeof(OTP_PARAM));
    opt_info.otp_addr = CIPLUS_M2M2_KEY_OTP_ADDR;
    opt_info.otp_key_pos = CIPLUS_M2M2_KEY_POS;

    if (RET_SUCCESS != ce_key_load(p_ce_dev0, &opt_info))
    {
        CIPLUS_PVR_DEBUG("load m2m2 key failed!");
        return -1;
    }
    return 0;
}

static int ciplus_load_pk_to_ce(void)
{
    p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    CE_DATA_INFO ce_data_info;
    UINT8 encrypted_pk[CIPLUS_PVR_KEY_LEN];
    int i;

    otp_init(NULL);
    MEMSET(encrypted_pk, 0, CIPLUS_PVR_KEY_LEN);
    for (i = 0; i < CIPLUS_PVR_KEY_LEN / 4; i++)
        otp_read((CIPLUS_PVR_KEY_OTP_ADDR+i)*4, &encrypted_pk[i*4], 4);

#if (CIPLUS_PVR_KEY_DEBUG_ENABLE)
    CIPLUS_PVR_DEBUG("load PVR key: ");
    for (i = 0; i < CIPLUS_PVR_KEY_LEN; i++)
        CIPLUS_PVR_DEBUG("%02x ",encrypted_pk[i]);
    CIPLUS_PVR_DEBUG("\n");
#endif

    MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));
    MEMCPY(ce_data_info.data_info.crypt_data, encrypted_pk, CIPLUS_PVR_KEY_LEN);
    ce_data_info.data_info.data_len             = CIPLUS_PVR_KEY_LEN;/* aes is 16 bytes des/tdes is 8 bytes*/
    ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
    ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
    ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
    ce_data_info.key_info.first_key_pos         = CIPLUS_M2M2_KEY_POS;
    ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
    ce_data_info.key_info.second_key_pos         = CIPLUS_PVR_KEY_POS;
    ce_data_info.otp_info.otp_addr                 = CIPLUS_M2M2_KEY_OTP_ADDR;    /* get otp data from this address( 0x4D,0x51,0x55,or0x59) */
    ce_data_info.otp_info.otp_key_pos             = CIPLUS_M2M2_KEY_POS;    /*the opt key will load to the position, it  make sure "otp_key_pos" = first_key_pos*/
    if (ce_key_generate(p_ce_dev0, &ce_data_info) != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("generate PVR Key fail!\n");
        return -1;
    }

#ifdef OTP_NOT_FUSED
    CE_DEBUG_KEY_INFO param;
    param.len = 4 ;
    param.sel = CE_KEY_READ ;
    ce_ioctl(p_ce_dev0, IO_CRYPT_DEBUG_GET_KEY, &param);
#endif

    return 0;
}

int ciplus_pvr_init(void)
{
    UINT32 aes_dev_id = INVALID_DSC_SUB_DEV_ID;
    int i;

    CIPLUS_PVR_DEBUG("%s() load m2m2 key and PVR key to CE\n", __FUNCTION__);

    ciplus_load_m2m2_key();
    ciplus_load_pk_to_ce();

    pvr_ce_dev = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    pvr_dsc_dev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

    if ((aes_dev_id = dsc_get_free_sub_device_id(AES)) != INVALID_DSC_SUB_DEV_ID)
    {
        pvr_aes_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aes_dev_id);
    }

    ASSERT(pvr_ce_dev && pvr_dsc_dev && pvr_aes_dev);

    CIPLUS_PVR_DEBUG("%s() occupy AES %d\n", __FUNCTION__, aes_dev_id);

    MEMSET(&g_cpm, 0, sizeof(ciplus_pvr_mgr));
    if ((g_cpm.mutex_id = osal_mutex_create()) == OSAL_INVALID_ID)
    {
        CIPLUS_PVR_ERROR("Create mutex failed!\n");
        ASSERT(0);
        return -1;
    }

    ciplus_pvr_lock();

    for (i = 0; i < CIPLUS_PVR_RSC_REC_NUM; ++i)
    {
        g_cpm.rec[i].pvr_hnd = 0;
        g_cpm.rec[i].crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
        g_cpm.rec[i].crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
        g_cpm.rec[i].crypto_key_pos = INVALID_CE_KEY_POS;
    }

    for (i = 0; i < CIPLUS_PVR_RSC_PLAY_NUM; ++i)
    {
        g_cpm.play[i].pvr_hnd = 0;
        g_cpm.play[i].crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
        g_cpm.play[i].crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
        g_cpm.play[i].crypto_key_pos = INVALID_CE_KEY_POS;
    }

    ciplus_pvr_un_lock();

    return 0;
}

enum ci_pvr_mode ciplus_get_pvr_mode()
{
    struct ci_uri_message uri_msg;
    enum URI_STATE state;

    state = uri_get_msg(&uri_msg);
    if (state == URI_RECEIVED)
    {
        if (uri_msg.emi_copy_control_info == 0)
        {
            CIPLUS_PVR_DEBUG("%s(): re-encrypt: 0, EMI = %d\n", __FUNCTION__, uri_msg.emi_copy_control_info);
            return CI_NO_RE_ENCRYPT;
        }
        else
        {
            CIPLUS_PVR_DEBUG("%s(): re-encrypt: 1, EMI = %d\n", __FUNCTION__, uri_msg.emi_copy_control_info);
            return CI_RE_ENCRYPT;
        }
    }
    else if (state == URI_NOT_RECEIVED) // no received URI
    {
        CIPLUS_PVR_DEBUG("%s(): Have not received URI\n", __FUNCTION__);
        return CI_NO_RECEIVED_URI;
    }
    else // no URI data
    {
        CIPLUS_PVR_DEBUG("%s(): URI is clear\n", __FUNCTION__);
        return CI_NO_MATCH_CAM;
    }
}

// encrypt URI, and pass to PVR with EMI and RL
int ciplus_offer_uri(UINT8 *enc_uri_msg, UINT32 *msglen, UINT8 *emi, UINT8 *rl)
{
    struct ci_uri_message uri_msg;
    enum URI_STATE state;

    MEMSET(&uri_msg, 0, sizeof(struct ci_uri_message));
    state = uri_get_msg(&uri_msg);

#if CIPLUS_PVR_URI_DEBUG_ENABLE
    int i;
    CIPLUS_PVR_DEBUG("offered URI message:\n");
    for (i = 0; i < sizeof(struct ci_uri_message); i++)
        CIPLUS_PVR_DEBUG("0x%X ",*(((UINT8 *)&uri_msg) + i));
    CIPLUS_PVR_DEBUG("\n");
#endif

    // TODO: encrypt URI
    MEMCPY(enc_uri_msg, &uri_msg, sizeof(struct ci_uri_message));

    *msglen = sizeof(struct ci_uri_message);
    *emi = uri_msg.emi_copy_control_info;
    *rl = uri_msg.rl_copy_control_info;
    return 0;
}

int ciplus_fetch_uri(UINT8 *enc_uri_msg, UINT32 msglen)
{
    struct ci_uri_message *uri_msg = (struct ci_uri_message *)enc_uri_msg;

    // TODO: decrypt URI
#if CIPLUS_PVR_URI_DEBUG_ENABLE
    int i;
    CIPLUS_PVR_DEBUG("fetched URI message:\n");
    for (i = 0; i < sizeof(struct ci_uri_message); i++)
        CIPLUS_PVR_DEBUG("0x%X ",*(((UINT8 *)uri_msg) + i));
    CIPLUS_PVR_DEBUG("\n");
#endif

    uri_msg->uri_from_pvr = 0x01;

    ci_del_uri_monitor_timer(ci_get_cur_dsc_slot());
    pvr_notify_uri(ci_get_cur_dsc_slot(), uri_msg);
    return 0;
}


#if 0
/**
 * encrypt/decrypt data: default to (TDES, CBC)
 */
int ciplus_crypto_data(pvr_crypto_data_param *cp)
{
    p_ce_device p_ce_dev0 = NULL;
    p_des_dev p_des_dev = NULL;
    UINT16 pid_list[1] = {0x1fff};
    DES_INIT_PARAM des_param ;
    KEY_PARAM key_param;
    CE_FOUND_FREE_POS_PARAM key_pos_param;
    UINT8 key_pos = INVALID_CE_KEY_POS;
    UINT32 des_dev_id = INVALID_DSC_SUB_DEV_ID;
    UINT32 stream_id = INVALID_CRYPTO_STREAM_ID;
    RET_CODE ret;

#if 0 // not encrypt/decrypt data, only for test
    MEMCPY(cp->output, cp->input, cp->data_len);
    return RET_SUCCESS;
#endif

    if ((des_dev_id = dsc_get_free_sub_device_id(DES)) == INVALID_DSC_SUB_DEV_ID)
    {
        CIPLUS_PVR_ERROR("dsc_get_free_sub_device_id() failed\n");
        return -1;
    }

    if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_CRYPTO_STREAM_ID)
    {
        CIPLUS_PVR_ERROR("dsc_get_free_stream_id() failed\n");
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    p_des_dev = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, des_dev_id);
    if (!(p_ce_dev0 && p_des_dev))
    {
        CIPLUS_PVR_ERROR("No valid device for crypto: (0x%X, 0x%X)\n", p_ce_dev0, p_des_dev);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    key_pos_param.ce_key_level = CIPLUS_CRYPTO_KEY_LEVEL;
    key_pos_param.pos = INVALID_CE_KEY_POS;
    key_pos_param.root = CIPLUS_M2M2_KEY_POS;
    ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("find free key pos fail!\n");
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    // generate encrypting key
    key_pos = (UINT8)key_pos_param.pos;
    ret = ce_generate_cw_key(cp->key_ptr, AES, CIPLUS_PVR_KEY_POS, key_pos);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("generate key fail!\n");
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    MEMSET(&des_param, 0, sizeof(DES_INIT_PARAM));
    des_param.dma_mode = PURE_DATA_MODE;
    des_param.key_from = KEY_FROM_CRYPTO;
    des_param.key_mode = TDES_ABA_MODE;
    des_param.parity_mode = EVEN_PARITY_MODE; //AUTO_PARITY_MODE0;
    des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    des_param.scramble_control = 0;
    des_param.stream_id = stream_id;
    des_param.work_mode = WORK_MODE_IS_CBC;
    des_param.sub_module = TDES;
    des_param.cbc_cts_enable = 0;
    ret = des_ioctl(p_des_dev, IO_INIT_CMD, (UINT32)&des_param);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("des_ioctl() IO_INIT_CMD failed\n");
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.handle = INVALID_CRYPTO_STREAM_HANDLE;
    key_param.ctr_counter = NULL;
    key_param.init_vector = cp->iv_ptr;
    key_param.key_length = cp->key_len; // 128 bits ,or  192bits or 256 bits
    key_param.pid_len = 1; //not used
    key_param.pid_list = pid_list; //not used
    key_param.p_des_iv_info = cp->iv_ptr;
    key_param.p_des_key_info = NULL;
    key_param.stream_id = stream_id; //0-3 for dmx id , 4-7 for pure data mode
    key_param.force_mode = 1;
    key_param.pos = key_pos; //Descrambler can find the key which store in Crypto Engine by the key_pos
    ret = des_ioctl(p_des_dev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("des_ioctl() IO_CREAT_CRYPT_STREAM_CMD failed\n");
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    // encrypt or decrypt data
    if (cp->encrypt)
        ret = des_encrypt(p_des_dev, stream_id, cp->input, cp->output, cp->data_len);
    else
        ret = des_decrypt(p_des_dev, stream_id, cp->input, cp->output, cp->data_len);

    if (ret != RET_SUCCESS)
    {
        CIPLUS_PVR_ERROR("%scrypt pure data failed! ret = %d\n", cp->encrypt ? "En" : "De", ret);
    }

    // release resource
    des_ioctl(p_des_dev, IO_DELETE_CRYPT_STREAM_CMD, key_param.handle);
    ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
    dsc_set_stream_id_idle(stream_id);
    dsc_set_sub_device_id_idle(DES, des_dev_id);
    return (ret == RET_SUCCESS) ? 0 : -1;
}
#endif
#endif
