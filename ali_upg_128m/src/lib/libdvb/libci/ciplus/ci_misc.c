/************************************************************************************
*    Ali Corp. All Rights Reserved. 2008 Copyright (C)
*
*    File:    ci_misc.c
*
*    Description:    This file contains all external functions used by the CI
*                  stack & APP etc. And these functions are associated with
*                   ALI's platforms and SIE.
*    History:
*                Date                Athor           Version            Reason
*        ==========    ======   =========    =========    =================
*    1.    2008.01.25         Steven          Ver 0.1         Create file.
*    2.    2008.03.21         Steven          Ver 0.2         Develop for serial & paralle mode
*    3.    2008.06.23         Steven          Ver 0.3         Add a ci_control struct to manage 2 CI slots.
*    4.    2009.10.20         Steven                        fix Crypto Works Cam switch audio issue.
*    4.    2009.10.22         Steven                        Add TTX/Subt pid
*
*************************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/decv.h>

#include <hld/cic/cic.h>
#include <api/libci/ci_plus.h>

#include "ci_stack.h"

#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_pub.h>
#endif
#include <api/libsi/sie_monitor.h>
#include <api/libtsi/sie.h>

#ifdef CI_SUPPORT
/* example:

1. CI_SERIAL_MODE
    api_ci_set_mode(CI_SERIAL_MODE);
    api_ci_set_dev(nim_dev_1, dmx_dev_1, 0);
    api_set_nim_ts_type(3, 0x03);
    ...
    api_set_nim_ts_type(2, 0x03);

    api_ci_set_dev(nim_dev_2, dmx_dev_2, 0);
    api_set_nim_ts_type(3, 0x03);

2. CI_PARALLEL_MODE
    api_ci_set_mode(CI_PARALLEL_MODE);
    api_ci_set_dev(nim_dev_1, dmx_dev_1, 0);
    api_ci_set_dev(nim_dev_2, dmx_dev_2, 1);
    api_set_nim_ts_type(3, 0x03);
    ...
    api_set_nim_ts_type(2, 0x03);

    api_ci_set_dev(nim_dev_3, dmx_dev_3, 0);
    api_ci_set_dev(nim_dev_4, dmx_dev_4, 1);
    api_set_nim_ts_type(3, 0x03);

*/

// CI Control Struct
struct ci_slot_control ci_control;

// PVR will check if PIDs are OK to record
static BOOL is_dsc_pid_lst_ok[CI_MAX_SLOT_NUM] = {0};
/*
 *  @We add this structure to record some addtional PID
 *  @which may be needed by PVR or some other module
 */
#define MAX_ADD_PID_NUM 32

struct addtional_pid_lst
{
    UINT16 pid_lst[MAX_ADD_PID_NUM];
    UINT8  pid_count;
};

static struct addtional_pid_lst add_pid_lst;

#ifndef DUAL_ENABLE
extern UINT32 g_vdec_display_index;
#endif

#ifndef NEW_DEMO_FRAME
extern INT32 cc_get_cmd_cnt();
#endif

//////////////////////////////////////////////////////////////////
#define CI_APP_MUTEX_LOCK()    \
    do                            \
    {                            \
        if(ci_control.ci_app_mutex == OSAL_INVALID_ID)\
        {                                        \
            ci_control.ci_app_mutex = osal_mutex_create();\
            ASSERT(ci_control.ci_app_mutex != OSAL_INVALID_ID);\
        } \
        osal_mutex_lock(ci_control.ci_app_mutex, OSAL_WAIT_FOREVER_TIME);\
    }while(0)

#define CI_APP_MUTEX_UNLOCK()    do{osal_mutex_unlock(ci_control.ci_app_mutex);}while(0)

extern void psi_pmt_get_ecm_pid(UINT8 *section, INT32 length, UINT16 param);

UINT32 ci_current_prog_id = CI_NO_PROG_PASS;
UINT32 ci_prog_id_bak = CI_NO_PROG_PASS;
UINT8  ci_current_slot = 0xff;

UINT8 ci_get_current_slot()
{
    return ci_current_slot;
}
void ci_set_current_slot(UINT8 slot)
{
    ci_current_slot = slot;
}

/* Reset Local CA_PMT */
void ci_reset_local_ca_pmt(void)
{
    int i;

    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        MEMSET(&ci_control.slot_info[i].ca_pmt, 0, sizeof(struct ci_ca_pmt));
    }
}

/* Set ca_pmt_cmd_id and ca_pmt_list_management */
void ci_set_pmt_cmd(UINT32 prog_id, BOOL resend, BOOL update, int slot)
{
    // Send New CA_PMT (e.g. When Receive New PMT or Switch Audio PID)
    if (update)
    {
        ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_OK_DESCRAMBLING;
        ci_control.slot_info[slot].ca_pmt.ca_pmt_list_management = CI_CPLM_UPDATE;
        ci_control.slot_info[slot].ca_pmt.force_resend = FALSE;
    }
    else
    {
        // Resend CA_PMT (e.g. When Video Hold, don't Descramble)
        if (resend)
        {
            ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_OK_DESCRAMBLING;
            ci_control.slot_info[slot].ca_pmt.force_resend = TRUE;
        }
        // First Send CA_PMT for Query whether CAM Card can Descramble this Program
        else
        {
#if (defined CI_PLUS_SUPPORT) || (defined HD_PLUS_SUPPORT)
            ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_OK_DESCRAMBLING;
#else
            ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_QUERY;
#endif
            ci_control.slot_info[slot].ca_pmt.force_resend = FALSE;
        }

        ci_control.slot_info[slot].ca_pmt.ca_pmt_list_management = CI_CPLM_ONLY;
    }
}

UINT8 slot_status = 0;    /* Passing CAM Card Status */
UINT8 nim_ts_type[] = {2, 2}; /* 2 slot at most */

