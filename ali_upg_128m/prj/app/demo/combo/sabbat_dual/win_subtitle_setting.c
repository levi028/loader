/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_subtitle_setting.c
*
*    Description: The menu for subtitle display setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>

#include "copper_common/system_data.h"
#include "copper_common/com_api.h"

#include "osdobjs_def.h"
#include "win_com_menu_define.h"
#include "win_com_popup.h"
#include "menus_root.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"
#include <api/libmp/mp_subtitle_api.h>

#include "win_com.h"
#include "win_subtitle.h"
#include "win_subtitle_setting.h"
#include "ctrl_key_proc.h"
#include "control.h"

#ifdef MP_SUBTITLE_SETTING_SUPPORT

/*******************************************************************************
*    Objects definition
*******************************************************************************/
#define SUBTITLE_TIMER_BASE_TIME 10


static VACTION subt_set_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT subt_set_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION subt_set_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT subt_set_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
#undef WIN_SH_IDX
#undef WIN_HL_IDX
#undef WIN_SL_IDX
#undef WIN_GRY_IDX
#define WIN_SH_IDX      WSTL_WIN_BODYLEFT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYLEFT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYLEFT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYLEFT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define CON_HLSUB_IDX    WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define TITLE_SH_IDX    WSTL_WIN_TITLE_01_HD

#ifndef SD_UI

#define TITLE_L    20//229//232
#define TITLE_T    30//181//185
#define TITLE_W    550
#define TITLE_H    60

#undef W_L
#undef W_T
#undef W_W
#undef W_H
#undef CON_L
#undef CON_T
#undef CON_W
#undef CON_H
#undef CON_GAP

#define    W_L        TITLE_L //180
#define    W_T    TITLE_T//TITLE_T+TITLE_H
#define    W_W    TITLE_W
#define    W_H    260//200//218//134//90
#define CON_L    (W_L + 10)//(W_L + 4)
#define CON_T    (W_T + 2 + TITLE_H)//(W_T + 2)
#define CON_W    (W_W - 20)//(W_W - 8)
#define CON_H    40//42


#define CON_GAP    0

#define TXT_L_OF      2
#define TXT_W          ((CON_W*5)/11)
#define TXT_H        CON_H//42//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          ((CON_W*6)/11)
#define SEL_H        CON_H//42
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#else

#undef W_L
#undef W_T
#undef W_W
#undef W_H
#undef CON_L
#undef CON_T
#undef CON_W
#undef CON_H
#undef CON_GAP

#define TITLE_L    104//229//232
#define TITLE_T    100//181//185
#define TITLE_W    400
#define TITLE_H    40

#define    W_L        TITLE_L //180
#define    W_T    TITLE_T//TITLE_T+TITLE_H
#define    W_W    TITLE_W
#define    W_H    200//200//218//134//90
#define CON_L    (W_L + 10)//(W_L + 4)
#define CON_T    (W_T + 2 + TITLE_H)//(W_T + 2)
#define CON_W    (W_W - 20)//(W_W - 8)
#define CON_H    32//42


#define CON_GAP    0

#define TXT_L_OF      2
#define TXT_W          ((CON_W*5)/11)
#define TXT_H        CON_H//42//28
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          ((CON_W*6)/11)
#define SEL_H        CON_H//42
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#endif


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,\
    l,t,w,h,hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, \
    CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    subt_set_item_con_keymap,subt_set_item_con_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXTSET(root,var_txt,nxt_obj,\
    ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, \
    SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_txtset,ID,idu,idd,\
                        l,t,w,h,res_idname,res_idset,setstr) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,\
    l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_txtset,\
    l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_idname)    \
    LDEF_TXTSET(&var_con,var_txtset,  NULL        ,1,1,1,1,1,\
    l + SEL_L_OF ,t + SEL_T_OF,SEL_W-1,SEL_H,res_idset,setstr)


#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,\
    ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l,t + TXT_T_OF,W_W-8,TXT_H,res_id)


DEF_TEXTFIELD(subt_set_title,&win_subt_set_con,\
    &subt_set_item_con1,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,TITLE_L,TITLE_T ,TITLE_W,TITLE_H, \
    TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,RS_SUBT_SETTING,NULL)

