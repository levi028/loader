/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mpvolume.c
*
*    Description: Mediaplayer volume and banance setup UI.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_MP_SUPPORT
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>

#include <api/libosd/osd_lib.h>
#include <hld/pan/pan_dev.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_media.h"
#include "win_mpvolume.h"
#include "win_mute.h"
/*******************************************************************************
*    Objects definition
*******************************************************************************/


/*******************************************************************************
*    Macro definition
*******************************************************************************/
#define VOL_W_L 440//250
//#define VOL_W_T 160//128
#define VOL_W_T 254
#define VOL_W_W 120
//#define VOL_W_H 211
#define VOL_W_H 80

#define BAN_TXT_L (VOL_W_L+10) //262
#define BAN_TXT_T (VOL_W_T+10) //140
#define BAN_TXT_W 100
#define BAN_TXT_H 24

#define BAN_BAR_L (VOL_W_L+10) //262
#define BAN_BAR_T (BAN_TXT_T+BAN_TXT_H+10) //175
#define BAN_BAR_W 100
#define BAN_BAR_H 10

#define BAN_BMP_W 16
#define BAN_BMP_H 16
#define BAN_BMP_L (BAN_BAR_L+(BAN_BAR_W-BAN_BMP_W)/2) //308
#define BAN_BMP_T (BAN_BAR_T-(BAN_BMP_H-BAN_BAR_H)/2) //173

#define VOL_BAR_L VOL_W_L+15//(VOL_W_L+50) //300
//#define VOL_BAR_T (BAN_BAR_T+BAN_BAR_H+10)//204
#define VOL_BAR_T (VOL_W_T+14)
#define VOL_BAR_W 100//22
#define VOL_BAR_H 22//100//144

#define VOL_TXT_L (VOL_W_L+10) //262
#define VOL_TXT_T (VOL_BAR_T+VOL_BAR_H+6) //310
#define VOL_TXT_W 100
#define VOL_TXT_H 24


#define VOL_WIN_SH_IDX    WSTL_POP_WIN_01_HD
#define VOL_TXT_SH_IDX    WSTL_TEXT_04_HD

#define BAN_SH_IDX        WSTL_BARBG_01
#define BAN_MID_SH_IDX    WSTL_NOSHOW_IDX
#define BAN_FG_SH_IDX    WSTL_NOSHOW_IDX

#define BAN_BMP_SH_IDX    WSTL_MIXBACK_WHITE_IDX

#define VOL_SH_IDX        WSTL_TEXT_20_HD//WSTL_VOLUME_02//WSTL_VOLUME_01
#define VOL_MID_SH_IDX    WSTL_NOSHOW_IDX//WSTL_VOLUME_01_BG
#define VOL_FG_SH_IDX    WSTL_VOLUME_02_FG

#define MP_MAX_VOLUME    100//96
#define MP_MAX_VOL_POS   10//12
#define MP_MIN_VOL_POS   0
#define MP_VOL_BLOCKS    (MP_MAX_VOL_POS - MP_MIN_VOL_POS)

#define MP_VOL_TIMER_0SEC 0
#define MP_VOL_TIMER_10SEC 10

#define MP_VOL_SYS_VALUE_0 0

/*******************************************************************************
*    Function decalare
*******************************************************************************/
static VACTION mpvolume_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT mpvolume_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static ID    mpvolume_timer = OSAL_INVALID_ID;
static UINT32 MPVOLUME_TIMER_TIME = 0;

static INT16 mp_vol_to_pos(INT16 vol);
static INT16 mp_pos_to_vol(INT16 pos);
static void mpvolume_timer_func(UINT unused);

/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, VOL_WIN_SH_IDX,VOL_WIN_SH_IDX,VOL_WIN_SH_IDX,VOL_WIN_SH_IDX,   \
    mpvolume_keymap,mpvolume_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, VOL_TXT_SH_IDX,VOL_TXT_SH_IDX,VOL_TXT_SH_IDX,VOL_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LEFT_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)    \
    DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, BAN_BMP_SH_IDX,BAN_BMP_SH_IDX,BAN_BMP_SH_IDX,BAN_BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_MPBAN_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh)    \
    DEF_PROGRESSBAR(var_bar,&root, nxt_obj, C_ATTR_ACTIVE, 0, \
    0,0,0,0,0, l,t,w,h, BAN_SH_IDX, BAN_SH_IDX, BAN_SH_IDX, BAN_SH_IDX,\
    NULL, NULL, style, 0, 0, BAN_MID_SH_IDX, fsh, \
    rl, rt, rw, rh, 1, 100, 100, 1)

#define LDEF_MPVOL_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh)    \
    DEF_PROGRESSBAR(var_bar,&root, nxt_obj, C_ATTR_ACTIVE, 0, \
    0,0,0,0,0, l,t,w,h, VOL_SH_IDX, VOL_SH_IDX, VOL_SH_IDX, VOL_SH_IDX,\
    NULL, NULL, style, 6, 0, VOL_MID_SH_IDX, fsh, \
    rl, rt, rw, rh, MP_MIN_VOL_POS, MP_MAX_VOL_POS, MP_VOL_BLOCKS, 1)

/*******************************************************************************
*    Objects evaluate
*******************************************************************************/
LDEF_WIN(g_win_mpvolume,&mpvolume_bar, VOL_W_L,VOL_W_T,VOL_W_W, VOL_W_H, 1)