/* Set TS Stream of Slot Pass or Bypass CAM Card */
/* type: in, 2 - Bypass CAM; 3 - Pass CAM */
void ci_set_nim_ts_type(struct nim_device* nim_dev, struct dmx_device* dmx_dev,
                    UINT8 type, UINT8 slot)
{
    int i;
#if (CI_MAX_SLOT_NUM <= 2)

#else
#error "ci max slot num > 2"
#endif

    BOOL ts_disable = FALSE;
    /* Send CA_PMT Even Clean Program */
    struct cic_io_command_signal signal_param;
    struct cic_device *cic_dev = (struct cic_device*)dev_get_by_id(HLD_DEV_TYPE_CIC, 0);

    CI_APP_MUTEX_LOCK();

    /* Process Switch TS */
    if (type == 3)
    {
        /* Encrypt Program: Switch TS Format First, then Pass CAM */
        if(nim_ts_type[slot] != 3)
        {
#if (SYS_CHIP_MODULE != ALI_S3602)
            // Disable TS Input
            dmx_io_control(dmx_dev, DMX_ENABLE_TS_INPUT, FALSE);
            ts_disable = TRUE;
            // Set NIM TS Mode to 3
            if (nim_dev != NULL)
                nim_io_control(nim_dev, NIM_DRIVER_SET_TS_MODE, 3);
            // Reset DMX
            if (dmx_dev != NULL)
                dmx_io_control(dmx_dev, IO_DMX_RESET, 0);
#endif
            if (ci_control.mode == CI_SERIAL_MODE)
            {
                for (i = 0; i < CI_MAX_SLOT_NUM; i++)
                    nim_ts_type[i] = 3;
            }
            else
                nim_ts_type[slot] = 3;
            APPL_PRINTF("%s: -> TS format %d\n", __FUNCTION__, nim_ts_type[slot]);
        }
        signal_param.signal = CIC_EMSTREAM;
        signal_param.status = 1;    /* Pass CAM */

        if (((slot_status & (1 << slot)) == 0)
            && CAM_STACK_ATTACH == api_ci_check_status(slot))
        {
#if (SYS_CHIP_MODULE != ALI_S3602)
            // Disable TS Input
            if (!ts_disable)
            {
                dmx_io_control(dmx_dev, DMX_ENABLE_TS_INPUT, FALSE);
                ts_disable = TRUE;
            }
#endif
            signal_param.slot = slot;
            // Set TS Pass CAM through CI Controller
            cic_io_control(cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
            APPL_PRINTF("%s: pass slot %d\n", __FUNCTION__, slot);
            slot_status |= (1<<slot);    /* Set Slot Status */
        }
    }
    /* Support Clear Program Bypass CAM */
    else if (type == 2)
    {
        /* Uncrypt Program: Bypass CAM First(in stack), then Switch TS Format */
        if(nim_ts_type[slot] != 2
            && (ci_control.mode == CI_PARALLEL_MODE
                || (ci_control.mode == CI_SERIAL_MODE && (slot_status & (~(1 << slot))) == 0)
                )
            )
        {
#if (SYS_CHIP_MODULE != ALI_S3602)
            // Disable TS Input
            dmx_io_control(dmx_dev, DMX_ENABLE_TS_INPUT, FALSE);
            ts_disable = TRUE;
            // Set NIM TS Mode to 2
            if (nim_dev != NULL)
                nim_io_control(nim_dev, NIM_DRIVER_SET_TS_MODE, 2);
            // Reset DMX
            if (dmx_dev != NULL)
                dmx_io_control(dmx_dev, IO_DMX_RESET, 0);
#endif
            if (ci_control.mode == CI_SERIAL_MODE)
            {
                for (i = 0; i < CI_MAX_SLOT_NUM; i++)
                    nim_ts_type[i] = 2;
            }
            else
                nim_ts_type[slot] = 2;
            APPL_PRINTF("%s: -> TS format %d\n", __FUNCTION__, nim_ts_type[slot]);
        }
        signal_param.signal = CIC_EMSTREAM;
        signal_param.status = 0;    /* Bypass CAM */

        if ((slot_status & (1 << slot)) != 0)
        {
#if (SYS_CHIP_MODULE != ALI_S3602)
            // Disable TS Input
            if (!ts_disable)
            {
                dmx_io_control(dmx_dev, DMX_ENABLE_TS_INPUT, FALSE);
                ts_disable = TRUE;
            }
#endif
            signal_param.slot = slot;
            // Set TS Bypass CAM through CI Controller
            cic_io_control(cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
            APPL_PRINTF("%s: bypass slot %d\n", __FUNCTION__, slot);
        }

        slot_status &= (~(1 << slot));    /* Set Slot Status */
    }
    else
    {
        APPL_PRINTF("ci_set_nim_ts_type: unknown ts type %d\n", type);
    }

#if (SYS_CHIP_MODULE != ALI_S3602)
    // Enable TS Input
    if (ts_disable)
        dmx_io_control(dmx_dev, DMX_ENABLE_TS_INPUT, TRUE);
#endif

    CI_APP_MUTEX_UNLOCK();
}

/*
 *  We may have some case: we need to bypass or pass the CAM
 *  directly, but we don't have CI device constructed, so we
 *  can't use api_set_nim_ts_type. This API could be a substitution
 *  of API api_set_nim_ts_type
 */
void api_set_ts_pass_cam(UINT8 pass, UINT8 slot_mask)
{
    int i;
    struct cic_device *cic_dev = (struct cic_device*)dev_get_by_id(HLD_DEV_TYPE_CIC, 0);
    struct cic_io_command_signal signal_param;

    for (i=0; i<CI_MAX_SLOT_NUM; i++)
    {
        if ((1 << i) & slot_mask)
        {
            signal_param.slot = i;
            signal_param.signal = CIC_EMSTREAM;
            signal_param.status = 0;    /* Bypass CAM */
            slot_status &= (~(1 << i));
            nim_ts_type[i] = 2;

            if (pass)
            {
                if (CAM_STACK_ATTACH == api_ci_check_status(i))
                {
                    slot_status |= (1 << i);
                    nim_ts_type[i] = 3;
                    signal_param.status = 1;    /* Pass CAM */
                }
                else
                    return;
            }

            cic_io_control(cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        }
    }
}

/* API for Set TS Stream Pass or Bypass CAM Card */
/* type: in, 2 - Bypass CAM; 3 - Pass CAM */
void api_set_nim_ts_type(UINT8 type, UINT8 slot_mask)
{
    int i;

    for (i=0; i<CI_MAX_SLOT_NUM; i++)
    {
        if ((slot_mask&(1<<i)) != 0
            && ci_control.slot_info[i].dmx_dev != NULL)
        {
            ci_set_nim_ts_type(ci_control.slot_info[i].nim_dev,
                            ci_control.slot_info[i].dmx_dev,
                            type, i);
        }
    }
}

/* Internal API for CI Module */
/* CI Control Function Initialization */
void ci_control_init()
{
    MEMSET(&ci_control, 0, sizeof(ci_control));
    ci_control.ci_app_mutex = OSAL_INVALID_ID;

    ci_control.state = osal_flag_create(0);
}

/* API for Set CI Slot Mode: Serial or Parallel */
/* mode: in, see "enum ci_slot_mode" */
void api_ci_set_mode(UINT8 mode)
{
    INT i;

    if (mode != ci_control.mode)
    {
        // When mode != last mode, should Set Bypass All CAM Cards First!
        api_set_nim_ts_type(2, 0xFF);

        CI_APP_MUTEX_LOCK();

        // When mode != last mode, shoulde Clear Device and Set New CI Mode
        for (i = 0; i < CI_MAX_SLOT_NUM; i++)
        {
            ci_control.slot_info[i].nim_dev = NULL;
            ci_control.slot_info[i].dmx_dev = NULL;
        }
        ci_control.mode = mode;

        CI_APP_MUTEX_UNLOCK();

        APPL_PRINTF("%s: to mode - %d\n", __FUNCTION__, mode);
    }
    else
    {
        APPL_PRINTF("%s: same mode - %d\n", __FUNCTION__, mode);
    }
}

/* API for Set Device List from some Slot */
/* nim_dev: in, for PVR, it may be NULL. */
/* dmx_dev: in, should not be NULL */
/* slot: in, No Meaning when ci_control.mode is CI_SERIAL_MODE. */
void api_ci_set_dev(void *nim_dev, void *dmx_dev, UINT8 slot)
{
    ASSERT(dmx_dev != NULL);
    ASSERT(slot < CI_MAX_SLOT_NUM);

    APPL_PRINTF("%s: nim %X, dmx %X -[%d]\n", __FUNCTION__, nim_dev, dmx_dev, slot);

    CI_APP_MUTEX_LOCK();
    if (ci_control.mode == CI_SERIAL_MODE)
    {
        for (slot = 0; slot < CI_MAX_SLOT_NUM; slot++)
        {
            ci_control.slot_info[slot].nim_dev = (struct nim_device*)nim_dev;
            ci_control.slot_info[slot].dmx_dev = (struct dmx_device*)dmx_dev;
        }
    }
    else     if (ci_control.mode == CI_PARALLEL_MODE) /* Parallel */
    {
        ci_control.slot_info[slot].nim_dev = (struct nim_device*)nim_dev;
        ci_control.slot_info[slot].dmx_dev = (struct dmx_device*)dmx_dev;
    }
    CI_APP_MUTEX_UNLOCK();
}

/* Internal API for CI Module */
/* Get whether NIM Locked or not */
UINT8 ci_cas_nim_get_lock(UINT8 slot)
{
    UINT8 lock = 1;
    INT32 ret;

    ASSERT(slot < CI_MAX_SLOT_NUM);

    // For PVR Playback
    if (RET_SUCCESS == dmx_io_control(ci_control.slot_info[slot].dmx_dev, DMX_IS_TSG_PLAYBACK, 0))
    {
        //TS Come From TS Generator
        return 1;
    }
    // For Normal Play, TS Come From NIM
    else if (ci_control.slot_info[slot].nim_dev != NULL)
    {
        if ((ret = nim_get_lock(ci_control.slot_info[slot].nim_dev, &lock)) != SUCCESS)
        {
            APPL_PRINTF("%s: operation error! %d\n", __FUNCTION__,ret);
            return 0;
        }
    }
    return lock;
}

/* Internal API for CI Module */
/* Check whether Audio and Video is Scrambled or not */
BOOL ci_is_av_scrambled(UINT8 slot)
{
    RET_CODE dmx_state = RET_FAILURE;
    struct io_param param;
    struct io_param_ex param_ex;
    UINT16 PID[2];
    UINT8 scramble_type=0;

    ASSERT(slot < CI_MAX_SLOT_NUM);

    if (ci_control.slot_info[slot].dmx_dev != NULL)
    {
        PID[0] = ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_VIDEO_INDEX];
        PID[1] = ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_AUDIO_INDEX];
        //MEMCPY(&PID[1], &ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_AUDIO_INDEX],
        //       ci_control.slot_info[slot].audio_count * sizeof(UINT16));
        if (ALI_M3329E==sys_ic_get_chip_id())
        {
            MEMSET(&param, 0, sizeof(param));
            param.io_buff_in = (UINT8 *)PID;
            param.io_buff_out= (UINT8 *)&scramble_type;
            dmx_state = dmx_io_control(ci_control.slot_info[slot].dmx_dev, IS_AV_SCRAMBLED_EXT, (UINT32)&param);
        }
        else if(ALI_S3602F == sys_ic_get_chip_id())
        {
            MEMSET(&param_ex, 0, sizeof(param_ex));
            param_ex.io_buff_in = (UINT8 *)PID;
            param_ex.io_buff_out= (UINT8 *)&scramble_type;
            dmx_state = dmx_io_control(ci_control.slot_info[slot].dmx_dev, IS_AV_SCRAMBLED_EXT, (UINT32)&param_ex);
        }
        else
        {
            //for 3602 do not support IS_AV_SCRAMBLED_EXT
            dmx_state = dmx_io_control(ci_control.slot_info[slot].dmx_dev, IS_AV_SCRAMBLED, (UINT32)&scramble_type);
        }
        //fix issue: for AC3 audio is AUD_STR_INVALID
//        if((dmx_state == SUCCESS ) && ((scramble_type & (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID)) != 0))
        if((dmx_state == SUCCESS ) && ((scramble_type & (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL|VDE_STR_INVALID)) != 0))
        {
            APPL_PRINTF("%s: scramble_type %X\n", __FUNCTION__,scramble_type);
            return TRUE;
        }
    }

    return FALSE;
}

/* Internal API for CI Module */
/* Check whether Video Decoder is Running or not */
BOOL ci_is_vdec_running(int slot)
{
    UINT32 i;
    struct vdec_device *vdec;
    struct vdec_status_info cur_status;
    BOOL ret = FALSE;
    if (ALI_S3602==sys_ic_get_chip_id() || ALI_S3602F==sys_ic_get_chip_id())
    {
        //which decv??
        vdec = (struct vdec_device *)get_selected_decoder();
        if (vdec == NULL)
        {
            APPL_PRINTF("%s: get vdec fail!\n",__FUNCTION__);
            ret = FALSE;
        }
        else {
            vdec_io_control(vdec, VDEC_IO_GET_STATUS, (UINT32)&cur_status);
            if((cur_status.u_first_pic_showed)  && (cur_status.u_cur_status == VDEC_DECODING)) {
                ret = TRUE;
            }
            else {
                if (cur_status.u_first_pic_showed) {
                    APPL_PRINTF("%s: FirstPicShowed, but VDEC not running!\n",__FUNCTION__);
                }
                else if ((cur_status.u_cur_status == VDEC_DECODING)) {
                    APPL_PRINTF("%s: vdec is decoding, but no picture show!\n",__FUNCTION__);
                }
                else {
                        APPL_PRINTF("%s: vdec is not decoding, no picture show!\n",__FUNCTION__);
                }
                ret = FALSE;
            }
        }
    }
    else
    {
#ifndef DUAL_ENABLE    //dual cpu
        // Which vdec for multi vdec???
        ret =  (g_vdec_display_index > 0);
#endif
    }
    return ret;
}

/* Copy Descriptors from the Descriptor Loop in PMT */
/* tag: in, Tag of Descriptor to be copied */
/* return: Copied Size */
static INT16 copy_descriptor_loop(UINT8 tag, UINT8 *dst, INT16 dst_len, UINT8 *src, INT16 src_len)
{
    INT16 ret_len = 0;
    UINT8 des_tag;
    UINT8 des_len;

    while (src_len > 0)
    {
        des_tag = src[0];
        des_len = src[1];

        if (des_tag == tag && dst_len >= (des_len + 2))
        {
            MEMCPY(dst, src, (des_len + 2));
            dst += (des_len + 2);
            dst_len -= (des_len + 2);
            ret_len += (des_len + 2);
        }

        src += (des_len + 2);
        src_len -= (des_len + 2);
    }

    APPL_PRINTF("%s: copied descriptor data len %d\n", __FUNCTION__, ret_len);
    return ret_len;
}

/* Create CA_PMT: Copy some PMT Information to CA_PMT */
static INT32 ci_pmt_create(UINT8 *pmt_buff, INT32 buff_len,
                        UINT8 *ca_pmt_buff, INT32 *pca_pmt_len)
{
       UINT16    sec_len;
    UINT16     prog_info_len;
    UINT16     ca_pmt_prog_info_len;
    UINT8      *es_loop_data;
    INT16      es_loop_len;
    UINT8    *ca_pmt_es_loop_data;

    UINT8     stream_type;
    UINT16     elementary_pid;
    UINT16     es_info_length;
    INT16     ca_pmt_es_info_len;
    INT32     ca_pmt_buff_len = *pca_pmt_len;

    UINT8   i;
    UINT8   tag_lst[] = {CA_DES_TAG, TELETEXT_DES_TAG, SUBTITLE_DES_TAG};
    UINT8   tag_found = 0x00;

    APPL_PRINTF(">>> %s\n", __FUNCTION__);

    *pca_pmt_len = 0;    /* Init to 0 */

    ca_pmt_buff[0] = CI_CPLM_ONLY;    /* ca_pmt_list_management */
    ca_pmt_buff[1] = pmt_buff[3];    /* program_number */
    ca_pmt_buff[2] = pmt_buff[4];

    ca_pmt_buff[3] = pmt_buff[5];    /* version_number, current_next_indicator */

    sec_len = ((pmt_buff[1] & 0x0f) << 8) | pmt_buff[2];        /* pmt length */
    if (buff_len < sec_len + 3)
    {
        APPL_PRINTF("%s: buff len %d < sec len %d!!!\n", __FUNCTION__, buff_len, sec_len+3);
        APPL_PRINTF("<<< %s\n", __FUNCTION__);
        return !SUCCESS;
    }
    prog_info_len = ((pmt_buff[10] & 0x03) << 8) | pmt_buff[11];        /* PMT program_info_length */

    if (buff_len < prog_info_len + 12)
    {
        APPL_PRINTF("%s: buff len %d < 12 + program info len %d!!!\n", __FUNCTION__, buff_len, prog_info_len);
        APPL_PRINTF("<<< %s\n", __FUNCTION__);
        return !SUCCESS;
    }

    // Program Level
    // For Get ca_pmt_prog_info_len
    ca_pmt_prog_info_len = 0;
    if (prog_info_len > 0)
    {
        // Copy CA_Descriptors
        ca_pmt_prog_info_len = copy_descriptor_loop(CA_DES_TAG, ca_pmt_buff + 7, ca_pmt_buff_len - 7,
                                    pmt_buff + 12, prog_info_len);
        if (ca_pmt_prog_info_len > 0)
        {
            ca_pmt_prog_info_len ++;    //1 byte of ca_pmt_cmd_id more

            ca_pmt_buff[6] = CI_CPCI_QUERY;    /* ca_pmt_cmd_id */
        }
    }
    ca_pmt_buff[4] = 0xF0 | (ca_pmt_prog_info_len >> 8);        /* CA_PMT program_info_length */
    ca_pmt_buff[5] = ca_pmt_prog_info_len & 0xFF;
    APPL_PRINTF("%s: ca_pmt prog info len %d\n", __FUNCTION__, ca_pmt_prog_info_len);

    // ES Level Loop
    es_loop_data = pmt_buff + prog_info_len + 12;
    es_loop_len = buff_len - prog_info_len - 12 - 4/*CRC*/;
    ca_pmt_es_loop_data = ca_pmt_buff + ca_pmt_prog_info_len + 6;
    ca_pmt_buff_len -= (ca_pmt_prog_info_len + 6);
    *pca_pmt_len = (ca_pmt_prog_info_len + 6);

    while (es_loop_len >= 5)
    {
        // PMT
        stream_type = es_loop_data[0];
        elementary_pid = ((es_loop_data[1] & 0x1F) << 8) | es_loop_data[2];
        es_info_length = ((es_loop_data[3] & 0x0F) << 8) | es_loop_data[4];

        // For Getting ca_pmt_es_info_len
        ca_pmt_es_info_len = 0;

        // Copy CA_Descriptors
        if (es_info_length > 0)
        {
            i = 0;
            tag_found = 0x00;

            while ((0 == ca_pmt_es_info_len) && (i < 3))
            {
                ca_pmt_es_info_len = copy_descriptor_loop(tag_lst[i],
                                        ca_pmt_es_loop_data + 6, ca_pmt_buff_len - 6,
                                        es_loop_data + 5, es_info_length);
                i++;
            }

            if (0 != ca_pmt_es_info_len)
                tag_found = tag_lst[i - 1];
        }
        else
            ca_pmt_es_info_len = 0;


        /* If we have any addtional PID need to be processed, we add it */
        if ((TELETEXT_DES_TAG == tag_found) || (SUBTITLE_DES_TAG == tag_found))
        {
            if (add_pid_lst.pid_count < MAX_ADD_PID_NUM)
                add_pid_lst.pid_lst[add_pid_lst.pid_count++] = elementary_pid;
        }

        // CA_PMT
        ca_pmt_es_loop_data[0] = stream_type;
        ca_pmt_es_loop_data[1] = 0xE0 | ((elementary_pid >> 8) & 0x1F);
        ca_pmt_es_loop_data[2] = elementary_pid & 0xFF;

        if (ca_pmt_es_info_len > 0)
        {
            ca_pmt_es_info_len ++;    //1 byte of ca_pmt_cmd_id more

            ca_pmt_es_loop_data[5] = CI_CPCI_QUERY;    /* ca_pmt_cmd_id */
        }

        ca_pmt_es_loop_data[3] = 0xF0 | (ca_pmt_es_info_len >> 8);    /* ES_info_length */
        ca_pmt_es_loop_data[4] = ca_pmt_es_info_len & 0xFF;

        // CA_PMT Next Loop
        ca_pmt_es_loop_data += (ca_pmt_es_info_len + 5);
        ca_pmt_buff_len -= (ca_pmt_es_info_len + 5);
        *pca_pmt_len += (ca_pmt_es_info_len + 5);

        APPL_PRINTF("%s: ca_pmt es info len %d\n",__FUNCTION__,ca_pmt_es_info_len);

        // PMT
        es_loop_len -= (es_info_length + 5);
        es_loop_data += (es_info_length + 5);
    } // end of while

    APPL_PRINTF("<<< %s\n", __FUNCTION__);
    return SUCCESS;
}

/* Send CA_PMT to CAM Cards which are connected to Slots */
static void ci_send_ca_pmt(UINT8 slot_mask)
{
    INT i;

    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        if ((slot_mask & (1 << i)) != 0)
        {
            api_ci_send_pmt(i, (UINT32)(&ci_control.slot_info[i].ca_pmt));
        }
    }
}

