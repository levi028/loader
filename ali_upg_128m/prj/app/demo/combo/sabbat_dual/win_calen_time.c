
/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_calen_time.c
*
*    Description: To add calender for UI
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#ifdef SUPPORT_IRAN_CALENDAR
#include <types.h>

#include <osal/osal.h>
#include <hld/pan/pan_dev.h>

#include <api/libosd/osd_lib.h>

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osdobjs_def.h"
#include "osd_rsc.h"
#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"

#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_com.h"
#include "control.h"
#include "win_game_com.h"
#include "win_calen_time.h"
#include "menus_root.h"

#define PERSIAN_CALENDER

#define WIN_CALEN_LEFT       17
#define WIN_CALEN_TOP        20
#define WIN_CALEN_WIDTH      960 //576
#define WIN_CALEN_HEIGHT     600 //400

#define MONTH_CON_L  WIN_CALEN_LEFT+38
#define MONTH_CON_T  WIN_CALEN_TOP+30
#define MONTH_CON_W  580  // 350
#define MONTH_CON_H  45  // 30

#undef TXTN_L_OF
#undef TXTN_W
#undef TXTN_H
#undef TXTN_T_OF
#undef TXTS_L_OF
#undef TXTS_W
#undef TXTS_H
#undef TXTS_T_OF

#define TXTN_L_OF   4
#define TXTN_W          150
#define TXTN_H      45 //30
#define TXTN_T_OF   ((42 - TXTN_H)/2)
#define TXTS_L_OF   (TXTN_L_OF + TXTN_W)
#define TXTS_W          200
#define TXTS_H      32 //45
#define TXTS_T_OF   ((42 - TXTS_H)/2)

// calender frame size define
#define CALEN_MAINFRAME_L           90 //55
#define CALEN_MAINFRAME_T           135 //90//(110 - 18)
#define CALEN_MAINFRAME_W           800//500
#define CALEN_MAINFRAME_H           370 //250//290

#define CALEN_DATE_ITEM_W    100 //65//48//72
#define CALEN_DATE_ITEM_H   52 //35//40//60
#define CALEN_DATE_ITEM_X    16
#define CALEN_DATE_ITEM_X_BORDER   5

#define CALEN_YEAR_H        64 //40
#define CALEN_WEEKLY_H  45 //30
#define CALEN_HEAD_H    (CALEN_YEAR_H+CALEN_WEEKLY_H)
#define CALEN_BODY_H    (CALEN_DATE_ITEM_H*6 + 2*CALEN_DATE_ITEM_X_BORDER)
#define CALEN_X_OFFSET      16

#define FOCUS_REC_X_OFFSEF 4
#define FOCUS_REC_Y_OFFSET 4

#define HELP_BMP_L 80
#define HELP_BMP_T 540
#define HELP_BMP_W  32 //20
#define HELP_BMP_H  30 //20

#define HELP_TXT_L (HELP_BMP1_L + HELP_BMP1_W + 5)
#define HELP_TXT_T HELP_BMP_T
#define HELP_TXT_W  150
#define HELP_TXT_H   32

#define MARK_SH_IDX                 WSTL_MIXBACK_BLACK_IDX

#define WSTL_TEXT_02                WSTL_BUTTON_01
#define WSTL_GAME_BUTTON_01     WSTL_BUTTON_01
#define WSTL_GAME_BUTTON_02     WSTL_BUTTON_05_8BIT
#define WSTL_GAME_BUTTON_03     WSTL_BUTTON_04_HD
#define WSTL_GAME_TEXT_01       WSTL_BUTTON_01_HD
#define WSTL_GAME_BG_02         WSTL_GAMEWIN_01_HD

#define TXT_SH_IDX   WSTL_TEXT_04_8BIT
#define TXT_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_02_FG
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define CON_EDIT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define CON_EDIT_HL_IDX   WSTL_BUTTON_07_HD
#define CON_EDIT_SL_IDX   WSTL_BUTTON_04_HD
#define CON_EDIT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD
#define YEAR_TXT_SH_IDX    WSTL_KEYBOARD_01_8BIT

#define DAYS_A_WEEK    7

