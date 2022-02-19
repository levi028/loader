/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chan_scan.c
*
*    Description: To realize the function to channel scan
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
#include <api/libpub/lib_frontend.h>
#endif
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_signal.h"
#include <api/libdb/db_interface.h>

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#include <hld/snd/snd.h>
#include <hld/deca/deca.h>
#include <api/libpub/lib_cc.h>
#include "platform/board.h"
#include "control.h"
#include "win_search.h"
//#define SUPPORT_DISEQC11
//#define SUPPORT_TONEBURST
/*******************************************************************************
*    Objects definition
*******************************************************************************/
#define FREQ_SETTING_SUPPORT 1

CONTAINER g_win_channelscan;

CONTAINER     chanscan_item1;        /* Scan Mode*/
CONTAINER     chanscan_item2;        /* Scan Band */
CONTAINER     chanscan_item3;        /* Ch No.  */
CONTAINER     chanscan_item4;        /* Frequency */
CONTAINER     chanscan_item5;        /* BandWidth */
CONTAINER     chanscan_item6;        /* Netwrok Search */
CONTAINER     chanscan_item7;        /* Start */

TEXT_FIELD     chanscan_txt1;
TEXT_FIELD     chanscan_txt2;
TEXT_FIELD     chanscan_txt3;
TEXT_FIELD     chanscan_txt4;
TEXT_FIELD     chanscan_txt5;
TEXT_FIELD     chanscan_txt6;
TEXT_FIELD     chanscan_txt7;

MULTISEL     chanscan_sel1;
MULTISEL     chanscan_sel2;
MULTISEL     chanscan_sel3;
EDIT_FIELD     chanscan_edt4;
MULTISEL     chanscan_sel5;
MULTISEL     chanscan_sel6;

TEXT_FIELD     chanscan_line1;
TEXT_FIELD     chanscan_line2;
TEXT_FIELD     chanscan_line3;
TEXT_FIELD     chanscan_line4;
TEXT_FIELD     chanscan_line5;
TEXT_FIELD     chanscan_line6;
TEXT_FIELD     chanscan_line7;

TEXT_FIELD     chanscan_split;

static VACTION chanscan_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT chanscan_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION chanscan_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT chanscan_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION chanscan_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT chanscan_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION chanscan_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT chanscan_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);




#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD
#define CON_ENTER_IDX   WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define SPLIT_LINE_SH    WSTL_LINE_MENU_HD

#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          250
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-TXT_W-20)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define SPLIT_LINE_L    CON_L
#ifdef SUPPORT_TWO_TUNER
#define SPLIT_LINE_T      (CON_T + 7 * (CON_H + CON_GAP)+2)
#else
#define SPLIT_LINE_T      (CON_T + 6 * (CON_H + CON_GAP) + 2)
#endif
#define SPLIT_LINE_W    CON_W
#define SPLIT_LINE_H    2

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    chanscan_item_keymap,chanscan_item_callback,  \
    conobj, ID,1)

#define LDEF_CON_HL(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    chanscan_item_keymap,chanscan_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    chanscan_item_sel_keymap,chanscan_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    chanscan_item_edt_keymap,chanscan_item_edt_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,pat,10,cursormode,NULL,sub,str)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_LIST(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON_HL(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_ENTER_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_ANT_LINE(root, var_txt, nxt_obj, l, t, w, h, str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, l, t, w, h, SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur_mode,pat,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    chanscan_keymap,chanscan_callback,  \
    nxt_obj, focus_id,0)

#define WIN  g_win_channelscan

char frepat[]  = "r000000~999999";
UINT16 fresub[10] = {0};
//static char* scan_mode_ids[] = {0};
//UINT16 scan_mode_ids2[]={0};
//char* scan_band_ids[]={0};
//UINT16  bandwidth_ids[]={0};
UINT8 scan_mode_sel =0 ;
static UINT8 tone_voice_init = 0;
__MAYBE_UNUSED__ static char* scan_mode_ids[] =
{
    "By Channel",
    "By Frequency",
};
UINT16 scan_mode_ids2[] = /* Should have the same order as stream_iso_639lang_abbr*/
{
    RS_SCAN_MODE_CHL,
    RS_SCAN_MODE_FRE,
};
char* scan_band_ids[]=
{
    "VHF",
    "UHF",
};
UINT16 bandwidth_ids[] =
{
    RS_6MHZ,
    RS_7MHZ,
    RS_8MHZ,
};

#if (FREQ_SETTING_SUPPORT)
LDEF_MENU_ITEM(WIN,chanscan_item1,&chanscan_item2,chanscan_txt1,chanscan_sel1, chanscan_line1, 1, 7, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INSTALLATION_SCAN_MODE, STRING_ID, 0, 2, scan_mode_ids2)
#else
LDEF_MENU_ITEM(WIN,chanscan_item1,&chanscan_item2,chanscan_txt1,chanscan_sel1, chanscan_line1, 1, 6, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INSTALLATION_SCAN_MODE, STRING_ID, 0, 2, scan_mode_ids2)
#endif

LDEF_MENU_ITEM(WIN,chanscan_item2,&chanscan_item3,chanscan_txt2,chanscan_sel2,chanscan_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INSTALLATION_SCAN_BAND, STRING_ANSI, 0, 2, scan_band_ids)

LDEF_MENU_ITEM(WIN,chanscan_item3,&chanscan_item4,chanscan_txt3,chanscan_sel3, chanscan_line3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_INSTALLATION_CHANNEL_NO, STRING_PROC, 0, 3, NULL)

LDEF_MENU_ITEM_EDT(WIN,chanscan_item4,&chanscan_item5,chanscan_txt4,chanscan_edt4, chanscan_line4, 4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_DISPLAY_FREQUENCY, NORMAL_EDIT_MODE, CURSOR_SPECIAL, frepat,fresub, display_strs[1])

