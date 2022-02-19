 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_playlist_setup.c
*
*    Description: for setup playlist of Mediaplayer.
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
#include <api/libosd/osd_lib.h>

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
#include "win_playlist_setup.h"
#include "ctrl_key_proc.h"
/*******************************************************************************
*    Macro definition
*******************************************************************************/
#ifndef SD_UI
#define SLIDE_W_L    254//(210+180-136)
#define SLIDE_W_T    168//(70+98)
#define SLIDE_W_W    520
#define SLIDE_W_H    224
#define SLIDE_GAP    8
#define SLIDE_TITLE_L    (SLIDE_W_L + 50)
#define SLIDE_TITLE_T    (SLIDE_W_T + SLIDE_GAP * 2)
#define SLIDE_TITLE_W    (SLIDE_W_W- 100)
#define SLIDE_TITLE_H    36
#define BTN1_L (SLIDE_W_L + ((SLIDE_W_W/2) - BTN_W)/2)//(210+310)//208
#define BTN2_L (SLIDE_W_L + (SLIDE_W_W/2) + ((SLIDE_W_W/2) - BTN_W)/2)//(210+460)//312
#define BTN_T  (SLIDE_W_T + 176)//(70+314)//242
#define BTN_W  120
#define BTN_H  38
#define CON_L    (SLIDE_W_L + 30)
#define CON_T    (SLIDE_W_T + 70)
#define CON_W    (SLIDE_W_W - 60)
#define CON_H    40
#define CON_GAP 4
#define TXT_L_OF      4
#define TXT_W          260
#define TXT_H        40
#define TXT_T_OF    ((CON_H - TXT_H)/2)
#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-SEL_L_OF)
#define SEL_H        40
#define SEL_T_OF    ((CON_H - SEL_H)/2)
#define LINE_L_OF    0
#define LINE_T_OF      CON_T
#define LINE_W        (CON_W)
#define LINE_H         4
#else
#define SLIDE_W_L    100//(210+180-136)
#define SLIDE_W_T    100//(70+98)
#define SLIDE_W_W    400
#define SLIDE_W_H    224
#define SLIDE_GAP    8
#define SLIDE_TITLE_L    (SLIDE_W_L + 50)
#define SLIDE_TITLE_T    (SLIDE_W_T + SLIDE_GAP * 1)
#define SLIDE_TITLE_W    (SLIDE_W_W- 100)
#define SLIDE_TITLE_H    36
#define BTN1_L (SLIDE_W_L + ((SLIDE_W_W/2) - BTN_W)/2)//(210+310)//208
#define BTN2_L (SLIDE_W_L + (SLIDE_W_W/2) + ((SLIDE_W_W/2) - BTN_W)/2)//(210+460)//312
#define BTN_T  (SLIDE_W_T + 176)//(70+314)//242
#define BTN_W  120
#define BTN_H  30
#define CON_L    (SLIDE_W_L + 30)
#define CON_T    (SLIDE_W_T + 70)
#define CON_W    (SLIDE_W_W - 60)
#define CON_H    32
#define CON_GAP 4
#define TXT_L_OF      4
#define TXT_W          210
#define TXT_H        32
#define TXT_T_OF    ((CON_H - TXT_H)/2)
#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-SEL_L_OF)
#define SEL_H        32
#define SEL_T_OF    ((CON_H - SEL_H)/2)
#define LINE_L_OF    0
#define LINE_T_OF      CON_T
#define LINE_W        (CON_W)
#define LINE_H         4
#endif
#define SLIDE_SH_IDX    WSTL_MP_POP_WIN_01_HD
#define SLIDE_TITLE_IDX WSTL_TEXT_12_HD//WSTL_TEXT_04_HD
#define SLIDE_ITEM_SH   WSTL_BUTTON_01_FG_HD//sharon WSTL_MP_TEXT_03_HD//WSTL_TEXT_04_HD
#define SLIDE_ITEM_HL   WSTL_BUTTON_02_FG_HD//WSTL_BUTTON_05_HD
#define SLIDE_SET_SH    WSTL_BUTTON_01_FG_HD//WSTL_BUTTON_03_HD//sharon WSTL_MP_TEXT_03_HD//WSTL_TEXT_04_HD
#define SLIDE_SET_HL    WSTL_BUTTON_04_HD
#define BTN_SH_IDX    WSTL_BUTTON_POP_SH_HD
#define BTN_HL_IDX    WSTL_BUTTON_POP_HL_HD
#define CON_SH_IDX   WSTL_BUTTON_03_HD//sharon WSTL_MP_TEXT_03_HD//WSTL_TEXT_04_HD
#define CON_HL_IDX   WSTL_BUTTON_02_HD//sharon WSTL_BUTTON_03_HD//WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_08_HD//WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_03_HD//WSTL_TEXT_20_HD
#define TIME_ID 1
#define REPEAT_ID 2
#define BTN_SAVE_ID 3
#define BTN_EXIT_ID 4
#define VACT_SET_INCREASE (VACT_PASS + 1)
#define VACT_SET_DECREASE (VACT_PASS + 2)
#define VACT_SAVE_EXIT (VACT_PASS + 3)
#define VACT_ABANDON_EXIT (VACT_PASS + 4)

