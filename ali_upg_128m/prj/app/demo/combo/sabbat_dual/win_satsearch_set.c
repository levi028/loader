/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_satsearch_set.c
*
*    Description: The function for use can modify sat search environment setting
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
#endif

#include <hld/decv/decv.h>

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

#include "win_satsearch_set.h"
#include "win_mainmenu_submenu.h"
#include "win_search.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_sat_srchset;

extern CONTAINER    satsrch_item1;        /* SAT */
extern CONTAINER    satsrch_item2;        /* FTA Only*/
extern CONTAINER    satsrch_item3;        /* Channel( TV + Radio / TV / Radio ) */
extern CONTAINER    satsrch_item4;    /* NIT (On/Off*/
extern CONTAINER    satsrch_item5;        /* Mode (Auto scan / Preset scan ) */
extern CONTAINER    satsrch_item6;        /* lnb id for 2-tuner */
extern CONTAINER    satsrch_item7;        /* Start */


extern TEXT_FIELD   satsrch_txt1;
extern TEXT_FIELD   satsrch_txt2;
extern TEXT_FIELD   satsrch_txt3;
extern TEXT_FIELD   satsrch_txt4;
extern TEXT_FIELD   satsrch_txt5;
extern TEXT_FIELD   satsrch_txt6;
extern TEXT_FIELD   satsrch_txt6_note;
extern TEXT_FIELD   satsrch_txt7;

extern TEXT_FIELD     satsrch_line1;
extern TEXT_FIELD     satsrch_line2;
extern TEXT_FIELD     satsrch_line3;
extern TEXT_FIELD     satsrch_line4;
extern TEXT_FIELD     satsrch_line5;
extern TEXT_FIELD     satsrch_line6;
extern TEXT_FIELD     satsrch_line7;

extern TEXT_FIELD   satsrch_multisats_back;
extern TEXT_FIELD   satsrch_multisats_name;

extern MULTISEL       satsrch_sel1;
extern MULTISEL    satsrch_sel2;
extern MULTISEL    satsrch_sel3;
extern MULTISEL    satsrch_sel4;
extern MULTISEL    satsrch_sel5;

extern TEXT_FIELD     satsrch_split;

static VACTION satsrch_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satsrch_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION satsrch_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satsrch_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION satsrch_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satsrch_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define CON_HLSUB_IDX    WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define SELSATS_BG_SH_IDX    WSTL_BUTTON_01_HD
#define SELSATS_FG_SH_IDX    WSTL_BUTTON_07_HD

#define SPLIT_LINE_SH    WSTL_LINE_MENU_HD

#ifndef SD_UI
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
#define TXT_W          300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-320)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define SELSATS_L    (W_L+20)
#define SELSATS_T    (W_T + 320)
#define SELSATS_W     (W_W - 60)
#define SELSATS_H     (W_H - 360)

#define SELSAT_L0     (SELSATS_L + 10)
#define SELSAT_L1     (SELSATS_L + SELSATS_W/2 + 10)
#define SELSAT_T      (SELSATS_T + 10)
#define SELSAT_W      (SELSATS_W/2 - 10)
#define SELSAT_H    38

#define SELSAT_ROW_CNT 2

#define SPLIT_LINE_L    CON_L
#define SPLIT_LINE_T      (CON_T + 7 * (CON_H + CON_GAP) )
#define SPLIT_LINE_W    CON_W
#define SPLIT_LINE_H    2
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 30)
#define CON_H        32//40
#define CON_GAP        3

#define TXT_L_OF      10
#define TXT_W          180//300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-200)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define SELSATS_L    (W_L+20)
#define SELSATS_T    (W_T + 240)
#define SELSATS_W     (W_W - 60)
#define SELSATS_H     (W_H - 360)

#define SELSAT_L0     (SELSATS_L + 10)
#define SELSAT_L1     (SELSATS_L + SELSATS_W/2 + 10)
#define SELSAT_T      (SELSATS_T + 5)
#define SELSAT_W      (SELSATS_W/2 - 10)
#define SELSAT_H    32

#define SELSAT_ROW_CNT 2

