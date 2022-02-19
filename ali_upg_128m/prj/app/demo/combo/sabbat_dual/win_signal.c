/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_signal.c
*
*    Description: signal quality & intensity menu.
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
#include <api/libpub29/lib_as.h>
#endif

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "key.h"
#include "control.h"
#include "win_signal.h"
/*******************************************************************************
*   Objects definition
*******************************************************************************/


#ifndef SD_UI
#ifdef  _BUILD_OTA_E_
#define W_L     64
#else
#define W_L     110
#endif
#define W_L_B   (W_L + 16)
#define W_T     310
#define W_W     630
#define W_W_B   (W_W - 16)
#define W_H     86

#define TXT_H           40//26
#define TXTL_L_OF       16
#define TXTL_L_OF_B     (TXTL_L_OF + 16)

#define TXTLL_T_OF  	4
#define TXTLQ_T_OF  (	TXTLL_T_OF + TXT_H + 2)
#define TXTL_W          250

#define TXTR_L_OF   	(TXTL_L_OF + TXTL_W + B_W + 10)
#define TXTRL_T_OF  	4
#define TXTRQ_T_OF  	(TXTLL_T_OF + TXT_H + 2)
#define TXTR_W          	80

#define B_L_OF   	 (TXTL_L_OF + TXTL_W)
#define B_L_OF_B   (B_L_OF + 16)
#define B_H      24 //24 //16
#define B_W      270
#define B_W_B        (B_W - 16)
#define BL_T_OF  	(TXTLL_T_OF + (TXT_H - B_H)/2)
#define BQ_T_OF 	 (BL_T_OF + TXT_H + 2)
#define MINI_SIGNAL_L 62
#define MINI_SIGNAL_T 210
#else
#ifdef  _BUILD_OTA_E_
#define W_L     64
#define  TXTL_W         140//250
#define  TXTR_W         40//80
#else
#define W_L     50//110
#define  TXTL_W         150//250
#define  TXTR_W         50//80
#endif
#define W_L_B   (W_L + 16)
#define W_T     300
#define W_W     450
#define W_W_B   (W_W - 16)
#define W_H     60

#define TXT_H           24//26
#define TXTL_L_OF       16
#define TXTL_L_OF_B     (TXTL_L_OF + 16)

#define TXTLL_T_OF  4
#define TXTLQ_T_OF  (TXTLL_T_OF + TXT_H + 2)


#define TXTR_L_OF       400//(TXTL_L_OF + TXTL_W + B_W + 20)
#define TXTRL_T_OF  4
#define TXTRQ_T_OF  (TXTLL_T_OF + TXT_H + 2)


#define B_L_OF   	(TXTL_L_OF + TXTL_W)
#define B_L_OF_B  (B_L_OF + 16)
#define B_H      	16//24 //24 //16
#define B_W      	210
#define B_W_B        (B_W - 16)
#define BL_T_OF  	(TXTLL_T_OF + (TXT_H - B_H)/2)
#define BQ_T_OF  	(BL_T_OF + TXT_H + 2)
#define MINI_SIGNAL_L 62
#define MINI_SIGNAL_T 210

#endif

#define WIN_SH_IDX                  		WSTL_NOSHOW_IDX //WSTL_WIN_SIGNAL_01
#define B_BG_SH_IDX                 WSTL_BARBG_01_8BIT
#define B_LEVEL_UNLOCK_SH_IDX       WSTL_BAR_03_8BIT//WSTL_BAR_03
#define B_LEVEL_LOCK_SH_IDX         WSTL_BAR_01_8BIT//WSTL_BAR_01
#define B_QUALITY_UNLOCK_SH_IDX     WSTL_BAR_03_8BIT//WSTL_BAR_03
#define B_QUALITY_LOCK_SH_IDX       WSTL_BAR_02_8BIT//WSTL_BAR_02
#define B_MID_SH_IDX                WSTL_NOSHOW_IDX
#define TXT_SH_IDX                  WSTL_TEXT_30_8BIT
           //WSTL_MIXBACK_IDX_06_8BIT //WSTL_NOSHOW_IDX //WSTL_MIXBACK_BLACK_IDX

