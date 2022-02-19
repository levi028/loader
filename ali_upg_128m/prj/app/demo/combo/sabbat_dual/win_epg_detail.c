/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: Win_epg_detail.c
*
*    Description: The function for epg detail
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
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <hld/dis/vpo.h>
#include <api/libosd/osd_lib.h>

//#include <api/libtsi/si_epg.h>
#include <api/libsi/lib_epg.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#include "copper_common/com_epg.h"
#include "win_epg.h"
#include "control.h"

#define VACT_EPG_LANG           (VACT_PASS + 4)

/*******************************************************************************
*   Objects definition
*******************************************************************************/
CONTAINER g_win_epg_detail;

BITMAP epg_detail_chan_bmp;
TEXT_FIELD epg_detail_chan;
TEXT_FIELD epg_detail_event_name;
TEXT_FIELD epg_detail_time;
TEXT_FIELD epg_detail_lang;
TEXT_FIELD epg_detail_content;

TEXT_FIELD  epg_detail_split1;

MULTI_TEXT epg_detail_info;
SCROLL_BAR epg_detail_scb;

TEXT_FIELD  epg_detail_split2;

BITMAP epg_detail_help0_bmp;
BITMAP epg_detail_help1_bmp;
TEXT_FIELD epg_detail_help0_txt;
TEXT_FIELD epg_detail_help1_txt;

static VACTION epg_detail_info_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_detail_info_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);;

static VACTION epg_detail_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT epg_detail_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_EPG_DETAIL_WIN_SH_IDX    WSTL_POP_WIN_01_HD

#define WIN_EPG_DETAIL_EVENT_SH_IDX WSTL_TEXT_04_HD

//#define LANG_SH_IDX WSTL_TEXT_10_HD

#define BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharo WSTL_SCROLLBAR_01_HD

#define BAR_MID_RECT_IDX    WSTL_NOSHOW_IDX
#define BAR_MID_THUMB_IDX   WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD

#define BMP_SH_IDX     WSTL_MIX_BMP_HD

#define SPLIT_LINE_SH   WSTL_WIN_MAINMENU_01_BG_HD

#ifndef SD_UI
#define WIN_EPG_DETAIL_W_L         250//424
#define WIN_EPG_DETAIL_W_T         120//160
#define WIN_EPG_DETAIL_W_W         550
#define WIN_EPG_DETAIL_W_H         426

#define EVENT_TIMEBMP_L (WIN_EPG_DETAIL_W_L + 20)
#define EVENT_TIMEBMP_T (WIN_EPG_DETAIL_W_T + 30)
#define EVENT_TIMEBMP_W 28
#define EVENT_TIMEBMP_H 28

#define CHAN_L (EVENT_TIMEBMP_L+EVENT_TIMEBMP_W)
#define CHAN_T (EVENT_TIMEBMP_T-6)
#define CHAN_W (WIN_EPG_DETAIL_W_W - 70)
#define CHAN_H 40

#define WIN_EPG_DETAIL_EVENT_NAME_L (CHAN_L)
#define WIN_EPG_DETAIL_EVENT_NAME_T (CHAN_T + CHAN_H)
#define WIN_EPG_DETAIL_EVENT_NAME_W (CHAN_W)
#define WIN_EPG_DETAIL_EVENT_EVENT_NAME_H 40

#define WIN_EPG_DETAIL_EVENT_TIME_L (WIN_EPG_DETAIL_EVENT_NAME_L)
#define WIN_EPG_DETAIL_EVENT_TIME_T (WIN_EPG_DETAIL_EVENT_NAME_T + WIN_EPG_DETAIL_EVENT_EVENT_NAME_H)
#define WIN_EPG_DETAIL_EVENT_TIME_W 320//180//124
#define WIN_EPG_DETAIL_EVENT_TIME_H 40