LDEF_MENU_ITEM_SEL(win_subt_set_con,subt_set_item_con1,&subt_set_item_con2,\
        subt_set_item_txtname1,subt_set_item_txtset1, 1, 4, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H,\
    RS_SUBT_SETTING_FONT_SIZE, 0, display_strs[0])

LDEF_MENU_ITEM_SEL(win_subt_set_con,subt_set_item_con2,&subt_set_item_con3,\
        subt_set_item_txtname2,subt_set_item_txtset2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,\
    RS_SUBT_SETTING_FG_COLOR, 0, display_strs[1])

LDEF_MENU_ITEM_SEL(win_subt_set_con,subt_set_item_con3,&subt_set_item_con4,\
        subt_set_item_txtname3,subt_set_item_txtset3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, \
    RS_SUBT_SETTING_BG_COLOR, 0, display_strs[2])

LDEF_MENU_ITEM_SEL(win_subt_set_con,subt_set_item_con4,NULL,\
        subt_set_item_txtname4,subt_set_item_txtset4, 4, 3, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, \
    RS_SUBT_SETTING_LOCATION, 0, display_strs[3])


DEF_CONTAINER(win_subt_set_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, W_L,W_T,W_W,W_H, \
    WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    subt_set_con_keymap,subt_set_con_callback,  \
    (POBJECT_HEAD)&subt_set_title, 1,0)

/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
#if 0
CONTAINER* subt_set_items[] =
{
    &subt_set_item_con1,
    &subt_set_item_con2,
    &subt_set_item_con3,
    &subt_set_item_con4,
};

POBJECT_HEAD subt_set_item_name[] =
{
    (POBJECT_HEAD)&subt_set_item_txtname1,
    (POBJECT_HEAD)&subt_set_item_txtname2,
    (POBJECT_HEAD)&subt_set_item_txtname3,
    (POBJECT_HEAD)&subt_set_item_txtname4,
};

POBJECT_HEAD subt_set_item_set[] =
{
    (POBJECT_HEAD)&subt_set_item_txtset1,
    (POBJECT_HEAD)&subt_set_item_txtset2,
    (POBJECT_HEAD)&subt_set_item_txtset3,
    (POBJECT_HEAD)&subt_set_item_txtset4,
};
#endif
enum
{
MENU_SUBT_SET_FONT_SIZE=1,
MENU_SUBT_SET_FG_COLOR,
MENU_SUBT_SET_BG_COLOR,
MENU_SUBT_SET_POSITION,
};

#define MAX_SUBT_COLOR_NUM    7
#define MAX_SUBT_FONT_SIZE    3
#define MAX_SUBT_POSITION_NUM        101 //0~100

static ID   subt_set_timer = OSAL_INVALID_ID;
static UINT32 SUBT_SET_TIMER_TIME = 5*1000; //5s for subtitle setting menu
#define SUBT_SET_TIMER_NAME     "vol"

static UINT8 subt_set_menu_layer = 0;//record the menu layer, val:0/1

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static void subt_set_timer_func(UINT unused)
{
    //BOOL msg_ret=FALSE;
    UINT32  __MAYBE_UNUSED__ key_tans=0;
    if(OSAL_INVALID_ID==subt_set_timer)
    {
        return;
    }

    UINT32 hkey=0;
    UINT32 i=0;

    api_stop_timer(&subt_set_timer);
    key_tans=ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
    for(i=0;i<=subt_set_menu_layer;i++)
    {
        //msg_ret=
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
    // send key instead of sending message
        //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
    }
}

static void subt_set_timer_refresh(void)
{
    if(OSAL_INVALID_ID==subt_set_timer)
    {
        return;
    }

    api_stop_timer(&subt_set_timer);
    subt_set_timer = api_start_timer(SUBT_SET_TIMER_NAME,\
                     SUBT_SET_TIMER_TIME,subt_set_timer_func);

}

static void win_subt_set_fontsize_string(INT8 input)
{
    INT8 temp[12]={0};
    UINT32  __MAYBE_UNUSED__ tansform_ret=0;
    char  *  __MAYBE_UNUSED__ sprintf_ret = NULL;
    UINT8 cnt = ARRAY_SIZE(temp)-1;

    if(SUBT_FONT_SIZE_NORMAL == input)
    {
        sprintf_ret=strncpy(temp, "Normal", cnt);
    }
    else if(SUBT_FONT_SIZE_SMALL == input)
    {
        sprintf_ret=strncpy(temp, "Small", cnt);
    }
    else
    {
        sprintf_ret=strncpy(temp, "Big", cnt);
    }

    tansform_ret=com_asc_str2uni((UINT8 *)temp, display_strs[0]);
}

