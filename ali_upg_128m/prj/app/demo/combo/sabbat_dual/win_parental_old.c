/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_parental.c
*
*    Description:   The realize of parental function
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
#include "win_password.h"

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION par_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT par_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION par_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT par_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER win_parental_con;

CONTAINER par_item_con1;
CONTAINER par_item_con2;
CONTAINER par_item_con3;
CONTAINER par_item_con4;

TEXT_FIELD par_item_txtname1;
TEXT_FIELD par_item_txtname2;
TEXT_FIELD par_item_txtname3;
TEXT_FIELD par_item_txtname4;

TEXT_FIELD par_item_txtset1;
TEXT_FIELD par_item_txtset2;
TEXT_FIELD par_item_txtset3;
TEXT_FIELD par_item_txtset4;

TEXT_FIELD par_item_line0;
TEXT_FIELD par_item_line1;


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    par_item_con_keymap,par_item_con_callback,  \
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
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_TXTSET2(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,WSTL_BUTTON_05_HD,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,name_id,setstr)  \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
    LDEF_TXTSET(&var_con,var_txtset,  NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MM_ITEM2(root,var_con,nxt_obj,var_txt_name,var_txtset,ID,idu,idd,l,t,w,h,name_id,setstr) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
    LDEF_TXTSET2(&var_con,var_txtset,  NULL        ,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)

LDEF_MM_ITEM(win_parental_con,par_item_con1, &par_item_con2,par_item_txtname1,par_item_txtset1,\
    par_item_line0,1,5,2, CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_MENU_LOCK,NULL)
LDEF_MM_ITEM(win_parental_con,par_item_con2, &par_item_con3,par_item_txtname2,  par_item_txtset2,\
    par_item_line1,2,1,4,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_CHANNEL_LOCK,NULL)
LDEF_MM_ITEM2(win_parental_con,par_item_con3, &par_item_con4,par_item_txtname3,par_item_txtset3,4,2,5,  \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_NEW_PASSWORD,display_strs[0])
LDEF_MM_ITEM2(win_parental_con,par_item_con4, NULL,par_item_txtname4,par_item_txtset4,5,4,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_CONFIRM_PASSWORD,\
                display_strs[1])

DEF_CONTAINER(win_parental_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    par_con_keymap,par_con_callback,  \
    (POBJECT_HEAD)&par_item_con1, 1,0)

UINT32      win_pl_password_value[2] ;
UINT8       g_pl_input_bit_number = 0;
UINT8       g_pl_input_status = 0;

#define TOP_PASSWORD_COUNT 3

/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static void win_par_set_menu_lock_string(UINT8 input)
{
    if(0 == input)
    {
        osd_set_text_field_content(&par_item_txtset1, STRING_ID,RS_COMMON_OFF);
    }
    else
    {
        osd_set_text_field_content(&par_item_txtset1, STRING_ID,RS_COMMON_ON);
    }
}

static void win_par_set_channel_lock_string(UINT8 input)
{
    if(0 == input)
    {
        osd_set_text_field_content(&par_item_txtset2, STRING_ID,RS_COMMON_OFF);
    }
    else
    {
        osd_set_text_field_content(&par_item_txtset2, STRING_ID,RS_COMMON_ON);
    }
}

static void win_par_switch_confirm(BOOL flag)
{
    UINT8 action =C_ATTR_INACTIVE;

    if(flag)
    {
        action = C_ATTR_ACTIVE;
    }
    else
    {
        action = C_ATTR_INACTIVE;
    }
    if( !osd_check_attr(&par_item_con4, action))
    {
        osd_set_attr(&par_item_con4, action);
        osd_set_attr(&par_item_txtname4, action);
        osd_set_attr(&par_item_txtset4, action);
        osd_draw_object((POBJECT_HEAD )&par_item_con4,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }

}

static void win_pl_set_password_str(UINT8 index)
{
    UINT8 temp[5] = {0};

    MEMSET(temp,0,sizeof(temp));
    if((g_pl_input_status==TRUE)&&(g_pl_input_bit_number>0))
    {
        switch(g_pl_input_bit_number)
        {
            case 1:
                strncpy((char *)temp, "*---", 4);
                break;
            case 2:
                strncpy((char *)temp, "**--", 4);
                break;
            case 3:
                strncpy((char *)temp, "***-", 4);
                break;
            case 4:
                strncpy((char *)temp, "****", 4);
                break;
            default:
                break;
        }
    }
    else
    {
        strncpy((char *)temp, "----", 4);
    }
    com_asc_str2uni(temp, display_strs[index]);
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION par_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
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
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        if((osd_get_focus_id((POBJECT_HEAD)&win_parental_con)==MENU_PARENTAL_PASSWORD)
            ||(osd_get_focus_id((POBJECT_HEAD)&win_parental_con)==MENU_PARENTAL_PASSWORD2))
        {
            act = VACT_NUM_0+(key-V_KEY_0);
        }
        else
        {
            act = VACT_PASS;
        }
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static void win_par_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_PARENTAL_MENU_LOCK:
            p_sys_data->menu_lock=1-p_sys_data->menu_lock;
            win_par_set_menu_lock_string(p_sys_data->menu_lock);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PARENTAL_CHANNEL_LOCK:
            p_sys_data->channel_lock=1-p_sys_data->channel_lock;
            win_par_set_channel_lock_string(p_sys_data->channel_lock);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_par_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_PARENTAL_MENU_LOCK:
            p_sys_data->menu_lock=1-p_sys_data->menu_lock;
            win_par_set_menu_lock_string(p_sys_data->menu_lock);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PARENTAL_CHANNEL_LOCK:
            p_sys_data->channel_lock=1-p_sys_data->channel_lock;
            win_par_set_channel_lock_string(p_sys_data->channel_lock);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_pl_input_number(UINT8 item,UINT8 number)
{
    if(g_pl_input_status==TRUE)
    {
        switch(g_pl_input_bit_number)
        {
            case 0:
            case 4:
                win_pl_password_value[item]=number;
                g_pl_input_bit_number=1;
                break;
            case 1:
            case 2:
            case 3:
                win_pl_password_value[item]=number+win_pl_password_value[item]*10;
                g_pl_input_bit_number++;
                break;
            default:
                break;
        }
    }
    else
    {
        g_pl_input_status=TRUE;
        win_pl_password_value[item]=number;
        g_pl_input_bit_number=1;
    }
}

static PRESULT par_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;
    //UINT8   input=0;
    UINT8   save=0;
    UINT8   id = osd_get_obj_id(pobj);
    SYSTEM_DATA *p_sys_data=sys_data_get();

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
            win_par_left_key(pobj,id);
        }
        else if( VACT_INCREASE== unact)
        {
            win_par_right_key(pobj,id);
        }
        else if( VACT_NUM_9 >=unact)
        {
            if((g_pl_input_status==TRUE)&&(g_pl_input_bit_number==TOP_PASSWORD_COUNT))
            {
                if(id==MENU_PARENTAL_PASSWORD)
                {
                    win_pl_input_number(0,(UINT8)(unact-VACT_NUM_0));
                    win_pl_set_password_str(0);
                    win_par_switch_confirm(TRUE);
                    g_pl_input_bit_number=0;
                    g_pl_input_status=FALSE;
                    win_pl_password_value[1]=0;
                    win_pl_set_password_str(1);
                    osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_PASSWORD2,C_UPDATE_ALL);
                    osd_draw_object((POBJECT_HEAD)&par_item_con3, C_UPDATE_ALL);
                    osd_track_object((POBJECT_HEAD)&par_item_con4,C_UPDATE_ALL);
                }
                else
                {
                    win_pl_input_number(1,(UINT8)(unact-VACT_NUM_0));
                    if(win_pl_password_value[0]==win_pl_password_value[1])
                    {
                        win_pl_set_password_str(1);
                        osd_track_object(pobj,C_UPDATE_ALL);
                   
                        win_compopup_init(WIN_POPUP_TYPE_OK);
                        win_compopup_set_msg(NULL,NULL,RS_MSG_CHANGE_PASSWORD_SUCCESS);
                        win_compopup_set_frame(GET_MID_L(550),GET_MID_T(220),550,200);
                        win_compopup_open_ext(&save);
                        p_sys_data->menu_password=win_pl_password_value[0];
                        return PROC_LEAVE;
                    }
                    else
                    {
                        g_pl_input_bit_number=0;
                        g_pl_input_status=FALSE;
                        win_pl_password_value[1]=0;
                        win_pl_set_password_str(1);
                        osd_track_object(pobj,C_UPDATE_ALL);
                    }
                }
            }
            else
            {
                win_pl_input_number(id-MENU_PARENTAL_PASSWORD,(UINT8)(unact-VACT_NUM_0));
                win_pl_set_password_str(id-MENU_PARENTAL_PASSWORD);
                osd_track_object(pobj,C_UPDATE_ALL);
            }
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION par_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_UP:
        if(osd_get_focus_id((POBJECT_HEAD)&win_parental_con)==MENU_PARENTAL_PASSWORD)
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_set_password_str(0);
        }
        else if(osd_get_focus_id((POBJECT_HEAD)&win_parental_con)==MENU_PARENTAL_PASSWORD2)
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_password_value[1]=0;
            win_pl_set_password_str(0);
            win_pl_set_password_str(1);
            win_par_switch_confirm(FALSE);
            osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_PASSWORD,C_UPDATE_ALL);
            osd_track_object((POBJECT_HEAD)&par_item_con3,C_UPDATE_ALL);
            act = VACT_PASS;
            break;
        }
        act = VACT_CURSOR_UP;
        break;

    case V_KEY_DOWN:
        if(osd_get_focus_id((POBJECT_HEAD)&win_parental_con)==MENU_PARENTAL_PASSWORD)
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_set_password_str(0);
        }
        else if(osd_get_focus_id((POBJECT_HEAD)&win_parental_con)==MENU_PARENTAL_PASSWORD2)
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_password_value[1]=0;
            win_pl_set_password_str(0);
            win_pl_set_password_str(1);
            win_par_switch_confirm(FALSE);
            osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_MENU_LOCK,C_UPDATE_ALL);
            osd_track_object((POBJECT_HEAD)&par_item_con1,C_UPDATE_ALL);
            osd_draw_object((POBJECT_HEAD)&par_item_con3, C_UPDATE_ALL);
            act = VACT_PASS;
            break;
        }
        act = VACT_CURSOR_DOWN;
        break;

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

