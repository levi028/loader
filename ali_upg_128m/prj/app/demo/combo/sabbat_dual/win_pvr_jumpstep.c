/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_jumpstep.c

*    Description: timeshift jump key/msg proc when do timeshifting operation.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef DVR_PVR_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

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

#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif

#include "menus_root.h"
#include "win_pvr_jumpstep.h"


static VACTION jumpstep_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT jumpstep_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION jumpstep_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT jumpstep_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    jumpstep_item_con_keymap,jumpstep_item_con_callback,  \
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
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,name_id,setstr)\
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con, var_line, NULL, 1, 1, 1, 1, 1, l + LINE_L_OF, t + TXTS_H, LINE_W, LINE_H, 0, 0)

LDEF_MM_ITEM(g_win_jumpstep,jumpstep_item_con1,NULL,jumpstep_item_txtname1,jumpstep_item_txtset1, \
            jumpstep_item_line1,1,1,1,CON_L,CON_T+(CON_H+CON_GAP)*0,CON_W,CON_H,RS_DVR_JUMP,display_strs[0])

DEF_CONTAINER(g_win_jumpstep,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    jumpstep_con_keymap,jumpstep_con_callback,  \
    (POBJECT_HEAD)&jumpstep_item_con1, 1,0)


/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
#if 0
CONTAINER *jumpstep_items[] =
{
    &jumpstep_item_con1,
};

POBJECT_HEAD jumpstep_item_name[] =
{
    (POBJECT_HEAD)&jumpstep_item_txtname1,
};

POBJECT_HEAD jumpstep_item_set[] =
{
    (POBJECT_HEAD)&jumpstep_item_txtset1,
};
#endif

static void win_jumpstep_set_string(UINT8 input)
{
    UINT8 temp[10] = {0};
    UINT8 cnt = ARRAY_SIZE(temp) - 1;
    char *  __MAYBE_UNUSED__ ret = NULL;

    switch(input)
    {
        case 0:
            ret = strncpy((char *)temp, "30 sec", cnt);
            break;
        case 1:
            ret = strncpy((char *)temp, "1 min", cnt);
            break;
        case 2:
            ret = strncpy((char *)temp, "5 min", cnt);
            break;
        case 3:
            ret = strncpy((char *)temp, "10 min", cnt);
            break;
        case 4:
            ret = strncpy((char *)temp, "30 min", cnt);
            break;
        default:
            break;
    }
    com_asc_str2uni(temp, display_strs[0]);
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION jumpstep_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

#if 0
static PRESULT comlist_menu_jumpstep_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    OBJLIST *ol = NULL;
    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)p_obj;

#if 0
    if(event==EVN_PRE_DRAW)
    {
    }
    else if(event == EVN_POST_CHANGE)
        cb_ret = PROC_LEAVE;
#endif
    if(event == EVN_POST_CHANGE)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}
#endif

static PRESULT jumpstep_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 unact = 0;
    UINT8 input = 0;
    UINT8  __MAYBE_UNUSED__ id = 0;

    id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        input = sys_data_get_jumpstep();
        if(VACT_INCREASE == unact)
        {
            input = (input+1)%5;
        }
        else if(VACT_DECREASE == unact)
        {
            if(!input)
            {
                input = 5;
            }
            input = (input-1)%5;
        }
        sys_data_set_jumpstep(input);
        win_jumpstep_set_string(input);
        osd_track_object(p_obj,C_UPDATE_ALL);
        ret = PROC_LOOP;
        break;
    default:
        break;
    }

    return ret;

}

static VACTION jumpstep_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
   
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

static UINT8 preval = 0;
static PRESULT jumpstep_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    pvr_play_rec_t  *pvr_info = NULL;

    pvr_info  = api_get_pvr_info();
    switch(event)
    {
    case EVN_PRE_OPEN:
        preval = sys_data_get_jumpstep();
        wincom_open_title((POBJECT_HEAD)&g_win_jumpstep,RS_DVR_JUMP, 0);
        win_jumpstep_set_string(preval);
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
        *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
        if(preval != sys_data_get_jumpstep())
        {
            sys_data_save(1);
        }
        break;

    case EVN_POST_CLOSE:
        break;
    case EVN_MSG_GOT:
#ifdef DISK_MANAGER_SUPPORT
        if ((CTRL_MSG_SUBTYPE_CMD_STO == param1) && (USB_STATUS_OVER == param2))
        {
            // if disk used by PVR detached, exit this menu

            if (FALSE == pvr_info->hdd_valid)
            {
                ret = PROC_LEAVE;
            }
        }
#endif
        break;
    default:
        break;
    }
    return ret;
}


////////////////////////////////////////////////
#endif//#ifdef DVR_PVR_SUPPORT