#define EVENT_LANG_L (WIN_EPG_DETAIL_EVENT_TIME_L + WIN_EPG_DETAIL_EVENT_TIME_W)
#define EVENT_LANG_T (WIN_EPG_DETAIL_EVENT_TIME_T)
#define EVENT_LANG_W 120//180//134
#define EVENT_LANG_H 40

#define EVENT_CONTENT_L (WIN_EPG_DETAIL_EVENT_TIME_L)
#define EVENT_CONTENT_T (WIN_EPG_DETAIL_EVENT_TIME_T + WIN_EPG_DETAIL_EVENT_TIME_H)
#define EVENT_CONTENT_W 140
#define EVENT_CONTENT_H 40

#define SPLIT_LINE1_L (EVENT_TIMEBMP_L)
#define SPLIT_LINE1_T (EVENT_CONTENT_T + EVENT_CONTENT_H + 4)
#define SPLIT_LINE1_W (EVENT_TIMEBMP_W + CHAN_W)
#define SPLIT_LINE1_H 2

#define DTL_INFO_L  CHAN_L
#define DTL_INFO_T  (SPLIT_LINE1_T + SPLIT_LINE1_H + 4)
#define DTL_INFO_W  400//294
#define DTL_INFO_H  170//156

#define WIN_EPG_DETAIL_SCB_L (CHAN_L + CHAN_W - 18)
#define WIN_EPG_DETAIL_SCB_T (DTL_INFO_T)
#define WIN_EPG_DETAIL_SCB_W 12//18
#define WIN_EPG_DETAIL_SCB_H (DTL_INFO_H)

#define SPLIT_LINE2_L (SPLIT_LINE1_L)
#define SPLIT_LINE2_T (DTL_INFO_T + DTL_INFO_H + 4)
#define SPLIT_LINE2_W (SPLIT_LINE1_W)
#define SPLIT_LINE2_H 2

#define HELP_BMP_GREEN_L (WIN_EPG_DETAIL_W_L + 120)
#define HELP_BMP_GREEN_T (DTL_INFO_T + DTL_INFO_H + 10/*16*/)
#define HELP_BMP_GREEN_W 28
#define HELP_BMP_GREEN_H 30

#define HELP_TXT_GREEN_L (HELP_BMP_GREEN_L + HELP_BMP_GREEN_W + 10)
#define HELP_TXT_GREEN_T (HELP_BMP_GREEN_T)
#define HELP_TXT_GREEN_W 130
#define HELP_TXT_GREEN_H 36


#define WIN_EPG_DETAIL_HELP_BMP_L (WIN_EPG_DETAIL_W_L + 300)//528)//738
#define WIN_EPG_DETAIL_HELP_BMP_T (DTL_INFO_T + DTL_INFO_H + 10/*16*/)
#define WIN_EPG_DETAIL_HELP_BMP_W 28
#define WIN_EPG_DETAIL_HELP_BMP_H 30

#define WIN_EPG_DETAIL_HELP_TXT_L (WIN_EPG_DETAIL_HELP_BMP_L + WIN_EPG_DETAIL_HELP_BMP_W + 10)
#define WIN_EPG_DETAIL_HELP_TXT_T (WIN_EPG_DETAIL_HELP_BMP_T-4)
#define WIN_EPG_DETAIL_HELP_TXT_W 114
#define WIN_EPG_DETAIL_HELP_TXT_H 36
#else
#define WIN_EPG_DETAIL_W_L         120//424
#define WIN_EPG_DETAIL_W_T         70//160
#define WIN_EPG_DETAIL_W_W         320
#define WIN_EPG_DETAIL_W_H         320

#define EVENT_TIMEBMP_L (WIN_EPG_DETAIL_W_L + 10)
#define EVENT_TIMEBMP_T (WIN_EPG_DETAIL_W_T + 15)
#define EVENT_TIMEBMP_W 24
#define EVENT_TIMEBMP_H 24

#define CHAN_L (EVENT_TIMEBMP_L+EVENT_TIMEBMP_W)
#define CHAN_T (EVENT_TIMEBMP_T)
#define CHAN_W (WIN_EPG_DETAIL_W_W - 70)
#define CHAN_H 30

