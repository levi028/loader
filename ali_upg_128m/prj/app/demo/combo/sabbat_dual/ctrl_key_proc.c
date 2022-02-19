/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_key_proc.c
 *
 *    Description: This source file contains control task key process.
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
#include <hld/decv/decv.h>
#include <api/libga/lib_ga.h> //Ben 180726#1
#include "copper_common/com_api.h"
#include "control.h"
#include "menus_root.h"
#include "ctrl_key_proc.h"
#include "win_signalstatus.h"
#include "win_com_popup.h"
#include "string.id"
#include "ap_ctrl_ttx.h"
#include "win_mute.h"
#include "win_pause.h"
#include "ctrl_debug.h"
#include "ctrl_popup_msg.h"
#include "win_hdtv_mode.h"
#include "pvr_ctrl_basic.h"
#include "key.h"
#include "power.h"
#include "disk_manager.h"
#include "platform/board.h"
#include "win_mainmenu_submenu.h"
#include "win_pvr_ctrl_bar.h"
#include "ap_ctrl_display.h"
#ifdef CEC_SUPPORT
#include "cec_link.h"
#include <api/libcec/lib_cec.h>
#endif
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_mmi.h"
#include "conax_ap/win_finger_popup.h"
#include "conax_ap/cas_fing_msg.h"
#endif
#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#endif
#ifdef ES_PLAYER_TEST
#include <hld/decv/decv_media_player.h>
#include <api/libmp/es_player_api.h>
#endif

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

#ifdef PIP_PNG_GIF
#include "ad_pic_img.h"
#endif

#define MP_UPG_MSG_USB_PLUGIN   254

//#define VSC_PERSIONALIZE_TEST

#ifdef SCART_TV_MODE_LED_DISPLAY
static char     pan_disstrs[5];
static char     pan_disstrs2[5];
static char     pan_dislen = 0;
static char     pan_dislen2 = 0;
#endif
static UINT8    bfirst_play_show = FALSE;
static UINT16   pre_played_chan = 0xffff;

static BOOL     enable_key_task_get_key = FALSE;    //g_key_task_get_key
static UINT8    tv_sat_state = 0;
static char     scart_tv_dis[] = " TU ";
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
BOOL            is_progname_on_top = FALSE;
static unsigned char cf_test_onoff = 0;
#endif
#ifdef _INVW_JUICE
#define INVE_VOL_STEP  2

extern void set_win_inview_juice_status(int status);
extern int  get_win_inview_juice_status(void);
extern int  win_pwd_open_inview;
#endif
#ifdef ES_PLAYER_TEST
ali_video_config    *decv_config = NULL;
ali_audio_config    *deca_config = NULL;
static OSAL_ID      video_tsk_id = OSAL_INVALID_ID;
static OSAL_ID      audio_tsk_id = OSAL_INVALID_ID;
#endif
#ifdef PLAY_TEST
void url_play_stop();
void url_update_speed(BOOL bShowOSD, BOOL bClear, UINT32 speed);
void url_update_dec_info(char *info);
RET_CODE win_media_player_init(mp_callback_func mp_cb);
BOOL url_play_response_key(UINT32 vkey);
#endif

/**
 * Some case need key task to get key value. In normal case, ap_control_task do this work.
 */
BOOL ap_is_key_task_get_key(void)
{
    return enable_key_task_get_key;
}

UINT32 scan_code_to_msg_code(struct pan_key *key_struct)
{
#ifdef _INVW_JUICE
    ALI_IR_KEY_INFO msg_code;
    ASSERT(sizeof(ALI_IR_KEY_INFO) == 4);
#else
    IR_KEY_INFO msg_code;

    ASSERT(4 == sizeof(IR_KEY_INFO));
#endif
    msg_code.type = key_struct->type;
    msg_code.state = key_struct->state;
    msg_code.count = key_struct->count;
    msg_code.ir_code = SET_IRCODE(key_struct->code);
    return *(UINT32 *) (&msg_code);
}

void msg_code_to_key_type(UINT32 msg_code, UINT8 *type)
{
#ifdef _INVW_JUICE
    ALI_IR_KEY_INFO key = *(ALI_IR_KEY_INFO *) &msg_code;
#else
    IR_KEY_INFO     key = *(IR_KEY_INFO *) &msg_code;

#endif

    *type = key.type;
}

void msg_code_to_key_count(UINT32 msg_code, UINT8 *count)
{
#ifdef _INVW_JUICE
    ALI_IR_KEY_INFO key = *(ALI_IR_KEY_INFO *) &msg_code;
#else
    IR_KEY_INFO     key = *(IR_KEY_INFO *) &msg_code;

#endif

    *count = key.count;
}

void msg_code_to_key_state(UINT32 msg_code, UINT8 *state)
{
#ifdef _INVW_JUICE
    ALI_IR_KEY_INFO key = *(ALI_IR_KEY_INFO *) &msg_code;

#else
    IR_KEY_INFO     key = *(IR_KEY_INFO *) &msg_code;

#endif

    *state = key.state;
}
extern 	UINT8 gacas_ui_is_key_lock(void);
UINT32 ap_hk_to_vk(UINT32 start, UINT32 key_info, UINT32 *vkey)
{
    UINT32                      i = 0;
    UINT16                      n = 0;
    UINT8                      key_count = 0;
    sys_state_t                 system_state = SYS_STATE_INITIALIZING;
    struct ir_key_map_t   *g_itou_key_tab = NULL;
    IR_KEY_INFO                 *pinfo = NULL;
#ifdef _INVW_JUICE
    ALI_IR_KEY_INFO             my_key = *(ALI_IR_KEY_INFO *) &key_info;

#else
    IR_KEY_INFO                 my_key = *(IR_KEY_INFO *) &key_info;

#endif

    if(NULL == vkey)
    {
        return 0;
    }
#ifdef SUPPORT_GACAS
	if(gacas_ui_is_key_lock() > 0)
	{
   		*vkey = V_KEY_NULL;
		return 0;
	}
#endif

	if(key_count==0)
	{
		;
	}

    system_state = api_get_system_state();
    n = board_get_key_map_cnt();
    g_itou_key_tab = board_get_key_map();
    if(NULL == g_itou_key_tab)
    {
        return 0;
    }

    *vkey = V_KEY_NULL;
    for(i = start; i < n; i++)
    {
        pinfo = &g_itou_key_tab[i].key_info;
        key_count = pinfo->count;
        if ((pinfo->type == my_key.type) && (pinfo->state == my_key.state) && (pinfo->ir_code == my_key.ir_code))
        {
            *vkey = g_itou_key_tab[i].ui_vkey;
            if (SYS_STATE_USB_MP == system_state)
            {
                if (V_KEY_CC == *vkey)
                {
                    *vkey = V_KEY_SUBTITLE;
                }
            }

#ifdef BIDIRECTIONAL_OSD_STYLE
            if (osd_get_mirror_flag() == TRUE)
            {
                if (*vkey == V_KEY_LEFT)
                {
                    *vkey = V_KEY_RIGHT;
                }
                else if (*vkey == V_KEY_RIGHT)
                {
                    *vkey = V_KEY_LEFT;
                }
            }
#endif
            break;
        }
    }

    return (i + 1);
}

UINT32 ap_vk_to_hk(UINT32 start, UINT32 vkey, UINT32 *key_info)
{
    UINT16                      i = 0;
    UINT16                      n = 0;
    struct ir_key_map_t   *g_itou_key_tab = NULL;

    if (NULL == key_info)
    {
        return 0;
    }
    n = board_get_key_map_cnt();
    g_itou_key_tab = board_get_key_map();
    if (NULL == g_itou_key_tab)
    {
        return 0;
    }

    *key_info = INVALID_HK;
    for (i = start; i < n; i++)
    {
        if (g_itou_key_tab[i].ui_vkey == vkey)
        {
            *key_info = *(UINT32 *) &g_itou_key_tab[i].key_info;
            break;
        }
    }

    return (i + 1);
}

UINT8 ap_get_vk(void)
{
    UINT32  msg_code = 0;
    UINT32  vkey = 0;

GET_HK:
    msg_code = ap_get_key_msg();
    if (INVALID_HK == msg_code)
    {
        goto GET_HK;
    }

    if (msg_code != INVALID_MSG)
    {
        ap_hk_to_vk(0, msg_code, &vkey);
        return (UINT8) vkey;
    }
    else
    {
        return V_KEY_NULL;
    }
}

BOOL ap_enable_key_task_get_key(BOOL enable)
{
    BOOL    old_value = enable_key_task_get_key;

    enable_key_task_get_key = enable;
    return old_value;
}

UINT32 ap_get_key_msg(void)
{
    return ap_get_popup_msg(0);
}

UINT32 ap_get_hk(void)
{
    OSAL_ER         retval = OSAL_E_FAIL;
    control_msg_t    msg;
    UINT32          msg_siz = 0;

    MEMSET(&msg, 0, sizeof(control_msg_t));
    retval = ap_receive_msg(&msg, (INT32 *)&msg_siz, 100);
    if (OSAL_E_OK != retval)
    {
        return INVALID_MSG;
    }

    if (msg.msg_type <= CTRL_MSG_TYPE_KEY)
    {
        return msg.msg_code;
    }
    else
    {
        return INVALID_HK;
    }
}

BOOL ap_send_key(UINT32 vkey, BOOL __MAYBE_UNUSED__ if_clear_buffer)
{
    UINT32  hkey = INVALID_HK;

    ap_vk_to_hk(0, vkey, &hkey);
    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
    return TRUE;
}


#ifdef _INVW_JUICE
RET_KEY_PROC_E invw_status_check(POBJECT_HEAD topmenu)
{
    if (inview_is_started())
    {
        if (get_win_inview_juice_status() == VACT_CLOSE)
        {
            set_win_inview_juice_status(-1);
            return RET_KEY_POP_PRE_MENU;
        }

        if ((topmenu == 0) && (inview_is_app_busy()))
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win_inviewinput, TRUE);
        }
    }

    return RET_KEY_NONE;
}
#endif

#ifdef DVR_PVR_SUPPORT
static BOOL is_pvr_key(UINT32 vkey)
{
    BOOL ret = FALSE;
    switch(vkey)
    {
        case V_KEY_RECORD:
        case V_KEY_STOP:
        case V_KEY_FF:
        case V_KEY_FB:
        case V_KEY_STEP:
        case V_KEY_SLOW:
        case V_KEY_DVR_INFOR:
            ret = TRUE;
        default:
            break;
    }
    return ret;
}

static RET_KEY_PROC_E ap_key_pvr_proc(UINT32 key, UINT32 vkey)
{
    UINT8               back_saved = 0;
    enum PVR_DISKMODE   pvr_mode = PVR_DISK_INVALID;
    char                rec_disk[16] = {0};
    char                tms_disk[16] = {0};
    pvr_play_rec_t  *pvr_info = NULL;

	if(0 == pvr_mode)
	{
		;
	}
	pvr_info  = api_get_pvr_info();

#ifdef _MHEG5_SUPPORT_
    if((pvr_info->hdd_valid) && is_pvr_key(vkey))
    {
        if(PROC_PASS != mheg_hook_proc(UI_MENU_OPEN,NULL, NULL))
        {
            ;
        }
    }
#endif
    
    //no PVR partition hint
    if (((V_KEY_RECORD == vkey)||(V_KEY_DVRLIST == vkey))
    && ((SCREEN_BACK_VIDEO == screen_back_state) || (SCREEN_BACK_RADIO == screen_back_state))
    && (VIEW_MODE_FULL == hde_get_mode()))
    {

        pvr_mode = pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));

#ifdef SUPPORT_CAS7
        win_mmipopup_close(1);
