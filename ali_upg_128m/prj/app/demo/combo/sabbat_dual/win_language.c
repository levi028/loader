 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_language.c
*
*    Description:   The menu for user to select language setting
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
#include "menus_root.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_list.h"
#include "win_language.h"
#include "copper_common/system_data.h"
/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION lan_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT lan_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION lan_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT lan_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define BTN_SH_IDX   WSTL_BUTTON_SHOR_HD
#define BTN_HL_IDX   WSTL_BUTTON_SHOR_HI_HD
#define BTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define BTN_GRY_IDX  WSTL_BUTTON_SHOR_GRY_HD

#ifndef SD_UI
#define BTN_W       160
#define BTN_H       44

//the main window frame
#define MAIN_W_L        74//210 - 136
#define MAIN_W_T        98//138 - 40
#ifdef SUPPORT_CAS_A
#define MAIN_W_W        886
#else
#define MAIN_W_W        866
#endif
#define MAIN_W_H        488
#else
#define BTN_W       120
#define BTN_H       32

//the main window frame
#define MAIN_W_L        17//210 - 136
#define MAIN_W_T        57//138 - 40
#define MAIN_W_W        570
#define MAIN_W_H        370
#endif

#define MENU_LANGUAGE_STREAM_NUMBER (sizeof(stream_lang_str_ids)/2)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    lan_item_con_keymap,lan_item_con_callback,  \
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

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,ID,idu,idd,l,t,w,h,name_id,setstr)  \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,  NULL,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#ifdef SHOW_WELCOME_SCREEN
#define LDEF_TXT_BUTTON(root,var_txt,nxt_obj,ID,l,t,w,h,name_id)     \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,0,0,0,0, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,name_id,NULL)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,l,t,w,h,name_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT_BUTTON(&var_con,var_txt,NULL,ID,l,t,w,h,name_id)

enum
{
    LAN_ITEM1_ID = 1,
    LAN_ITEM2_ID,
    LAN_ITEM3_ID,
    LAN_ITEM4_ID,
    LAN_ITEM5_ID,
    ITEM_NEXT_ID,
    MAX_ID
};

LDEF_MM_ITEM(win_lan_con,lan_item_con1, &lan_item_con2,lan_item_txtname1,lan_item_txtset1,lan_item_line1,1,6,2,   \
            CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_LANGUAGE,display_strs[6])
#else
LDEF_MM_ITEM(win_lan_con,lan_item_con1, &lan_item_con2,lan_item_txtname1,lan_item_txtset1,lan_item_line1,1,5,2,   \
            CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_LANGUAGE,display_strs[0])
#endif

LDEF_MM_ITEM(win_lan_con,lan_item_con2, &lan_item_con3,lan_item_txtname2,  lan_item_txtset2,lan_item_line2,2,1,3,   \
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_SYSTEM_LANGUAGE_FIRST_AUDIO,NULL)
LDEF_MM_ITEM(win_lan_con,lan_item_con3, &lan_item_con4,lan_item_txtname3,lan_item_txtset3,lan_item_line3,3,2,4, \
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_SYSTEM_LANGUAGE_SECOND_AUDIO,NULL)
LDEF_MM_ITEM(win_lan_con,lan_item_con4, &lan_item_con5,lan_item_txtname4,lan_item_txtset4,lan_item_line4,4,3,5, \
                CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_SYSTEM_LANGUAGE_SUBTITLE_LANGUAGE,NULL)

#ifdef SHOW_WELCOME_SCREEN
LDEF_MM_ITEM(win_lan_con,lan_item_con5, &lan_item_con6,lan_item_txtname5,lan_item_txtset5,lan_item_line5,5,4,6, \
                CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_LANGUAGE_TELETEXT,NULL)

LDEF_MENU_ITEM_TXT(win_lan_con, lan_item_con6, NULL, lan_item_txtname6, 6, 5, 1, \
                   CON_W-40, CON_T + (CON_H + CON_GAP)*8, BTN_W, BTN_H, RS_DISPLAY_NEXT)
#else
LDEF_MM_ITEM(win_lan_con,lan_item_con5, NULL,lan_item_txtname5,lan_item_txtset5,lan_item_line5,5,4,1,   \
                CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_LANGUAGE_TELETEXT,NULL)
#endif

DEF_CONTAINER(win_lan_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    lan_con_keymap,lan_con_callback,  \
    (POBJECT_HEAD)&lan_item_con1, 1,0)

static UINT16 m_title_string_id = RS_SYSTEM_LANGUAGE;//RS_SYSTEM_WELCOME;

