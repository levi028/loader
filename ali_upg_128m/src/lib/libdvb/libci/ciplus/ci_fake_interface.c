/*****************************************************************************
 *    Copyrights(C) 2010 ALI Corp. All Rights Reserved.
 *
 *    FILE NAME:        ci_fake_interface.c
 *
 *    DESCRIPTION:     CI+ fake interfaces
 *
 *    HISTORY:
 *        Date         Author         Version     Notes
 *        =========    =========    =========    =========
 *        2010/2/26   Steven      0.1
 *
 *****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
//#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>

#include <openssl/ossl_typ.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <api/libci/ci_plus.h>
#include <api/libci/ci_cc_api.h>
#ifdef DSC_SUPPORT
#include <hld/dsc/dsc.h>
#endif
#include "ci_license.h"
#include "ci_stack.h"
//#include <api/libpvr/lib_pvr3.h>
//#include <hld/cic/cic.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_frontend.h>
#include <api/libdb/db_interface.h>

#define INVALID_DSC_SUB_DEV_ID            0xff
#define INVALID_DSC_STREAM_ID            0xff
#define INVALID_DSC_SLOT_NO             0xff
#define INVALID_DSC_DERYPT_TYPE         0xffffffff

static UINT32 ci_tuned_tp_id = 0;
static BOOL is_tuned_tp_in_db = FALSE;
static UINT32 ci_camup_match_tp = -1;
#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
extern UINT32 ciplus_cc_begin_tick;
extern UINT32 ciplus_cc_prev_tick;
#endif

struct HC_ID
{
    UINT8 slot;
    UINT16 network_id;
    UINT16 org_network_id;
    UINT16 ts_id;
    UINT16 service_id;
};

static struct HC_ID ci_tuned_id;
UINT32 ciplus_camup_prog_id = 0;

#ifdef CI_PLUS_SUPPORT
static UINT8 ciplus_current_dsc_slot = INVALID_DSC_SLOT_NO;
static OSAL_ID dsc_mutex = OSAL_INVALID_ID;
/*#define    enter_dsc_mutex()    \
            do{    \
                if (dsc_mutex == OSAL_INVALID_ID)    \
                {    \
                    dsc_mutex = osal_mutex_create();    \
                    ASSERT(dsc_mutex != OSAL_INVALID_ID);    \
                }    \
                osal_mutex_lock(dsc_mutex, OSAL_WAIT_FOREVER_TIME);    \
            }while(0)
#define    leave_dsc_mutex() \
            do{    \
                ASSERT(dsc_mutex != OSAL_INVALID_ID);\
                osal_mutex_unlock(dsc_mutex);    \
            }while(0)
*/
void    enter_dsc_mutex(){    \
            do{    \
                if (dsc_mutex == OSAL_INVALID_ID)    \
                {    \
                    dsc_mutex = osal_mutex_create();    \
                    ASSERT(dsc_mutex != OSAL_INVALID_ID);    \
                }    \
                osal_mutex_lock(dsc_mutex, OSAL_WAIT_FOREVER_TIME);    \
            }while(0);}
void    leave_dsc_mutex(){ \
            do{    \
                ASSERT(dsc_mutex != OSAL_INVALID_ID);\
                osal_mutex_unlock(dsc_mutex);    \
            }while(0);}

#define PID_MASK    0x1FFF
#define DUMP_PRINTF     PRINTF

void dump(const unsigned char *b, char *s, int n, int type)
{
    if (s)
        DUMP_PRINTF("%s",s);

    if (type == DUMP_HEX)
    {
        while(n-- > 0)
        {
            DUMP_PRINTF(" %02x",*b++);
        }
    }
    else
    {
        DUMP_PRINTF(" ");
        while(n-- > 0)
        {
            DUMP_PRINTF("%c",*b++);
        }
    }
    DUMP_PRINTF("\n");
}

/////////////////////////////////////////////////////////////////////////

#ifdef DSC_SUPPORT
struct ciplus_crypto_para
{
    p_des_dev  p_des_dev;
    UINT8     des_dev_id;
    p_aes_dev  p_aes_dev;
    UINT8     aes_dev_id;
    p_dsc_dev  p_dsc_dev;

    UINT32    last_handle[CI_MAX_ES_NUM];
    UINT16    pid_list[CI_MAX_ES_NUM];
    UINT8     pid_num;
    UINT8     stream_id;
    UINT8     slot;
};
static struct ciplus_crypto_para ciplus_crypto =
    {
        .aes_dev_id = INVALID_DSC_SUB_DEV_ID,
        .des_dev_id = INVALID_DSC_SUB_DEV_ID,
        .stream_id = INVALID_DSC_STREAM_ID
    };
#endif

#ifdef DSC_SUPPORT
extern RET_CODE des_api_decrypt(p_des_dev p_des_dev,UINT16 dmx_id,  UINT8 *input, UINT8 *output, UINT32 total_length);
extern RET_CODE aes_api_decrypt(p_aes_dev p_aes_dev,UINT8 dmx_id, UINT8 *input, UINT8 *output, UINT32 total_length);
#endif

UINT8 ci_get_cur_dsc_slot()
{
    return ciplus_current_dsc_slot;
}