/*******************************************************************************
*    Function decalare
*******************************************************************************/
//static void win_open_setup_help(void);
static PRESULT win_setting_proc(VACTION unact);
static void win_setting_display(BOOL b_draw);
static VACTION setup_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT setup_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION slideset_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT slideset_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION slideset_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT slideset_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SLIDE_SH_IDX,SLIDE_SH_IDX,SLIDE_SH_IDX,SLIDE_SH_IDX,   \
    setup_con_keymap,setup_con_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    slideset_item_keymap,slideset_item_callback,  \
    conobj, ID,1)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, \
    SLIDE_TITLE_IDX,SLIDE_TITLE_IDX,SLIDE_TITLE_IDX,SLIDE_TITLE_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 4,0,res_id,NULL)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SLIDE_ITEM_SH,SLIDE_ITEM_HL,SLIDE_ITEM_SH,SLIDE_ITEM_SH,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT2(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SLIDE_SET_SH,SLIDE_SET_HL,SLIDE_SET_SH,SLIDE_SET_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,l,t,w,h,ID,idl,idr,idu,idd,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SH_IDX,BTN_SH_IDX,   \
    slideset_btn_keymap,slideset_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_POP_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_POP_LINE_HD,WSTL_LINE_HD,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)


#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_sel,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_sel,l + TXT_L_OF,t ,TXT_W,TXT_H,res_id)    \
    LDEF_TXT2(&var_con,var_sel,NULL,l + SEL_L_OF,t ,SEL_W,SEL_H,str)    \
    LDEF_POP_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + CON_H,LINE_W,LINE_H,0,0)

/*******************************************************************************
*    Objects evaluate
*******************************************************************************/

LDEF_WIN(g_win_slidesetup,&slide_title,SLIDE_W_L,SLIDE_W_T,SLIDE_W_W,SLIDE_W_H,1)

LDEF_TITLE(g_win_slidesetup,slide_title,&slide_con_item1,SLIDE_TITLE_L,SLIDE_TITLE_T,SLIDE_TITLE_W, \
    SLIDE_TITLE_H,RS_GAME_SETUP)

LDEF_MENU_ITEM_SEL(g_win_slidesetup,slide_con_item1,&slide_con_item2,slide_time,slide_time_sel,slide_line1, 1, 3, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_MP_SLIDE_SHOW_TIME, display_strs[1])

LDEF_MENU_ITEM_SEL(g_win_slidesetup,slide_con_item2,&slide_save,slide_repeat,slide_repeat_sel,slide_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_MP_SLIDE_SHOW_REPEAT, NULL)

LDEF_TXT_BTN(g_win_slidesetup,slide_save,&slide_exit,BTN1_L, BTN_T, BTN_W,BTN_H,3,4,4,2,1,RS_COMMON_SAVE)
LDEF_TXT_BTN(g_win_slidesetup,slide_exit,NULL,       BTN2_L, BTN_T, BTN_W,BTN_H,4,3,3,2,1,RS_COMMON_CANCEL)


/*******************************************************************************
*    Variable definition
*******************************************************************************/
IMAGE_SLIDE_INFO image_slide_setting ;
static IMAGE_SLIDE_INFO image_slide_temp;
static UINT16 slide_repeat_strids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
};
static TEXT_FIELD *txt_settings[] =
{
    &slide_time_sel,
    &slide_repeat_sel,
};
/*******************************************************************************
*    Function Definition
*******************************************************************************/
static VACTION setup_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_ABANDON_EXIT;
        break;
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
      case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    default:
        act = VACT_PASS;
        break;

    }

    return act;
}
static PRESULT setup_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    VACTION unact = 0;
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        MEMCPY(&image_slide_temp,&image_slide_setting,sizeof(IMAGE_SLIDE_INFO));
        win_setting_display(FALSE);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_setting_proc(unact);
        break;
    default:
        break;
    }

    return ret;
}