#define WIN_EPG_DETAIL_EVENT_NAME_L (CHAN_L)
#define WIN_EPG_DETAIL_EVENT_NAME_T (CHAN_T + CHAN_H)
#define WIN_EPG_DETAIL_EVENT_NAME_W (CHAN_W)
#define WIN_EPG_DETAIL_EVENT_EVENT_NAME_H 30

#define WIN_EPG_DETAIL_EVENT_TIME_L (WIN_EPG_DETAIL_EVENT_NAME_L)
#define WIN_EPG_DETAIL_EVENT_TIME_T (WIN_EPG_DETAIL_EVENT_NAME_T + WIN_EPG_DETAIL_EVENT_EVENT_NAME_H)
#define WIN_EPG_DETAIL_EVENT_TIME_W 240//120//124
#define WIN_EPG_DETAIL_EVENT_TIME_H 30

#define EVENT_LANG_L (WIN_EPG_DETAIL_EVENT_TIME_L )
#define EVENT_LANG_T (WIN_EPG_DETAIL_EVENT_TIME_T + WIN_EPG_DETAIL_EVENT_TIME_H)
#define EVENT_LANG_W 90//134
#define EVENT_LANG_H 30

#define EVENT_CONTENT_L (EVENT_LANG_L + EVENT_LANG_W)
#define EVENT_CONTENT_T (WIN_EPG_DETAIL_EVENT_TIME_T + WIN_EPG_DETAIL_EVENT_TIME_H)
#define EVENT_CONTENT_W 140
#define EVENT_CONTENT_H 30

#define SPLIT_LINE1_L (EVENT_TIMEBMP_L)
#define SPLIT_LINE1_T (EVENT_CONTENT_T + EVENT_CONTENT_H + 4)
#define SPLIT_LINE1_W (EVENT_TIMEBMP_W + CHAN_W)
#define SPLIT_LINE1_H 2

#define DTL_INFO_L  CHAN_L
#define DTL_INFO_T  (SPLIT_LINE1_T + SPLIT_LINE1_H + 4)
#define DTL_INFO_W  220//294
#define DTL_INFO_H  120//156

#define WIN_EPG_DETAIL_SCB_L (CHAN_L + CHAN_W - 18)
#define WIN_EPG_DETAIL_SCB_T (DTL_INFO_T)
#define WIN_EPG_DETAIL_SCB_W 12//18
#define WIN_EPG_DETAIL_SCB_H (DTL_INFO_H)

#define SPLIT_LINE2_L (SPLIT_LINE1_L)
#define SPLIT_LINE2_T (DTL_INFO_T + DTL_INFO_H + 4)
#define SPLIT_LINE2_W (SPLIT_LINE1_W)
#define SPLIT_LINE2_H 2

#define HELP_BMP_GREEN_L (WIN_EPG_DETAIL_W_L + 60)
#define HELP_BMP_GREEN_T (DTL_INFO_T + DTL_INFO_H + 10/*16*/)
#define HELP_BMP_GREEN_W 24
#define HELP_BMP_GREEN_H 24

#define HELP_TXT_GREEN_L (HELP_BMP_GREEN_L + HELP_BMP_GREEN_W + 10)
#define HELP_TXT_GREEN_T (HELP_BMP_GREEN_T)
#define HELP_TXT_GREEN_W 90
#define HELP_TXT_GREEN_H 30


#define WIN_EPG_DETAIL_HELP_BMP_L (WIN_EPG_DETAIL_W_L + 190)//528)//738
#define WIN_EPG_DETAIL_HELP_BMP_T (DTL_INFO_T + DTL_INFO_H + 10/*16*/)
#define WIN_EPG_DETAIL_HELP_BMP_W 24
#define WIN_EPG_DETAIL_HELP_BMP_H 24