//destroy(stop) stream when channel change/enter menu,
//but still need keep scrambler type & key.
RET_CODE ciplus_dsc_stop(UINT8 slot)
{
#ifdef DSC_SUPPORT
    RET_CODE  ret = SUCCESS;
    struct ci_cc_device *cc_dev;
    UINT8 i = 0;

    struct dec_parse_param param;

    UINT32 decrypt_type = INVALID_DSC_DERYPT_TYPE;
    void *decrypt_dev = NULL;

    cc_dev = ci_cc_get_dev(slot);

    APPL_PRINTF("%s\n",__FUNCTION__);

    enter_dsc_mutex();

    if (cc_dev && cc_dev->is_dsc_stream_created)
    {
        APPL_PRINTF("%s: pid num = %d, running scrambler = %s \n", __FUNCTION__, cc_dev->pid_num,
                    ((cc_dev->running_scrambler == 0)?"DES_56_ECB":"AES_128_CBC"));
        APPL_PRINTF("%s: dsc stream created = %d \n", __FUNCTION__, cc_dev->is_dsc_stream_created);

        if (cc_dev->running_scrambler== DES_56_ECB)
        {
            for (i = 0; i < cc_dev->pid_num; i++)
            {
                APPL_PRINTF("%s: 0x%x,last handle = %d, pid = 0x%x \n", __FUNCTION__,
                            ciplus_crypto.p_des_dev,cc_dev->last_handle[i], cc_dev->pid_lst[i]);
                ret = des_ioctl(ciplus_crypto.p_des_dev ,IO_DELETE_CRYPT_STREAM_CMD ,
                                cc_dev->last_handle[i]);
            }
            decrypt_type = DES;
            decrypt_dev = ciplus_crypto.p_des_dev;
        }
        else if (cc_dev->running_scrambler == AES_128_CBC)
        {
            for (i = 0; i < cc_dev->pid_num; i++)
            {
                APPL_PRINTF("%s: last handle = %d, pid = 0x%x \n", __FUNCTION__,
                            cc_dev->last_handle[i], cc_dev->pid_lst[i]);
                ret = aes_ioctl(ciplus_crypto.p_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD , cc_dev->last_handle[i]);
            }
            decrypt_type = AES;
            decrypt_dev = ciplus_crypto.p_aes_dev;
        }
        else
        {
            APPL_PRINTF("%s - bad scrambler %d\n",__FUNCTION__,cc_dev->running_scrambler);
            return RET_FAILURE;
        }

        param.dec_dev = NULL;
        param.type = INVALID_DSC_DERYPT_TYPE;

        dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
                       IO_GET_DEC_HANDLE, (UINT32)&param);

        APPL_PRINTF("%s: decrypt type = 0x%x, decrypt dev = 0x%x\n",
                        __FUNCTION__, param.type, param.dec_dev);
        APPL_PRINTF("%s: cur decrypt type = %s, cur decrypt dev = 0x%x\n",
                        __FUNCTION__, (decrypt_type == AES)?"AES":"DES", decrypt_dev);

        if ((decrypt_type == param.type) && (param.dec_dev == decrypt_dev))
        {
            param.dec_dev = NULL;
            param.type = INVALID_DSC_DERYPT_TYPE;
            dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
                           IO_SET_DEC_HANDLE, (UINT32)&param);
            dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
                            IO_SET_DEC_STATUS, 0);
        }

        cc_dev->is_dsc_stream_created = 0;
        APPL_PRINTF("%s: dsc stream created = %d \n", __FUNCTION__, cc_dev->is_dsc_stream_created);
    }

    if (INVALID_DSC_SUB_DEV_ID != ciplus_crypto.des_dev_id)
    {
        dsc_set_sub_device_id_idle(DES, ciplus_crypto.des_dev_id);
        ciplus_crypto.des_dev_id = INVALID_DSC_SUB_DEV_ID;
    }

    if (INVALID_DSC_SUB_DEV_ID != ciplus_crypto.aes_dev_id)
    {
        dsc_set_sub_device_id_idle(AES, ciplus_crypto.aes_dev_id);
        ciplus_crypto.aes_dev_id = INVALID_DSC_SUB_DEV_ID;
    }

    if (INVALID_DSC_STREAM_ID != ciplus_crypto.stream_id)
    {
        dsc_set_stream_id_idle(ciplus_crypto.stream_id);
        ciplus_crypto.stream_id = INVALID_DSC_STREAM_ID;
    }

    if (cc_dev)
    {
        cc_dev->p_aes_dev = NULL;
        cc_dev->p_des_dev = NULL;
        cc_dev->p_dsc_dev = NULL;
        cc_dev->pid_num = 0;
    }

    ciplus_crypto.p_aes_dev = NULL;
    ciplus_crypto.p_des_dev = NULL;
    ciplus_crypto.p_dsc_dev = NULL;
    ciplus_crypto.pid_num = 0;

    ciplus_current_dsc_slot = INVALID_DSC_SLOT_NO;

    leave_dsc_mutex();
    return ret;
#endif
}

UINT32 ciplus_get_ts_stream_id()
{
    return ciplus_crypto.stream_id;
}

void *ciplus_get_decrypt_dev(UINT8 slot, int *mode)
{
    struct ci_cc_device *cc_dev;

    if (slot > CI_MAX_SLOT_NUM - 1) return NULL;

    cc_dev = ci_cc_get_dev(slot);

    if (NULL == cc_dev) return NULL;

    if (DES_56_ECB == cc_dev->running_scrambler)
    {
        if (mode) *mode = DES;
        return cc_dev->p_des_dev;
    }
    else if (AES_128_CBC == cc_dev->running_scrambler)
    {
        if (mode) *mode = AES;
        return cc_dev->p_aes_dev;
    }

    return NULL;
}