#if (FREQ_SETTING_SUPPORT)
LDEF_MENU_ITEM(WIN,chanscan_item5,&chanscan_item6,chanscan_txt5,chanscan_sel5, chanscan_line5, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_INSTALLATION_BANDWIDTH, STRING_ID, 0, 3, bandwidth_ids)

LDEF_MENU_ITEM(WIN,chanscan_item6,&chanscan_item7,chanscan_txt6,chanscan_sel6, chanscan_line6, 6, 5, 7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, RS_INSTALLATION_NIT_SCAN, STRING_ID, 0, 2, yesno_ids)

LDEF_MENU_ITEM_TXT(WIN,chanscan_item7,NULL,chanscan_txt7, chanscan_line7, 7, 6, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H, RS_COMMON_SEARCH)
#else

LDEF_MENU_ITEM(WIN,chanscan_item5,&chanscan_item6,chanscan_txt5,chanscan_sel5, chanscan_line5, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_INSTALLATION_NIT_SCAN, STRING_ID, 0, 2, yesno_ids)


LDEF_MENU_ITEM_TXT(WIN,chanscan_item6,NULL,chanscan_txt6, chanscan_line6, 6, 5, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, RS_COMMON_SEARCH)
#endif

LDEF_ANT_LINE(WIN, chanscan_split, &chanscan_item1, SPLIT_LINE_L, SPLIT_LINE_T, SPLIT_LINE_W, SPLIT_LINE_H, NULL)

LDEF_WIN(WIN,&chanscan_split,W_L, W_T, W_W, W_H,1)

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/




POBJECT_HEAD chanscan_items[] =
{
    (POBJECT_HEAD)&chanscan_item1,
    (POBJECT_HEAD)&chanscan_item2,
    (POBJECT_HEAD)&chanscan_item3,
    (POBJECT_HEAD)&chanscan_item4,
    (POBJECT_HEAD)&chanscan_item5,
    (POBJECT_HEAD)&chanscan_item6,
#if (FREQ_SETTING_SUPPORT)
    (POBJECT_HEAD)&chanscan_item7,
#endif

};

extern UINT8    antset_cur_tuner;
static ID set_frontend = OSAL_INVALID_ID;
static BOOL current_frontend_is_isdbt = FALSE;


#define SCAN_MODE_CNT        (chanscan_sel1.n_count)
#define SCAN_MODE_IDX        (chanscan_sel1.n_sel)
#define SCAN_BAND_CNT        (chanscan_sel2.n_count)
#define SCAN_BAND_IDX        (chanscan_sel2.n_sel)
#if (FREQ_SETTING_SUPPORT)
#define BANDWIDTH_CNT        (chanscan_sel5.n_count)
#define BANDWIDTH_IDX        (chanscan_sel5.n_sel)
#define NETWORK_SRH_IDX        (chanscan_sel6.n_sel)
#else
#define NETWORK_SRH_IDX        (chanscan_sel5.n_sel)
#endif
typedef enum
{
    SCAN_MODE_ID = 1,
    SCAN_BAND_ID,
    CH_NO_ID,
    FREQUENCY_ID,
#if (FREQ_SETTING_SUPPORT)
    BANDWIDTH_ID,
#endif
    NETWORK_SRH_ID,
    START_ID
}WIN_ANTENNA_ITEM_ID;

win_scanch_channel_info_t scan_chan_info;
band_param    country_param[MAX_BAND_COUNT];

#if defined(DVBT2_SUPPORT)
static UINT32 start = 0;
#endif

static UINT32 ch_no = 0;
//modify for channel scan mode show the title auto scan on T tuner 2011 11 25
extern BOOL is_auto_scan;

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

#define VACT_POP_UP    (VACT_PASS + 1)

#ifdef ANTENNA_INSTALL_BEEP
extern UINT8 beep_bin_array[];
extern UINT32 beep_bin_array_size;
#endif
static void win_chanscan_load_setting();
void win_chanscan_get_setting(UINT32* freq);
static void switch_scan_mode_select(BOOL select);
void init_ch_no_info(band_param *buf);
void channel_param(band_param *buf,UINT32 chan_no,UINT32 *start_freq,UINT32 *end_freq,UINT32 *channel_no,UINT32 *bandwidth,BOOL *band_type);
void get_band_type(band_param *buf,UINT32* frequency,BOOL *band_type);
void win_chanscan_set_mode_string(UINT8 input);
static void win_chanscan_set_channel_no(T_NODE *t_node);
static void win_chanscan_set_search_param();
static void chanscan_set_frontend_handler(void);
static void win_chanscan_tuner_frontend(BOOL set_frontend2);
void drv_audio_gen_tonevoice(UINT8 level, UINT8 init)   //050629 modify the Aerial adjustment to 8 level ming yi
{
    deca_tone_voice((struct deca_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECA), level, init);
}
void drv_audio_stop_tonevoice()
{
    deca_stop_tone_voice((struct deca_device *) dev_get_by_type(NULL, HLD_DEV_TYPE_DECA));   //050629 modify the Aerial adjustment to 8 level ming yi
}
void get_ch_no(UINT32 ch_no, UINT16* unistr)
{
    UINT32 bandwidth = 0, start_freq = 0, end_freq = 0, channel_no = 0;
    BOOL bandtype = 0;
    UINT32 freq = 0;
    char str[30]={0};
    INT32 ret = 0;

    channel_param(( band_param*)&country_param, ch_no, &start_freq, &end_freq, &channel_no, &bandwidth, &bandtype);
    freq = start_freq+(bandwidth/2);
    ret = snprintf(str, 30, "CH%02d (%6d KHz)", (int)ch_no, (int)freq);
    if(0 == ret)
        ali_trace(&ret);
    com_asc_str2uni(str, unistr);
}