#define WIN_EPG_DETAIL_HELP_TXT_L (WIN_EPG_DETAIL_HELP_BMP_L + WIN_EPG_DETAIL_HELP_BMP_W + 10)
#define WIN_EPG_DETAIL_HELP_TXT_T (WIN_EPG_DETAIL_HELP_BMP_T-4)
#define WIN_EPG_DETAIL_HELP_TXT_W 80
#define WIN_EPG_DETAIL_HELP_TXT_H 30

#endif

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,sh,align,res_id,str)     \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    align, 0,0,res_id,str)

#define LDEF_BMP(root,var_bmp,nxt_obj,l,t,w,h,icon)   \
  DEF_BITMAP(var_bmp,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, BMP_SH_IDX,BMP_SH_IDX,BMP_SH_IDX,BMP_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_EPG_LINE(parent, var_txt, nxt_obj, l, t, w, h, str)      \
    DEF_TEXTFIELD(var_txt,&parent,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, \
    l, t, w, h,  \
    SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_MTXT(root,var_mtxt,next_obj,ID,l,t,w,h,sh,cnt,sb,content)    \
    DEF_MULTITEXT(var_mtxt,&root,next_obj,C_ATTR_ACTIVE,0, \
        ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
        epg_detail_info_keymap,epg_detail_info_callback,  \
        C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,sb,content)

#define LDEF_BAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, BAR_SH_IDX, BAR_HL_IDX, BAR_SH_IDX, BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, BAR_MID_THUMB_IDX, BAR_MID_RECT_IDX, \
        0, 18, w, h - 36, 100, 1)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_EPG_DETAIL_WIN_SH_IDX,WIN_EPG_DETAIL_WIN_SH_IDX, \
    WIN_EPG_DETAIL_WIN_SH_IDX,WIN_EPG_DETAIL_WIN_SH_IDX,   \
    epg_detail_keymap,epg_detail_callback,  \
    nxt_obj, focus_id,0)

static TEXT_CONTENT detail_mtxt_content[]=
{
    {STRING_UNICODE,{0}},
};

LDEF_BMP(g_win_epg_detail,epg_detail_chan_bmp,&epg_detail_chan,\
                EVENT_TIMEBMP_L,EVENT_TIMEBMP_T,EVENT_TIMEBMP_W,EVENT_TIMEBMP_H,IM_ORANGE_ARROW_S)

LDEF_TXT(g_win_epg_detail,epg_detail_chan,&epg_detail_event_name, \
            CHAN_L,CHAN_T,CHAN_W,CHAN_H,WIN_EPG_DETAIL_EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[0])

LDEF_TXT(g_win_epg_detail,epg_detail_event_name,&epg_detail_time, \
            WIN_EPG_DETAIL_EVENT_NAME_L,WIN_EPG_DETAIL_EVENT_NAME_T, \
            WIN_EPG_DETAIL_EVENT_NAME_W,WIN_EPG_DETAIL_EVENT_EVENT_NAME_H,\
            WIN_EPG_DETAIL_EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[1])

LDEF_TXT(g_win_epg_detail,epg_detail_time,&epg_detail_lang, \
            WIN_EPG_DETAIL_EVENT_TIME_L,WIN_EPG_DETAIL_EVENT_TIME_T, \
            WIN_EPG_DETAIL_EVENT_TIME_W,WIN_EPG_DETAIL_EVENT_TIME_H,\
            WIN_EPG_DETAIL_EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[2])

LDEF_TXT(g_win_epg_detail,epg_detail_lang,&epg_detail_content, \
            EVENT_LANG_L,EVENT_LANG_T,EVENT_LANG_W,EVENT_LANG_H,\
            WIN_EPG_DETAIL_EVENT_SH_IDX,C_ALIGN_RIGHT| C_ALIGN_VCENTER,0,display_strs[3])

LDEF_TXT(g_win_epg_detail,epg_detail_content,&epg_detail_split1, \
            EVENT_CONTENT_L,EVENT_CONTENT_T,EVENT_CONTENT_W,EVENT_CONTENT_H,\
            WIN_EPG_DETAIL_EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,0,display_strs[4])