#endif
        if ((!pvr_info->hdd_valid) || ((!rec_disk[0])))
        {
#ifdef _INVW_JUICE
            if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
            {
#endif
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);

                win_compopup_set_msg_ext("No Record partition!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(2000);
                win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
            }
#endif
#ifdef SUPPORT_CAS7
            win_ca_mmi_check();
#endif
            return  RET_KEY_RETURN;
        }
    }

    if (api_pvr_respond_key(key, vkey, key) != PROC_PASS)
    {
#ifdef SUPPORT_CAS7
        win_mmipopup_close(1);
        win_ca_mmi_check();
#endif
        return  RET_KEY_RETURN;
    }

    return  RET_KEY_NONE;
}
#endif

#ifdef _INVW_JUICE
static void ap_key_invw_volume_proc(UINT32 key, UINT32 vkey)
{
    SYSTEM_DATA *sys_data = NULL;

    if ((vkey == V_KEY_V_UP) || (vkey == V_KEY_V_DOWN))
    {
        sys_data = sys_data_get();
        if (sys_data->volume > 0 && get_mute_state())
        {
            set_mute_on_off(TRUE);

            restore_mute_bg();
        }

        if (vkey == V_KEY_V_UP)
        {
            sys_data->volume += INVE_VOL_STEP;
        }

        if ((V_KEY_V_DOWN == vkey) && (sys_data->volume >= INVE_VOL_STEP))
        {
            sys_data->volume -= INVE_VOL_STEP;
        }

        api_audio_set_volume(sys_data->volume);
    }
    else if (vkey == V_KEY_MUTE)
    {
        // process MUTE key whilst Inview menu has OSD
        set_mute_on_off(TRUE);
    }
}

static void ap_key_invw_new_menu(POBJECT_HEAD *menu, UINT32 vkey, BOOL *bflag, PRESULT *proc_ret)
{
    if (menu == (POBJECT_HEAD) & g_win_inviewinput)
    {
        int inview_code;
        int ret_code;
        ret_code = inview_code_map(vkey, &inview_code);
        inview_resume(inview_code);
        inview_handle_ir(inview_code);
        osal_task_sleep(100);   /* fix of #6523 - see redmine for details */
        *proc_ret = PROC_PASS;
        bflag = FALSE;
    }
    else
    {
        inview_pause(FALSE, 1);
        if ((vkey != V_KEY_RECALL) && (vkey != V_KEY_TVRADIO))
        {
            *proc_ret = osd_obj_open(menu, MENU_OPEN_TYPE_KEY + vkey);
        }
    }
}
#endif

