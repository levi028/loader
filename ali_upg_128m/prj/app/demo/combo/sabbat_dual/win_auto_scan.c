/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_auto_scan.c
*
*    Description: The function to realize auto scan
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/nim/nim.h>
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

#include "win_com_menu_define.h"
#include "win_auto_scan.h"
#include "win_search.h"
/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER        g_win_autoscan;
extern CONTAINER        item_country;     //country
extern CONTAINER        item_fta_only;    // FTA Only
extern CONTAINER        item_start;       //start search
extern TEXT_FIELD       item_country_label;
extern TEXT_FIELD       item_fta_only_label;
extern TEXT_FIELD       item_start_label;
extern MULTISEL         item_country_sel;
extern MULTISEL         item_fta_only_sel;

#ifdef DVBT2_SUPPORT
extern CONTAINER        item_t2_signal_only;    // T2 Only
extern TEXT_FIELD       item_t2_signal_only_label;
extern MULTISEL         item_t2_signal_only_sel;
#ifdef DVBT2_LITE_SUPPORT
extern CONTAINER        item_t2_lite_support;    // T2-Lite support
extern TEXT_FIELD       item_t2_lite_support_label;
extern MULTISEL         item_t2_lite_support_sel;
#endif
#endif

#ifdef POLAND_SPEC_SUPPORT
extern CONTAINER        item_action; //action
extern TEXT_FIELD        item_action_label; //action
extern MULTISEL        action_sel;
#endif

static VACTION autoscan_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT autoscan_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION autoscan_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT autoscan_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION autoscan_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT autoscan_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define TXT_BUTTON_IDX     WSTL_BUTTON_02_HD//WSTL_NL_BUTTON_01_HD
/****************************************************************************************/
#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    autoscan_item_keymap,autoscan_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_TXT_BUTTON(root,var_txt,nxt_obj,ID,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    autoscan_item_sel_keymap,autoscan_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL,ID, l + TXTS_L_OF ,t + TXTS_T_OF,TXTS_W,TXTS_H,style,cur,cnt,ptbl)

/*
#define LDEF_MENU_ITEM_TXT(root,varCon,nxtObj,varTxt,ID,IDu,IDd,l,t,w,h,resID) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT_BUTTON(&varCon,varTxt,NULL,ID,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W-50,TXTN_H,resID)
*/
#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT_BUTTON(&var_con,var_txt,NULL,ID,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W-50,TXTN_H,res_id)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    autoscan_keymap,autoscan_callback,  \
    nxt_obj, focus_id,0)

#define WIN g_win_autoscan

#define COUNTRY_ID      1
#define FTA_ONLY_ID	 	2
#ifdef POLAND_SPEC_SUPPORT
	#ifdef	DVBT2_SUPPORT 
		#define T2_ONY_ID		3
		#ifdef DVBT2_LITE_SUPPORT		
		#define T2_LITE_SUPPORT_ID	4
		#define POLAND_SPEC_ID	5
		#define START_ID	6
		#else
		#define POLAND_SPEC_ID	4
		#define START_ID	5
		#endif
	#else
		#define START_ID	4	
	#endif	
#else
	#ifdef	DVBT2_SUPPORT 
		#define T2_ONY_ID	3
		#ifdef DVBT2_LITE_SUPPORT
		#define T2_LITE_SUPPORT_ID	4
		#define START_ID	5
		#else
		#define START_ID	4
		#endif
	#else
		#define START_ID	3
	#endif	
#endif

POBJECT_HEAD autoscan_items[] =
{
    (POBJECT_HEAD)&item_country,
    (POBJECT_HEAD)&item_fta_only,
 #ifdef DVBT2_SUPPORT
	(POBJECT_HEAD)&item_t2_signal_only,
	#ifdef DVBT2_LITE_SUPPORT
	(POBJECT_HEAD)&item_t2_lite_support,
	#endif
#endif	
    (POBJECT_HEAD)&item_start,
#ifdef POLAND_SPEC_SUPPORT
    (POBJECT_HEAD)&item_action,
#endif
};