/* PMT Callback Function Registered to sie_monitor */
/* Process after Receive PMT: Create CA_PMT through PMT and Send it to CI Slots to Descramble Program */
//1. pmt -> ca_pmt
//2. send ca_pmt to ci slots to descramble program.
static INT32 ci_ca_pmt_cb(UINT8 *section, INT32 length, UINT32 param)
{
    UINT32 sec_len, crc;
    int slot;
    UINT8 slot_mask = 0;
    BOOL crc_update = FALSE;
//#ifdef NEW_DEMO_FRAME
    struct sim_cb_param *simcb = (struct sim_cb_param*)param;
//#endif

    UINT8 *pid_num;
    UINT8 i_pid;
    /* Check if PMT CRC is Different or not */
    sec_len = ((section[1] & 0x0f) << 8) | section[2];
    crc = section[sec_len - 1] | section[sec_len] << 8 | section[sec_len + 1] << 16 | section[sec_len + 2] << 24;

    for (slot = 0; slot < CI_MAX_SLOT_NUM; slot++)
    {
#ifdef NEW_DEMO_FRAME
        if (simcb->sim_id == ci_control.slot_info[slot].m_id)
#endif
        {
//#ifdef NEW_DEMO_FRAME
            if((NULL != (ci_control.slot_info[slot].dmx_dev)
               && (simcb->dmx_idx !=  ((ci_control.slot_info[slot].dmx_dev->type) & HLD_DEV_ID_MASK))))
                continue;
//#endif

            if(crc == ci_control.slot_info[slot].ca_pmt.pmt_crc
                || ci_control.slot_info[slot].ca_pmt.program_id == 0)
            {
                continue;
            }

            crc_update = TRUE;

            // Parse PMT Data First!
            APPL_PRINTF("ca_pmt_cb: create ca_pmt obj ....\n");

            /* Create New CA_PMT */
            CI_APP_MUTEX_LOCK();
            ci_control.slot_info[slot].ca_pmt.length = CA_PMT_BUF_LEN;

            add_pid_lst.pid_count = 0;

            ci_pmt_create(section, length, ci_control.slot_info[slot].ca_pmt.buffer,
                         (INT32*)&ci_control.slot_info[slot].ca_pmt.length);

            while (add_pid_lst.pid_count > 0) {
                for (i_pid=0; i_pid<ci_control.slot_info[slot].ca_pmt.es_pid.pid_num; i_pid++) {
                    if (ci_control.slot_info[slot].ca_pmt.es_pid.pid[i_pid] == add_pid_lst.pid_lst[add_pid_lst.pid_count-1])
                        break;
                }
                if (i_pid == ci_control.slot_info[slot].ca_pmt.es_pid.pid_num) {
                    ci_control.slot_info[slot].ca_pmt.es_pid.pid[i_pid] = add_pid_lst.pid_lst[add_pid_lst.pid_count-1];
                    ci_control.slot_info[slot].ca_pmt.es_pid.pid_num++;
                }
                add_pid_lst.pid_count--;
            }

            if (ci_control.slot_info[slot].ca_pmt.pmt_crc == 0)
            {
                // Create CA_PMT First Time
                ci_set_pmt_cmd(ci_control.slot_info[slot].ca_pmt.program_id, FALSE, FALSE, slot);
            }
            else
            {
                // Create CA_PMT Update
                ci_set_pmt_cmd(ci_control.slot_info[slot].ca_pmt.program_id, FALSE, TRUE, slot);
            }

            ci_control.slot_info[slot].ca_pmt.pmt_crc = crc;

            is_dsc_pid_lst_ok[slot] = TRUE;

            slot_mask |= (1 << slot);

            CI_APP_MUTEX_UNLOCK();
        }
    }

    // If CA_PMT Update, then Resend CA_PMT
    if (crc_update)
    {
        ci_send_ca_pmt(slot_mask);

        APPL_PRINTF("send ca_pmt obj ....\n");
    }
    return SUCCESS;
}