CONTAINER           win_calender_con;

#ifdef YMD_FORMAT_DDMMYY
char win_time_date[] = "d42";
#else
char win_time_date[] = "d02";
#endif
char win_time_time[] = "t1";

static VACTION calen_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT calen_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION calen_item_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT calen_item_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static void update_date();
#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,3, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_SH_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    calen_item_con_keymap,calen_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,3, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_SH_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, idl, idr, idu, idd, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,3, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,WSTL_BUTTON_05_HD,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    calen_item_sel_keymap,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MM_ITEM2(root,var_con,nxt_obj,var_txt_name,var_txtset,ID,idu,idd,l,t,w,h,name_id,setstr,pat) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l+TXTN_L_OF+46, t+TXTN_T_OF+41,TXTN_W,TXTN_H,name_id,NULL)\
    LDEF_EDIT(&var_con,var_txtset,NULL,1,2,2,1,1,l + TXTS_L_OF+155, t+5,TXTS_W,TXTS_H,NORMAL_EDIT_MODE , CURSOR_NORMAL,\
                        pat,NULL,NULL, setstr)

LDEF_MM_ITEM2(win_calender_con,month_item_con, &calen_title,month_item_txtname,year_item_txtset,2,2,2,  \
                MONTH_CON_L, MONTH_CON_T, MONTH_CON_W, MONTH_CON_H, RS_HELP_FIND,display_strs[1],win_time_date)

BITMAP win_calen_bmp1;
BITMAP win_calen_bmp2;
BITMAP win_calen_bmp3;
BITMAP win_calen_bmp4;

TEXT_FIELD  win_calen_txt1;
TEXT_FIELD  win_calen_txt2;
TEXT_FIELD  win_calen_txt3;
TEXT_FIELD  win_calen_txt4;

#define MAX_TITLE_NAME_LENGTH        27
UINT16 calen_title_str[MAX_TITLE_NAME_LENGTH];

static VACTION calen_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT calen_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,3, \
    0,0,0,0,0, l,t,w,h, WSTL_GAME_BG_02,WSTL_GAME_BG_02,WSTL_GAME_BG_02,WSTL_GAME_BG_02,   \
    calen_con_keymap,calen_con_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,3, \
    4,3,0,2,5, l,t,w,h, WSTL_BUTTON_05_HD,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,shidx,str_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,3, \
        0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,str_id,str)

#define LDEF_BMP(root,varbmp,nxt_obj,l,t,w,h,bmp_id)      \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,3, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_SH_IDX,MARK_SH_IDX,MARK_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_BMP_TXT(root,var_bmp,var_txt,nxt_obj,bmp_l,bmp_t,bmp_w,bmp_h,txt_t,txt_w,txt_h,shidx,str_id,bmp_id,num) \
    LDEF_BMP(&root,var_bmp,&var_txt,bmp_l+(bmp_w+txt_w+18)*num,bmp_t,bmp_w,bmp_h,bmp_id) \
    LDEF_TXT(&root,var_txt,&nxt_obj,bmp_l+bmp_w+(bmp_w+txt_w+18)*num,txt_t,txt_w,txt_h,shidx,str_id,NULL)

#define LDEF_BMP_TXT1(root,var_bmp,var_txt,nxt_obj,bmp_l,bmp_t,bmp_w,bmp_h,txt_t,txt_w,txt_h,shidx,str_id,bmp_id,num) \
    LDEF_BMP(&root,var_bmp,&var_txt,bmp_l+(bmp_w+txt_w+18)*num,bmp_t,bmp_w,bmp_h,bmp_id) \
    LDEF_TXT(&root,var_txt,NULL,bmp_l+bmp_w+(bmp_w+txt_w+18)*num,txt_t,txt_w,txt_h,shidx,str_id,NULL)

LDEF_TITLE(win_calender_con, calen_title, &win_calen_bmp1, \
        CALEN_MAINFRAME_L+600, CALEN_MAINFRAME_T-45, CALEN_MAINFRAME_W-600, 45, calen_title_str)
