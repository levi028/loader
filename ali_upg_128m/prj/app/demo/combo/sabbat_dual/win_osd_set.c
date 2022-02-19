/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_osd_set.c
*
*    Description:   The realize of OSD setting
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
#include "win_osd_set.h"

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION osd_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT osd_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION osd_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT osd_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    osd_item_con_keymap,osd_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL, C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL, C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,name_id,setstr)  \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

LDEF_MM_ITEM(win_osdset_con,osd_item_con1, &osd_item_con2,osd_item_txtname1,osd_item_txtset1,osd_item_line1,1,3,2,  \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_OSD_SETTING_SUBTITLE_DISPLAY,NULL)
LDEF_MM_ITEM(win_osdset_con,osd_item_con2, &osd_item_con3,osd_item_txtname2,  osd_item_txtset2,osd_item_line2,2,1,3, \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_SYSTEM_OSD_SETTING_OSD_TIMEOUT,display_strs[0])
LDEF_MM_ITEM(win_osdset_con,osd_item_con3, NULL,osd_item_txtname3,osd_item_txtset3,osd_item_line3,3,2,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_SYSTEM_OSD_SETTING_OSD_TRANSPARENCY,display_strs[1])

DEF_CONTAINER(win_osdset_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    osd_con_keymap,osd_con_callback,  \
    (POBJECT_HEAD)&osd_item_con1, 1,0)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
#define MAX_OSD_TIMEOUT 10

static void win_osd_set_subtitle_string(UINT8 input)
{
    if(0 == input)
    {
        osd_set_text_field_content(&osd_item_txtset1, STRING_ID,RS_COMMON_OFF);
    }
    else
    {
        osd_set_text_field_content(&osd_item_txtset1, STRING_ID,RS_COMMON_ON);
    }
}

static void win_osd_set_timeout_string(UINT8 input)
{
    UINT8 temp[5] = {0};
    INT32 ret = 0;

    ret = snprintf((char *)temp, 5, "%d", input);
    if(0 == ret)
    {
        ali_trace(&ret);
    }
    com_asc_str2uni(temp, display_strs[0]);
}

static void win_osd_set_trans_string(UINT8 input)
{
    UINT8 temp[5] = {0};
    int ret = 0;

    if(input)
    {
        ret = snprintf((char *)temp, 5, "%d0%%", input);
        if(0 == ret)
        {
            ali_trace(&ret);
        }
        com_asc_str2uni(temp, display_strs[1]);
    }
    else
    {
        strncpy((char *)temp,"Off", 4);
        com_asc_str2uni(temp, display_strs[1]);
    }
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION osd_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;

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

UINT8 win_osd_get_num(void)
{
    UINT8 id = osd_get_focus_id((POBJECT_HEAD)&win_osdset_con);
    UINT8 num = 0;

    switch(id)
    {
        case MENU_OSD_TIMEOUT:
            num=MAX_OSD_TIMEOUT;
            break;
        case MENU_OSD_TRANSPARENCY:
            num=MAX_OSD_TRANSPARENCY+1;
            break;
        default:
            break;
    }
    return num;
}

PRESULT comlist_menu_osd_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16  i       = 0;
    UINT16  wtop    = 0;
    UINT8   id      = 0;
    OBJLIST *ol     = NULL;
    UINT8   temp[4] = {0};
    PRESULT cbret   = PROC_PASS;
    INT32 ret = 0;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        id=osd_get_focus_id((POBJECT_HEAD)&win_osdset_con);
        switch(id)
        {
            case MENU_OSD_TIMEOUT:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        ret = snprintf((char *)temp, 4, "%d", i+wtop+1);
                        if(0 == ret)
                        {
                            ali_trace(&ret);
                        }
                        win_comlist_set_str(i + wtop,(char *)temp,NULL,0);
                    }
                break;
            case MENU_OSD_TRANSPARENCY:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                    {
                        ret = snprintf((char *)temp, 4, "%d0%%", i+wtop);
                        if(0 == ret)
                        {
                            ali_trace(&ret);
                        }
                        if(i+wtop)
                        {
                            win_comlist_set_str(i + wtop,(char *)temp,NULL,0);
                        }
                        else
                        {
                            win_comlist_set_str(i + wtop,NULL,NULL,RS_COMMON_OFF);
                        }
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

static void win_osd_enter_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;

    MEMSET(&param,0,sizeof(COM_POP_LIST_PARAM_T));
    MEMSET(&rect,0,sizeof(OSD_RECT));

    rect.u_left=CON_L+TXTS_L_OF;
    rect.u_width=TXTS_W;
    rect.u_height=250;
    param.selecttype = POP_LIST_SINGLESELECT;
    switch(id)
    {
        case MENU_OSD_TIMEOUT:
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*1;
            #else
            rect.u_height = 250;
            rect.u_top=CON_T + (CON_H)*1;
            #endif

            param.cur=p_sys_data->osd_set.time_out-1;
            p_sys_data->osd_set.time_out = win_com_open_sub_list(POP_LIST_MENU_OSDSET,&rect,&param)+1;
            win_osd_set_timeout_string(p_sys_data->osd_set.time_out);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_OSD_TRANSPARENCY:
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*2;
            #else
            rect.u_height = 250;
            rect.u_top=CON_T + (CON_H + CON_GAP)*1 + CON_H;
            #endif

            param.cur=p_sys_data->osd_set.osd_trans;
            p_sys_data->osd_set.osd_trans = win_com_open_sub_list(POP_LIST_MENU_OSDSET,&rect,&param);
            win_osd_set_trans_string(p_sys_data->osd_set.osd_trans);
            osd_track_object(pobj,C_UPDATE_ALL);
            sys_data_set_palette(0);
            break;
        default:
            break;
    }
}

static void win_osd_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data = sys_data_get();

    switch(id)
    {
        case MENU_OSD_SUB_DISPLAY:
            if(0==p_sys_data->osd_set.subtitle_display)
            {
                p_sys_data->osd_set.subtitle_display=1;
            }
            else
            {
                p_sys_data->osd_set.subtitle_display--;
            }
            win_osd_set_subtitle_string(p_sys_data->osd_set.subtitle_display);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_OSD_TIMEOUT:
            if(1==p_sys_data->osd_set.time_out)
            {
                p_sys_data->osd_set.time_out=MAX_OSD_TIMEOUT;
            }
            else
            {
                p_sys_data->osd_set.time_out--;
            }
            win_osd_set_timeout_string(p_sys_data->osd_set.time_out);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_OSD_TRANSPARENCY:
            if(0==p_sys_data->osd_set.osd_trans)
            {
                p_sys_data->osd_set.osd_trans=MAX_OSD_TRANSPARENCY;
            }
            else
            {
                p_sys_data->osd_set.osd_trans--;
            }
            win_osd_set_trans_string(p_sys_data->osd_set.osd_trans);
            osd_track_object(pobj,C_UPDATE_ALL);
            sys_data_set_palette(0);
            break;
        default:
            break;
    }
}