static RET_KEY_PROC_E ap_subkey_proc(UINT32 *pkey, UINT32 *pvkey, UINT32 *prevkey, UINT32 *pstart, UINT32 *pnext_start)
{
    UINT32  key = *pkey;
    UINT32  vkey = *pvkey;
    UINT32  start = *pstart;
    UINT32  next_start = *pnext_start;

#ifdef DISABLE_PVR_TMS
    if ((V_KEY_RECORD == vkey) || (V_KEY_DVRLIST == vkey))
    {
        vkey = V_KEY_NULL;
    }
#endif

#ifdef SHOW_MAF_FLAG_ON_SCREEN
    if (vkey == V_KEY_SWAP)
    {
        static BOOL open_layer_onoff = TRUE;
        if (open_layer_onoff)
        {
            vp_test_osd_layer2_init();
        }
        open_layer_onoff = FALSE;
    }
#endif

#ifdef SUPPORT_CAS7
    if ((V_KEY_EXIT == vkey) && (0 == conax_no_card_flg) && (1 == g_ca7_correct_card))
    {
        win_mmipopup_close(1);
    }
#endif

    if (V_KEY_NULL != vkey)
    {
#ifdef MULTI_CAS
#ifndef _BUILD_OTA_E_
#if (CAS_TYPE == CAS_C2000A)
        if (&g_win2_mainmenu == (CONTAINER *)menu_stack_get_top())
        {
            ap_cas_caslib_print(vkey);
        }

#elif (CAS_TYPE == CAS_C2200A)
        if (vkey == V_KEY_RED)
        {
            ippv_window_open();
        }
#endif
#endif
#endif

#ifdef _BC_CA_ENABLE_
        if(vkey == V_KEY_YELLOW)
        {
            if(0 == cf_test_onoff)
            {
                cf_test_onoff = 1;
                IO_Printf("\n    **BC CF test On**\n");
            }
            else
            {
                cf_test_onoff = 0;
                IO_Printf("\n    **BC CF test Off**\n");
            }
            bc_cf_test_mode(cf_test_onoff);
        }
        else if(vkey == V_KEY_GREEN)
        {
            bc_dvr_test_set_testcase();
            IO_Printf("\n   **DVR test case = %d **\n", bc_dvr_test_get_testcase());
        }
        else if(vkey == V_KEY_MOVE)
        {
            bc_set_iptv_test();
        }
#endif

#ifndef _BUILD_OTA_E_
        if (nim_debug_key_proc(vkey))
        {
            *pvkey = vkey;
            *pkey = key;
            *pstart = start;
            *pnext_start = next_start;

            return RET_KEY_RETURN;
        }
#endif
    }

    if (0 == start)
    {
        *prevkey = vkey;
    }

    start = next_start;
    if (V_KEY_NULL == vkey)
    {
        if (*prevkey == vkey)
        {   /* First time */
            *pvkey = vkey;
            *pkey = key;
            *pstart = start;
            *pnext_start = next_start;

            return RET_KEY_RETURN;
        }
        else
        {
            /* If try mapping the key for sever times ,then
               try to use the first mapping vkey for unkown key processing.
               ( Only valid for no menu on screen processing )*/
            vkey = *prevkey;
            *pvkey = vkey;
            *pkey = key;
            *pstart = start;
            *pnext_start = next_start;

            return RET_KEY_UNKOWN_KEY_PROC;
        }
    }

#ifdef CEC_SUPPORT
    else if (V_KEY_PLAY == vkey)
    {
        cec_link_one_touch_play();
    }
    else if (V_KEY_POWER == vkey)
    {
        cec_link_power_off_by_local_user();
    }
    else if (V_KEY_STOP == vkey)
    {
        cec_link_stop_play();
    }
    else if (V_KEY_V_UP == vkey)
    {
        if (api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
        {
            cec_link_system_audio_control_vol_up();
            vkey = V_KEY_NULL;
        }
    }
    else if (V_KEY_RIGHT == vkey)
    {
        if (api_cec_link_get_current_menu_activate_status() == CEC_MENU_STATE_DEACTIVATE)
        {
            //NO Menu Display
            if (api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
            {
                cec_link_system_audio_control_vol_up();
                vkey = V_KEY_NULL;
            }
        }
    }
    else if (V_KEY_V_DOWN == vkey)
    {
        if (api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
        {
            cec_link_system_audio_control_vol_down();
            vkey = V_KEY_NULL;
        }
    }
    else if (V_KEY_LEFT == vkey)
    {
        if (api_cec_link_get_current_menu_activate_status() == CEC_MENU_STATE_DEACTIVATE)
        {
            //NO Menu Display
            if (api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
            {
                cec_link_system_audio_control_vol_down();
                vkey = V_KEY_NULL;
            }
        }
    }
    else if (V_KEY_MUTE == vkey)
    {
        if (api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
        {
            cec_link_system_audio_control_toggle_mute();
            vkey = V_KEY_NULL;
        }
    }
#endif
#ifdef FSC_SUPPORT
    else if(vkey == V_KEY_REPEATAB)
    {
        if(NULL == menu_stack_get_top())
        {
            fsc_switch_mode();
        }
            
    }
#endif
    else
    {
#if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
        on_event_user_pressed_key(vkey);
#endif
    }

    *pvkey = vkey;
    *pkey = key;
    *pstart = start;
    *pnext_start = next_start;

    return RET_KEY_NONE;
}

#ifdef SUPPORT_CAS9 //Fix #31060 Fp show once while enter main menu or epg menu
static void win_ca_finger_print_clean(void)
{
    struct vpo_osd_show_time_s show_finger;
	
    //fixed issue,  The fingerprint is shown while enter main menu or EPG
    MEMSET(&show_finger,0,sizeof(struct vpo_osd_show_time_s));

    if(1 == is_fp_displaying())
    {
        win_fingerpop_close();
        //libc_printf("Force Stop FP\n");
    }
}
#endif

static RET_KEY_PROC_E ap_key_proc_no_menu(POBJECT_HEAD *topmenu, UINT32 key, UINT32 vkey,
      UINT32 __MAYBE_UNUSED__ osd_msg_type, UINT32 osd_msg_code, PRESULT *proc_ret)
{
#ifndef _BUILD_OTA_E_
    UINT8   rec_pos = 0;
    pvr_play_rec_t  *pvr_info = NULL;
#endif
    BOOL    bfind = FALSE;
    BOOL    bflag =FALSE;
    POBJECT_HEAD    new_menu = NULL;
    
#ifdef PLAY_TEST
    P_NODE playing_pnode;
#endif

#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif
    bfind = menus_find_root(CTRL_MSG_SUBTYPE_KEY, osd_msg_code, &bflag, &new_menu);

#if defined(SUPPORT_CAS9)   //Fix #31060 Fp show once while enter main menu or epg menu
//if((TRUE==bfind) && ((new_menu== (POBJECT_HEAD) & g_win2_mainmenu) || (new_menu== (POBJECT_HEAD) & g_win2_epg)))
//1225 FP
if((TRUE==bfind) && ((new_menu== (POBJECT_HEAD) & g_win2_epg)))//(new_menu== (POBJECT_HEAD) & g_win2_mainmenu) ||
{
    //libc_printf("Check FP\n");
    win_ca_finger_print_clean();
}
#endif

#ifdef FAST_CHCHG_TEST  // fisrt do channel change(send CC cmd->PUB)..then draw OSD progname_bar
    if (bfind && new_menu == (POBJECT_HEAD) & g_win2_progname)
    {
        UINT32  t_vkey;
        ap_hk_to_vk(0, osd_msg_code, &t_vkey);

        VACTION act = win_fast_chchg_progname_keymap(t_vkey);
        if ((*topmenu = menu_stack_get_top()) == NULL)
        {
            api_set_fast_chchg(TRUE);
        }

        win_progname_unkown_act_proc(act);  // respond key here
        bflag = FALSE;  // will not send this key to channel bar.
    }
#endif
#ifdef DVR_PVR_SUPPORT
    if (bfind && new_menu == (POBJECT_HEAD) & g_win_pvr_ctrl)
    {
        #ifdef MANUAL_TMS
        if(vkey == V_KEY_PAUSE)
        {
     
        //If you want pause vpo before start timeshift, enable these code.
/*                        
        //Pause vpo 
    	struct vpo_device *vpo_old = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
        #ifdef DUAL_VIDEO_OUTPUT    
        	struct vpo_device *vpo_new = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
        #else
                struct vpo_device *vpo_new = NULL;
        #endif	
    vpo_ioctl(vpo_old,VPO_IO_PLAYMODE_CHANGE,NORMAL_2_ABNOR);	
    if(vpo_new != NULL)
    vpo_ioctl(vpo_new,VPO_IO_PLAYMODE_CHANGE,NORMAL_2_ABNOR);   

       api_video_set_pause(1);
//*/                

        if(1)	//Ben 180621#1
        {

              //Pop message "Preparing to timeshift ..."
        	UINT8 back_saved;

        	win_compopup_init(WIN_POPUP_TYPE_SMSG);
        	win_compopup_set_msg_ext("Preparing to timeshift ...", NULL, 0);
        	#ifndef SD_UI
        	win_compopup_set_frame(290, 250, 448, 100);
        	#else
        	win_compopup_set_frame(150, 150, 300, 100);
        	#endif
        	win_compopup_open_ext(&back_saved); 
        }
        //start timeshift
        api_pvr_tms_proc(TRUE); // add code
//        b_tms_Flag = TRUE;

    	win_compopup_smsg_restoreback();   //Clear pop message 
                                
        }
         #endif

    
        if (!pvr_enter_menu(vkey))
        #if 0 //def MANUAL_TMS
        {
                 api_pvr_tms_proc(FALSE);// add code
        	 return RET_KEY_UNKOWN_KEY_PROC; //not enter pvr_ctrl_bar!
         }
         #else
               return RET_KEY_UNKOWN_KEY_PROC; //not enter pvr_ctrl_bar!
        #endif	
    }

    if ((key & 0x8000)    /* use same key (old RC) for prog_bar and pvr_bar */
    && (V_KEY_INFOR == vkey) && pvr_info->hdd_valid
    && ((NULL != api_pvr_get_rec_by_prog_id( pvr_info->play.play_chan_id, &rec_pos))
       || ( pvr_info->play.play_chan_id == pvr_info->tms_chan_id)
       || ( pvr_info->play.play_handle != 0)))
    {
        if (ap_pvr_set_state() != PVR_STATE_IDEL)
        {
            new_menu = (POBJECT_HEAD) & g_win_pvr_ctrl;
            bflag = FALSE;
            bfind = TRUE;
        }
    }

    if (bfind)
    {
        if (new_menu == (POBJECT_HEAD) & g_win_usb_filelist)
        {
            if (!check_storage_item_is_active())
            {
                return RET_KEY_UNKOWN_KEY_PROC;   //cann't enter usb file list!
            }
        }
        else if ((new_menu == (POBJECT_HEAD) & g_win_record)
             && (!pvr_info->hdd_valid
#ifdef RAM_TMS_TEST
                || sys_data_get_ram_tms_en()
#endif
#if (BC_PVR_CHANNEL_ID_NUM < 3)
                || ( pvr_info->rec_num >= PVR_MAX_RECORDER)
#endif
                ))
        {
#if (!defined CAS9_REC2_PLUS_USBPLAY1) && (!defined(GCA_REC2_PLUS_USBPLAY1))  
#if (BC_PVR_CHANNEL_ID_NUM < 3)
            if ( pvr_info->rec_num >= PVR_MAX_RECORDER)
            {
                UINT8   back_saved = 0;

                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                win_compopup_set_msg_ext("Please stop a record first!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(2000);
                win_compopup_smsg_restoreback();
            }
#endif
            return RET_KEY_UNKOWN_KEY_PROC;       //cann't enter record management!
#endif  
        }
    }
#endif
    if (!bfind)
    {
#ifdef AUDIO_DESCRIPTION_SUPPORT
        if (vkey == V_KEY_PIP)
        {
            win_popup_ad_setting();
        }
#endif
#ifdef PLAY_TEST
        if(vkey == V_KEY_RED)
        {
            if(api_get_system_state() == SYS_STATE_TEST_PLAY)
            {
                if(menu_stack_get_top() == (POBJECT_HEAD)&g_win_play)
                {
                    osd_obj_close((POBJECT_HEAD)&g_win_play,C_CLOSE_CLRBACK_FLG);
                    menu_stack_pop();
                }
                //url_play_stop();
                url_update_speed(0, 0, 0);
				url_play_stop();
                url_update_dec_info(NULL);
                media_player_release();
				win_media_player_init((mp_callback_func)(mp_apcallback));
            }
            api_set_system_state(SYS_STATE_NORMAL);
            MEMSET(&playing_pnode,0,sizeof(P_NODE));
            ap_get_playing_pnode(&playing_pnode);
            api_play_channel(playing_pnode.prog_id, TRUE, TRUE, TRUE);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD)&g_win2_progname, TRUE);
        }
#endif
        return RET_KEY_HK_TO_VKEY;
    }
    else /* Enter a new menu*/
    {
#ifdef PLAY_TEST
        if(pvr_info->pvr_state && new_menu == (POBJECT_HEAD)&g_win_play)
        {
            return RET_KEY_UNKOWN_KEY_PROC;
        }
#endif
#ifdef DVR_PVR_SUPPORT
		if(!pvr_info->hdd_valid && new_menu == (POBJECT_HEAD)&g_win_record)
		{
			return RET_KEY_UNKOWN_KEY_PROC;
		}
#endif
#if (defined(MULTI_CAS) && defined(SUPPORT_CAS_A))
        on_event_system_is_entering_new_mmi_menu(new_menu);

        if (get_signal_stataus_show())/* sync green MMI, need?? */
        {
            show_signal_status_osdon_off(0);
        }

#ifdef SHOW_ALI_DEMO_ON_SCREEN
        show_ali_demo(FALSE);
#endif
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if ((get_mmi_msg_cnt() > 0)
            && (get_mmi_showed() != CA_MMI_PRI_05_MSG 
            ||((new_menu != (POBJECT_HEAD)&g_win2_progname)
            &&(new_menu != (POBJECT_HEAD)&g_win2_volume)
            &&(new_menu != (POBJECT_HEAD)&g_win2_audio)
            &&(new_menu != (POBJECT_HEAD)&g_win2_light_chanlist))))
        {
#if defined(SUPPORT_CAS7)           //SUPPORT_CAS7 alone
            win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
            win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
            //MSG IS SHOWING,DON'T ALLOW TO OPEN LIGHT CHANLIST.
            if((get_mmi_showed() != CA_MMI_PRI_05_MSG) 
                &&(new_menu != (POBJECT_HEAD)&g_win2_light_chanlist)) 
                win_mmipopup_close(CA_MMI_PRI_05_MSG);
        }
#endif
#if (defined(_MHEG5_V20_ENABLE_))
        if (PROC_PASS != ali_mheg_hook_proc(UI_MENU_OPEN, new_menu, NULL))
        {
            ;
        }
#endif
#ifdef _MHEG5_SUPPORT_
        if(PROC_PASS != mheg_hook_proc(UI_MENU_OPEN,new_menu, NULL))
        {
            *proc_ret = PROC_LOOP;
            return RET_KEY_NONE;
        }
#endif
        if (get_signal_stataus_show())
        {
            show_signal_status_osdon_off(0);
        }

#ifdef SHOW_ALI_DEMO_ON_SCREEN
        show_ali_demo(FALSE);
#endif
        win_msg_popup_close();

        /* Open the new menu*/
#ifdef _INVW_JUICE
        ap_key_invw_new_menu(new_menu, vkey, &bflag, proc_ret);
#else
        *proc_ret = osd_obj_open(new_menu, MENU_OPEN_TYPE_KEY + vkey);
#endif
        if (*proc_ret != PROC_LEAVE)
        {
            menu_stack_push(new_menu);
#ifdef CEC_SUPPORT
            // When Menu Create (deactivated>
            cec_link_report_menu_status();
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            if (menu_stack_get_top() == (POBJECT_HEAD) & g_win2_mainmenu)
            {
                set_mmi_showed(CA_MMI_PRI_02_CAMENU);
            }
            else if (menu_is_mmi0())
            {
                set_mmi_showed(CA_MMI_PRI_00_NO);
            }
            else if ((menu_stack_get_top() != (POBJECT_HEAD) &g_win2_progname)
                && (menu_stack_get_top() != (POBJECT_HEAD) &g_win_pvr_ctrl))
            {
                set_mmi_showed(CA_MMI_PRI_10_DEF);
            }
#endif
            if (bflag)
            {
                *topmenu = new_menu;
                return RET_KEY_MENU_KEY_PROC;
            }
            *proc_ret = PROC_LOOP;
        }
        else
        {
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            if (get_mmi_msg_cnt() > 0)
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
                MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: open root menu fail; code: 0\n");
                set_mmi_showed(CA_MMI_PRI_10_DEF);
            }
#endif
            *proc_ret = PROC_PASS;
        }
    }

    return RET_KEY_NONE;
}

static RET_KEY_PROC_E ap_key_nonkey_msg_proc(UINT32 msg_type, UINT32 msg_code, POBJECT_HEAD topmenu,
                        PRESULT *proc_ret, UINT32 *osd_msg_type, UINT32 *osd_msg_code)
{
    if ((CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT == msg_type) || (CTRL_MSG_SUBTYPE_CMD_EXIT_TOP == msg_type))
    {
        if (CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT == msg_type)
        {
            if ((msg_code == (UINT32) topmenu) && (topmenu != NULL))
            {
                *proc_ret = osd_obj_close(topmenu, C_DRAW_SIGN_EVN_FLG | C_CLOSE_CLRBACK_FLG);
                *proc_ret = PROC_PASS;
                return RET_KEY_POP_PRE_MENU;
            }

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            else if ((msg_code))
            {
                *proc_ret = osd_obj_close((POBJECT_HEAD) msg_code, C_DRAW_SIGN_EVN_FLG | C_CLOSE_CLRBACK_FLG);
                show_mute_on_off();
                show_pause_on_off();
                if ((get_mmi_msg_cnt() > 0) && (CA_MMI_PRI_02_CAMENU== get_mmi_showed()))
                {
                    set_mmi_showed(CA_MMI_PRI_10_DEF);
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
                    MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: cmd exit win; code:0\n");
                }

                *proc_ret = PROC_LOOP;
            }
#endif
        }
        else    //CTRL_MSG_SUBTYPE_CMD_EXIT_TOP
        {
            if (NULL == topmenu)
            {
                return RET_KEY_RETURN;
            }

            *proc_ret = osd_obj_close(topmenu, C_DRAW_SIGN_EVN_FLG | C_CLOSE_CLRBACK_FLG);
            *proc_ret = PROC_PASS;
            return RET_KEY_POP_PRE_MENU;
        }
    }

#ifdef DVR_PVR_SUPPORT
    else if ((CTRL_MSG_SUBTYPE_STATUS_PVR == msg_type) || (CTRL_MSG_SUBTYPE_STATUS_USB == msg_type))
    {
        *proc_ret = ap_pvr_message_proc(msg_type, msg_code, 0);
#ifdef SECURITY_MP_UPG_SUPPORT
        if ((CTRL_MSG_SUBTYPE_STATUS_PVR == msg_type) && (MP_UPG_MSG_USB_PLUGIN == msg_code))
        {
            ap_mp_upg_set_usb_plugin(TRUE);
        }
#endif
        return RET_KEY_CEHCK_PROC_RETURN;
    }
#endif
#ifdef CI_SUPPORT
    else if (CTRL_MSG_SUBTYPE_STATUS_CI == msg_type)
    {
        *proc_ret = ap_ci_message_proc(msg_type, msg_code, 0);
        return RET_KEY_CEHCK_PROC_RETURN;
    }
#endif
    else
    {
        *osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type & 0xFFFF);
        *osd_msg_code = msg_code;
    }

    return RET_KEY_NONE;
}

static RET_KEY_PROC_E ap_key_pop_pre_menu(PRESULT *proc_ret)
{
#ifndef _BUILD_OTA_E_
    BOOL            bhint_flag = FALSE;
#endif
    POBJECT_HEAD    new_menu = NULL;

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    if (menu_stack_get_top() == (POBJECT_HEAD) & g_win2_progname)
    {
        is_progname_on_top = TRUE;

    }
    else
    {
        is_progname_on_top = FALSE;
    }
#endif

#ifdef _INVW_JUICE
    // don't pop up the menu when in password window
    if (win_pwd_open_inview == 0)
    {
        menu_stack_pop();
        new_menu = menu_stack_get_top();
    }

#else
    menu_stack_pop();
    new_menu = menu_stack_get_top();
#endif

#ifdef CEC_SUPPORT
    // When Menu Create (activated>
    cec_link_report_menu_status();
#endif

    if (new_menu != NULL)    /* Open the top menu */
    {
#ifdef SHOW_ALI_DEMO_ON_SCREEN
        show_ali_demo(FALSE);
#endif
        win_msg_popup_close();
#ifndef _BUILD_OTA_E_
        if (new_menu != (POBJECT_HEAD)&g_win2_mainmenu)
        {
            *proc_ret = osd_obj_open(new_menu, MENU_OPEN_TYPE_STACK);
            if (PROC_LEAVE == *proc_ret)
            {
                return RET_KEY_POP_PRE_MENU;
            }
        }
        else
        {
            osd_track_object((POBJECT_HEAD)&mm_rightmenu_con, C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
        }
#else
	 	*proc_ret = osd_obj_open(new_menu, MENU_OPEN_TYPE_STACK);
     	if (PROC_LEAVE == *proc_ret)
      	{
                return RET_KEY_POP_PRE_MENU;
     	}
#endif
        *proc_ret = PROC_LOOP;
    }
    else    /* Restore previous menu */
    {
#ifdef _INVW_JUICE
        if (inview_is_started())
        {
            if (inview_is_app_busy())
            {
                return RET_KEY_RETURN;
            }
        }
#endif

#if (defined(_MHEG5_V20_ENABLE_))
        if (PROC_PASS != ali_mheg_hook_proc(UI_MENU_CLOSE, topmenu, NULL))
        {
            ;
        }
#endif
#ifdef _MHEG5_SUPPORT_
        if(PROC_PASS != mheg_hook_proc(UI_MENU_CLOSE, NULL, NULL))
        {
		;
        }
#endif
        if (get_pause_state())
        {
            set_pause_on_off(FALSE);
        }

#ifdef _INVW_JUICE
        if (inview_is_started())
        {
            if (!inview_is_app_busy())
            {
#endif
                show_mute_on_off();
                show_pause_on_off();
#ifdef _INVW_JUICE
            }
        }
#endif

#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if ((get_mmi_msg_cnt() > 0) && (CA_MMI_PRI_02_CAMENU== get_mmi_showed()))
        {
            set_mmi_showed(CA_MMI_PRI_10_DEF);
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
            MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: exit win; code:0\n");
        }
#endif

#ifdef DVR_PVR_SUPPORT
#ifdef _INVW_JUICE
        if (inview_is_started())
        {
            if (!inview_is_app_busy())
            {
                bhint_flag = get_rec_hint_state();
                show_rec_hint_osdon_off(bhint_flag);
            }
        }
#else
        bhint_flag = get_rec_hint_state();
        show_rec_hint_osdon_off(bhint_flag);
#endif
#endif
        *proc_ret = PROC_LOOP;
    }

    return RET_KEY_NONE;
}

#if (TTX_ON == 1)
static void ap_key_ttx_proc(void)
{
    UINT16              cur_channel = 0;
    P_NODE              p_node;
    struct t_ttx_lang   *lang_list = NULL;
    UINT8               lang_num = 0;

    ttxeng_get_init_lang(&lang_list, &lang_num);

    get_prog_at(cur_channel, &p_node);


    if (0 == lang_num)
    {
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if (get_mmi_showed() != CA_MMI_PRI_00_NO)
        {
            if (get_mmi_msg_cnt() > 0)
            {
#if defined(SUPPORT_CAS7)   //SUPPORT_CAS7 alone
                win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
                win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
            }

            if (get_mmi_showed() != CA_MMI_PRI_01_SMC)
            {
                set_mmi_showed(CA_MMI_PRI_10_DEF);
            }
        }
#endif

        if (get_signal_stataus_show())
        {
            show_signal_status_osdon_off(0);
        }

#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
        UINT8   back_saved;
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext(NULL, NULL, RS_DISPLAY_NO_DATA);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
#else
        win_com_popup_open(WIN_POPUP_TYPE_SMSG, NULL, RS_DISPLAY_NO_DATA);
        osal_task_sleep(1000);
        win_compopup_close();
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if (get_mmi_msg_cnt() > 0)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
            MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: no txt;  code:0\n");
            set_mmi_showed(CA_MMI_PRI_10_DEF);
        }
#endif
    }
    else
    {
         if (bfirst_play_show)
         {
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
             if (get_mmi_showed() != CA_MMI_PRI_00_NO)
             {
                 if (get_mmi_msg_cnt() > 0)
                 {
#if defined(SUPPORT_CAS7)   //SUPPORT_CAS7 alone
                      win_mmipopup_close(CA_MMI_PRI_01_SMC);
#else
                      win_pop_msg_close(CA_MMI_PRI_01_SMC);
#endif
                 }

                 if (get_mmi_showed() != CA_MMI_PRI_01_SMC)
                 {
                     set_mmi_showed(CA_MMI_PRI_10_DEF);
                 }
             }
#endif

#if (SUBTITLE_ON == 1)
              api_subt_show_onoff(FALSE);
#endif
              if (get_signal_stataus_show())
              {
                  show_signal_status_osdon_off(0);
              }

              bfirst_play_show = 0;
          }

           api_set_system_state(SYS_STATE_TEXT);   //save first
           ttx_osd_open();

           api_restore_system_state();
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
           if (get_mmi_msg_cnt() > 0)
           {
               ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
               MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: exit txt; code:0\n");
               set_mmi_showed(CA_MMI_PRI_10_DEF);
           }
#endif
       }
}
#endif
#ifdef MULTIVIEW_SUPPORT
static void ap_key_mv_proc()
{
    UINT8   back_saved;
    if (RADIO_CHAN == sys_data_get_cur_chan_mode())
    {
        return;
    }
    if(api_pvr_is_playing())
    {
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("Please stop record playing first!", NULL, 0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
        return;
    }
    if (api_pvr_is_recording())
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);

        win_compopup_set_msg("Please stop record before enter multiview mode?", NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_NO);

        win_popup_choice_t  choice = win_compopup_open_ext(&back_saved);
        if (choice != WIN_POP_CHOICE_YES)
        {
            return;
        }

        //stop all record
        api_stop_play_record(FALSE);
        api_pvr_tms_proc(FALSE);
        api_stop_record(0, 1);//stop first record
        api_stop_record(0, 1);//because stop first record,other record index should set to 1 again
    }

    sys_data_check_channel_groups();

    UINT8   av_flag = sys_data_get_cur_chan_mode();
    if (sys_data_get_group_num() == 0)
    {
        sys_data_set_cur_chan_mode(av_flag == TV_CHAN ? RADIO_CHAN : TV_CHAN);
    }

    if (sys_data_get_group_num() == 0)
    {
        sys_data_set_cur_chan_mode(av_flag);
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg(NULL, NULL, RS_MSG_NO_CHANNELS);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        return;
    }
    else
    {
        ap_multiview_osd_switch(TRUE);
        if (sys_data_get_multiviewmode() == 0)
        {
            if (osd_obj_open((POBJECT_HEAD) & g_cnt_multiview, 0) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_cnt_multiview);
            }
        }
        else
        {
            if (osd_obj_open((POBJECT_HEAD) & g_cnt_multiview_ext, 0) != PROC_LEAVE)
            {
                menu_stack_push((POBJECT_HEAD) & g_cnt_multiview_ext);
            }
        }
    }
}
#endif

