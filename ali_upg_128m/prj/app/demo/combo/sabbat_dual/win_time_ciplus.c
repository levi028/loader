/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_timer_ciplus.c
*
*    Description: The menu to do timer for ciplus
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
#include "osd_config.h"
#include "osd_rsc.h"
#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_time_ciplus.h"

#ifdef CI_PLUS_PVR_SUPPORT

/*******************************************************************************
*   Objects definition
*******************************************************************************/

static VACTION time_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT time_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

char show_date[24] = "";
char show_time[24] = "";

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,NULL,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,0,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,ID,idu,idd,l,t,w,h,name_id,setstr)  \
    LDEF_CON(&root,var_con,nxt_obj,ID,0,0,0,0,l,t,w,h,&var_txt_name,1)  \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,NULL,0,0,0,0,0,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)

LDEF_MM_ITEM(win_time_con,time_item_con1, &time_item_con2,time_item_txtname1,time_item_txtset1,0,0,0,   \
                CON_L, CON_T + 1.5*CON_GAP + (CON_H + CON_GAP)*0,CON_W,CON_H,   RS_SYSTEM_TIME_DATE,show_date)
LDEF_MM_ITEM(win_time_con,time_item_con2, NULL,time_item_txtname2,  time_item_txtset2,0,0,0,    \
                CON_L, CON_T + 1.5*CON_GAP + (CON_H + CON_GAP)*1,CON_W,CON_H,   RS_SYSTEM_TIME,show_time)

DEF_CONTAINER(win_time_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    time_con_keymap,time_con_callback,  \
    (POBJECT_HEAD)&time_item_con1, 1,0)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
CONTAINER* time_items[] =
{
    &time_item_con1,
    &time_item_con2,
};

POBJECT_HEAD time_item_name[] =
{
    (POBJECT_HEAD)&time_item_txtname1,
    (POBJECT_HEAD)&time_item_txtname2,
};

POBJECT_HEAD time_item_set[] =
{
    (POBJECT_HEAD)&time_item_txtset1,
    (POBJECT_HEAD)&time_item_txtset2,
};

date_time win_time_local_time;


static void set_time_date(void)
{
    char temp[32];
    snprintf(temp,32,"%04d-%02d-%02d", win_time_local_time.year, win_time_local_time.month, win_time_local_time.day);
    com_asc_str2uni(temp, show_date);
}

static void set_time_clock(void)
{
    char temp[32];
    snprintf(temp,32,"%02d:%02d:%02d", win_time_local_time.hour, win_time_local_time.min, win_time_local_time.sec);
    com_asc_str2uni(temp, show_time);
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION time_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        default:
            act = VACT_PASS;
            break;
    }
    return act;
}

static PRESULT  win_time_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    if (msg_type == CTRL_MSG_SUBTYPE_CMD_TIMESECONDUPDATE)
    {
        get_local_time(&win_time_local_time);
        set_time_date();
        set_time_clock();
        osd_draw_object((POBJECT_HEAD)&time_item_con1, C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD)&time_item_con2, C_UPDATE_ALL);
    }
}

static PRESULT time_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    switch(event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title((POBJECT_HEAD)&win_time_con,RS_SYS_LOCAL_TIMER_SET, 0);
            get_local_time(&win_time_local_time);
            set_time_date();
            set_time_clock();
            osd_set_container_focus((CONTAINER*)p_obj,1);
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_MSG_GOT:
            ret = win_time_message_proc(param1,param2);
            break;
        case EVN_PRE_CLOSE:
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;
        case EVN_POST_CLOSE:
            break;
    }
    return ret;
}

#endif
