/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_diag_energy.c
*
*    Description: The realize of diag_energy
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
#else
#include <api/libpub/lib_frontend.h>
#endif
#include <hld/decv/decv.h>
#include <api/libosd/osd_lib.h>
#include <api/libtsi/psi_db.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_diag_energy.h"
#include "win_diagnosis.h"
#include "ctrl_key_proc.h"

#define QE_PRINTF(...)
#define RGB16(word) (0x8000|(word))
#define RGB24_RGB1555(R,G,B) (0x8000|(((R)>>3)<<10)    \
                  |(((G)>>3)<<5)|((B)>>3))
//RGB888 to ARGB1555

#define DIAG_COUNT 16

#define CAPTURE_BUFFER_LEN    (1024*4)
#define MOVE_SPEED_LOW    2
#define MOVE_SPEED_HIGH    50

#define FIGURE_HIGHT     400
#define FIGURE_WIDTH    512

#define COOR_WIDTH        510
#define COOR_HIGHT        400
#define COOR_TOP        (W_T+40)
#define COOR_LEFT        (W_L+100)
#define COOR_X_START    (COOR_LEFT+40)
#define COOR_Y_START    (COOR_TOP+410)

#define COOR_X_END        (COOR_X_START+FIGURE_WIDTH)
#define COOR_Y_END        (COOR_Y_START-FIGURE_HIGHT)

#define FIGURE_COLOR    RGB24_RGB1555(0,0,0)
#define COOR_COLOR        RGB24_RGB1555(0,0xff,0)
#define CXCY_COLOR        RGB24_RGB1555(0xff,0,0)
#define BG_COLOR        RGB16(0x635A)

#define W_L    80
#define W_T    10
#define W_W    900
#define W_H    600

#define TXT_INFO_W     90
#define TXT_INFO_H    44
#define TXT_INFO_T     (W_T + 30)
#define TXT_INFO_L     (W_L + 20)

#define TXT_FOCUS_W (W_W - 120)
#define TXT_FOCUS_H    44
#define TXT_FOCUS_T (W_T + 80 + COOR_HIGHT)
#define TXT_FOCUS_L (W_L + 60)

#define TXT_HELP_W     (W_W-120)
#define TXT_HELP_H    44
#define TXT_HELP_T     (TXT_FOCUS_T + 44)
#define TXT_HELP_L     TXT_FOCUS_L

#define PROG_WIN_SH_IDX         WSTL_POP_WIN_01_8BIT
#define TXT_INFO_IDX            WSTL_TEXT_30_8BIT

static UINT32 diag_capture_freq=0;
static UINT32 *diag_capture_buffer = NULL;
static UINT32 diag_capture_max_value=0;
static UINT8  diag_capture_max_v_parity=0;
static struct nim_device *diag_capture_nim=NULL;

static UINT32 virtual_dir_x=0; //COOR_Y_START; Horizontal Coordinate y position
static UINT32 virtual_dir_y=0; //COOR_X_START; Vertial Coordinate x position

static VACTION win_energy_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT win_energy_callback(POBJECT_HEAD pobj,\
                   VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER g_con_diag_energy;

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)            \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0,        \
              0,0,0,0,0, l,t,w,h, TXT_INFO_IDX,TXT_INFO_IDX,TXT_INFO_IDX,TXT_INFO_IDX, \
              NULL,NULL,                    \
              C_ALIGN_CENTER|C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_WIN(var_wnd,nxt_obj,l,t,w,h,sh,next_obj,focus_id)    \
    DEF_CONTAINER(var_wnd,NULL,NULL,C_ATTR_ACTIVE,0,        \
              0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,        \
              win_energy_keymap,win_energy_callback,    \
              next_obj, focus_id,1)

    LDEF_TXT(&g_con_diag_energy,text_eng_infoy,\
         &text_eng_focus,TXT_INFO_L, TXT_INFO_T,\
         TXT_INFO_W,TXT_INFO_H, display_strs[0])
    LDEF_TXT(&g_con_diag_energy,text_eng_focus,\
         &text_eng_help, TXT_FOCUS_L,TXT_FOCUS_T,\
         TXT_FOCUS_W,TXT_FOCUS_H,display_strs[1])
        LDEF_TXT(&g_con_diag_energy,text_eng_help, NULL,\
             TXT_HELP_L,TXT_HELP_T,TXT_HELP_W,TXT_HELP_H,display_strs[2])

    LDEF_WIN(g_con_diag_energy,NULL,W_L,W_T,\
         W_W,W_H,PROG_WIN_SH_IDX,&text_eng_infoy,1);


//;

void set_capture_freq(struct nim_device *nim, UINT32 freq)
{
    diag_capture_nim = nim;
    diag_capture_freq = freq;
}