void get_frequency(UINT32 ch_no, UINT16* unistr)
{
    UINT32 bandwidth = 0, start_freq = 0, end_freq = 0, channel_no = 0;
    BOOL bandtype = 0;
    UINT32 freq = 0;
    char str[30]={0};
    INT32 ret = 0;

    channel_param(( band_param*)&country_param, ch_no, &start_freq, &end_freq, &channel_no, &bandwidth, &bandtype);
    freq = start_freq+(bandwidth/2);
    ret = snprintf(str,30, "%6d KHz", (int)freq);
    if(0 == ret)
        ali_trace(&ret);
    com_asc_str2uni(str, unistr);
}

static VACTION chanscan_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_ENTER:
        act = VACT_POP_UP;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT chanscan_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = VACT_PASS;
    UINT32 sel = 0;
    UINT8 b_id = 0;
    //char str[30] = {0};
    UINT16 * uni_str = NULL;
    //UINT8* pstr = NULL;
    //SYSTEM_DATA* sys_data = NULL;
    POBJECT_HEAD item = NULL;
    S_NODE s_node;
    //UINT16 cur_idx = 0;
    //COM_POP_LIST_TYPE_T list_type = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    //MULTISEL* msel = NULL;
    //UINT32 bandwidth=0,start_freq=0,end_freq=0,channel_no=0;
    //BOOL bandtype=0;
    //UINT32 frequency=0;
    POBJECT_HEAD submenu = NULL;

    MEMSET(&s_node, 0 ,sizeof(s_node));
    MEMSET(&rect, 0, sizeof(rect));
    MEMSET(&param, 0, sizeof(param));
    //sys_data = sys_data_get();
    sys_data_get();

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        if(b_id == SCAN_MODE_ID)
        {
            win_chanscan_load_setting();
            win_chanscan_set_mode_string(SCAN_MODE_IDX);
        }
        else if((b_id == SCAN_BAND_ID) && (SCAN_MODE_IDX == 0))
        {
            if(SCAN_BAND_IDX)
                ch_no = scan_chan_info.uhf_current_ch_no;
            else
                ch_no = scan_chan_info.vhf_current_ch_no;

            osd_draw_object((POBJECT_HEAD )&chanscan_item3,C_UPDATE_ALL);
        }
        //win_signal_refresh();
        api_stop_timer(&set_frontend);
        set_frontend = api_start_timer("SETANT",600,(OSAL_T_TIMER_FUNC_PTR)chanscan_set_frontend_handler);
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;

        if(b_id == CH_NO_ID)
        {
            if(SCAN_BAND_IDX)
                get_ch_no(ch_no, uni_str);
            else
                get_ch_no(ch_no, uni_str);
        }
        break;
    case EVN_UNKNOWN_ACTION:
    case EVN_KEY_GOT:
        unact = (VACTION)(param1>>16);

        if((unact == VACT_ENTER) && (b_id == START_ID))
        {
            win_chanscan_set_search_param();
            submenu = (POBJECT_HEAD)&g_win_search;
            if(osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
                menu_stack_push(submenu);
            ret = PROC_LOOP;
        }

        item = chanscan_items[b_id - 1];

        if(b_id == CH_NO_ID)
        {
            if(unact == VACT_ENTER)
            {
            }
            else if(unact == VACT_DECREASE)
            {
                if((ch_no == scan_chan_info.uhf_start_ch_no)&&(SCAN_BAND_IDX == 1))
                    ch_no = scan_chan_info.uhf_end_ch_no;
                else if((ch_no == scan_chan_info.vhf_start_ch_no)&&(SCAN_BAND_IDX == 0))
                    ch_no = scan_chan_info.vhf_end_ch_no;
                else
                {
                    //ChNo maybe 0, should judge here
                    if(ch_no)
                        ch_no--;
                }
            }
            else if(unact == VACT_INCREASE)
            {
                if((ch_no == scan_chan_info.uhf_end_ch_no)&&(SCAN_BAND_IDX == 1))
                    ch_no = scan_chan_info.uhf_start_ch_no;
                else if((ch_no == scan_chan_info.vhf_end_ch_no)&&(SCAN_BAND_IDX == 0))
                    ch_no = scan_chan_info.vhf_start_ch_no;
                else
                    ch_no++;
            }
            osd_track_object(item, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            api_stop_timer(&set_frontend);
            set_frontend = api_start_timer("SETANT",600,(OSAL_T_TIMER_FUNC_PTR)chanscan_set_frontend_handler);
            //win_signal_refresh();
            ret = PROC_LOOP;
        }
        //win_signal_refresh();
        break;
    default:
        break;
    }

    return ret;
}

