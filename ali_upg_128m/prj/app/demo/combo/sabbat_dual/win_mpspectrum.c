/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mpspectrum.c
*
*    Description: spectrum analysis
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef USB_MP_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "control.h"
#include "win_mpspectrum.h"
#include "win_mute.h"

/*******************************************************************************
*    WINDOW's objects declaration
*******************************************************************************/
#define SPECTRUM_BAR_CNT    7
#define SPECTRUM_BAR_STEP            2//1


#define BAR_MIN_POS        0
#define BAR_MAX_POS        100



/*******************************************************************************
*    Macro definition
*******************************************************************************/
#define SPECTRUM_CON_SH_IDX        WSTL_MP_MUTE_BG_HD//WSTL_TEXT_05_HD//WSTL_PREV_01
#define B_BG_SH_IDX                WSTL_MP_MUTE_BG_HD //WSTL_MP_SPCTRM_GRAY_HD
#define B_MID_SH_IDX            WSTL_MP_SPCTRM_BAR_HD

//same as FL_INFO_BMP in win_filelist.c
#define    WIN_SPECTRUM_L            636
#define    WIN_SPECTRUM_T            85//93
#define    WIN_SPECTRUM_W            278//290
#define    WIN_SPECTRUM_H             166//160

#define    WIN_SPECTRUM_BAR_L        (WIN_SPECTRUM_L+18)
#define    WIN_SPECTRUM_BAR_W        32//20

#define    WIN_SPECTRUM_BAR_T        (WIN_SPECTRUM_T+20)
#define    WIN_SPECTRUM_BAR_H         128//138//128

#define    WIN_SPECTRUM_PEAK_H     8
#define    WIN_SPECTRUM_PEAK_T        (WIN_SPECTRUM_BAR_T+WIN_SPECTRUM_BAR_H-WIN_SPECTRUM_PEAK_H)

#define    WIN_SPECTRUM_BAR_GAP    4//6

#define  WIN_SPECTUM_BAR_SPLIT (WIN_SPECTRUM_BAR_W+WIN_SPECTRUM_BAR_GAP)

/*******************************************************************************
*    Function decalare
*******************************************************************************/

static BOOL update_win_spectrum(void);
static BOOL mute_win_spectrum(void);

/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_CON(var_wnd,nxt_obj,focus_id,l,t,w,h)        \
        DEF_CONTAINER(var_wnd,NULL,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, SPECTRUM_CON_SH_IDX,SPECTRUM_CON_SH_IDX,SPECTRUM_CON_SH_IDX,SPECTRUM_CON_SH_IDX,   \
        NULL,NULL,  \
        nxt_obj, focus_id,0)

#define LDEF_SPECTRUM_BAR(root,var_bar,nxt_obj,l,t,w,h,rcl,rct,rcw,rch,fsh)    \
        DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0,0,0,0,0, l,t,w,h, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX,\
        NULL, NULL, \
        PROGRESSBAR_VERT_REVERSE|PBAR_STYLE_RECT_STYLE, 0, 0, B_BG_SH_IDX, fsh, \
        rcl,rct,rcw,rch, BAR_MIN_POS, BAR_MAX_POS, BAR_MAX_POS, 0)

