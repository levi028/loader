/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ali_demo.c
*
*    Description: The menu of ali demo.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <hld/decv/decv.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "win_com_menu_define.h"
#include "win_com.h"
#include "menus_root.h"

#include "win_ali_demo.h"
#ifdef SHOW_ALI_DEMO_ON_SCREEN

/*******************************************************************************
* Objects declaration
*******************************************************************************/

#define TXTN_SH_IDX   WSTL_BUTTON_01_HD
#define TXTN_HL_IDX   WSTL_BUTTON_05_HD
#define TXTN_SL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_07_HD

#define CON_SH_IDX  WSTL_BUTTON_01_HD
#define CON_HL_IDX  WSTL_BUTTON_05_HD
#define CON_HL1_IDX WSTL_BUTTON_02_HD
#define CON_SL_IDX  WSTL_BUTTON_01_HD
#define CON_GRY_IDX WSTL_BUTTON_07_HD

#define BMP_SH_IDX   WSTL_MIXBACK_BLACK_IDX
#define BMP_HL_IDX   WSTL_MIXBACK_BLACK_IDX
#define BMP_SL_IDX   WSTL_MIXBACK_BLACK_IDX
#define BMP_GRY_IDX  WSTL_MIXBACK_BLACK_IDX

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,NULL,  \
    conobj, ID,1)

#define LDEF_BMP(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id)       \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_TXT1(root,var_txt,nxt_obj,l,t,w,h,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,CON_HL1_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_DEMO(root,var_con,nxt_obj,var_txt,ID,idu,idd,l,t,w,h, hl,str) \
    LDEF_CON(root,var_con,&var_txt,ID,ID,ID,idu,idd,l,t,w+4,h+4,hl,&var_txt,ID)   \
    LDEF_TXT1(root,var_txt,NULL,l+10,t,w-20,h,str)

static UINT16 txt_display_buffer[40];

LDEF_DEMO(NULL,g_win_ali_demo,NULL,ali_demo_txt,1,0,0, \
       720,0,275,45,CON_HL1_IDX,txt_display_buffer) //40


static BOOL is_open = FALSE;

static char* tv_mode_sd_demo[] =
{
#ifdef _SD_ONLY_
    "480i",
    "576i",
#else
    "By Source",
    "480i",
    "576i",
#endif
};

static char *tv_mode_demo[] =
{
#ifdef _SD_ONLY_
    //"By Source",
    "480i",
    "480p",
    "576i",
    "576p",
#else
    "By Source",
    "By Native TV",
    "480i",
    "480P",
    "576i",
    "576P",
#ifndef SD_PVR
    "720P 50",
    "720P 60",
    "1080i 25",
    "1080i 30",
#ifdef HDMI_1080P_SUPPORT
    "1080P 50",
    "1080P 60",
#ifndef HDMI_CERTIFICATION_UI
    "1080P 25",
    "1080P 30",
    "1080P 24",
#endif
#endif
#endif
#endif
};

static char *get_tv_mode_string_demo(UINT32 index)
{
    if (SCART_RGB == sys_data_get_scart_out())
    {
        if (ARRAY_SIZE(tv_mode_sd_demo) > index)
        {
            return tv_mode_sd_demo[index];
        }
        else
        {
            return tv_mode_sd_demo[0];
        }
    }
    else
    {
        if (ARRAY_SIZE(tv_mode_demo) > index)
        {
            return tv_mode_demo[index];
        }
        else
        {
            return tv_mode_demo[0];
        }
    }
}

void show_ali_demo(BOOL flag)
{
    OBJECT_HEAD *obj = NULL;
    char string[50] = { 0 };
    INT32 sprintf_ret = 0;
    enum TV_SYS_TYPE e_tv_mode = TV_MODE_AUTO;
    UINT16 str[16] = {0};
    UINT8 cnt= 0;
    UINT32 index = 0;

    obj = (OBJECT_HEAD*)&g_win_ali_demo;

    if(flag && (is_open == FALSE))
    {
        if(get_current_decoder() == H265_DECODER)
        {
           strncpy(string, "ALi HEVC ",9);
           cnt= 9;
        }
        else
        {
           strncpy(string, "ALi demo ",9);
           cnt =9;
        }

        e_tv_mode = sys_data_get_tv_mode();

        switch(e_tv_mode)
        {
        case TV_MODE_AUTO:
            e_tv_mode = api_video_get_tvout();
    
            e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
            index = tvmode_setting_to_osd(e_tv_mode);
            break;
        case TV_MODE_BY_EDID:
            e_tv_mode = api_video_get_tvout();
    
            e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
            if(TV_MODE_NTSC443 == e_tv_mode)
            {
                e_tv_mode=TV_MODE_NTSC358;
            }
            index = pal_ntsc_tvmode_setting_to_osd(e_tv_mode);
            break;
        default:
            e_tv_mode = sys_data_get_sd_tv_type(e_tv_mode);
            index = tvmode_setting_to_osd(e_tv_mode);
            break;
        }
        strncpy(string+cnt, get_tv_mode_string_demo(index),8);

        //if(NULL == menu_stack_get_top())
        //{
            api_inc_wnd_count();
            osd_set_text_field_content(&ali_demo_txt, STRING_ANSI, (UINT32)string);
            osd_draw_object(obj, C_UPDATE_ALL);
            is_open = TRUE;
        //}
    }
    else if((!flag) && is_open)
    {
        //if(menu_stack_get_top() != NULL)
        //{
           api_dec_wnd_count();
           osd_clear_object(obj,0);
           is_open = FALSE;
        //}
    }
}
#endif

