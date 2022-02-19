/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ci_common.c
*
*    Description: The common function of CI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>
#include <hld/dmx/dmx.h>
#include <hld/cic/cic.h>
#include <hld/decv/decv.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "ctrl_util.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#ifdef CI_SUPPORT
#include <api/libci/ci_plus.h>
#include "win_ci_common.h"

#define C_CI_DMX_ID     0

#ifndef SYS_CI_MODE
#define SYS_CI_MODE     CI_PARALLEL_MODE
#endif

static struct _win_ci_slots_info ci_slots_info;
static UINT8 ci_data_buff[CI_DATA_BUFFER_LEN];

UINT8 win_ci_msg_to_status(UINT8 msg_type)
{
    UINT8 status;

    switch(msg_type)
    {
        // CAM plug-in
        case API_MSG_CAMIN:
            status = CAM_ATTACH;
            break;
        // CAM pull-out
        case API_MSG_CAMOUT:
            status = CAM_STACK_DETACH;
            break;
        // CI create transport connect
        case API_MSG_CTC:
            status = CAM_STACK_ATTACH;
            break;
        // CI delete transport connect
        case API_MSG_DTC:
            status = CAM_DETACH;
            break;
        // unknown status
        default:
            status = 0xff;
    }

    return status;
}

static void win_ci_set_slot_status_text(UINT8 slot, UINT8 status)
{
    char ansi_str[CI_MENU_TXT_LENGTH];
    char *str;
    void *ci_buffer = (void*)win_ci_get_tmp_buf();
    unsigned int t_len = 0;
/* //for card name not reflash when enter a new card!
    if((status == ci_slots_info.status[slot]) && (ci_slots_info.status_str[slot][0] != 0))
    {
        return;
    }
*/
    ci_slots_info.status_str[slot][0] = 0;

    snprintf(ansi_str, CI_MENU_TXT_LENGTH, "Slot %d: ", slot + 1);
    switch(status)
    {
        case CAM_DETACH:
            if (CI_MENU_TXT_LENGTH > strlen(ansi_str))
            {
                t_len = CI_MENU_TXT_LENGTH - strlen(ansi_str);
                strncat(ansi_str, "Not Available", t_len-1);
            }
            break;
        case CAM_ATTACH:
            if (CI_MENU_TXT_LENGTH > strlen(ansi_str))
            {
                t_len = CI_MENU_TXT_LENGTH - strlen(ansi_str);
                strncat(ansi_str, "Initializing", t_len - 1);
            }
            break;
        case CAM_STACK_DETACH:
            if (CI_MENU_TXT_LENGTH > strlen(ansi_str))
            {
                t_len = CI_MENU_TXT_LENGTH - strlen(ansi_str);
                strncat(ansi_str, "Uninstalling", t_len-1);
            }
            break;
        case CAM_STACK_ATTACH:
            str = api_ci_get_menu_string(ci_buffer, CI_DATA_BUFFER_LEN, slot);
            if(str != NULL)
            {
                if (CI_MENU_TXT_LENGTH > strlen(ansi_str))
                {
                    t_len = CI_MENU_TXT_LENGTH - strlen(ansi_str);
                    strncat(ansi_str, str, t_len - 1);
                }
            }

            break;
        default:
            if (CI_MENU_TXT_LENGTH > strlen(ansi_str))
            {
                t_len = CI_MENU_TXT_LENGTH - strlen(ansi_str);
                strncat(ansi_str, "Unknown", t_len - 1);
            }
            break;
    }

    com_asc_str2uni((UINT8*)ansi_str, ci_slots_info.status_str[slot]);

    return;

}

void win_ci_set_slot_status(UINT8 slot, UINT8 status)
{

    win_ci_set_slot_status_text(slot, status);

    ci_slots_info.status[slot] = status;

}

UINT8 win_ci_get_slot_status(UINT8 slot)
{
    return (ci_slots_info.status[slot]);

}

UINT8 * win_ci_get_tmp_buf(void)
{
    MEMSET((void*)ci_data_buff, 0, CI_DATA_BUFFER_LEN);
    return ci_data_buff;
}