static void get_energy(void)
{
    UINT32 i=0;
    UINT32 cap_param[3]={0};
    UINT32 temp=0;
    INT32  __MAYBE_UNUSED__ io_ret=0;

    cap_param[0] = diag_capture_freq;
    cap_param[1] = (UINT32)diag_capture_buffer;
    cap_param[2] = 0;

    diag_capture_max_value = 0;
    io_ret=nim_ioctl_ext(diag_capture_nim, \
                 NIM_DRIVER_START_CAPTURE, &cap_param);

    if(NULL == diag_capture_buffer)
    {
        ASSERT(0);
        return;
    }
    // Get Max data for energy;
    for(i = 0; i < 1024; i++)
    {
        if(diag_capture_max_value < diag_capture_buffer[i])
        {
            diag_capture_max_value = diag_capture_buffer[i];
            temp=i%2;
            if(temp)
            {
                diag_capture_max_v_parity = 1;
            }
            else
            {
                diag_capture_max_v_parity = 0;
            }
        }
    }
    QE_PRINTF(diag_capture_max_value);
    QE_PRINTF(diag_capture_max_v_parity);
}

static void draw_energy_figure(void)
{
    UINT32 i=0;
    UINT32 dwdottop=0;
    UINT32 color_r=0;
    UINT32 color_g=0;
    UINT32 color_b=0;


    if(NUM_ZERO == diag_capture_max_value)
    {
        return;
    }
    // Draw the energy figure
    for(i = 0; i < 512; i++)
    {
        dwdottop = diag_capture_buffer[i*2+diag_capture_max_v_parity]\
            *FIGURE_HIGHT/diag_capture_max_value;
        osd_draw_pixel(i+COOR_X_START,\
                  COOR_Y_START - dwdottop, \
                  RGB24_RGB1555(color_r,color_g,color_b), NULL);
    }
}

static void draw_virtual_cx(UINT32 color)
{
    if(virtual_dir_x != COOR_Y_START)
    {
        osd_draw_hor_line(COOR_X_START, virtual_dir_x, COOR_WIDTH, color, NULL);
    }
}

static void draw_virtual_cy(UINT32 color)
{
    if(virtual_dir_y != COOR_X_START)
    {
        osd_draw_ver_line(virtual_dir_y, COOR_TOP, COOR_HIGHT, color, NULL);
    }
}

static void draw_cx_cy_focus(void)
{
    UINT8 focus[32]={0};
    UINT32 x=0;
    UINT32 y=0;
    int  __MAYBE_UNUSED__ str_ret=0;

    if(virtual_dir_y < COOR_X_START)
    {
        virtual_dir_y = COOR_X_START;
    }

    if(virtual_dir_x > COOR_Y_START)
    {
        virtual_dir_x = COOR_Y_START;
    }
    x = (virtual_dir_y-COOR_X_START)*2;
    y = (COOR_Y_START-virtual_dir_x)*diag_capture_max_value/FIGURE_HIGHT;
    //str_ret=sprintf(focus, "mid freq:%d. (%d,%d)",diag_capture_freq,x,y);
    str_ret = snprintf((char *)focus, 32, "mid freq:%lu. (%lu,%lu)",diag_capture_freq,x,y);
    QE_PRINTF("energy:(%d,%d), osd pos(%d:%d)\n",\
          x,y,virtual_dir_y,virtual_dir_x);
    osd_set_text_field_content(&text_eng_focus,STRING_ANSI, (UINT32)focus);
    osd_draw_object((POBJECT_HEAD)&text_eng_focus,C_UPDATE_ALL);
}

static BOOL on_change_virtual_cx(BOOL bdirdown, UINT32 step)
{
    UINT32 dwnewc=0;
    UINT32 color_r=0xff;
    UINT32 color_g=0;
    UINT32 color_b=0;
    if(bdirdown)
    {
        if(virtual_dir_x+step > COOR_Y_START)
        {
            dwnewc = COOR_Y_END+step;
        }
        else
        {
            dwnewc = virtual_dir_x+step;
        }
    }
    else
    {
        if(virtual_dir_x < COOR_Y_END+step)
        {
            dwnewc = COOR_Y_START-step;
        }
        else
        {
            dwnewc = virtual_dir_x-step;
        }
    }

    OSD_RECT rect;

    MEMSET(&rect,0,sizeof(OSD_RECT));

    rect.u_left = COOR_X_START;
    rect.u_top= virtual_dir_x;
    rect.u_width= FIGURE_WIDTH;
    rect.u_height= 1;
    osd_set_clip_rect(CLIP_INSIDE_RECT,&rect);
    draw_virtual_cx(BG_COLOR);
    draw_energy_figure();
    draw_virtual_cy( RGB24_RGB1555(color_r,color_g,color_b));
    osd_clear_clip_rect();
    virtual_dir_x = dwnewc;
    draw_virtual_cx( RGB24_RGB1555(color_r,color_g,color_b));
    draw_cx_cy_focus();
    return TRUE;
}