#define WIN_SH_IDX_HD                   		WSTL_NOSHOW_IDX //WSTL_WIN_SIGNAL_01
#define B_BG_SH_IDX_HD                  WSTL_BARBG_01_HD
#define B_LEVEL_UNLOCK_SH_IDX_HD        WSTL_BAR_03_HD//WSTL_BAR_03
#define B_LEVEL_LOCK_SH_IDX_HD          WSTL_BAR_01_HD//WSTL_BAR_01
#define B_QUALITY_UNLOCK_SH_IDX_HD      WSTL_BAR_03_HD//WSTL_BAR_03
#define B_QUALITY_LOCK_SH_IDX_HD        WSTL_BAR_02_HD//WSTL_BAR_02
#define B_MID_SH_IDX_HD                 WSTL_NOSHOW_IDX
#define TXT_SH_IDX_HD                   WSTL_BUTTON_01_HD
                                        //WSTL_TEXT_30_8BIT //WSTL_MIXBACK_IDX_06_8BIT
                                        //WSTL_NOSHOW_IDX //WSTL_MIXBACK_BLACK_IDX

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh,fsh) \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX,\
        NULL, NULL, style, 0, 0, B_MID_SH_IDX, fsh, \
        rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_PROGRESS_BAR2(root,var_bar,nxt_obj,l,t,w,h,style,bx,by,rl,rt,rw,rh,fsh)  \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX,\
        NULL, NULL, style, bx,by, B_MID_SH_IDX, fsh, \
        rl,rt , rw, rh, 1, 10, 10, 1)


#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)  \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_SH_IDX,TXT_SH_IDX,TXT_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    NULL,NULL,  \
    nxt_obj, focus_id,0)

#define HIGH_QUALITY 90
#define LOW_QUALITY  10
#define MAX_QUALITY  100

#define LOCK_QUALITY_THRESHOLD         30
#define UNLOCK_QUALITY_THRESHOLD     20

#define HIGH_LEVEL     90
#define LOW_LEVEL     10

#define NOSHOW_LEVEL   4
#define NOSHOW_QUALITY 4

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/
static UINT16 level_percent[10] = {0};
static UINT16 quality_percent[10] ={0};

LDEF_TXT(&g_win_signal,sig_txt_level,&sig_bar_level,    \
        W_L + TXTL_L_OF,W_T + TXTLL_T_OF, TXTL_W, TXT_H,RS_INFO_SIGNAL_INTENSITY,NULL)
LDEF_TXT(&g_win_signal,sig_txt_quality,&sig_bar_quality,    \
        W_L + TXTL_L_OF,W_T + TXTLQ_T_OF, TXTL_W, TXT_H,RS_INFO_SIGNAL_QUALITY,NULL)

LDEF_TXT(&g_win_signal,sig_txt_level_per,&sig_txt_quality,  \
        W_L + TXTR_L_OF,W_T + TXTRL_T_OF, TXTR_W, TXT_H,0,level_percent)
LDEF_TXT(&g_win_signal,sig_txt_quality_per,NULL ,   \
        W_L + TXTR_L_OF,W_T + TXTRQ_T_OF, TXTR_W, TXT_H,0,quality_percent)

#ifndef SD_UI
LDEF_PROGRESS_BAR(&g_win_signal,sig_bar_level,&sig_txt_level_per,   \
        W_L + B_L_OF, W_T + BL_T_OF, B_W, B_H, PROGRESSBAR_HORI_NORMAL| PBAR_STYLE_RECT_STYLE,2,5,B_W,B_H-10,0)
/*LDEF_PROGRESS_BAR2(&g_win_signal,sig_bar_level,&sig_txt_level_per,    \
  W_L + B_L_OF, W_T + BL_T_OF, B_W, B_H, PROGRESSBAR_HORI_REVERSE| PBAR_STYLE_RECT_STYLE,1,0,0,5,B_W,B_H-10,0)*/

LDEF_PROGRESS_BAR(&g_win_signal,sig_bar_quality,&sig_txt_quality_per,   \
        W_L + B_L_OF, W_T + BQ_T_OF, B_W, B_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,B_W,B_H-10,0)
