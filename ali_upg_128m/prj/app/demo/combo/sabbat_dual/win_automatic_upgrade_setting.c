/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_automatic_upgrade_setting.c
*
*    Description: To do the setting of the auto upgrade function
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_popup.h"


/*******************************************************************************
*    Objects definition
*******************************************************************************/
#if(defined POLAND_SPEC_SUPPORT)

char aus_time_pat_time[] = "t1";


#define SCAN_STANDBY_ID  1
#define SCAN_OPERATION_ID  2
#define SCAN_TIME_ID  3

extern CONTAINER g_win_automatic_upgrade_setting;

extern CONTAINER aus_item_con1;
extern CONTAINER aus_item_con2;
extern CONTAINER aus_item_con3;

extern TEXT_FIELD aus_item_txtname1;
extern TEXT_FIELD aus_item_txtname2;
extern TEXT_FIELD aus_item_txtname3;


extern TEXT_FIELD aus_item_txtset1;
extern TEXT_FIELD aus_item_txtset2;
extern EDIT_FIELD aus_item_edit1;


extern TEXT_FIELD aus_item_line1;
extern TEXT_FIELD aus_item_line2;
extern TEXT_FIELD aus_item_line3;

static VACTION aus_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT aus_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION aus_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT aus_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
VACTION win_aus_edit_keymap(POBJECT_HEAD p_obj, UINT32 key);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    aus_item_con_keymap,aus_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, idl, idr, idu, idd, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,WSTL_BUTTON_05_HD,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    win_aus_edit_keymap,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,name_id,setstr)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
    LDEF_TXTSET(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MM_ITEM2(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,name_id,setstr,pat)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
       LDEF_EDIT(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,NORMAL_EDIT_MODE , CURSOR_NORMAL, pat,NULL,NULL, setstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
        DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    aus_con_keymap,aus_con_callback,  nxt_obj, focus_id,0)

#define WIN  g_win_automatic_upgrade_setting


LDEF_MM_ITEM(g_win_automatic_upgrade_setting,aus_item_con1, &aus_item_con2, aus_item_txtname1,aus_item_txtset1,aus_item_line1,1,3,2,    \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,RS_IN_STANDBY_MODE,NULL)

LDEF_MM_ITEM(g_win_automatic_upgrade_setting, aus_item_con2, & aus_item_con3, aus_item_txtname2,  aus_item_txtset2, aus_item_line2,2,1,3,    \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,RS_IN_OPERATION_MODE, NULL)
LDEF_MM_ITEM2(g_win_automatic_upgrade_setting, aus_item_con3, NULL, aus_item_txtname3, aus_item_edit1, aus_item_line3,3,2,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,RS_SYSTEM_TIME,display_strs[1], aus_time_pat_time)

LDEF_WIN(WIN,&aus_item_con1,W_L, W_T, W_W, W_H,1)

/////////////////////////////////////////////////////////////////////////////////

void win_aus_set_channel_upgrade_standby_enable_string(UINT8 input)
{
    if(input==0)
        osd_set_text_field_content(&aus_item_txtset1, STRING_ID,RS_COMMON_OFF);
    else
        osd_set_text_field_content(&aus_item_txtset1, STRING_ID,RS_COMMON_ON);

}

void win_aus_set_channel_upgrade_operation_enable_string(UINT8 input)
{
    if(input==0)
        osd_set_text_field_content(&aus_item_txtset2, STRING_ID,RS_COMMON_OFF);
    else
        osd_set_text_field_content(&aus_item_txtset2, STRING_ID,RS_COMMON_ON);
}

void get_automatic_channel_upgrade_time_input(UINT8* hour, UINT8* min)
{
    date_time dt;
    osd_get_edit_field_time_date(&aus_item_edit1,&dt);
    *hour = dt.hour;
    *min =  dt.min;
}

void set_automatic_channel_upgrade_time(UINT8 hour, UINT8 min)
{
    osd_set_edit_field_content(&aus_item_edit1, STRING_NUMBER, hour*100+min);
}


static UINT8 check_is_valid_hh_mm(UINT8 hour, UINT8 min)
{
    UINT8 hh;
    UINT8 mm;

    hh=hour;
    mm=min;
    if(mm>=60 || hh>23)
        return 0;
    else
        return 1;
}


VACTION win_aus_edit_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            action = VACT_EDIT_LEFT;
            break;
        case V_KEY_RIGHT:
            action = VACT_EDIT_RIGHT;
            break;
        case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
        case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
        case V_KEY_8:    case V_KEY_9:
            action = key - V_KEY_0 + VACT_NUM_0;
            break;
        default:
            break;
    }

    return action;
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION aus_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

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

    return act;

}