static void win_subt_set_fg_color_string(INT8 input)
{
    INT8 temp[12]={0};
    UINT32  __MAYBE_UNUSED__ tansform_ret=0;
    char  *  __MAYBE_UNUSED__ sprintf_ret = NULL;
    UINT8 cnt = ARRAY_SIZE(temp)-1;

    if(SUBT_COLOR_BLACK == input)
    {
        sprintf_ret = strncpy(temp, "Black", cnt);
    }
    else if(SUBT_COLOR_RED == input)
    {
        sprintf_ret = strncpy(temp, "Red", cnt);
    }
    else if(SUBT_COLOR_GREEN== input)
    {
        sprintf_ret = strncpy(temp, "Green", cnt);
    }
    else if(SUBT_COLOR_YELLOW== input)
    {
        sprintf_ret = strncpy(temp, "Yellow", cnt);
    }
    
    else if(SUBT_COLOR_WHITE== input)
    {
        sprintf_ret = strncpy(temp, "White", cnt);
    }
    else if(SUBT_COLOR_TRANSPARENT== input)
    {
        sprintf_ret = strncpy(temp, "Transparent", cnt);
    }
	else 
    {
        sprintf_ret = strncpy(temp, "Blue", cnt);
    }

    tansform_ret=com_asc_str2uni((UINT8 *)temp, display_strs[1]);
}

static void win_subt_set_bg_color_string(INT8 input)
{
    INT8 temp[12]={0};
    UINT32  __MAYBE_UNUSED__ tansform_ret=0;
    char  *  __MAYBE_UNUSED__ strcpy_ret = NULL;
    UINT8 cnt = ARRAY_SIZE(temp) - 1;

    if(SUBT_COLOR_BLACK == input)
    {
        strcpy_ret = strncpy(temp, "Black", cnt);//sprintf(temp, "Black", input);
    }
    else if(SUBT_COLOR_RED == input)
    {
        strcpy_ret = strncpy(temp, "Red", cnt);//sprintf(temp, "Red", input);
    }
    else if(SUBT_COLOR_GREEN== input)
    {
        strcpy_ret = strncpy(temp, "Green", cnt);
    }
    else if(SUBT_COLOR_YELLOW== input)
    {
        strcpy_ret = strncpy(temp, "Yellow", cnt);
    }
    else if(SUBT_COLOR_BLUE== input)
    {
        strcpy_ret = strncpy(temp, "Blue", cnt);
    }
    else if(SUBT_COLOR_WHITE== input)
    {
        strcpy_ret = strncpy(temp, "White", cnt);
    }
    else if(SUBT_COLOR_TRANSPARENT== input)
    {
        strcpy_ret = strncpy(temp, "Transparent", cnt);
    }
	else 
    {
        strcpy_ret = strncpy(temp, "Blue", cnt);
    }

    tansform_ret=com_asc_str2uni((UINT8 *)temp, display_strs[2]);
}

static void win_subt_set_position_string(INT8 input)
{
    INT8 temp[12]={0};
    UINT32  __MAYBE_UNUSED__ tansform_ret=0;
    int sprintf_ret = 0;
    INT16 select_disp = 50-input;//0~100  => +50~0~-50
    UINT8 cnt = ARRAY_SIZE(temp)-1;

    if(NUM_ZERO== select_disp)
    {
        strncpy(temp,"Buttom",cnt);
    }
    else
    {
        sprintf_ret=snprintf(temp, ARRAY_SIZE(temp),"%d", select_disp);
        if(0 == sprintf_ret)
        {
            ali_trace(&sprintf_ret);
    }
    }
    tansform_ret=com_asc_str2uni((UINT8 *)temp, display_strs[3]);
}


static VACTION subt_set_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    subt_set_timer_refresh();

    return act;

}

