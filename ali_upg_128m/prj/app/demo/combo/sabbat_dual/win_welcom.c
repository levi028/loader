/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_welcom.c
*
*    Description: welcome menu.
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
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#endif
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"

#include "win_welcom.h"
#include "win_com_popup.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"
#include "win_language.h"
#include "win_time.h"
#include "win_countryband.h"
#include "win_auto_scan.h"
#include "control.h"
#include "win_tvsys.h"
#ifdef SHOW_WELCOME_FIRST

#if (defined(HDTV_SUPPORT) && defined(HDMI_ENABLE))
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#endif

//extern UINT16 *country_strs2[];
//#define WELCOME_COUNTRY_NCNT    17

#if defined(CHINESE_SUPPORT) && defined(PERSIAN_SUPPORT)
#define WELCOME_LANGUAGE_OSD_NUMBER 12
#elif defined(CHINESE_SUPPORT) || defined(PERSIAN_SUPPORT)
#define WELCOME_LANGUAGE_OSD_NUMBER 11
#else
#define WELCOME_LANGUAGE_OSD_NUMBER 10
#endif

#define WEL_TV_MODE_NCNT    (sizeof(wel_tv_mode) / sizeof(wel_tv_mode[0]))


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,\
                    hl,conobj,focus_id)      \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
                    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX, \
                    CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,welcom_item_keymap,\
                    welcom_item_callback, conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX, \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_TXTSET2(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX, \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    welcom_item_sel_keymap,welcom_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_txtset,ID,idu,idd,\
                        l,t,w,h,res_idname,res_idset,setstr) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,\
                &var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_txtset,l + TXT_L_OF,t + TXT_T_OF,TXT_W,\
                TXT_H,res_idname)    \
    LDEF_TXTSET(&var_con,var_txtset,NULL,1,1,1,1,1,l + SEL_L_OF ,t + SEL_T_OF,\
                SEL_W-1,SEL_H,res_idset,setstr)

#define LDEF_MENU_ITEM_SEL2(root,var_con,nxt_obj,var_txt,var_txtset,ID,idu,idd,\
                        l,t,w,h,res_idname,res_idset,setstr) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,\
                &var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_txtset,l + TXT_L_OF,t + TXT_T_OF,TXT_W,\
                TXT_H,res_idname)    \
    LDEF_TXTSET2(&var_con,var_txtset,NULL,1,1,1,1,1,l + SEL_L_OF ,t + SEL_T_OF,\
                    SEL_W-1,SEL_H,res_idset,setstr)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,\
                &var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l,t + TXT_T_OF,W_W-8,TXT_H,res_id)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    welcom_keymap,welcom_callback,  \
    nxt_obj, focus_id,0)

static VACTION welcom_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT welcom_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION welcom_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT welcom_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION welcom_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT welcom_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

static VACTION welcom_con_keymap(POBJECT_HEAD p_obj, UINT32 key);



