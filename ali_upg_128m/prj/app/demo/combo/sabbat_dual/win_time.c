/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_time.c
*
*    Description: The menu for time
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
#include <api/libsi/si_tdt.h>

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

#include "time_zone_name.h"
#include "win_time.h"

typedef struct
{
    UINT16 region_str;
    UINT8 timezone;
    UINT8 country_code[3];// reference to ISO3166-1
} REGION_PARAM_T;
#define TIME_ZONE(_hoff, _moff) (UINT8)(((int)(_hoff)*2+23)+(int)(_moff)/30)

#ifdef DVBC_SUPPORT
static REGION_PARAM_T region_param[] =
{    
    { RS_COUNTRY_GERMANY, TIME_ZONE(1, 0), "DEU" }, //DEU,not GER(maybe also GER)
    { RS_REGION_AUS, TIME_ZONE(10, 0), "AUS" },
    { RS_COUNTRY_SWITZERLAND, TIME_ZONE(1, 0), "CHE" },
    { RS_REGION_ITA, TIME_ZONE(1, 0), "ITA" },    
    { RS_COUNTRY_GREECE, TIME_ZONE(2, 0), "GRC" },
    { RS_COUNTRY_PORTUGAL, TIME_ZONE(0, 0), "PRT" },  
    { RS_REGION_ARG, TIME_ZONE(-3, 0), "ARG" },
    { RS_REGION_DEN, TIME_ZONE(1, 0), "DEN" },
    { RS_REGION_FRA, TIME_ZONE(1, 0), "FRA" },
    { RS_REGION_POL, TIME_ZONE(1, 0), "POL" },
    { RS_REGION_RUS, TIME_ZONE(3, 0), "RUS" },
    { RS_REGION_SPA, TIME_ZONE(1, 0), "SPA" },
};
#else
static REGION_PARAM_T region_param[] =
{
    { RS_REGION_ARG, TIME_ZONE(-3, 0), "ARG" },
    { RS_REGION_AUS, TIME_ZONE(10, 0), "AUS" },
    { RS_REGION_BRA, TIME_ZONE(-3, 0), "BRA" },
    { RS_REGION_CHN, TIME_ZONE(8, 0), "CHN" },
    { RS_REGION_DEN, TIME_ZONE(1, 0), "DEN" },
    { RS_REGION_FIN, TIME_ZONE(2, 0), "FIN" },
    { RS_REGION_FRA, TIME_ZONE(1, 0), "FRA" },
    { RS_REGION_GER, TIME_ZONE(1, 0), "DEU" }, //DEU,not GER(maybe also GER)
    { RS_REGION_HK, TIME_ZONE(8, 0), "HKG" },
    { RS_REGION_ITA, TIME_ZONE(1, 0), "ITA" },
    { RS_REGION_POL, TIME_ZONE(1, 0), "POL" },
    { RS_REGION_RUS, TIME_ZONE(3, 0), "RUS" },
    { RS_REGION_SIG, TIME_ZONE(8, 0), "SIG" },
    { RS_REGION_SPA, TIME_ZONE(1, 0), "SPA" },
    { RS_REGION_SWE, TIME_ZONE(1, 0), "SWE" },
    { RS_REGION_TW, TIME_ZONE(8, 0), "TWN" },
    { RS_REGION_UK, TIME_ZONE(0, 0), "GBR" },
#ifdef PERSIAN_SUPPORT
    { RS_REGION_IRAN, TIME_ZONE(3, 30), "IRN" },
#endif
};
#endif
#define REGION_CNT  ARRAY_SIZE(region_param)

#ifndef CI_PLUS_PVR_SUPPORT
//#define TIME_DISPLAY
/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION time_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT time_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION time_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT time_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION win_time_edit_keymap(POBJECT_HEAD pobj, UINT32 key);

#ifndef SD_UI

#define MTXT_SH_IDX WSTL_TEXT_09_HD
#define MTXT_L  (W_L + 30)
#ifdef TIME_DISPLAY
#define MTXT_T ( CON_T + (CON_H + CON_GAP)*7 + 8)
#else
#define MTXT_T ( CON_T + (CON_H + CON_GAP)*6 + 8)
#endif
#define MTXT_W (W_W - 60)
#define MTXT_H 150

#define TIME_CON_GAP CON_GAP

#define MENU_TIMESET_GMT_OFFESET_POPUP_H    300
#else
#define SD_CON_GAP 4

#define TIME_CON_GAP SD_CON_GAP

#define MTXT_SH_IDX WSTL_TEXT_09_HD
#define MTXT_L  (W_L + 30)
#ifdef TIME_DISPLAY
#define MTXT_T ( CON_T + (CON_H + SD_CON_GAP)*7 + 5)
#else
#define MTXT_T ( CON_T + (CON_H + SD_CON_GAP)*6 + 5)
#endif
#define MTXT_W (W_W - 60)
#define MTXT_H 150

