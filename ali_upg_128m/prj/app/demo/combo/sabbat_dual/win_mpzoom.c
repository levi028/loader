/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_mpzoom.c
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
#include <hld/dis/vpo.h>

#include <api/libosd/osd_lib.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/pe.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_mpzoom.h"

/*******************************************************************************
*    WINDOW's objects defintion MACRO
*******************************************************************************/
#define MPZOOM_WIN_SH_IDX        WSTL_WIN_SLIST_02_8BIT  //WSTL_ZOOM_WIN_01
#define MPZOOM_TITLE_SH_IDX      WSTL_MIXBACK_IDX_04_8BIT  //WSTL_MIXBACK_BLACK_IDX
#define MPZOOM_BIGRECT_SH_IDX    WSTL_ZOOM_WIN_01_RECT
#define MPZOOM_SMALLRECT_SH_IDX  WSTL_ZOOM_WIN_01_RECT
#ifndef SD_UI
#define MPZOOM_W_L 722//682//818//426
#define MPZOOM_W_T 380//400//440//286
#define MPZOOM_W_W 244//214//150
#define MPZOOM_W_H 250//220//140

#define MPZOOM_TITLE_L (MPZOOM_W_L + 16)
#define MPZOOM_TITLE_T (MPZOOM_W_T + 10)
#define MPZOOM_TITLE_W (MPZOOM_W_W - 32)//120
#define MPZOOM_TITLE_H 40//24

#define IMG_R_L  (MPZOOM_W_L + 26)
#define IMG_R_T  (MPZOOM_W_T + 80)
#define IMG_R_W  184//160//120
#define IMG_R_H  140//80

#define ZOOM_BMP_L  (IMG_R_L + (IMG_R_W - ZOOM_BMP_W)/2)
#define ZOOM_BMP_T  (IMG_R_T + (IMG_R_H - ZOOM_BMP_H)/2)
#define ZOOM_BMP_W  21//22
#define ZOOM_BMP_H  21//22

#define ZOOM_LARROW_L ( IMG_R_L - 8)
#define ZOOM_LARROW_T ( IMG_R_T + (IMG_R_H - ZOOM_LARROW_H)/2)
#define ZOOM_LARROW_W 6
#define ZOOM_LARROW_H 10

#define ZOOM_TARROW_L (IMG_R_L + (IMG_R_W - ZOOM_TARROW_W)/2)
#define ZOOM_TARROW_T ( IMG_R_T - 8)
#define ZOOM_TARROW_W 10
#define ZOOM_TARROW_H 6

#define ZOOM_RARROW_L ( IMG_R_L + IMG_R_W + 2)
#define ZOOM_RARROW_T ( IMG_R_T + (IMG_R_H - ZOOM_RARROW_H)/2)
#define ZOOM_RARROW_W 6
#define ZOOM_RARROW_H 10

#define ZOOM_DARROW_L (IMG_R_L + (IMG_R_W - ZOOM_DARROW_W)/2)
#define ZOOM_DARROW_T ( IMG_R_T + IMG_R_H + 2)
#define ZOOM_DARROW_W 10
#define ZOOM_DARROW_H 6
#else
#define MPZOOM_W_L 400//682//818//426
#define MPZOOM_W_T 270//400//440//286
#define MPZOOM_W_W 165//214//150
#define MPZOOM_W_H 166//220//140

#define MPZOOM_TITLE_L (MPZOOM_W_L + 16)
#define MPZOOM_TITLE_T (MPZOOM_W_T + 10)
#define MPZOOM_TITLE_W (MPZOOM_W_W - 32)//120
#define MPZOOM_TITLE_H 24//24

#define IMG_R_L  (MPZOOM_W_L + 23)
#define IMG_R_T  (MPZOOM_W_T + 62)
#define IMG_R_W  120//160//120
#define IMG_R_H  80//80

#define ZOOM_BMP_L  (IMG_R_L + (IMG_R_W - ZOOM_BMP_W)/2)
#define ZOOM_BMP_T  (IMG_R_T + (IMG_R_H - ZOOM_BMP_H)/2)
#define ZOOM_BMP_W  21//22
#define ZOOM_BMP_H  21//22