DEF_TEXTFIELD(welcom_title,&g_win_welcom,&welcom_item_con1,C_ATTR_ACTIVE,0, \
                0,0,0,0,0,TITLE_L,TITLE_T ,TITLE_W,TITLE_H, TITLE_SH_IDX,\
                TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,NULL,NULL,     \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

LDEF_MENU_ITEM_SEL(g_win_welcom,welcom_item_con1,&welcom_item_con2,\
                    welcom_item_txt1,welcom_item_txt6, 1, 5, 2, \
                    CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, \
                    RS_SYSTEM_TIME_REGION_NAME, 0, NULL)

LDEF_MENU_ITEM_SEL(g_win_welcom,welcom_item_con2,&welcom_item_con3,\
                    welcom_item_txt2,welcom_item_txt7, 2, 1, 3, \
                    CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,\
                    RS_SYSTEM_LANGUAGE, 0, display_strs[0])

LDEF_MENU_ITEM_SEL2(g_win_welcom,welcom_item_con3,&welcom_item_con4,\
                    welcom_item_txt3,welcom_item_txt8, 3, 2, 4, \
                    CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, \
                    RS_SYSTEM_TV_DISPLAY_MODE, 0, display_strs[1])

LDEF_MENU_ITEM_SEL(g_win_welcom,welcom_item_con4,&welcom_item_con5,\
                    welcom_item_txt4,welcom_item_txt9, 4, 3, 5, \
                    CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, \
                    RS_SYSTEM_TV_ASPECT_MODE, 0, display_strs[2])


LDEF_MENU_ITEM_TXT(g_win_welcom,welcom_item_con5,NULL,welcom_item_txt5,5,4,1,\
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H,RS_COMMON_OK)

DEF_CONTAINER(g_win_welcom,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,\
    welcom_keymap,welcom_callback,  \
    (POBJECT_HEAD)&welcom_title, 1,0)

//LDEF_WIN(g_win_welcom,&welcom_item_con1,W_L, W_T, W_W, W_H,1)

//extern void win_autoscan_set_search_param(void);
//extern void win_autoscan_set_isdbt_param(BOOL isdbt_val);

UINT16 country_strs[] = //Should have the same order as stream_iso_639lang_abbr
{
    RS_REGION_ARG,
    RS_REGION_AUS,
    RS_REGION_BRA,
    RS_REGION_CHN,
    RS_REGION_DEN,
    RS_REGION_FIN,
    RS_REGION_FRA,
    RS_REGION_GER,
    RS_REGION_HK,
    RS_REGION_ITA,
    RS_REGION_POL,
    RS_REGION_RUS,
    RS_REGION_SIG,
    RS_REGION_SPA,
    RS_REGION_SWE,
    RS_REGION_TW,
    RS_REGION_UK,
#ifdef PERSIAN_SUPPORT
    RS_REGION_IRAN,
#endif
};

#define WELCOME_COUNTRY_NCNT  ARRAY_SIZE(country_strs)

#ifndef HDTV_SUPPORT
char* wel_tv_mode[] =
{
    "Auto",
    "PAL",
    "PAL-M",
    "PAL-N",
    "NTSC",
    "SECAM"
};

UINT8 wel_tv_mode_set[] =
{
    TV_MODE_AUTO,
    TV_MODE_PAL,
    TV_MODE_PAL_M,
    TV_MODE_PAL_N,
    TV_MODE_NTSC358,
    TV_MODE_SECAM
};
#else
#ifdef SD_PVR
char* wel_tv_mode[] =
{
    "576i",
};

UINT8 wel_tv_mode_set[] =
{
    TV_MODE_PAL,
};
#else
#ifdef HDMI_TV_TEST
//wel_tv_mode[] should be the same with win_tvsys.c
char* wel_tv_mode[] =
{
    "By Source",
    "By Native TV",
    "480i",
    "480p",
    "576i",
    "576p",
#ifndef SD_PVR
    "720p_50",
    "720p_60",
    "1080i_25",
    "1080i_30",
#ifdef HDMI_1080P_SUPPORT
    "1080p_50",
    "1080p_60",
#ifndef HDMI_CERTIFICATION_UI
    "1080p_25",
    "1080p_30",
    "1080p_24",
#endif
#endif
#endif
};

//wel_tv_mode_set[] should be the same with win_tvsys.c
UINT8 wel_tv_mode_set[] =
{
    TV_MODE_AUTO,
    TV_MODE_BY_EDID,
    TV_MODE_NTSC358,
    TV_MODE_480P,
    TV_MODE_PAL,
    TV_MODE_576P,
#ifndef SD_PVR
    TV_MODE_720P_50,
    TV_MODE_720P_60,
    TV_MODE_1080I_25,
    TV_MODE_1080I_30,
#ifdef HDMI_1080P_SUPPORT
    TV_MODE_1080P_50,
    TV_MODE_1080P_60,
#ifndef HDMI_CERTIFICATION_UI
    TV_MODE_1080P_25,
    TV_MODE_1080P_30,
    TV_MODE_1080P_24,
#endif
#endif
#endif
};

char* wel_tv_mode_sd[] =
{
    "By Source",
    "480i",
    "576i",
};

UINT8 wel_tv_mode_set_sd[] =
{
    TV_MODE_AUTO,
    TV_MODE_NTSC358,
    TV_MODE_PAL,
};
#else
char* wel_tv_mode[] =
{
    //"By Source",
    //"By Native TV",
    //"480i",
    //"480p",
    "576i",
    "576p",
    "720p",//"720p_50",
    //"720p_60",
    "1080i",//"1080i_25",
    //"1080i_30",
};

UINT8 wel_tv_mode_set[] =
{
    //TV_MODE_AUTO,
    //TV_MODE_BY_EDID,
    //TV_MODE_NTSC358,
    //TV_MODE_480P,
    TV_MODE_PAL,
    TV_MODE_576P,
    TV_MODE_720P_50,
    //TV_MODE_720P_60,
    TV_MODE_1080I_25,
    //TV_MODE_1080I_30,
};

char* wel_tv_mode_sd[] =
{
    //"By Source",
    "480i",
    "576i",
};

UINT8 wel_tv_mode_set_sd[] =
{
    //TV_MODE_AUTO,
    TV_MODE_NTSC358,
    TV_MODE_PAL,
};
#endif

//#define WEL_TV_MODE_SD_NCNT   ARRAY_SIZE(wel_tv_mode_set_sd)
#define WEL_TV_MODE_SD_NCNT (sizeof(wel_tv_mode_sd) / sizeof(wel_tv_mode_sd[0]))

#endif
#if (defined(HDMI_VIDEO_RES_SWITCH)&&(HDMI_VIDEO_RES_SWITCH == HDMI_RES_AUTO_DETECT))
char* wel_tv_mode_dvi[] =
{
//  "By Source",
    "By Native TV",
    "480p",
    "576p",
    "720p_50",
    "720p_60",
    "1080i_25",
    "1080i_30",
#ifdef HDMI_1080P_SUPPORT
    "1080p_24",
    "1080p_25",
    "1080p_30",
    "1080p_50",
    "1080p_60",
#endif
};

UINT8 wel_tv_mode_set_dvi[] =
{
//  TV_MODE_AUTO,
    TV_MODE_BY_EDID,
    TV_MODE_480P,
    TV_MODE_576P,
    TV_MODE_720P_50,
    TV_MODE_720P_60,
    TV_MODE_1080I_25,
    TV_MODE_1080I_30,
#ifdef HDMI_1080P_SUPPORT
    TV_MODE_1080P_24,
    TV_MODE_1080P_25,
    TV_MODE_1080P_30,
    TV_MODE_1080P_50,
    TV_MODE_1080P_60,
#endif
};
#define WEL_TV_MODE_DVI_NCNT  (sizeof(wel_tv_mode_dvi)/sizeof(wel_tv_mode_dvi[0]))
#endif

#endif


char* wel_tv_ratio[] =
{
    "Auto",
    "4:3PS",
    "4:3LB",
    "16:9",
};
#define WEL_TV_RATIO_NCNT   (sizeof(wel_tv_ratio) / sizeof(wel_tv_ratio[0]))

char* wel_tv_ratio_hd[] =
{
    "16:9",
};
#define WEL_TV_RATIO_HD_NCNT  (sizeof(wel_tv_ratio_hd) / sizeof(wel_tv_ratio_hd[0]))

//adding welcome page when only open dvbt 2011 10 19
//extern MULTISEL item_country_sel;
UINT16 region_ids1[] =
{
    RS_REGION_ARG,
    RS_REGION_AUS,
    RS_REGION_BRA,
    RS_REGION_CHN,
    RS_REGION_DEN,
    RS_REGION_FIN,
    RS_REGION_FRA,
    RS_REGION_GER,
    RS_REGION_HK,
    RS_REGION_ITA,
    RS_REGION_POL,
    RS_REGION_RUS,
    RS_REGION_SIG,
    RS_REGION_SPA,
    RS_REGION_SWE,
    RS_REGION_TW,
    RS_REGION_UK,
#ifdef PERSIAN_SUPPORT
    RS_REGION_IRAN,
#endif
};

UINT8 g_enter_welcom;
UINT32 old_tvmode,new_tvmode;
UINT32 tvout_set_old,tvout_set_new;
UINT32 real_tvmode;

#ifdef SUPPORT_IRAN_CALENDAR
UINT8 iran_region_offset = 16;
BOOL iran_region_flag = 0;
#endif

void welcome_av_load_setting(void);
static UINT32 welcome_get_tv_mode_count(void);
UINT32 welcome_tvmode_setting_to_osd(UINT32 set);
static UINT32 welcome_get_tv_mode_set(UINT32 index);
static char* welcome_get_tv_mode_string(UINT32 index);
static void welcome_av_load_tv_mode_setting(BOOL b_draw);

void welcom_lan_set_menu_lan_string(UINT8 input)
{
    UINT16 str[50] = {0};

    com_uni_str_copy_char_n( (UINT8 *)str, osd_lang_strs[input], 50-1);
    osd_set_text_field_content(&welcom_item_txt7, STRING_UNICODE,(UINT32)str);
}


void welcom_time_set_country_string(UINT8 input)
{
#ifdef SUPPORT_IRAN_CALENDAR
    if(input == iran_region_offset)
    {
        iran_region_flag = 1;
    }
    else
    {
        iran_region_flag = 0;
    }
#endif
    osd_set_text_field_content(&welcom_item_txt6, STRING_ID,country_strs[input]);
}

static VACTION welcom_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static VACTION welcom_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act  = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
    }

    return act;
}