//start scrambler with last keys(no new key) when channel change.
RET_CODE ciplus_dsc_start(UINT8 slot)
{
#ifdef DSC_SUPPORT
#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
    UINT32 ciplus_tmp_tick = osal_get_tick();
    libc_printf("# Enter (@%s)\n", __FUNCTION__);
    libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", ciplus_tmp_tick, ciplus_tmp_tick-ciplus_cc_begin_tick, ciplus_tmp_tick-ciplus_cc_prev_tick);
    ciplus_cc_prev_tick = ciplus_tmp_tick;
#endif

    struct ci_cc_device *cc_dev;
    UINT8 i = 0;
    UINT8 aes_dev_id, des_dev_id;
    UINT8 stream_id;
    p_aes_dev p_aes_dev = NULL;
    p_des_dev p_des_dev = NULL;

    struct dec_parse_param param;
    void *decrypt_dev = NULL;
    UINT32 decrypt_type = INVALID_DSC_DERYPT_TYPE;

    UINT8 dsc_reset_needed = FALSE;

    ciplus_current_dsc_slot = slot;

    ciplus_crypto.pid_num = 0;

    /* We need to init the dscrambler parameters */
    ciplus_crypto.p_dsc_dev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
    ciplus_crypto.slot = slot;

    aes_dev_id = INVALID_DSC_SUB_DEV_ID;
    des_dev_id = INVALID_DSC_SUB_DEV_ID;
    stream_id = INVALID_DSC_STREAM_ID;

    if ((aes_dev_id = dsc_get_free_sub_device_id(AES)) != INVALID_DSC_SUB_DEV_ID)
    {
        p_aes_dev= (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aes_dev_id);
    }

    if ((des_dev_id = dsc_get_free_sub_device_id(DES)) != INVALID_DSC_SUB_DEV_ID)
    {
        p_des_dev = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, des_dev_id);
    }

    stream_id = dsc_get_free_stream_id(TS_MODE);

    /* Currently we have the URI received, so we should have the full
     * pid list in ci information, we just need to get it
     */
    api_ci_get_dscpid_lst(1 << slot, ciplus_crypto.pid_list,
                          &ciplus_crypto.pid_num, CI_MAX_ES_NUM);

    cc_dev = ci_cc_get_dev(slot);

    if (NULL == cc_dev) return RET_FAILURE;

    APPL_PRINTF("%s: current program number = %d \n", __FUNCTION__, cc_dev->program_number);

    APPL_PRINTF("DSC start\n");