static VACTION chanscan_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    switch(key)
    {
    case V_KEY_ENTER:
    //case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT chanscan_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 b_id = 0;
    VACTION unact = VACT_PASS;
    POBJECT_HEAD submenu = NULL;
    //UINT8 back_saved = 0;
    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
    case EVN_KEY_GOT:
        unact = (VACTION)(param1>>16);
        if((unact == VACT_ENTER) && (b_id == START_ID))
        {
            win_chanscan_tuner_frontend(TRUE);
            win_chanscan_set_search_param();
            submenu = (POBJECT_HEAD)&g_win_search;
#ifdef ANTENNA_INSTALL_BEEP
            if(tone_voice_init ==1)
            {
            #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
                api_audio_stop_tonevoice();
            #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
                api_audio_beep_stop();
            #endif
            tone_voice_init =0;
            }
#endif

            if(osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
                menu_stack_push(submenu);
            ret = PROC_LOOP;
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION chanscan_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        break;
    }

    return act;
}


static PRESULT chanscan_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT16 default_satidx = 0;
    //UINT32 sat_cnt = 0;
    //MULTISEL* msel = NULL;
    struct nim_device* nim_dev = NULL;
    UINT16 title_id = 0;

#ifdef ANTENNA_INSTALL_BEEP
    SYSTEM_DATA* sys_data = NULL;
    UINT8 level = 0,quality =0,lock =0;
    static UINT32 interval =0;

    sys_data = sys_data_get();
#endif


    switch(event)
    {
    case EVN_PRE_OPEN:
        if(param2 != MENU_OPEN_TYPE_STACK)
        {
            if((param2 & MENU_FOR_ISDBT) == MENU_FOR_ISDBT)
                current_frontend_is_isdbt = TRUE;
            else
                current_frontend_is_isdbt = FALSE;
        }
        if(current_frontend_is_isdbt)
            title_id = RS_ISDBT_CHAN_SCAN;
        else
            title_id = RS_DVBT_CHAN_SCAN;
            is_auto_scan=FALSE;//modify for channel scan mode show the title auto scan on T tuner 2011 11 25
        wincom_open_title(p_obj, title_id,0);
        switch_scan_mode_select(SCAN_MODE_IDX);
        win_chanscan_load_setting();

        osd_set_container_focus((CONTAINER*)p_obj,1);
        snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0),SND_DAC_MUTE,0);
        break;
    case EVN_POST_OPEN:
#ifdef ANTENNA_INSTALL_BEEP
            tone_voice_init = 0;
#endif
        win_chanscan_set_mode_string(SCAN_MODE_IDX);
        //win_signal_open(pObj);
        nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
        if( ((FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM)) && 
            (FRONTEND_TYPE_ISDBT == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM)))&& 
            ((board_frontend_is_isdbt(0) && current_frontend_is_isdbt) ||
             (!board_frontend_is_isdbt(0) && !current_frontend_is_isdbt)))
            cur_tuner_idx = 0;
        else
        {
            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
            if(((FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM)) && 
                (FRONTEND_TYPE_ISDBT == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM))) && 
                ((board_frontend_is_isdbt(1) && current_frontend_is_isdbt) ||
                 (!board_frontend_is_isdbt(1) && !current_frontend_is_isdbt)))
                cur_tuner_idx = 1;
        }
#ifdef SUPPORT_TWO_TUNER
        win_signal_open_ex(p_obj,p_obj->frame.u_left,p_obj->frame.u_top + 7 * (CON_H + CON_GAP) + 20);
#else
        win_signal_open_ex(p_obj,p_obj->frame.u_left,p_obj->frame.u_top + 7 * (CON_H + CON_GAP) + 20);
#endif

        win_chanscan_tuner_frontend(FALSE);
        win_signal_refresh();
        //win_signal_update();
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
#ifdef ANTENNA_INSTALL_BEEP
        if(sys_data->install_beep)
        {
        #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
            api_audio_stop_tonevoice();
        #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
            api_audio_beep_stop();
        #endif
        }
#endif
        break;
    case EVN_POST_CLOSE:
        win_signal_close();
        //api_nim_stop_autoscan();
        sys_data_save(1);
        //drvAudioStopTonevoice();
        snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0),SND_DAC_MUTE,1);
        break;
    case EVN_MSG_GOT:
        if(param1 == CTRL_MSG_SUBTYPE_STATUS_SIGNAL)
        {
#if defined(DVBT2_SUPPORT)
            lock    = (UINT8)(param2>>0);
            if(lock)
            {
                start = osal_get_tick();
            }
            else
            {
                //Auto retry to detect and lock the signal that it's either DVB-T or DVB-T2 signal.
                UINT32 timeout = 4000; //It shall not less then 4s.
                UINT32 now;
                now = osal_get_tick();
                if (now - start >= timeout)
                {
                    win_chanscan_tuner_frontend(FALSE);
                    start = osal_get_tick();
                    break;
                }
            }
#endif

            win_signal_refresh();
            //win_signal_update();
#ifdef ANTENNA_INSTALL_BEEP
            if(sys_data->install_beep)
            {
                level   = (UINT8)(param2>>16);
                quality = (UINT8)(param2>>8);
                lock    = (UINT8)(param2>>0);
            #if(defined(DVBT_BEE_TONE) && ANTENNA_INSTALL_BEEP == 1)
                if(quality<20)
                          level = 0;
                        else if(quality<40)
                            level = 1;
                        else if(quality<55)
                            level = 3;
                        else if(quality <70)
                            level = 5;
                        else
                            level = 7;
                api_audio_gen_tonevoice(level, tone_voice_init);
            #elif (defined(AUDIO_SPECIAL_EFFECT) && ANTENNA_INSTALL_BEEP == 2)
                if(lock)
                    level = 100;//100/quality;
                else
                    level = 1;//100/quality;
                if((!tone_voice_init) || (level!=interval))
                {
                    if(!tone_voice_init)
                        api_audio_beep_start(beep_bin_array,beep_bin_array_size);
                    interval = level;
                    api_audio_beep_set_interval(interval);
                }
            #endif
                tone_voice_init = 1;
            }
#endif
        }
        else if(param1 == CTRL_MSG_SUBTYPE_CMD_TP_TURN)
        {
            win_chanscan_tuner_frontend(FALSE);
        }
        break;
    default:
        break;
    }
    return ret;
}