/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static CONTAINER *lan_items[] =
{
    &lan_item_con1,
    &lan_item_con2,
    &lan_item_con3,
    &lan_item_con4,
    &lan_item_con5,
#ifdef SHOW_WELCOME_SCREEN
    &lan_item_con6,
#endif
};

static UINT16 stream_lang_str_ids[] = /* Should have the same order as stream_iso_639lang_abbr*/
{
    RS_LANGUAGE_ENGLISH,
    RS_LANGUAGE_FRENCH,
    RS_LANGUAGE_GERMAN,
    RS_LANGUAGE_ITALIAN,
    RS_LANGUAGE_SPANISH,
    RS_LANGUAGE_PORTUGUESE,
    RS_LANGUAGE_RUSSIAN,
    RS_LANGUAGE_TURKISH,
    RS_LANGUAGE_POLISH,
    RS_LANGUAGE_ARIBIC,
    RS_LANGUAGE_PERSIAN,
#ifdef HINDI_LANGUAGE_SUPPORT
    RS_LANGUAGE_HINDI,
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
    RS_LANGUAGE_TELUGU,
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
    RS_LANGUAGE_BENGALI,
#endif	
};

static UINT8 lang_eng[]        = {0x00,0x45,0x00,0x6e,0x00,0x67,0x00,0x6c,0x00,0x69,0x00,0x73,0x00,0x68,0x00,0x00};
static UINT8 lang_french[]     = {0x00,0x46,0x00,0x72,0x00,0x61,0x00,0x6e,0x00,0xe7,0x00,0x61,0x00,\
                                                    0x69,0x00,0x73,0x00,0x00};
#ifndef SUPPORT_CAS9
static UINT8 lang_german[]     = {0x00,0x44,0x00,0x65,0x00,0x75,0x00,0x74,0x00,0x73,0x00,0x63,0x00,0x68,0x00,0x00};
static UINT8 lang_italian[]    = {0x00,0x49,0x00,0x74,0x00,0x61,0x00,0x6c,0x00,0x69,0x00,0x61,0x00,\
                                                    0x6e,0x00,0x6f,0x00,0x00};
static UINT8 lang_spanish[]    = {0x00,0x45,0x00,0x73,0x00,0x70,0x00,0x61,0x00,0xf1,0x00,0x6f,0x00,0x6c,0x00,0x00};
static UINT8 lang_portuguese[]= {0x00,0x50,0x00,0x6f,0x00,0x72,0x00,0x74,0x00,0x75,0x00,0x67,0x00,\
                                                    0x75,0x00,0xea,0x00,0x73,0x00,0x00};
static UINT8 lang_russian[]    = {0x04,0x20,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x3a,0x04,0x38,0x04,0x39,0x00,0x00};
static UINT8 lang_turkish[]    = {0x00,0x54,0x00,0xfc,0x00,0x72,0x00,0x6b,0x00,0xe7,0x00,0x65,0x00,0x00};
static UINT8 lang_polish[] = {0x00,0x50,0x00,0x6f,0x00,0x6c,0x00,0x73,0x00,0x6b,0x00,0x69,0x00,0x00};
static UINT8 lang_aribic[] = {0xfe,0xf2,0xfe,0x91,0xfe,0xae,0xfe,0xcb,0x00,0x00};
//static UINT8 lang_thai[] = {0x00,0x54,0x00,0x48,0x00,0x41,0x00,0x49,0x00,0x00};
static UINT8 lang_persian[]={0xfe,0xf2,0xfe,0xb3,0xfe,0xad,0xfe,0x8e,0xfe,0xd4,0xfe,0xdf,0xfe,0x8d,0x00,0x00};
#endif
#ifdef HINDI_LANGUAGE_SUPPORT
static UINT8 lang_hindi[]={0x09,0x39,0x09,0x3f,0x09,0x28,0x09,0x4d,0x09,0x26,0x09,0x40,0x00,0x00};
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
static UINT8 lang_telugu[]={0x0c,0x07,0x0c,0x02,0x0c,0x17,0x0c,0x4d,0x0c,0x32,0x0c,0x40,0x0c,0x37,0x0c,0x4d,0x00,0x00};
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
static UINT8 lang_bengali[]={0x09,0x87,0x09,0x82,0x09,0xb0,0x09,0xc7,0x09,0x9c,0x09,0xbf,0x00,0x00};
#endif