#define MENU_TIMESET_GMT_OFFESET_POPUP_H    300
#endif
#ifdef SHOW_WELCOME_FIRST
#define COUNTRY_NCNT    (sizeof(country_strs2) / sizeof(country_strs2[0]))
#endif
#define isleap(y) ((0 == ((y) % 4) && ((y) % 100) != 0) || 0 == ((y) % 400))
#if ((!defined(CI_PLUS_SUPPORT)) && (!defined(SUPPORT_CAS9))\
    &&(!defined(_NV_PROJECT_SUPPORT_))&&(!defined(_GEN_CA_ENABLE_))\
    && (!defined(REMOVE_SPECIAL_MENU)))
#define BOTTOM_O_GMT_LIST 2
#else
#define BOTTOM_O_GMT_LIST 1
#endif
#define JAN 1
#define FEB 2
#define MAR 3
#define APR 4
#define MAY 5
#define JUNE 6
#define JULY 7
#define AUG 8
#define SEP 9
#define OCT 10
#define NOV 11
#define DEC 12
#define VALID_YEAR_MAX 2099
#define VALID_YEAR_MIN 1901
#define DAYS31_A_MONTH 31
#define DAYS30_A_MONTH 30
#define DAYS29_A_MONTH 29
#define DAYS28_A_MONTH 28

static char win_time_pat_date[] = "d02";
static char win_time_pat_time[] = "t1";

#ifdef SHOW_WELCOME_FIRST
#ifdef DVBC_SUPPORT
static UINT16 country_strs2[] = /* Should have the same order as stream_iso_639lang_abbr*/
{
    RS_REGION_GER,
    RS_REGION_AUS,
    RS_COUNTRY_SWITZERLAND,
    RS_REGION_ITA,
    RS_COUNTRY_GREECE,
    RS_COUNTRY_PORTUGAL,
};
#else
static UINT16 country_strs2[] = /* Should have the same order as stream_iso_639lang_abbr*/
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
#endif

#ifdef DVBC_SUPPORT
UINT8 gmt_offset_value_by_region[COUNTRY_NCNT]={17,43,39,25,27,25};
#else
#ifdef PERSIAN_SUPPORT
UINT8 gmt_offset_value_by_region[COUNTRY_NCNT]={17,43,17,39,25,27,25,25,39,25,25,29,39,25,25,39,23,30};
#else
UINT8 gmt_offset_value_by_region[COUNTRY_NCNT]={17,43,17,39,25,27,25,25,39,25,25,29,39,25,25,39,23};
#endif
#endif
#endif
//modify end

#define LDEF_CON(root, var_con,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    time_item_con_keymap,time_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)