/* API for other Modules */
/* When Video Hold, DM (Dec Monitor) Send CA_PMT again Directly */
void cc_send_ca_pmt(UINT32 prog_id)
{
    int i;
    UINT8 slot_mask = 0;

    APPL_PRINTF(">>> %s, program %X\n",__FUNCTION__, prog_id);

    // Set CA_PMT Cmd and Select Slot
    CI_APP_MUTEX_LOCK();
    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        if (ci_control.slot_info[i].ca_pmt.pmt_crc != 0
            && ci_control.slot_info[i].ca_pmt.program_id == prog_id)
        {
            ci_set_pmt_cmd(ci_control.slot_info[i].ca_pmt.program_id, TRUE, FALSE, i);
            slot_mask |= (1 << i);
        }
    }
    CI_APP_MUTEX_UNLOCK();

    // Send CA_PMT
    ci_send_ca_pmt(slot_mask);
}

/* API for Send CA_PMT First Time to some Slot */
void api_send_ca_pmt(UINT8 slot)
{
    APPL_PRINTF("%s: slot %d\n",__FUNCTION__,slot);

    ASSERT(slot < CI_MAX_SLOT_NUM);
    if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0
        && ci_control.slot_info[slot].ca_pmt.program_id != 0)
    {
        CI_APP_MUTEX_LOCK();
        //ci_set_pmt_cmd(0, FALSE, FALSE, slot);
#if (defined CI_PLUS_SUPPORT) || (defined HD_PLUS_SUPPORT)
        ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_OK_DESCRAMBLING;
#else
        ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_QUERY;
#endif
        //some cam need force to send CI_CPCI_QUERY of ca_pmt.
        ci_control.slot_info[slot].ca_pmt.force_resend = TRUE;
        ci_control.slot_info[slot].ca_pmt.ca_pmt_list_management = CI_CPLM_ONLY;
        CI_APP_MUTEX_UNLOCK();

        api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
    }
}