#ifdef SUPPORT_CAS9
UINT8 *osd_lang_strs[] =
{
    lang_eng,
    lang_french,
#ifdef HINDI_LANGUAGE_SUPPORT
    lang_hindi,
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
    lang_telugu,
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
    lang_bengali,
#endif	
};
#else
UINT8* osd_lang_strs[] =
{
    lang_eng,
    lang_french,
#ifdef HINDI_LANGUAGE_SUPPORT
    lang_hindi,
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
    lang_telugu,
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
    lang_bengali,
#endif	
    lang_german,
    lang_italian,
    lang_spanish,
    lang_portuguese,
    lang_russian,
    lang_turkish,
    lang_polish,
    lang_aribic,
    lang_persian,
    //  lang_thai
};
#endif

#define MENU_LANGUAGE_OSD_NUMBER (sizeof(osd_lang_strs)/sizeof(osd_lang_strs[0]))

UINT8 win_language_get_menu_language_osd_num(void)
{
    return MENU_LANGUAGE_OSD_NUMBER;
}

UINT8 win_language_get_menu_language_stream_num(void)
{
    return MENU_LANGUAGE_STREAM_NUMBER;
}

PRESULT comlist_menu_language_osd_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i= 0;
    UINT16 wtop= 0;
        OBJLIST *ol = NULL;

    PRESULT cbret = PROC_PASS;

        ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        wtop = osd_get_obj_list_top(ol);
        for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
        {
            win_comlist_set_str(i + wtop,NULL,(char *)osd_lang_strs[i+wtop],0 );
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }
    return cbret;
}

PRESULT comlist_menu_language_stream_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT cbret = PROC_PASS;
    UINT16 i= 0;
    UINT16 wtop= 0;
    OBJLIST *ol = NULL;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        wtop = osd_get_obj_list_top(ol);
        for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
        {
            win_comlist_set_str(i + wtop,NULL,NULL,stream_lang_str_ids[i + wtop] );
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }
    return cbret;
}

static void win_lan_set_menu_lan_string(UINT8 input)
{
#define MAX_UNI_STR_LEN   50
    UINT16 str[MAX_UNI_STR_LEN] = {0};
    INT32 str_ret = 0;

    //com_uni_str_copy_char( (UINT8 *)str, osd_lang_strs[input]);
    str_ret = com_uni_str_copy_char_n( (UINT8 *)str, osd_lang_strs[input], MAX_UNI_STR_LEN-1);
    if(0 == str_ret)
    {
        ali_trace(&str_ret);
    }
    osd_set_text_field_content(&lan_item_txtset1, STRING_UNICODE,(UINT32)str);
}

static void win_lan_set_1st_audio_lan_string(UINT8 input)
{
    osd_set_text_field_content(&lan_item_txtset2, STRING_ID,stream_lang_str_ids[input]);
}

static void win_lan_set_2nd_audio_lan_string(UINT8 input)
{
    osd_set_text_field_content(&lan_item_txtset3, STRING_ID,stream_lang_str_ids[input]);
}

static void win_lan_set_subtitle_lan_string(UINT8 input)
{
    osd_set_text_field_content(&lan_item_txtset4, STRING_ID,stream_lang_str_ids[input]);
}

