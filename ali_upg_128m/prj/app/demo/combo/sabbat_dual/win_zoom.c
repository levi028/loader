/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_zoom.c
*
*    Description: zoom menu(press botton "ZOOM").
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
#include <hld/dis/vpo.h>


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

#include "win_zoom.h"

/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/

//extern CONTAINER g_win_zoom;

static VACTION win_zoom_keymap(POBJECT_HEAD pobj,UINT32 key);
static PRESULT win_zoom_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

/*******************************************************************************
*   WINDOW's objects defintion MACRO
*******************************************************************************/
#define ZOOM_WIN_SH_IDX        WSTL_WIN_SLIST_02_8BIT//sharon WSTL_ZOOM_WIN_8BIT
#define ZOOM_TITLE_SH_IDX      WSTL_MIXBACK_IDX_04_8BIT
#define ZOOM_BIGRECT_SH_IDX    WSTL_ZOOM_WIN_01_RECT
#define ZOOM_SMALLRECT_SH_IDX  WSTL_ZOOM_WIN_01_RECT

#define LDEF_BMP(root,var_bmp,l,t,w,h,sh,icon)   \
    DEF_BITMAP(var_bmp,root,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, ZOOM_TITLE_SH_IDX,ZOOM_TITLE_SH_IDX,\
        ZOOM_TITLE_SH_IDX,ZOOM_TITLE_SH_IDX,   \
        NULL,NULL, C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_WIN(var_wnd,nxt_obj,focus_id,l,t,w,h)     \
    DEF_CONTAINER(var_wnd,nxt_obj,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, ZOOM_WIN_SH_IDX,ZOOM_WIN_SH_IDX,ZOOM_WIN_SH_IDX,\
    ZOOM_WIN_SH_IDX, win_zoom_keymap,win_zoom_callback, nxt_obj, focus_id,0)

#ifndef SD_UI
#define W_L 722//426
#define W_T 380//286
#define W_W 244//214//150
#define W_H 250//220//140


#define TITLE_L (W_L + 16)
#define TITLE_T (W_T + 10)
#define TITLE_W (W_W - 32) //180
#define TITLE_H 40 //68//24


#define TV_R_L  (W_L + 26)
#define TV_R_T  (W_T + 80)
#define TV_R_W  184
#define TV_R_H  140


#define ZOOM_BMP_L  (TV_R_L + (TV_R_W - ZOOM_BMP_W)/2)
#define ZOOM_BMP_T  (TV_R_T + (TV_R_H - ZOOM_BMP_H)/2)
#define ZOOM_BMP_W  21//22
#define ZOOM_BMP_H  21//22

#define ZOOM_LARROW_L ( TV_R_L - 8)
#define ZOOM_LARROW_T ( TV_R_T + (TV_R_H - ZOOM_LARROW_H)/2)
#define ZOOM_LARROW_W 6
#define ZOOM_LARROW_H 10

#define ZOOM_TARROW_L (TV_R_L + (TV_R_W - ZOOM_TARROW_W)/2)
#define ZOOM_TARROW_T ( TV_R_T - 8)
#define ZOOM_TARROW_W 10
#define ZOOM_TARROW_H 6

#define ZOOM_RARROW_L ( TV_R_L + TV_R_W + 2)
#define ZOOM_RARROW_T ( TV_R_T + (TV_R_H - ZOOM_RARROW_H)/2)
#define ZOOM_RARROW_W 6
#define ZOOM_RARROW_H 10

#define ZOOM_DARROW_L (TV_R_L + (TV_R_W - ZOOM_DARROW_W)/2)
#define ZOOM_DARROW_T ( TV_R_T + TV_R_H + 2)
#define ZOOM_DARROW_W 10
#define ZOOM_DARROW_H 6
#else
#define W_L 400//426
#define W_T 270//286
#define W_W 165//214//150
#define W_H 166//220//140

#define TITLE_L (W_L + 16)
#define TITLE_T (W_T + 10)
#define TITLE_W (W_W - 32) //180
#define TITLE_H 24//68//24


#define TV_R_L  (W_L + 23)
#define TV_R_T  (W_T + 62)
#define TV_R_W  120
#define TV_R_H  80


#define ZOOM_BMP_L  (TV_R_L + (TV_R_W - ZOOM_BMP_W)/2)
#define ZOOM_BMP_T  (TV_R_T + (TV_R_H - ZOOM_BMP_H)/2)
#define ZOOM_BMP_W  21//22
#define ZOOM_BMP_H  21//22

#define ZOOM_LARROW_L ( TV_R_L - 8)
#define ZOOM_LARROW_T ( TV_R_T + (TV_R_H - ZOOM_LARROW_H)/2)
#define ZOOM_LARROW_W 6
#define ZOOM_LARROW_H 10

#define ZOOM_TARROW_L (TV_R_L + (TV_R_W - ZOOM_TARROW_W)/2)
#define ZOOM_TARROW_T ( TV_R_T - 8)
#define ZOOM_TARROW_W 10
#define ZOOM_TARROW_H 6

#define ZOOM_RARROW_L ( TV_R_L + TV_R_W + 2)
#define ZOOM_RARROW_T ( TV_R_T + (TV_R_H - ZOOM_RARROW_H)/2)
#define ZOOM_RARROW_W 6
#define ZOOM_RARROW_H 10

#define ZOOM_DARROW_L (TV_R_L + (TV_R_W - ZOOM_DARROW_W)/2)
#define ZOOM_DARROW_T ( TV_R_T + TV_R_H + 2)
#define ZOOM_DARROW_W 10
#define ZOOM_DARROW_H 6

#endif

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
LDEF_BMP(&g_win_zoom,win_zoom_bmp,ZOOM_BMP_L,ZOOM_BMP_T,ZOOM_BMP_W,\
            ZOOM_BMP_H,WSTL_NOSHOW_IDX,IM_ZOOM_IN)
LDEF_BMP(&g_win_zoom,win_zoom_larrow,ZOOM_LARROW_L,ZOOM_LARROW_T,\
            ZOOM_LARROW_W,ZOOM_LARROW_H,ZOOM_TITLE_SH_IDX,IM_ZOOM_LEFT)
LDEF_BMP(&g_win_zoom,win_zoom_tarrow,ZOOM_TARROW_L,ZOOM_TARROW_T,\
            ZOOM_TARROW_W,ZOOM_TARROW_H,ZOOM_TITLE_SH_IDX,IM_ZOOM_UP)
LDEF_BMP(&g_win_zoom,win_zoom_rarrow,ZOOM_RARROW_L,ZOOM_RARROW_T,\
            ZOOM_RARROW_W,ZOOM_RARROW_H,ZOOM_TITLE_SH_IDX,IM_ZOOM_RIGHT)
LDEF_BMP(&g_win_zoom,win_zoom_darrow,ZOOM_DARROW_L,ZOOM_DARROW_T,\
            ZOOM_DARROW_W,ZOOM_DARROW_H,ZOOM_TITLE_SH_IDX,IM_ZOOM_DOWN)

LDEF_TXT(&g_win_zoom,win_zoom_title,NULL,TITLE_L,TITLE_T,TITLE_W, TITLE_H,0,display_strs[0])

LDEF_WIN(g_win_zoom,NULL,0,W_L,W_T,W_W,W_H)
/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/

#define TV_DEST_L   0//32
#define TV_DEST_T   0//96
#define TV_DEST_W   (PICTURE_WIDTH - TV_DEST_L*2)
#define TV_DEST_H   (PICTURE_HEIGHT - TV_DEST_T*2)

#define TV_SRC_L    32
#define TV_SRC_T    96
#define TV_SRC_W    (PICTURE_WIDTH - TV_SRC_L*2)
#define TV_SRC_H    (PICTURE_HEIGHT - TV_SRC_T*2)

#define TV_FULL_SRC_L   0
#define TV_FULL_SRC_T   0
#define TV_FULL_SRC_W   PICTURE_WIDTH
#define TV_FULL_SRC_H   PICTURE_HEIGHT

#define TV_FULL_DEST_L   0
#define TV_FULL_DEST_T   0
#define TV_FULL_DEST_W   PICTURE_WIDTH
#define TV_FULL_DEST_H   PICTURE_HEIGHT

#define MAX_STEP_PIXS   3

#define VACT_MOVE_LEFT  (VACT_PASS + 1)
#define VACT_MOVE_RIGHT     (VACT_PASS + 2)
#define VACT_MOVE_UP        (VACT_PASS + 3)
#define VACT_MOVE_DOWN  (VACT_PASS + 4)
#define VACT_MOVE_STEP  (VACT_PASS + 5)

static UINT8 zoom_int_scales[] =
{
//    1,
    2,
    4,
    6,
    8,
    12,
    16
};

static double zoom_scales[] =
{
//    1.0,
    1.41421,
    2.0,
    2.44949,
    2.82843,
    3.46410,
    4.0
};

#define ZOOM_MAX_TIMES  ( sizeof(zoom_int_scales)/sizeof(zoom_int_scales[0]) )

static UINT8 zoom_times = 0;
static struct position osd_center_pos ;//x,y

static void get_tv_osd_src(struct rect *tv_src_rect, struct osdrect *osd_src_rect);
static PRESULT win_zoom_proc(VACTION act);

/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/

static void get_tv_osd_src(struct rect *tv_src_rect,struct osdrect *osd_src_rect)
{
    double zoom_scale = 0;
    UINT32 half_width = 0;
    UINT32 half_height = 0;

    if((NULL == tv_src_rect) || (NULL == osd_src_rect))
    {
        return ;
    }
    zoom_scale = zoom_scales[zoom_times];
    osd_src_rect->u_width    = (UINT16)(TV_R_W / zoom_scale);
    osd_src_rect->u_height   = (UINT16)(TV_R_H / zoom_scale);
    half_width  = osd_src_rect->u_width/2;
    half_height = osd_src_rect->u_height/2;

    if(osd_center_pos.u_x < half_width)
    {
        osd_center_pos.u_x = half_width;
    }
    if(osd_center_pos.u_x> (TV_R_W - half_width) )
    {
        osd_center_pos.u_x = (TV_R_W - half_width);
    }
    if(osd_center_pos.u_y < half_height)
    {
        osd_center_pos.u_y = half_height;
    }
    if(osd_center_pos.u_y > (TV_R_H - half_height) )
    {
        osd_center_pos.u_y = TV_R_H - half_height;
    }
    osd_src_rect->u_left = TV_R_L + ( osd_center_pos.u_x - half_width);
    osd_src_rect->u_top  = TV_R_T + ( osd_center_pos.u_y - half_height);

    tv_src_rect->u_width     = osd_src_rect->u_width*TV_SRC_W/TV_R_W;
    tv_src_rect->u_height    = osd_src_rect->u_height*TV_SRC_H/TV_R_H;
    tv_src_rect->u_start_x    = TV_SRC_L +  (osd_src_rect->u_left - TV_R_L)*TV_SRC_W/TV_R_W;
    tv_src_rect->u_start_y    = TV_SRC_T +  (osd_src_rect->u_top - TV_R_T)*TV_SRC_H/TV_R_H;

#if 0
    if(0 == zoom_times)
    {
        tv_src_rect->u_start_x = TV_FULL_SRC_L;
        tv_src_rect->u_start_y = TV_FULL_SRC_T;
        tv_src_rect->u_width  = TV_FULL_SRC_W;
        tv_src_rect->u_height = TV_FULL_SRC_H;
    }
#endif
}

static void win_zoom_draw_tv_zoom_rect(void)
{
    struct rect tv_src_rect;
    struct rect tv_dest_rect;
    char str[30] = {0};
    POBJECT_HEAD obj = NULL;
    const UINT32 max_scale_times = 16;

    struct osdrect osd_dest_rect;
    struct osdrect osd_src_rect;
    CONTAINER *  __MAYBE_UNUSED__ win=NULL;

    MEMSET(&tv_src_rect,0,sizeof(struct rect));
    MEMSET(&tv_dest_rect,0,sizeof(struct rect));
    MEMSET(&osd_dest_rect,0,sizeof(struct osdrect));
    MEMSET(&osd_src_rect,0,sizeof(struct osdrect));
    win = &g_win_zoom;

    obj = (POBJECT_HEAD)(&g_win_zoom);
    //osd_draw_object_frame(&obj->frame,ZOOM_WIN_SH_IDX);
    //osd_draw_object(obj, C_UPDATE_ALL);

    get_tv_osd_src(&tv_src_rect,&osd_src_rect);

    snprintf(str,30, "ZOOM   X%d",zoom_int_scales[zoom_times]);
    osd_set_text_field_content(&win_zoom_title,STRING_ANSI,(UINT32)str);
    osd_draw_object( (POBJECT_HEAD)&win_zoom_title, C_UPDATE_ALL);

#if 1

    osd_set_rect(&osd_dest_rect, TV_R_L, TV_R_T, TV_R_W, TV_R_H);
    osd_draw_object_frame(&osd_dest_rect,ZOOM_BIGRECT_SH_IDX);

    if(osd_src_rect.u_left>TV_R_L)
    {
        osd_draw_object((POBJECT_HEAD)&win_zoom_larrow, C_UPDATE_ALL);
    }
    if(osd_src_rect.u_top>TV_R_T)
    {
        osd_draw_object((POBJECT_HEAD)&win_zoom_tarrow, C_UPDATE_ALL);
    }
    if( (osd_src_rect.u_left + osd_src_rect.u_width)< (TV_R_L + TV_R_W) )
    {
        osd_draw_object((POBJECT_HEAD)&win_zoom_rarrow, C_UPDATE_ALL);
    }
    if( (osd_src_rect.u_top + osd_src_rect.u_height) < (TV_R_T + TV_R_H) )
    {
        osd_draw_object((POBJECT_HEAD)&win_zoom_darrow, C_UPDATE_ALL);
    }
    osd_draw_object_frame(&osd_src_rect,ZOOM_SMALLRECT_SH_IDX);

    obj = (POBJECT_HEAD)&win_zoom_bmp;
    osd_set_rect(&obj->frame, TV_R_L + osd_center_pos.u_x - ZOOM_BMP_W/2 , \
        TV_R_T + osd_center_pos.u_y - ZOOM_BMP_H/2, ZOOM_BMP_W, ZOOM_BMP_H);
    osd_draw_object(obj, C_UPDATE_ALL);

    tv_dest_rect.u_start_x    = TV_DEST_L;
    tv_dest_rect.u_start_y    = TV_DEST_T;
    tv_dest_rect.u_width     = TV_DEST_W;
    tv_dest_rect.u_height    = TV_DEST_H;

#if 0
    if(0 == zoom_times)
    {
        tv_dest_rect.u_start_x = TV_FULL_DEST_L;
        tv_dest_rect.u_start_y = TV_FULL_DEST_T;
        tv_dest_rect.u_width  = TV_FULL_DEST_W;
        tv_dest_rect.u_height = TV_FULL_DEST_H;
    }

    if(0 == zoom_times)
    {
        tv_src_rect.u_start_x = TV_FULL_SRC_L;
        tv_src_rect.u_start_y = TV_FULL_SRC_T;
        tv_src_rect.u_width  = TV_FULL_SRC_W;
        tv_src_rect.u_height = TV_FULL_SRC_H;
    }
#endif

    if (tv_src_rect.u_start_x % max_scale_times)
    {
        tv_src_rect.u_start_x &= 0xfff0;
    }
    if (tv_src_rect.u_width % max_scale_times)
    {
        tv_src_rect.u_width += 15;
        tv_src_rect.u_width &= 0xfff0;
    }

    vpo_zoom(g_vpo_dev,&tv_src_rect,&tv_dest_rect);
#ifdef DUAL_VIDEO_OUTPUT
    vpo_zoom(g_sd_vpo_dev,&tv_src_rect,&tv_dest_rect);
#endif
#endif

}

static VACTION win_zoom_keymap(POBJECT_HEAD pobj,UINT32 key)
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

static PRESULT win_zoom_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT proc_result = PROC_PASS;
    VACTION unact = 0;
    struct rect tv_src_rect;
    struct rect tv_dest_rect;
    struct osdrect osd_src_rect;

    MEMSET(&tv_src_rect,0,sizeof(struct rect));
    MEMSET(&tv_dest_rect,0,sizeof(struct rect));
    MEMSET(&osd_src_rect,0,sizeof(struct osdrect));
    get_tv_osd_src(&tv_src_rect,&osd_src_rect);

    switch(event)
    {
    case EVN_PRE_OPEN:
        api_inc_wnd_count();
        zoom_times = 0;
        osd_center_pos.u_x = TV_R_W/2;
        osd_center_pos.u_y = TV_R_H/2;
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        tv_src_rect.u_start_x = TV_FULL_SRC_L;
        tv_src_rect.u_start_y = TV_FULL_SRC_T;
        tv_src_rect.u_width  = TV_FULL_SRC_W;
        tv_src_rect.u_height = TV_FULL_SRC_H;

        tv_dest_rect.u_start_x    = TV_FULL_DEST_L;//TV_DEST_L;
        tv_dest_rect.u_start_y    = TV_FULL_DEST_T;//TV_DEST_T;
        tv_dest_rect.u_width     = TV_FULL_DEST_W;//TV_DEST_W;
        tv_dest_rect.u_height    = TV_FULL_DEST_H;//TV_DEST_H;
        vpo_zoom(g_vpo_dev,&tv_src_rect,&tv_dest_rect);
#ifdef DUAL_VIDEO_OUTPUT
        vpo_zoom(g_sd_vpo_dev,&tv_src_rect,&tv_dest_rect);
#endif
        api_dec_wnd_count();
#if defined( SUPPORT_CAS9) ||defined (SUPPORT_CAS7)
        restore_ca_msg_when_exit_win();
#endif
        break;
    case EVN_PRE_DRAW:
        //win_zoom_draw_tv_zoom_rect();
        //proc_result = PROC_LOOP;
        break;
    case EVN_POST_DRAW:
        win_zoom_draw_tv_zoom_rect();
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        proc_result = win_zoom_proc(unact);
        break;
    default :
        break;
    }

    return proc_result;
}