void win_ci_get_slot_status_text(UINT8 slot, UINT16* uni_slot_status)
{
    MEMCPY(uni_slot_status, ci_slots_info.status_str[slot], CI_MENU_TXT_LENGTH);

}

#ifdef CI_SLOT_DYNAMIC_DETECT
BOOL api_set_ci_slot_attached(UINT32 slot, BOOL b_attach);
BOOL api_send_ca_pmt_auto(void);
BOOL cc_tsg_ci_slot_select(UINT32 slot);
BOOL cc_tsg_ci_slot_switch(BOOL b_forced);
#endif



void win_ci_on_transport_connect(UINT8 slot)
{
    UINT8 i;
    struct nim_device *nim_dev;
    struct dmx_device *dmx_dev;
    struct vdec_device *vdec_dev;
    UINT32 scrm_flag;
    UINT8 dmx_id, nim_id, slot_id;
    P_NODE playing_pnode;
    struct list_info play_pvr_info;
    api_get_play_pvr_info(&play_pvr_info);
#ifdef CI_BYPASS_ENABLE
    BOOL ci_bypass = FALSE;
    UINT8 lock = 0;
#endif

#ifdef NEW_DEMO_FRAME
    struct ts_route_info ts_route;
    INT32 ts_route_id;
#endif

    enum ci_protect_mode prot_mode;
#ifdef DVR_PVR_SUPPORT
    struct list_info p_info;
    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
#endif

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    if (CAM_STACK_ATTACH == api_ci_check_status(slot))
    {
#ifdef DVR_PVR_SUPPORT
#ifdef CI_SLOT_DYNAMIC_DETECT
        api_set_ci_slot_attached(slot, TRUE);
#endif
#endif
        g_ca_prog_reset_flag = 0;

        nim_id = 0;
        dmx_id = 0;
#ifdef NEW_DEMO_FRAME
        ts_route_id = ts_route_check_ci(slot);
        if((ts_route_id >= 0) && (ts_route_get_by_id(ts_route_id, &ts_route) != RET_FAILURE))
        {
            nim_id = ts_route.nim_id;
            dmx_id = ts_route.dmx_id;
        }
#endif
        nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
        dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);

        //api_ci_set_mode(sys_data_get_ci_mode());
        //api_ci_set_dev(nim_dev, dmx_dev, slot);


#ifdef CI_BYPASS_ENABLE
//for not scrambled program, bypass cam!

        if ((screen_back_state != SCREEN_BACK_MENU/*play tv/radio*/)
#ifdef DVR_PVR_SUPPORT
            || (api_pvr_check_tsg_state()/*tsg*/)
#endif
            )
        {
#ifdef DVR_PVR_SUPPORT
            if (api_pvr_check_tsg_state())
            {
                ;
            }
            else
#endif
            {
                nim_get_lock(nim_dev, &lock);

                //!scrambled program
                if (lock
                    && dmx_dev != NULL
                    && SUCCESS != dmx_io_control(dmx_dev, IS_AV_SCRAMBLED, (UINT32)&scrm_flag))
                {
                    ci_bypass = TRUE;
                }
            }
        }

        if (!ci_bypass)
        {
            api_set_nim_ts_type(3, 1<<slot);
            api_send_ca_pmt(slot);
        }

#else //CI_BYPASS_ENABLE
        //always pass cam!
#ifdef CI_PLUS_SERVICESHUNNING
            //if (PVR)
            /* if (0)
            {
                prot_mode = CI_PROTECTED_IN_ACTIVE;
            }
            else
            {
                prot_mode = ci_service_shunning(playing_pnode.tp_id, playing_pnode.prog_number);
            }
            if(prot_mode == CI_PROTECTED_IN_ACTIVE) */
            {

                // We have some CAM may corrupt the TS data
/*              if (CICAM_DVB_CI == api_ci_get_cam_type(slot)
                    && !playing_pnode.ca_mode)
*/
            
            pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &p_info);
            if( (CICAM_DVB_CI == api_ci_get_cam_type(slot) && !playing_pnode.ca_mode && !pvr_info->play.play_handle)
                ||(( pvr_info->play.play_handle) && !p_info.is_scrambled))   // Fix BUG43147
                {
                    //TO DO......
                    api_set_nim_ts_type(2, 1<<slot);
                }
                else
                {
                    api_set_nim_ts_type(3, 1<<slot);
                      //We may don't have the device list constructed here
                      //So we need to operate CI again
                      //If PVR is playing, we're sure the devices is available
                      //And the live play sim id may different from PVR's
                   
                    if (api_pvr_is_playing() && play_pvr_info.ca_mode
                          && !play_pvr_info.is_reencrypt)
                                api_send_ca_pmt(slot);
                      else
                                chchg_operate_ci(0, 1<<slot);
                }
            }