#define SPLIT_LINE_L    CON_L
#define SPLIT_LINE_T      (CON_T + 7 * (CON_H + CON_GAP) )
#define SPLIT_LINE_W    CON_W
#define SPLIT_LINE_H    2

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    satsrch_item_keymap,satsrch_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_NOTE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    satsrch_item_sel_keymap,satsrch_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 22,0,style,ptabl,cur,cnt)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_NOTE(root,var_con,nxt_obj,var_txt,var_note, var_line, ID,idu,idd,\
                        l,t,w,h,res_id,note_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_note,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_TXT_NOTE(&var_con,var_note,NULL/*&varLine*/,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,note_id)    \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MULTI_BACK(root,var_txt,nxt_obj)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, SELSATS_L,SELSATS_T,SELSATS_W,SELSATS_H, SELSATS_BG_SH_IDX,SELSATS_BG_SH_IDX,SELSATS_BG_SH_IDX,SELSATS_BG_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_VCENTER | C_ALIGN_TOP, 0,0,0,NULL)


#define LDEF_MULTI_TXT(root,var_txt)        \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, SELSATS_FG_SH_IDX,SELSATS_FG_SH_IDX,SELSATS_FG_SH_IDX,SELSATS_FG_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[0])

#define LDEF_MULTI_LINE(root, var_txt, nxt_obj, l, t, w, h, str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, l, t, w, h, SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    satsrch_keymap,satsrch_callback,  \
    nxt_obj, focus_id,0)


#define WIN g_win_sat_srchset

extern UINT16 scan_channel_type_ids[];
extern UINT16 scan_mode_ids[];

LDEF_MENU_ITEM_SEL(WIN,satsrch_item1,&satsrch_item2,satsrch_txt1,satsrch_sel1,satsrch_line1, 1, 7, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INFO_SATELLITE, STRING_PROC, 0, 0, NULL)

LDEF_MENU_ITEM_SEL(WIN,satsrch_item2,&satsrch_item3,satsrch_txt2,satsrch_sel2, satsrch_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, FATONLY_DEFAULT, 2, yesno_ids)

LDEF_MENU_ITEM_SEL(WIN,satsrch_item3,&satsrch_item4,satsrch_txt3,satsrch_sel3, satsrch_line3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_INSTALLATION_SCAN_CHANNEL, STRING_ID, 0, 3, scan_channel_type_ids)

LDEF_MENU_ITEM_SEL(WIN,satsrch_item4,&satsrch_item5,satsrch_txt4,satsrch_sel4, satsrch_line4, 4, 3, 5, \
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_INSTALLATION_NIT_SCAN, STRING_ID, 0, 2, yesno_ids)

LDEF_MENU_ITEM_SEL(WIN,satsrch_item5,&satsrch_item6,satsrch_txt5,satsrch_sel5, satsrch_line5, 5, 4, 6, \
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_INSTALLATION_SCAN_MODE, STRING_ID, 0, 3, scan_mode_ids)

LDEF_MENU_ITEM_NOTE(WIN,satsrch_item6,&satsrch_item7,satsrch_txt6,satsrch_txt6_note, satsrch_line6, 6, 5, 7, \
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, RS_TUNER, 0)

LDEF_MENU_ITEM_TXT(WIN,satsrch_item7,NULL,satsrch_txt7, satsrch_line7, 7,6,1,\
        CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H,RS_COMMON_SEARCH)

LDEF_MULTI_LINE(WIN, satsrch_split, NULL, SPLIT_LINE_L, SPLIT_LINE_T, SPLIT_LINE_W, SPLIT_LINE_H, NULL)

//LDEF_MULTI_BACK(WIN,satsrch_multisats_back,NULL)
LDEF_MULTI_TXT(WIN,satsrch_multisats_name)

LDEF_WIN(WIN,&satsrch_item1,W_L, W_T, W_W, W_H,1)

#define SAT_ID        1
#define FTA_ID        2
#define CHAN_ID        3
#define NIT_ID        4
#define MODE_ID        5
#define START_ID    7
#define SATSRCH_SAT_CNT	(satsrch_sel1.n_count)
#define SATSRCH_SAT_IDX	(satsrch_sel1.n_sel)
#define SATSRCH_FTA_IDX	(satsrch_sel2.n_sel)
#define SATSRCH_CHAN_IDX	(satsrch_sel3.n_sel)
#define SATSRCH_NIT_IDX	(satsrch_sel4.n_sel)
#define SATSRCH_MODE_IDX	(satsrch_sel5.n_sel)
#define VACT_POP_UP	(VACT_PASS + 1)
#ifdef NIM_S3501_ASCAN_TOOLS
#define VACT_ASCAN_EDIT	(VACT_PASS + 2)
#endif
#define AS_FREQ_MIN     950
#define AS_FREQ_MAX     2200
/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