static PRESULT welcom_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 b_id = 0;
    VACTION unact = VACT_PASS;
    __MAYBE_UNUSED__ POBJECT_HEAD submenu = NULL;
    SYSTEM_DATA* p_sys_data=sys_data_get();
    INT32 hoffset = 0;
    INT32 moffset = 0;
    INT32 sel = 0;
    __MAYBE_UNUSED__ UINT32 m_num = 0;
    __MAYBE_UNUSED__ UINT32 sel_ratio = 0;
    char* sel_mode = NULL;
    POBJECT_HEAD p_obj2 = (POBJECT_HEAD)&welcom_item_txt8;
    __MAYBE_UNUSED__ POBJECT_HEAD p_obj_item = (POBJECT_HEAD)&welcom_item_con3;
    UINT16 sel2 = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    __MAYBE_UNUSED__ enum TV_SYS_TYPE e_tv_mode = TV_MODE_AUTO;
    SYSTEM_DATA* sys_data = sys_data_get(); //modify for adding welcome page when only open dvbt2011 10 18
    UINT8 max_osd_lang_num = 0;

    b_id = osd_get_obj_id(p_obj);
    MEMSET(&rect,0,sizeof(OSD_RECT));
    MEMSET(&param,0,sizeof(COM_POP_LIST_PARAM_T));
    rect.u_left=162+204;
    rect.u_width=240;
    rect.u_top = p_obj2->frame.u_top;
    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            if((VACT_ENTER == unact) && (WELCOM_OK == b_id))
            {
                //modify for adding welcome page when only open dvbt 2011 10 19
                osd_set_multisel_count(&item_country_sel,ARRAY_SIZE(region_ids1));
                osd_set_multisel_sel_table(&item_country_sel, region_ids1);
                osd_set_multisel_sel(&item_country_sel, sys_data->country);
                #ifdef _ISDBT_ENABLE_
                    win_autoscan_set_isdbt_param(TRUE);
                #endif
                win_autoscan_set_search_param();
                menu_stack_pop_all();
                ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,(UINT32)&g_win2_search, FALSE);
                g_enter_welcom=1;
            }
            else if((VACT_ENTER == unact) && (WELCOM_TVMODE == b_id))
            {
                rect.u_height=150;//40*5+20;
                param.cur = welcome_tvmode_setting_to_osd(p_sys_data->avset.tv_mode);
                old_tvmode =param.cur;
                tvout_set_old = api_video_get_tvout();
                sel2 = win_com_open_sub_list(POP_LIST_MENU_TVSYS,&rect,&param);
                p_sys_data->avset.tv_mode = tvmode_osd_to_setting(sel2);
                new_tvmode = welcome_tvmode_setting_to_osd(p_sys_data->avset.tv_mode);
                if(sel2!=param.cur)
                {
                    api_video_set_tvout(p_sys_data->avset.tv_mode);
                    tvout_set_new = api_video_get_tvout();
                }
                sel_mode = welcome_get_tv_mode_string((UINT32)sel2);
                osd_set_text_field_content(&welcom_item_txt8, STRING_ANSI,(UINT32)sel_mode);
                osd_track_object((POBJECT_HEAD)&welcom_item_con3,C_UPDATE_ALL);
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
                e_tv_mode = api_video_get_tvout();
                if (e_tv_mode >= TV_MODE_720P_50) // HD
                {
                    sel_ratio = win_av_get_index(p_sys_data);
                    osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio_hd[sel_ratio]);
                    osd_track_object((POBJECT_HEAD)&g_win_welcom,C_UPDATE_ALL);
                }