static void win_subt_set_enter_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    MEMSET(&rect,0,sizeof(rect));
    MEMSET(&param,0,sizeof(param));
    rect.u_left=CON_L+TXTS_L_OF;
    rect.u_width=TXTS_W;
    rect.u_height=250;
    param.selecttype = POP_LIST_SINGLESELECT;
    //PRESULT result=PROC_PASS;

    switch(id)
    {
        case MENU_SUBT_SET_FONT_SIZE:
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*1;
            #else
            rect.u_height = 180;
            rect.u_top=CON_T + (CON_H + CON_GAP)*1;
            #endif

            param.cur=p_sys_data->ext_subt_param.font_size;
            subt_set_menu_layer+=1;//enter second layer
            p_sys_data->ext_subt_param.font_size = \
        win_com_open_sub_list(POP_LIST_SUBT_SET,&rect,&param);
            subt_set_menu_layer-=1;//leve second layer
            win_subt_set_fontsize_string(p_sys_data->ext_subt_param.font_size);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_FG_COLOR:
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*2;
            #else
            rect.u_height = 180;
            rect.u_top=CON_T + (CON_H + CON_GAP)*2;
            #endif

            param.cur=p_sys_data->ext_subt_param.fg_color;
            subt_set_menu_layer+=1;//enter second layer
            p_sys_data->ext_subt_param.fg_color =\
        win_com_open_sub_list(POP_LIST_SUBT_SET,&rect,&param);
            subt_set_menu_layer-=1;//leve second layer
            win_subt_set_fg_color_string(p_sys_data->ext_subt_param.fg_color);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_BG_COLOR:
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*3;
            #else
            rect.u_height = 180;
            rect.u_top=CON_T + (CON_H + CON_GAP)*3;
            #endif

            param.cur=p_sys_data->ext_subt_param.bg_color;
            subt_set_menu_layer+=1;//enter second layer
            p_sys_data->ext_subt_param.bg_color = \
        win_com_open_sub_list(POP_LIST_SUBT_SET,&rect,&param);
            subt_set_menu_layer-=1;//leve second layer
            win_subt_set_bg_color_string(p_sys_data->ext_subt_param.bg_color);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_POSITION:
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*4;
            #else
            rect.u_height = 180;
            rect.u_top=CON_T + (CON_H + CON_GAP)*4;
            #endif

            param.cur=p_sys_data->ext_subt_param.y_offset;
            subt_set_menu_layer+=1;//enter second layer
            p_sys_data->ext_subt_param.y_offset = \
                   win_com_open_sub_list(POP_LIST_SUBT_SET,&rect,&param);
            subt_set_menu_layer-=1;//leve second layer
            win_subt_set_position_string(p_sys_data->ext_subt_param.y_offset);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }

}