/* API Extension for Send CA_PMT First Time to some Slot */
void api_send_ca_pmt_ext(UINT8 slot, enum ci_ca_pmt_cmd_id cmd_id)
{
    APPL_PRINTF("%s: slot %d\n",__FUNCTION__,slot);

    ASSERT(slot < CI_MAX_SLOT_NUM);
    if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0)
    {
        CI_APP_MUTEX_LOCK();
        ci_control.slot_info[slot].ca_pmt.cmd_id = cmd_id;
        ci_control.slot_info[slot].ca_pmt.force_resend = TRUE;
        ci_control.slot_info[slot].ca_pmt.ca_pmt_list_management = CI_CPLM_ONLY;
        CI_APP_MUTEX_UNLOCK();

        api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
    }
}

/* API for Send CA_PMT First Time to some Slot */
BOOL api_send_prog_ca_pmt(UINT8 slot, UINT32 prog_id)
{
    APPL_PRINTF("%s: slot %d\n",__FUNCTION__,slot);

    ASSERT(slot < CI_MAX_SLOT_NUM);

//fix issue caused by "reinsert card while timeshift"
#if ((!defined(CC_USE_TSG_PLAYER))&&(!defined(CI_SLOT_DYNAMIC_DETECT)))
    if(ci_control.slot_info[slot].ca_pmt.program_id==1)
    {
        CI_APP_MUTEX_LOCK();
        ci_control.slot_info[slot].ca_pmt.program_id=prog_id;
        CI_APP_MUTEX_UNLOCK();
    }
#endif

    if (ci_control.slot_info[slot].ca_pmt.pmt_crc != 0
        && ci_control.slot_info[slot].ca_pmt.program_id == prog_id)
    {
        CI_APP_MUTEX_LOCK();
        //ci_set_pmt_cmd(0, FALSE, FALSE, slot);
        ci_control.slot_info[slot].ca_pmt.cmd_id = CI_CPCI_QUERY;
        //some cam need force to send CI_CPCI_QUERY of ca_pmt.
        ci_control.slot_info[slot].ca_pmt.force_resend = TRUE;
        ci_control.slot_info[slot].ca_pmt.ca_pmt_list_management = CI_CPLM_ONLY;
        CI_APP_MUTEX_UNLOCK();

        api_ci_send_pmt(slot, (UINT32)(&ci_control.slot_info[slot].ca_pmt));
    }
}