#endif
                if (old_tvmode != new_tvmode)
                {
                    if (0 == new_tvmode)
                    {
                        real_tvmode = old_tvmode;
                    }
                    else
                    if (0 == old_tvmode)
                    {
                        if (new_tvmode != real_tvmode)
                        {
                            old_tvmode = real_tvmode;
                            welcome_popup_timer();
                        }
//                      welcome_popup_timer();
                    }
                    else
                    {
                        welcome_popup_timer();
                    }
                }
            }
            else if(VACT_DECREASE == unact)
            {
                switch (b_id)
                {
                case WELCOM_REGION:
                    p_sys_data->country = (0==p_sys_data->country) ? (WELCOME_COUNTRY_NCNT-1) : (p_sys_data->country-1);
                    welcom_time_set_country_string(p_sys_data->country);
                    sys_data_set_country(p_sys_data->country);
                    osd_track_object((POBJECT_HEAD)&welcom_item_con1, C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
                    break;
                case WELCOM_LANGUAGE:
                    max_osd_lang_num = win_language_get_menu_language_osd_num();
                    max_osd_lang_num = (max_osd_lang_num < WELCOME_LANGUAGE_OSD_NUMBER)?\
                        max_osd_lang_num:WELCOME_LANGUAGE_OSD_NUMBER;
                    p_sys_data->lang.osd_lang = (0 == p_sys_data->lang.osd_lang) ? \
                                                (max_osd_lang_num-1) : (p_sys_data->lang.osd_lang-1);
                    welcom_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
                    sys_data_select_language(p_sys_data->lang.osd_lang);
                    osd_track_object((POBJECT_HEAD)&g_win_welcom,C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
                    break;
                case WELCOM_TVASPECT:
                    sel = win_av_get_index(p_sys_data);
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
                    e_tv_mode = api_video_get_tvout();
                    if (e_tv_mode >= TV_MODE_720P_50) // HD
                    {
                        sel = (sel > 0) ? (sel-1) : (INT32)(WEL_TV_RATIO_HD_NCNT-1);
                    }
                    else
#endif
                    {
                        sel = (sel > 0) ? (sel-1) : (INT32)(WEL_TV_RATIO_NCNT-1);
                    }
                    win_av_set_ratio(p_sys_data,sel);
                    sys_data_set_display_mode(&(p_sys_data->avset));
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
                    if (e_tv_mode >= TV_MODE_720P_50) // HD
                    {
                        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio_hd[sel]);
                    }
                    else
#endif
                    {

                        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio[sel]);
                    }
                    osd_track_object((POBJECT_HEAD)&welcom_item_txt9, C_UPDATE_ALL);
                    break;
                }
            }
            else if(VACT_INCREASE == unact)
            {
                switch (b_id)
                {
                case WELCOM_REGION:
                    p_sys_data->country = ((WELCOME_COUNTRY_NCNT-1)==p_sys_data->country) ? 0 : (p_sys_data->country+1);
                    welcom_time_set_country_string(p_sys_data->country);
                    sys_data_set_country(p_sys_data->country);
                    osd_track_object((POBJECT_HEAD)&welcom_item_con1, C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
                    break;
                case WELCOM_LANGUAGE:
                    max_osd_lang_num = win_language_get_menu_language_osd_num();
                    max_osd_lang_num = (max_osd_lang_num < WELCOME_LANGUAGE_OSD_NUMBER)?\
                        max_osd_lang_num:WELCOME_LANGUAGE_OSD_NUMBER;
                    p_sys_data->lang.osd_lang = ((max_osd_lang_num-1) == p_sys_data->lang.osd_lang) ? \
                                                0 : (p_sys_data->lang.osd_lang+1);
                    welcom_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
                    sys_data_select_language(p_sys_data->lang.osd_lang);
                    osd_track_object((POBJECT_HEAD)&g_win_welcom,C_DRAW_SIGN_EVN_FLG| C_UPDATE_ALL);
                    break;
                case WELCOM_TVASPECT:
                    sel = win_av_get_index(p_sys_data);
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
                    e_tv_mode = api_video_get_tvout();
                    if (e_tv_mode >= TV_MODE_720P_50) // HD
                    {
                        sel = (sel < (INT32)(WEL_TV_RATIO_HD_NCNT-1)) ? (sel+1) : 0;
                    }
                    else
#endif
                    {
                        sel = (sel < (INT32)(WEL_TV_RATIO_NCNT-1)) ? (sel+1) : 0;
                    }
                    win_av_set_ratio(p_sys_data,sel);
                    sys_data_set_display_mode(&(p_sys_data->avset));
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
                    if (e_tv_mode >= TV_MODE_720P_50) // HD
                    {
                        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio_hd[sel]);
                    }
                    else
#endif
                    {
                        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio[sel]);
                    }
                    osd_track_object((POBJECT_HEAD)&welcom_item_txt9, C_UPDATE_ALL);
                    break;
            }
        }
        if(p_sys_data->local_time.buse_gmt)
        {
            sys_data_set_utcoffset( gmt_offset_value_by_region[p_sys_data->country]);
            sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
            set_stc_offset( hoffset, moffset, 0);
        }
        ret = PROC_LOOP;
        break;
    }
    return ret;
}