LDEF_EPG_LINE(g_win_epg_detail, epg_detail_split1, &epg_detail_info, \
       SPLIT_LINE1_L, SPLIT_LINE1_T, SPLIT_LINE1_W, SPLIT_LINE1_H, NULL);

LDEF_BAR(epg_detail_info,epg_detail_scb,6,WIN_EPG_DETAIL_SCB_L,WIN_EPG_DETAIL_SCB_T, \
        WIN_EPG_DETAIL_SCB_W, WIN_EPG_DETAIL_SCB_H)

LDEF_MTXT(g_win_epg_detail,epg_detail_info,&epg_detail_split2,1, \
            DTL_INFO_L,DTL_INFO_T,DTL_INFO_W,DTL_INFO_H,WIN_EPG_DETAIL_EVENT_SH_IDX,1, \
            &epg_detail_scb,detail_mtxt_content)

LDEF_EPG_LINE(g_win_epg_detail, epg_detail_split2, &epg_detail_help0_bmp, \
       SPLIT_LINE2_L, SPLIT_LINE2_T, SPLIT_LINE2_W, SPLIT_LINE2_H, NULL);

LDEF_BMP(g_win_epg_detail,epg_detail_help0_bmp,&epg_detail_help0_txt,\
                HELP_BMP_GREEN_L,HELP_BMP_GREEN_T,HELP_BMP_GREEN_W,HELP_BMP_GREEN_H,IM_EPG_COLORBUTTON_GREEN)
LDEF_TXT(g_win_epg_detail,epg_detail_help0_txt,&epg_detail_help1_bmp, \
                HELP_TXT_GREEN_L,HELP_BMP_GREEN_T,HELP_TXT_GREEN_W,HELP_BMP_GREEN_H,\
                WIN_EPG_DETAIL_EVENT_SH_IDX,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_SYSTEM_LANGUAGE,NULL)

LDEF_BMP(g_win_epg_detail,epg_detail_help1_bmp,&epg_detail_help1_txt,\
                WIN_EPG_DETAIL_HELP_BMP_L,WIN_EPG_DETAIL_HELP_BMP_T, \
                WIN_EPG_DETAIL_HELP_BMP_W,WIN_EPG_DETAIL_HELP_BMP_H,IM_EPG_COLORBUTTON_RED)
LDEF_TXT(g_win_epg_detail,epg_detail_help1_txt,NULL, \
                WIN_EPG_DETAIL_HELP_TXT_L,WIN_EPG_DETAIL_HELP_BMP_T, \
                WIN_EPG_DETAIL_HELP_TXT_W,WIN_EPG_DETAIL_HELP_BMP_H,WIN_EPG_DETAIL_EVENT_SH_IDX,\
                C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_COMMON_CANCEL,NULL)

LDEF_WIN(g_win_epg_detail,&epg_detail_chan_bmp,WIN_EPG_DETAIL_W_L, WIN_EPG_DETAIL_W_T, \
            WIN_EPG_DETAIL_W_W, WIN_EPG_DETAIL_W_H,1)

/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/
#define DETAIL_BUFFER_LEN       1024*3

static UINT16 *detail_str = NULL;
static UINT16 current_channel= 0;
static struct winepginfo *current_event_info = NULL;