/* Internal API for CI Module */
/* Get Current Program id of Current Playing Channel */
UINT32 ci_get_cur_program_id(int slot)
{
    ASSERT(slot >= 0 && slot < CI_MAX_SLOT_NUM);
    return ci_control.slot_info[slot].ca_pmt.program_id;
}

/* API for CI Module */
/* Get Current Program id of Current Playing Channel */
UINT32 api_ci_set_cur_progid(int slot, UINT32 prog_id)
{
    ASSERT(slot >= 0 && slot < CI_MAX_SLOT_NUM);
    ci_control.slot_info[slot].ca_pmt.program_id = prog_id;
}

/* API for CI Switch Audio PID, Send Descramble Cmd to CI Slots */
void api_ci_switch_aid(UINT32 prog_id, UINT16 pid)
{
    int i, j;
    UINT8 slot_mask = 0;

    APPL_PRINTF("%s: program %X, pid %d\n",__FUNCTION__,prog_id,pid);

    CI_APP_MUTEX_LOCK();
    for (i=0; i<CI_MAX_SLOT_NUM; i++)
    {
        if (prog_id == ci_control.slot_info[i].ca_pmt.program_id)
        {
            for (j = 0; j < ci_control.slot_info[i].audio_count; j++)
            {
                if (pid == ci_control.slot_info[i].ca_pmt.es_pid.pid[j + CI_AUDIO_INDEX])
                {
                    ci_control.slot_info[i].ca_pmt.es_pid.pid[j + CI_AUDIO_INDEX] =
                                ci_control.slot_info[i].ca_pmt.es_pid.pid[CI_AUDIO_INDEX];
                    ci_control.slot_info[i].ca_pmt.es_pid.pid[CI_AUDIO_INDEX] = pid;

                    break;
                }
            }

            if (ci_control.slot_info[i].ca_pmt.pmt_crc != 0)
            {
//091015 fix issue: Kaon Crypto Works Cam Card, switch audio pid, no video.
//some card maybe has a little mosaic when switch audio, it's not serious.
//                ci_set_pmt_cmd(ci_control.slot_info[i].ca_pmt.program_id, FALSE, TRUE, i);
                ci_set_pmt_cmd(ci_control.slot_info[i].ca_pmt.program_id, TRUE, FALSE, i);
                slot_mask |= (1 << i);
            }
        }
    }
    CI_APP_MUTEX_UNLOCK();

    ci_send_ca_pmt(slot_mask);
}

//set state in API (of task PUB)
void ci_set_state(UINT32 state)
{
    if (state == CI_STATE_START)
    {
        osal_flag_set(ci_control.state, CI_STATE_START);
    }
    else if (state == CI_STATE_STOP)
    {
        osal_flag_clear(ci_control.state, CI_STATE_START);
    }
    else
    {
    }
}

//get state in Stack task.
UINT32 ci_get_state(int slot)
{
    UINT32 ptn_flag;
    ER ret;

    ret = osal_flag_wait(&ptn_flag,ci_control.state,CI_STATE_START
        ,TWF_ANDW, 0);
    if(E_OK == ret)
    {
        if ((ptn_flag&CI_STATE_START) != 0)
            return CI_STATE_START;
        else
            return CI_STATE_STOP;
    }
    else
        return CI_STATE_STOP;
}