#define ZOOM_LARROW_L ( IMG_R_L - 8)
#define ZOOM_LARROW_T ( IMG_R_T + (IMG_R_H - ZOOM_LARROW_H)/2)
#define ZOOM_LARROW_W 6
#define ZOOM_LARROW_H 10

#define ZOOM_TARROW_L (IMG_R_L + (IMG_R_W - ZOOM_TARROW_W)/2)
#define ZOOM_TARROW_T ( IMG_R_T - 8)
#define ZOOM_TARROW_W 10
#define ZOOM_TARROW_H 6

#define ZOOM_RARROW_L ( IMG_R_L + IMG_R_W + 2)
#define ZOOM_RARROW_T ( IMG_R_T + (IMG_R_H - ZOOM_RARROW_H)/2)
#define ZOOM_RARROW_W 6
#define ZOOM_RARROW_H 10

#define ZOOM_DARROW_L (IMG_R_L + (IMG_R_W - ZOOM_DARROW_W)/2)
#define ZOOM_DARROW_T ( IMG_R_T + IMG_R_H + 2)
#define ZOOM_DARROW_W 10
#define ZOOM_DARROW_H 6
#endif

/*******************************************************************************
*    WINDOW's objects declaration
*******************************************************************************/
#define LDEF_BMP(root,var_bmp,l,t,w,h,sh,icon)    \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, MPZOOM_TITLE_SH_IDX,MPZOOM_TITLE_SH_IDX,MPZOOM_TITLE_SH_IDX,MPZOOM_TITLE_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,str)
        
static VACTION win_mpzoom_keymap(POBJECT_HEAD pobj,UINT32 key);
static PRESULT win_mpzoom_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define LDEF_WIN(var_wnd,nxt_obj,focus_id,l,t,w,h)        \
    DEF_CONTAINER(var_wnd,nxt_obj,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MPZOOM_WIN_SH_IDX,MPZOOM_WIN_SH_IDX,MPZOOM_WIN_SH_IDX,MPZOOM_WIN_SH_IDX,   \
    win_mpzoom_keymap,win_mpzoom_callback,  \
    nxt_obj, focus_id,0)

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
LDEF_BMP(&g_win_mpzoom,win_mpzoom_bmp,ZOOM_BMP_L,ZOOM_BMP_T,ZOOM_BMP_W,ZOOM_BMP_H,WSTL_NOSHOW_IDX,IM_ZOOM_IN)
LDEF_BMP(&g_win_mpzoom,win_mpzoom_larrow,ZOOM_LARROW_L,ZOOM_LARROW_T, \
    ZOOM_LARROW_W,ZOOM_LARROW_H,MPZOOM_TITLE_SH_IDX,IM_ZOOM_LEFT)
LDEF_BMP(&g_win_mpzoom,win_mpzoom_tarrow,ZOOM_TARROW_L,ZOOM_TARROW_T, \
    ZOOM_TARROW_W,ZOOM_TARROW_H,MPZOOM_TITLE_SH_IDX,IM_ZOOM_UP)
LDEF_BMP(&g_win_mpzoom,win_mpzoom_rarrow,ZOOM_RARROW_L,ZOOM_RARROW_T, \
    ZOOM_RARROW_W,ZOOM_RARROW_H,MPZOOM_TITLE_SH_IDX,IM_ZOOM_RIGHT)
LDEF_BMP(&g_win_mpzoom,win_mpzoom_darrow,ZOOM_DARROW_L,ZOOM_DARROW_T, \
    ZOOM_DARROW_W,ZOOM_DARROW_H,MPZOOM_TITLE_SH_IDX,IM_ZOOM_DOWN)

LDEF_TXT(&g_win_mpzoom,win_mpzoom_title,NULL,MPZOOM_TITLE_L, \
    MPZOOM_TITLE_T,MPZOOM_TITLE_W,MPZOOM_TITLE_H,0,display_strs[0])

LDEF_WIN(g_win_mpzoom,NULL,0,MPZOOM_W_L,MPZOOM_W_T,MPZOOM_W_W,MPZOOM_W_H)
/*******************************************************************************
*    Local functions & variables declare
*******************************************************************************/
static UINT8 mpzoom_int_scales[] =
{
//    1,
    2,
    4,
    8,
    16
};