static void win_subt_set_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_SUBT_SET_FONT_SIZE:
            if(SUBT_FONT_SIZE_NORMAL==p_sys_data->ext_subt_param.font_size)
            {
                p_sys_data->ext_subt_param.font_size=SUBT_FONT_SIZE_BIG;
            }
            else
            {
                p_sys_data->ext_subt_param.font_size--;
            }
            win_subt_set_fontsize_string(p_sys_data->ext_subt_param.font_size);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_FG_COLOR:
            if(SUBT_COLOR_BLACK==p_sys_data->ext_subt_param.fg_color)
            {
                p_sys_data->ext_subt_param.fg_color=SUBT_COLOR_TRANSPARENT;
            }
            else
            {
                p_sys_data->ext_subt_param.fg_color--;
            }
            win_subt_set_fg_color_string(p_sys_data->ext_subt_param.fg_color);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_BG_COLOR:
            if(SUBT_COLOR_BLACK==p_sys_data->ext_subt_param.bg_color)
            {
                p_sys_data->ext_subt_param.bg_color=SUBT_COLOR_TRANSPARENT;
            }
            else
            {
                p_sys_data->ext_subt_param.bg_color--;
            }
            win_subt_set_bg_color_string(p_sys_data->ext_subt_param.bg_color);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_POSITION:
            if(NUM_ZERO==p_sys_data->ext_subt_param.y_offset)
            {
                p_sys_data->ext_subt_param.y_offset=MAX_SUBT_POSITION_NUM-1;
            }
            else
            {
                p_sys_data->ext_subt_param.y_offset--;
            }
            win_subt_set_position_string(p_sys_data->ext_subt_param.y_offset);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_subt_set_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_SUBT_SET_FONT_SIZE:
            if(SUBT_FONT_SIZE_BIG==p_sys_data->ext_subt_param.font_size)
            {
                p_sys_data->ext_subt_param.font_size=SUBT_FONT_SIZE_NORMAL;
            }
            else
            {
                p_sys_data->ext_subt_param.font_size++;
            }
            win_subt_set_fontsize_string(p_sys_data->ext_subt_param.font_size);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_FG_COLOR:
            if(SUBT_COLOR_BLUE==p_sys_data->ext_subt_param.fg_color)
            {
                p_sys_data->ext_subt_param.fg_color=SUBT_COLOR_BLACK;
            }
            else
            {
                p_sys_data->ext_subt_param.fg_color++;
            }
            win_subt_set_fg_color_string(p_sys_data->ext_subt_param.fg_color);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_BG_COLOR:
            if(SUBT_COLOR_BLUE==p_sys_data->ext_subt_param.bg_color)
            {
                p_sys_data->ext_subt_param.bg_color=SUBT_COLOR_BLACK;
            }
            else
            {
                p_sys_data->ext_subt_param.bg_color++;
            }
            win_subt_set_bg_color_string(p_sys_data->ext_subt_param.bg_color);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_SUBT_SET_POSITION:
            if((MAX_SUBT_POSITION_NUM-NUM_ONE)==\
            p_sys_data->ext_subt_param.y_offset)
            {
                p_sys_data->ext_subt_param.y_offset=0;
            }
            else
            {
                p_sys_data->ext_subt_param.y_offset++;
            }
            win_subt_set_position_string(p_sys_data->ext_subt_param.y_offset);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static PRESULT subt_set_item_con_callback\
               (POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;
    //UINT8 input=0;
    UINT8   id = osd_get_obj_id(pobj);

    //SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if(VACT_ENTER == unact)
        {
            win_subt_set_enter_key(pobj,id);
        }
        else if(VACT_DECREASE == unact)
        {
            win_subt_set_left_key(pobj,id);
        }
        else if(VACT_INCREASE == unact)
        {
            win_subt_set_right_key(pobj,id);
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION subt_set_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
        case V_KEY_BLUE:
            act = VACT_CLOSE;
            break;
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            act = osd_container_key_map(pobj,key);
            break;
    }

    subt_set_timer_refresh();

    return act;
}

static PRESULT subt_set_con_callback\
               (POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    __MAYBE_UNUSED__ unact=0;
    //BOOL send_msg=FALSE;

    SYSTEM_DATA *p_sys_data=NULL;

    switch(event)
    {
    case EVN_PRE_OPEN:
        p_sys_data=sys_data_get();
        win_subt_set_fontsize_string(p_sys_data->ext_subt_param.font_size);
        win_subt_set_fg_color_string(p_sys_data->ext_subt_param.fg_color);
        win_subt_set_bg_color_string(p_sys_data->ext_subt_param.bg_color);
        win_subt_set_position_string(p_sys_data->ext_subt_param.y_offset);
        SUBT_SET_TIMER_TIME = p_sys_data->osd_set.time_out;
        if((NUM_ZERO==SUBT_SET_TIMER_TIME) ||( SUBTITLE_TIMER_BASE_TIME<SUBT_SET_TIMER_TIME))
    {
        SUBT_SET_TIMER_TIME = 5;
    }
        SUBT_SET_TIMER_TIME *= 1000;
        break;

    case EVN_POST_OPEN:
        subt_set_timer = api_start_timer(SUBT_SET_TIMER_NAME, \
                     SUBT_SET_TIMER_TIME,subt_set_timer_func);
        break;

    case EVN_MSG_GOT:
#ifdef USB_MP_SUPPORT
        if(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER == param1)
        {
            //send_msg=
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VIDEO_OVER,0,TRUE);
            subt_set_timer_func(0);
        }
#endif
        break;

    case EVN_PRE_CLOSE:
        unact = (VACTION)(param1>>16);
        ret = PROC_PASS;
        break;

    case EVN_POST_CLOSE:
        api_stop_timer(&subt_set_timer);
            sys_data_save(1);
        break;
    default:
        break;
    }
    return ret;
}