#ifdef ES_PLAYER_TEST

static void ap_key_es_player(UINT8 vkey)
{
    if (V_KEY_GREEN == vkey)
    {
        if ((OSAL_INVALID_ID == video_tsk_id) && (OSAL_INVALID_ID == audio_tsk_id))
        {
            api_stop_play(0);
            decv_config = MALLOC(sizeof(ali_video_config));
            if (decv_config != NULL)
            {
                MEMSET(decv_config, 0, sizeof(ali_video_config));
                win_es_player_video_init(decv_config, &video_tsk_id);
            }

            deca_config = MALLOC(sizeof(ali_audio_config));
            if (deca_config != NULL)
            {
                MEMSET(deca_config, 0, sizeof(ali_audio_config));
                win_es_player_audio_init(deca_config, &audio_tsk_id);
            }
        }
    }
    else if (V_KEY_RED == vkey)
    {
        if (video_tsk_id != OSAL_INVALID_ID)
        {
            mpg_cmd_stop_proc(0);
            win_es_player_video_close(decv_config, video_tsk_id);
            video_tsk_id = OSAL_INVALID_ID;
            if (decv_config)
            {
                FREE(decv_config);
                decv_config = NULL;
            }
        }

        if (audio_tsk_id != OSAL_INVALID_ID)
        {
            win_es_player_audio_close(deca_config, audio_tsk_id);
            audio_tsk_id = OSAL_INVALID_ID;
            if (deca_config)
            {
                FREE(deca_config);
                deca_config = NULL;
            }
        }
    }

}
#endif

#ifdef SCART_TV_MODE_LED_DISPLAY
static void scart_tv_mode_display(void)
{
    if (1 == tv_sat_state)
    {
        pan_dislen = led_data_len;
        MEMCPY(pan_disstrs, led_display_data, pan_dislen);
        key_pan_display(scart_tv_dis, 4);
        led_display_flag = 2;   /*Write to buffer only*/
        pan_dislen2 = led_data_len;
        MEMCPY(pan_disstrs2, led_display_data, pan_dislen2);
    }
    else
    {
        if (pan_dislen)
        {
            led_display_flag = 0;
            if (0 == MEMCMP(led_display_data, scart_tv_dis, 4))
            {
                key_pan_display((char *)pan_disstrs, pan_dislen);
            }
            else
            {
                key_pan_display((char *)led_display_data, led_data_len);
            }
        }
    }
}
#endif