static double mpzoom_scales[] =
{
//    1.0,
    1.41421,
    2.0,
    2.82843,
    4.0
};

#define MPZOOM_MAX_TIMES  ( sizeof(mpzoom_int_scales)/sizeof(mpzoom_int_scales[0]) )

static UINT8 mpzoom_times = 0;
static struct position mposd_center_pos;//x,y


#define IMG_DEST_L   0//32
#define IMG_DEST_T   0//96
#define IMG_DEST_W   (720 - IMG_DEST_L*2)
#define IMG_DEST_H   (576 - IMG_DEST_T*2)

#define IMG_SRC_L    0//32
#define IMG_SRC_T    0//96
#define IMG_SRC_W    (720 - IMG_SRC_L*2)
#define IMG_SRC_H    (576 - IMG_SRC_T*2)

#define IMG_FULL_SRC_L   0
#define IMG_FULL_SRC_T   0
#define IMG_FULL_SRC_W   720
#define IMG_FULL_SRC_H   576

#define IMG_FULL_DEST_L   0
#define IMG_FULL_DEST_T   0
#define IMG_FULL_DEST_W   720
#define IMG_FULL_DEST_H   576

#define MAX_STEP_PIXS   3

#define VACT_MOVE_LEFT     (VACT_PASS + 1)
#define VACT_MOVE_RIGHT     (VACT_PASS + 2)
#define VACT_MOVE_UP         (VACT_PASS + 3)
#define VACT_MOVE_DOWN     (VACT_PASS + 4)
#define VACT_MOVE_STEP     (VACT_PASS + 5)

static void mpget_img_osd_src(struct rect *img_src_rect, struct osdrect *osd_src_rect);
static PRESULT win_mpzoom_proc(VACTION act);

/*******************************************************************************
*    Window's keymap, proc and  callback
*******************************************************************************/

static void mpget_img_osd_src(struct rect *img_src_rect,struct osdrect *osd_src_rect)
{
    double zoom_scale = 0;
    UINT32 half_width = 0;
    UINT32 half_height = 0;

    zoom_scale = mpzoom_scales[mpzoom_times];
    osd_src_rect->u_width    = (UINT16)(IMG_R_W / zoom_scale);
    osd_src_rect->u_height   = (UINT16)(IMG_R_H / zoom_scale);
    half_width  = osd_src_rect->u_width/2;
    half_height = osd_src_rect->u_height/2;

    if(mposd_center_pos.u_x < half_width)
    {
        mposd_center_pos.u_x = half_width;
    }
    if(mposd_center_pos.u_x> (IMG_R_W - half_width) )
    {
        mposd_center_pos.u_x = (IMG_R_W - half_width);
    }
    if(mposd_center_pos.u_y < half_height)
    {
        mposd_center_pos.u_y = half_height;
    }
    if(mposd_center_pos.u_y > (IMG_R_H - half_height) )
    {
        mposd_center_pos.u_y = IMG_R_H - half_height;
    }

    osd_src_rect->u_left = IMG_R_L + ( mposd_center_pos.u_x - half_width);
    osd_src_rect->u_top  = IMG_R_T + ( mposd_center_pos.u_y - half_height);

    img_src_rect->u_width     = osd_src_rect->u_width*IMG_SRC_W/IMG_R_W;
    img_src_rect->u_height    = osd_src_rect->u_height*IMG_SRC_H/IMG_R_H;
    img_src_rect->u_start_x    = IMG_SRC_L + (osd_src_rect->u_left - IMG_R_L)*IMG_SRC_W/IMG_R_W;
    img_src_rect->u_start_y    = IMG_SRC_T + (osd_src_rect->u_top - IMG_R_T)*IMG_SRC_H/IMG_R_H;

#if 0
    if(0==mpzoom_times)
    {
        img_src_rect->u_start_x = IMG_FULL_SRC_L;
        img_src_rect->u_start_y = IMG_FULL_SRC_T;
        img_src_rect->u_width  = IMG_FULL_SRC_W;
        img_src_rect->u_height = IMG_FULL_SRC_H;
    }
#endif
}