static PRESULT par_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT32 i=0;
    //UINT8   unact=0;
    //CONTAINER *cont = (CONTAINER*)pobj;
    //UINT8   id = osd_get_obj_id(pobj);
    SYSTEM_DATA *p_sys_data=NULL;

    UINT32 default_key_exit[] =
    {
        V_KEY_EXIT,V_KEY_MENU,V_KEY_LEFT,V_KEY_UP,V_KEY_DOWN
    };

    switch(event)
    {
    case EVN_PRE_OPEN:

        if(win_pwd_open(default_key_exit,ARRAY_SIZE(default_key_exit))==FALSE)
        {
            return PROC_LEAVE;
        }
        wincom_open_title((POBJECT_HEAD)&win_parental_con,RS_SYSTME_PARENTAL_LOCK, 0);
        osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_MENU_LOCK,0);
        p_sys_data=sys_data_get();
        win_par_set_menu_lock_string(p_sys_data->menu_lock);
        win_par_set_channel_lock_string(p_sys_data->channel_lock);
        win_par_switch_confirm(FALSE);
        g_pl_input_bit_number=0;
        g_pl_input_status=FALSE;
        win_pl_password_value[0]=0;
        win_pl_password_value[1]=0;
        win_pl_set_password_str(0);
        win_pl_set_password_str(1);
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
        sys_data_save(1);
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;

    case EVN_POST_CLOSE:
        break;
    default:
        break;
    }
    return ret;
}