static VACTION welcom_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_LEFT:
            //act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            //act = VACT_INCREASE;
            break;
        case V_KEY_ENTER:
            //act = VACT_POP_UP;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT welcom_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //VACTION unact = VACT_PASS;
    UINT32 sel = 0;
    UINT8 b_id = 0;
    //char str[10] = {0};
    UINT16 * uni_str = NULL;
    //UINT8* pstr = NULL;
    //UINT16 cur_idx = 0;
    //COM_POP_LIST_TYPE_T list_type = POP_LIST_TYPE_INVALID;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    //UINT16 i = 0;
    //UINT16 n = 0;
    //UINT32 flag = 0;
    //INT32 hoffset = 0;
    //INT32 moffset = 0;
    //SYSTEM_DATA* p_sys_data=sys_data_get();
    b_id = osd_get_obj_id(p_obj);

    MEMSET(&rect,0,sizeof(OSD_RECT));
    MEMSET(&param,0,sizeof(COM_POP_LIST_PARAM_T));
    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        if(WELCOM_LANGUAGE == b_id)
        {
            ;
        }
        else if(WELCOM_REGION == b_id)
        {
            ;
        }
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        break;
    }

    return ret;
}

static VACTION welcom_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT welcom_callback(POBJECT_HEAD p_obj, VEVENT event,UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //MULTISEL* msel =NULL;
    //UINT16 title_id = 0;
    //UINT16 sat_idx = 0;
    //UINT16 sat_cnt = 0;
    OSD_RECT rect;
    //POBJECT_HEAD lastitem = NULL;
    SYSTEM_DATA* p_sys_data=sys_data_get();
    TEXT_FIELD* txt = NULL;
    UINT8 lang_code[8] = {0};

    MEMSET(&rect,0,sizeof(OSD_RECT));
    switch(event)
    {
    case EVN_PRE_OPEN:

        if(TRUE == get_signal_stataus_show())
        {
            show_signal_status_osdon_off(0);
        }
        g_enter_welcom=0;
        txt = &welcom_title;
        welcom_time_set_country_string(p_sys_data->country);
        welcom_lan_set_menu_lan_string(p_sys_data->lang.osd_lang);
        sys_data_select_language(p_sys_data->lang.osd_lang);
        osd_set_text_field_content(txt,STRING_ID, (UINT32)RS_INSTALLATION_WELCOME);
        welcome_av_load_setting();
        break;
    case EVN_POST_OPEN:
        //win_satsrch_set_drawselectsats();
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        //*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        /*set epg language*/
        strncpy(lang_code,stream_iso_639lang_abbr[p_sys_data->lang.osd_lang], (8-1));
        lang_code[7] = 0;
    #ifdef EPG_MULTI_LANG
        epg_set_lang_code(lang_code);
    #endif
    #ifndef NEW_DEMO_FRAME
        uich_chg_apply_aud_language();
    #endif
        sys_data_save(1);
        break;
    case EVN_POST_CLOSE:
        menu_stack_pop_all();
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_mainmenu, FALSE);
        break;
    case EVN_MSG_GOT:
            if(CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH == param1)
            {
                welcome_av_load_tv_mode_setting(TRUE);
                ret = PROC_LOOP;
            }
        break;
    default:
        break;
    }
    return ret;
}