static void win_mpzoom_draw_tv_zoom_rect(void)
{
    struct rect img_src_rect;
    struct rect img_dest_rect;
    char str[30] ;
    //int n = 0;
    POBJECT_HEAD obj = NULL;
    struct osdrect osd_dest_rect;
    struct osdrect osd_src_rect ;
    CONTAINER *win = NULL;

	if(NULL == win)
	{
		;
	}
	MEMSET(&img_src_rect,0,sizeof(struct rect));
    MEMSET(&img_dest_rect,0,sizeof(struct rect));
    MEMSET(&osd_dest_rect,0,sizeof(struct osdrect));
    MEMSET(&osd_src_rect,0,sizeof(struct osdrect));
    MEMSET(str,0,sizeof(str));

    win = &g_win_mpzoom;

    mpget_img_osd_src(&img_src_rect,&osd_src_rect);

    snprintf(str,30,"ZOOM   X%d",mpzoom_int_scales[mpzoom_times]);
    osd_set_text_field_content(&win_mpzoom_title,STRING_ANSI,(UINT32)str);
    osd_draw_object( (POBJECT_HEAD)&win_mpzoom_title, C_UPDATE_ALL);

    osd_set_rect(&osd_dest_rect, IMG_R_L, IMG_R_T, IMG_R_W, IMG_R_H);
    osd_draw_object_frame(&osd_dest_rect,MPZOOM_BIGRECT_SH_IDX);

    if(osd_src_rect.u_left>IMG_R_L)
    {
        osd_draw_object((POBJECT_HEAD)&win_mpzoom_larrow, C_UPDATE_ALL);
     }

    if(osd_src_rect.u_top>IMG_R_T)
    {
        osd_draw_object((POBJECT_HEAD)&win_mpzoom_tarrow, C_UPDATE_ALL);
     }

    if( (osd_src_rect.u_left + osd_src_rect.u_width)< (IMG_R_L + IMG_R_W) )
    {
        osd_draw_object((POBJECT_HEAD)&win_mpzoom_rarrow, C_UPDATE_ALL);
    }

    if( (osd_src_rect.u_top + osd_src_rect.u_height) < (IMG_R_T + IMG_R_H) )
    {
        osd_draw_object((POBJECT_HEAD)&win_mpzoom_darrow, C_UPDATE_ALL);
    }

    osd_draw_object_frame(&osd_src_rect,MPZOOM_SMALLRECT_SH_IDX);

    obj = (POBJECT_HEAD)&win_mpzoom_bmp;
    osd_set_rect(&obj->frame, IMG_R_L + mposd_center_pos.u_x - ZOOM_BMP_W/2 ,\
            IMG_R_T + mposd_center_pos.u_y - ZOOM_BMP_H/2, ZOOM_BMP_W, ZOOM_BMP_H);
    osd_draw_object(obj, C_UPDATE_ALL);

    img_dest_rect.u_start_x    = IMG_DEST_L;
    img_dest_rect.u_start_y    = IMG_DEST_T;
    img_dest_rect.u_width     = IMG_DEST_W;
    img_dest_rect.u_height    = IMG_DEST_H;

#if 0
/*
    if(0==mpzoom_times)
    {
        img_dest_rect.uStartX    = IMG_FULL_DEST_L;
        img_dest_rect.uStartY    = IMG_FULL_DEST_T;
        img_dest_rect.uWidth     = IMG_FULL_DEST_W;
        img_dest_rect.uHeight    = IMG_FULL_DEST_H;
    }
*/
    if(0==mpzoom_times)
    {
        img_src_rect.u_start_x = IMG_FULL_SRC_L;
        img_src_rect.u_start_y = IMG_FULL_SRC_T;
        img_src_rect.u_width  = IMG_FULL_SRC_W;
        img_src_rect.u_height = IMG_FULL_SRC_H;
    }

//    img_src_rect.uStartX = 0;
//    img_src_rect.uStartY = 0;
//    img_src_rect.uWidth  = (720/mpzoom_int_scales[mpzoom_times]);
//    img_src_rect.uHeight = (576/mpzoom_int_scales[mpzoom_times]);
#endif
    image_zoom(&img_dest_rect, &img_src_rect);
}