UINT16 scan_channel_type_ids[] =
{
    RS_INSTALLATION_SCAN_CHANNEL_TVRADIO,
    RS_INFO_TV,//RS_INSTALLATION_SCAN_CHANNEL_TV,
    RS_INSTALLATION_SCAN_CHANNEL_RADIO,
};

UINT16 scan_mode_ids[] =
{
    RS_INSTALLATION_AUTO_SCAN,
    RS_INSTALLATION_PRESET_SCAN,
    RS_INSTALLATION_ACCURATE_SCAN,
};

typedef enum
{
    IDX_SCAN_MODE_AUTO_SCAN = 0,
    IDX_SCAN_MODE_PRESET_SCAN ,
    IDX_SCAN_MODE_ACCURATE_SCAN ,
}scan_mode_index_t;

MULTISEL* satsrch_sels[] =
{
    &satsrch_sel1,
    &satsrch_sel2,
    &satsrch_sel3,
    &satsrch_sel4,
};

UINT16 sat_ids[MAX_SAT_NUM]={0};
UINT8  sat_select[MAX_SAT_NUM]={0};

static UINT8  search_type = 0;


static void win_satsrch_set_drawselectsats(void);
static void win_satsrch_set_search_param(void);
static void win_satsrch_set_nitactive(BOOL update);



UINT8 win_satsearch_set_get_search_type(void)
{
    return search_type;
}
static void satsrch_set_item_sel_post_change(POBJECT_HEAD pobj, UINT32 param1)
{
    UINT8 bid= 0;
    UINT32 sel= 0;
    SYSTEM_DATA *sys_data = NULL;
    bid = osd_get_obj_id(pobj);
    sys_data = sys_data_get();
    sel = param1;
    switch(bid)
    {
		case FTA_ID:
        		sys_data->satsrch_search_mode= sel;
			 break;
		case CHAN_ID:
       			sys_data->satsrch_prog_type = sel;
			 break;
		case NIT_ID:
				sys_data->satsrch_nit_search = sel;
			 break;
		case MODE_ID:
				sys_data->satsrch_scan_mode = sel;
			 break;
    }
}
static void win_satsrch_set_load_setting(void)
{
	//MULTISEL *msel = NULL;
	SYSTEM_DATA *sys_data = NULL;
	CONTAINER* win =&g_win_sat_srchset;
	UINT8 action = 0;
	sys_data = sys_data_get();
	SATSRCH_FTA_IDX = sys_data->satsrch_search_mode;
	SATSRCH_CHAN_IDX = sys_data->satsrch_prog_type;
 	SATSRCH_NIT_IDX	= sys_data->satsrch_nit_search;
 	SATSRCH_MODE_IDX = sys_data->satsrch_scan_mode;
	action = (SATSRCH_MODE_IDX == IDX_SCAN_MODE_PRESET_SCAN)? C_ATTR_ACTIVE : C_ATTR_INACTIVE;
	if ((action == C_ATTR_INACTIVE)&&(NIT_ID == osd_get_container_focus(win)))
	{
		osd_set_container_focus(win, MODE_ID);
	}
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION satsrch_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
#ifdef SELECT_SAT_ONLY
        if(FALSE == win_search_is_multi_sat_search())
#endif
        act = VACT_POP_UP;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}