/*LDEF_PROGRESS_BAR2(&g_win_signal,sig_bar_quality,&sig_txt_quality_per,    \
      W_L + B_L_OF, W_T + BQ_T_OF, B_W, B_H, PROGRESSBAR_HORI_NORMAL| PBAR_STYLE_RECT_STYLE,1,0,0,5,B_W,B_H-10,0)*/
#else
LDEF_PROGRESS_BAR(&g_win_signal,sig_bar_level,&sig_txt_level_per,   \
        W_L + B_L_OF, W_T + BL_T_OF, B_W, B_H, PROGRESSBAR_HORI_NORMAL| PBAR_STYLE_RECT_STYLE,2,2,B_W,B_H-4,0)
/*LDEF_PROGRESS_BAR2(&g_win_signal,sig_bar_level,&sig_txt_level_per,    \
      W_L + B_L_OF, W_T + BL_T_OF, B_W, B_H, PROGRESSBAR_HORI_REVERSE| PBAR_STYLE_RECT_STYLE,1,0,0,5,B_W,B_H-10,0)*/

LDEF_PROGRESS_BAR(&g_win_signal,sig_bar_quality,&sig_txt_quality_per,   \
        W_L + B_L_OF, W_T + BQ_T_OF, B_W, B_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,2,B_W,B_H-4,0)
/*LDEF_PROGRESS_BAR2(&g_win_signal,sig_bar_quality,&sig_txt_quality_per,    \
      W_L + B_L_OF, W_T + BQ_T_OF, B_W, B_H, PROGRESSBAR_HORI_NORMAL| PBAR_STYLE_RECT_STYLE,1,0,0,5,B_W,B_H-10,0)*/

#endif
LDEF_WIN(g_win_signal,&sig_txt_level,W_L,W_T,W_W,W_H,0)

//Open following define will open the signal linear display
//#define DISPLAY_SIGNAL_QUALITY_EXT