/*******************************************************************************
*   Local functions definition
*******************************************************************************/
static INT32 win_epg_detail_info_update(eit_event_info_t *ep,INT32 idx)
{
    INT32 str_len=0;
    INT32 len= 0;
    UINT16 *p= 0;
    UINT8 *s= 0;

    p=detail_str;
    if ( !ep)
   {
    libc_printf("%s : The function paratem is NULL ! \n",__FUNCTION__);
    return 0;
   }

    s = epg_get_event_language(ep);
    if(NULL != s)
    {
        if(0 == MEMCMP(s,"ara",3))
            {
            detail_mtxt_content[0].b_text_type = STRING_REVERSE;
            }
        else
            {
            detail_mtxt_content[0].b_text_type = STRING_UNICODE;
            }
    }

    if(STRING_REVERSE == detail_mtxt_content[0].b_text_type)
    {
        len = epg_get_event_all_extented_detail(ep, p, DETAIL_BUFFER_LEN);
        if(0 != len)
        {
            p[len] = 0x2a00;  //'*'
            len++;
            p[len] = 0;
            if(len < DETAIL_BUFFER_LEN-1)
            {
                p[len] = 0x0a00;  //"\n"
                len++;
                p[len] = 0;
            }
        }

        str_len = epg_get_event_all_short_detail(ep, p+len, DETAIL_BUFFER_LEN-len);
        if(0 != str_len)
        {
            len += str_len;
            p[len] = 0x2a00;  //'*'
            len++;
            p[len] = 0;
        }

        api_str_reverse(p,len);
    }
    else
    {
        len = epg_get_event_all_short_detail(ep,p+1,DETAIL_BUFFER_LEN-1);
        if(0 != len)
        {
            p[0] = 0x2a00;  //'*'
            len++;
            if(len < DETAIL_BUFFER_LEN-1)
            {
                p[len] = 0x0a00;  //"\n"
                len++;
                p[len] = 0;
            }
        }

        str_len = epg_get_event_all_extented_detail(ep,p+len+1,DETAIL_BUFFER_LEN-len-1);
        if(0 != str_len)
        {
            p[len] = 0x2a00;    //'*'
            len += str_len+1;
        }
    }
    epg_detail_info.n_line = 0;

    if((NULL != p) &&( len > 0))
    {
        osd_set_attr((POBJECT_HEAD)&epg_detail_scb, C_ATTR_ACTIVE);
    }
    else
    {
        osd_set_attr((POBJECT_HEAD)&epg_detail_scb, C_ATTR_HIDDEN);
    }

    detail_mtxt_content[0].text.p_string = detail_str;

    return len;

}
static void set_osd_attr_invalid_event(INT8 *str, INT32 str_size, PTEXT_FIELD name, PTEXT_FIELD time,
                                    PTEXT_FIELD lang,PTEXT_FIELD content)
{
    strncpy(str,"", str_size-1);
    str[str_size-1] = 0;
    osd_set_text_field_content(name, STRING_ANSI, (UINT32)str);
    osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);
    osd_set_text_field_content(lang, STRING_ANSI, (UINT32)str);
    osd_set_text_field_content(content, STRING_ANSI, (UINT32)str);
    osd_set_attr((POBJECT_HEAD)&epg_detail_scb, C_ATTR_HIDDEN);
    detail_mtxt_content[0].text.p_string = NULL;

  return;
}
static void win_epg_detail_display(UINT16 channel, struct winepginfo*epginfo)
{
    P_NODE service_info;
//  T_NODE tp_info;
    PTEXT_FIELD chan = &epg_detail_chan;
    PTEXT_FIELD name = &epg_detail_event_name;
    PTEXT_FIELD lang = &epg_detail_lang;
    PTEXT_FIELD time = &epg_detail_time;
    PTEXT_FIELD content = &epg_detail_content;
    eit_event_info_t *ep = NULL;
    INT32 event_num= 0;
    UINT16 unistr[30] = {0};
    INT8 str[30] = {0};
    INT8 str2[30] = {0};
    INT32 len= 0;
    UINT8 *s= NULL;
    UINT16 num = 0; //event index
    UINT8 lang_tatol = 0;
    UINT8 lang_select = 0;
    const UINT8 str_size = 30;

    MEMSET(&service_info, 0, sizeof(P_NODE));
    /*channel*/
    if(SUCCESS == get_prog_at(channel, &service_info))
    {
        //goto NO_VALID_EVENT;
        set_osd_attr_invalid_event(str,30,name,time,lang,content);
    }

    if(service_info.ca_mode)
    {
        snprintf(str, 30, "%04d $",channel + 1);
    }
    else
    {
        snprintf(str, 30, "%04d ",channel + 1);
    }

    com_asc_str2uni((UINT8 *)str,unistr);
    len = com_uni_str_len(unistr);

    com_uni_str_copy_char_n((UINT8*)&unistr[len], service_info.service_name, 30-len);
    osd_set_text_field_content(chan, STRING_UNICODE, (UINT32)unistr);

    if(INVALID_ID == epginfo->event_idx)
    {
        //goto NO_VALID_EVENT;
        set_osd_attr_invalid_event(str,30,name,time,lang,content);
    }

    ep = epg_get_cur_service_event(channel, SCHEDULE_EVENT, &epginfo->start, &epginfo->end, &event_num, FALSE);
/*  if(NULL == ep)
    {
        ep=epg_get_cur_service_event(channel, PRESENT_EVENT, &epginfo->start, &epginfo->end, &event_num, FALSE);
        if (ep == NULL)
            ep=epg_get_cur_service_event(channel, FOLLOWING_EVENT, &epginfo->start, &epginfo->end, &event_num, FALSE);
    }*/

    while(NULL != ep)
    {
        if(ep->event_id==epginfo->event_idx)
        {
            s = epg_get_event_name(ep,&len);
            if (NULL == s)
            {
               // goto NO_VALID_EVENT;
               set_osd_attr_invalid_event(str,30,name,time,lang,content);
            }

            osd_set_text_field_content(name,STRING_UNICODE,(UINT32)s);

            epg_get_event_start_time(ep,(UINT8 *)str);
            len = STRLEN(str);

            if (str_size - len > 1)
            {
                strncpy(&str[len],"~", (str_size-len-1));
                len = STRLEN(str);
            }

            epg_get_event_end_time(ep,(UINT8 *)str2);
            if ((str_size-len) > ((INT32)strlen(str2)))
            {
                strncpy(&str[len],str2, (str_size-len-1));
            }

            osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);

            s = (UINT8*)epg_get_event_language(ep);
            if (NULL == s)
            {
                //goto NO_VALID_EVENT;
                set_osd_attr_invalid_event(str,30,name,time,lang,content);
            }

            lang_tatol = epg_get_event_language_count(ep);
            lang_select = epg_get_event_language_select(ep);
            if ( 0 == lang_tatol )
            {
                snprintf(str, 30, "%s(%d/%d)", s, 0, lang_tatol);
            }
            else
            {
                snprintf(str, 30, "%s(%d/%d)", s, lang_select+1, lang_tatol);
            }
            osd_set_text_field_content(lang,STRING_ANSI,(UINT32)str);

            epg_get_event_content_type(ep,(UINT8 *)str,30,&len);

            osd_set_text_field_content(content, STRING_ANSI, (UINT32)str);

            len = win_epg_detail_info_update(ep,0);

            break;
        }
//      ep=ep->next;
        ++num;
        ep = epg_get_schedule_event(num);
    }

    if(NULL == ep)
    {
         set_osd_attr_invalid_event(str,30,name,time,lang,content);
#if 0
NO_VALID_EVENT:
        STRCPY(str,"");
        osd_set_text_field_content(name, STRING_ANSI, (UINT32)str);
        osd_set_text_field_content(time, STRING_ANSI, (UINT32)str);
        osd_set_text_field_content(lang, STRING_ANSI, (UINT32)str);
        osd_set_text_field_content(content, STRING_ANSI, (UINT32)str);

        osd_set_attr((POBJECT_HEAD)&epg_detail_scb, C_ATTR_HIDDEN);
        detail_mtxt_content[0].text.p_string = NULL;
#endif
    }

   return;
}


