/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: menus_root.c
*
*    Description: define sturct rootwinpool, and functions about finding meuns.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libosd/osd_lib.h>

#include "menus_root.h"
#include "win_com.h"
#include "ctrl_key_proc.h"

static rootmenuhandle_t rootwinpool[] /*__attribute__((section(".flash")))*/=
{
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_MENU,  FALSE,(POBJECT_HEAD)&g_win2_mainmenu},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_UP,     TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_DOWN,   TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_C_UP,   TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_C_DOWN, TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_P_UP,   TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_P_DOWN, TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_F_UP,   TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_F_DOWN, TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_TVRADIO,TRUE,   (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_INFOR,  FALSE,  (POBJECT_HEAD)&g_win2_progname},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_RECALL, FALSE,  (POBJECT_HEAD)&g_win2_progname},

    {CTRL_MSG_SUBTYPE_KEY, V_KEY_0,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_1,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_2,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_3,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_4,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_5,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_6,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_7,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_8,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_9,       TRUE, (POBJECT_HEAD)&g_win2_proginput},
//    {CTRL_MSG_SUBTYPE_KEY, V_KEY_YELLOW,TRUE, (POBJECT_HEAD)&g_win2_proginput},
        
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_LEFT,    TRUE, (POBJECT_HEAD)&g_win2_volume},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_RIGHT,   TRUE, (POBJECT_HEAD)&g_win2_volume},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_V_DOWN,  TRUE, (POBJECT_HEAD)&g_win2_volume},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_V_UP,    TRUE, (POBJECT_HEAD)&g_win2_volume},

   {CTRL_MSG_SUBTYPE_KEY,V_KEY_ENTER,FALSE,(POBJECT_HEAD)&g_win2_light_chanlist},
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_LIST,FALSE,(POBJECT_HEAD)&g_win2_light_chanlist},
   {CTRL_MSG_SUBTYPE_KEY,V_KEY_EPG,FALSE,(POBJECT_HEAD)&g_win2_epg},

//#ifndef HDTV_SUPPORT
//#if (SYS_CHIP_MODULE !=ALI_S3602)
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_ZOOM, FALSE,   (POBJECT_HEAD)&g_win_zoom},
//#endif
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_FIND, FALSE,   (POBJECT_HEAD)&g_win_find},
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_AUDIO,FALSE,   (POBJECT_HEAD)&g_win2_audio},
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_BLUE,FALSE,   (POBJECT_HEAD)&g_win2_audio},


    {CTRL_MSG_SUBTYPE_KEY,V_KEY_SLEEP,FALSE,   (POBJECT_HEAD)&g_win_sleeptimer},
    //{CTRL_MSG_SUBTYPE_KEY,V_KEY_EPG,  FALSE,   (POBJECT_HEAD)&g_win2_epg},

#if(SUBTITLE_ON == 1)
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_SUBTITLE,FALSE,(POBJECT_HEAD)&g_cnt_subtitle},
#endif

#ifdef CC_BY_OSD
    {CTRL_MSG_SUBTYPE_KEY,V_KEY_BLUE,FALSE, (POBJECT_HEAD)&g_win_closedcaption},
#endif
#if(ISDBT_CC == 1)
    {CTRL_MSG_SUBTYPE_KEY,   V_KEY_RED,    FALSE,   (POBJECT_HEAD)&g_win_cc},
#endif

    {CTRL_MSG_SUBTYPE_KEY,   V_KEY_SAT, FALSE,(POBJECT_HEAD)&g_win2_favlist},
    {CTRL_MSG_SUBTYPE_KEY,   V_KEY_FAV, FALSE,(POBJECT_HEAD)&g_win2_favlist},

#ifdef AV_DELAY_SUPPORT
    {CTRL_MSG_SUBTYPE_KEY,   V_KEY_MP,  FALSE,(POBJECT_HEAD)&win_av_delay_con},
#else
#if (SYS_CHIP_MODULE !=ALI_S3602)
    {CTRL_MSG_SUBTYPE_KEY,   V_KEY_MP,  FALSE,(POBJECT_HEAD)&g_cnt_multiview},
#endif
#endif