#if 0
UINT16 isdbt_country_ids[] = /* Should have the same order as stream_iso_639lang_abbr*/
{
    RS_REGION_BRA,
    RS_REGION_ARG,
};

UINT16 dvbt_country_ids[] = /* Should have the same order as stream_iso_639lang_abbr*/
{
    RS_REGION_ARG,
    RS_REGION_AUS,
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
UINT16 region_ids[] =
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

#ifdef POLAND_SPEC_SUPPORT
static UINT16 action_id[]=
{
    RS_REINSTALL,
    RS_UPDATE,
};
#define ACTION                         (action_sel.n_sel)
#endif

/*******************************************************************************
*    Component  Object  Define
*******************************************************************************/
#ifdef POLAND_SPEC_SUPPORT
	#ifdef DVBT2_SUPPORT
	LDEF_MENU_ITEM_SEL(WIN, item_country, &item_fta_only, item_country_label, item_country_sel, COUNTRY_ID, START_ID, FTA_ONLY_ID, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_NET_COUNTRY, STRING_ID, 0, 2, region_ids)
	LDEF_MENU_ITEM_SEL(WIN, item_fta_only, &item_t2_signal_only, item_fta_only_label, item_fta_only_sel, FTA_ONLY_ID, COUNTRY_ID, T2_ONY_ID, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, 0, 2, yesno_ids)
		#ifdef DVBT2_LITE_SUPPORT
		LDEF_MENU_ITEM_SEL(WIN, item_t2_signal_only, &item_t2_lite_support, item_t2_signal_only_label, item_t2_signal_only_sel, T2_ONY_ID, FTA_ONLY_ID, T2_LITE_SUPPORT_ID, \
		 CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, 0, STRING_ID, 0, 2, yesno_ids)
		LDEF_MENU_ITEM_SEL(WIN, item_t2_lite_support, &item_action, item_t2_lite_support_label, item_t2_lite_support_sel, T2_LITE_SUPPORT_ID, T2_ONY_ID, POLAND_SPEC_ID, \
		 CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, 0, STRING_ID, 0, 2, yesno_ids)
		LDEF_MENU_ITEM_SEL(WIN, item_action, &item_start, item_action_label, action_sel, POLAND_SPEC_ID, T2_LITE_SUPPORT_ID, START_ID, \
		                   CON_L, CON_T + (CON_H + CON_GAP)*4 CON_W, CON_H, RS_ACTION, STRING_ID, 0, 2, action_id)
		LDEF_MENU_ITEM_TXT(WIN, item_start, NULL, item_start_label, START_ID, POLAND_SPEC_ID, COUNTRY_ID, \
		                   CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, RS_COMMON_SEARCH)
		#else
		LDEF_MENU_ITEM_SEL(WIN, item_t2_signal_only, &item_action, item_t2_signal_only_label, item_t2_signal_only_sel, FTA_ONLY_ID, COUNTRY_ID, START_ID, \
	                 CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, 0, STRING_ID, 0, 2, yesno_ids)
		LDEF_MENU_ITEM_SEL(WIN, item_action, &item_start, item_action_label, action_sel, POLAND_SPEC_ID, T2_LITE_SUPPORT_ID, START_ID, \
		                   CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_ACTION, STRING_ID, 0, 2, action_id)
		LDEF_MENU_ITEM_TXT(WIN, item_start, NULL, item_start_label, START_ID, POLAND_SPEC_ID, COUNTRY_ID, \
		                   CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_COMMON_SEARCH)
		#endif
	#else
	LDEF_MENU_ITEM_SEL(WIN, item_country, &item_fta_only, item_country_label, item_country_sel, 1, 4, 2, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_NET_COUNTRY, STRING_ID, 0, 2, region_ids)
	LDEF_MENU_ITEM_SEL(WIN, item_fta_only, &item_action, item_fta_only_label, item_fta_only_sel, 2, 1, 3, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, 0, 2, yesno_ids)
	LDEF_MENU_ITEM_SEL(WIN, item_action, &item_start, item_action_label, action_sel, 3, 2, 4, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_ACTION, STRING_ID, 0, 2, action_id)
	LDEF_MENU_ITEM_TXT(WIN, item_start, NULL, item_start_label, 4, 3, 1, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_COMMON_SEARCH)
	#endif		                   
#else
	LDEF_MENU_ITEM_SEL(WIN, item_country, &item_fta_only, item_country_label, item_country_sel, COUNTRY_ID, START_ID, FTA_ONLY_ID, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_NET_COUNTRY, STRING_ID, 0, 2, region_ids)
	#ifdef DVBT2_SUPPORT
	LDEF_MENU_ITEM_SEL(WIN, item_fta_only, &item_t2_signal_only, item_fta_only_label, item_fta_only_sel, FTA_ONLY_ID, COUNTRY_ID, T2_ONY_ID, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, 0, 2, yesno_ids)
		#ifdef DVBT2_LITE_SUPPORT
		LDEF_MENU_ITEM_SEL(WIN, item_t2_signal_only, &item_t2_lite_support, item_t2_signal_only_label, item_t2_signal_only_sel, T2_ONY_ID, FTA_ONLY_ID, T2_LITE_SUPPORT_ID, \
		                   CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, 0, STRING_ID, 0, 2, yesno_ids)
		LDEF_MENU_ITEM_SEL(WIN, item_t2_lite_support, &item_start, item_t2_lite_support_label, item_t2_lite_support_sel, T2_LITE_SUPPORT_ID, T2_ONY_ID, START_ID, \
		                   CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, 0, STRING_ID, 0, 2, yesno_ids)
		LDEF_MENU_ITEM_TXT(WIN, item_start, NULL, item_start_label, START_ID, T2_LITE_SUPPORT_ID, COUNTRY_ID, \
		                 CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_COMMON_SEARCH)
		#else
		LDEF_MENU_ITEM_SEL(WIN, item_t2_signal_only, &item_start, item_t2_signal_only_label, item_t2_signal_only_sel, T2_ONY_ID, FTA_ONLY_ID, START_ID, \
	                 CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, 0, 2, yesno_ids)
		LDEF_MENU_ITEM_TXT(WIN, item_start, NULL, item_start_label, START_ID, T2_ONY_ID, COUNTRY_ID, \
		                 CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_COMMON_SEARCH)
		#endif	 
	#else
	LDEF_MENU_ITEM_SEL(WIN, item_fta_only, &item_start, item_fta_only_label, item_fta_only_sel, FTA_ONLY_ID, COUNTRY_ID, START_ID, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FTA_ONLY, STRING_ID, 0, 2, yesno_ids)
	LDEF_MENU_ITEM_TXT(WIN, item_start, NULL, item_start_label, START_ID, FTA_ONLY_ID, COUNTRY_ID, \
	                   CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_COMMON_SEARCH)
	#endif
#endif
LDEF_WIN(WIN, &item_country, W_L, W_T, W_W, W_H, START_ID)


static BOOL current_frontend_is_isdbt = FALSE;
//modify for channel scan mode show the title auto scan on T tuner 2011 11 25
extern BOOL is_auto_scan;

#define VACT_POP_UP	(VACT_PASS + 1)
//#define AS_FREQ_MIN     950
//#define AS_FREQ_MAX     2200
void win_autoscan_set_search_param(void)
{
	UINT32  search_mode =0;
	UINT32 prog_type =0;
    S_NODE s_node;
    struct as_service_param param;

	//struct vdec_device *vdec = NULL;
	//struct vdec_io_get_frm_para vfrm_param;
	UINT32 addr =0, len =0;
	UINT8 u_band_cnt =0,index_band_cnt =0;
	SYSTEM_DATA* sys_data = NULL;
    MEMSET(&s_node, 0 ,sizeof(s_node));
    MEMSET(&param, 0, sizeof(param));
    sys_data = sys_data_get();
#ifdef POLAND_SPEC_SUPPORT
       extern g_enter_welcom;
       TIMER_SET_CONTENT* timer;

       INT16 i,n;

    if(g_enter_welcom == 3)
    {
        action_sel.n_sel = 1; //update
    }

    if(ACTION == 0) //reinstall
    {
        sim_close_monitor(0);
        n = get_sat_num(VIEW_ALL);
        recreate_prog_view(PROG_ALL_MODE,0);
        for(i=0;i<n;i++)
        {
        get_sat_at(i, VIEW_ALL,&s_node);
        del_child_prog(TYPE_SAT_NODE, s_node.sat_id);
        }

        update_data();
        sys_data_check_channel_groups();
                    /*turn off all timer*/

                    for(i=0;i<MAX_TIMER_NUM;i++)
                    {
                        timer = &sys_data->timer_set.timer_content[i];
                        timer->timer_mode = TIMER_MODE_OFF;
                    }

        set_auto_scan_update_mode(FALSE);
    }
    else
    {
        set_auto_scan_update_mode(TRUE);
    }
#endif

    //tv/radio/all
    prog_type = P_SEARCH_TV|P_SEARCH_RADIO;

    /* FTA Only */
    if (osd_get_multisel_sel(&item_fta_only_sel) == 1)
        search_mode = P_SEARCH_FTA|P_SEARCH_NIT;
    else// if (sSATSRCH_FTA_IDX == 1)
        search_mode = P_SEARCH_FTA|P_SEARCH_SCRAMBLED|P_SEARCH_NIT;

    //get_default_bandparam(OSD_GetMultiselSel(&item_country_sel),(Band_param*)  &uCountryParam);
      osd_set_multisel_sel(&item_country_sel, sys_data->country);

    if(current_frontend_is_isdbt)
        get_isdbt_bandparam(osd_get_multisel_sel(&item_country_sel), (band_param *)&u_country_param);
    else
        get_dvbt_bandparam(osd_get_multisel_sel(&item_country_sel), (band_param *)&u_country_param);

#ifdef POLAND_SPEC_SUPPORT
    if(region_ids[sys_data->country]==RS_REGION_POL)
        get_dvbt_bandparam_for_poland((band_param *)&u_country_param);
#endif

    u_band_cnt = 0;
    for(index_band_cnt=0;index_band_cnt<MAX_BAND_COUNT;index_band_cnt++)
    {
        if(u_country_param[index_band_cnt].start_freq==0)//warning shooting xing
            break;
        else
            u_band_cnt++;
    }

    if(current_frontend_is_isdbt)
    {
        param.sat_ids[0] = get_frontend_satid(FRONTEND_TYPE_ISDBT, 1);
    	param.as_frontend_type = FRONTEND_TYPE_ISDBT;
    }
    else
    {
        param.sat_ids[0] = get_frontend_satid(FRONTEND_TYPE_T, 0);
    	param.as_frontend_type = FRONTEND_TYPE_T; 	
    }
    //param.as_from = AS_FREQ_MIN;
    //param.as_to = AS_FREQ_MAX;
    param.as_prog_attr = prog_type|search_mode;
    param.as_sat_cnt = 1;
    param.as_method = AS_METHOD_FFT;
    param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
    param.as_handler = NULL;
    param.band_group_param=u_country_param;
    param.scan_cnt = u_band_cnt;


    // set param
    win_search_set_param(&param);

    //update_data();

    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
    addr &= 0x8fffffff;
    len = __MM_AUTOSCAN_DB_BUFFER_LEN;

    db_search_init((UINT8 *)addr, len);
}


static VACTION autoscan_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT autoscan_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	//VACTION unact = VACT_PASS;
	UINT32 sel = 0;
	UINT8 b_id = 0;
	UINT16 * uni_str = NULL;
    OSD_RECT rect;
    MEMSET(&rect, 0, sizeof(rect));

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        break;
    case EVN_UNKNOWN_ACTION:
     /*
        unact = (VACTION)(param1>>16);
        if(unact ==  VACT_POP_UP)
        {
            OSD_SetRect2(&rect,&pObj->frame);
            OSD_TrackObject((POBJECT_HEAD)&autoscan_item1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        */
        break;
     default:
        break;
    }
    return ret;
}