static void win_epg_detail_refresh(void)
{
    PTEXT_FIELD chan = &epg_detail_chan;
    PTEXT_FIELD name = &epg_detail_event_name;
    PTEXT_FIELD lang = &epg_detail_lang;
    PTEXT_FIELD content = &epg_detail_content;
    PMULTI_TEXT info = &epg_detail_info;

    /* refresh item */
    osd_track_object((POBJECT_HEAD)chan, C_UPDATE_ALL);
    osd_track_object((POBJECT_HEAD)name, C_UPDATE_ALL);
    osd_track_object((POBJECT_HEAD)lang, C_UPDATE_ALL);
    osd_track_object((POBJECT_HEAD)content, C_UPDATE_ALL);
    osd_track_object((POBJECT_HEAD)info, C_UPDATE_ALL);

   return;
}

/*
 * win_epg_detail_lang_change - switch language and redraw detail window
 */
static void win_epg_detail_lang_change(void)
{
    UINT8 lang_select= 0;
    UINT8 count= 0;
    INT32 event_num= 0;
    eit_event_info_t *ep = NULL;

    ep = epg_get_cur_service_event(current_channel, SCHEDULE_EVENT,
                &current_event_info->start, &current_event_info->end, &event_num, FALSE);

    count = epg_get_event_language_count(ep);
    lang_select = epg_get_event_language_select(ep);

    if (count <= 1)
    {
        return;
    }

    lang_select = (lang_select + 1) % count;
    epg_set_event_langcode_select(ep->lang[lang_select].lang_code2);

    win_epg_detail_display(current_channel, current_event_info);

    win_epg_detail_refresh();

}