static void win_osd_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data = sys_data_get();

    switch(id)
    {
        case MENU_OSD_SUB_DISPLAY:
            if(1==p_sys_data->osd_set.subtitle_display)
            {
                p_sys_data->osd_set.subtitle_display=0;
            }
            else
            {
                p_sys_data->osd_set.subtitle_display++;
            }
            win_osd_set_subtitle_string(p_sys_data->osd_set.subtitle_display);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_OSD_TIMEOUT:
            if(MAX_OSD_TIMEOUT == p_sys_data->osd_set.time_out)
            {
                p_sys_data->osd_set.time_out=1;
            }
            else
            {
                p_sys_data->osd_set.time_out++;
            }
            win_osd_set_timeout_string(p_sys_data->osd_set.time_out);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_OSD_TRANSPARENCY:
            if(MAX_OSD_TRANSPARENCY == p_sys_data->osd_set.osd_trans)
            {
                p_sys_data->osd_set.osd_trans=0;
            }
            else
            {
                p_sys_data->osd_set.osd_trans++;
            }
            win_osd_set_trans_string(p_sys_data->osd_set.osd_trans);
            osd_track_object(pobj,C_UPDATE_ALL);
            sys_data_set_palette(0);
            break;
        default:
            break;
    }
}

static PRESULT osd_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret   = PROC_PASS;
    UINT8   unact = 0;
    UINT8   id    = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if(VACT_ENTER == unact)
        {
            win_osd_enter_key(pobj,id);
        }
        else if(VACT_DECREASE == unact)
        {
            win_osd_left_key(pobj,id);
        }
        else if(VACT_INCREASE == unact)
        {
            win_osd_right_key(pobj,id);
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION osd_con_keymap(POBJECT_HEAD pobj, UINT32 key)
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
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT osd_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    SYSTEM_DATA *p_sys_data = NULL;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&win_osdset_con,RS_SYSTEM_OSD_SETTING, 0);
#ifdef SUPPORT_POP_SUBT
        remove_menu_item((CONTAINER *) pobj,(POBJECT_HEAD)&osd_item_con1,(CON_H + CON_GAP));
#endif
        p_sys_data=sys_data_get();
        if(0 == p_sys_data->osd_set.subtitle_display)
        {
            p_sys_data->osd_set.subtitle_lang = 0xFF;/*If subtitle off,clear set_sub_lang_idx*/
        }
        win_osd_set_subtitle_string(p_sys_data->osd_set.subtitle_display);
        win_osd_set_timeout_string(p_sys_data->osd_set.time_out);
        win_osd_set_trans_string(p_sys_data->osd_set.osd_trans);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        p_sys_data = sys_data_get();
        if(0 == p_sys_data->osd_set.subtitle_display)
        {
            p_sys_data->osd_set.subtitle_lang = 0xFF;/*If subtitle off,clear set_sub_lang_idx*/
        }
        sys_data_save(1);
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    default:
        break;
    }
    return ret;
}