static VACTION slideset_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_SET_DECREASE;
        break;
    case V_KEY_RIGHT:
        act = VACT_SET_INCREASE;
        break;
    default:
        act = VACT_PASS;
        break;

    }

    return act;

}

static PRESULT slideset_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    VACTION unact = 0;
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_setting_proc(unact);
        break;
    default:
        break;
    }

    return ret;
}

// save ,exit btn
static VACTION slideset_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;
    CONTAINER *win = &g_win_slidesetup;
    UINT8 b_id = 0;

    b_id = osd_get_container_focus(win);
    switch(key)
    {
    case V_KEY_ENTER:
        if(BTN_SAVE_ID == b_id)
        {
         act  = VACT_SAVE_EXIT;
        }
        else
        {
            act = VACT_ABANDON_EXIT;
        }
        break;
    default:
        act = VACT_PASS;
        break;

    }

    return act;

}

static PRESULT slideset_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    VACTION unact = 0;
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_setting_proc(unact);
        break;
    default:
        break;
    }
    return ret;
}


void win_playlist_setup_open(void)
{
    UINT32 hkey = 0;
    CONTAINER *pop_win = NULL;
    PRESULT ret = PROC_LOOP;
        BOOL old_value = FALSE;

    old_value = ap_enable_key_task_get_key(TRUE);

    pop_win = &g_win_slidesetup;
    osd_set_container_focus(pop_win, 1);
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


static PRESULT win_setting_proc(VACTION unact)
{
    UINT8 b_id = 0;
    CONTAINER *win = NULL;
    PRESULT ret = PROC_PASS;
    UINT8 tmp = 0;
    SYSTEM_DATA *sys_data = sys_data_get();

    switch(unact)
    {
        case VACT_SET_DECREASE:
        case VACT_SET_INCREASE:
            ret = PROC_LOOP;
            win = &g_win_slidesetup;
            b_id = osd_get_container_focus(win);
            switch(b_id)
            {
                case TIME_ID:
                    if(VACT_SET_INCREASE == unact )
                    {
                        image_slide_temp.u_time_gap = (image_slide_temp.u_time_gap + 1) % SLIDE_MAX_SEC;
                    }
                    else
                    {
                        tmp = image_slide_temp.u_time_gap;
                        image_slide_temp.u_time_gap = (tmp + SLIDE_MAX_SEC - 1) % SLIDE_MAX_SEC;
                    }
                    break;
                case REPEAT_ID:
                    image_slide_temp.b_repeat = 1 - image_slide_temp.b_repeat;
                    break;
                default:
                    break;
            }
            win_setting_display(TRUE);
            break;
        case VACT_SAVE_EXIT:
            MEMCPY(&image_slide_setting,&image_slide_temp,sizeof(IMAGE_SLIDE_INFO));
            if(NULL != sys_data)
            {
                MEMCPY(&sys_data->image_slide_setting,&image_slide_setting,sizeof(IMAGE_SLIDE_INFO));
                sys_data_save(0);
            }
            ret = PROC_LEAVE;
            break;
        case VACT_ABANDON_EXIT:
            ret = PROC_LEAVE;
            break;
        default:
            break;
    }
    return ret;
}

static void win_setting_display(BOOL b_draw)
{
    UINT8 i = 0;
    UINT8 b_id = 0;
    char str_buff[10] = {0};
    TEXT_FIELD *p_txt = NULL;
    CONTAINER *win = NULL;

    if(0 == image_slide_temp.u_time_gap)
    {
        strncpy(str_buff, "Off", 9);//sprintf(str_buff, "Off");
    }
    else
    {
        snprintf(str_buff,10,"%d Sec",image_slide_temp.u_time_gap);
    }

    win = &g_win_slidesetup;
    b_id = osd_get_container_focus(win);
    for(i = 0; i < 2; i++)
    {
        p_txt = txt_settings[i];
        if(0 == i)
        {
            osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)str_buff);
        }
        else
        {
            osd_set_text_field_content(p_txt,STRING_ID,slide_repeat_strids[image_slide_temp.b_repeat]);
        }
        if(((i + 1)  == b_id) && (b_draw))
        {
            osd_track_object((POBJECT_HEAD)p_txt, C_UPDATE_ALL);
        }
    }
}


#endif