UINT8 win_subt_set_get_num(void)
{
    UINT8 id=osd_get_focus_id((POBJECT_HEAD)&win_subt_set_con);

    UINT8 num=0;

    switch(id)
    {
        case MENU_SUBT_SET_FONT_SIZE:
            num=MAX_SUBT_FONT_SIZE;
            break;
        case MENU_SUBT_SET_FG_COLOR:
            num=MAX_SUBT_COLOR_NUM;
            break;
        case MENU_SUBT_SET_BG_COLOR:
            num=MAX_SUBT_COLOR_NUM;
            break;
        case MENU_SUBT_SET_POSITION:
            num=MAX_SUBT_POSITION_NUM;
            break;
        default:
            break;
    }
    return num;
}

VACTION comlist_menu_subt_set_listkeymap(POBJECT_HEAD pobj, UINT32 vkey)
{
    VACTION act = VACT_PASS;

    switch(vkey)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
        break;
    case V_KEY_ENTER:
        act = VACT_SELECT;
        break;
    default:
        break;
    }

    subt_set_timer_refresh();

    return act;
}

VACTION comlist_menu_subt_set_winkeymap(POBJECT_HEAD pobj, UINT32 vkey)
{
    PRESULT ret = VACT_PASS;

    switch(vkey)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
            ret = VACT_CLOSE;
        break;
    default:
        break;
    }

    //libc_printf("winkey:%d\n",vkey);
    subt_set_timer_refresh();

    return ret;
}

PRESULT comlist_menu_subt_set_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    INT16 select_disp = 0;
    INT16 i=0;
    INT16 wtop=0;
    UINT8 id=0;

    OBJLIST *ol=NULL;
    INT8 temp[10]={0};
    char  *  __MAYBE_UNUSED__ disp_result = NULL;
    int spriret = 0;
    PRESULT cbret = PROC_PASS;
    UINT8 cnt = ARRAY_SIZE(temp)-1;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        id=osd_get_focus_id((POBJECT_HEAD)&win_subt_set_con);
        switch(id)
        {
            case MENU_SUBT_SET_FONT_SIZE:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        if(SUBT_FONT_SIZE_NORMAL ==  i+wtop)
                        {
                disp_result=strncpy(temp, "Normal",  cnt);
            }
                    else if(SUBT_FONT_SIZE_SMALL ==  i+wtop)
            {
                disp_result=strncpy(temp, "Small",  cnt);
            }
                    else
            {
                disp_result=strncpy(temp, "Big", cnt);
            }

                    win_comlist_set_str(i + wtop,temp,NULL,0);
                    }
                break;
            case MENU_SUBT_SET_FG_COLOR:
            case MENU_SUBT_SET_BG_COLOR:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        if(SUBT_COLOR_BLACK ==  i+wtop)
            {
                disp_result=strncpy(temp, "Black",  cnt);
            }
                    else if(SUBT_COLOR_RED ==  i+wtop)
            {
                disp_result=strncpy(temp, "Red",  cnt);
            }
                    else if(SUBT_COLOR_GREEN==  i+wtop)
            {
                disp_result=strncpy(temp, "Green",  cnt);
            }
                    else if(SUBT_COLOR_YELLOW==  i+wtop)
            {
                disp_result=strncpy(temp, "Yellow",  cnt);
            }
                    else if(SUBT_COLOR_WHITE==  i+wtop)
            {
                disp_result=strncpy(temp, "White",  cnt);
            }
                    else if(SUBT_COLOR_TRANSPARENT==  i+wtop)
            {
                disp_result=strncpy(temp, "Transparent",  cnt);
            }
					 else 
            {
                disp_result=strncpy(temp, "Blue",  cnt);
            }

                    win_comlist_set_str(i + wtop,temp,NULL,0);
                    }
                break;
            case MENU_SUBT_SET_POSITION:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        select_disp = 50-(i+wtop);//0~100  => +50~0~-50

                        if(NUM_ZERO== select_disp)
            {
                disp_result=strncpy(temp,"Buttom",cnt);
            }
                    else
            {
                spriret=snprintf(temp,ARRAY_SIZE(temp), "%d", select_disp);
                            if(0 == spriret)
                            {
                                ali_trace(&spriret);
            }
            		    }
                    win_comlist_set_str(i + wtop,temp,NULL,0);
                    }
                break;
            default:
                break;
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }

    return cbret;
}
#endif