//    enter_dsc_mutex();
    //use last key
    if (cc_dev->running_scrambler != -1)
    {
        //re-create stream
        if (cc_dev->is_dsc_stream_created == 0)
        {
            //We need to set the device only when no stream created
            ciplus_crypto.p_aes_dev = p_aes_dev;
            ciplus_crypto.p_des_dev= p_des_dev;

            cc_dev->p_aes_dev = ciplus_crypto.p_aes_dev;
            cc_dev->p_des_dev = ciplus_crypto.p_des_dev;
            cc_dev->p_dsc_dev = ciplus_crypto.p_dsc_dev;
            MEMCPY(cc_dev->pid_lst, ciplus_crypto.pid_list, ciplus_crypto.pid_num * sizeof(UINT16));
            cc_dev->pid_num = ciplus_crypto.pid_num;

            ciplus_crypto.aes_dev_id = aes_dev_id;
            ciplus_crypto.des_dev_id = des_dev_id;
            ciplus_crypto.stream_id = stream_id;

            APPL_PRINTF("%s: pid num = %d, running scrambler = %s \n", __FUNCTION__, cc_dev->pid_num,
                        ((cc_dev->running_scrambler == 0)?"DES_56_ECB":"AES_128_CBC"));
            for (i = 0; i < cc_dev->pid_num; i++)
            {
                APPL_PRINTF("%s: pid = 0x%x \n", __FUNCTION__, cc_dev->pid_lst[i]);
            }

            if (cc_dev->running_scrambler == DES_56_ECB)
            {
                for (i = 0; i < cc_dev->pid_num; i++)
                {
                    ci_cc_des_create_stream(cc_dev->priv, cc_dev->p_des_dev, cc_dev->p_dsc_dev,
                                        &cc_dev->last_handle[i], cc_dev->pid_lst + i,
                                        1, ciplus_crypto.stream_id, i);
                    APPL_PRINTF("%s: last handle = %d, pid =  0x%x\n", __FUNCTION__,
                                cc_dev->last_handle[i], *(cc_dev->pid_lst + i));
                }

                decrypt_dev = cc_dev->p_des_dev;
                decrypt_type = DES;
            }
            else if (cc_dev->running_scrambler == AES_128_CBC)
            {
                for (i = 0; i < cc_dev->pid_num; i++)
                {
                    ci_cc_aes_create_stream(cc_dev->priv, cc_dev->p_aes_dev, cc_dev->p_dsc_dev,
                                        &cc_dev->last_handle[i], cc_dev->pid_lst + i,
                                        1, ciplus_crypto.stream_id, i);
                    APPL_PRINTF("%s: last handle = %d, pid =  0x%x\n", __FUNCTION__,
                                cc_dev->last_handle[i], *(cc_dev->pid_lst + i));
                }

                decrypt_dev = cc_dev->p_aes_dev;
                decrypt_type = AES;

            }
            else
            {
                leave_dsc_mutex();
                APPL_PRINTF("%s - bad scrambler %d\n",__FUNCTION__,cc_dev->running_scrambler);
                return RET_FAILURE;
            }

            param.dec_dev = NULL;
            param.type = INVALID_DSC_DERYPT_TYPE;

            dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
                            IO_GET_DEC_HANDLE, (UINT32)&param);

            APPL_PRINTF("%s: decrypt type = 0x%x, decrypt dev = 0x%x\n",
                        __FUNCTION__, param.type, param.dec_dev);

            if ((INVALID_DSC_DERYPT_TYPE == param.type) && (NULL == param.dec_dev))
            {
                APPL_PRINTF("%s: set dscrambler for live CIPLUS channel\n", __FUNCTION__);
                param.dec_dev = decrypt_dev;
                param.type = decrypt_type;
                dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
                               IO_SET_DEC_HANDLE, (UINT32)&param);
                dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0),
                               IO_SET_DEC_STATUS, (UINT32)1);
                dsc_ioctl(ciplus_crypto.p_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, ciplus_crypto.stream_id);
            }

            cc_dev->is_dsc_stream_created = 1;
        }
        else
        {
            //We have streams available, so we need to free the applied resource
            //We need to set device to idle when we start it for the following case
            //    @play channel 1
            //    @change to channel 2
            //    @change to channel 1 quickly
            //    @    cc_play_channel is interrupt between cc_pre_play_channel and cc_post_play_channel
            //    @    so we have unmatched uri and channel pid info
            dsc_set_sub_device_id_idle(DES, des_dev_id);
            dsc_set_sub_device_id_idle(AES, aes_dev_id);
            dsc_set_stream_id_idle(stream_id);

            if (ciplus_crypto.pid_num != cc_dev->pid_num)
                dsc_reset_needed = TRUE;

            if (!dsc_reset_needed)
            {
                for (i = 0; i < cc_dev->pid_num; i++)
                {
                    if (ciplus_crypto.pid_list[i] != cc_dev->pid_lst[i])
                    {
                        dsc_reset_needed = TRUE;
                        break;
                    }
                }
            }

            if (dsc_reset_needed)
            {
                APPL_PRINTF("%s: warning, PID change, need reset DSC\n", __FUNCTION__);
                api_ci_msg_to_app(slot, API_MSG_CIPLUS_DSC_RESET);
            }

               APPL_PRINTF("%s - %d in descrambling!\n",__FUNCTION__,cc_dev->running_scrambler);
        }
    }
    else
    {
        APPL_PRINTF("DSC not really started for no key!\n");
        return RET_FAILURE;
    }
#ifdef CI_PLUS_CHANGE_CHANNEL_TIME_DEBUG
    ciplus_tmp_tick = osal_get_tick();
    libc_printf("# Leave (@%s)\n", __FUNCTION__);
    libc_printf("#\t tic = %d, tic used = %d, gap = %d\n", ciplus_tmp_tick, ciplus_tmp_tick-ciplus_cc_begin_tick, ciplus_tmp_tick-ciplus_cc_prev_tick);
    ciplus_cc_prev_tick = ciplus_tmp_tick;
#endif

    return RET_SUCCESS;

//    leave_dsc_mutex();
#endif
}
//when plug out, close the scrambler
RET_CODE ciplus_dsc_close(UINT8 slot)
{
#ifdef DSC_SUPPORT
    struct ci_cc_device *cc_dev;

    cc_dev = ci_cc_get_dev(slot);

    APPL_PRINTF("%s\n",__FUNCTION__);

    ciplus_dsc_stop(slot);

    if (cc_dev)
        cc_dev->running_scrambler = -1;

    //reset PID, KEY, IV
    //...
#endif
}

//when pvr stop record or timeshift to live, need to set dsc again
void ciplus_pvr_dsc_to_live(UINT8 slot)
{
    UINT32 para = 0;
    struct dec_parse_param param;
    struct ci_cc_device *cc_dev;

#ifdef DSC_SUPPORT
    cc_dev = ci_cc_get_dev(slot);

    APPL_PRINTF("%s: set dsc for live play\n", __FUNCTION__);

    if (ciplus_crypto.p_dsc_dev == NULL)
    {
        ciplus_crypto.p_dsc_dev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
        if(ciplus_crypto.p_dsc_dev==NULL) {
            APPL_PRINTF("%s - get DSC Dev failed!\n",__FUNCTION__);
        }
        else {
            cc_dev->p_dsc_dev = ciplus_crypto.p_dsc_dev;
        }
    }
    if(cc_dev->is_dsc_stream_created == 1)
    {
        if (cc_dev->running_scrambler == DES_56_ECB)
        {
            if (cc_dev->p_des_dev == NULL)
            {
                if ((ciplus_crypto.des_dev_id = dsc_get_free_sub_device_id(DES)) != INVALID_DSC_SUB_DEV_ID)
                {
                    ciplus_crypto.p_des_dev= (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, ciplus_crypto.des_dev_id);
                    cc_dev->p_des_dev = ciplus_crypto.p_des_dev;
                }
            }

            param.dec_dev = ciplus_crypto.p_des_dev;
            param.type = DES;
        }
        else if (cc_dev->running_scrambler == AES_128_CBC)
        {
            if (cc_dev->p_aes_dev == NULL)
            {
                if ((ciplus_crypto.aes_dev_id = dsc_get_free_sub_device_id(AES)) != INVALID_DSC_SUB_DEV_ID)
                {
                    ciplus_crypto.p_aes_dev= (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_AES, ciplus_crypto.aes_dev_id);
                    cc_dev->p_aes_dev = ciplus_crypto.p_aes_dev;
                }
            }

            param.dec_dev = ciplus_crypto.p_aes_dev;
            param.type = AES;
        }
        else
        {
            APPL_PRINTF("%s - back to channel with descrambler but not DES or AES\n", __FUNCTION__);
            return;
        }
    }
    else
    {
        APPL_PRINTF("%s - back to channel without descrambler\n", __FUNCTION__);
        return;
    }

    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_SET_DEC_HANDLE, (UINT32)&param);
    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_SET_DEC_STATUS, (UINT32)1);
    dsc_ioctl(ciplus_crypto.p_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, ciplus_crypto.stream_id);