void welcome_av_load_setting(void)
{
    SYSTEM_DATA* sys_data = NULL;
    //UINT32 i = 0;
    UINT32 sel = 0;
    //UINT32 cnt = 0;
    //UINT32 rf_mode = 0;
    char* sel_mode = NULL;
    __MAYBE_UNUSED__ enum TV_SYS_TYPE e_tv_mode = TV_MODE_AUTO;
    
    sys_data = sys_data_get();

    /* TV MODE */
    sel = welcome_tvmode_setting_to_osd(sys_data->avset.tv_mode);
    sel_mode = welcome_get_tv_mode_string(sel);

    osd_set_text_field_content(&welcom_item_txt8, STRING_ANSI,(UINT32)sel_mode);

    /* TV Aspect */
    sel = win_av_get_index(sys_data);
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
    e_tv_mode = api_video_get_tvout();
    if (e_tv_mode >= TV_MODE_720P_50) // HD
    {
        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio_hd[sel]);
    }
    else
#endif
    {
        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio[sel]);
    }
}

static UINT32 welcome_get_tv_mode_count(void)
{
#if (defined(HDMI_ENABLE) && defined(HDMI_VIDEO_RES_SWITCH)&&(HDMI_VIDEO_RES_SWITCH == HDMI_RES_AUTO_DETECT))
    UINT32 hdmi_mode = 0;
    UINT32 native_res_index = 0;
    UINT32 i = 0;
    struct hdmi_device *hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    enum HDMI_API_RES support_res[8] = {HDMI_RES_INVALID};

    if((SCART_YUV == sys_data_get_scart_out()) &&(hdmi_dev!=NULL)
        &&(SUCCESS==hdmi_dev->io_control(hdmi_dev,HDMI_CMD_INTERFACE_MODE, (UINT32)&hdmi_mode, NULL)))
    {
        if (SUCCESS == api_get_hdmi_all_video_resolution(&native_res_index, support_res))
        {
            for(i = 0; i<8 && support_res[i] !=HDMI_RES_INVALID; i++)
            ;
            return i+1; // +1 for "By Native TV"
        }

        if (hdmi_mode != TRUE) // DVI Mode
        {
            return WEL_TV_MODE_DVI_NCNT;
        }
    }
    else
#endif
    {
        if (SCART_RGB == sys_data_get_scart_out() )
        {
            return WEL_TV_MODE_SD_NCNT;
        }
        return WEL_TV_MODE_NCNT;
    }
}

