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
#include "win_parental.h"
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

#define MENU_LOCK_STREAM_NUMBER 2
#define TOP_PASSWORD_COUNT 3

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
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL) \
    LDEF_TXTSET(&var_con,var_txtset,  NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MM_ITEM2(root,var_con,nxt_obj,var_txt_name,var_txtset,ID,idu,idd,l,t,w,h,name_id,setstr) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET2(&var_con,var_txtset,  NULL        ,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)

LDEF_MM_ITEM(win_parental_con,par_item_con1, &par_item_con2,par_item_txtname1,par_item_txtset1,par_item_line0,1,4,2,\
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_MENU_LOCK,NULL)
LDEF_MM_ITEM(win_parental_con,par_item_con2, &par_item_con3,par_item_txtname2,  par_item_txtset2,par_item_line1,2,1,3,\
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_PARENTAL_RATING_LOCK,NULL)
LDEF_MM_ITEM(win_parental_con,par_item_con3, &par_item_con4,par_item_txtname3,  par_item_txtset3,par_item_line2,3,2,4,\
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_PARENTAL_CONTENT_LOCK,NULL)
LDEF_MM_ITEM2(win_parental_con,par_item_con4, &par_item_con5,par_item_txtname4,par_item_txtset4,4,3,5,  \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_NEW_PASSWORD,display_strs[0])
LDEF_MM_ITEM2(win_parental_con,par_item_con5, NULL,par_item_txtname5,par_item_txtset5,5,4,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_SYSTME_PARENTAL_LOCK_CONFIRM_PASSWORD,\
                display_strs[1])

DEF_CONTAINER(win_parental_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    par_con_keymap,par_con_callback,  \
    (POBJECT_HEAD)&par_item_con1, 1,0)

static UINT32      win_pl_password_value[2];
static UINT8       g_pl_input_bit_number = 0;
static UINT8       g_pl_input_status = 0;

/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static CONTAINER *par_items[] =
{
    &par_item_con1,
    &par_item_con2,
    &par_item_con3,
    &par_item_con4,
    &par_item_con5,
};

#ifdef PARENTAL_SUPPORT
#ifndef POLAND_SPEC_SUPPORT
static UINT16 stream_rating_str_ids[] = /*CSA only 4 level for rating control -10,-12,-16,-18*/
{
    RS_COMMON_OFF,
    RS_PARENTAL_AGE_10,
    RS_PARENTAL_AGE_12,
    RS_PARENTAL_AGE_14,
    RS_PARENTAL_AGE_16,
    RS_PARENTAL_AGE_18,
};
#else
static UINT16 stream_rating_str_ids[] =
{
    RS_COMMON_OFF,
    RS_PARENTAL_AGE_7,
    RS_PARENTAL_AGE_12,
    RS_PARENTAL_AGE_16,
    RS_PARENTAL_AGE_18,
};
#endif
#define MENU_RATING_STREAM_NUMBER (sizeof(stream_rating_str_ids)/2)

static UINT16 stream_content_str_ids[] =
{
    RS_COMMON_OFF,
    RS_PARENTAL_DRUGS,
    RS_PARENTAL_VIOLENCE,
    RS_PARENTAL_SEX,
    RS_PARENTAL_VIOLENCE_DRUGS,
    RS_PARENTAL_SEX_DRUGS,
    RS_PARENTAL_VIOLENCE_SEX,
    RS_PARENTAL_VIOLENCE_SEX_DRUGS,
};
#define MENU_CONTENT_STREAM_NUMBER (sizeof(stream_content_str_ids)/2)

#endif
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
#ifdef PARENTAL_SUPPORT
    osd_set_text_field_content(&par_item_txtset2, STRING_ID,stream_rating_str_ids[input]);
#else
    if(0 == input)
        osd_set_text_field_content(&par_item_txtset2, STRING_ID,RS_COMMON_OFF);
    else
        osd_set_text_field_content(&par_item_txtset2, STRING_ID,RS_COMMON_ON);
#endif
}

static void win_par_set_content_lock_string(UINT8 input)
{
#ifdef PARENTAL_SUPPORT
    osd_set_text_field_content(&par_item_txtset3, STRING_ID,stream_content_str_ids[input]); 
#else
    if(0 == input)
        osd_set_text_field_content(&par_item_txtset3, STRING_ID,RS_COMMON_OFF);
    else
        osd_set_text_field_content(&par_item_txtset3, STRING_ID,RS_COMMON_ON);
#endif
}