static PRESULT satsrch_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	VACTION unact = VACT_PASS;
	UINT32 sel = 0;
	UINT8 b_id =0;
 	//char str[10]={0};
    UINT16 * uni_str = NULL;
    //UINT8* pstr = NULL;
	UINT16 cur_idx =0;
	COM_POP_LIST_TYPE_T list_type = 0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
	UINT16 i=0,n = 0;
    S_NODE s_node;
	UINT32 flag = 0;

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&param, 0 ,sizeof(param));
    MEMSET(&s_node, 0 ,sizeof(s_node));
    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        if(b_id == MODE_ID)
            win_satsrch_set_nitactive(TRUE);
        if(b_id == SAT_ID)
        {
            get_sat_at(sel,SET_SELECTED,&s_node);
            osd_set_text_field_content(&satsrch_txt6_note , STRING_ID, (s_node.tuner1_valid ? RS_LNB1 : RS_LNB2));
            osd_draw_object((POBJECT_HEAD)&satsrch_txt6_note, C_UPDATE_ALL);
        }
		satsrch_set_item_sel_post_change(p_obj,param1);
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        if(b_id == SAT_ID)
        {
            get_tuner_sat_name(2,sel,uni_str);
            sys_data_set_cur_satidx(sel);
        }
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact ==  VACT_POP_UP)
        {
            osd_set_rect2(&rect,&p_obj->frame);
            switch(b_id)
            {
            case SAT_ID:
                rect.u_left -= 80;
                rect.u_width += 80;
                list_type     = POP_LIST_TYPE_SAT;
                rect.u_height = 300;
                param.id     = TUNER_EITHER;
                param.cur     = SATSRCH_SAT_IDX;
                if(FALSE == win_search_is_multi_sat_search())
                    param.selecttype = POP_LIST_SINGLESELECT;
                else
                {
                    MEMSET(sat_select,0,sizeof(sat_select));
                    n = get_tuner_sat_cnt(TUNER_EITHER);
                    if(n == 0)
                    {
                        list_type = 0xFF;
                        break;
                    }

                    for(i=0;i<n;i++)
                    {
                        get_tuner_sat(TUNER_EITHER,i,&s_node);
                        if(s_node.selected_flag)
                            sat_select[i] = 1;
                    }
#if (defined(SUPPORT_TWO_TUNER) || defined(SELECT_SAT_ONLY))
                    param.selecttype = POP_LIST_MULTISELECT;
#else
                    param.selecttype = POP_LIST_SINGLESELECT;
#endif
                    param.select_array = sat_select;
                }
                break;
            default:
                list_type = 0xFF;
            }

            if(list_type == 0xFF)
                break;

            cur_idx = win_com_open_sub_list(list_type,&rect,&param);
            if(FALSE == win_search_is_multi_sat_search())
            {
				if((cur_idx == LIST_INVALID_SEL_IDX) || (cur_idx == param.cur))
                    break;
            }
            else
            {
                /* Check the select satellite */

#if (defined(SUPPORT_TWO_TUNER) || defined(SELECT_SAT_ONLY))
                for(i=0;i<n;i++)
                {
                    if(win_comlist_ext_check_item_sel(i))
                        flag = 1;
                    else
                        flag = 0;
                    get_tuner_sat(TUNER_EITHER,i,&s_node);
                    if(s_node.selected_flag != flag)
                    {
                        s_node.selected_flag = flag;
                        modify_sat(s_node.sat_id, &s_node);
                    }
                }
                update_data(TYPE_SAT_NODE);
#endif
            }

            if(FALSE == win_search_is_multi_sat_search())
                SATSRCH_SAT_IDX = cur_idx;
            get_sat_at(cur_idx,SET_SELECTED,&s_node);
            osd_set_text_field_content(&satsrch_txt6_note , STRING_ID, (s_node.tuner1_valid ? RS_LNB1 : RS_LNB2));
            osd_draw_object((POBJECT_HEAD)&satsrch_txt6_note, C_UPDATE_ALL);
            osd_track_object((POBJECT_HEAD)&satsrch_item1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            if(win_search_is_multi_sat_search())
            {
                /* TODO :
                    Draw the select satellites name */
                    win_satsrch_set_drawselectsats();
            }


        }
		break;
    default:
        break;
    }

    return ret;

}


static VACTION satsrch_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;
    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    #ifdef NIM_S3501_ASCAN_TOOLS
    case V_KEY_RED:
        act = VACT_ASCAN_EDIT;
        break;
    #endif
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT satsrch_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8 b_id = 0;
	VACTION unact = VACT_PASS;
	POBJECT_HEAD submenu = NULL;
	UINT8 back_saved = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
		if((unact == VACT_ENTER) && (b_id == START_ID))
        {

			if((win_search_is_multi_sat_search()) && (get_tuner_sat_cnt(TUNER_EITHER_SELECT) == 0))
            {
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg(NULL, NULL, RS_DISPLAY_NO_SATELLITE);
                win_compopup_open_ext(&back_saved);
                ret = PROC_LOOP;
                break;
            }
            win_satsrch_set_search_param();
            submenu = (POBJECT_HEAD)&g_win2_search;
            if(osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
                menu_stack_push(submenu);

            ret = PROC_LOOP;
        }
        #ifdef NIM_S3501_ASCAN_TOOLS
        if(unact == VACT_ASCAN_EDIT     && b_id == START_ID)
        {
            win_ase_open();
            //ret = ret = PROC_LOOP;
            osd_track_object( (POBJECT_HEAD)&g_win_sat_srchset, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }

        #endif

		break;
    default:
        break;
    }

    return ret;
}