static UINT32 welcome_get_tv_mode_set(UINT32 index)
{
#if (defined(HDMI_ENABLE) && defined(HDMI_VIDEO_RES_SWITCH) && (HDMI_VIDEO_RES_SWITCH == HDMI_RES_AUTO_DETECT))
    UINT32 hdmi_mode = 0;
    UINT32 native_res_index = 0;
    UINT32 i = 0;
    UINT32 set_index = 0;

    struct hdmi_device *hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    enum HDMI_API_RES support_res[8] = {HDMI_RES_INVALID};

    if((SCART_YUV == sys_data_get_scart_out()) && (hdmi_dev != NULL)
        && (SUCCESS == hdmi_dev->io_control(hdmi_dev,HDMI_CMD_INTERFACE_MODE, (UINT32)&hdmi_mode, NULL)))
    {
        if (SUCCESS == api_get_hdmi_all_video_resolution(&native_res_index, support_res))
        {
            if(0 == index)
            {
                return wel_tv_mode_set[0];
            }
            if(HDMI_RES_480I == support_res[index-1])
            {
                set_index = 1;
            }
            else if (HDMI_RES_480P == support_res[index-1])
            {
                set_index = 2;
            }
            else if (HDMI_RES_576I == support_res[index-1])
            {
                set_index = 3;
            }
            else if (HDMI_RES_576P == support_res[index-1])
            {
                set_index = 4;
            }
            else if (HDMI_RES_720P_50 == support_res[index-1])
            {
                set_index = 5;
            }
            else if (HDMI_RES_720P_60 == support_res[index-1])
            {
                set_index = 6;
            }
            else if (HDMI_RES_1080I_25 == support_res[index-1])
            {
                set_index = 7;
            }
            else if (HDMI_RES_1080I_30 == support_res[index-1])
            {
                set_index = 8;
            }
        #ifdef HDMI_1080P_SUPPORT
            else if (HDMI_RES_1080P_24 == support_res[index-1])
            {
                set_index = 9;
            }
            else if (HDMI_RES_1080P_25 == support_res[index-1])
            {
                set_index = 10;
            }
            else if (HDMI_RES_1080P_30 == support_res[index-1])
            {
                set_index = 11;
            }
            else if (HDMI_RES_1080P_50 == support_res[index-1])
            {
                set_index = 12;
            }
            else if (HDMI_RES_1080P_60 == support_res[index-1])
            {
                set_index = 13;
            }
        #endif
            else
            {
                set_index = 0;
            }
            return wel_tv_mode_set[set_index];
        }

        if (hdmi_mode != TRUE) // DVI Mode
        {
            return wel_tv_mode_set_dvi[index];
        }
    }
    else
#endif
    {
        if (SCART_RGB == sys_data_get_scart_out())
        {
            return (UINT32)wel_tv_mode_set_sd[index];
        }
        return (UINT32)wel_tv_mode_set[index];
    }
}