static VACTION win_mpzoom_keymap(POBJECT_HEAD pobj,UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_MOVE_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_MOVE_RIGHT;
        break;
    case V_KEY_UP:
        act = VACT_MOVE_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_MOVE_DOWN;
        break;
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    case V_KEY_ZOOM:
        act = VACT_MOVE_STEP;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT win_mpzoom_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT proc_result = PROC_PASS;
    VACTION unact = 0;
    struct rect img_src_rect;
    struct rect img_dest_rect;
    struct osdrect osd_src_rect;

    MEMSET(&img_src_rect,0,sizeof(struct rect));
    MEMSET(&img_dest_rect,0,sizeof(struct rect));
    MEMSET(&osd_src_rect,0,sizeof(struct osdrect));

    mpget_img_osd_src(&img_src_rect,&osd_src_rect);

    switch(event)
    {
     case EVN_PRE_OPEN:
        mpzoom_times = 0;
        mposd_center_pos.u_x = IMG_R_W/2;
        mposd_center_pos.u_y = IMG_R_H/2;
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        img_src_rect.u_start_x = IMG_FULL_SRC_L;
        img_src_rect.u_start_y = IMG_FULL_SRC_T;
        img_src_rect.u_width  = IMG_FULL_SRC_W;
        img_src_rect.u_height = IMG_FULL_SRC_H;

        img_dest_rect.u_start_x    = IMG_FULL_DEST_L;
        img_dest_rect.u_start_y    = IMG_FULL_DEST_T;
        img_dest_rect.u_width     = IMG_FULL_DEST_W;
        img_dest_rect.u_height    = IMG_FULL_DEST_H;
        image_zoom(&img_dest_rect, &img_src_rect);
        break;
    case EVN_PRE_DRAW:
        break;
    case EVN_POST_DRAW:
        win_mpzoom_draw_tv_zoom_rect();
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        proc_result = win_mpzoom_proc(unact);
        break;
    default:
        break;

     }

    return proc_result;
}

static PRESULT win_mpzoom_proc(VACTION act)
{
    PRESULT proc_result = PROC_LOOP;
    struct osdrect osd_src_rect;
    struct rect img_src_rect;
    int step = 0;

    MEMSET(&osd_src_rect,0,sizeof(struct osdrect));
    MEMSET(&img_src_rect,0,sizeof(struct rect));
    mpget_img_osd_src(&img_src_rect,&osd_src_rect);

    switch(act)
    {
    case VACT_MOVE_STEP:
        mpzoom_times++;
        mpzoom_times %= MPZOOM_MAX_TIMES;
        if(0 == mpzoom_times)
        {
            proc_result = PROC_LEAVE;
        }
        break;
    case VACT_MOVE_LEFT:
        step = osd_src_rect.u_left - IMG_R_L;
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
        {
                step = MAX_STEP_PIXS;
        }
            mposd_center_pos.u_x -= step;
        }
        break;
    case VACT_MOVE_UP:
        step = osd_src_rect.u_top - IMG_R_T;
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
        {
                step = MAX_STEP_PIXS;
        }
            mposd_center_pos.u_y -= step;
        }
        break;
    case VACT_MOVE_RIGHT:
        step = ( IMG_R_L + IMG_R_W ) - (osd_src_rect.u_left + osd_src_rect.u_width);
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
        {
                step = MAX_STEP_PIXS;
        }
            mposd_center_pos.u_x += step;
        }
        break;
    case VACT_MOVE_DOWN:
        step = ( IMG_R_T + IMG_R_H ) - (osd_src_rect.u_top + osd_src_rect.u_height);
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
        {
                step = MAX_STEP_PIXS;
        }
            mposd_center_pos.u_y += step;
        }
        break;
    default:
        break;
    }

    osd_draw_object((POBJECT_HEAD)&g_win_mpzoom, C_DRAW_SIGN_EVN_FLG |C_UPDATE_ALL);

    return proc_result;
}

void win_mpzoom_open(void)
{
    UINT32 hkey = 0;
    CONTAINER *pop_win = NULL;
    PRESULT ret = PROC_LOOP;
    BOOL old_value = FALSE;

    old_value = ap_enable_key_task_get_key(TRUE);

    pop_win = &g_win_mpzoom;
    wincom_backup_region(&pop_win->head.frame);
    osd_obj_open((POBJECT_HEAD)pop_win, 0);
    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }

        ret = osd_obj_proc((POBJECT_HEAD)pop_win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);
}

#endif