static void win_par_switch_confirm(BOOL flag)
{
    UINT8 action = 0;

    if(flag)
    {
        action = C_ATTR_ACTIVE;
    }
    else
    {
        action = C_ATTR_INACTIVE;
    }
    if( !osd_check_attr(&par_item_con5, action))
    {
        osd_set_attr(&par_item_con5, action);
        osd_set_attr(&par_item_txtname5, action);
        osd_set_attr(&par_item_txtset5, action);
        osd_draw_object((POBJECT_HEAD )&par_item_con5,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }

}

static void win_pl_set_password_str(UINT8 index)
{
    UINT8 temp[5] = {0};
    INT32 ret = 0;

    MEMSET(temp,0,sizeof(temp));
    if((TRUE == g_pl_input_status)&&(g_pl_input_bit_number>0))
    {
        switch(g_pl_input_bit_number)
        {
            case 1:
                ret = snprintf((char*)temp, 5, "*---");
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                break;
            case 2:
                ret = snprintf((char*)temp, 5, "**--");
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                break;
            case 3:
                ret = snprintf((char*)temp, 5, "***-");
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                break;
            case 4:
                ret = snprintf((char*)temp, 5, "****");
                if(0 == ret)
                {
                    ali_trace(&ret);
                }
                break;
            default:
                break;
        }
    }
    else
    {
        ret = snprintf((char*)temp, 5, "----");
        if(0 == ret)
        {
            ali_trace(&ret);
        }
    }
    #ifdef BIDIRECTIONAL_OSD_STYLE
    if (TRUE == osd_get_mirror_flag() )
    {
        UINT8 i, tmp;
        for (i=0; i<2; i++)
        {
            tmp = temp[i];
            temp[i] = temp[3-i];
            temp[3-i] = tmp;
        }
    }
    #endif
    com_asc_str2uni(temp, display_strs[index]);
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION par_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

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
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        if((MENU_PARENTAL_PASSWORD == osd_get_focus_id((POBJECT_HEAD)&win_parental_con))
            ||(MENU_PARENTAL_PASSWORD2 == osd_get_focus_id((POBJECT_HEAD)&win_parental_con)))
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
    UINT8 age=0;
    UINT8 content=0;

    age=p_sys_data->rating_sel&0xf;
    content=p_sys_data->rating_sel>>4;

    switch(id)
    {
        case MENU_PARENTAL_MENU_LOCK:
            p_sys_data->menu_lock=1-p_sys_data->menu_lock;
            win_par_set_menu_lock_string(p_sys_data->menu_lock);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PARENTAL_CHANNEL_LOCK:
            if(0 == age)
            {
                age=MENU_RATING_STREAM_NUMBER-1;
            }
            else
            {
                age--;
            }
            p_sys_data->rating_sel = (p_sys_data->rating_sel&0xf0)|age;
            win_par_set_channel_lock_string(age);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PARENTAL_CONTENT_LOCK:
            /* need Sam to separate (UINT8)rating_sel */
            if(0 == content)
            {
                content=MENU_CONTENT_STREAM_NUMBER-1;
            }
            else
            {
                content--;
            }
            p_sys_data->rating_sel = (content<<4) | (p_sys_data->rating_sel&0xf);
            win_par_set_content_lock_string(content);//bit4~bit7
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_par_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    UINT8 age=0;
    UINT8 content=0;

    age=p_sys_data->rating_sel&0xf;
    content=p_sys_data->rating_sel>>4;

    switch(id)
    {
        case MENU_PARENTAL_MENU_LOCK:
            p_sys_data->menu_lock=1-p_sys_data->menu_lock;
            win_par_set_menu_lock_string(p_sys_data->menu_lock);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PARENTAL_CHANNEL_LOCK:
            if((MENU_RATING_STREAM_NUMBER-1) == age)
            {
                age=0;
            }
            else
            {
                age++;
            }
            p_sys_data->rating_sel = (p_sys_data->rating_sel&0xf0)|age;
            win_par_set_channel_lock_string(age);

            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_PARENTAL_CONTENT_LOCK:
            /* need Sam to separate (UINT8)rating_sel */
            if((MENU_CONTENT_STREAM_NUMBER-1) == content)
            {
                content = 0;
            }
            else
            {
                content++;
            }
            p_sys_data->rating_sel = (content<<4) | (p_sys_data->rating_sel&0xf);
            win_par_set_content_lock_string(content);//bit4~bit7

            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_par_enter_key(POBJECT_HEAD pobj, UINT8 id)
{
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT8 sel=0;
    SYSTEM_DATA *p_sys_data = sys_data_get();

    rect.u_left = CON_L+TXTS_L_OF;
    rect.u_width = TXTS_W;
    rect.u_height = 150;
    param.selecttype = POP_LIST_SINGLESELECT;

    switch (id)
    {
        case MENU_PARENTAL_MENU_LOCK:
            rect.u_top = CON_T;
            param.cur = p_sys_data->menu_lock;
            p_sys_data->menu_lock = win_com_open_sub_list(POP_LIST_MENU_PARENTAL_MENU_LOCK, &rect, &param);
            win_par_set_menu_lock_string(p_sys_data->menu_lock);
            break;
        case MENU_PARENTAL_CHANNEL_LOCK:
            rect.u_top = CON_T + (CON_H + CON_GAP)*1;
            param.cur = p_sys_data->rating_sel&0xf;
            sel = win_com_open_sub_list(POP_LIST_MENU_PARENTAL_CHANNEL_LOCK, &rect, &param);
            p_sys_data->rating_sel = (p_sys_data->rating_sel&0xf0)|sel;
            win_par_set_channel_lock_string(sel);
            break;
        case MENU_PARENTAL_CONTENT_LOCK:
            rect.u_width = TXTS_W+20;
            rect.u_top = CON_T + (CON_H + CON_GAP)*2;
            param.cur = p_sys_data->rating_sel>>4;
            sel = win_com_open_sub_list(POP_LIST_MENU_PARENTAL_CONTENT_LOCK, &rect, &param);
            p_sys_data->rating_sel = (sel<<4)|(p_sys_data->rating_sel&0xf);
            win_par_set_content_lock_string(sel);
            break;
        default:
            break;
    }
    osd_track_object(pobj,C_UPDATE_ALL);
}

UINT8 win_parental_get_osd_num(void)
{
    UINT8 id = 0;
    UINT8 ret = 0;

    id = osd_get_focus_id((POBJECT_HEAD)&win_parental_con);
    switch (id)
    {
        case MENU_PARENTAL_MENU_LOCK:
            ret = MENU_LOCK_STREAM_NUMBER;
            break;
        case MENU_PARENTAL_CHANNEL_LOCK:
            ret = MENU_RATING_STREAM_NUMBER;
            break;
        case MENU_PARENTAL_CONTENT_LOCK:
            ret = MENU_CONTENT_STREAM_NUMBER;
            break;
        default:
            break;
    }
    return ret;
}


PRESULT comlist_menu_parental_osd_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i = 0;
    UINT16 wtop = 0;
    UINT8 id = 0;
        OBJLIST *ol = NULL;

    PRESULT cbret = PROC_PASS;

        ol = (OBJLIST*)pobj;

    if (EVN_PRE_DRAW == event)
    {
        id = osd_get_focus_id((POBJECT_HEAD)&win_parental_con);
        wtop = osd_get_obj_list_top(ol);
        switch (id)
        {
            case MENU_PARENTAL_MENU_LOCK:
                win_comlist_set_str(0, NULL, NULL, RS_COMMON_OFF);
                win_comlist_set_str(1, NULL, NULL, RS_COMMON_ON);
                break;
            case MENU_PARENTAL_CHANNEL_LOCK:
                for (i=0; i<ol->w_dep && (i+wtop)<ol->w_count; i++)
                {
                    win_comlist_set_str(i+wtop, NULL, NULL, stream_rating_str_ids[i+wtop]);
                }
                break;
            case MENU_PARENTAL_CONTENT_LOCK:
                for (i=0; i<ol->w_dep && (i+wtop)<ol->w_count; i++)
                {
                    win_comlist_set_str(i+wtop, NULL, NULL, stream_content_str_ids[i+wtop]);
                }
                break;
            default:
                break;
        }
    }
    else if (EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }
    return cbret;
}

static void win_pl_input_number(UINT8 item,UINT8 number)
{
    if(TRUE == g_pl_input_status)
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
    UINT8   unact= PROC_PASS;
    UINT8   save = 0;
    UINT8   id = osd_get_obj_id(pobj);
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH) ;
        if(VACT_DECREASE == unact)
        {
            win_par_left_key(pobj,id);
        }
        else if(VACT_INCREASE == unact)
        {
            win_par_right_key(pobj,id);
        }
        else if(VACT_ENTER == unact)
        {
            win_par_enter_key(pobj,id);
        }
        else if(unact <= VACT_NUM_9)
        {
            if((TRUE == g_pl_input_status)&&(TOP_PASSWORD_COUNT == g_pl_input_bit_number))
            {
                if(MENU_PARENTAL_PASSWORD == id)
                {
                    win_pl_input_number(0,(UINT8)(unact-VACT_NUM_0));
                    win_pl_set_password_str(0);
                    win_par_switch_confirm(TRUE);
                    g_pl_input_bit_number=0;
                    g_pl_input_status=FALSE;
                    win_pl_password_value[1]=0;
                    win_pl_set_password_str(1);
                    osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_PASSWORD2,C_UPDATE_ALL);
                    osd_draw_object((POBJECT_HEAD)&par_item_con4, C_UPDATE_ALL);
                    osd_track_object((POBJECT_HEAD)&par_item_con5,C_UPDATE_ALL);
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
                        #ifndef SD_UI
                        win_compopup_set_frame(GET_MID_L(550),GET_MID_T(220),550,200);
                        #endif
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
    VACTION act = PROC_PASS;

    switch(key)
    {
    case V_KEY_UP:
        if(MENU_PARENTAL_PASSWORD == osd_get_focus_id((POBJECT_HEAD)&win_parental_con))
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_set_password_str(0);
        }
        else if(MENU_PARENTAL_PASSWORD2 == osd_get_focus_id((POBJECT_HEAD)&win_parental_con))
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_password_value[1]=0;
            win_pl_set_password_str(0);
            win_pl_set_password_str(1);
            win_par_switch_confirm(FALSE);
            osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_PASSWORD,C_UPDATE_ALL);
            osd_track_object((POBJECT_HEAD)&par_item_con4,C_UPDATE_ALL);
            act = VACT_PASS;
            break;
        }
        act = VACT_CURSOR_UP;
        break;

    case V_KEY_DOWN:
        if(MENU_PARENTAL_PASSWORD == osd_get_focus_id((POBJECT_HEAD)&win_parental_con))
        {
            g_pl_input_bit_number=0;
            g_pl_input_status=FALSE;
            win_pl_password_value[0]=0;
            win_pl_set_password_str(0);
        }
        else if(MENU_PARENTAL_PASSWORD2 == osd_get_focus_id((POBJECT_HEAD)&win_parental_con))
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
            osd_draw_object((POBJECT_HEAD)&par_item_con4, C_UPDATE_ALL);
            act = VACT_PASS;
            break;
        }
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_ENTER:
        act = VACT_ENTER;
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
    CONTAINER *cont = (CONTAINER*)pobj;
    SYSTEM_DATA *p_sys_data = NULL;

    UINT32 default_key_exit[] = {V_KEY_EXIT,V_KEY_MENU,V_KEY_LEFT,V_KEY_UP,V_KEY_DOWN};

    switch(event)
    {
    case EVN_PRE_OPEN:
#ifndef ISDBT_SUPPORT
        remove_menu_item(cont, (OBJECT_HEAD *)par_items[2], CON_H + CON_GAP);
#endif
#if (defined _S3281_ || defined _C0200A_CA_ENABLE_)
        remove_menu_item(cont, (OBJECT_HEAD *)par_items[1], CON_H + CON_GAP);
#endif
        if(FALSE == win_pwd_open(default_key_exit,ARRAY_SIZE(default_key_exit)))
        {
            return PROC_LEAVE;
        }
        wincom_open_title((POBJECT_HEAD)&win_parental_con,RS_SYSTME_PARENTAL_LOCK, 0);
        osd_change_focus((POBJECT_HEAD)&win_parental_con,MENU_PARENTAL_MENU_LOCK,0);
        p_sys_data=sys_data_get();
        win_par_set_menu_lock_string(p_sys_data->menu_lock);
        /* need Sam to separate (UINT8)rating_sel */
        win_par_set_channel_lock_string(p_sys_data->rating_sel&0x0f);
        win_par_set_content_lock_string(p_sys_data->rating_sel>>4);
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