static void ap_key_mute_pause(POBJECT_HEAD topmenu, UINT32 vkey)
{
#ifdef DVR_PVR_SUPPORT
    pvr_play_rec_t  *pvr_info = NULL;
#endif
    OSD_RECT        *pmenu_rect = NULL;
    OSD_RECT        rect;
    OSD_RECT        cross_rect;
    BOOL            bshow_osd = FALSE;
    sys_state_t     system_state = SYS_STATE_INITIALIZING;

    system_state = api_get_system_state();
#ifdef DVR_PVR_SUPPORT
    pvr_info  = api_get_pvr_info();
#endif
    if (topmenu != NULL)
    {
        pmenu_rect = &topmenu->frame;
    }
    else
    {
        bshow_osd = TRUE;
    }

#ifdef _BUILD_OTA_E_
    return;
#else
    if (topmenu == (POBJECT_HEAD) & g_win2_search)
    {
        return;
    }
#ifdef YOUTUBE_ENABLE
	if(topmenu == (POBJECT_HEAD)&g_win_youtube_play)
		return;
#endif

#ifdef _INVW_JUICE
    if (inview_is_started())
    {
        if (inview_is_app_busy())
        {
            return;
        }
    }

    inview_pause(FALSE, 1);
#endif
#endif
    if (SCREEN_BACK_MENU == screen_back_state)
    {
        bshow_osd = FALSE;
    }
    else
    {
        if (V_KEY_MUTE == vkey)
        {
#ifdef CEC_SUPPORT
            if (api_cec_get_system_audio_control_feature() && (!api_cec_get_system_audio_mode_status()))
#endif
            {
                get_mute_rect(&rect);
            }
        }
        else
        {
            get_pause_rect(&rect);
        }

        if (topmenu != NULL)
        {
            if (menu_stack_get(0) != topmenu)
            {           /* If there more menus in the stack*/
                bshow_osd = FALSE;
            }
            else
            {
                osd_get_rects_cross(pmenu_rect, &rect, &cross_rect);
                if ((0 == cross_rect.u_width) || (0 == cross_rect.u_height))
                {
                    bshow_osd = TRUE;
                }
                else
                {
                    bshow_osd = FALSE;
                }

#ifdef DVR_PVR_SUPPORT
                if (topmenu == (POBJECT_HEAD) & g_win_pvr_ctrl)
                {
                    bshow_osd = FALSE;
                }
#endif
            }
        }
    }

    if (V_KEY_MUTE == vkey)
    {
        if ((SYS_STATE_9PIC == system_state) || (SYS_STATE_4_1PIC == system_state)
        #ifdef _MHEG5_SUPPORT_
          || (mheg_app_avaliable())   
        #endif
        )
        {
            set_mute_on_off(FALSE);
            save_mute_state();
            return;
        }

        if (SYS_STATE_SEARCH_PROG == system_state)
        {
            return;     //for special case do not respond
        }

#ifdef CEC_SUPPORT
        if (api_cec_get_system_audio_control_feature() && (!api_cec_get_system_audio_mode_status()))
#endif
        {
            set_mute_on_off(bshow_osd);
            save_mute_state();
        }
    }
    else
    {
        if ((SYS_STATE_SEARCH_PROG == system_state)
        || (SYS_STATE_9PIC == system_state)
        || (SYS_STATE_4_1PIC == system_state))
        {
            return; //for special case do not respond
        }
#ifndef _BUILD_OTA_E_
#ifdef CAS9_V6
        //vicky140113#3 Newglee PT
        //Derek Yang sync from c-tree for redmine #22201
        UINT16 trickplay = 0;
        CAS9_URI_PARA turi;
        UINT8 back_saved;

        conax_get_uri_info(&turi, 0);
        trickplay = turi.buri_trickplay;
        if((ECNX_URI_TRICK_NTMS == trickplay) )
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_PAUSE_DUE_URI);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(3000); //vicky140111#5 Newglee PT -Longer hint
            win_compopup_smsg_restoreback();
              return;
        }
        //End
#endif
#endif
#ifdef DVR_PVR_SUPPORT
        if ( PVR_STATE_IDEL == pvr_info->pvr_state)
        {
            set_pause_on_off(bshow_osd);
        }
#else
        set_pause_on_off(bshow_osd);
#endif
    }
}
static void ap_key_unknown_proc(UINT32 msg_type, UINT32 __MAYBE_UNUSED__ msg_code, 
    UINT32 __MAYBE_UNUSED__ key, UINT32 vkey)
{
    POBJECT_HEAD    topmenu = NULL;
    sys_state_t     system_state = SYS_STATE_INITIALIZING;
    UINT8           cur_mode = 0;
    SYSTEM_DATA     *sys_data = NULL;
    UINT16          cur_channel = 0;

	if(0 == system_state)
	{
		;
	}
	system_state = api_get_system_state();
    cur_mode = sys_data_get_cur_chan_mode();

    topmenu = menu_stack_get_top();

    sys_data = sys_data_get();
    cur_channel = sys_data_get_cur_group_cur_mode_channel();

    if (cur_channel != pre_played_chan)
    {
        bfirst_play_show = TRUE;
        pre_played_chan = cur_channel;
    }

    if (msg_type <= CTRL_MSG_TYPE_KEY)
    {
        /* Menu not processed key processing */
        if (V_KEY_POWER == vkey)
        {
#ifndef _BUILD_OTA_E_
            if( api_pvr_is_rec_only_or_play_rec())
            {
                UINT8 back_saved;
                #ifdef _MHEG5_SUPPORT_
                mheg_hook_proc(UI_MENU_OPEN,NULL, NULL);
                #endif 

                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg("Please stop record before shutdown,are you sure to stop?", NULL, 0);
                win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
                win_popup_choice_t choice = win_compopup_open_ext(&back_saved);

                if(choice == WIN_POP_CHOICE_YES)
                {
                    //stop all record
                    api_stop_play_record(FALSE);
                    api_pvr_tms_proc(FALSE);
                    api_stop_record(0,1);
                    api_stop_record(0,1);
                }
                #ifdef _MHEG5_SUPPORT_
                mheg_hook_proc(UI_MENU_CLOSE,NULL, NULL);
                #endif
            }

            if( !api_pvr_is_rec_only_or_play_rec() )
                power_switch(0);
#endif
        }
        else if ((V_KEY_MUTE == vkey)
             || ((V_KEY_PAUSE == vkey)
                && (TV_CHAN == cur_mode)
                && (VIEW_MODE_FULL == hde_get_mode())))
        {
            ap_key_mute_pause(topmenu, vkey);
        }
        else if (V_KEY_RGBCVBS == vkey)
        {
            unsigned int    sw = 0;

            sw = sys_data->avset.scart_out;
            sys_data->avset.scart_out = (SCART_RGB == sw) ? SCART_CVBS : SCART_RGB;
            sw = (SCART_RGB == sys_data->avset.scart_out) ? 1 : 0;
            api_scart_rgb_on_off(sw);
        }
        else if (V_KEY_TVSAT == vkey)
        {
            tv_sat_state = tv_sat_state ? 0 : 1;
            api_scart_tvsat_switch(tv_sat_state);
#ifdef SCART_TV_MODE_LED_DISPLAY
            scart_tv_mode_display();
#endif
        }
        else if (V_KEY_USBREMOVE == vkey)
        {
#ifdef USB_MP_SUPPORT
            if (NULL == topmenu)
            {
#ifdef SUPPORT_CAS9
                if (CA_MMI_PRI_05_MSG== get_mmi_showed())
                {
                    return;
                }
#endif
#ifdef SUPPORT_CAS7
                win_mmipopup_close(CA_MMI_PRI_01_SMC);
#endif
#ifndef _BUILD_USB_LOADER_
                usb_remove_safely_by_hotkey();
#endif
#ifdef SUPPORT_CAS9
                if (get_mmi_msg_cnt() > 0)
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
                    MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: exit txt; code:0\n");
                }
#endif
            }
#endif
        }
        else if (V_KEY_TEXT == vkey)
        {
#if (TTX_ON == 1)
            if (NULL == topmenu)
            {
                ap_key_ttx_proc();
            }
#endif
        }
#ifdef CVBS_HDMI_SWITCH
				else if (V_KEY_RED == vkey)
				{
					struct vpo_io_get_info dis_info;
					SYSTEM_DATA *sys_data = NULL;
					MEMSET(&dis_info, 0, sizeof(struct vpo_io_get_info));
					sys_data = sys_data_get();
					vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
					//if (NULL == topmenu)
					{
						if (sys_data->tv_output == CVBS_ON)
						{
							sys_data->tv_output = HDMI_ON;
							libc_printf("CVBS OFF!\n");
							if(NULL!=g_vpo_dev)
							{
								vpo_ioctl(g_vpo_dev, VPO_IO_UNREG_DAC, VDAC_USE_CVBS_TYPE);
								osal_task_sleep(50);
							}
							api_set_hdmi_phy_clk_onoff(1);
						}
						else if(sys_data->tv_output == HDMI_ON)
						{
							sys_data->tv_output = CVBS_ON;
							libc_printf("CVBS ON!\n");
							switch_tv_mode_uri(dis_info.tvsys, dis_info.bprogressive);
							osal_task_sleep(50);
							api_set_hdmi_phy_clk_onoff(0);
						}
		
						 sys_data_save(1);
					}
				}
#endif

#ifdef MULTIVIEW_SUPPORT
        else if (V_KEY_YELLOW == vkey)
        {
            if ((NULL == topmenu) 
#ifdef _MHEG5_SUPPORT_                
                && !mheg_app_avaliable()
#endif
            )
            {
                ap_key_mv_proc();
            }
        }
#endif
#if ((defined HDTV_SUPPORT) && !(defined SD_PVR) && !(defined _BUILD_OTA_E_))
#ifdef _INVW_JUICE
        else if (V_KEY_VIDEO_FORMAT == vkey)
        {
            if ((topmenu != (POBJECT_HEAD) &g_win_inviewinput)
            && (topmenu != (POBJECT_HEAD) & g_win_mpeg_player)
            && (topmenu != (POBJECT_HEAD) & g_win_usb_filelist)
            && (topmenu != (POBJECT_HEAD) & g_win_record))
            {
                hdtv_mode_pre_open(TRUE);
                api_stop_timer(&menu_vformat_timer_id);
                menu_vformat_timer_id = api_start_timer("HDMD", 6000, menu_vformat_ui_handler);
                if (topmenu == (POBJECT_HEAD) & win_av_con)
                {
                    ap_send_msg_to_top_menu(topmenu, CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH, 0);
                }
            }
        }
#endif
#endif
#if defined(_C0200A_CA_ENABLE_) && !defined(_BUILD_UPG_LOADER_)
        else if (V_KEY_RED == vkey)
        {
            if (NULL == topmenu)
            {
                win_purchase_show_credit();
            }
        }
#endif
#ifdef ES_PLAYER_TEST
        ap_key_es_player(vkey); // for ES player
#endif
		//TPE 180725#1
		
		else if (V_KEY_REPEATAB == vkey)
		{

	#ifndef	_BUILD_OTA_E_
			#if 0
			extern UINT32 csa_init;
			if(csa_init != TRUE)
			{
				link_dmx_and_dsc_chanel(0);
				libc_printf("link_dmx_and_dsc_chanel!!!\n");
			}

			//play_clear_cw_stream();
			play_encrypted_cw_stream();
			#else
			libc_printf("test play scramble ts\n");
            char enc_cw_odd[8] = {0x30,0x31,0x32,0x33,0x14,0x15,0x16,0x17,};
            char enc_cw_even[8] = {0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,};
            char key1[16] = {0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
							 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,};
            char key2[16] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
							 0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,};	
		

			UINT16 pid_v = 513;
			UINT16 pid_a = 4112;
            // call these 2 function once
            gacas_device_init(1);
            gacas_create_key_samaphore();

            // these functions should be called for AP requesting
            gacas_set_key2(key2, AES_TYPE);
            gacas_set_key1(key1, AES_TYPE);
            gacas_set_rootkey_index(0);
            gacas_set_cw_type(AES_TYPE);

            gacas_create_stream_to_ce(pid_v, VIDEO_DSC);
            gacas_set_odd_even_key(DES_VIDEO, enc_cw_odd, enc_cw_even);

            gacas_create_stream_to_ce(pid_a, AUDIO_DSC);
            gacas_set_odd_even_key(AUDIO_DSC, enc_cw_odd, enc_cw_even);				
			#endif
	#endif		
		}
		
		//

	}
	
}