//add judge here 2011-10-31
static void win_chanscan_scanband_setting()
{
    //add judge here 2011-10-31 for if the VHF/UHF have no channel, can't change Scan Band
    if(scan_chan_info.vhf_current_ch_no ==0 && scan_chan_info.uhf_current_ch_no !=0)
    {

        //if the VHF have no channel, don't let select Scan Band
        osd_set_attr(&chanscan_item2, C_ATTR_INACTIVE);
        osd_set_attr(&chanscan_txt2, C_ATTR_INACTIVE);
        osd_set_attr(&chanscan_sel2, C_ATTR_INACTIVE);
        SCAN_BAND_IDX=1;//set to UHF
    }
    else if(scan_chan_info.uhf_current_ch_no ==0 && scan_chan_info.vhf_current_ch_no !=0)
    {

        //if the UHF have no channel, don't let select Scan Band
        osd_set_attr(&chanscan_item2, C_ATTR_INACTIVE);
        osd_set_attr(&chanscan_txt2, C_ATTR_INACTIVE);
        osd_set_attr(&chanscan_sel2, C_ATTR_INACTIVE);
        SCAN_BAND_IDX=0;//set to VHF
    }
}

static void win_chanscan_load_setting()
{
    EDIT_FIELD    *edf = NULL;
    MULTISEL    *msel = NULL;
    T_NODE        t_node;
    P_NODE        p_node;
    UINT32        p_num = 0;
    UINT32        bandwidth=0,start_freq=0,end_freq=0,channel_no=0;
    BOOL        bandtype=0;
    UINT16        cur_chan = 0;
    //UINT8        str[16] = {0};
    SYSTEM_DATA*    sys_data = NULL;
    UINT8    u_cur_country_idx = 0;
    UINT16 sat_id = 0;
    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&p_node, 0x0, sizeof (p_node));
    sys_data = sys_data_get();

    u_cur_country_idx = sys_data->country;
    if(current_frontend_is_isdbt)
    {
        sat_id = get_frontend_satid(FRONTEND_TYPE_ISDBT, 1);
        get_isdbt_bandparam(u_cur_country_idx, (band_param *)&country_param);
    }
    else
    {
        sat_id = get_frontend_satid(FRONTEND_TYPE_T, 0);
        get_dvbt_bandparam(u_cur_country_idx, (band_param *)&country_param);
    }
    //get_default_bandparam(uCurCountryIdx, (Band_param *)&country_param);

    MEMSET(&scan_chan_info, 0, sizeof(scan_chan_info));
    init_ch_no_info(( band_param*)&country_param);

    //p_num = get_node_num(TYPE_PROG_NODE,NULL);
    MEMSET(&p_node, 0, sizeof(P_NODE));
    //p_num = get_prog_num(VIEW_ALL | TV_CHAN | RADIO_CHAN , 0);
    p_num = db_check_prog_exist(VIEW_ALL | PROG_TVRADIO_MODE , 0);//speed UI
    if (0 != p_num)
    {
        cur_chan = sys_data_get_cur_group_cur_mode_channel();
        get_prog_at(cur_chan,&p_node);
    }

    //change on 2011-10-28 shoud check p_num, if p_num is not zero, can use p_node to get t_node
    if(p_node.sat_id == sat_id && p_num)//MAX_SAT_NUM-1)
    {
        get_tp_by_id(p_node.tp_id,&t_node);
        win_chanscan_set_channel_no(&t_node);
        channel_param(( band_param*)&country_param,ch_no,&start_freq,&end_freq,&channel_no,&bandwidth,&bandtype);
        BANDWIDTH_IDX=((bandwidth/1000)-6);
    }
    else
    {
        ch_no=(country_param[0].band_type&0x007f);
        SCAN_BAND_IDX=((country_param[0].band_type&0x80)!=0?1:0);

        if(SCAN_BAND_IDX)
            ch_no = scan_chan_info.uhf_current_ch_no;
        else
            ch_no = scan_chan_info.vhf_current_ch_no;

        BANDWIDTH_IDX=((country_param[0].bandwidth/1000)-6);
    }

    //add judge here 2011-10-31 for avoid the VHF/UHF have no channel
    if(scan_chan_info.vhf_current_ch_no ==0 && scan_chan_info.uhf_current_ch_no !=0)
    {
        SCAN_BAND_IDX=1;//set to UHF
        ch_no = scan_chan_info.uhf_current_ch_no;
    }
    else if(scan_chan_info.uhf_current_ch_no ==0 && scan_chan_info.vhf_current_ch_no !=0)
    {
        SCAN_BAND_IDX=0;//set to VHF
        ch_no = scan_chan_info.vhf_current_ch_no;
    }

    t_node.band_type = SCAN_BAND_IDX;
    msel = &chanscan_sel2;
    osd_set_multisel_sel(msel, SCAN_BAND_IDX);

    edf = &chanscan_edt4;
    channel_param(( band_param*)&country_param, ch_no, &start_freq, &end_freq, &channel_no, &bandwidth, &bandtype);
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" KHz");
    osd_set_edit_field_content(edf, STRING_NUMBER, start_freq+(bandwidth/2));
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    t_node.bandwidth=BANDWIDTH_IDX;
    msel = &chanscan_sel5;
    osd_set_multisel_sel(msel, BANDWIDTH_IDX);
}

static void chanscan_set_frontend_handler(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,0,FALSE);
}