#ifdef CI_PLUS_SUPPORT
extern UINT8 dis_yuv_output_for_ict;
#endif
/* API for When Stop Playing some Service, Stop Send Cmd to CI Slots */
void api_ci_stop_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask)
{
    int slot;
    static struct ci_uri_message uri_msg_data;
    static struct ci_uri_message *uri_msg = &uri_msg_data;
    BOOL b_ci_plus_cam = FALSE;

    APPL_PRINTF(">>> api_ci_stop_service: unregister callback\n");

#ifndef CI_PLUS_URI_RESPONSE_REALTIME
    //set state in API (of task PUB)
    ci_set_state(CI_STATE_STOP);
#endif

    // Unregister CA_PMT Callback Function for SI Monitor
#ifdef NEW_DEMO_FRAME
    if (service == NULL || service->monitor_id == (UINT32)(-1))
    {
        si_monitor_unregister_pmt_cb((section_parse_cb_t)ci_ca_pmt_cb);
    }
    else
    {
        sim_unregister_scb(service->monitor_id,ci_ca_pmt_cb);
    }
#else
       si_monitor_unregister_pmt_cb((section_parse_cb_t)ci_ca_pmt_cb);
    si_monitor_unregister_pmt_cb((section_parse_cb_t)psi_pmt_get_ecm_pid);
#endif

#ifndef CI_FREE_PASS_CAM
    for (slot = 0; slot < CI_MAX_SLOT_NUM; slot++)
    {
        if (CICAM_CI_PLUS == api_ci_get_cam_type(slot))
            b_ci_plus_cam = TRUE;
        if (((1<<slot)&slot_mask) != 0)
        {
#ifdef CI_PLUS_SUPPORT
            ciplus_dsc_stop(slot);
            api_ci_not_selected(slot);
#endif
        }
    }
#endif

    //CI+, stop descrambler
#ifdef CI_PLUS_SUPPORT
    ci_prog_id_bak = ci_current_prog_id;
#endif

#if defined(CI_PLUS_SUPPORT) && (!(defined CI_PLUS_NO_YPBPR))
    //Clear uri state then set to default
    // api_uri_bak();
    //When stop ci service, we need to resume all configuration
    if (b_ci_plus_cam)
    {
#ifndef CI_NO_MG
        if (api_ict_control() || api_mg_control())
#else
        if (api_ict_control())
#endif
        {
#ifdef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS
#ifndef CI_PLUS_CLOSE_YPBPR_ICTMGCGMS_TEST_CASE
/* CI_PLUS_CLOSE_YPBPR_ICTMGCGMS_TEST_CASE:define it in test case(always close yuv after received ict = 1, until next ict = 0 received)*/
/* Don't define it when MP, then stb will enable YUV everytime change channel, until received next ict = 1.*/
            api_ci_msg_to_app(0, API_MSG_ENABLE_YUV_OUTPUT);
            dis_yuv_output_for_ict= 0;
#endif
#endif
        }
    }
    api_uri_clear();
#endif


    // ci_current_prog_id = CI_NO_PROG_PASS;

    //CI+, stop sdt filter
#ifdef CI_PLUS_SERVICESHUNNING
//    ci_stop_sdt_filter();    //20100805
#endif
}

UINT32 ci_get_current_prog()
{
    return ci_current_prog_id;
}

/*
 *  @api_ci_dsc_pid_stats
 *  @Usage: check if CI pid list is OK
 */

BOOL api_ci_dsc_pid_stats(UINT8 slot)
{
    return is_dsc_pid_lst_ok[slot];
}

/*
 *  @api_ci_get_dscpid_lst
 *  @Usage: get current played slot pid list
 */
BOOL api_ci_get_dscpid_lst(UINT8 slot_mask, UINT16 *pid_lst, UINT8 *pid_num, UINT8 max_num)
{
    UINT8 slot;
    UINT8 pid_num_copied = 0;

    if (pid_num) *pid_num = 0;

    if (pid_lst)
    {
        for (slot = 0; slot < CI_MAX_SLOT_NUM; slot++)
        {
            if ((1 << slot && slot_mask) && api_ci_dsc_pid_stats(slot))
            {
                if (max_num < ci_control.slot_info[slot].ca_pmt.es_pid.pid_num)
                {
                    MEMSET(pid_lst, 0x00, pid_num_copied * sizeof(UINT16));
                    if (pid_num) *pid_num = 0;
                    return FALSE;
                }
                MEMCPY(pid_lst + pid_num_copied, &ci_control.slot_info[slot].ca_pmt.es_pid.pid,
                       ci_control.slot_info[slot].ca_pmt.es_pid.pid_num * sizeof(UINT16));
                pid_num_copied += ci_control.slot_info[slot].ca_pmt.es_pid.pid_num;
                max_num -= pid_num_copied;

                if (pid_num) *pid_num += ci_control.slot_info[slot].ca_pmt.es_pid.pid_num;
            }
        }

        return TRUE;
    }
    else
        return FALSE;
}