#endif
}

#ifdef CI_PLUS_TEST_CASE

#include <hld/dis/vpo.h>
#include <hld/snd/snd.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <api/libtsi/db_3l.h>

extern struct vpo_device*   g_vpo_dev;
extern struct vpo_device*   g_sd_vpo_dev;
extern struct snd_device*   g_snd_dev;

static BOOL ICT = FALSE;
static BOOL MG = FALSE;
UINT16 pid_list[3] = {0x1fff, 0x1fff, 0x1fff};

extern UINT32 ciplus_case_idx_spdif;
//CI PLUS TEST CASE
BOOL api_ciplus_test_spdif(UINT32 case_idx)
{
    struct snd_spdif_scms spdif;
    //spdif.category_code = CATEGORY_CODE_EUROPE;
    spdif.category_code = 0x0c;

    switch(case_idx)
    {
        case 51:
            //SPDIF: Cp_bit=1,L_bit=0
            spdif.copyright = 1;
            spdif.l_bit = 0;
            snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SET_SPDIF_SCMS, (UINT32)&spdif);
            break;
        case 52:
            //SPDIF: Cp_bit=0,L_bit=1
            spdif.copyright = 0;
            spdif.l_bit = 1;
            snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SET_SPDIF_SCMS, (UINT32)&spdif);
            break;
        case 53:
            //SPDIF: Cp_bit=0,L_bit=0
            spdif.copyright = 0;
            spdif.l_bit = 0;
            snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SET_SPDIF_SCMS, (UINT32)&spdif);
            break;
        default:
            break;
    }
    //CIPlus_case_idx_spdif = 0xff;
    return TRUE;
}

BOOL api_ciplus_testcase(UINT32 case_idx)
{
    struct ci_uri_message uri_msg_test;
    struct snd_spdif_scms spdif;
    struct vpo_io_cgms_info cgms;
    struct vpo_io_get_info dis_info;
    int slot = 0;

    struct ci_cc_device *cc_dev = ci_cc_get_dev(ci_get_cur_dsc_slot());

    if (NULL == cc_dev) return FALSE;

    //MEMSET(&uri_msg_test, 0, sizeof(struct ci_uri_message));
    uri_msg_test.protocol_version = 0x01;
    uri_msg_test.emi_copy_control_info = 0x11;

    switch(case_idx)
    {
        case 10:
            //DES
            pid_list[0] = 0x900;
            pid_list[1] = 0x901;
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_des_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 20:
            //AES SD
            pid_list[0] = 513;
            pid_list[1] = 660;
            //aes_fix();
            //dsc_api_attach();
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 30:
            //AES HD
            pid_list[0] = 6410;
            pid_list[1] = 6420;
            //aes_fix();
            //dsc_api_attach();
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 31:
            //AES HD
            pid_list[0] = 33;
            pid_list[1] = 34;
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 32:
            //AES HD
            pid_list[0] = 1535;
            pid_list[1] = 1539;
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 34:
            //AES HD
            pid_list[0] = 2001;
            pid_list[1] = 2002;
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 35:
            //AES HD
            pid_list[0] = 1025;
            pid_list[1] = 1042;
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 36:
            //AES HD
            pid_list[0] = 6210;
            pid_list[1] = 6221;
            cc_dev->des_update_key(cc_dev->priv, cc_dev->p_aes_dev, pid_list,
                                   2, cc_dev->last_handle);

            MG = FALSE;
            break;
        case 40:
            //stop DES
            des_ioctl(cc_dev->p_des_dev ,IO_DELETE_CRYPT_STREAM_CMD , cc_dev->last_handle);
            dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_SET_DEC_STATUS, 0);

            MG = FALSE;
            break;
        case 50:
            //stop AES
            aes_ioctl(cc_dev->p_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD , cc_dev->last_handle);
            dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_SET_DEC_STATUS, 0);

            MG = FALSE;
            break;
        case 61:
            break;
        case 62:
            break;
        case 63:
            break;
        case 64:
            break;
        case 71:
            //CGMS: APS=0,CGMS_A=1
            cgms.aps = 0;
            cgms.cgms = 1;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 72:
            //CGMS: APS=1,CGMS_A=1
            cgms.aps = 1;
            cgms.cgms = 1;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 73:
            //CGMS: APS=2,CGMS_A=1
            cgms.aps = 2;
            cgms.cgms = 1;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 74:
            //CGMS: APS=3,CGMS_A=1
            cgms.aps = 3;
            cgms.cgms = 1;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 75:
            //CGMS: APS=0,CGMS_A=3
            cgms.aps = 0;
            cgms.cgms = 3;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 76:
            //CGMS: APS=1,CGMS_A=3
            cgms.aps = 1;
            cgms.cgms = 3;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 77:
            //CGMS: APS=2,CGMS_A=3
            cgms.aps = 2;
            cgms.cgms = 3;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 78:
            //CGMS: APS=3,CGMS_A=3
            cgms.aps = 3;
            cgms.cgms = 3;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 79:
            //CGMS: APS=0,CGMS_A=2
            cgms.aps = 0;
            cgms.cgms = 2;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 80:
            //CGMS: APS=1,CGMS_A=2
            cgms.aps = 1;
            cgms.cgms = 2;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 81:
            //CGMS: APS=2,CGMS_A=2
            cgms.aps = 2;
            cgms.cgms = 2;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 82:
            //CGMS: APS=3,CGMS_A=2
            cgms.aps = 3;
            cgms.cgms = 2;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 83:
            //CGMS: APS=0,CGMS_A=0
            cgms.aps = 0;
            cgms.cgms = 0;
            vpo_ioctl(g_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);
            vpo_ioctl(g_sd_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cgms);

            APPL_PRINTF("%s: APS=%d, CGMS=%d\n",__FUNCTION__, cgms.aps, cgms.cgms);
            MG = FALSE;
            break;
        case 99:
            //ICT: Analog SD(DE scaling down) + HDMI SD(DE scaling down)
            uri_msg_test.ict_copy_control_info = 1;
            if(uri_msg_test.ict_copy_control_info == 1)
            {
                vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
                if((!dis_info.bprogressive) && (dis_info.tvsys <= SECAM))
                    return;
                api_ci_msg_to_app(slot, API_MSG_CI_ICT_ENABLE);
            }
            ICT = TRUE;

            MG = FALSE;
            break;
        case 98:
            ICT = FALSE;
            MG = FALSE;
            break;
        case 100:
            //HDCP: ON
            api_set_hdmi_hdcp_onoff(TRUE);

            MG = FALSE;
            break;
        default:
            MG = FALSE;
            break;
    }
    return TRUE;
}