LDEF_MPVOL_BAR(g_win_mpvolume,mpvolume_bar,&mpvolume_txt,VOL_BAR_L,VOL_BAR_T,VOL_BAR_W,VOL_BAR_H,\
        PBAR_STYLE_RECT_STYLE|PROGRESSBAR_HORI_NORMAL,3, 3, (VOL_BAR_W-6), (VOL_BAR_H -6 ),VOL_FG_SH_IDX)

LDEF_TXT(g_win_mpvolume,mpvolume_txt,NULL,VOL_TXT_L,VOL_TXT_T,VOL_TXT_W,VOL_TXT_H,RS_HELP_VOLUMN)

LDEF_TXT(g_win_mpvolume,mpbalance_txt,&mpbalance_bar,BAN_TXT_L,BAN_TXT_T,BAN_TXT_W,BAN_TXT_H,RS_MP_BALANCE)

LDEF_MPBAN_BAR(g_win_mpvolume,mpbalance_bar,&mpbalance_bmp,BAN_BAR_L,BAN_BAR_T,BAN_BAR_W,BAN_BAR_H,\
        PBAR_STYLE_RECT_STYLE|PROGRESSBAR_HORI_NORMAL,0,0,0,0,BAN_FG_SH_IDX)

LEFT_BMP(g_win_mpvolume,mpbalance_bmp,NULL,BAN_BMP_L,BAN_BMP_T,BAN_BMP_W,BAN_BMP_H,IM_PVR_DOT)


#define VACT_VOL_UP (VACT_PASS + 1)
#define VACT_VOL_DOWN (VACT_PASS + 2)
#define VACT_BALANCE_L (VACT_PASS + 3)
#define VACT_BALANCE_R (VACT_PASS + 4)

/*******************************************************************************
*    Functions code
*******************************************************************************/
static VACTION mpvolume_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    case V_KEY_LEFT:
    case V_KEY_V_DOWN:
        act = VACT_VOL_DOWN;
        break;
    case V_KEY_RIGHT:
    case V_KEY_V_UP:
        act = VACT_VOL_UP;
        break;   
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT mpvolume_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    UINT16 vol_pos = 0;
    SYSTEM_DATA *psys = NULL;
    PPROGRESS_BAR  mpvol_bar = NULL;

    mpvol_bar = &mpvolume_bar;
    switch(event)
    {
           case EVN_PRE_OPEN:
            psys = sys_data_get();
            if((psys->volume > MP_VOL_SYS_VALUE_0) && (get_mute_state()) )
            {
                set_mute_on_off(TRUE);
            }
            vol_pos = mp_vol_to_pos(psys->volume);
            osd_set_progress_bar_value(mpvol_bar,vol_pos);
            MPVOLUME_TIMER_TIME = psys->osd_set.time_out;
            if((MP_VOL_TIMER_0SEC == MPVOLUME_TIMER_TIME) || (MPVOLUME_TIMER_TIME>MP_VOL_TIMER_10SEC))
            {
                MPVOLUME_TIMER_TIME = 5;
            }
            MPVOLUME_TIMER_TIME *= 1000;
            break;
        case EVN_POST_OPEN:
            mpvolume_timer = api_start_timer("mpvol",MPVOLUME_TIMER_TIME,mpvolume_timer_func);
            break;
        case EVN_POST_CLOSE:
            api_stop_timer(&mpvolume_timer);
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            switch(unact)
            {
            case VACT_VOL_UP:
            case VACT_VOL_DOWN:
                psys = sys_data_get();
                vol_pos = mp_vol_to_pos(psys->volume);
                if(VACT_VOL_DOWN == unact )
                {
                    vol_pos--;
                }
                else if(VACT_VOL_UP == unact )
                {
                    vol_pos++;
                }
                osd_set_progress_bar_value(mpvol_bar,vol_pos);
                psys->volume = mp_pos_to_vol(osd_get_progress_bar_pos(mpvol_bar));
                if((psys->volume > 0) && (get_mute_state()) )
                {
                    set_mute_on_off(TRUE);
                }
                api_audio_set_volume(psys->volume);
                osd_draw_object((POBJECT_HEAD)mpvol_bar,C_UPDATE_ALL);
                api_stop_timer(&mpvolume_timer);
                mpvolume_timer = api_start_timer("mpvol",MPVOLUME_TIMER_TIME,mpvolume_timer_func);
                break;
            case VACT_BALANCE_L:
            case VACT_BALANCE_R:
                break;
            default:
                break;
            }
            break;
        default:
            break;
    }

    return ret;
}

static INT16 mp_vol_to_pos(INT16 vol)
{
    if(vol > MP_MAX_VOLUME)
    {
        vol = MP_MAX_VOLUME;
    }
    else if(vol < 0)
    {
        vol = 0;
    }
    return (vol * MP_VOL_BLOCKS) / MP_MAX_VOLUME;
}

static INT16 mp_pos_to_vol(INT16 pos)
{
    if(pos > MP_VOL_BLOCKS)
    {
        pos = MP_VOL_BLOCKS;
    }
    else if(pos < 0)
    {
        pos = 0;
    }
    return (pos * MP_MAX_VOLUME) / MP_VOL_BLOCKS;
}

static void mpvolume_timer_func(UINT unused)
{
    // construct virtual operation
    UINT32 msg_code = 0;

    api_stop_timer(&mpvolume_timer);
    ap_vk_to_hk(0, V_KEY_EXIT, &msg_code);
    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
}


#endif