#ifdef HDTV_SUPPORT
#if 0//def SD_PVR
   {CTRL_MSG_SUBTYPE_KEY,V_KEY_VIDEO_FORMAT,FALSE,(POBJECT_HEAD)&g_win_palntsc},
#else
 {CTRL_MSG_SUBTYPE_KEY,V_KEY_VIDEO_FORMAT,FALSE,(POBJECT_HEAD)&g_win_hdtv_mode},
#endif
#endif

#ifdef DVR_PVR_SUPPORT
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_RECORD,  TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_FF,      TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_FB,      TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_SLOW,    TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_B_SLOW,  TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_PLAY,    TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_PAUSE,   TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_STOP,    TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_PREV,    TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_NEXT,    TRUE,  (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_PVR_INFO,FALSE, (POBJECT_HEAD)&g_win_pvr_ctrl},
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_DVRLIST, FALSE, (POBJECT_HEAD)&g_win_record},
#endif
#ifdef USB_MP_SUPPORT
#ifndef _NV_PROJECT_SUPPORT_
  {CTRL_MSG_SUBTYPE_KEY,V_KEY_FILELIST,FALSE,(POBJECT_HEAD)&g_win_usb_filelist},
#endif
#endif
#ifdef MULTIFEED_SUPPORT
    {CTRL_MSG_SUBTYPE_KEY, V_KEY_PIP_LIST,FALSE,(POBJECT_HEAD)&g_win_multifeed},
#endif
#ifdef PLAY_TEST
        {CTRL_MSG_SUBTYPE_KEY, V_KEY_SWAP, FALSE, (POBJECT_HEAD)&g_win_play},
#endif 
};

/* for checking hot-key is valid or not, for special using */
static BOOL menus_check_key(UINT32 __MAYBE_UNUSED__ msgType, UINT32 msgCode, 
    BOOL*bFlag, POBJECT_HEAD *winhandle)
{
    UINT32 vkey = 0; 	
    BOOL ret = TRUE;
    PVR_HANDLE pback_handle = 0,tms_handle = 0;       
    pvr_play_rec_t *pvr_info = NULL;
    
    ap_hk_to_vk(0, msgCode, &vkey);
    pvr_info  = api_get_pvr_info();      
    
    pback_handle = pvr_info->play.play_handle;
    tms_handle= pvr_info->tms_r_handle;
    if((0 != pback_handle) && (0 == tms_handle) && (V_KEY_INFOR == vkey))
    {
        /* during playback, we skip INFO key handle. */                
        *bFlag = FALSE;
        *winhandle = NULL;
        return FALSE;
    }
    
    return ret;
}
extern 	void gaui_deal_all(void);
BOOL menus_find_root(UINT32 msg_type,UINT32 msg_code,BOOL *b_flag, POBJECT_HEAD *winhandle)
{
    int i = 0;
    int n = 0;
    rootmenuhandle_t *roothandle = NULL;
    UINT32  vkey = 0;
    BOOL ckey = TRUE;    

    if((NULL == b_flag) || (NULL == winhandle))
    {
        return FALSE;
    }

    ckey = menus_check_key(msg_type,msg_code, b_flag, winhandle);
    if(FALSE == ckey)
    {
        return FALSE;
    }
    
    	ap_hk_to_vk(0, msg_code, &vkey);
#ifdef SUPPORT_GACAS
	gaui_deal_all();
//	gacas_ui_sem_dialog_wait();
//	n = gacas_ui_update_dialog(vkey);
//	gacas_ui_sem_dialog_signal();
//	if(n == 0)
//		return FALSE;
#endif
#ifdef _INVW_JUICE
    if (V_KEY_MENU == vkey)
    {
        ap_osd_int_restore();
    }
#endif

    n = sizeof(rootwinpool)/sizeof(rootwinpool[0]);
    for(i=0;i<n;i++)
    {
        roothandle = &rootwinpool[i];
        if((roothandle->msg_type == msg_type) && (roothandle->msg_code == vkey))
        {
            *b_flag = roothandle->process_msg;
            *winhandle = roothandle->root;
            return TRUE;
        }
    }

    *winhandle = NULL;

    return FALSE;
}