static BOOL on_change_virtual_cy(BOOL bdirright, UINT32 step)
{
    UINT32 dwnewc=0;
    UINT32 color_r=0xff;
    UINT32 color_g=0;
    UINT32 color_b=0;
    OSD_RECT rect;
    
    if(bdirright)
    {
        if(virtual_dir_y+step > COOR_X_END)
        {
            dwnewc = COOR_X_START+step;
        }
        else
        {
            dwnewc = virtual_dir_y+step;
        }
    }
    else
    {
        if(virtual_dir_y<COOR_X_START+step)
        {
            dwnewc = COOR_X_END-step;
        }
        else
        {
            dwnewc = virtual_dir_y-step;
        }
    }

    MEMSET(&rect,0,sizeof(OSD_RECT));
    rect.u_left = virtual_dir_y;
    rect.u_top= COOR_TOP;
    rect.u_width= 1;
    rect.u_height= FIGURE_HIGHT;
    osd_set_clip_rect(CLIP_INSIDE_RECT,&rect);
    draw_virtual_cy(BG_COLOR);
    draw_energy_figure();
    draw_virtual_cx(RGB24_RGB1555(color_r,color_g,color_b));
    osd_clear_clip_rect();
    virtual_dir_y = dwnewc;
    draw_virtual_cy(RGB24_RGB1555(color_r,color_g,color_b));
    draw_cx_cy_focus();
    return TRUE;
}

static UINT8 move_virtual_coordinate(UINT8 msgcode)
{
    UINT8 result = PROC_PASS;
    BOOL  __MAYBE_UNUSED__ flag=FALSE;

    switch(msgcode)
    {
        case V_KEY_RIGHT:
        flag=on_change_virtual_cy(TRUE, MOVE_SPEED_LOW);
        break;
        case V_KEY_6:
        flag=on_change_virtual_cy(TRUE, MOVE_SPEED_HIGH);
        break;

        case V_KEY_LEFT:
        flag=on_change_virtual_cy(FALSE, MOVE_SPEED_LOW);
        break;
        case V_KEY_4:
        flag=on_change_virtual_cy(FALSE, MOVE_SPEED_HIGH);
        break;

    case V_KEY_UP:
        flag=on_change_virtual_cx(FALSE, MOVE_SPEED_LOW);
        break;
        case V_KEY_2:
        flag=on_change_virtual_cx(FALSE, MOVE_SPEED_HIGH);
        break;

    case V_KEY_DOWN:
        flag=on_change_virtual_cx(TRUE, MOVE_SPEED_LOW);
        break;
        case V_KEY_8:
        flag=on_change_virtual_cx(TRUE, MOVE_SPEED_HIGH);
        break;
    default:
        break;
    }

    return result;
}

static VACTION win_energy_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT win_energy_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32  vkey=0;
    POBJECT_HEAD topmenu = NULL;
    MEMSET(&topmenu,0,sizeof(POBJECT_HEAD));
    UINT32 color_r=0;
    UINT32 color_g=0xff;
    UINT32 color_b=0;
    UINT8  __MAYBE_UNUSED__ func_ret=0;
    UINT32   __MAYBE_UNUSED__ key_ret=0;

    switch(event)
    {
    case EVN_PRE_OPEN:
        //api_inc_wnd_count();
        wincom_close_title();
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            osd_clear_object( (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
            topmenu = menu_stack_get_top();
            if(topmenu)
            {
                osd_clear_object( topmenu, C_UPDATE_ALL);
            }
        }
        if(NULL == diag_capture_buffer)
        {
            diag_capture_buffer = MALLOC(CAPTURE_BUFFER_LEN);
            if(NULL == diag_capture_buffer)
            {
                ali_trace("failed\n"); 
                return PROC_LEAVE;
            }
        }
        get_energy();
        virtual_dir_y = COOR_X_START;
        virtual_dir_x = COOR_Y_START;
        osd_set_text_field_content(&text_eng_infoy, \
                    STRING_NUMBER, (UINT32)diag_capture_max_value);
        osd_set_text_field_content(&text_eng_help, \
                    STRING_ANSI, (UINT32)"Up Down Left Right Move Slow,2,4,6,8 Move Fast");
        break;
    case EVN_POST_OPEN:
        // Draw coordinate
        osd_draw_hor_line(COOR_X_START-40,\
                COOR_Y_START, COOR_WIDTH+100,  \
                RGB24_RGB1555(color_r,color_g,color_b), NULL);
        osd_draw_ver_line(COOR_X_START,\
                COOR_TOP, COOR_HIGHT+60,   \
                RGB24_RGB1555(color_r,color_g,color_b), NULL);
        draw_energy_figure();
        draw_cx_cy_focus();

        break;
    case EVN_MSG_GOT:
        break;
    case EVN_PRE_CLOSE:
        if(diag_capture_buffer)
        {
            FREE(diag_capture_buffer);
            diag_capture_buffer = NULL;
        }
        break;
    case EVN_POST_CLOSE:
        //api_dec_wnd_count();
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            osd_track_object(            \
                (POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
        }
        break;

    case EVN_UNKNOWNKEY_GOT:
        key_ret=ap_hk_to_vk(0,param1,&vkey);
        func_ret=move_virtual_coordinate(vkey);
        break;
    default:
        break;
    }

    return ret;
}