static VACTION satsrch_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT satsrch_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	MULTISEL* msel = NULL;
	UINT16 title_id = 0,sat_idx = 0,sat_cnt = 0;
	//OSD_RECT rect;
	//POBJECT_HEAD lastitem = NULL;
    SYSTEM_DATA* sys_data = sys_data_get();
	__MAYBE_UNUSED__ UINT16 i = 0;
    __MAYBE_UNUSED__ UINT16 start_pos = 0;
	__MAYBE_UNUSED__ UINT8 flag = 0;
    S_NODE s_node;
	//char asc_name[60]={0};

    MEMSET(&s_node, 0 ,sizeof(s_node));
  //  UINT16 default_sat_idx;

    switch(event)
    {
    case EVN_PRE_OPEN:
        sat_idx = 0;
        if(FALSE == win_search_is_multi_sat_search())
        {
            if(sys_data_get_cur_satidx() == (UINT16)(~0))
                sat_idx = win_load_default_satidx();
            else
                sat_idx = sys_data_get_cur_satidx();
        }

        recreate_sat_view(VIEW_ALL, 0);
		win_satsrch_set_load_setting();

#if (defined(SUPPORT_TWO_TUNER) || !defined(SELECT_SAT_ONLY))
        start_pos = get_frontend_satid(FRONTEND_TYPE_S, 0) -1;
        for(i=start_pos; i<get_sat_num(VIEW_ALL)-start_pos; i++)
        {
            get_sat_at((UINT16)i,VIEW_ALL,&s_node);
         #if 0
            com_uni_str_to_asc(s_node.sat_name, asc_name);
            if((STRCMP(asc_name, "ISDBT") == 0) || (STRCMP(asc_name, "DVBT") == 0) || (STRCMP(asc_name, "DVBC") == 0))
                continue;
         #endif
            flag = s_node.tuner1_valid | s_node.tuner2_valid;

            if(s_node.selected_flag != flag)
            {
                s_node.selected_flag = flag;
                modify_sat(s_node.sat_id, &s_node);
            }
        }
#endif

        msel = satsrch_sels[SAT_ID - 1];
        sat_cnt = get_tuner_sat_cnt(TUNER_EITHER_SELECT);
        osd_set_multisel_count(msel, sat_cnt);
        if(sat_idx >= sat_cnt)
        {
            sat_idx = 0;
        }
        osd_set_multisel_sel(msel,sat_idx);
        if(win_search_is_multi_sat_search())
            title_id = RS_INSTALLATION_MULTI_SEARCH;
        else
            title_id = RS_INSTALLATION_SINGLE_SEARCH;
        wincom_open_title(p_obj,title_id,0);

        /*lastitem = (POBJECT_HEAD)&satsrch_item7;
        if(single_multi_srch_flag)
            OSD_SetObjpNext(lastitem, &satsrch_multisats_back);
            //OSD_SetObjpNext(lastitem, NULL);
        else
            OSD_SetObjpNext(lastitem, NULL);*/

        win_satsrch_set_nitactive(FALSE);
        set_container_active(&satsrch_item6, C_ATTR_INACTIVE);
        get_sat_at(sat_idx,SET_SELECTED,&s_node);
        if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF)
        {
            osd_set_text_field_content(&satsrch_txt6_note , STRING_ID, (s_node.tuner1_valid ? RS_LNB1 : RS_LNB2));
        }
        else
        {
            osd_set_text_field_content(&satsrch_txt6_note , STRING_ID, RS_LNB1);
        }
        osd_draw_object((POBJECT_HEAD)&satsrch_txt6_note, C_UPDATE_ALL);
        break;
    case EVN_POST_OPEN:
        if(win_search_is_multi_sat_search())
        {
            osd_draw_object((POBJECT_HEAD)&satsrch_split,C_UPDATE_ALL);
        }
        win_satsrch_set_drawselectsats();
        break;
    case EVN_PRE_CLOSE:
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