#define LDEF_EDIT(root, var_num, nxt_obj, id, idl, idr, idu, idd, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,WSTL_BUTTON_05_HD,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    win_time_edit_keymap,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,id,idu,idd,l,t,w,h,name_id,setstr)  \
    LDEF_CON(&root,var_con,nxt_obj,id,id,id,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_TXTSET(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MM_ITEM2(root,var_con,nxt_obj,var_txt_name,var_txtset,var_line,id,idu,idd,l,t,w,h,name_id,setstr,pat) \
    LDEF_CON(&root,var_con,nxt_obj,id,id,id,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_EDIT(&var_con,var_txtset,NULL/*&varLine*/,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,\
                    NORMAL_EDIT_MODE , CURSOR_NORMAL, pat,NULL,NULL, setstr)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MTXT(root,var_mtxt,next_obj,l,t,w,h,cnt,content) \
    DEF_MULTITEXT(var_mtxt,&root,next_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, MTXT_SH_IDX,MTXT_SH_IDX,MTXT_SH_IDX,MTXT_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,NULL,content)

#ifdef TIME_DISPLAY
LDEF_MM_ITEM(win_time_con,time_item_con1, &time_item_con2,time_item_txtname1,time_item_txtset1,time_item_line1,\
                1,7,2,CON_L, CON_T + (CON_H + TIME_CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_TIME_REGION_NAME,NULL)
#else
LDEF_MM_ITEM(win_time_con,time_item_con1, &time_item_con2,time_item_txtname1,time_item_txtset1,time_item_line1,\
                1,6,2, CON_L, CON_T + (CON_H + TIME_CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_TIME_REGION_NAME,NULL)
#endif
LDEF_MM_ITEM(win_time_con,time_item_con2, &time_item_con3,time_item_txtname2,  time_item_txtset2,time_item_line2,\
                2,1,3,CON_L, CON_T + (CON_H + TIME_CON_GAP)*1,CON_W,CON_H, RS_SYSTEM_TIME_GMT_USAGE,NULL)
LDEF_MM_ITEM(win_time_con,time_item_con3, &time_item_con5,time_item_txtname3,time_item_txtset3,time_item_line3,\
                3,2,5,CON_L, CON_T + (CON_H + TIME_CON_GAP)*2,CON_W,CON_H, RS_SYSTEM_TIME_GMT_OFFSET,display_strs[0])
LDEF_MM_ITEM2(win_time_con,time_item_con5, &time_item_con6,time_item_txtname5,time_item_edit5,time_item_line5,\
                5,3,6,CON_L, CON_T + (CON_H + TIME_CON_GAP)*3,CON_W,CON_H, RS_SYSTEM_TIME_DATE,display_strs[1],\
                win_time_pat_date)

#ifdef TIME_DISPLAY
LDEF_MM_ITEM2(win_time_con,time_item_con6, &time_item_con7,time_item_txtname6,time_item_edit6,time_item_line6,\
                6,5,7,CON_L, CON_T + (CON_H + TIME_CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_TIME,\
                display_strs[2],win_time_pat_time)
LDEF_MM_ITEM(win_time_con,time_item_con7, &time_mtxt_zone_city,time_item_txtname7,time_item_txtset7,time_item_line7,\
                7,6,1,CON_L, CON_T + (CON_H + TIME_CON_GAP)*5,CON_W,CON_H, RS_SYSTEM_TIME_DISPLAY,NULL)
#else
LDEF_MM_ITEM2(win_time_con,time_item_con6, &time_mtxt_zone_city,time_item_txtname6,time_item_edit6,time_item_line6,\
                6,5,1, CON_L, CON_T + (CON_H + TIME_CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_TIME,display_strs[2],\
                win_time_pat_time)
LDEF_MM_ITEM(win_time_con,time_item_con7, &time_mtxt_zone_city,time_item_txtname7,time_item_txtset7,time_item_line7,\
                7,6,1,CON_L, CON_T + (CON_H + TIME_CON_GAP)*5,CON_W,CON_H, RS_SYSTEM_TIME_DISPLAY,NULL)
#endif

LDEF_MTXT(win_time_con,time_mtxt_zone_city,NULL,\
        MTXT_L, MTXT_T,MTXT_W,MTXT_H,1,timezone_mtxt_content)

DEF_CONTAINER(win_time_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    time_con_keymap,time_con_callback,  \
    (POBJECT_HEAD)&time_item_con1, 1,0)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static date_time win_time_local_time;

TEXT_CONTENT timezone_mtxt_content[] =
{
    {STRING_UNICODE,{0}},
};

static UINT16 gmt_usage_strs[] =
{
    RS_SYSTEM_TIME_GMT_BYREGION,
    RS_SYSTEM_TIME_GMT_USER,
#if ((!defined(CI_PLUS_SUPPORT)) && (!defined(SUPPORT_CAS9)) \
    && (!defined(_NV_PROJECT_SUPPORT_)) &&(!defined(_GEN_CA_ENABLE_)) \
    && (!defined(REMOVE_SPECIAL_MENU)))
    RS_COMMON_OFF,
#endif
};

static void win_time_set_region_string(UINT8 input)
{
    osd_set_text_field_content(&time_item_txtset1, STRING_ID, region_param[input].region_str);
}

static void win_time_set_gmt_enable_string(UINT8 input)
{

    osd_set_text_field_content(&time_item_txtset2, STRING_ID,gmt_usage_strs[input] );
}
static void win_time_set_display_string(UINT8 input)
{
    if(0 == input)
    {
        osd_set_text_field_content(&time_item_txtset7, STRING_ID,RS_COMMON_OFF);
    }
    else
    {
        osd_set_text_field_content(&time_item_txtset7, STRING_ID,RS_COMMON_ON);
    }
}

static void win_time_switch_gmt_type(UINT8 enable)
{
    switch(enable)
    {
        case 0:
            if( !osd_check_attr(&time_item_con3, C_ATTR_INACTIVE))
            {
                osd_set_attr(&time_item_con3, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtname3, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtset3, C_ATTR_INACTIVE);
            }
            if( !osd_check_attr(&time_item_con5, C_ATTR_INACTIVE))
            {
                osd_set_attr(&time_item_con5, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtname5, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_edit5, C_ATTR_INACTIVE);
            }
            if( !osd_check_attr(&time_item_con6, C_ATTR_INACTIVE))
            {
                osd_set_attr(&time_item_con6, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtname6, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_edit6, C_ATTR_INACTIVE);
            }
            break;
        case 1:
            if( !osd_check_attr(&time_item_con3, C_ATTR_ACTIVE))
            {
                osd_set_attr(&time_item_con3, C_ATTR_ACTIVE);
                osd_set_attr(&time_item_txtname3, C_ATTR_ACTIVE);
                osd_set_attr(&time_item_txtset3, C_ATTR_ACTIVE);
            }
            if( !osd_check_attr(&time_item_con5, C_ATTR_INACTIVE))
            {
                osd_set_attr(&time_item_con5, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtname5, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_edit5, C_ATTR_INACTIVE);
            }
            if( !osd_check_attr(&time_item_con6, C_ATTR_INACTIVE))
            {
                osd_set_attr(&time_item_con6, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtname6, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_edit6, C_ATTR_INACTIVE);
            }
            break;
        default:
            if( !osd_check_attr(&time_item_con3, C_ATTR_INACTIVE))
            {
                osd_set_attr(&time_item_con3, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtname3, C_ATTR_INACTIVE);
                osd_set_attr(&time_item_txtset3, C_ATTR_INACTIVE);
            }
            if( !osd_check_attr(&time_item_con5, C_ATTR_ACTIVE))
            {
                osd_set_attr(&time_item_con5, C_ATTR_ACTIVE);
                osd_set_attr(&time_item_txtname5, C_ATTR_ACTIVE);
                osd_set_attr(&time_item_edit5, C_ATTR_ACTIVE);
            }
            if( !osd_check_attr(&time_item_con6, C_ATTR_ACTIVE))
            {
                osd_set_attr(&time_item_con6, C_ATTR_ACTIVE);
                osd_set_attr(&time_item_txtname6, C_ATTR_ACTIVE);
                osd_set_attr(&time_item_edit6, C_ATTR_ACTIVE);
            }
            break;
    }
}

static void win_time_set_gmt_offset_string(UINT8 input)
{
    UINT8 hour= 0;
    UINT8 minute= 0;
    UINT8 offset= 0;
    UINT8 temp[15] = {0};
    INT32 ret = 0;

    if(input>=(HOURS_IN_ONEDAY-1))
    {
        offset=input-(HOURS_IN_ONEDAY-1);
        hour=offset/2;
        minute=offset%2 * 30;
        ret = snprintf((char *)temp, 15, "UTC + %02d:%02d", hour, minute);
        if(0 == ret)
        {
            ali_trace(&ret);
        }
    }
    else
    {
        offset=(HOURS_IN_ONEDAY-1)-input;
        hour=offset/2;
        minute=offset%2 * 30;
        ret = snprintf((char *)temp, 15, "UTC - %02d:%02d", hour, minute);
        if(0 == ret)
        {
            ali_trace(&ret);
    }

    }
    com_asc_str2uni(temp, display_strs[0]);
}

static void win_time_set_time_zone_citys(BOOL update)
{
    MULTI_TEXT *mtxt = &time_mtxt_zone_city;
    SYSTEM_DATA *p_sys_data=sys_data_get();

    TEXT_CONTENT *txtcont = timezone_mtxt_content;

    struct time_zone_name *time_zone = NULL;
    INT32 i= 0;
    INT32 hoffset= 0;
    INT32 moffset= 0;
    INT32 str_len= 0;

    txtcont->b_text_type = STRING_UNICODE;
    txtcont->text.p_string = len_display_str;

    if(BOTTOM_O_GMT_LIST == p_sys_data->gmt_select)
    {
        com_asc_str2uni((UINT8 *)"", len_display_str);
    }
    else
    {
        sys_data_gmtoffset_2_hmoffset(&hoffset,&moffset);
        time_zone = get_time_zone_name(hoffset, moffset);
        if(NULL == time_zone)
        {
            com_asc_str2uni((UINT8 *)"", len_display_str);
        }
        else
        {
            str_len = 0;
            for(i=0;i<time_zone->namecnt;i++)
            {
                if(i!=0)
                {
                    str_len = com_uni_str_len(len_display_str);
                    set_uni_str_char_at(len_display_str,',', str_len);
                    str_len++;
                }
                com_asc_str2uni((UINT8 *)time_zone->city_name[i], &len_display_str[str_len]);
            }
        }
    }

    if(update)
    {
        osd_draw_object((POBJECT_HEAD)mtxt, C_UPDATE_ALL);
    }
}

static void win_time_get_input(void)
{
    osd_get_edit_field_time_date(&time_item_edit5,&win_time_local_time);
    osd_get_edit_field_time_date(&time_item_edit6,&win_time_local_time);
}

UINT8 check_is_valid_mon_day_year(void)
{
    UINT8 mm=0;
    UINT8 is_leap_year=0;
    UINT8 dd=0;
    UINT16 year=0;
    UINT8 ret = 0;

    year=win_time_local_time.year;
    mm=win_time_local_time.month;
    dd=win_time_local_time.day;

    is_leap_year = isleap(year);

    if((year<VALID_YEAR_MIN) || (year>VALID_YEAR_MAX))
    {
        ret = 0;
    }
    if((mm<JAN) || (mm>DEC))
    {
        ret = 0;
    }
    if(((JAN == mm )||(MAR ==mm )||(MAY ==mm )||(JULY == mm)||(AUG == mm )||(OCT == mm )||(DEC ==mm ))
        && ((dd<1)|| (dd>DAYS31_A_MONTH)))
    {
        ret = 0;
    }
    if(((APR == mm )||(JUNE ==mm )||(SEP == mm )||(NOV == mm )) && ((dd<1)|| (dd>DAYS30_A_MONTH)))
    {
        ret = 0;
    }
    if((1 == is_leap_year ) &&(FEB== mm ) && ((dd<1)|| (dd>DAYS29_A_MONTH)))
    {
        ret = 0;
    }
    if((0 == is_leap_year ) &&(FEB == mm ) && ((dd<1)|| (dd>DAYS28_A_MONTH)))
    {
        ret = 0;
    }
    ret = 1;
    return ret;
}


UINT8 check_is_valid_hh_mm(void)
{
    UINT8 hh = 0;
    UINT8 mm = 0;
    UINT8 ret =0;

    hh=win_time_local_time.hour;
    mm=win_time_local_time.min;
    if((mm>=MINUTES_IN_ONEHOUR) || (hh>(HOURS_IN_ONEDAY-1)))
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }
    return ret;
}

static void set_time_data(void)
{
    UINT32 dwvalue = 0;

    dwvalue =win_time_local_time.year*10000+win_time_local_time.month*100+win_time_local_time.day;
    osd_set_edit_field_content(&time_item_edit5, STRING_NUMBER, dwvalue);
}

static void set_time_clock(void)
{
    UINT32 dwvalue = 0;

    dwvalue = win_time_local_time.hour*100+win_time_local_time.min;
    osd_set_edit_field_content(&time_item_edit6, STRING_NUMBER, dwvalue);
}

static VACTION win_time_edit_keymap(POBJECT_HEAD pobj, UINT32 key)
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
        case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
        case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
        case V_KEY_8:   case V_KEY_9:
            action = key - V_KEY_0 + VACT_NUM_0;
            break;
        default:
            break;
    }

    return action;
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION time_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act =VACT_PASS;

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

UINT8 win_time_get_num(void)
{
    UINT8 id=osd_get_focus_id((POBJECT_HEAD)&win_time_con);
    UINT8 num=0;

    switch(id)
    {
        case MENU_TIMESET_REGION_SELECT:
            num = REGION_CNT;
            break;
        case MENU_TIMESET_GMT_ENABLE:
            num = ARRAY_SIZE(gmt_usage_strs);
            break;
        case MENU_TIMESET_GMT_OFFESET:
            num=48;
            break;
        default:
            break;
    }
    return num;
}

PRESULT comlist_menu_time_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT8 id= 0;
    OBJLIST *ol = NULL;
    UINT8 temp[15]={0};
    UINT8 hour= 0;
    UINT8 minute= 0;
    UINT8 offset= 0;
    PRESULT cbret = PROC_PASS;
    INT32 ret = 0;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        id=osd_get_focus_id((POBJECT_HEAD)&win_time_con);
        switch(id)
        {
            case MENU_TIMESET_REGION_SELECT:
                wtop = osd_get_obj_list_top(ol);
                for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                {
                    win_comlist_set_str(i + wtop,NULL, NULL, region_param[i + wtop].region_str );
                }

                break;
            case MENU_TIMESET_GMT_ENABLE:
                wtop = osd_get_obj_list_top(ol);
                for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                {
                    win_comlist_set_str(i + wtop,NULL, NULL,gmt_usage_strs[i + wtop]);
                }
                break;
            case MENU_TIMESET_GMT_OFFESET:
                wtop = osd_get_obj_list_top(ol);
                for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
                {
                    if(i+wtop>=(HOURS_IN_ONEDAY-1))
                    {
                        offset= (i+wtop) - (HOURS_IN_ONEDAY-1);
                        hour=offset/2;
                        minute=offset%2 * 30;
                        ret = snprintf((char *)temp, 15, "UTC + %02d:%02d", hour, minute);
                        if(0 == ret)
                        {
                            ali_trace(&ret);
                        }
                    }
                    else
                    {
                        offset=(HOURS_IN_ONEDAY-1)-(i+wtop);
                        hour=offset/2;
                        minute=offset%2 * 30;
                        ret = snprintf((char *)temp, 15, "UTC - %02d:%02d", hour, minute);
                        if(0 == ret)
                        {
                            ali_trace(&ret);
                        }
                    }
                    win_comlist_set_str(i + wtop,(char *)temp,NULL,0);
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

/*
 * select: region index
 * region select change will effect GMT Offset only if GMT Usage selected BY REGION
 */
static void _region_select_status( POBJECT_HEAD pobj, UINT8 select )
{
    INT32 hoffset= 0;
    INT32 moffset= 0;
    SYSTEM_DATA *p_sys_data=sys_data_get();
    UINT32 cntry_code = 0xFFFFFF;
    REGION_PARAM_T *p_region = NULL;

    p_sys_data->country= select;
    win_time_set_region_string( select );
    if(0 == p_sys_data->gmt_select) // by region
    {
        p_region = &region_param[ p_sys_data->country];
        p_sys_data->local_time.gmtoffset_count = p_region->timezone;
        win_time_set_gmt_offset_string(p_sys_data->local_time.gmtoffset_count);
        cntry_code = p_region->country_code[0]<<16 | p_region->country_code[1]<<8 | p_region->country_code[2];
        tdt_set_country_code( cntry_code );
        osd_track_object(pobj,C_UPDATE_ALL);

        sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
        set_stc_offset( hoffset, moffset, 0);
        get_local_time(&win_time_local_time);
        set_time_data();
        set_time_clock();
        osd_draw_object((POBJECT_HEAD )&time_item_con3,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&time_item_con5,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&time_item_con6,C_UPDATE_ALL); 
    }
    if(1 == p_sys_data->gmt_select ) // user define
    {

        p_region = &region_param[ p_sys_data->country];
        p_sys_data->local_time.gmtoffset_count = p_region->timezone;
        win_time_set_gmt_offset_string(p_sys_data->local_time.gmtoffset_count);
        cntry_code = p_region->country_code[0]<<16 | p_region->country_code[1]<<8 | p_region->country_code[2];
        tdt_set_country_code( cntry_code );
        osd_track_object(pobj,C_UPDATE_ALL);
       
        sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
        set_stc_offset( hoffset, moffset, 0);
        get_local_time(&win_time_local_time);
        set_time_data();
        set_time_clock();
        osd_draw_object((POBJECT_HEAD )&time_item_con3,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&time_item_con5,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&time_item_con6,C_UPDATE_ALL);
    }
    osd_track_object(pobj,C_UPDATE_ALL);
    win_time_set_time_zone_citys(TRUE);

}

/*
 * select: 0, BY REGION; 1, USER DEFINE; 2, GMT OFF
 *       BY REGION  USER DEFINE  GMT OFF
 * --- |----------------------------------
 * TDT |    sync       sync        xx
 * TOT |    sync        xx         xx
 * above table, sync means time should sync tdt/tot, xx means do not sync tdt/tot
 */
static void _gmt_usage_status( POBJECT_HEAD pobj, UINT8 select )
{
    INT32 hoffset= 0;
    INT32 moffset= 0;
    SYSTEM_DATA *p_sys_data=sys_data_get();
    REGION_PARAM_T *p_region = NULL;

    p_sys_data->gmt_select = select;
#if ((!defined(CI_PLUS_SUPPORT)) && (!defined(SUPPORT_CAS9)))
    if( BOTTOM_O_GMT_LIST == p_sys_data->gmt_select  )
    {
        p_sys_data->local_time.buse_gmt = 0;
    }
    else
    {
        p_sys_data->local_time.buse_gmt = 1;
    }
#else
    p_sys_data->local_time.buse_gmt = 1;
#endif
    win_time_set_gmt_enable_string( select );
    osd_track_object(pobj,C_UPDATE_ALL);
    win_time_switch_gmt_type( select );

    if( 0  ==p_sys_data->gmt_select) // by region
    {
        p_region = &region_param[ p_sys_data->country];
        p_sys_data->local_time.gmtoffset_count = p_region->timezone;
        win_time_set_gmt_offset_string(p_sys_data->local_time.gmtoffset_count);
        osd_track_object(pobj,C_UPDATE_ALL);

        enable_time_offset_parse( );
        sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
        set_stc_offset( hoffset, moffset, 0);
        get_local_time(&win_time_local_time);
        set_time_data();
        set_time_clock();
        osd_draw_object((POBJECT_HEAD )&time_item_con3,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&time_item_con5,C_UPDATE_ALL);
        osd_draw_object((POBJECT_HEAD )&time_item_con6,C_UPDATE_ALL);
    }
    else
    {
        disable_time_offset_parse();
    }

    if(p_sys_data->local_time.buse_gmt)
    {
        enable_time_parse();
        get_utc(&win_time_local_time);
        set_stc_time(&win_time_local_time);
        set_time_data();
        set_time_clock();
    }
    else
    {
        disable_time_parse();
        p_sys_data->local_time.gmtoffset_count = (HOURS_IN_ONEDAY-1);
        win_time_set_gmt_offset_string(p_sys_data->local_time.gmtoffset_count);
    }
    osd_draw_object((POBJECT_HEAD )&time_item_con3,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&time_item_con5,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&time_item_con6,C_UPDATE_ALL);
    win_time_set_time_zone_citys(TRUE);
}

/*
 * offset_count: the result of TIME_ZONE( h, m )
 */
static void _gmt_offset_status( POBJECT_HEAD pobj, UINT8 offset_count )
{
    INT32 hoffset= 0;
    INT32 moffset= 0;
    SYSTEM_DATA *p_sys_data=sys_data_get();

    p_sys_data->local_time.gmtoffset_count = offset_count;
    win_time_set_gmt_offset_string(offset_count);
    osd_track_object(pobj,C_UPDATE_ALL);

    sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
    set_stc_offset( hoffset, moffset, 0);
    get_local_time(&win_time_local_time);
    set_time_data();
    set_time_clock();
    osd_draw_object((POBJECT_HEAD )&time_item_con5,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&time_item_con6,C_UPDATE_ALL);

    win_time_set_time_zone_citys(TRUE);
}

static void win_time_enter_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT8 cur_select= 0;

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&param, 0 ,sizeof(param));
    rect.u_left=CON_L+TXTS_L_OF;
    rect.u_width=TXTS_W;
    rect.u_height=MENU_TIMESET_GMT_OFFESET_POPUP_H;
    param.selecttype = POP_LIST_SINGLESELECT;
    switch(id)
    {
        case MENU_TIMESET_REGION_SELECT:
            rect.u_height= 300;
            rect.u_top=CON_T + (CON_H + TIME_CON_GAP)*0;
            param.cur=p_sys_data->country;
            cur_select = win_com_open_sub_list(POP_LIST_MENU_TIME,&rect,&param);
            _region_select_status( pobj, cur_select );

            break;

        case MENU_TIMESET_GMT_ENABLE:
            rect.u_height= 200;
            rect.u_top=CON_T + (CON_H + TIME_CON_GAP)*1;
            param.cur=p_sys_data->gmt_select;

            cur_select = win_com_open_sub_list(POP_LIST_MENU_TIME,&rect,&param);
            _gmt_usage_status( pobj, cur_select );

            break;
        case MENU_TIMESET_GMT_OFFESET:
            rect.u_height = MENU_TIMESET_GMT_OFFESET_POPUP_H;
            rect.u_top=CON_T + (CON_H + TIME_CON_GAP)*2;
            param.cur=p_sys_data->local_time.gmtoffset_count;
            cur_select = win_com_open_sub_list(POP_LIST_MENU_TIME,&rect,&param);
            _gmt_offset_status( pobj, cur_select );

            break;
        default:
            break;
    }
}

static void win_time_left_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_TIMESET_REGION_SELECT:
            if( 0 ==  p_sys_data->country )
            {
                p_sys_data->country = REGION_CNT-1;
            }
            else
            {
                p_sys_data->country--;
            }
            _region_select_status( pobj, p_sys_data->country );
            break;
        case MENU_TIMESET_GMT_ENABLE:
            if( 0 == p_sys_data->gmt_select  )
            {
                p_sys_data->gmt_select = BOTTOM_O_GMT_LIST;
            }
            else
            {
                p_sys_data->gmt_select--;
            }
            _gmt_usage_status( pobj, p_sys_data->gmt_select );

            break;
        case MENU_TIMESET_DISPLAY:
            p_sys_data->local_time.time_display=1-p_sys_data->local_time.time_display;
            win_time_set_display_string(p_sys_data->local_time.time_display);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_TIMESET_GMT_OFFESET:
            if(0 == p_sys_data->local_time.gmtoffset_count)
            {
                p_sys_data->local_time.gmtoffset_count=47;
            }
            else
            {
                p_sys_data->local_time.gmtoffset_count--;
            }
            _gmt_offset_status( pobj, p_sys_data->local_time.gmtoffset_count );

            break;
        default:
            break;
    }
}

static void win_time_right_key(POBJECT_HEAD pobj,UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();

    switch(id)
    {
        case MENU_TIMESET_REGION_SELECT:
            if( (REGION_CNT-1) == p_sys_data->country)
            {
                p_sys_data->country = 0;
            }
            else
            {
                p_sys_data->country++;
            }
            _region_select_status( pobj, p_sys_data->country );
            break;
        case MENU_TIMESET_GMT_ENABLE:
            if( BOTTOM_O_GMT_LIST == p_sys_data->gmt_select  )
            {
                p_sys_data->gmt_select = 0;
            }
            else
            {
                p_sys_data->gmt_select++;
            }
            _gmt_usage_status( pobj, p_sys_data->gmt_select );

            break;
        case MENU_TIMESET_DISPLAY:
            p_sys_data->local_time.time_display=1-p_sys_data->local_time.time_display;
            win_time_set_display_string(p_sys_data->local_time.time_display);
            osd_track_object(pobj,C_UPDATE_ALL);
            break;
        case MENU_TIMESET_GMT_OFFESET:
            if(47 == p_sys_data->local_time.gmtoffset_count)
            {
                p_sys_data->local_time.gmtoffset_count=0;
            }
            else
            {
                p_sys_data->local_time.gmtoffset_count++;
            }
            _gmt_offset_status( pobj, p_sys_data->local_time.gmtoffset_count );

            break;
        default:
            break;
    }
}

static PRESULT time_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=VACT_PASS;
    UINT8   id = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>NON_ACTION_LENGTH) ;
        switch(unact)
        {
            case VACT_ENTER:
                win_time_enter_key(pobj,id);
                break;
            case VACT_DECREASE:
                win_time_left_key(pobj,id);
                break;
            case VACT_INCREASE:
                win_time_right_key(pobj,id);
                break;
            default:
                break;
        }
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION time_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=VACT_PASS;
    SYSTEM_DATA *p_sys_data=sys_data_get();
    date_time dt;
    INT32 hoffset= 0;
    INT32 moffset= 0;
    UINT8 save= 0;
    INT32 ret1= 0;

    MEMSET(&dt, 0, sizeof(dt));
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
        win_time_get_input();
        if(p_sys_data->local_time.buse_gmt)
        {
            sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
            set_stc_offset( hoffset, moffset, 0);
            get_utc(&dt);
            set_stc_time(&dt);
            enable_time_parse();
        }
        else
        {
            ret1 = api_check_valid_date(win_time_local_time.year,win_time_local_time.month,win_time_local_time.day);
            if((ret1 != 0) || (win_time_local_time.hour>(HOURS_IN_ONEDAY-1))
                || (win_time_local_time.min>=MINUTES_IN_ONEHOUR))
            {
                win_compopup_init(WIN_POPUP_TYPE_OKNO);
                win_compopup_set_msg(NULL,NULL,RS_MSG_INVALID_INPUT_CONTINUE);
                win_compopup_set_frame(GET_MID_L(300),GET_MID_T(100),320,200);
                if(WIN_POP_CHOICE_YES == win_compopup_open_ext(&save))
                {
                    act = VACT_PASS;
                }
                else
                {
                    act = VACT_CLOSE;
                }
            }
            else
            {
                set_stc_offset(0,0,0);
                set_stc_time(&win_time_local_time);
                p_sys_data->local_time.gmtoffset_count = (HOURS_IN_ONEDAY-1);
                disable_time_parse();    
            }
        }

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

static PRESULT time_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    SYSTEM_DATA *p_sys_data = NULL;

    p_sys_data=sys_data_get();

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&win_time_con,RS_SYS_LOCAL_TIMER_SET/*RS_SYSTEM_TIME*/, 0);
        if( p_sys_data->gmt_select> BOTTOM_O_GMT_LIST )
        {
            p_sys_data->gmt_select = p_sys_data->local_time.buse_gmt;
        }
        win_time_set_region_string( p_sys_data->country);
        win_time_set_gmt_enable_string( p_sys_data->gmt_select );
        win_time_set_display_string(p_sys_data->local_time.time_display);
        win_time_set_gmt_offset_string(p_sys_data->local_time.gmtoffset_count);
        get_local_time(&win_time_local_time);
        set_time_data();
        set_time_clock();
        win_time_switch_gmt_type(p_sys_data->gmt_select);
        win_time_set_time_zone_citys(FALSE);
        osd_set_container_focus((CONTAINER*)pobj,1);
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
        osd_get_edit_field_time_date(&time_item_edit5, &(p_sys_data->sys_dt));
        if(p_sys_data->gmt_select == 2)//should set hh:mm in this mode
        {   
            osd_get_edit_field_time_date(&time_item_edit6, &(p_sys_data->sys_dt));
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
#endif//#ifndef CI_PLUS_PVR_SUPPORT

UINT8 *win_time_get_country_code( UINT8 n )
{
    UINT8 *ret =0;

    if( n >= sizeof(region_param)/sizeof(region_param[0]) )
    {
        ret = NULL;//0;
    }
    else
    {
        ret = region_param[n].country_code;
    }
    return ret;
}