LDEF_TXT(&win_calender_con,calen_text,NULL,0,0,0,0,WSTL_MIXBACK_WHITE_IDX,0,display_strs[2])
LDEF_TXT(&win_calender_con,week_text,NULL,0,0,0,0,WSTL_MIXBACK_BLACK_IDX,0,display_strs[3])

LDEF_BMP_TXT(win_calender_con,win_calen_bmp1,win_calen_txt1,win_calen_bmp2,\
    HELP_BMP_L, HELP_BMP_T, HELP_BMP_W, HELP_BMP_H,HELP_TXT_T,HELP_TXT_W,HELP_TXT_H,\
    WSTL_GAME_TEXT_01, RS_MONTH_INC,IM_EPG_COLORBUTTON_RED,0)
LDEF_BMP_TXT(win_calender_con,win_calen_bmp2,win_calen_txt2,win_calen_bmp3,\
    HELP_BMP_L, HELP_BMP_T, HELP_BMP_W, HELP_BMP_H,HELP_TXT_T,HELP_TXT_W,HELP_TXT_H,\
    WSTL_GAME_TEXT_01, RS_MONTH_DEC,IM_EPG_COLORBUTTON_GREEN,1)
LDEF_BMP_TXT(win_calender_con,win_calen_bmp3,win_calen_txt3,win_calen_bmp4,\
    HELP_BMP_L, HELP_BMP_T, HELP_BMP_W, HELP_BMP_H,HELP_TXT_T,HELP_TXT_W,HELP_TXT_H,\
    WSTL_GAME_TEXT_01, RS_YEAR_INC,IM_EPG_COLORBUTTON_YELLOW,2)
LDEF_BMP_TXT1(win_calender_con,win_calen_bmp4,win_calen_txt4,NULL,\
    HELP_BMP_L, HELP_BMP_T, HELP_BMP_W, HELP_BMP_H,HELP_TXT_T,HELP_TXT_W,HELP_TXT_H,\
    WSTL_GAME_TEXT_01, RS_YEAR_DEC,IM_EPG_COLORBUTTON_BLUE,3)
LDEF_WIN(win_calender_con, &month_item_con, WIN_CALEN_LEFT, WIN_CALEN_TOP, WIN_CALEN_WIDTH, WIN_CALEN_HEIGHT,1)

#ifdef PERSIAN_CALENDER
    #define CAL_BEGINYEAR   1377 //1999
    #define CAL_ENDYEAR     1478 //2100
#else
    #define CAL_BEGINYEAR   1999 //1999
    #define CAL_ENDYEAR     2100 //2100
#endif
typedef struct
{
    UINT16  u16x;
    UINT16  u16y;
    UINT16  u16width;
    UINT16  u16height;
}COORDINATE_POS;

#define COLOR_DARKRED 0xD6
#define COLOR_YELLOW 0x92
#define COLOR_PINRED 0xE7
#define COLOR_GREEN 0xC0
#define COLOR_LIGHTBROWN 0xA0
#define COLOR_WHITE 0xFC
#define COLOR_BLACK 0xD0
#define COLOR_GRAY 0xF8
#define COLOR_LIGHTERGRAY 0xF9
#define COLOR_BACK 0xB1
#define COLOR_BLUE 0xD3
#define COLOR_CYAN 0xB5
#define COLOR_RED 0x18
#define COLOR_BROWN 0xD5
#define COLOR_LIGHTGRAY 0xF6
#define COLOR_DARKGRAY 0xF5

#define COLOR_D_GREEN   0x10FF8D00
#define COLOR_Y_GREEN   0x10FFB200

#define MONTHMAX 12
#ifdef PERSIAN_CALENDER
#define SOLAR_MONTH 12
#else
#define SOLAR_MONTH 2
#endif

#define THOUSANDFOLD_LEAP_VALUE  310

#define VACT_MONTH_INC (VACT_PASS + 1)
#define VACT_MONTH_DEC (VACT_PASS + 2)
#define VACT_YEAR_INC  (VACT_PASS + 3)
#define VACT_YEAR_DEC  (VACT_PASS + 4)

