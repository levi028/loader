/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_volume.c
*
*    Description: volume menu.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif

#include <api/libclosecaption/lib_closecaption.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_mute.h"
#include "win_volume.h"
#include "win_filelist.h"
#include "control.h"
#include "ctrl_key_proc.h"
#ifdef CEC_SUPPORT
#include "cec_link.h"
#include <api/libcec/lib_cec.h>
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
//extern CONTAINER    g_win_volume;


//SD
#define WIN_SH_IDX          WSTL_VOLUME_01_8BIT //WSTL_VOLUME_01
#define VOLUME_BMP_SH_IDX   WSTL_MIXBACK_IDX_06_8BIT//WSTL_MIXBACK_BLACK_IDX
#define VOLUME_SH_IDX       WSTL_VOLUME_01_BG_8BIT //WSTL_VOLUME_01_BG
#define VOLUME_MID_SH_IDX   WSTL_NOSHOW_IDX     ///*WSTL_VOLUME_01_BG*/
#define VOLUME_FG_SH_IDX    WSTL_VOLUME_01_FG_8BIT

#ifndef SD_UI
#define W_L  198//334 //20
#define W_T  460//500//500334 //360
#define W_W  602 //560
#define W_H  60  //40

#define BMP_W       60 //40
#define BMP_H       60 //40
#define BMP_L_OFF   10
#define BMP_T_OFF   ((W_H - BMP_H)/2)

#define R_L_OFF   0 //100
#define R_T_OFF   14
#define R_R_OFF   4
#define R_L (W_L + R_L_OFF+100)
#define R_T (W_T)  //+10)
#define R_W (W_W - 110 - R_R_OFF)
#define R_H W_H
#define RCT_W       R_W
#define RCT_H       (W_H - R_T_OFF*2)
#else
#define W_L  24//334 //20
#define W_T  360//500//500334 //360
#define W_W  560 //560
#define W_H  44  //40

#define BMP_W       32 //40
#define BMP_H       44 //40
#define BMP_L_OFF   10
#define BMP_T_OFF   ((W_H - BMP_H)/2)

#define R_L_OFF   50 //100
#define R_T_OFF   10
#define R_R_OFF   10
#define R_L (W_L + R_L_OFF)
#define R_T (W_T)  //+10)
#define R_W (W_W - R_L_OFF - R_R_OFF)
#define R_H W_H
#define RCT_W       R_W
#define RCT_H       (W_H - R_T_OFF*2)

#endif

#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)       \
    DEF_BITMAP(var_bmp,root,nxt_obj,C_ATTR_ACTIVE,0, 0,0,0,0,0, l,t,w,h, \
        VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX,\
        VOLUME_BMP_SH_IDX,NULL,NULL,C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, 1, 1, 1, 1, 1,\
        l, t, w, h, VOLUME_SH_IDX, VOLUME_SH_IDX, VOLUME_SH_IDX, VOLUME_SH_IDX,\
        win_volume_bar_keymap, win_volume_bar_callback, style, 4, 0, \
        VOLUME_MID_SH_IDX, VOLUME_FG_SH_IDX, rl,rt , rw, rh, 0, 100, 50, 1)

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_wnd,NULL,NULL,C_ATTR_ACTIVE,0, 0,0,0,0,0, l,t,w,h, \
        WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
        win_volume_keymap,win_volume_callback, nxt_obj, focus_id,1)


static VACTION win_volume_bar_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT win_volume_bar_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION win_volume_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT win_volume_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);



LDEF_BMP(&g_win_volume,vol_bmp,&vol_bar, W_L + BMP_L_OFF, W_T + BMP_T_OFF, \
            BMP_W, BMP_H,IM_VOLUME)

LDEF_PROGRESS_BAR(&g_win_volume, vol_bar, NULL,R_L , R_T, R_W, R_H, \
                    PBAR_STYLE_RECT_STYLE | PROGRESSBAR_HORI_NORMAL,0,\
                    R_T_OFF,RCT_W,RCT_H)