static void win_lan_set_teletext_lan_string(UINT8 input)
{
    osd_set_text_field_content(&lan_item_txtset5, STRING_ID,stream_lang_str_ids[input]);
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION lan_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= VACT_PASS;

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

static void ap_language_setting(UINT16 audio_lang_1, UINT16 audio_lang_2)
 {
    sys_data_select_audio_language(audio_lang_1,audio_lang_2);
#ifndef NEW_DEMO_FRAME
    uich_chg_apply_aud_language();
#endif
 }

static void win_language_enter_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;

    MEMSET(&rect, 0 , sizeof(rect));
    MEMSET(&param, 0 , sizeof(param));
#ifndef SD_UI
    rect.u_left=CON_L+TXTS_L_OF;
    rect.u_width=TXTS_W;
    rect.u_height=340;
    param.selecttype = POP_LIST_SINGLESELECT;
#else
    rect.u_left=CON_L+TXTS_L_OF;
    rect.u_width=TXTS_W;
    rect.u_height=250;
    param.selecttype = POP_LIST_SINGLESELECT;
#endif
    switch(id)
    {
        case MENU_LANGUAGE_MENU_LAN:
            rect.u_top=CON_T;
            param.cur=p_sys_data->lang.osd_lang;
            p_sys_data->lang.osd_lang = win_com_open_sub_list(POP_LIST_MENU_LANGUAGE_OSD,&rect,&param);
            win_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
            sys_data_select_language(p_sys_data->lang.osd_lang);
            #ifdef BIDIRECTIONAL_OSD_STYLE
            osd_clear_screen();
            osd_draw_object((POBJECT_HEAD)&g_win_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            #endif
            wincom_open_title((POBJECT_HEAD)&win_lan_con,m_title_string_id, 0);
            osd_track_object((POBJECT_HEAD)&win_lan_con,C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
            break;
        case MENU_LANGUAGE_1ST_AUDIO_LAN:
            rect.u_top=CON_T + (CON_H + CON_GAP)*1;
            param.cur=p_sys_data->lang.audio_lang_1;
            p_sys_data->lang.audio_lang_1 = win_com_open_sub_list(POP_LIST_MENU_LANGUAGE_STREAM,&rect,&param);
            win_lan_set_1st_audio_lan_string(p_sys_data->lang.audio_lang_1);
            osd_track_object(pobj,C_UPDATE_ALL);
            ap_language_setting(p_sys_data->lang.audio_lang_1, p_sys_data->lang.audio_lang_2);
            break;
        case MENU_LANGUAGE_2ND_AUDIO_LAN:
            rect.u_top=CON_T + (CON_H + CON_GAP)*2;
            param.cur=p_sys_data->lang.audio_lang_2;
            p_sys_data->lang.audio_lang_2 = win_com_open_sub_list(POP_LIST_MENU_LANGUAGE_STREAM,&rect,&param);
            win_lan_set_2nd_audio_lan_string(p_sys_data->lang.audio_lang_2);
            osd_track_object(pobj,C_UPDATE_ALL);
            ap_language_setting(p_sys_data->lang.audio_lang_1, p_sys_data->lang.audio_lang_2);
            break;
        case MENU_LANGUAGE_SUB_LAN:
            rect.u_top=CON_T + (CON_H + CON_GAP)*3;
            param.cur=p_sys_data->lang.sub_lang;
            p_sys_data->lang.sub_lang = win_com_open_sub_list(POP_LIST_MENU_LANGUAGE_STREAM,&rect,&param);
            win_lan_set_subtitle_lan_string(p_sys_data->lang.sub_lang);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_LANGUAGE_TXT_LAN:
            rect.u_top=CON_T + (CON_H + CON_GAP)*3;
            #ifndef SD_UI
            rect.u_height=300;
            #else
            rect.u_height=200;
            #endif
            param.cur=p_sys_data->lang.ttx_lang;
            p_sys_data->lang.ttx_lang = win_com_open_sub_list(POP_LIST_MENU_LANGUAGE_STREAM,&rect,&param);
            win_lan_set_teletext_lan_string(p_sys_data->lang.ttx_lang);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_language_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_LANGUAGE_MENU_LAN:
            if(0 == p_sys_data->lang.osd_lang)
            {
                p_sys_data->lang.osd_lang=MENU_LANGUAGE_OSD_NUMBER-1;
            }
            else
            {
                p_sys_data->lang.osd_lang--;
            }
            win_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
            sys_data_select_language(p_sys_data->lang.osd_lang);
            #ifdef BIDIRECTIONAL_OSD_STYLE
            osd_clear_screen();
            osd_draw_object((POBJECT_HEAD)&g_win_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            #endif
            wincom_open_title((POBJECT_HEAD)&win_lan_con,m_title_string_id, 0);
            osd_track_object((POBJECT_HEAD)&win_lan_con,C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
            break;
        case MENU_LANGUAGE_1ST_AUDIO_LAN:
            if(0 == p_sys_data->lang.audio_lang_1)
            {
                p_sys_data->lang.audio_lang_1=MENU_LANGUAGE_STREAM_NUMBER-1;
            }
            else
            {
                p_sys_data->lang.audio_lang_1--;
            }
            win_lan_set_1st_audio_lan_string(p_sys_data->lang.audio_lang_1);
            osd_track_object(pobj,C_UPDATE_ALL);
            ap_language_setting(p_sys_data->lang.audio_lang_1, p_sys_data->lang.audio_lang_2);
            break;
        case MENU_LANGUAGE_2ND_AUDIO_LAN:
            if(0 == p_sys_data->lang.audio_lang_2)
            {
                p_sys_data->lang.audio_lang_2=MENU_LANGUAGE_STREAM_NUMBER-1;
            }
            else
            {
                p_sys_data->lang.audio_lang_2--;
            }
            win_lan_set_2nd_audio_lan_string(p_sys_data->lang.audio_lang_2);
            osd_track_object(pobj,C_UPDATE_ALL);
            ap_language_setting(p_sys_data->lang.audio_lang_1, p_sys_data->lang.audio_lang_2);
            break;
        case MENU_LANGUAGE_SUB_LAN:
            if(0 == p_sys_data->lang.sub_lang)
            {
                p_sys_data->lang.sub_lang=MENU_LANGUAGE_STREAM_NUMBER-1;
            }
            else
            {
                p_sys_data->lang.sub_lang--;
            }
            win_lan_set_subtitle_lan_string(p_sys_data->lang.sub_lang);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_LANGUAGE_TXT_LAN:
            if(0 == p_sys_data->lang.ttx_lang)
            {
                p_sys_data->lang.ttx_lang=MENU_LANGUAGE_STREAM_NUMBER-1;
            }
            else
            {
                p_sys_data->lang.ttx_lang--;
            }
            win_lan_set_teletext_lan_string(p_sys_data->lang.ttx_lang);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static void win_language_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_LANGUAGE_MENU_LAN:
            if((MENU_LANGUAGE_OSD_NUMBER-1) == p_sys_data->lang.osd_lang)
            {
                p_sys_data->lang.osd_lang=0;
            }
            else
            {
                p_sys_data->lang.osd_lang++;
            }
            win_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
            sys_data_select_language(p_sys_data->lang.osd_lang);
            #ifdef BIDIRECTIONAL_OSD_STYLE
            osd_clear_screen();
            osd_draw_object((POBJECT_HEAD)&g_win_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            #endif
            wincom_open_title((POBJECT_HEAD)&win_lan_con,m_title_string_id, 0);
            osd_track_object((POBJECT_HEAD)&win_lan_con,C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
            break;
        case MENU_LANGUAGE_1ST_AUDIO_LAN:
            if((MENU_LANGUAGE_STREAM_NUMBER-1) == p_sys_data->lang.audio_lang_1)
            {
                p_sys_data->lang.audio_lang_1=0;
            }
            else
            {
                p_sys_data->lang.audio_lang_1++;
            }
            win_lan_set_1st_audio_lan_string(p_sys_data->lang.audio_lang_1);
            osd_track_object(pobj,C_UPDATE_ALL);
            ap_language_setting(p_sys_data->lang.audio_lang_1, p_sys_data->lang.audio_lang_2);
            break;
        case MENU_LANGUAGE_2ND_AUDIO_LAN:
            if((MENU_LANGUAGE_STREAM_NUMBER-1) == p_sys_data->lang.audio_lang_2)
            {
                p_sys_data->lang.audio_lang_2=0;
            }
            else
            {
                p_sys_data->lang.audio_lang_2++;
            }
            win_lan_set_2nd_audio_lan_string(p_sys_data->lang.audio_lang_2);
            osd_track_object(pobj,C_UPDATE_ALL);
            ap_language_setting(p_sys_data->lang.audio_lang_1, p_sys_data->lang.audio_lang_2);
            break;
        case MENU_LANGUAGE_SUB_LAN:
            if((MENU_LANGUAGE_STREAM_NUMBER-1) == p_sys_data->lang.sub_lang)
            {
                p_sys_data->lang.sub_lang=0;
            }
            else
            {
                p_sys_data->lang.sub_lang++;
            }
            win_lan_set_subtitle_lan_string(p_sys_data->lang.sub_lang);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_LANGUAGE_TXT_LAN:
            if((MENU_LANGUAGE_STREAM_NUMBER-1) == p_sys_data->lang.ttx_lang)
            {
                p_sys_data->lang.ttx_lang=0;
            }
            else
            {
                p_sys_data->lang.ttx_lang++;
            }
            win_lan_set_teletext_lan_string(p_sys_data->lang.ttx_lang);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        default:
            break;
    }
}

static PRESULT lan_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= PROC_PASS;
    UINT8   id = osd_get_obj_id(pobj);

#ifdef SHOW_WELCOME_SCREEN
    POBJECT_HEAD submenu = NULL;
#endif
#ifdef EPG_MULTI_LANG
    SYSTEM_DATA *p_sys_data=sys_data_get();
    UINT8 lang_code[8]={0};
#endif

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH) ;
#ifdef SHOW_WELCOME_SCREEN
        if(id != ITEM_NEXT_ID)
        {
#endif
            switch(unact)
            {
                case VACT_ENTER:
                    win_language_enter_key(pobj,id);
                    break;
                case VACT_DECREASE:
                    win_language_left_key(pobj,id);
                    break;
                case VACT_INCREASE:
                    win_language_right_key(pobj,id);
                    break;
                default:
                    break;
            }
#ifdef SHOW_WELCOME_SCREEN
        }
        else if(VACT_ENTER == unact)
        {
    #ifdef EPG_MULTI_LANG
            /*set epg language*/
            p_sys_data = sys_data_get();
            strncpy(lang_code,stream_iso_639lang_abbr[p_sys_data->lang.osd_lang], (8-1));
            lang_code[7] = 0;
            epg_set_lang_code(lang_code);
    #endif

    #ifndef NEW_DEMO_FRAME
            uich_chg_apply_aud_language();
    #endif
            sys_data_save(1);
            ttxeng_set_g0_set();
            //show_win_as_welcome = FALSE;
            submenu = (POBJECT_HEAD) & win_country_net_con;
            if (osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
            {
                menu_stack_pop();
                menu_stack_push(submenu);
            }
        }
#endif
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION lan_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

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

static PRESULT lan_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
#ifdef EPG_MULTI_LANG
    UINT8 lang_code[8]={0};
#endif
    SYSTEM_DATA *p_sys_data = NULL;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&win_lan_con,m_title_string_id, 0);
#ifdef SHOW_WELCOME_SCREEN
//modify for the title of language submenu show welcome
//welcome page has been solve by other method, not use the language submenu anymore
        //if(show_win_as_welcome)
        if(1)
        {
            osd_set_attr(&lan_item_con6, C_ATTR_ACTIVE);
            ((POBJECT_HEAD)(&lan_item_con1))->b_up_id = 6;
            ((POBJECT_HEAD)(&lan_item_con5))->b_down_id = 6;
            win_lan_con.head.style.b_show_idx = WSTL_WIN_BODYLEFT_01_HD;
            win_lan_con.head.style.b_hlidx = WSTL_WIN_BODYLEFT_01_HD;
            win_lan_con.head.style.b_sel_idx = WSTL_WIN_BODYLEFT_01_HD;
            win_lan_con.head.style.b_gray_idx = WSTL_WIN_BODYLEFT_01_HD;
            osd_set_rect(&(win_lan_con.head.frame), MAIN_W_L, W_T, MAIN_W_W, W_H);
        }
        else
        {
            osd_set_attr(&lan_item_con6, C_ATTR_HIDDEN);
            ((POBJECT_HEAD)(&lan_item_con1))->b_up_id = 5;
            ((POBJECT_HEAD)(&lan_item_con5))->b_down_id = 1;
            win_lan_con.head.style.b_show_idx = WIN_SH_IDX;
            win_lan_con.head.style.b_hlidx = WIN_HL_IDX;
            win_lan_con.head.style.b_sel_idx = WIN_SL_IDX;
            win_lan_con.head.style.b_gray_idx = WIN_GRY_IDX;

            osd_set_rect(&(win_lan_con.head.frame), W_L, W_T, W_W, W_H);
            //remove_menu_item(&win_lan_con, (OBJECT_HEAD *)&lan_item_con6, CON_H + CON_GAP);

        }
         ((CONTAINER*)(&win_lan_con))->focus_object_id = 1;
#endif

        p_sys_data=sys_data_get();
        win_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
        win_lan_set_1st_audio_lan_string(p_sys_data->lang.audio_lang_1);
        win_lan_set_2nd_audio_lan_string(p_sys_data->lang.audio_lang_2);
        win_lan_set_subtitle_lan_string(p_sys_data->lang.sub_lang);
        win_lan_set_teletext_lan_string(p_sys_data->lang.ttx_lang);
#ifdef SUPPORT_POP_SUBT
        remove_menu_item(&win_lan_con, (POBJECT_HEAD)lan_items[3], (CON_H + CON_GAP));
#endif
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
#ifdef EPG_MULTI_LANG
        /*set epg language*/
        p_sys_data = sys_data_get();
        strncpy(lang_code,stream_iso_639lang_abbr[p_sys_data->lang.osd_lang], (8-1));
        lang_code[7] = 0;
        epg_set_lang_code(lang_code);
#endif

#ifndef NEW_DEMO_FRAME
        uich_chg_apply_aud_language();
#endif
        sys_data_save(1);
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

        break;

    case EVN_POST_CLOSE:
    #if (TTX_ON == 1)
        ttxeng_set_g0_set();
    #endif
        break;
    default:
        break;
    }
    return ret;
}