UINT16 current_year;
UINT8 current_month, current_date, u8focus_date;
UINT8 month_total_days;
UINT8 first_flag;
static UINT8 u8cal_x, u8cal_y;
static UINT16 u16focus_year ;
static UINT8  u8focus_month;
date_time date_time_calen;

static COORDINATE_POS * current_pos=NULL;
static COORDINATE_POS calen_frame;
static COORDINATE_POS calen_frame_top;
static COORDINATE_POS calen_frame_body;
static BOOL find_flag=FALSE;

static UINT16  gregorian_month[] =
{
    RS_CONAX_JAN,
    RS_CONAX_FAB,
    RS_CONAX_MAR,
    RS_CONAX_APR,
    RS_CONAX_MAY,
    RS_CONAX_JUN,
    RS_CONAX_JUL,
    RS_CONAX_AUG,
    RS_CONAX_SEP,
    RS_CONAX_OCT,
    RS_CONAX_NOV,
    RS_CONAX_DEC,
};

#ifdef PERSIAN_CALENDER
static UINT16  gregorian_weekday[] =
{
    RS_SATURDAY,
    RS_SUNDAY,
    RS_MONDAY,
    RS_TUESDAY,
    RS_WEDNESDAY,
    RS_THURSDAY,
    RS_FRIDAY,
};
#else
static UINT16  gregorian_weekday[] =
{
    RS_SUNDAY,
    RS_MONDAY,
    RS_TUESDAY,
    RS_WEDNESDAY,
    RS_THURSDAY,
    RS_FRIDAY,
    RS_SATURDAY,
};
#endif

void win_calen_time_get_input()
{
   osd_get_edit_field_time_date(&year_item_txtset,&date_time_calen);
}

void win_calen_time_set_input()
{
    UINT32 dwvalue = 0;

    if (!first_flag)
        get_local_time(&date_time_calen);

    dwvalue = date_time_calen.year*10000+date_time_calen.month*100+date_time_calen.day;
    osd_set_edit_field_content(&year_item_txtset, STRING_NUMBER, dwvalue);
    if(find_flag)
        osd_track_object((POBJECT_HEAD )&month_item_con,C_UPDATE_ALL);
    else
            osd_draw_object((POBJECT_HEAD )&month_item_con,C_UPDATE_ALL);
}

static VACTION calen_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static VACTION calen_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        case V_KEY_UP:
        case V_KEY_DOWN:
            act = VACT_ENTER;
            break;  
        default:
            act = VACT_PASS;
            break;
    }

    return act;
}
void calender_draw_title(void);
void calendar_init (UINT16 u16para1, UINT16 u16para2, UINT16 u16para3);
static PRESULT calen_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    UINT8   unact,input;
    UINT8   m_focus_id;
    UINT8   save;
    INT32   ret1;
    UINT8   id = osd_get_obj_id(p_obj);

    switch(event)
    {   
        case EVN_FOCUS_PRE_GET:
            break;

        case EVN_FOCUS_PRE_LOSE:
            break;

        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16) ;
            if(unact == VACT_ENTER)
            {
                win_calen_time_get_input();
                ret1 = api_check_valid_date(date_time_calen.year,date_time_calen.month,date_time_calen.day);
                if(ret1 != 0)
                {
                    win_compopup_init(WIN_POPUP_TYPE_OKNO);
                    win_compopup_set_msg(NULL,NULL,RS_MSG_INVALID_INPUT_CONTINUE);
                    win_compopup_set_frame(0xF0,0x96,0x1FC,0xD4);
                    if(win_compopup_open_ext(&save) == WIN_POP_CHOICE_YES)
                    ret = PROC_LOOP;
             else
                {
                    first_flag=0x00;
                    win_calen_time_set_input();
                }
                }
                else
                {
          find_flag=FALSE;
                    win_calen_time_set_input();
     #ifdef PERSIAN_CALENDER
                     converse_local_time_to_iran(&date_time_calen);
                     calendar_init(date_time_calen.year,date_time_calen.month,date_time_calen.day);
                     converse_iran_time_to_local(&date_time_calen);
     #else
                     calendar_init(date_time_calen.year,date_time_calen.month,date_time_calen.day);
     #endif
                     m_focus_id = osd_get_focus_id((POBJECT_HEAD)&win_calender_con);
                     osd_set_attr((POBJECT_HEAD)&month_item_con, C_ATTR_INACTIVE);
                     osd_set_container_focus(&win_calender_con, 1);
                     m_focus_id = osd_get_focus_id((POBJECT_HEAD)&win_calender_con);
                     osd_set_attr((POBJECT_HEAD)&win_calender_con, C_ATTR_ACTIVE);
                     osd_draw_object((POBJECT_HEAD )&win_calender_con,C_UPDATE_FOCUS);
                }

            }
            ret = PROC_LOOP;
            break;
    }
    return ret;
}