/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION epg_detail_info_keymap(POBJECT_HEAD pobj, UINT32 key)
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
        case V_KEY_P_UP:
            act = VACT_CURSOR_PGUP;
            break;
        case V_KEY_P_DOWN:
            act = VACT_CURSOR_PGDN;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;

}

static PRESULT epg_detail_info_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static PRESULT epg_detail_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_PASS;

    switch(act)
    {
        case VACT_EPG_LANG:
            win_epg_detail_lang_change();
            break;
        default:
            break;
    }

    return ret;
}


static VACTION epg_detail_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_EXIT:
            act = VACT_CLOSE;
            break;
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        case V_KEY_RED:
            act = VACT_CLOSE;
            break;
        case V_KEY_GREEN:
            act = VACT_EPG_LANG;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;

}

static PRESULT epg_detail_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact= PROC_PASS;

    switch(event)
    {
        case EVN_PRE_OPEN:
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            break;
        case EVN_POST_CLOSE:
            if (detail_str)
            {
                FREE(detail_str);
                detail_str=NULL;
            }
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            ret = epg_detail_unkown_act_proc(unact);
            break;
        default:
            break;
    }

    return ret;
}

void win_epg_detail_open(UINT16 channel, struct winepginfo *epginfo)
{
    POBJECT_HEAD pobj = (POBJECT_HEAD)&g_win_epg_detail;
    PRESULT bresult = PROC_LOOP;
    UINT32 hkey= 0;
    BOOL old_value= FALSE;

    if (NULL == detail_str)
    {
        detail_str=MALLOC(DETAIL_BUFFER_LEN*sizeof(UINT16));
    }

    if(NULL == detail_str)
    {
        return;
    }

    MEMSET(detail_str,0,DETAIL_BUFFER_LEN*sizeof(UINT16));

    current_channel = channel;
    current_event_info = epginfo;

    wincom_backup_region(&g_win_epg_detail.head.frame);

    win_epg_detail_display(channel,epginfo);

    osd_obj_open(pobj, MENU_OPEN_TYPE_OTHER);

    old_value = ap_enable_key_task_get_key(TRUE);
    while(PROC_LEAVE != bresult)
    {

        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey)  ||(INVALID_MSG == hkey))
        {
            continue;
        }

        bresult = osd_obj_proc(pobj, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    ap_enable_key_task_get_key(old_value);

    if (detail_str)
    {
        FREE(detail_str);
        detail_str=NULL;
    }
    wincom_restore_region();
}