#define LDEF_SPECTRUM_PEAK(root,var_bmp,nxt_obj,l,t,w,h)    \
        DEF_BITMAP(var_bmp, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0,0,0,0,0, l,t,w,h, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX,\
        NULL, NULL, \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, IM_SPECTRAM_BLUE)

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
LDEF_CON(spectrum_con, &spectrum_bar_0, 0, WIN_SPECTRUM_L, WIN_SPECTRUM_T, WIN_SPECTRUM_W, WIN_SPECTRUM_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_0, &spectrum_peak_0, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*0, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_0,&spectrum_bar_1,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*0,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_1, &spectrum_peak_1, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*1, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_1,&spectrum_bar_2,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*1,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_2, &spectrum_peak_2, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*2, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_2,&spectrum_bar_3,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*2,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_3, &spectrum_peak_3, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*3, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_3,&spectrum_bar_4,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*3,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_4, &spectrum_peak_4, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*4, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_4,&spectrum_bar_5,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*4,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_5, &spectrum_peak_5, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*5, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_5,&spectrum_bar_6,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*5,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

LDEF_SPECTRUM_BAR(&spectrum_con,  spectrum_bar_6, &spectrum_peak_6, \
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*6, WIN_SPECTRUM_BAR_T, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, \
        0, 0, WIN_SPECTRUM_BAR_W, WIN_SPECTRUM_BAR_H, B_MID_SH_IDX)

LDEF_SPECTRUM_PEAK(&spectrum_con,spectrum_peak_6,NULL,\
        WIN_SPECTRUM_BAR_L+WIN_SPECTUM_BAR_SPLIT*6,    WIN_SPECTRUM_PEAK_T,WIN_SPECTRUM_BAR_W,WIN_SPECTRUM_PEAK_H)

/*******************************************************************************
*    Local functions & variables declare
*******************************************************************************/
typedef struct SPECTRUM_INFO
{
    UINT16 bar_peak[SPECTRUM_BAR_CNT];
    UINT16 bar_step[SPECTRUM_BAR_CNT];
    INT32 bar_precent[SPECTRUM_BAR_CNT];
}spectrum_info;
static spectrum_info spectrum_infor = {{0},{0},{0}};
static UINT16 update_callback_count = 0;
static PROGRESS_BAR *barlst_items[] =
{
    &spectrum_bar_0,
    &spectrum_bar_1,
    &spectrum_bar_2,
    &spectrum_bar_3,
    &spectrum_bar_4,
    &spectrum_bar_5,
    &spectrum_bar_6,
};

static BITMAP *peaklst_items[] =
{
    &spectrum_peak_0,
    &spectrum_peak_1,
    &spectrum_peak_2,
    &spectrum_peak_3,
    &spectrum_peak_4,
    &spectrum_peak_5,
    &spectrum_peak_6,
};

static void init_bars(void)
{
    UINT16 i = 0;

    for(i=0; i<SPECTRUM_BAR_CNT; i++)
    {
        spectrum_infor.bar_peak[i] = 0;
        spectrum_infor.bar_step[i] = SPECTRUM_BAR_STEP;
        spectrum_infor.bar_precent[i] = 0;
    }
    return;
}

static void calc_bars(void)
{
    //static UINT16 i = 0;
    UINT16 i = 0;
    INT32 val = 0;

    //static UINT16 val_i = 0;
    // calc peak of bar

    for(i=0; i<SPECTRUM_BAR_CNT; i++)
    {
        if(spectrum_infor.bar_precent[i] > spectrum_infor.bar_peak[i]) // is new peak?
        {
            spectrum_infor.bar_peak[i] = spectrum_infor.bar_precent[i];
            // reset step
            spectrum_infor.bar_step[i] = SPECTRUM_BAR_STEP;
        }
        else
        {
            val = spectrum_infor.bar_peak[i] - spectrum_infor.bar_step[i];

            if(val < 0)
            {
                val = 0;
            }

            if(val < spectrum_infor.bar_precent[i])
            {
                spectrum_infor.bar_peak[i] = (UINT16)spectrum_infor.bar_precent[i];
                // reset step
                spectrum_infor.bar_step[i] = SPECTRUM_BAR_STEP;
            }
            else
            {
                spectrum_infor.bar_peak[i] = (UINT16)val;
                spectrum_infor.bar_step[i] = spectrum_infor.bar_step[i]*2;
            }
        }

    }
    return;
}

void update_bars(UINT32 param)
{
    if(get_mute_state())
    {
        mute_win_spectrum();
    }
    else
    {
        // calc
        calc_bars();
        // update bars
        update_win_spectrum();
    }
}

static void update_callback(UINT32 param)
{
    INT32 *precent = (INT32 *)param;

    update_callback_count++;
    if(0 == update_callback_count%3)
    {
#ifdef DUAL_ENABLE
        osal_cache_invalidate(precent, sizeof(spectrum_infor.bar_precent));
#endif
        MEMCPY(spectrum_infor.bar_precent, precent, sizeof(spectrum_infor.bar_precent));
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SPECTRUM_UPDATE,param,FALSE);
        osal_task_sleep(5);
    }
}

typedef void (*spectrum_callback)(INT32*);

BOOL open_spectrum_dev(void)
{
    spec_param param;

    MEMSET(&param,0,sizeof(spec_param));
    init_bars();
    param.collumn_num = SPECTRUM_BAR_CNT;
    param.spec_call_back = (spectrum_callback)update_callback;
    snd_io_control(g_snd_dev, SND_SPECTRUM_START, (UINT32)&param);

    return TRUE;
}


BOOL close_spectrum_dev(void)
{
    snd_io_control(g_snd_dev, SND_SPECTRUM_STOP, 0);
    return TRUE;
}