static VACTION autoscan_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;
    switch (key)
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

static PRESULT autoscan_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8 b_id =0 ;
	VACTION unact = VACT_PASS;
	POBJECT_HEAD submenu = NULL;
    UINT32 searchT2SignalOnly = 0;
	struct nim_device *nim_dev = NULL;
	//UINT8 back_saved =0;
    SYSTEM_DATA* sys_data = sys_data_get();
    b_id = osd_get_obj_id(p_obj);
    
  	if(g_tuner_num == 1)
		cur_tuner_idx = 0;
	if(cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(unact == VACT_ENTER && b_id == START_ID)
        {
            #ifdef DVBT2_SUPPORT
            if(FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM))
            {
                searchT2SignalOnly = osd_get_multisel_sel(&item_t2_signal_only_sel);
                nim_io_control(nim_dev, NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY, searchT2SignalOnly);//jary0719
                #ifdef DVBT2_LITE_SUPPORT
                sys_data->t2_lite_support= osd_get_multisel_sel(&item_t2_lite_support_sel);
                nim_io_control(nim_dev, NIM_DRIVER_T2_LITE_ENABLE, (UINT32)(sys_data->t2_lite_support));
                #endif
            }
            #endif

            sys_data->country = osd_get_multisel_sel(&item_country_sel);
            sys_data_save(1);
            win_autoscan_set_search_param();
            submenu = (POBJECT_HEAD)&g_win_search;
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


static VACTION autoscan_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT autoscan_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT16 title_id =0;
    OSD_RECT rect;
    SYSTEM_DATA* sys_data = sys_data_get();
    S_NODE s_node;
    TEXT_FIELD *pTxt;
	UINT32 searchT2SignalOnly;
    struct nim_device *nim_dev = NULL;

    MEMSET(&rect, 0, sizeof(rect));
    MEMSET(&s_node, 0, sizeof(s_node));
    	
  	if(g_tuner_num == 1)
		cur_tuner_idx = 0;
	if(cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;
    
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
            title_id = RS_ISDBT_AUTO_SCAN;
        else
            title_id = RS_DVBT_AUTO_SCAN;

        osd_set_multisel_count(&item_country_sel, ARRAY_SIZE(region_ids));
        osd_set_multisel_sel_table(&item_country_sel, region_ids);
        //OSD_SetMultiselSel(&item_country_sel, sys_data->region_id);
        osd_set_multisel_sel(&item_country_sel, sys_data->country);

#ifdef DVBT2_SUPPORT
        pTxt = &item_t2_signal_only_label;
        osd_set_text_field_str_point(pTxt,display_strs[1]);
        osd_set_text_field_content(pTxt,STRING_ANSI,(UINT32)"T2 Signal Only");
#ifdef DVBT2_LITE_SUPPORT
        pTxt = &item_t2_lite_support_label;
        osd_set_multisel_sel(&item_t2_lite_support_sel, sys_data->t2_lite_support);
        osd_set_text_field_str_point(pTxt,display_strs[2]);
        osd_set_text_field_content(pTxt,STRING_ANSI,(UINT32)"T2-Lite Support");
#endif
#endif		
        is_auto_scan=TRUE;//modify for channel scan mode show the title auto scan on T tuner 2011 11 25
        wincom_open_title(p_obj,title_id,0);
       // OSD_SetMultiselSel(&item_country_sel, sys_data->country);
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
         sys_data->country = osd_get_multisel_sel(&item_country_sel);
#ifdef DVBT2_SUPPORT
        if(FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM))
        {
            searchT2SignalOnly = osd_get_multisel_sel(&item_t2_signal_only_sel);
            nim_io_control(nim_dev, NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY, searchT2SignalOnly);
#ifdef DVBT2_LITE_SUPPORT
            //sys_data->t2_lite_support = osd_get_multisel_sel(&item_t2_lite_support_sel);
            nim_io_control(nim_dev, NIM_DRIVER_T2_LITE_ENABLE, (UINT32)(sys_data->t2_lite_support));
#endif
        }
#endif

        sys_data_save(1);
        break;
    case EVN_POST_CLOSE:
        //sdt_monitor_off();
		break;		
    default:
        break;
    }
    return ret;
}

void win_autoscan_set_isdbt_param(BOOL isdbt_val)
{
    current_frontend_is_isdbt=isdbt_val;
}