void calender_draw_title(void)
{
    TEXT_FIELD* txt=NULL;
    UINT8 *uni_month=NULL;
    char  str[64];
    char uni_temp[64];
    //char uni_temp[20];

    txt = &calen_title;

    snprintf(str,64,"K %d/%d/%d",current_year,current_month,current_date);
    com_asc_str2uni((UINT8*)str,(UINT16*)uni_temp);

    osd_set_text_field_content(txt, STRING_UNICODE,(UINT32)uni_temp);
    osd_draw_object((POBJECT_HEAD )&calen_title,C_UPDATE_ALL);

}

/**
* Name   : persian_isleap()
* Desc    :判断伊朗年是平年还是闰年。方法是：
           将这一年加38后，乘以31，再除以128。
           当结果的小数部分大于或等于0.31时，
           所指的这一年是一个平年。另一方面，如果小于0.31，
           则这一年是闰年，但是如果连续两年小于0.31，
           则第一年是闰年和第二年是平年。
* Param  :Iran year
*
* Return : 平年：0，闰年：1
************************************************************/
UINT8 persian_isleap(UINT16 year)
{
    UINT16 now;
    UINT16 bef;

    now = ( (year + 38 ) * 31000 / 128 ) % 1000; /*31放大1000倍，使余数放大1000倍*/
    if(now>= THOUSANDFOLD_LEAP_VALUE) /*与310比较，大于为平年*/
    {
        return 0;
    }
    else
    {
        bef = (( year - 1 + 38 ) * 31000 / 128) % 1000;/*计算上一年的值，判断年续两年<=310的情况*/
        if(bef < THOUSANDFOLD_LEAP_VALUE)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

static UINT8 _get_leap( UINT16 year )
{
    if (year % 400 == 0)
    {
        return 1;
    }
    else if (year % 100 == 0)
    {
        return 0;
    }
    else if (year % 4 == 0)
    {
        return 1;
    }
    return 0;
}

static UINT8 solar_cal[12] =
{
#ifdef PERSIAN_CALENDER
    31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29
#else
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
#endif
};

UINT8 get_days_of_month( UINT16 u16solar_year, UINT8 u8solar_month )
{
#ifdef PERSIAN_CALENDER
    if (persian_isleap(u16solar_year) != 0 && u8solar_month == SOLAR_MONTH)
#else
    if (_get_leap(u16solar_year) != 0 && u8solar_month == SOLAR_MONTH)
#endif
    {
        return solar_cal[u8solar_month - 1] + 1;
    }

    if (u8solar_month>MONTHMAX)
    {
        return 0;
    }
    return solar_cal[u8solar_month - 1];
}

static UINT16 _get_daysof_year( UINT16 u16solar_year )
{
#ifdef PERSIAN_CALENDER
    if (persian_isleap(u16solar_year) != 0)
#else
    if (_get_leap(u16solar_year) != 0)
#endif
    {
        return 366;
    }
    else
    {
        return 365;
    }
}

UINT8 solar_calendar( UINT16 u16year, UINT8 u8month, UINT8 u8day )
{
    UINT8 u8week, u8idx;

    UINT16 u16the_day_in_year, u16the_day_in_period;

    u16the_day_in_year = u8day;

    for (u8idx = 0; u8idx<u8month - 1; u8idx++)
    {
#ifdef PERSIAN_CALENDER
        if (MONTHMAX == u8idx )
        {
            if (1 == persian_isleap(u16year))
            {
                u16the_day_in_year++;
            }
        }
#else
        if (1 == u8idx)
        {
            if ( 1 == _get_leap(u16year))
            {
                u16the_day_in_year++;
            }
        }
#endif
        u16the_day_in_year += solar_cal[u8idx];
    }
#ifdef PERSIAN_CALENDER
    UINT16 i;
    UINT16 count_leap=0;

    for(i=1304;i<u16year;i++)
    {
        if(persian_isleap(i))
        {
            count_leap+=1;
        }
    }

    u16the_day_in_period = u16year - 1304  + count_leap + u16the_day_in_year;
#else
    u16the_day_in_period = u16year - 1 + (u16year - 1) / 4 - (u16year - 1)/100+(u16year-1)/400+u16the_day_in_year;
#endif
    u8week = u16the_day_in_period % 7;

#ifdef PERSIAN_CALENDER
    if(0 == u8week)
    {
        u8week = 6;
    }
    else
    {
        u8week -= 1;
    }
#endif
//libc_printf("ear-month-day: %d,%d,%d,week =%d\n",u16Year,u8Month,u8Day,u8Week);
    return (u8week);
}

void calender_draw_item(UINT8 u8date, UINT8 b8focus)
{
    UINT16 u16content_x, u16content_y, u16content_w, u16content_h;
    UINT16 u16number_x, u16number_y;
    UINT16 u16focus_x1, u16focus_x2;
    UINT16 u16focus_y1, u16focus_y2;
    UINT16 u16focus_w,  u16focus_h;
    UINT8 u8idx;

    u8cal_x = solar_calendar(current_year,current_month,u8date);

    for(u8idx=1;u8idx<8;u8idx++)
    {
        if((DAYS_A_WEEK-1) == solar_calendar(current_year,current_month,u8idx))
            break;
    }
    if(u8date<=u8idx)
    {
        u8cal_y=0;
    }
    else
    {
        if(0 == (u8date-u8idx)%7)
            u8cal_y=(u8date-u8idx)/7;
        else
            u8cal_y=(u8date-u8idx)/7+1;
    }

    u16content_x = current_pos->u16x + 10 + u8cal_x*CALEN_DATE_ITEM_W;
    u16content_y = current_pos->u16y + 5 + CALEN_WEEKLY_H + u8cal_y*CALEN_DATE_ITEM_H;
    u16content_w = CALEN_DATE_ITEM_W;
    u16content_h = CALEN_DATE_ITEM_H;
//libc_printf("\nu8Date=%d,u8CalX=%d,u8CalY=%d,current_month=%d-u8idx=%d-\n",u8Date,u8CalX,u8CalY,current_month,u8idx);

    if(b8focus)
    {
        u16focus_w  = u16content_w ;//- 2;
        u16focus_h  = u16content_h - FOCUS_REC_X_OFFSEF;

        u16focus_x1 = u16content_x ;//- FOCUS_REC_OFFSET_X;
        u16focus_x2 = u16focus_x1 + u16focus_w-2;
        u16focus_y1 = u16content_y;
        u16focus_y2 = u16content_y + u16focus_h;

        osd_draw_fill(u16focus_x1, u16focus_y1, u16focus_w,   2,         0x10fff100,NULL);
        osd_draw_fill(u16focus_x1, u16focus_y1, 2,           u16focus_h, 0x10fff100,NULL);
        osd_draw_fill(u16focus_x2, u16focus_y1, 2,           u16focus_h, 0x10fff100,NULL);
        osd_draw_fill(u16focus_x1, u16focus_y2, u16focus_w,   2,         0x10fff100,NULL);
    }

    TEXT_FIELD* txt=NULL;
    txt = &calen_text;
    osd_set_rect(&calen_text.head.frame, u16content_x,u16content_y,u16content_w,u16content_h);
    osd_set_text_field_content(&calen_text,STRING_ANSI, (UINT32)" ");
    osd_set_text_field_content(txt,STRING_NUMBER,(UINT32)u8date);
    osd_draw_object((POBJECT_HEAD )&calen_text,C_UPDATE_ALL);
}

void calender_draw_background(void)
{
    UINT8 u8idx;
    UINT16 u16weekly_x, u16weekly_y;
    UINT8 *uni_month=NULL;
    TEXT_FIELD* txt=NULL;
    char uni_buff[64];

    calen_frame_top.u16x=CALEN_MAINFRAME_L;
    calen_frame_top.u16y=CALEN_MAINFRAME_T;
    calen_frame_top.u16width=CALEN_MAINFRAME_W;
    calen_frame_top.u16height=30;
    current_pos = &calen_frame_top;

    calen_frame_body.u16x=CALEN_MAINFRAME_L;
    calen_frame_body.u16y=CALEN_MAINFRAME_T+45;
    calen_frame_body.u16width=CALEN_MAINFRAME_W;
    calen_frame_body.u16height=CALEN_MAINFRAME_H-45;

    osd_draw_fill(CALEN_MAINFRAME_L, CALEN_MAINFRAME_T, CALEN_MAINFRAME_W, CALEN_MAINFRAME_H, COLOR_D_GREEN,NULL);
    osd_draw_fill(current_pos->u16x,current_pos->u16y,current_pos->u16width, CALEN_WEEKLY_H,COLOR_Y_GREEN,NULL);

    for(u8idx=0; u8idx<7; u8idx++)
    {
        u16weekly_x =CALEN_DATE_ITEM_W;
        u16weekly_x = current_pos->u16x + CALEN_DATE_ITEM_X + (UINT16)(u8idx)*CALEN_DATE_ITEM_W;
        u16weekly_y = current_pos->u16y ;

        txt = &week_text;
        uni_month = osd_get_unicode_string(gregorian_weekday[u8idx]);
        com_uni_str_copy_char((UINT8*)uni_buff, uni_month);
        osd_set_rect(&week_text.head.frame, u16weekly_x, u16weekly_y,CALEN_DATE_ITEM_W,CALEN_DATE_ITEM_H);
        osd_set_text_field_content(txt, STRING_UNICODE,(UINT32)uni_buff);
        osd_draw_object((POBJECT_HEAD )&week_text,C_UPDATE_ALL);
    }
}

void calender_draw_all(void)
{
    UINT8 u8idx;
    UINT8 u8key;
    UINT16 X;
    UINT16 Y;
    UINT16 W;
    UINT16 H;

    calen_frame.u16x=CALEN_MAINFRAME_L;
    calen_frame.u16y=CALEN_MAINFRAME_T;
    calen_frame.u16width=CALEN_MAINFRAME_W;
    calen_frame.u16height=CALEN_MAINFRAME_H;
    current_pos = &calen_frame;
    X = current_pos->u16x;
    Y = current_pos->u16y + CALEN_WEEKLY_H + 5;
    W= current_pos->u16width;
    H = current_pos->u16height - CALEN_WEEKLY_H;
    osd_draw_fill(X, Y,W, H, COLOR_D_GREEN,NULL);
    calender_draw_title();

    for(u8idx=1; u8idx<=month_total_days; u8idx++)
    {
        if((u8idx == current_date)
            &&(current_month == u8focus_month)
            &&(current_year == u16focus_year))
            {
                calender_draw_item(u8idx, TRUE);
            }
            else
            {
                calender_draw_item(u8idx, FALSE);
            }
    }
}

void calendar_init (UINT16 u16para1, UINT16 u16para2, UINT16 u16para3)
{
    date_time st_time;
    get_local_time(&st_time);
    calender_draw_background();

    current_year = u16para1;
    current_month = u16para2;
    current_date = u16para3;
    if(current_year>CAL_ENDYEAR
        || current_year<CAL_BEGINYEAR
        || current_month>12
        || current_month<1
        || current_date>31
        || current_date<1)
    {
#ifdef PERSIAN_CALENDER
        current_year = 1389;//2004-622;
        current_month = 1;
        current_date = 1;
#else
        current_year = 2004;
        current_month = 1;
        current_date = 22;
#endif
    }

    u16focus_year = current_year;
    u8focus_month = current_month;
    u8focus_date = current_date;

    month_total_days = get_days_of_month(current_year, current_month);

    calender_draw_all();
    first_flag = 1;
}

static VACTION calen_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act;

    switch (key)
    {
        case V_KEY_MENU:
        case V_KEY_EXIT:
            first_flag = 0;
            act = VACT_CLOSE;
            break;
        case V_KEY_UP:
        case V_KEY_GREEN:
            act = VACT_MONTH_DEC;
            break;
        case V_KEY_DOWN:
        case V_KEY_RED:
            act = VACT_MONTH_INC;
            break;
        case V_KEY_FIND:
            act = VACT_SELECT;
            break;
        case V_KEY_LEFT:
        case V_KEY_BLUE:
            act = VACT_YEAR_DEC;
            break;

        case V_KEY_RIGHT:
        case V_KEY_YELLOW:
            act = VACT_YEAR_INC;
            break;
        default:
            act = osd_container_key_map(obj, key);
            break;
    }
    return act;
}

static PRESULT calen_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
        PRESULT ret = PROC_PASS;
        VACTION unact;

        switch (event)
        {
            case EVN_PRE_OPEN:
                break;

            case EVN_POST_CLOSE:
                osd_draw_object((POBJECT_HEAD)&g_win_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            break;

            case EVN_POST_DRAW:
                game_1st_draw();
                break;

            case EVN_MSG_GOT:
                break;

            case EVN_KEY_GOT:
                unact = (VACTION)(param1>>16);
                ret = game_key_proc(unact,0,0);
                break;
        }
        return ret;
}
void update_date()
{
UINT8 month_total_days;
date_time date_time_calen;
        month_total_days = get_days_of_month(current_year, current_month);
        date_time_calen.day   = current_date;
        date_time_calen.month = current_month;
        date_time_calen.year  = current_year;
        u16focus_year = current_year;
        u8focus_month = current_month;
        u8focus_date = current_date;
#ifdef PERSIAN_CALENDER
        converse_iran_time_to_local(&date_time_calen);
#endif
        win_calen_time_set_input();
         calender_draw_all();
}
static PRESULT calen_key_change_process(UINT32 vkey, UINT8 key_repeat_cnt, UINT8 key_status)
{
    PRESULT ret = PROC_LOOP;
    switch(vkey)
    {
     case VACT_MONTH_DEC:
            if(current_month>1)
            {
                current_month--;
            }
            else
            {
                if(current_year>CAL_BEGINYEAR)
                {
                    current_year--;
                    current_month = 12;
                }
            }
        update_date();
            break;
      case VACT_MONTH_INC:
            if(current_month<MONTHMAX)
            {
                current_month++;
            }
            else
            {
                if(current_year<CAL_ENDYEAR)
                {
                    current_year++;
                    current_month = 1;
                }
            }
        update_date();
            break;
      case VACT_YEAR_DEC:
            if(current_year>CAL_BEGINYEAR)
            {
                current_year--;
            }
            else
            {
                current_year = CAL_ENDYEAR;
            }
        update_date();
            break;
      case VACT_YEAR_INC:
            if(current_year<CAL_ENDYEAR)
            {
                current_year++;
            }
            else
            {
                current_year = CAL_BEGINYEAR;
            }
        update_date();
            break;
       case VACT_SELECT:
            osd_set_container_focus(&win_calender_con, 2);
            osd_set_attr((POBJECT_HEAD)&month_item_con, C_ATTR_ACTIVE);
            osd_draw_object((POBJECT_HEAD )&month_item_con,C_UPDATE_FOCUS);
       find_flag=TRUE;
       update_date();
            break;
       case VACT_CLOSE:
            ret = PROC_LEAVE;
            break;
    }

    return ret;
}

static void calen_1st_draw(void)
{
    int i;
    date_time st_time;
    get_local_time(&st_time);
    win_calen_time_set_input();
#ifdef PERSIAN_CALENDER
    converse_local_time_to_iran(&st_time);
    calendar_init(st_time.year,st_time.month,st_time.day);
#else
    calendar_init(st_time.year,st_time.month,st_time.day);
#endif
}

void calender_init(void)
{
    game_key_proc = calen_key_change_process;
    game_1st_draw = calen_1st_draw;
}
#endif

