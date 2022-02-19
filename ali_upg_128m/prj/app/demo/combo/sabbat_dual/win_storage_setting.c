/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_storage_setting.c
*
*    Description: the ui of storage setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libmp/lib_mp.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "menus_root.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "disk_manager.h"
#include "win_storage_setting.h"

/*******************************************************************************
*    Objects definition
*******************************************************************************/


//extern CONTAINER sto_set_item_con2;
//extern TEXT_FIELD sto_set_item_txtname2;
//extern TEXT_FIELD sto_set_item_txtset2;
//extern TEXT_FIELD sto_set_item_line2;

//extern CONTAINER sto_set_item_con3;
//extern TEXT_FIELD sto_set_item_txtname3;
//extern TEXT_FIELD sto_set_item_txtset3;
//extern TEXT_FIELD sto_set_item_line3;

#define LINE_L_OF   0
#define LINE_W      CON_W
#define LINE_H      4

#define VACT_RECORD_ALL     (VACT_PASS+1)

static VACTION sto_set_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT sto_set_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION sto_set_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT sto_set_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    sto_set_item_con_keymap,sto_set_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 28,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line, ID,idu,idd,l,t,w,h,name_id,setstr)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,(TXTN_W-20), \
    TXTN_H,name_id,NULL) \
    LDEF_TXTSET(&var_con,var_txtset, NULL/*&varLine*/,1,1,1,1,1,l + (TXTS_L_OF-20), t + TXTS_T_OF,TXTS_W,\
    TXTS_H,0,setstr) \
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, l + LINE_L_OF, t + CON_H, LINE_W, LINE_H, 0, 0)

LDEF_MM_ITEM(g_win_sto_setting,sto_set_item_con1, NULL,sto_set_item_txtname1,sto_set_item_txtset1, \
        sto_set_item_line1,1,1,1,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,    RS_STORAGE_SELECT,NULL)

DEF_CONTAINER(g_win_sto_setting,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    sto_set_con_keymap,sto_set_con_callback,  \
    (POBJECT_HEAD)&sto_set_item_con1, 1,0)


/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
static UINT8 m_sd_ide_statue = 0;    // 1: sd active, 0: ide active

static void win_pvr_sd_ide_save(void)
{
    //char hint[64];
    //UINT8 back_saved;
    //win_popup_choice_t choice;

    if(m_sd_ide_statue != sys_data_get_sd_ide_statue())
    {
        storage_switch_sd_ide_proc(m_sd_ide_statue, FALSE);
    }
}

static void win_sto_set_sd_ide_set_string(UINT8 input, PTEXT_FIELD ptxt)
{
    if(0==input)
    {
        osd_set_text_field_content(ptxt, STRING_ID,RS_STORAGE_HDD);
    }
    else
    {
        osd_set_text_field_content(ptxt, STRING_ID,RS_STORAGE_SD);
    }
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION sto_set_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

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


static void win_sto_set_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    if(1 == id)
    {
        m_sd_ide_statue = !m_sd_ide_statue;
        win_sto_set_sd_ide_set_string(m_sd_ide_statue, &sto_set_item_txtset1);
    }

    osd_track_object(pobj,C_UPDATE_ALL);
}

static void win_sto_set_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    if(1 == id)
    {
        m_sd_ide_statue = !m_sd_ide_statue;
        win_sto_set_sd_ide_set_string(m_sd_ide_statue, &sto_set_item_txtset1);
    }
    osd_track_object(pobj,C_UPDATE_ALL);
}

static void win_sto_set_enter_key(POBJECT_HEAD pobj,UINT8 id)
{
}

static PRESULT sto_set_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = VACT_PASS;
    //UINT8 input;
    UINT8    id = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if(VACT_DECREASE == unact)
        {
            win_sto_set_left_key(pobj,id);
        }
        else if(VACT_INCREASE == unact)
        {
            win_sto_set_right_key(pobj,id);
        }
        else if(VACT_ENTER == unact)
        {
            win_sto_set_enter_key(pobj,id);
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION sto_set_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

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
    case V_KEY_RECORD:
        act = VACT_RECORD_ALL;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT sto_set_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    //UINT8    unact;
    //UINT32 i;
    //BOOL subt_flag;
    //CONTAINER *cont = (CONTAINER*)pobj;
    //UINT8    id = OSD_GetObjID(pobj);
    //char new_volume[16];

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_sto_setting,RS_STORAGE_SETTING, 0);
        m_sd_ide_statue = sys_data_get_sd_ide_statue();
        win_sto_set_sd_ide_set_string(m_sd_ide_statue,&sto_set_item_txtset1);
        osd_set_container_focus(&g_win_sto_setting, 1);
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
        *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
        win_pvr_sd_ide_save();
        sys_data_save(1);
        break;

    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    default:
        break;
    }
    return ret;
}