static void win_chanscan_tuner_frontend(BOOL set_frontend2)
{
    struct as_service_param param;
    UINT32 frequency=0,bandwidth=0,start_freq=0,end_freq=0,channel_no=0;
    struct nim_device *nim_dev1 = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct nim_device *nim_dev2 = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
    union ft_xpond xponder;
    static struct ft_frontend ft;
    //struct nim_device* nim_dev = NULL;
    UINT32 type1=0,type2=0;

    MEMSET(&param, 0, sizeof(struct as_service_param));
    MEMSET(&ft, 0, sizeof(struct ft_frontend));
    MEMSET(&xponder, 0, sizeof(union ft_xpond));
    if(NULL != nim_dev1)
        type1 = dev_get_device_sub_type(nim_dev1, HLD_DEV_TYPE_NIM);
    if(NULL != nim_dev2)
        type2 = dev_get_device_sub_type(nim_dev2, HLD_DEV_TYPE_NIM);
    if(SCAN_MODE_IDX==0)  //by channel
    {
        channel_param(( band_param*)&country_param,ch_no,&start_freq,&end_freq,&channel_no,&bandwidth,(BOOL*)&param.ft[0].t_param.band_type);
        frequency = start_freq+bandwidth/2;
    }
    else  //by frequency
    {
        win_chanscan_get_setting(&frequency);
        get_band_type(( band_param*)&country_param,&frequency,(BOOL*) &param.ft[0].t_param.band_type);

        if (BANDWIDTH_IDX==0)
            bandwidth=6000;
        else if (BANDWIDTH_IDX==1)
            bandwidth=7000;
        else if (BANDWIDTH_IDX==2)
            bandwidth=8000;

        if(current_frontend_is_isdbt)
            start_freq = frequency - bandwidth/2;
    }

    if(!current_frontend_is_isdbt)
    {
        //so for, because frequency of nim_m3101 only valid between 40000Khz-
        // 900000Khz, beyond the range nim_3101 do not change the frequency.
        // but we wan to get real time lock status, so we set a valid frequency
        // (40000+1) to change frequency.
        if ((frequency <= 40000) || (frequency >= 900000))
            frequency = 40001;
    }

    if(!current_frontend_is_isdbt)
    {
        ft.xpond.t_info.type = FRONTEND_TYPE_T;
        ft.xpond.t_info.frq = frequency;
        ft.xpond.t_info.band_width = bandwidth;
#if defined(DVBT2_SUPPORT)
        ft.xpond.t_info.usage_type = (UINT8)USAGE_TYPE_AERIALTUNE;
#else
        ft.xpond.t_info.usage_type = (UINT8)USAGE_TYPE_CHANSCAN;
#endif

        if(NULL != nim_dev1)
        {
            if(FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev1, HLD_DEV_TYPE_NIM) && !board_frontend_is_isdbt(0))
            {
//              frontend_tuning(nim_dev1, NULL, &xponder, 1);
                ft.nim = nim_dev1;
                uich_chg_aerial_signal_monitor(&ft);
            }
        }



        if((set_frontend2&&(NULL != nim_dev2))
         ||((type1==FRONTEND_TYPE_S&& type2==FRONTEND_TYPE_T)||(type1==FRONTEND_TYPE_T&& type2==FRONTEND_TYPE_S)))
        {
            //frontend_set_nim(nim_dev2, NULL, &xponder, 1);
            if(FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev2, HLD_DEV_TYPE_NIM) && !board_frontend_is_isdbt(1))
            {
//              frontend_tuning(nim_dev2, NULL, &xponder, 1);
                ft.nim = nim_dev2;
                uich_chg_aerial_signal_monitor(&ft);
            }
        }

    }
    else
    {
        ft.xpond.t_info.type = FRONTEND_TYPE_ISDBT;
        ft.xpond.t_info.frq = frequency;
        ft.xpond.t_info.band_width = bandwidth; // KHz
        ft.xpond.t_info.usage_type = (UINT8)USAGE_TYPE_CHANSCAN;

        if((FRONTEND_TYPE_ISDBT == dev_get_device_sub_type(nim_dev1, HLD_DEV_TYPE_NIM)) && board_frontend_is_isdbt(0))
        {
            ft.xpond.t_info.type = FRONTEND_TYPE_ISDBT;
            ft.nim = nim_dev1;
            uich_chg_aerial_signal_monitor(&ft);
        }

        if((set_frontend2&&(NULL != nim_dev2))
         ||(((type1==FRONTEND_TYPE_S)&& ((type2==FRONTEND_TYPE_T) || (type2==FRONTEND_TYPE_ISDBT)))
            ||(((type1==FRONTEND_TYPE_T)||(type1==FRONTEND_TYPE_ISDBT))&& (type2==FRONTEND_TYPE_S))))
        {
            if(FRONTEND_TYPE_ISDBT== dev_get_device_sub_type(nim_dev2, HLD_DEV_TYPE_NIM) && board_frontend_is_isdbt(1))
            {
                ft.nim = nim_dev2;
                uich_chg_aerial_signal_monitor(&ft);
            }
        }
    }

#if defined(DVBT2_SUPPORT)
    start = osal_get_tick();
#endif

}

static void win_chanscan_set_search_param()
{
    //UINT32  search_mode = 0;
    //UINT32 prog_type = 0;
    struct as_service_param param;
    UINT32 addr = 0, len = 0;
    UINT32 frequency=0,bandwidth=0,start_freq=0,end_freq=0,channel_no=0;
    T_NODE t_node;
    INT32 ret = 0;
    T_NODE temp_t_node;
    UINT16 sat_id = 0;

    MEMSET(&param, 0x0, sizeof (param));

    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&temp_t_node, 0x0, sizeof (temp_t_node));

    if(current_frontend_is_isdbt)
    {
        sat_id = get_frontend_satid(FRONTEND_TYPE_ISDBT, 1);
    	t_node.ft_type = FRONTEND_TYPE_ISDBT;
    }
    else
    {
        sat_id = get_frontend_satid(FRONTEND_TYPE_T, 0);
	    t_node.ft_type = FRONTEND_TYPE_T;
    }
    t_node.sat_id = sat_id;// MAX_SAT_NUM-1;

    if(SCAN_MODE_IDX==0)  //by channel
    {
        channel_param(( band_param*)&country_param,ch_no,&start_freq,&end_freq,&channel_no,&bandwidth,(BOOL*)&param.ft[0].t_param.band_type);
        t_node.frq = start_freq+bandwidth/2;
        t_node.bandwidth = bandwidth/1000;
        t_node.band_type = SCAN_BAND_IDX;

        param.ft[0].t_param.band_type=SCAN_BAND_IDX;
    }
    else  //by frequency
    {
        win_chanscan_get_setting(&frequency);
        get_band_type(( band_param*)&country_param,&frequency,(BOOL*) &param.ft[0].t_param.band_type);

        if (BANDWIDTH_IDX==0)
            bandwidth=6000;
        else if (BANDWIDTH_IDX==1)
            bandwidth=7000;
        else if (BANDWIDTH_IDX==2)
            bandwidth=8000;

        t_node.frq = frequency;
        t_node.bandwidth = bandwidth/1000;
        t_node.band_type = param.ft[0].t_param.band_type;
    }

    recreate_tp_view(VIEW_SINGLE_SAT, 1);
    ret = lookup_node(TYPE_TP_NODE, &t_node, t_node.sat_id);
    if (ret != SUCCESS)
    {
        add_node(TYPE_TP_NODE, t_node.sat_id, &t_node);
        update_data();
        libc_printf("add a new node\n");
    }