#if 0
BOOL update_win_spectrum()
{
    UINT16 i, val;
    POBJECT_HEAD p_obj;
    PPROGRESS_BAR p_bar;

    p_obj = (POBJECT_HEAD)&spectrum_bar_con;

    for(i=0; i<SPECTRUM_BAR_CNT; i++)
    {
        osd_move_object(p_obj,WIN_SPECTRUM_BAR_L+i*WIN_SPECTUM_BAR_SPLIT,WIN_SPECTRUM_BARCON_T,0);
        val = (bar_precent[i]*TOTAL_BLOCKS)/SPEC_MAX;

/*        pBar = &spectrum_bar_0;
        OSD_SetProgressBarValue(pBar, BAR_MAX_POS);
        pBar = &spectrum_bar_1;
        OSD_SetProgressBarValue(pBar, BAR_MAX_POS);
        pBar = &spectrum_bar_2;
        OSD_SetProgressBarValue(pBar, BAR_MAX_POS);
*/
        if(val <= BAR2_BLOCKS)
        {
            p_bar = &spectrum_bar_0;
            osd_set_progress_bar_value(p_bar, 0);
            p_bar = &spectrum_bar_1;
            osd_set_progress_bar_value(p_bar, 0);
            p_bar = &spectrum_bar_2;
            osd_set_progress_bar_value(p_bar, (val*BAR_MAX_POS)/BAR2_BLOCKS);
        }
        else if(val <= (BAR2_BLOCKS+BAR1_BLOCKS))
        {
            p_bar = &spectrum_bar_0;
            osd_set_progress_bar_value(p_bar, 0);
            p_bar = &spectrum_bar_1;
            osd_set_progress_bar_value(p_bar, (val-BAR2_BLOCKS)*BAR_MAX_POS/BAR1_BLOCKS);
            p_bar = &spectrum_bar_2;
            osd_set_progress_bar_value(p_bar, BAR_MAX_POS);
        }
        else
        {
            if(val > TOTAL_BLOCKS)
                val = TOTAL_BLOCKS;
            p_bar = &spectrum_bar_0;
            osd_set_progress_bar_value(p_bar, (val-BAR1_BLOCKS-BAR2_BLOCKS)*BAR_MAX_POS/BAR0_BLOCKS);
            p_bar = &spectrum_bar_1;
            osd_set_progress_bar_value(p_bar, BAR_MAX_POS);
            p_bar = &spectrum_bar_2;
            osd_set_progress_bar_value(p_bar, BAR_MAX_POS);
        }

        osd_draw_object(p_obj, C_UPDATE_ALL);
    }
    return TRUE;
}

#else

static BOOL update_win_spectrum(void)
{
    UINT16 i = 0;
    UINT16 val = 0;
    POBJECT_HEAD pobj = NULL;
    PPROGRESS_BAR pbar = NULL;
    PBITMAP    pbmp = NULL;

    pobj = (POBJECT_HEAD)&spectrum_con;

    for(i=0; i<SPECTRUM_BAR_CNT; i++)
    {
        pbar = barlst_items[i];
        val = spectrum_infor.bar_precent[i];
        osd_set_progress_bar_value(pbar,val);

        pbmp = peaklst_items[i];
        osd_move_object((POBJECT_HEAD)pbmp,WIN_SPECTRUM_BAR_L+i*WIN_SPECTUM_BAR_SPLIT,
                WIN_SPECTRUM_PEAK_T-((spectrum_infor.bar_peak[i]*120)/100),0);
    }

    osd_draw_object(pobj, C_UPDATE_ALL);
    return TRUE;
}

static BOOL mute_win_spectrum(void)
{
    UINT16 i = 0;
    //UINT16 val = 0;
    POBJECT_HEAD pobj = NULL;
    PPROGRESS_BAR pbar = NULL;
    PBITMAP    pbmp = NULL;

    pobj = (POBJECT_HEAD)&spectrum_con;

    for(i=0; i<SPECTRUM_BAR_CNT; i++)
    {
        pbar = barlst_items[i];
        osd_set_progress_bar_value(pbar,0);

        pbmp = peaklst_items[i];
        osd_move_object((POBJECT_HEAD)pbmp,WIN_SPECTRUM_BAR_L+i*WIN_SPECTUM_BAR_SPLIT,WIN_SPECTRUM_PEAK_T,0);
    }

    osd_draw_object(pobj, C_UPDATE_ALL);
    return TRUE;
}

#endif

#endif