#ifdef _CAS9_VSC_ENABLE_
#ifdef VSC_PERSIONALIZE_TEST
void vsc_personalization_simulation(void)
{    
    static int vsc_p_inited = FALSE;
	if(!vsc_p_inited)
	{
	 UINT8 i = 0;
	 UINT32 num_cmd = 19;
	 UINT16 vrf_length = 0; // the length of verification code
	 UINT8 vrf_code[256] ={0}; // verification code
	 UINT16 status = 0;
	 const UINT16 CORRECT_STATUS = 0x0090; // 0x0090 is the correct response code of running the perso cmd
	 UINT8 persodata[134] = {0};
	 UINT32 vscdata_length[19] = {84,134,134,134,134,134,134,134,134,134,134,134,134,134,134,134,134,134,2};
UINT8 vscdata[19][134] = {
{
	0xA1,0x52,0x57,0x32,0xCB,0x43,0x9C,0x0E,
	0xB4,0x41,0x03,0xF9,0xAE,0xCD,0x11,0x61,
	0x3E,0x76,0xFF,0x41,0x79,0x70,0xA5,0xDC,
	0xE7,0x34,0xF1,0x32,0x71,0x7B,0xB2,0xAD,
	0xDC,0x5A,0xF0,0x70,0x75,0x34,0x13,0x34,
	0xB9,0x48,0x63,0x9C,0x52,0x39,0x90,0x1B,
	0xA7,0xC3,0x36,0x44,0xE1,0x9A,0xDE,0xBB,
	0x2F,0xBB,0x67,0xDA,0x4D,0x29,0x16,0x7E,
	0x6D,0x33,0x04,0xF7,0xC7,0x4C,0xA1,0xBA,
	0x4D,0x77,0x50,0xCF,0xFD,0x27,0x2A,0xC1,
	0xC0,0x66,0x1B,0xC4,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x0E,0x0A,
	0x88,0x2A,0xA5,0xD6,0xA8,0xAC,0xB5,0x1C,
	0x90,0x13,0xA0,0xAB,0x1B,0xB2,0x53,0x8F,
	0x33,0x37,0x1B,0x69,0xA3,0x2A,0xFF,0x7C,
	0xDD,0xE7,0x1B,0x1E,0x09,0x9F,0x76,0x05,
	0x29,0x04,0x55,0x6B,0xD5,0x02,0x8C,0x29,
	0x07,0x3E,0x2E,0xFB,0xE4,0x02,0x69,0x76,
	0xE5,0xE8,0x07,0xBC,0x6E,0x85,0x5D,0xD8,
	0x69,0x7B,0x04,0xA6,0xB6,0x95,0x86,0x33,
	0x43,0x24,0xDF,0x0E,0x8F,0x7D,0xC1,0xC8,
	0xB8,0xE7,0x94,0x3B,0x7F,0x1D,0xDC,0xC3,
	0xB8,0x30,0x77,0x42,0x56,0x68,0x77,0xA0,
	0xE2,0x4C,0xA1,0x23,0xCE,0xA7,0x78,0x42,
	0xC3,0xAE,0xD4,0xEA,0xF4,0xBF,0x8D,0xBC,
	0xDA,0xF3,0x6F,0x59,0xD5,0x57,0x77,0x9D,
	0xB5,0xDF,0xC9,0x9C,0x42,0xE1,0x49,0x33,
	0x7E,0xC0,0x2F,0x7B,0x50,0xB9,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x44,0xD4,
	0xE5,0x63,0x16,0x0C,0x45,0xF9,0x1E,0xF6,
	0x63,0x74,0xCA,0x44,0x13,0x1D,0x40,0xCF,
	0x75,0x76,0x40,0xF6,0x14,0x86,0x47,0x5A,
	0x8D,0xFE,0x43,0xDC,0xB3,0xB6,0xB4,0x11,
	0x5B,0x2C,0xFF,0x04,0x0C,0x67,0x08,0x5A,
	0xB7,0xCA,0x1B,0x82,0x53,0xBE,0x39,0x48,
	0xAA,0xEE,0x92,0x99,0xB6,0x61,0xE6,0x2F,
	0x7E,0xDF,0x20,0x1B,0xA9,0x58,0x53,0x35,
	0xB0,0x56,0x30,0x20,0xF4,0xC2,0x32,0xB6,
	0xB3,0x5F,0x9E,0xA4,0xA5,0x9C,0xE9,0xC5,
	0x0F,0x14,0xD9,0xA2,0xD8,0x57,0x12,0xDB,
	0xE8,0xEF,0x9D,0xC2,0xEF,0x62,0x6C,0xA2,
	0xAD,0x59,0x69,0x38,0x39,0xA5,0x85,0xD8,
	0xD3,0xB3,0x06,0x76,0x51,0x9D,0xDD,0x82,
	0xF5,0xFC,0x35,0x76,0xA3,0x0C,0xF5,0x43,
	0xAA,0x24,0x25,0x34,0x7D,0x22,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x37,0x4A,
	0x13,0xD3,0x1C,0x70,0x2E,0x88,0x08,0x92,
	0x6F,0xB3,0x88,0xBD,0x3B,0xB3,0xA8,0x22,
	0xF5,0x9C,0xF0,0xC3,0x0C,0x75,0x2D,0xD0,
	0x27,0x95,0x51,0x39,0x02,0x11,0xCA,0xCD,
	0xE7,0x78,0xD5,0x99,0xF0,0x59,0xEC,0x85,
	0x1D,0xD8,0x4C,0x81,0xD4,0x70,0x85,0x27,
	0x4D,0xC0,0x49,0xC0,0x86,0x25,0x25,0x98,
	0xF7,0xBC,0x28,0x2E,0xB8,0x24,0xFB,0xDF,
	0x6C,0xB4,0x58,0x96,0x01,0x44,0xDE,0x83,
	0x82,0x1A,0xF9,0x70,0x1D,0xC6,0x3D,0x3D,
	0x34,0x6F,0x23,0xA7,0x5F,0x92,0x40,0xCB,
	0xDD,0x59,0xC3,0x63,0x68,0xC5,0x01,0x3D,
	0x7D,0x60,0x7E,0x36,0x96,0x8E,0xAF,0x0B,
	0xA7,0x36,0x97,0xE6,0xAB,0x50,0x14,0xFE,
	0x02,0x1B,0xCD,0xDC,0x5A,0xA8,0xED,0xB8,
	0xA6,0x6F,0x00,0xB4,0x96,0x6C,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x85,0x8A,
	0x18,0x80,0x5B,0x71,0x44,0xF1,0x23,0x2B,
	0xA7,0xD4,0xAF,0x13,0x47,0xD2,0xC5,0x20,
	0xAD,0xC7,0x9A,0x98,0x31,0x25,0x9B,0xD2,
	0x38,0x6B,0xC0,0xCD,0x4F,0x4C,0x00,0x19,
	0xBE,0x9F,0xED,0xC4,0x5B,0xEA,0xF4,0xFC,
	0x76,0x52,0x64,0x22,0x18,0x92,0x72,0xB2,
	0xA0,0xAB,0xC5,0x37,0x80,0x48,0xA7,0xE3,
	0x4F,0x82,0x6D,0x04,0x36,0x14,0xD5,0xCE,
	0xF8,0x79,0xD5,0x62,0x76,0xA2,0xF5,0x25,
	0x6D,0x5F,0x08,0xF5,0x05,0xA7,0x35,0x8A,
	0x1E,0x5B,0x91,0xA7,0x12,0x0D,0xC2,0x41,
	0x1E,0x11,0xC9,0x2C,0x42,0x6F,0xB0,0x2F,
	0x4A,0x2F,0x6E,0xEF,0xE2,0xA3,0x39,0x94,
	0x5B,0x5D,0x94,0x0E,0xC2,0xBF,0x3C,0x86,
	0x8E,0x6E,0x0F,0xD6,0x5B,0x7D,0x07,0xB4,
	0x82,0xE5,0xA0,0x5C,0x1E,0x00,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x0C,0x7A,
	0xF7,0x82,0x0D,0xC4,0x4C,0x5F,0xCF,0x93,
	0xD4,0xE7,0x57,0xD0,0x8F,0x06,0x52,0x4E,
	0x38,0x18,0x1D,0x0B,0xA3,0xD5,0xFD,0xB2,
	0x78,0xAF,0xCA,0xD5,0xD0,0x54,0x1F,0xA4,
	0xC0,0x8D,0xF9,0x42,0xB8,0xC5,0xD6,0xC8,
	0x4D,0x26,0x6A,0xD7,0xF1,0xBB,0x07,0x26,
	0x5E,0x75,0xEB,0x5C,0x74,0x6D,0xB8,0x1A,
	0x35,0xD5,0xBA,0x69,0xDF,0x1C,0x4F,0x55,
	0x9B,0xD8,0x00,0xA3,0x6C,0xB5,0x5F,0x3F,
	0x9F,0x73,0xBF,0xAD,0x54,0x1E,0x3E,0xF4,
	0x80,0x99,0x4B,0x8E,0x0B,0x12,0xCF,0xE5,
	0xA8,0x79,0xB2,0xE0,0x82,0xF2,0x12,0x3E,
	0x42,0x05,0x34,0x3F,0x2E,0xC5,0x68,0x3B,
	0xC3,0x52,0x87,0x43,0x50,0xF8,0xD4,0xE7,
	0x48,0xB0,0x72,0x3A,0x7C,0x88,0xD7,0x1F,
	0xA0,0xDA,0x70,0x61,0xD5,0x71,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x0A,0x49,
	0xD3,0xA6,0xC7,0x8C,0x0B,0x7E,0x59,0x2F,
	0x22,0x62,0x0F,0xD6,0xB4,0xAB,0x50,0xD2,
	0x45,0x28,0x93,0xE8,0xB5,0xE2,0xCC,0xCE,
	0xB2,0xBA,0xA5,0x7C,0x27,0xF4,0xA7,0xBB,
	0x0E,0xA0,0x79,0x5A,0x25,0x19,0xC0,0xD0,
	0x49,0x27,0x4F,0x65,0x3E,0x9E,0xA2,0xF4,
	0x70,0x62,0x39,0x32,0xAC,0x74,0x75,0x07,
	0x1F,0xCE,0x7D,0xD5,0xAA,0xEB,0xB4,0x9B,
	0x12,0x44,0x87,0x87,0x11,0x03,0x37,0xB3,
	0x9D,0xD8,0x74,0x63,0x63,0x4E,0x35,0x75,
	0x12,0x3B,0x1B,0x8D,0xC7,0x1D,0x33,0x72,
	0xBE,0x3F,0x7F,0x45,0x44,0x8A,0xED,0x02,
	0xA3,0x7B,0xFB,0x45,0x16,0xD8,0x5C,0xFC,
	0x44,0x92,0x19,0xFF,0x45,0xDD,0xF6,0xDA,
	0x3B,0x93,0x8A,0x9A,0x96,0x51,0x43,0xB9,
	0x30,0x2C,0x85,0xC4,0x42,0x91,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x4C,0xE3,
	0x96,0xCD,0x8F,0x8D,0x66,0x23,0x75,0x63,
	0x6B,0x72,0xC3,0x01,0x6A,0x01,0x58,0xD8,
	0x2B,0x61,0x9B,0x5A,0x0A,0xC8,0x1D,0xC0,
	0xB7,0xDD,0x31,0x1F,0xB5,0x75,0x60,0x32,
	0xF8,0x72,0x51,0x64,0xEE,0xA3,0x2F,0xB6,
	0xCD,0xA6,0x9A,0x16,0xA5,0x09,0x79,0x82,
	0x2D,0xA5,0x38,0x24,0x5E,0x84,0xF8,0x18,
	0xDF,0x5C,0xCE,0x4F,0x4B,0x6E,0x01,0xFC,
	0xA5,0xBB,0x08,0x5F,0xB0,0x36,0x82,0x4B,
	0xD5,0x55,0xF3,0x0F,0xBB,0x9E,0x31,0xD1,
	0x9A,0x9A,0x36,0x17,0xDB,0x2D,0x2D,0xFD,
	0xDF,0x83,0xE3,0xDE,0x27,0x0A,0x27,0xA5,
	0xA7,0xB0,0xB6,0x0F,0x25,0x06,0x67,0x47,
	0x83,0x6C,0xA0,0x5D,0x04,0xA0,0xDB,0x7A,
	0x33,0x66,0xE8,0x20,0x8C,0x93,0x92,0x14,
	0x7A,0x99,0xFB,0xC0,0x0E,0xD0,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x0E,0xF4,
	0x59,0x36,0x99,0x2D,0xB2,0xF6,0x12,0xF9,
	0x35,0x93,0xB1,0xE3,0x25,0xD0,0xD9,0x9D,
	0x61,0xAD,0x30,0xD3,0x81,0x21,0xAE,0x01,
	0x63,0x31,0x23,0x3C,0xBE,0x06,0x1C,0x85,
	0x43,0xBE,0x57,0x34,0x12,0x56,0xF7,0xB0,
	0x1B,0x69,0x0F,0x81,0x16,0x05,0x65,0xCA,
	0x25,0x06,0x71,0x86,0xA3,0x68,0xC2,0xD0,
	0x2C,0x38,0x1B,0xF0,0x81,0xF0,0x1A,0xC0,
	0xEB,0x4A,0xB0,0xB8,0x27,0x95,0x4C,0xB0,
	0x7A,0xED,0xD6,0x57,0xD6,0x42,0xC8,0x70,
	0x1A,0xBA,0xBD,0x9E,0xB8,0xC0,0x85,0x6F,
	0x93,0x14,0x49,0xB6,0xF3,0xCF,0x47,0x72,
	0xF9,0x1C,0x37,0xEB,0x1D,0x25,0x20,0xCF,
	0x3D,0x38,0x6A,0x67,0xD4,0x93,0x06,0x32,
	0x99,0x13,0x9D,0x53,0xD0,0xFF,0x47,0xCE,
	0xE9,0x35,0x85,0xE0,0x24,0x4F,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x4B,0xA3,
	0x10,0xE2,0x84,0xCC,0x67,0xF2,0x3F,0x04,
	0x11,0xF2,0xF2,0x79,0xB5,0x72,0xAD,0x86,
	0x30,0x4A,0x61,0xBD,0x53,0x79,0x61,0xE9,
	0x2C,0x7A,0x00,0x7C,0x3A,0x3A,0x35,0xBD,
	0xE3,0xC2,0xAD,0xFB,0x3E,0x42,0x2B,0x99,
	0x54,0x3D,0xF5,0x3C,0x3B,0xE4,0x82,0x33,
	0x14,0xCC,0x15,0x64,0x6A,0x69,0x32,0xFE,
	0x0F,0x79,0xA8,0x38,0x54,0x6A,0xFF,0x93,
	0xC0,0x79,0xF3,0x22,0xD6,0x4F,0xEC,0x6A,
	0x44,0xF0,0xB0,0x82,0x14,0x7C,0x89,0xC9,
	0xDB,0x31,0x1A,0x82,0xCC,0xB8,0x6C,0xB1,
	0x76,0x7B,0x4E,0x53,0xD4,0x14,0x8F,0xC7,
	0x39,0xFD,0x47,0x1A,0x64,0x25,0x02,0x1E,
	0x40,0x64,0xE4,0x07,0x23,0x73,0x9E,0x95,
	0x7F,0xF6,0xF2,0xA2,0x9D,0xFB,0xBB,0x27,
	0x1E,0x8D,0x59,0xDE,0xEE,0x8C,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x2B,0xA9,
	0x53,0x87,0x80,0xE9,0x52,0xAA,0x77,0x49,
	0x14,0x5C,0x8B,0xE6,0x07,0xF9,0x67,0xEE,
	0x1C,0xD9,0xED,0xDD,0x90,0x4A,0xBA,0x12,
	0x1F,0x00,0xBC,0x52,0xE7,0x95,0x6E,0xD3,
	0xEB,0x34,0x3C,0xD9,0x41,0x47,0x70,0x4F,
	0x2C,0x8C,0x67,0xFE,0x28,0xC7,0xD1,0xFF,
	0x17,0x68,0x1E,0x35,0x1B,0x55,0xA6,0xD8,
	0x91,0xAE,0x6A,0x8E,0x72,0x2D,0x94,0x25,
	0x9C,0xA2,0xB8,0x1B,0xBE,0x9A,0xBF,0x46,
	0x04,0x10,0xC2,0x8A,0xDE,0x83,0x2F,0x44,
	0x0A,0x92,0xF6,0x3C,0x8A,0xAF,0xC2,0x96,
	0x79,0xD6,0x1E,0x23,0xC6,0xF4,0x37,0x87,
	0x9A,0x4A,0x17,0xEB,0x83,0xD4,0xB8,0x66,
	0x62,0xF4,0x81,0xA6,0x4E,0x09,0x3C,0x37,
	0x60,0x5D,0x6B,0x24,0xCF,0x1A,0x71,0xF4,
	0x1E,0x3F,0xAB,0x6B,0x16,0xD5,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x66,0x36,
	0xC6,0x78,0x27,0xEB,0x21,0xAC,0x7F,0x27,
	0x2C,0x51,0x24,0x53,0x08,0x81,0x70,0xC2,
	0x07,0x18,0x81,0xD0,0xDE,0xB6,0x3F,0x9B,
	0xD0,0x73,0x10,0x78,0x4F,0x0A,0xEA,0x9D,
	0x83,0x5D,0xD4,0x6D,0x04,0x48,0x41,0x30,
	0xDB,0xA5,0x88,0x2D,0x88,0x22,0x51,0x2A,
	0x47,0xF1,0xBE,0xCA,0x1B,0x82,0xF5,0x24,
	0x81,0xD2,0xCE,0xAF,0xFF,0x1D,0x18,0x60,
	0x56,0xD2,0xFF,0x5E,0x3D,0x65,0xD2,0x05,
	0x88,0x44,0x8C,0x33,0x5A,0xE9,0xBB,0x19,
	0x5B,0x98,0x0A,0x25,0x20,0xBF,0x95,0xA4,
	0xC6,0xB2,0x95,0x61,0xDF,0x99,0xF4,0xE4,
	0x48,0xFE,0x9E,0x5E,0xCD,0x9A,0x31,0x21,
	0x1F,0x4E,0xD6,0x54,0x93,0x3A,0xBA,0x8C,
	0xCA,0xD7,0x6E,0xAE,0x40,0x1B,0x48,0xF4,
	0xA2,0xEC,0xED,0x7E,0x12,0x66,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x5A,0x50,
	0x9E,0x1E,0xE2,0x1E,0x08,0x5E,0xAC,0x10,
	0xCB,0x51,0x5B,0xB1,0xBF,0x45,0xB9,0x14,
	0x4A,0xCA,0xE7,0xFF,0x08,0x49,0x2F,0x6F,
	0x50,0x70,0x1F,0xD7,0x12,0x33,0x85,0x5F,
	0x57,0x0E,0xBA,0x13,0xA7,0xDD,0x8D,0x4A,
	0x34,0x93,0xAF,0x34,0x13,0x1F,0x85,0xEC,
	0xD7,0x16,0x40,0xDC,0x4C,0xB6,0x90,0xF6,
	0xE3,0x7A,0x3A,0xA0,0xD4,0x60,0xA0,0xEB,
	0x92,0xD9,0x2B,0x50,0xC8,0xD1,0x14,0xD6,
	0x70,0x80,0xE9,0xD1,0xDA,0x6F,0x5E,0x12,
	0x60,0xB1,0x38,0xDE,0x22,0xE6,0xCF,0x8F,
	0x3E,0x08,0x79,0x0C,0x27,0x32,0x2F,0xDA,
	0x44,0x96,0x36,0x04,0x83,0x54,0x97,0xAF,
	0x76,0x1C,0x49,0xF3,0xC4,0xF8,0xFF,0x0E,
	0xA2,0xEE,0x35,0x60,0x84,0xF1,0x9B,0x5E,
	0xD6,0x2A,0x70,0x7E,0xC3,0x2D,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x7C,0xC8,
	0xC4,0xB7,0xDF,0xCE,0xEF,0xF2,0x7A,0x26,
	0x92,0x47,0x6D,0xF9,0xFA,0xC4,0x62,0x42,
	0xB4,0x5C,0xFA,0x46,0xE3,0xA5,0xF8,0x58,
	0x61,0x00,0x5B,0xBB,0x25,0x66,0x38,0x4C,
	0x0F,0xDF,0xE8,0x0C,0x70,0x45,0xCF,0xD7,
	0xAE,0x24,0x5F,0xBC,0xEF,0x7D,0x48,0x58,
	0x0A,0x10,0x81,0x41,0xB8,0xBD,0x79,0x07,
	0x0D,0x2E,0x92,0x58,0x29,0xC0,0xC2,0xC9,
	0x78,0x62,0xF2,0x34,0x44,0xB8,0x00,0x80,
	0x45,0x39,0xA2,0xAA,0x68,0x3A,0xC4,0x22,
	0x1D,0x57,0x8A,0x9C,0x57,0x87,0x06,0xD6,
	0x89,0x7A,0x49,0x64,0x2E,0x58,0x46,0x8F,
	0xDE,0xF7,0x9C,0x59,0x60,0xD1,0x6C,0x7D,
	0x40,0x82,0x3C,0x65,0x3D,0xFF,0x9F,0xD3,
	0x46,0xCC,0xDD,0x53,0x70,0xA5,0x38,0xC3,
	0xF2,0x1B,0xD9,0xB7,0xF5,0x68,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x3B,0x40,
	0x11,0xD9,0x8F,0x29,0xBF,0x41,0x2C,0xB7,
	0x17,0x31,0x67,0xF1,0x40,0xC6,0xEB,0xEA,
	0xDA,0x68,0x97,0x4A,0x6E,0xCF,0x5D,0xDC,
	0x58,0xD8,0xDC,0xBF,0x54,0x97,0x25,0x24,
	0x84,0x43,0xCB,0xC9,0x0F,0x4C,0xD2,0x90,
	0xB0,0xA6,0x0C,0x95,0xBC,0x7D,0x04,0x56,
	0x59,0xE5,0xBC,0x81,0x4F,0x95,0x22,0x3B,
	0x22,0xF1,0xC6,0x67,0x4C,0x83,0x25,0x78,
	0x36,0xBF,0xE9,0xC7,0xED,0xCA,0x57,0x98,
	0x56,0xBC,0xC2,0xA2,0xB3,0x4D,0xA5,0xC5,
	0xF6,0x84,0x28,0xC9,0x59,0xA1,0x40,0xA2,
	0x52,0xC1,0x2B,0x37,0xA7,0x77,0x24,0x5D,
	0x0F,0xA9,0xBF,0x16,0x81,0x07,0xAA,0x57,
	0x7E,0x8F,0xE3,0x64,0xF2,0x57,0xA3,0x0E,
	0x98,0x65,0x0D,0x0D,0xC9,0x42,0x68,0x70,
	0xD4,0xBC,0x88,0x6D,0x3C,0x14,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x24,0x67,
	0x19,0x03,0xF4,0xD4,0x0E,0x04,0x21,0xD3,
	0xF5,0xDB,0x1B,0x26,0x5A,0xFB,0x30,0x97,
	0x93,0xEC,0x81,0x2F,0xCA,0xF7,0x43,0xD7,
	0xDE,0xE7,0x59,0xEF,0x3B,0x9D,0x5E,0xED,
	0x37,0xA9,0x34,0xF2,0xC4,0x1C,0x80,0xBA,
	0x54,0xEA,0xA4,0xFA,0xDB,0xFB,0x37,0xF6,
	0xF2,0x8D,0x9B,0xE5,0xD5,0x2C,0x00,0x6D,
	0x98,0xBF,0xC8,0xF7,0x58,0x2E,0xD8,0xCE,
	0x91,0xF5,0xBE,0x2E,0xC9,0x3F,0xD0,0x52,
	0xD8,0xEA,0xD5,0x1D,0xEB,0xC8,0x34,0xFE,
	0x15,0x89,0x60,0xCB,0xAB,0x3E,0x4E,0xA2,
	0xDD,0x21,0x83,0x5D,0xF0,0xA6,0x47,0x95,
	0x7A,0xCB,0x6C,0x8E,0xE7,0x39,0xE2,0x06,
	0x9E,0xE1,0x5C,0x58,0xC7,0xB5,0x46,0x63,
	0x51,0x1D,0x19,0xC1,0xE1,0x9B,0x12,0x27,
	0x8E,0x57,0xAE,0xD1,0x70,0xEF,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x1D,0xFA,
	0x56,0x3F,0xEE,0xE7,0x63,0xFF,0xC6,0x92,
	0x61,0x73,0x29,0xC0,0x41,0xB9,0x9E,0x82,
	0x62,0x44,0x2A,0xFF,0xDB,0xA6,0x8F,0xD2,
	0xE3,0x8D,0x4B,0x30,0xAF,0x26,0xB4,0x83,
	0x57,0xC4,0xC1,0x3E,0x47,0x39,0x2F,0x3C,
	0x79,0x5C,0x28,0xE0,0x8A,0x2F,0xE1,0x97,
	0x60,0x26,0xD9,0x27,0x1E,0x28,0x8C,0x47,
	0x89,0x6A,0xF2,0x46,0x04,0x78,0x08,0xF4,
	0xAE,0x82,0x6E,0x5E,0xA4,0xAB,0x78,0x86,
	0xF4,0x28,0x5C,0xF1,0x63,0xB5,0x11,0xA2,
	0xF5,0x63,0xF4,0x39,0xD8,0x13,0xA7,0x74,
	0xBC,0xA4,0xC3,0xA7,0x4E,0x67,0xD5,0xE6,
	0x69,0x53,0xEE,0x20,0xAF,0x46,0x33,0xC3,
	0xDF,0x4C,0x7D,0xA3,0x2B,0x87,0x67,0x11,
	0xEB,0x34,0xBE,0x6F,0x46,0x59,0xD0,0x52,
	0x52,0xF1,0x20,0x3B,0x85,0x14,
},
{
	0xA2,0x84,0xC0,0x82,0x00,0x03,0x42,0xEB,
	0x9B,0x97,0xEE,0xB6,0x76,0x1E,0xDC,0x85,
	0xD3,0x91,0x5C,0x54,0x45,0xA8,0xBD,0x50,
	0x1A,0xD7,0x5B,0x8E,0xD7,0x27,0x0F,0xCB,
	0xEA,0x02,0xB6,0xAB,0xB9,0x53,0xE3,0x9B,
	0x50,0x8E,0x2E,0xD2,0x0E,0xE9,0xAD,0x96,
	0x79,0xAE,0x82,0x47,0x3A,0x09,0xBA,0x61,
	0x00,0x5E,0xC9,0x12,0x6B,0x67,0xC9,0x25,
	0x54,0x02,0x7D,0xD4,0xC6,0x17,0x05,0x6E,
	0x98,0xC9,0xE3,0x04,0xE5,0xB9,0x85,0xE0,
	0xC1,0x18,0xF9,0x43,0x7B,0x44,0x66,0x71,
	0x57,0xD8,0x86,0xF1,0xD3,0xDA,0x7B,0xCD,
	0xC7,0xBB,0xAD,0xC2,0xDA,0x35,0x63,0xE1,
	0x31,0x1E,0x90,0x7F,0x7E,0x2B,0x4A,0x26,
	0xFE,0xE9,0xCA,0x6E,0xB1,0x87,0x95,0xDB,
	0xD4,0x99,0x5C,0xEF,0xC9,0x30,0x32,0xB0,
	0xA8,0x61,0xE5,0xA4,0xCE,0xBA,
},
{
	0xC0,0x00,
},
};

	 libc_printf("Brook test vsc personalize -Start!\n");
	 for(i=0;i<num_cmd;i++) // execute perso cmd
	 {   
	  MEMCPY(persodata,&vscdata[i][0],vscdata_length[i]);   
	  vsc_lib_dispatch_cmd(0, persodata, vscdata_length[i], vrf_code, &vrf_length, &status);
	  if(CORRECT_STATUS != status)
	  {
	   libc_printf("vsc_lib_dispatch_cmd error\n");
	  }
	 }
        libc_printf("Brook test vsc personalize - Done!\n"); 
        vsc_p_inited = TRUE;
	}
}
#endif
#endif