LDEF_WIN(g_win_volume,&vol_bmp,W_L,W_T,W_W,W_H,1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

static ID   volume_timer = OSAL_INVALID_ID;
static UINT32 VOLUME_TIMER_TIME = 0;
#define VOLUME_TIMER_NAME   "vol"


//extern void win_comlst_set_style(POBJECT_HEAD pobj,UINT32 shidx,UINT32 hlidx,UINT32 slidx,UINT32 gyidx);
static void win_volume_switch_clrstyle(void);

static void volume_timer_func(UINT unused);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION win_volume_bar_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

#ifdef CEC_SUPPORT
    if(api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
    {
        act = PROC_LEAVE;
    }
#endif

    switch(key)
    {
#ifdef _INVW_JUICE
    case V_KEY_F_DOWN:
#endif
    case V_KEY_LEFT:
    case V_KEY_V_DOWN:
        act = VACT_DECREASE;
        break;
#ifdef _INVW_JUICE
    case V_KEY_F_UP:
#endif
    case V_KEY_RIGHT:
    case V_KEY_V_UP:
        act = VACT_INCREASE;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT win_volume_bar_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    PROGRESS_BAR *bar=NULL;
    SYSTEM_DATA *sys_data=NULL;
    UINT32 vkey = 0;

    bar = (PROGRESS_BAR*)p_obj;

    sys_data = sys_data_get();

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        {
            sys_data->volume = osd_get_progress_bar_pos(bar);
            if((sys_data->volume > 0) && (get_mute_state()))
            {
                if(FALSE == get_channel_parrent_lock())
                {
                    set_mute_on_off(TRUE);
                    save_mute_state();
                    //added by Sharon to resolve mute clear display bug
                    restore_mute_bg();
                }
            }
            api_audio_set_volume(sys_data->volume);
            api_stop_timer(&volume_timer);
            volume_timer = api_start_timer(VOLUME_TIMER_NAME,VOLUME_TIMER_TIME, volume_timer_func);
        }
        ret = PROC_LOOP;
        break;
    case EVN_UNKNOWNKEY_GOT:
        ap_hk_to_vk(0, param1, &vkey);
        if(V_KEY_NULL == vkey)
        {
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION win_volume_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT win_volume_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    SYSTEM_DATA *sys_data=NULL;
    PROGRESS_BAR *bar=NULL;
    UINT8 max_osd_show_time = 10;

    bar = &vol_bar;
    sys_data = sys_data_get();

#ifdef CEC_SUPPORT
    if(api_cec_get_system_audio_control_feature() && api_cec_get_system_audio_mode_status())
    {
        return PROC_LEAVE;
    }
#endif


    switch(event)
    {
    case EVN_PRE_OPEN:
        win_volume_switch_clrstyle();
        api_inc_wnd_count();
        VOLUME_TIMER_TIME = sys_data->osd_set.time_out;
        if((0 == VOLUME_TIMER_TIME) || (VOLUME_TIMER_TIME>max_osd_show_time))
        {
            VOLUME_TIMER_TIME = 5;
        }
    VOLUME_TIMER_TIME *= 1000;
#if (CC_ON == 1 )
        cc_vbi_show_on(FALSE);
#endif
        osd_set_progress_bar_pos(bar, sys_data->volume);
        break;
    case EVN_POST_OPEN:
        volume_timer = api_start_timer(VOLUME_TIMER_NAME,VOLUME_TIMER_TIME,volume_timer_func);
        break;
    case EVN_UNKNOWNKEY_GOT:
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, param1,FALSE);
        ret = PROC_LEAVE;
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_CMD_EXIT == param1)
        {
            ret = PROC_LEAVE;
        }
    #ifdef DLNA_DMR_SUPPORT
        if(CTRL_MSG_SUBTYPE_DLNA_DMR_VOL == param1)
        {
            osd_set_progress_bar_pos(bar, sys_data->volume);
            osd_draw_object((POBJECT_HEAD)bar, C_UPDATE_ALL);
            ret = PROC_LOOP;
        }

    #endif

#ifdef USB_MP_SUPPORT
        else if(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER == param1)
        {
            ret = PROC_LEAVE;
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,0,TRUE);
        }
#endif
        break;
    case EVN_PRE_CLOSE:
        // if open volume window by popup window, not clear OSD.
        if(menu_stack_get_top() != (POBJECT_HEAD)&g_win_volume)
        {
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        }
        break;
    case EVN_POST_CLOSE:
        api_stop_timer(&volume_timer);
        sys_data_save(1);
        api_dec_wnd_count();
#if (CC_ON == 1 )
        cc_vbi_show_on(TRUE);
#endif
        break;
    default:
        break;
    }

    return ret;
}


static void volume_timer_func(UINT unused)
{
    UINT32 hkey = 0;

    api_stop_timer(&volume_timer);
    ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE); // send key instead of sending message
    //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
}

static void win_volume_switch_clrstyle(void)
{
    PPROGRESS_BAR pprbar = NULL;
    POBJECT_HEAD  pobj = NULL;
    PBITMAP       pbmp = NULL;

    pobj    = (POBJECT_HEAD) &g_win_volume;
    win_comlst_set_style(pobj,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX);
    pobj    = (POBJECT_HEAD) &vol_bmp;
    pbmp = &vol_bmp;
    pbmp->w_icon_id = IM_VOLUME;
    win_comlst_set_style(pobj,VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX, VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX);    
    pobj    = (POBJECT_HEAD) &vol_bar;
    win_comlst_set_style(pobj,VOLUME_SH_IDX,VOLUME_SH_IDX,VOLUME_SH_IDX,VOLUME_SH_IDX);
    pprbar  = &vol_bar;
    pprbar->w_tick_bg = VOLUME_MID_SH_IDX;
    pprbar->w_tick_fg = VOLUME_FG_SH_IDX;
}