#if defined(_MHEG5_V20_ENABLE_)
    param.as_prog_attr = P_SEARCH_ALL|P_SEARCH_DATA;
#else
    param.as_prog_attr = P_SEARCH_ALL;
#endif

    if(NETWORK_SRH_IDX == 1)
        param.as_prog_attr |= P_SEARCH_NIT;

    param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;

    param.as_method = AS_METHOD_FREQ_BW;
    if(current_frontend_is_isdbt)
    {
    	param.as_frontend_type = FRONTEND_TYPE_ISDBT;
    }
    else
    {
    	param.as_frontend_type = FRONTEND_TYPE_T;
    }
    param.as_sat_cnt = 1;
    param.sat_ids[0] = sat_id;// MAX_SAT_NUM-1;//1;

    //param.as_sat_id = 1;

    param.as_handler = NULL;

    param.as_from = t_node.tp_id;
    param.as_to = 0;

    //param.ft[0].t_param.bandwidth = bandwidth;
   // param.ft_count = 1;

    // set param
    win_search_set_param(&param);

    update_data();

    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
    addr &= 0x8fffffff;
    len = __MM_AUTOSCAN_DB_BUFFER_LEN;

    db_search_init((UINT8 *)addr, len);
}


/* switch Scan Mode by Channel or by Frequency */
static void switch_scan_mode_select(BOOL select)
{
    if(select) /* By Frequency */
    {
        if( !osd_check_attr(&chanscan_item2, C_ATTR_INACTIVE))
        {
            osd_set_attr(&chanscan_item2, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_txt2, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_sel2, C_ATTR_INACTIVE);
        }
        if( !osd_check_attr(&chanscan_item3, C_ATTR_INACTIVE))
        {
            osd_set_attr(&chanscan_item3, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_txt3, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_sel3, C_ATTR_INACTIVE);
        }
        if( !osd_check_attr(&chanscan_item4, C_ATTR_ACTIVE))
        {
            osd_set_attr(&chanscan_item4, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_txt4, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_edt4, C_ATTR_ACTIVE);
        }
#if (FREQ_SETTING_SUPPORT)
        if( !osd_check_attr(&chanscan_item5, C_ATTR_ACTIVE))
        {
            osd_set_attr(&chanscan_item5, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_txt5, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_sel5, C_ATTR_ACTIVE);
        }
#endif
    }
    else  /* By Channel */
    {
        if( !osd_check_attr(&chanscan_item2, C_ATTR_ACTIVE))
        {
            osd_set_attr(&chanscan_item2, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_txt2, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_sel2, C_ATTR_ACTIVE);
        }
        if( !osd_check_attr(&chanscan_item3, C_ATTR_ACTIVE))
        {
            osd_set_attr(&chanscan_item3, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_txt3, C_ATTR_ACTIVE);
            osd_set_attr(&chanscan_sel3, C_ATTR_ACTIVE);
        }
        if( !osd_check_attr(&chanscan_item4, C_ATTR_INACTIVE))
        {
            osd_set_attr(&chanscan_item4, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_txt4, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_edt4, C_ATTR_INACTIVE);
        }
#if (FREQ_SETTING_SUPPORT)
        if( !osd_check_attr(&chanscan_item5, C_ATTR_INACTIVE))
        {
            osd_set_attr(&chanscan_item5, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_txt5, C_ATTR_INACTIVE);
            osd_set_attr(&chanscan_sel5, C_ATTR_INACTIVE);
        }
#endif
    }
}

void win_chanscan_set_mode_string(UINT8 input)
{
    switch_scan_mode_select((input==0)? 0:1);
    //add on 2011-10-31
  win_chanscan_scanband_setting();
    osd_track_object((POBJECT_HEAD)&chanscan_item1,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&chanscan_item2,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&chanscan_item3,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&chanscan_item4,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&chanscan_item5,C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD )&chanscan_item6,C_UPDATE_ALL);
}