#ifdef DISPLAY_SIGNAL_QUALITY_EXT
static const UINT32 agc_table[256] =
{
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   3,   4,   5,
  6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
 22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,
 38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  52,  55,  57,
 60,  62,  65,  67,  70,  72,  75,  77,  78,  79,  80,  82,  83,  84,  85,  87,
 88,  89,  90,  92,  93,  94,  95,  97,  98,  99, 101, 102, 103, 104, 105, 106,
107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
139, 140, 141, 142, 144, 145, 146, 147, 149, 150, 151, 152, 154, 155, 156, 157,
159, 160, 161, 162, 164, 165, 166, 167, 169, 170, 171, 173, 175, 176, 178, 180,
182, 184, 187, 189, 192, 194, 197, 199, 202, 204, 207, 209, 212, 215, 218, 222,
225, 228, 232, 235, 238, 242, 245, 248, 252, 255, 258, 262, 265, 268, 272, 275,
278, 282, 285, 288, 292, 295, 298, 302, 305, 308, 312, 317, 322, 327, 332, 337,
342, 347, 352, 357, 362, 367, 372, 377, 382, 387, 392, 397, 402, 407, 412, 417,
422, 427, 432, 437, 442, 447, 452, 457, 462, 467, 472, 477, 482, 487, 492, 493,
494, 495, 496, 497, 498, 499, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};
#endif

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
#ifdef WIFI_SUPPORT
BOOL win_signal_set_wifi_level_quality(UINT32 level,UINT32 quality,UINT32 lock)
{
    PROGRESS_BAR *bar = NULL;
    POBJECT_HEAD p_obj = NULL;
    UINT16 level_shidx = 0;
    UINT16 quality_shidx = 0;

    if(lock)
    {
        level_shidx  = B_LEVEL_LOCK_SH_IDX;
        quality_shidx = B_QUALITY_LOCK_SH_IDX;
    }
    else
    {
        level_shidx = B_LEVEL_UNLOCK_SH_IDX;
        quality_shidx = B_QUALITY_UNLOCK_SH_IDX;
    }
    if(level < NOSHOW_LEVEL)
    {
        level_shidx = WSTL_NOSHOW_IDX;
    }
    if(quality < NOSHOW_QUALITY)
    {
        quality_shidx = WSTL_NOSHOW_IDX;
    }
    bar = &sig_bar_level;
    osd_set_progress_bar_pos(bar,level);
    bar->w_tick_fg = level_shidx;

    bar = &sig_bar_quality;

    osd_set_progress_bar_pos(bar,quality);
    bar->w_tick_fg = quality_shidx;

    /* If signal display is on */
    p_obj = (POBJECT_HEAD)&g_win_signal;
    if(NULL == p_obj->p_root)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif

void win_signal_set_level_quality(UINT32 level,UINT32 quality,UINT32 lock)
{
	PROGRESS_BAR *bar = NULL;
	//POBJECT_HEAD p_obj = NULL;
	UINT16 level_shidx = 0;
	UINT16 quality_shidx = 0;

	if(lock)
	{
		level_shidx  	= B_LEVEL_LOCK_SH_IDX;
		quality_shidx 	= B_QUALITY_LOCK_SH_IDX;
	}
	else
	{
		level_shidx 	= B_LEVEL_UNLOCK_SH_IDX;
		quality_shidx 	= B_QUALITY_UNLOCK_SH_IDX;
	}
	if(level < NOSHOW_LEVEL)
	{
		level_shidx 	= WSTL_NOSHOW_IDX;
	}
	if(quality < NOSHOW_QUALITY)
	{
		quality_shidx 	= WSTL_NOSHOW_IDX;
	}
	bar = &sig_bar_level;
	osd_set_progress_bar_pos(bar,level);
	bar->w_tick_fg = level_shidx;

	bar = &sig_bar_quality;
	#if 0
	if(quality < LOW_QUALITY)
	{
		quality = 10;
	}
	#endif
	osd_set_progress_bar_pos(bar,quality);
	bar->w_tick_fg = quality_shidx;

#if 0
	/* If signal display is on */
	p_obj = (POBJECT_HEAD)&g_win_signal;
	if(NULL == p_obj->p_root)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
#endif
}

void win_signal_open(POBJECT_HEAD w)
{
    POBJECT_HEAD p_obj = NULL;
    UINT8 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;

    struct nim_device *nim_dev = NULL;

    if(1 == get_tuner_cnt())
    {
        cur_tuner_idx = 0;
    }
    if(0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    if(NULL == w)
    {
        return ;
    }
    if(w->b_type != OT_CONTAINER)
    {
        return ;
    }
    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->p_root = w;

    p_obj = (POBJECT_HEAD)&sig_bar_level;
    p_obj->frame.u_left = W_L + B_L_OF;
    p_obj->frame.u_width = B_W;

    p_obj = (POBJECT_HEAD)&sig_bar_quality;
    p_obj->frame.u_left = W_L + B_L_OF;
    p_obj->frame.u_width = B_W;

    p_obj = (POBJECT_HEAD)&sig_txt_level;
    p_obj->frame.u_left = W_L + TXTL_L_OF;

    p_obj = (POBJECT_HEAD)&sig_txt_quality;
    p_obj->frame.u_left = W_L + TXTL_L_OF;


    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->frame.u_left = W_L;
    p_obj->frame.u_width = W_W;

    nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
    nim_get_agc(nim_dev, &level);
#endif
    nim_get_snr(nim_dev, &quality);
    if(NULL != nim_dev->do_ioctl)
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);
}

void win_signal_open_ex(POBJECT_HEAD w , UINT32 x, UINT32 y)
{
    POBJECT_HEAD p_obj = NULL;
    PPROGRESS_BAR p_bar = NULL;
    UINT8 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;

    struct nim_device *nim_dev = NULL;

    if( 0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    if(NULL == w)
    {
        return ;
    }
    if(w->b_type != OT_CONTAINER)
    {
        return ;
    }
    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->p_root = w;
    osd_set_color(p_obj,WIN_SH_IDX_HD,WIN_SH_IDX_HD,WIN_SH_IDX_HD,WIN_SH_IDX_HD);

    p_obj = (POBJECT_HEAD)&sig_bar_level;
    //p_obj->frame.u_left = x+B_L_OF;//W_L + B_L_OF;
    p_obj->frame.u_width = B_W;
    osd_set_color(p_obj,B_BG_SH_IDX_HD,B_BG_SH_IDX_HD,B_BG_SH_IDX_HD,B_BG_SH_IDX_HD);
    p_bar = (PPROGRESS_BAR)&sig_bar_level;
    p_bar->w_tick_bg = B_MID_SH_IDX_HD;
    //libc_printf("bar:%d,%d\n",B_BG_SH_IDX_HD,B_MID_SH_IDX_HD);

    p_obj = (POBJECT_HEAD)&sig_bar_quality;
    //p_obj->frame.u_left = x+ B_L_OF;//W_L + B_L_OF;
    p_obj->frame.u_width = B_W;
    osd_set_color(p_obj,B_BG_SH_IDX_HD,B_BG_SH_IDX_HD,B_BG_SH_IDX_HD,B_BG_SH_IDX_HD);
    p_bar = (PPROGRESS_BAR)&sig_bar_quality;
    p_bar->w_tick_bg = B_MID_SH_IDX_HD;

    p_obj = (POBJECT_HEAD)&sig_txt_level;
    //p_obj->frame.u_left = x+TXTL_L_OF;//W_L + TXTL_L_OF;
    osd_set_color(p_obj,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD);
    //libc_printf("txt:%d\n",TXT_SH_IDX_HD);

    p_obj = (POBJECT_HEAD)&sig_txt_quality;
    //p_obj->frame.u_left = x + TXTL_L_OF;//W_L + TXTL_L_OF;
    osd_set_color(p_obj,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD);

    p_obj = (POBJECT_HEAD)&sig_txt_level_per;
    //p_obj->frame.u_left = x + TXTR_L_OF;//W_L + TXTL_L_OF;
    //p_obj->frame.u_top = y + TXTRL_T_OF;
    osd_set_color(p_obj,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD);

    p_obj = (POBJECT_HEAD)&sig_txt_quality_per;
    //p_obj->frame.u_left = x + TXTR_L_OF;//W_L + TXTL_L_OF;
    //p_obj->frame.u_top = y + TXTRQ_T_OF;
    osd_set_color(p_obj,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD,TXT_SH_IDX_HD);

    p_obj = (POBJECT_HEAD)&g_win_signal;
    //p_obj->frame.u_left = W_L;
    //p_obj->frame.u_top = W_T;
    //p_obj->frame.u_top = y;
    p_obj->frame.u_width = W_W;
    osd_move_object(p_obj,x,y,0);

    nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
    nim_get_agc(nim_dev, &level);
#endif

    nim_get_snr(nim_dev, &quality);

    if(NULL != nim_dev->do_ioctl)
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    if(!lock)
    {
        level = 0;
        quality = 0;
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);
}

void win_minisignal_open(POBJECT_HEAD w)
{
    POBJECT_HEAD p_obj = NULL;
    PPROGRESS_BAR p_bar = NULL;
    UINT8 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;

    struct nim_device *nim_dev = NULL;

    if( 0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    if(NULL == w)
    {
        return ;
    }
    if(w->b_type != OT_CONTAINER)
    {
        return ;
    }
    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->p_root = w;
    //osd_set_color(p_obj,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_bar_level;
    //p_obj->frame.u_left = W_L + B_L_OF;
    p_obj->frame.u_width = B_W;
    osd_set_color(p_obj,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX);
    p_bar = (PPROGRESS_BAR)&sig_bar_level;
    p_bar->w_tick_bg = B_MID_SH_IDX;

    p_obj = (POBJECT_HEAD)&sig_bar_quality;
    //p_obj->frame.u_left = W_L + B_L_OF;
    p_obj->frame.u_width = B_W;
    osd_set_color(p_obj,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX);
    p_bar = (PPROGRESS_BAR)&sig_bar_quality;
    p_bar->w_tick_bg = B_MID_SH_IDX;

    p_obj = (POBJECT_HEAD)&sig_txt_level;
    //p_obj->frame.u_left = W_L + TXTL_L_OF;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_quality;
    //p_obj->frame.u_left = W_L + TXTL_L_OF;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_level_per;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_quality_per;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&g_win_signal;
    //p_obj->frame.u_left = W_L;
    //p_obj->frame.u_width = W_W;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);


    osd_move_object(p_obj,MINI_SIGNAL_L, MINI_SIGNAL_T,FALSE);

    nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
    nim_get_agc(nim_dev, &level);
#endif
    nim_get_snr(nim_dev, &quality);
    if(NULL != nim_dev->do_ioctl)
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);
}

void win_minisignal_open_ex(POBJECT_HEAD w,UINT32 x, UINT32 y)
{
    POBJECT_HEAD p_obj = NULL;
    PPROGRESS_BAR p_bar = NULL;
    UINT8 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;

    struct nim_device *nim_dev = NULL;

    if( 0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    if(NULL == w)
    {
        return ;
    }
    if(w->b_type != OT_CONTAINER)
    {
        return ;
    }
    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->p_root = w;
    //osd_set_color(p_obj,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_bar_level;
    //p_obj->frame.u_left = W_L + B_L_OF;
    p_obj->frame.u_width = B_W;
    osd_set_color(p_obj,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX);
    p_bar = (PPROGRESS_BAR)&sig_bar_level;
    p_bar->w_tick_bg = B_MID_SH_IDX;

    p_obj = (POBJECT_HEAD)&sig_bar_quality;
    //p_obj->frame.u_left = W_L + B_L_OF;
    p_obj->frame.u_width = B_W;
    osd_set_color(p_obj,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX,B_BG_SH_IDX);
    p_bar = (PPROGRESS_BAR)&sig_bar_quality;
    p_bar->w_tick_bg = B_MID_SH_IDX;
    
    p_obj = (POBJECT_HEAD)&sig_txt_level;
    //p_obj->frame.u_left = W_L + TXTL_L_OF;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_quality;
    //p_obj->frame.u_left = W_L + TXTL_L_OF;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_level_per;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_quality_per;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&g_win_signal;
    //p_obj->frame.u_left = W_L;
    //p_obj->frame.u_width = W_W;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    //osd_move_object(p_obj,MINI_SIGNAL_L, MINI_SIGNAL_T,FALSE);
    osd_move_object(p_obj,x,y,FALSE);
    nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
    nim_get_agc(nim_dev, &level);
#endif
    nim_get_snr(nim_dev, &quality);
    if(NULL != nim_dev->do_ioctl)
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);
}

void win_minisignal_restore(void)
{
    POBJECT_HEAD p_obj = NULL;

    p_obj = (POBJECT_HEAD)&g_win_signal;
    osd_set_color(p_obj, WIN_SH_IDX, WIN_SH_IDX, WIN_SH_IDX, WIN_SH_IDX);
    osd_move_object(p_obj,W_L, W_T,FALSE);

    p_obj = (POBJECT_HEAD)&sig_txt_level;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_quality;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_level_per;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);

    p_obj = (POBJECT_HEAD)&sig_txt_quality_per;
    osd_set_color(p_obj, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX, TXT_SH_IDX);
}

void win_signal_open_tplist(POBJECT_HEAD w)
{
    POBJECT_HEAD p_obj = NULL;
    UINT8 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;

    struct nim_device *nim_dev = NULL;

    if( 0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    if(NULL == w)
    {
        return ;
    }
    if(w->b_type != OT_CONTAINER)
    {
        return ;
    }
    p_obj = (POBJECT_HEAD)&sig_bar_level;
    p_obj->frame.u_left = W_L + B_L_OF_B;
    p_obj->frame.u_width = B_W_B;

    p_obj = (POBJECT_HEAD)&sig_bar_quality;
    p_obj->frame.u_left = W_L + B_L_OF_B;
    p_obj->frame.u_width = B_W_B;

    p_obj = (POBJECT_HEAD)&sig_txt_level;
    p_obj->frame.u_left = W_L + TXTL_L_OF_B;

    p_obj = (POBJECT_HEAD)&sig_txt_quality;
    p_obj->frame.u_left = W_L + TXTL_L_OF_B;


    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->frame.u_left = W_L_B;
    p_obj->frame.u_width = W_W_B;

    p_obj->p_root = w;

    nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
    nim_get_agc(nim_dev, &level);
#endif
    nim_get_snr(nim_dev, &quality);
    if(NULL != nim_dev->do_ioctl)
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);
}


void win_signal_close(void)
{
    POBJECT_HEAD p_obj = NULL;

    p_obj = (POBJECT_HEAD)&g_win_signal;
    p_obj->p_root = NULL;
}

void win_signal_update(void)
{
    POBJECT_HEAD p_obj = NULL;
    PROGRESS_BAR *bar = NULL;
    TEXT_FIELD *txt = NULL;

#ifdef CI_PLUS_SUPPORT
    if(is_ciplus_menu_exist())
    {
        return ;//FALSE;
    }
#endif

#ifdef _MHEG5_SUPPORT_
    if(mheg_app_avaliable())
    {
        return;
    }
#endif

    p_obj = (POBJECT_HEAD)&g_win_signal;
    if(NULL == p_obj->p_root)
    {
        return ;//FALSE;
    }

    bar = &sig_bar_level;
    txt = &sig_txt_level_per;
    osd_set_text_field_content(txt, STRING_NUM_PERCENT,osd_get_progress_bar_pos(bar));

    bar = &sig_bar_quality;
    txt = &sig_txt_quality_per;
    osd_set_text_field_content(txt, STRING_NUM_PERCENT,osd_get_progress_bar_pos(bar));

    osd_draw_object(p_obj, C_UPDATE_ALL);

    return ;//TRUE;
}

void win_signal_refresh(void)
{
    UINT8 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;
    struct nim_device *nim_dev = NULL;

    if(SINGLE_TUNER == g_tuner_num)
    {
        cur_tuner_idx = 0;
    }
    if( 0 == cur_tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    nim_get_lock(nim_dev, &lock);
#ifdef _S3281_
    nim_get_agc(nim_dev, &level);
#endif
    nim_get_snr(nim_dev, &quality);
    if(NULL != nim_dev->do_ioctl)
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);

    win_signal_update();
}

void dem_signal_to_display(UINT8 lock,UINT8 *level,UINT8 *quality)
{
#if 1//def DISPLAY_SIGNAL_QUALITY_EXT
	#if 1
	return;
	#else
    #if ((defined _S3811_)&&(defined DVBT_SUPPORT||defined ISDBT_SUPPORT||defined DVBS_SUPPORT))
        return;
    #elif ((defined _M3503_)&&(defined DVBS_SUPPORT))
        return;
    #elif ((defined _M3821_)&&(defined DVBT_SUPPORT))
        return;
	#elif ((defined _M3505_)&&(defined DVBS_SUPPORT))
		return;
    #else
        UINT32 temp = 0;

        if((NULL == level) && (NULL == quality))
        {
            return ;
        }
        temp = agc_table[(*level)];
        *level = (UINT8)(temp/5);

        if (lock)
        {
            temp = *quality;
            if (temp<0)
            {
                temp = 0;
            }
            if (temp>MAX_QUALITY)
            {
                temp = 100;
            }
            *quality = temp;
        }
        else
        {
            *quality = 0;
        }
    #endif
	#endif
#else
    #if (defined(ACCURATE_SSI_SQI))//accurate value from demod,don't modify it
        struct nim_device* nim_dev;
        if(SINGLE_TUNER == g_tuner_num)
        {
            cur_tuner_idx = 0;
        }
        if( 0 == cur_tuner_idx)
        {
            nim_dev = g_nim_dev;
        }
        else
        {
            nim_dev = g_nim_dev2;
        }
        if ((FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM))
        || (FRONTEND_TYPE_ISDBT == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM)))
        {
            if(*level>HIGH_LEVEL)
            {
                *level = 90;
            }
            if(*quality>HIGH_QUALITY)
            {
                *quality = 90;
            }
            if(!lock)
            {
                *level= 0;
                *quality = 0;
            }
            return;
        }
    #endif

    if(*level>HIGH_LEVEL)
    {
       *level = 90;
    }
    if (*level < LOW_LEVEL)
    {
       *level = *level*11/2;
    }
    else
    {
       *level = *level / 2 + 50;
    }
    if(*level>HIGH_LEVEL)
    {
       *level = 90;
    }
    if(!lock)
    {
       *level /= 2;
    }
    if(lock)
    {
        if (*quality < LOCK_QUALITY_THRESHOLD)
        {
            *quality = *quality*7/ 3;
        }
        else
        {
            *quality = *quality / 3 + 60;
        }
    }
    else
    {
        if (*quality >= UNLOCK_QUALITY_THRESHOLD)
        {
            *quality =  *quality*3/ 7;
        }
    }
    if(*quality>HIGH_QUALITY)
    {
        *quality = 90;
    }
#endif
}