static void win_satsrch_set_drawselectsats(void)
{
    //struct osdrect rect;
    UINT16 i = 0,sat_cnt =0,num = 0;
    UINT16 row = 0;
    TEXT_FIELD* txt = &satsrch_multisats_name;
    S_NODE s_node;
	UINT16 top = 0;
	//UINT16 str[50] ={0};
	//UINT16 n_str_len = 0;
	INT32 str_ret = 0;

    MEMSET(&s_node, 0 ,sizeof(s_node));
    if(FALSE == win_search_is_multi_sat_search())
    {
        return;
    }
    MEMSET(sat_ids,0,sizeof(sat_ids)/sizeof(sat_ids[0]));
    sat_cnt = num = get_tuner_sat_cnt(TUNER_EITHER_SELECT);//get_selected_sat_num();
    if(num > SELSAT_ROW_CNT *2)
        num = SELSAT_ROW_CNT *2;
    txt->p_string = display_strs[0];
    for(i=0;i<sat_cnt;i++)
    {
        get_tuner_sat(TUNER_EITHER_SELECT,i,&s_node);
        sat_ids[i] = s_node.sat_id;
    }

    for(i=0;i<SELSAT_ROW_CNT*2;i++)
    {
        row = i/2;

        top = SELSAT_T + row * SELSAT_H+40;
        if(i%2 == 0)
            osd_set_rect(&txt->head.frame, SELSAT_L0, top, SELSAT_W, SELSAT_H-6);
        else
            osd_set_rect(&txt->head.frame, SELSAT_L1, top, SELSAT_W, SELSAT_H-6);
        get_tuner_sat(TUNER_EITHER_SELECT,i,&s_node);
        if(i<sat_cnt)
        {
            if(s_node.di_seq_c_type !=6)
		    {
			    //com_uni_str_copy_char((UINT8*)txt->p_string, s_node.sat_name);
                str_ret = com_uni_str_copy_char_n((UINT8*)txt->p_string, s_node.sat_name, MAX_DISP_STR_LEN-1);
                if(0 == str_ret)
                {
                    ali_trace(&str_ret);
                }
		    }
        }

        else
            txt->p_string[0] = 0;//ComAscStr2Uni("", txt->pString);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }

    if(num == 0)
    {
        txt->p_string = NULL;
        #ifndef SD_UI
        top = SELSAT_T + 2 * SELSAT_H+40;
        #else
        top = SELSAT_T + 40;
        #endif
        osd_set_rect(&txt->head.frame, SELSAT_L0 + SELSAT_W/2, top, SELSAT_W, SELSAT_H-6);
        osd_set_text_field_content(txt, STRING_ID , RS_DISPLAY_NO_SATELLITE/*RS_NO_SATELLITE_SELECT*/);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }
}
static void win_satsrch_set_search_param(void)
{
	UINT32  search_mode = 0;
	UINT32 prog_type = 0;
    S_NODE s_node;
    struct as_service_param param;
	struct nim_device *nim = NULL;
	//struct vdec_device *vdec = NULL;
	//struct vdec_io_get_frm_para vfrm_param;
	UINT32 addr = 0, len = 0;
	UINT32 i = 0;
#ifdef PLSN_SUPPORT
    SYSTEM_DATA *sys_data = NULL;
#endif
    
    MEMSET(&s_node, 0 ,sizeof(s_node));
    MEMSET(&param, 0 ,sizeof(param));
    //tv/radio/all
    if(SATSRCH_CHAN_IDX==1)
        prog_type = P_SEARCH_TV;
    else if(SATSRCH_CHAN_IDX==2)
        prog_type = P_SEARCH_RADIO;
    else// if(SATSRCH_CHAN_IDX==0)
        prog_type = P_SEARCH_TV|P_SEARCH_RADIO;

    /* FTA Only */
    if (SATSRCH_FTA_IDX == 1)
        search_mode = P_SEARCH_FTA;
    else// if (sSATSRCH_FTA_IDX == 1)
        search_mode = P_SEARCH_FTA|P_SEARCH_SCRAMBLED;

    param.as_from = AS_FREQ_MIN;
    param.as_to = AS_FREQ_MAX;
#ifdef MULTISTREAM_SUPPORT
    param.ms_enable = 1;    // Enable MultiStream search
#endif

#ifdef PLSN_SUPPORT
    sys_data = sys_data_get();
    param.super_scan = sys_data->super_scan;
#endif

    param.as_prog_attr = prog_type|search_mode;
#ifndef NEW_DEMO_FRAME
    if(FALSE == win_search_is_multi_sat_search())
    {
        get_tuner_sat(TUNER_EITHER,SATSRCH_SAT_IDX,&s_node);
        param.as_sat_id = s_node.sat_id;
        if (s_node.lnb_type == LNB_CTRL_22K)
        {
            param.as_from = 950;
            param.as_to = 2150;
        }
    }
    else
    {
         param.as_sat_ids = sat_ids;
    }
#else
    param.as_frontend_type = FRONTEND_TYPE_S;
    if(FALSE == win_search_is_multi_sat_search())
    {
        get_tuner_sat(TUNER_EITHER,SATSRCH_SAT_IDX, &s_node);
        param.sat_ids[0] = s_node.sat_id;
        param.as_sat_cnt = 1;
    }
    else
    {
        param.as_sat_cnt = get_tuner_sat_cnt(TUNER_EITHER_SELECT);

        // multi-search max satellite number is 256
        if(param.as_sat_cnt > 256)
            param.as_sat_cnt = 256;
        MEMCPY(param.sat_ids, sat_ids, param.as_sat_cnt * sizeof(UINT16));
    }
#endif

    search_type = SATSRCH_MODE_IDX;
	if((SATSRCH_MODE_IDX == IDX_SCAN_MODE_AUTO_SCAN) ||(SATSRCH_MODE_IDX == IDX_SCAN_MODE_ACCURATE_SCAN) )
    {
        if(FALSE == win_search_is_multi_sat_search())
        {
            param.as_method = AS_METHOD_FFT;
        }
        else
        {
            param.as_method = AS_METHOD_MULTI_FFT;
        }
    #ifdef MULTISTREAM_SUPPORT
        param.scan_type = 1;    // if enable multistream search or not in auto scan. 1: enable; 0:disable
    #endif

    }
    else if (SATSRCH_MODE_IDX == IDX_SCAN_MODE_PRESET_SCAN)
    {
        if(FALSE == win_search_is_multi_sat_search())
            param.as_method = (SATSRCH_NIT_IDX==0)? AS_METHOD_SAT_TP : AS_METHOD_NIT;
            else
                    param.as_method = (SATSRCH_NIT_IDX==0)? AS_METHOD_MULTI_TP : AS_METHOD_MULTI_NIT;

    }
     for(i = 0;i < param.as_sat_cnt;i++)
     {
          get_sat_by_id(param.sat_ids[i], &s_node);
          if(s_node.tuner1_valid == 1)
              {
               nim=    (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
              }
         else if(s_node.tuner2_valid == 1)
             {
             nim=    (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
             }
         else
             continue;
         if(SATSRCH_MODE_IDX == IDX_SCAN_MODE_ACCURATE_SCAN)
             nim_io_control(nim, NIM_DRIVER_SET_BLSCAN_MODE, 1);
         else
            nim_io_control(nim, NIM_DRIVER_SET_BLSCAN_MODE, 0);
    }

    //Accurate scan
    //else //if (SATSRCH_MODE_IDX == IDX_SCAN_MODE_ACCURATE_SCAN)
    //{
    //    param.as_method = AS_METHOD_ACCURATE;
    //}
    param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
    param.as_handler = NULL;

    // set param
    win2_search_set_param(&param);

    //update_data();
#if 0
    vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
    vdec_io_control(vdec, VDEC_IO_GET_FRM, (UINT32)&vfrm_param);
    addr = vfrm_param.t_frm_info.u_c_addr;
    len = (vfrm_param.t_frm_info.u_height*vfrm_param.t_frm_info.u_width*3)/2;
#else
    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
    addr &= 0x8fffffff;
    len = __MM_AUTOSCAN_DB_BUFFER_LEN;
#endif
    db_search_init((UINT8 *)addr, len);

}


static void win_satsrch_set_nitactive(BOOL update)
{
	UINT8 action = 0;

    action = (SATSRCH_MODE_IDX == IDX_SCAN_MODE_PRESET_SCAN)? C_ATTR_ACTIVE : C_ATTR_INACTIVE;
    set_container_active(&satsrch_item4, action);
    if(update)
        osd_draw_object( (POBJECT_HEAD)&satsrch_item4, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
}