//Just for test
BOOL api_ict_control()
{
    return ICT;
}

BOOL api_mg_control()
{
    return MG;
}

void api_ci_stop_aes()
{
    aes_ioctl(ciplus_crypto.p_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD , last_handle);
    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_SET_DEC_STATUS, 0);
}

void api_ci_stop_des()
{
    des_ioctl(ciplus_crypto.p_des_dev ,IO_DELETE_CRYPT_STREAM_CMD , last_handle);
    dmx_io_control((struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_SET_DEC_STATUS, 0);
}
#endif

#ifdef S3602F_AES_PATCH
void aes_set_prior()
{
    //1) set de-scrambler has highest priority
    *(UINT32*)(0xb8001010)&=~(0x1<<29);        // Use CFG to define latency
    *(UINT32*)(0xb8001074)&=~(0xff<<8);        // Sub Arbitor Latency to 0
    *(UINT32*)(0xb8001074)|=~(0x80<<8);        // Sub Arbitor Latency to 0x80
    *(UINT32*)(0xb800106c)&=~(0xF<<20);        // Slice out = 0
    *(UINT32*)(0xb800106c)|= (0x8<<20);        // Slice out = 8
    *(UINT32*)(0xb8001038)&=~(0x1<<10);        // Enable priority
    *(UINT32*)(0xb800103c)&=~(0xff<<16);    // Main Arbitor Latency to 0 for master 2

    // 2)set VE has lower priority
    *(UINT32*)(0xb8001010)&=~(0x3<<20);
    *(UINT32*)(0xb8001024)&=~(0xffff);
    *(UINT32*)(0xb8001024)|=(0x8080);
    *(UINT32*)(0xb8001018)&=~(0xff<<16);
    *(UINT32*)(0xb8001018)|=(0x88<<16);        // Slice out = 4

    // 3)set DE has lower priority
    *(UINT32*)(0xb8001010)&=~(0x3<<18);
    *(UINT32*)(0xb8001020)&=~(0xffff<<16);
    *(UINT32*)(0xb8001020)|=(0x8080<<16);
    *(UINT32*)(0xb8001018)&=~(0xff<<8);
    *(UINT32*)(0xb8001018)|=(0x88<<8);        // Slice out = 2

    // 4)set cpu/see low priority    
    *(UINT32*)(0xb8001010)&=~(0x3<<16);
    //*(UINT32*)(0xb8001020)|=(0xffff);
    //*(UINT32*)(0xb8001018)&=~(0xff);

    //disable cpu0,see hi_priority
    *(UINT32*)(0xb8001010)|=(0x3);
}
#endif
#endif

//AP need set answer according current System's state.
//see Table 14.7
void ci_notify_cam_upgrade(UINT8 upgrade_type, UINT16 download_time, UINT8 *answer)
{
    *answer = 0x02;
    return;
/*
    if(upgrade_type == 0x00)    //delayed_upgrade mode
    {
        if(pvr_check_record_active())    //unattended mode -- PVR
            *answer = 0x00;
        else    //user mode
            *answer = 0x02;
    }
    else if(upgrade_type == 0x01)    //immediate_upgrade mode
    {
        if(pvr_check_record_active())    //unattended mode -- PVR
            *answer = 0x01;
        else    //user mode
            *answer = 0x02;
    }
*/
}

UINT32 api_ci_get_match_tp()
{
    return ci_camup_match_tp;
}

static UINT32 ci_get_prog_id(UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id)
{
#ifdef NETWORK_ID_ENABLE
    S_NODE s_node;
    T_NODE t_node;
    P_NODE p_node;
    UINT16 sat_num =0, tp_num, prog_num;
    UINT16 i =0,j =0, k;
    APPL_PRINTF("%s: search : net id: %X, org net id: %X, ts id: %X, service id: %X\n",__FUNCTION__,network_id,org_network_id,ts_id,service_id);
    recreate_sat_view(VIEW_ALL,0);
    sat_num = get_sat_num(VIEW_ALL);

    for(i=0; i<sat_num; i++)
    {
        recreate_sat_view(VIEW_ALL,0);
        get_sat_at(i, VIEW_ALL, &s_node);
        recreate_tp_view(VIEW_SINGLE_SAT ,s_node.sat_id);
        recreate_prog_view(VIEW_SINGLE_SAT|PROG_TVRADIO_MODE,s_node.sat_id);
        tp_num = get_tp_num_sat(s_node.sat_id);

        for(j=0; j < tp_num; j++)
        {
            get_tp_at(s_node.sat_id, j, &t_node);
            APPL_PRINTF("%s: tp - net id: %X, org net id: %X, ts id: %X\n",__FUNCTION__,t_node.net_id,t_node.network_id,t_node.t_s_id);
            if(t_node.network_id == org_network_id
                && t_node.t_s_id == ts_id
                && t_node.net_id == network_id ) //found the tp
            {
                is_tuned_tp_in_db = TRUE;
                APPL_PRINTF("%s: find tp %X\n",__FUNCTION__,t_node.tp_id);
                ci_camup_match_tp = t_node.tp_id;

                //prog_num = get_prog_num_tp(t_node.tp_id);
                for (k=0; /*k<prog_num*/; k++)
                {
                    if (SUCCESS != get_prog_at(k, &p_node))
                        break;
                    APPL_PRINTF("%s: service id %X\n",__FUNCTION__,p_node.prog_number);
                    if (p_node.prog_number == service_id)
                    {
                        APPL_PRINTF("%s: find prog %X\n",__FUNCTION__,p_node.prog_id);
                        return p_node.prog_id;
                    }
                }
            }
        }
    }
#endif
APPL_PRINTF("%s: not find service!\n",__FUNCTION__);
    return 0;
}

extern UINT8 ci_get_current_slot();
extern UINT32 ci_get_cur_program_id(int slot);
static UINT32 ci_get_prog_id_by_ft_type(UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id, UINT16 ft_type)
{
    APPL_PRINTF("%s:--->\n",__FUNCTION__);
    UINT8 slot = 0;
    UINT16 num = 0;
    UINT16 tpcnt = 0;
    UINT16 nim_type = 0;;
    UINT32 prog_id = 0;
    UINT32 i = 0;
    P_NODE node;
    T_NODE t_node;

    MEMSET(&node, 0x0, sizeof (node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    slot = ci_get_current_slot();
    prog_id = ci_get_cur_program_id(slot);
    get_prog_by_id(prog_id , &node);
    nim_type=ft_type;

    if (nim_type ==FRONTEND_TYPE_S)
    {
        APPL_PRINTF("%s: come to S\n",__FUNCTION__);
        recreate_tp_view(VIEW_SINGLE_SAT,node.sat_id);
        tpcnt = get_tp_num_sat(node.sat_id);
        for (i=0; i<tpcnt; i++)
        {
            get_tp_at(node.sat_id, i, &t_node);
            if(t_node.network_id == org_network_id&& t_node.t_s_id == ts_id
            #if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_))
                && t_node.net_id == network_id
            #endif
            )
            {
                ci_tuned_tp_id = t_node.tp_id;
                ci_camup_match_tp = t_node.tp_id;
                APPL_PRINTF("%s: find tp %X\n",__FUNCTION__,t_node.tp_id);
                break;
            }
        }
    }
    else    if (nim_type ==FRONTEND_TYPE_C)
    {
        APPL_PRINTF("%s: come to C\n",__FUNCTION__);
        recreate_tp_view(VIEW_SINGLE_SAT, 1);
        tpcnt = get_tp_num_sat(1);
        for(i = 0; i < tpcnt; i++)
        {
            if(get_tp_by_pos(i, &t_node)==SUCCESS)
            if(t_node.network_id == org_network_id && t_node.t_s_id == ts_id
            #if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_))
                && t_node.net_id == network_id
            #endif
            )
            {
                ci_tuned_tp_id = t_node.tp_id;
                ci_camup_match_tp = t_node.tp_id;
                APPL_PRINTF("%s: find tp %X\n",__FUNCTION__,t_node.tp_id);
                break;
            }
        }
    }
    else    if (nim_type ==FRONTEND_TYPE_T)
    {
        //just process the service in the DB case, if not, CAM upgrade will not success
        APPL_PRINTF("%s: come to T\n",__FUNCTION__);
        recreate_tp_view(VIEW_SINGLE_SAT, 1);
        tpcnt = get_tp_num_sat(1);
        for(i = 0; i < tpcnt; i++)
        {
            if(get_tp_by_pos(i, &t_node)==SUCCESS)
            if(t_node.network_id == org_network_id && t_node.t_s_id == ts_id
            #if(defined( _MHEG5_SUPPORT_) || defined( _MHEG5_V20_ENABLE_))
                && t_node.net_id == network_id
            #endif
            )
            {
                ci_tuned_tp_id = t_node.tp_id;
                ci_camup_match_tp = t_node.tp_id;
                APPL_PRINTF("%s: find tp %X\n",__FUNCTION__,t_node.tp_id);
                break;
            }
        }
    }

    recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE,t_node.tp_id);
    num = get_prog_num(VIEW_SINGLE_TP|PROG_TVRADIO_MODE,t_node.tp_id);
    for (i=0; i<num; i++)
    {
        get_prog_at(i, &node);
        if (node.prog_number == service_id)
        {
            APPL_PRINTF("%s: find prog %X\n",__FUNCTION__,node.prog_id);
            recreate_prog_view(VIEW_ALL|PROG_TV_MODE,0);
            return node.prog_id;
        }
    }
    recreate_prog_view(VIEW_ALL|PROG_TV_MODE,0);
    APPL_PRINTF("%s: not find service!\n",__FUNCTION__);
    return 0;
}




/*Old version for tune service process*/
/*This version Search DB just for FRONTEND_TYPE_S, but it seems work badly */
UINT32 api_ci_get_prog_id(UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id)
{
    APPL_PRINTF("%s is not work anymore. we keep it just for compatibility consideration.\n", __FUNCTION__);
    APPL_PRINTF("You can use api_ci_get_prog_id_by_ft_type() for a replacement.\n");
    return ci_get_prog_id(network_id, org_network_id, ts_id, service_id);
}
/*New version for tune service process*/
/*This version Search DB according to ft_type, it seems work well */
UINT32 api_ci_get_prog_id_by_ft_type(UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id, UINT16 ft_type)
{
    return ci_get_prog_id_by_ft_type(network_id, org_network_id, ts_id, service_id, ft_type);
}

UINT32 api_ci_get_tuned_prog (void)
{
    return ciplus_camup_prog_id;
}
BOOL api_ci_tuned_in_db()
{
    APPL_PRINTF("%s is not work anymore. we keep it just for compatibility consideration.\n", __FUNCTION__);
    APPL_PRINTF("You can use api_ci_tuned_in_db_by_ft_type() for a replacement.\n");
    return is_tuned_tp_in_db;
}
UINT32 api_ci_tuned_in_db_by_ft_type()
{
    return ci_tuned_tp_id;
}

void api_ci_get_hc_id(UINT8 *slot, UINT16 *network_id, UINT16 *org_network_id, UINT16 *ts_id, UINT16 *service_id)
{
    if (slot != NULL)
        *slot = ci_tuned_id.slot;
    if (org_network_id != NULL)
        *org_network_id = ci_tuned_id.org_network_id;
    if (network_id != NULL)
        *network_id = ci_tuned_id.network_id;
    if (service_id != NULL)
        *service_id = ci_tuned_id.service_id;
    if (ts_id != NULL)
        *ts_id = ci_tuned_id.ts_id;
}

/*For CI+ CAM UpGrade Function 2012/03/16*/
INT32 ci_tune_service_by_ft_type(UINT8 slot, UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id)
{
    APPL_PRINTF("%s ->\n", __FUNCTION__);
    /* save parameters */
    ci_tuned_id.slot = slot;
    ci_tuned_id.org_network_id = org_network_id;
    ci_tuned_id.network_id = network_id;
    ci_tuned_id.service_id = service_id;
    ci_tuned_id.ts_id = ts_id;
    /* Tell app tune service */
    api_ci_msg_to_app(slot, API_MSG_CI_TUNE_SERVICE);
    APPL_PRINTF("%s -> send msg[API_MSG_CI_TUNE_SERVICE] to app\n",__FUNCTION__);
    return SUCCESS;
}


INT32 ci_tune_service(UINT8 slot, UINT16 network_id, UINT16 org_network_id, UINT16 ts_id, UINT16 service_id)
{
    UINT32 prog_id = 0;

    APPL_PRINTF("%s ->\n",__FUNCTION__);
    is_tuned_tp_in_db = FALSE;
    prog_id = ci_get_prog_id(network_id, org_network_id, ts_id, service_id);
    ci_tuned_id.slot = slot;
    ci_tuned_id.org_network_id = org_network_id;
    ci_tuned_id.network_id = network_id;
    ci_tuned_id.service_id = service_id;
    ci_tuned_id.ts_id = ts_id;

    if (0 != prog_id)
    {
#ifdef NEW_DEMO_FRAME
        ciplus_camup_prog_id = prog_id;
        api_ci_msg_to_app(slot, API_MSG_CI_TUNE_SERVICE);
#else
        uich_chg_play_prog(0, prog_id);
#endif
        APPL_PRINTF("%s -> ok\n",__FUNCTION__);
        return SUCCESS;
    }
    else
    {
        ciplus_camup_prog_id = 0;
        api_ci_msg_to_app(slot, API_MSG_CI_TUNE_SERVICE);

        APPL_PRINTF("%s -> can't find needed service, process in app\n",__FUNCTION__);
        return SUCCESS;
    }
}