void ap_key_commsg_proc(UINT32 msg_type, UINT32 msg_code)
{
    POBJECT_HEAD                topmenu = NULL;
    PRESULT                     proc_ret = 0;
    UINT32                      key = 0;
    UINT32                      vkey = 0;
    UINT32                      prevkey = 0xFFFFFFFF;
    UINT32                      osd_msg_type = 0;
    UINT32                      osd_msg_code = 0;
    UINT32                      start = 0;
    UINT32                      next_start = 0;
    RET_KEY_PROC_E              key_ret = RET_KEY_NONE;

    topmenu = menu_stack_get_top();

#ifdef _INVW_JUICE
    if (RET_KEY_POP_PRE_MENU == invw_status_check(topmenu))
    {
        proc_ret = PROC_LEAVE;
        goto POP_PRE_MENU;
    }
#endif

    if (msg_type <= CTRL_MSG_TYPE_KEY)
    {
        key = msg_code;
        if (CTRL_MSG_SUBTYPE_KEY == msg_type)
        {
            osd_msg_type = (MSG_TYPE_KEY << 16);
HK_TO_VKEY:
            next_start = ap_hk_to_vk(start, key, &vkey);
            key_ret = ap_subkey_proc(&key, &vkey, &prevkey, &start, &next_start);
            if (RET_KEY_RETURN == key_ret)
            {
                return;
            }
            if (RET_KEY_UNKOWN_KEY_PROC == key_ret)
            {
                proc_ret = PROC_PASS;
                goto UNKOWN_KEY_PROC;
            }
        }
        else        /*CTRL_MSG_SUBTYPE_KEY_UI,abandon type*/
        {
            vkey = key;
            next_start = ap_vk_to_hk(0, vkey, &key);
        }
#ifdef PLAY_TEST
        if((SYS_STATE_TEST_PLAY == api_get_system_state())&& !url_play_response_key(vkey))
        {
            return;
        }
#endif
        if(V_KEY_MOVE == vkey)
        {
            #ifdef VSC_PERSIONALIZE_TEST
            #ifdef _CAS9_VSC_ENABLE_
            vsc_personalization_simulation();
            #endif
            #endif
        }

#ifdef AUTO_CC_TEST
        handle_vkey_msg(vkey);
#endif
        osd_msg_code = key;
#ifdef DVR_PVR_SUPPORT
        key_ret = ap_key_pvr_proc(key, vkey);
        if (RET_KEY_RETURN == key_ret)
        {
            return ;
        }
#endif
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
#ifdef SELF_TEST_ENABLE
        self_test_main(vkey);
#endif
#endif
#ifdef PIP_PNG_GIF
    if ((api_get_system_state() == SYS_STATE_NORMAL) && (menu_stack_get_top() == (POBJECT_HEAD)NULL))
    {
        app_ad_pic_processkey(vkey);
    }
#endif
#ifdef APP_PIC_BG_AUX 
    if (menu_stack_get_top() == (POBJECT_HEAD)&g_win2_mainmenu) {
        app_ad_pic_processkey2(vkey);
    }
#endif

    }
    else
    {
        // other msg process
        key_ret = ap_key_nonkey_msg_proc(msg_type, msg_code, topmenu, &proc_ret, &osd_msg_type, &osd_msg_code);

        if (RET_KEY_RETURN == key_ret)
        {
            return;
        }
        if (RET_KEY_POP_PRE_MENU == key_ret)
        {
            goto POP_PRE_MENU;
        }
        if (RET_KEY_CEHCK_PROC_RETURN == key_ret)
        {
            goto CEHCK_PROC_RETURN;
        }
    }
    proc_ret = PROC_PASS;
    if (topmenu != NULL)
    { // now has some menu displaying, forward msg to menu
MENU_KEY_PROC:
        proc_ret = osd_obj_proc(topmenu, osd_msg_type, osd_msg_code, osd_msg_code);
#ifdef _INVW_JUICE
        if (proc_ret == PROC_PASS)
        {
            ap_key_invw_volume_proc(key, vkey);
        }
#endif
CEHCK_PROC_RETURN :
        if (PROC_LEAVE == proc_ret)
        {
POP_PRE_MENU:
            key_ret = ap_key_pop_pre_menu(&proc_ret);

            if (RET_KEY_RETURN == key_ret)
            {
                return;
            }

            if (RET_KEY_POP_PRE_MENU == key_ret)
            {
                goto POP_PRE_MENU;
            }
        }
        else if ((CTRL_MSG_SUBTYPE_CMD_STO == msg_type) && (USB_STATUS_OVER == msg_code))
        {
#if (!defined(_BUILD_OTA_E_) || defined(_BUILD_USB_LOADER_))
#ifndef DISK_MANAGER_SUPPORT
            storage_dev_mount_hint(1);
#endif
#endif
        }

#ifdef AUDIO_DESCRIPTION_SUPPORT
#ifndef _BUILD_OTA_E_

        else if (msg_type == CTRL_MSG_SUBTYPE_STATUS_AUDIO_DESCRIPTION)
        {
            POBJECT_HEAD    topmenu = NULL;

            topmenu = menu_stack_get_top();
            if ((topmenu != (POBJECT_HEAD) & g_win2_mainmenu)
            && (topmenu != (POBJECT_HEAD) & g_win_record)
            && (topmenu != (POBJECT_HEAD) & g_win_usb_filelist))
            {
                aud_desc_show_hint(msg_code);
            }
        }
#endif		
#endif
#if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        else
        {
            if ((CA_MMI_PRI_00_NO == get_mmi_showed()) && (get_mmi_msg_cnt() > 0))
            {
                ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
                MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: no menu and no msg show; code:%d\n", 0);
            }
        }
#endif
    }
    else
    {   // no menu on the top
        if (msg_type <= CTRL_MSG_TYPE_KEY)
        {
            key_ret = ap_key_proc_no_menu(&topmenu, key, vkey, osd_msg_type, osd_msg_code, &proc_ret);
            if (RET_KEY_UNKOWN_KEY_PROC == key_ret)
            {
                goto UNKOWN_KEY_PROC;
            }
            if (RET_KEY_HK_TO_VKEY == key_ret)
            {
                goto HK_TO_VKEY;
            }
            if (RET_KEY_MENU_KEY_PROC == key_ret)
            {
                goto MENU_KEY_PROC;
            }
        }
        else if ((CTRL_MSG_SUBTYPE_CMD_STO == msg_type) && (USB_STATUS_OVER == msg_code))
        {
#if (!defined(_BUILD_OTA_E_) || defined(_BUILD_USB_LOADER_))
#ifndef DISK_MANAGER_SUPPORT
            storage_dev_mount_hint(1);
#endif
#endif
        }

#ifdef AUDIO_DESCRIPTION_SUPPORT
#ifndef _BUILD_OTA_E_

        else if (msg_type == CTRL_MSG_SUBTYPE_STATUS_AUDIO_DESCRIPTION)
        {
            topmenu = menu_stack_get_top();
            if ((topmenu != (POBJECT_HEAD) & g_win2_mainmenu) && (topmenu != (POBJECT_HEAD) & g_win_record))
            {
                aud_desc_show_hint(msg_code);
            }
        }
#endif		
#endif
    }

UNKOWN_KEY_PROC:
    if (PROC_PASS == proc_ret)
    {
        ap_key_unknown_proc(msg_type, msg_code, key, vkey);
    }
}