static char* welcome_get_tv_mode_string(UINT32 index)
{
#if (defined(HDMI_ENABLE) && defined(HDMI_VIDEO_RES_SWITCH)&& (HDMI_VIDEO_RES_SWITCH == HDMI_RES_AUTO_DETECT))
    UINT32 hdmi_mode = 0;
    UINT32 native_res_index = 0;
    UINT32 i = 0;
    UINT32 set_index = 0;
    struct hdmi_device *hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    enum HDMI_API_RES support_res[8] = {HDMI_RES_INVALID};

    if((SCART_YUV == sys_data_get_scart_out()) && (hdmi_dev != NULL) &&
        (SUCCESS == hdmi_dev->io_control(hdmi_dev,HDMI_CMD_INTERFACE_MODE, (UINT32)&hdmi_mode, NULL)))
    {
        if (SUCCESS == api_get_hdmi_all_video_resolution(&native_res_index, support_res))
        {
            if(0 == index)
            {
                return wel_tv_mode[0];
            }
            if(HDMI_RES_480I == support_res[index-1])
            {
                set_index = 1;
            }
            else if (HDMI_RES_480P == support_res[index-1])
            {
                set_index = 2;
            }
            else if (HDMI_RES_576I == support_res[index-1])
            {
                set_index = 3;
            }
            else if (HDMI_RES_576P == support_res[index-1])
            {
                set_index = 4;
            }
            else if (HDMI_RES_720P_50 == support_res[index-1])
            {
                set_index = 5;
            }
            else if (HDMI_RES_720P_60 == support_res[index-1])
            {
                set_index = 6;
            }
            else if (HDMI_RES_1080I_25 == support_res[index-1])
            {
                set_index = 7;
            }
            else if (HDMI_RES_1080I_30 == support_res[index-1])
            {
                set_index = 8;
            }
        #ifdef HDMI_1080P_SUPPORT
            else if (HDMI_RES_1080P_24 == support_res[index-1])
            {
                set_index = 9;
            }
            else if (HDMI_RES_1080P_25 == support_res[index-1])
            {
                set_index = 10;
            }
            else if (HDMI_RES_1080P_30 == support_res[index-1])
            {
                set_index = 11;
            }
            else if (HDMI_RES_1080P_50 == support_res[index-1])
            {
                set_index = 12;
            }
            else if (HDMI_RES_1080P_60 == support_res[index-1])
            {
                set_index = 13;
            }
        #endif

            else
            {
                set_index = 0;
            }
            return wel_tv_mode[set_index];
        }

        if (hdmi_mode != TRUE) // DVI Mode
        {
            return wel_tv_mode_dvi[index];
        }
    }
    else
#endif
    {
        if (SCART_RGB == sys_data_get_scart_out() )
        {
            return wel_tv_mode_sd[index];
        }
        return wel_tv_mode[index];
    }
}

UINT32 welcome_tvmode_setting_to_osd(UINT32 set)
{
    UINT32 i = 0;
    UINT32 n = welcome_get_tv_mode_count();

    for (i = 0; i < n; i++)
    {
        if (set == welcome_get_tv_mode_set(i))
        {
            break;
        }
    }
    if(i == n)
    {
        i = 0;
    }
    return i;
}

UINT32 welcome_popup_timer()
{
    UINT8 back_saved = 0;

    UINT32 cur_tvmode = 0;
    char* sel_mode = NULL;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    SYSTEM_DATA* p_sys_data=sys_data_get();

    win_compopup_init(WIN_POPUP_TYPE_OKNO);
    win_compopup_set_frame(300, 200, 400, 230);

    win_compopup_set_msg_ext(NULL,NULL, RS_POPUP_TVMODE_SAVE);
    win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
    win_compopup_set_timer(1000, CTRL_MSG_TYPE_POPUP_TIMER, 15);

    choice = win_compopup_open_ext(&back_saved);
    if(choice != WIN_POP_CHOICE_YES)
    {
        p_sys_data->avset.tv_mode = tvmode_osd_to_setting(old_tvmode);
        api_video_set_tvout(p_sys_data->avset.tv_mode);
        cur_tvmode = old_tvmode;
    }
    else
    {
        cur_tvmode = new_tvmode;
    }
    sel_mode = welcome_get_tv_mode_string(cur_tvmode);
    osd_set_text_field_content(&welcom_item_txt8, STRING_ANSI,(UINT32)sel_mode);
    osd_track_object((POBJECT_HEAD)&welcom_item_con3,C_UPDATE_ALL);
    return 0;
}

static void welcome_av_load_tv_mode_setting(BOOL b_draw)
{

    SYSTEM_DATA* p_sys_data = sys_data_get();
    UINT32 sel = 0;
    char* sel_mode = NULL;
    __MAYBE_UNUSED__ enum TV_SYS_TYPE e_tv_mode = TV_MODE_AUTO;

    /* TV MODE */
    sel = welcome_tvmode_setting_to_osd(p_sys_data->avset.tv_mode);
    sel_mode = welcome_get_tv_mode_string(sel);
    if (b_draw)
    {
        osd_set_text_field_content(&welcom_item_txt8,STRING_ANSI,(UINT32)sel_mode);
    }
#ifdef HDTV_CLOSE_ASPECT_RATIO_43
    /* TV Aspect */
    sel = win_av_get_index(p_sys_data);
    e_tv_mode = api_video_get_tvout();
    if (e_tv_mode >= TV_MODE_720P_50) // HD
    {
        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio_hd[sel]);
    }
    else
    {
        osd_set_text_field_content(&welcom_item_txt9, STRING_ANSI,(UINT32)wel_tv_ratio[sel]);
    }
#endif

    osd_track_object((POBJECT_HEAD)&g_win_welcom,C_UPDATE_ALL);
}

#endif