/* When Start Playing Some Services, Set CI Route and Send Cmd to CI Slots to Descramble this Program. */
/* if dev_list == NULL && service == NULL, means use last config */
/* if slot_mask == 0, means no slot work */
void api_ci_start_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask)
{
    int slot;
    static struct ci_device_list ci_dev;    //used to keep last info
    static struct ci_service_info ci_serv;
    P_NODE p_node;
    UINT8  i;

    APPL_PRINTF(">>> %s - slot_mask %d\n", __FUNCTION__, slot_mask);
#if defined(CI_PLUS_SUPPORT) && (!(defined CI_PLUS_NO_YPBPR))
    // api_uri_bak_resume();
    // api_uri_bak_clear();
#endif

    add_pid_lst.pid_count = 0;

    if (dev_list != NULL && service != NULL)
    {
        MEMCPY(&ci_dev, dev_list, sizeof(ci_dev));
        MEMCPY(&ci_serv, service, sizeof(ci_serv));
    }

    // Record prog_id for later check
    if (NULL != service)
        ci_current_prog_id = service->prog_id;

    for (slot = 0; slot < CI_MAX_SLOT_NUM; slot++)
    {
        if (((1<<slot)&slot_mask) == 0)
        {
            if (ci_control.mode == CI_SERIAL_MODE)
            {
                //reset slot
                CI_APP_MUTEX_LOCK();
                ci_control.slot_info[slot].m_id = -1;
                ci_control.slot_info[slot].ca_pmt.pmt_crc = 0;
                MEMSET(&ci_control.slot_info[slot].ca_pmt, 0, sizeof(struct ci_ca_pmt));
                CI_APP_MUTEX_UNLOCK();
            }
            continue;
        }

        is_dsc_pid_lst_ok[slot] = FALSE;
        ci_set_current_slot((UINT8)slot);
        // CI Route should be Made Up before Calling this Function.
        // For PVR, nim_dev is NULL. if dmx_dev is the Same, no Need Reset TS Type
        if ((ci_control.slot_info[slot].nim_dev != ci_dev.nim_dev && ci_dev.nim_dev != NULL)
            || ci_control.slot_info[slot].dmx_dev != ci_dev.dmx_dev)
        {
            APPL_PRINTF("%s: device update!\n", __FUNCTION__);
            api_set_nim_ts_type(2, 1<<slot);
            api_ci_set_dev(ci_dev.nim_dev, ci_dev.dmx_dev, slot);
        }

//#ifndef NEW_DEMO_FRAME
        // new pub has no this interface!
        if (cc_get_cmd_cnt() == 0) //only respond last cc cmd
//#endif
        {
            //pass this cam
            api_set_nim_ts_type(3, 1<<slot);

            //always change monitor id
            CI_APP_MUTEX_LOCK();
            ci_control.slot_info[slot].m_id = ci_serv.monitor_id;
            CI_APP_MUTEX_UNLOCK();

            //fix BUG13800
            //fix BUG15259 - 080409: BUG15259 is conflicting with BUG13800,
            //so BUG13800 need fix in AP.
//fix BUG25253: for 3602, always send ca_pmt
//1. if always send ca_pmt, maybe has some mosaic when exit menu.
//2. if not always send ca_pmt, some CAM maybe not descramble when exit menu.
//3. some CAM shall not sent ca_pmt when exit menu.
//so 1 or 2 ???
#if (!defined CI_PLUS_SUPPORT) && (!defined HD_PLUS_SUPPORT)
#if ((defined (CI_SLOT_DYNAMIC_DETECT)) || ((!defined (CC_USE_TSG_PLAYER))&&(!defined (CI_STREAM_PASS_MATCH_CARD))))
            if((CC_CMD_RESET_CRNT_CH != ci_serv.prog_id && \
                ci_control.slot_info[slot].ca_pmt.program_id != ci_serv.prog_id) ||
                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_VIDEO_INDEX] != ci_serv.video_pid & 0x1FFF ||
                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_AUDIO_INDEX] != ci_serv.audio_pid & 0x1FFF/* ||
                ci_control.slot_info[slot].m_id != ci_serv.monitor_id*/)    //revert for BUG20104 conflicted with BUG22693
#endif
#endif
            {
                CI_APP_MUTEX_LOCK();
                INT32 prog_in_db = FALSE;
                UINT8 *pid_num = &ci_control.slot_info[slot].ca_pmt.es_pid.pid_num;

                if(CC_CMD_RESET_CRNT_CH != ci_serv.prog_id)
                    ci_control.slot_info[slot].ca_pmt.program_id = ci_serv.prog_id;

                *pid_num = 0;
                ci_control.slot_info[slot].ca_pmt.es_pid.pid[(*pid_num)++] = ci_serv.video_pid & 0x1FFF;

                //If program is not in DB, it's may be a playback, just need
                //pass the PID in the ci_service
                prog_in_db = get_prog_by_id(ci_serv.prog_id, &p_node);
                if (SUCCESS != prog_in_db)
                {
                    APPL_PRINTF("%s: prog_in_db = %d, played program is not in DB, use PID in parameters\n",
                                __FUNCTION__, prog_in_db);
                    ci_control.slot_info[slot].ca_pmt.es_pid.pid[(*pid_num)++] = ci_serv.audio_pid;
                }
                else
                {
                    MEMCPY(&ci_control.slot_info[slot].ca_pmt.es_pid.pid[*pid_num],
                           &p_node.audio_pid, p_node.audio_count * sizeof(UINT16));
                    *pid_num += p_node.audio_count;

                    for (i = 0; i < p_node.audio_count; i++)
                    {
                        ci_control.slot_info[slot].ca_pmt.es_pid.pid[i + CI_AUDIO_INDEX] &= 0x1FFF;
                        if (ci_serv.audio_pid& 0x1FFF == ci_control.slot_info[slot].ca_pmt.es_pid.pid[i + CI_AUDIO_INDEX])
                        {
                            ci_control.slot_info[slot].ca_pmt.es_pid.pid[i + CI_AUDIO_INDEX] =
                                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_AUDIO_INDEX] & 0x1FFF;
                            ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_AUDIO_INDEX] = ci_serv.audio_pid & 0x1FFF;;

                            break;
                        }
                    }
                    ci_control.slot_info[slot].audio_count = p_node.audio_count;
                }
                if (ci_serv.ttx_pid != 0x1fff)
                    ci_control.slot_info[slot].ca_pmt.es_pid.pid[(*pid_num)++] = ci_serv.ttx_pid;
                if (ci_serv.subt_pid != 0x1fff)
                    ci_control.slot_info[slot].ca_pmt.es_pid.pid[(*pid_num)++] = ci_serv.subt_pid;
                //any more pid...
//                ci_control.slot_info[slot].ca_pmt.es_pid.pid[CI_xxx_INDEX] = ci_serv.xxx_pid;

                ci_control.slot_info[slot].ca_pmt.pmt_crc = 0;
                CI_APP_MUTEX_UNLOCK();

                APPL_PRINTF("%s: program id - 0x%X, slot - %d\n",__FUNCTION__,ci_control.slot_info[slot].ca_pmt.program_id,slot);
                APPL_PRINTF("%s: v - %d, a - %d\n",__FUNCTION__,ci_serv.video_pid,ci_serv.audio_pid);
            }

            // Register Callback Function to sie_monitor
#ifdef NEW_DEMO_FRAME
//            if (ci_serv.monitor_id == (UINT32)(-1))
//                si_monitor_register_pmt_cb((section_parse_cb_t)ci_ca_pmt_cb);
//            else
                sim_register_scb(ci_serv.monitor_id, ci_ca_pmt_cb, NULL);
#else
            si_monitor_register_pmt_cb((section_parse_cb_t)ci_ca_pmt_cb);
            si_monitor_register_pmt_cb((section_parse_cb_t)psi_pmt_get_ecm_pid);
#endif
            APPL_PRINTF("%s: register callback\n",__FUNCTION__);

        }
//#ifndef NEW_DEMO_FRAME
        else
        {
            APPL_PRINTF("     cc cmt count != 0\n\n");
        }
//#endif
    }
    //set state in API (of task PUB)
    ci_set_state(CI_STATE_START);

}

/*BUG22331*/
/*
void api_ci_init_pmtcrc(UINT8 slot)
{
    ci_control.slot_info[slot].pmt_crc = 0;
}
*/

#else    //CI_SUPPORT
void api_set_nim_ts_type(UINT8 type, UINT8 slot_mask)
{
}
void cc_send_ca_pmt(UINT32 prog_id)
{
}
void api_send_ca_pmt(UINT8 slot)
{
}
void api_send_ca_pmt_ext(UINT8 slot, enum ci_ca_pmt_cmd_id cmd_id)
{
}
BOOL api_send_prog_ca_pmt(UINT8 slot, UINT32 prog_id)
{
    return TRUE;
}
void api_ci_switch_aid(UINT32 prog_id, UINT16 pid)
{
}
void api_ci_stop_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask)
{
}
void api_ci_start_service(struct ci_device_list *dev_list,
                    struct ci_service_info *service,
                    UINT8 slot_mask)
{
}
#endif    //CI_SUPPORT