void init_ch_no_info(band_param *buf)
{
    UINT16 j=0;
    while(buf[j].start_freq!= 0)
       {
           if (j==0)
        {
            if((((buf[j].band_type & 0x0080)==0x0000)&&((buf[j+1].band_type & 0x0080)==0x0080)))
            {
                scan_chan_info.vhf_start_ch_no=scan_chan_info.vhf_current_ch_no=(buf[j].band_type & 0x7f);
                scan_chan_info.vhf_end_ch_no=((buf[j].band_type&0x7f00)>>8);
                scan_chan_info.uhf_start_ch_no=scan_chan_info.uhf_current_ch_no=(buf[j+1].band_type&0x7f);
            }
            else if((((buf[j].band_type & 0x0080)==0x0000)&&(buf[j+1].start_freq ==0x0000)))//one VHF
            {
                scan_chan_info.vhf_start_ch_no=scan_chan_info.vhf_current_ch_no=(buf[j].band_type & 0x7f);
                scan_chan_info.vhf_end_ch_no=((buf[j].band_type&0x7f00)>>8);
            }
            else if((((buf[j].band_type & 0x0080)==0x0080)&&(buf[j+1].start_freq ==0x0000)))//one UHF
            {
                scan_chan_info.uhf_start_ch_no=scan_chan_info.uhf_current_ch_no=(buf[j].band_type & 0x7f);
                scan_chan_info.uhf_end_ch_no=((buf[j].band_type&0x7f00)>>8);
            }
            else if((buf[j].band_type & 0x0080)==0x0080)
                scan_chan_info.uhf_start_ch_no=scan_chan_info.uhf_current_ch_no=(buf[j].band_type & 0x7f);
            else
                scan_chan_info.vhf_start_ch_no=scan_chan_info.vhf_current_ch_no=(buf[j].band_type & 0x7f);
        }
        else if((((buf[j].band_type & 0x0080)==0x0000)&&((buf[j+1].band_type & 0x0080)==0x0080)))
        {
            scan_chan_info.vhf_end_ch_no=((buf[j].band_type&0x7f00)>>8);
            scan_chan_info.uhf_start_ch_no=scan_chan_info.uhf_current_ch_no=(buf[j+1].band_type&0x7f);
        }
        else if((((buf[j].band_type & 0x0080)==0x0080)&&(buf[j+1].start_freq ==0x0000)))//set UHF
        {
            scan_chan_info.uhf_end_ch_no=((buf[j].band_type&0x7f00)>>8);
        }
        else if((((buf[j].band_type & 0x0080)==0x0000)&&(buf[j+1].start_freq ==0x0000)))//all VHF
        {
            scan_chan_info.vhf_end_ch_no=((buf[j].band_type&0x7f00)>>8);
        }
            j++;

       }

    scan_chan_info.vhf_current_ch_no=scan_chan_info.vhf_start_ch_no;
    scan_chan_info.uhf_current_ch_no=scan_chan_info.uhf_start_ch_no;
}

void channel_param(band_param *buf,UINT32 chan_no,UINT32 *start_freq,UINT32 *end_freq,UINT32 *channel_no,UINT32 *bandwidth,BOOL *band_type)
{
    UINT16 j=0;
    while(chan_no >((buf[j].band_type&0x7f00)>>8))
    {
        j++;
    }
    *start_freq=(buf[j].start_freq+(buf[j].bandwidth*(chan_no-(buf[j].band_type&0x007f))));
    *end_freq=*start_freq+buf[j].bandwidth;
    *bandwidth=buf[j].bandwidth;
    *band_type=((buf[j].bandwidth&0x0080)==0x0080?1:0);
    *channel_no=chan_no;
}

void get_band_type(band_param *buf,UINT32* frequency,BOOL *band_type)
{
    UINT32 fi=0;
     while(buf[fi].start_freq!=0)
    {
        if((*frequency<=buf[fi].end_freq)&&(*frequency>=buf[fi].start_freq))
        {
            *band_type=((buf[fi].band_type&0x80)==0x80?1:0);
            break;
        }
        else
        {
            if(fi==0)
            {
                if(*frequency<buf[0].start_freq)
                {
                    *band_type=((buf[0].band_type&0x80)==0x80?1:0);
                    break;
                }
            }
            else if((*frequency>buf[fi-1].end_freq)&&(*frequency<buf[fi].start_freq))
            {
                *band_type=((buf[fi-1].band_type&0x80)==0x80?1:0);
                break;
            }
            else
                *band_type=1;
        }
        fi++;
    }
}

void win_chanscan_get_setting(UINT32* freq)
{
    EDIT_FIELD    *edf = NULL;

    edf = &chanscan_edt4;
    osd_get_edit_field_int_value(edf, freq);
}

static VACTION chanscan_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
        case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
        case V_KEY_8:    case V_KEY_9:
            act = key - V_KEY_0 + VACT_NUM_0;
            break;
        case V_KEY_LEFT:
            act = VACT_EDIT_LEFT;
            break;
        case V_KEY_RIGHT:
            act = VACT_EDIT_RIGHT;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;

}

static PRESULT chanscan_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT32 frequency=0;
    switch(event)
    {
        case EVN_PRE_CHANGE:
            break;
        case EVN_POST_CHANGE:
            win_chanscan_get_setting(&frequency);
            if(frequency>(UINT32)((BANDWIDTH_IDX+6)*1000))
            {
                api_stop_timer(&set_frontend);
                set_frontend = api_start_timer("SETANT",500,(OSAL_T_TIMER_FUNC_PTR)chanscan_set_frontend_handler);
                //win_signal_refresh();
            }
            break;
        default:
            break;
    }
    return ret;
}

static void win_chanscan_set_channel_no(T_NODE *t_node)
{
    UINT32 channel_number=0,channel_number_frequency=0,fi=0;;

     while((t_node->frq !=0)&&(country_param[fi].start_freq!=0))
    {
        if((t_node->frq<=country_param[fi].end_freq)&&(t_node->frq>=country_param[fi].start_freq))
        {
            channel_number_frequency+=(t_node->frq);
            channel_number=(country_param[fi].band_type&0x7F);
            channel_number+=((channel_number_frequency-country_param[fi].start_freq)/country_param[fi].bandwidth);
            SCAN_BAND_IDX=((country_param[fi].band_type&0x80)!=0?1:0);
            break;
        }
        fi++;
    }
    if(channel_number)
    {
        if(SCAN_BAND_IDX)
            ch_no=scan_chan_info.uhf_current_ch_no=channel_number;
        else
            ch_no=scan_chan_info.vhf_current_ch_no=channel_number;
    }
}