void win_aus_left_right_key(POBJECT_HEAD p_obj,UINT8 id)
{
    INT32 hoffset, moffset;
    AUTOSCAN_TIME* as_time = sys_data_get_autoscan_upgrade_info();

    switch(id)
    {

        case SCAN_STANDBY_ID:
            as_time->mode_stby=1-as_time->mode_stby;
            win_aus_set_channel_upgrade_standby_enable_string(as_time->mode_stby);
            osd_track_object(p_obj,C_UPDATE_ALL);
            break;
        case SCAN_OPERATION_ID:
            as_time->mode_oper=1-as_time->mode_oper;
            win_aus_set_channel_upgrade_operation_enable_string(as_time->mode_oper);
            osd_track_object(p_obj,C_UPDATE_ALL);
            break;
    }
}


static PRESULT aus_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    unact,input;
    UINT8    id = osd_get_obj_id(p_obj);
    UINT8 save;
    UINT8 hour, min;

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
         if(unact == VACT_DECREASE)
        {
            win_aus_left_right_key(p_obj,id);
        }
        else if(unact == VACT_INCREASE)
        {
            win_aus_left_right_key(p_obj,id);
        }
        ret = PROC_LOOP;
        break;
    case EVN_KEY_GOT:
        ret = PROC_PASS;
        break;
    }
    return ret;

}

static VACTION aus_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    SYSTEM_DATA* p_sys_data=sys_data_get();
    date_time dt;
    INT32 hoffset, moffset;
    UINT8 save;
    INT32 ret1,ret2;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;

    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;

    case V_KEY_EXIT:
    case V_KEY_MENU:
            act = VACT_CLOSE;
        break;

    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;

    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT aus_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 i;
    UINT8    unact;
    CONTAINER* cont = (CONTAINER*)p_obj;
    UINT8    id = osd_get_obj_id(p_obj);
    CONTAINER* root = &g_win_automatic_upgrade_setting;
    AUTOSCAN_TIME* as_time = sys_data_get_autoscan_upgrade_info();

    UINT8 hour, min;
    POBJECT_HEAD obj =(OBJECT_HEAD*) param2;
    UINT8 save;
    date_time tmp_time;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_automatic_upgrade_setting,RS_AUTOMATIC_CHANNEL_LIST_UPGRADE,0);
        win_aus_set_channel_upgrade_standby_enable_string(as_time->mode_stby);
        win_aus_set_channel_upgrade_operation_enable_string(as_time->mode_oper);
        set_automatic_channel_upgrade_time(as_time->hour, as_time->min);
                osd_set_container_focus((CONTAINER*)p_obj,1);
        break;
    case EVN_POST_OPEN:
        break;

    case EVN_ITEM_PRE_CHANGE:
        if(obj == (POBJECT_HEAD)&aus_item_con3)
        {
                   get_automatic_channel_upgrade_time_input(&hour, &min);
                if(!(check_is_valid_hh_mm(hour, min)))
                {
                        win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_frame(GET_MID_L(500),GET_MID_T(200),500,200);
                    win_compopup_set_msg_ext(NULL,NULL,RS_MSG_INVALID_INPUT_CONTINUE);
                    win_compopup_open_ext(&save);
                    ret = PROC_LOOP;
                }
              }
        break;

    case EVN_PRE_CLOSE:
        if(as_time->mode_stby == 0)
        {
                win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_frame(GET_MID_L(780),GET_MID_T(200),780,250);
                    win_compopup_set_msg_ext(NULL,NULL,RS_AUTOMATIC_CHANNEL_UPGRADE_STANDBYOFF_HINT);
                    win_compopup_open_ext(&save);
        }

        get_automatic_channel_upgrade_time_input(&hour, &min);
        if(as_time->hour != hour || as_time->min != min)
        {
            as_time->hour = hour;
            as_time->min = min;
            as_time->running = 0;
        }

        libc_printf("autoscan setting\n");

        if(as_time->mode_oper == 1)
        {
            get_local_time(&tmp_time);
            uiget_recent_autoscan_time(&tmp_time, as_time);
            as_time->mjd = tmp_time.mjd;
            libc_printf("Operation mode setting :\n");
            autoscan_time_debug_printf(as_time, 0);
        }

        sys_data_save(1);
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;

    case EVN_POST_CLOSE:
        break;
    }
    return ret;
}
#endif