#else
        api_set_nim_ts_type(3, 1<<slot);
        //fix BUG15594
        api_send_ca_pmt(slot);
#endif

#endif //CI_BYPASS_ENABLE

#ifdef NEW_DEMO_FRAME
        if((1)
#else
        if((get_cc_crnt_state() != 0)
#endif
#ifdef DVR_PVR_SUPPORT
            || (api_pvr_check_tsg_state())
#endif
            ) // CC_STATE_STOP
        {
            //when card in, need stop vdec to avoid masic.
            vdec_dev = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
            if (vdec_dev != NULL
                && dmx_dev != NULL
                && SUCCESS == dmx_io_control(dmx_dev, IS_AV_SCRAMBLED, (UINT32)&scrm_flag))
            {
                //libc_printf("%s: reset vdec!!!\n",__FUNCTION__);
//to avoid mosaic
#ifdef NEW_DEMO_FRAME
                vdec_io_control(vdec_dev,VDEC_IO_REST_VBV_BUF,0);
#else
                vdec_stop(vdec_dev, FALSE, FALSE);
                vdec_start(vdec_dev);
#endif
            }
        }
    }
    else    //cam out
    {

        ap_ci_reset_ca_pmt_status();
#ifdef DVR_PVR_SUPPORT
#ifdef CI_SLOT_DYNAMIC_DETECT
        api_set_ci_slot_attached(slot, FALSE);
#endif
#endif

        api_set_nim_ts_type(2, 1<<slot);

#ifdef CI_PLUS_SUPPORT
#ifdef DSC_SUPPORT
        if (CICAM_CI_PLUS == api_ci_get_cam_type(slot))
        {
        //CI+, close descrambler
#ifdef CI_PLUS_NEW_CC_LIB
            ciplus_dsc_close(slot);
#else
            ciplus_dsc_close();
#endif
        }
#ifndef CI_PLUS_NEW_CC_LIB
        //when CIPlus CAM out, if pvr is playbacking with AES, need to set dsc again
        if(( pvr_info->play.play_handle) && (play_pvr_info.is_reencrypt) && (!play_pvr_info.is_scrambled))
        {
            ciplus_pvr_dsc_to_pvr();
        }
#endif
#endif
#endif

        dmx_dev = dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
        //send ca_pmt to other cam cards.
        if ( dmx_dev!= NULL
            && SUCCESS == dmx_io_control(dmx_dev, IS_AV_SCRAMBLED, (UINT32)&scrm_flag))
        {
            for (i = 0; i < CI_MAX_SLOT_NUM; i++)
            {
                if (slot != i && CAM_STACK_ATTACH == api_ci_check_status(i))
                    api_send_ca_pmt(i);
            }
        }
    }
}

void win_ci_force_send_ca_pmt(void)
{
#ifdef DVR_PVR_SUPPORT
#ifdef CI_SLOT_DYNAMIC_DETECT
    api_send_ca_pmt_auto();
#endif
#endif
}

#else

#if 0
#undef api_set_nim_ts_type
#undef api_send_ca_pmt
#undef api_ci_switch_aid

void api_set_nim_ts_type(UINT8 type, UINT8 slot_mask)
{
}

void api_send_ca_pmt(UINT8 slot)
{
}
#endif
//void api_ci_switch_aid(UINT32 prog_id, UINT16 pid)
//{
//}
#endif