static PRESULT win_zoom_proc(VACTION act)
{
    PRESULT proc_result = PROC_LOOP;
    struct osdrect osd_src_rect;
    struct rect tv_src_rect;
    int step = 0;

    MEMSET(&osd_src_rect,0,sizeof(struct osdrect));
    MEMSET(&tv_src_rect,0,sizeof(struct rect));
    get_tv_osd_src(&tv_src_rect,&osd_src_rect);

    switch(act)
    {
    case VACT_MOVE_STEP:
        zoom_times++;
        zoom_times %= ZOOM_MAX_TIMES;
        if(0 == zoom_times)
        {
            proc_result = PROC_LEAVE;
        }
        break;
    case VACT_MOVE_LEFT:
        step = osd_src_rect.u_left - TV_R_L;
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
            {
                step = MAX_STEP_PIXS;
            }
        osd_center_pos.u_x -= step;
        }
        break;
    case VACT_MOVE_UP:
        step = osd_src_rect.u_top - TV_R_T;
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
            {
                step = MAX_STEP_PIXS;
            }
            osd_center_pos.u_y -= step;
        }

        break;
    case VACT_MOVE_RIGHT:
        step = ( TV_R_L + TV_R_W ) - (osd_src_rect.u_left + osd_src_rect.u_width);
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
            {
                step = MAX_STEP_PIXS;
            }
            osd_center_pos.u_x += step;
        }
        break;
    case VACT_MOVE_DOWN:
        step = ( TV_R_T + TV_R_H ) - (osd_src_rect.u_top + osd_src_rect.u_height);
        if(step>0)
        {
            if(step>MAX_STEP_PIXS)
            {
                step = MAX_STEP_PIXS;
            }
            osd_center_pos.u_y += step;
        }
        break;
    default :
        break;
    }

    //win_zoom_draw_tv_zoom_rect();
    osd_draw_object((POBJECT_HEAD)&g_win_zoom, C_DRAW_SIGN_EVN_FLG |C_UPDATE_ALL);

    return proc_result;
}

