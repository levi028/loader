/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ota_set.c
*
*    Description:   The setting of OTA upgrade
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
#include <bus/tsi/tsi.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_frontend.h>
#endif
#include <api/libosd/osd_lib.h>
#ifdef AUTO_OTA
#include <api/libtsi/si_types.h>
#endif
#include <api/libchunk/chunk.h>
#include <sys_parameters.h>
#include <api/libc/fast_crc.h>
#include <hld/nim/nim_tuner.h>

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
#include "win_ouc.h"

#ifdef SUPPORT_BC
#include "bc_ap/bc_osm.h"
#endif
#ifdef SUPPORT_BC_STD
#include "bc_ap_std/bc_osm.h"
#endif
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif
#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_upg.h>
#include <udi/nv/ca_mmap.h>
#endif

#if defined(SUPPORT_BC)
#include "bc_ap/bc_osm.h"
#endif

#include "win_com.h"
#include "control.h"

#define  WIN_OTA_SET_PRIVATE_MACROS
#include "win_ota_set.h"
#include "platform/board.h"

//vicky_OTA_PARA
#if defined (_CAS9_CA_ENABLE_)  
#include <api/libchunk/chunk_hdr.h>
#endif
#include "gos_ui/gacas_loader_db.h"
#include "gos_ui/gacas_upg.h"

#define OTA_PRINTF(...)
//#define OTA_PRINTF          libc_printf

//===================== local variables ==========================
static UINT16 otaset_display_strs[3][30]={{0,},};
static char pid_pat[];

#ifdef _BUILD_UPG_LOADER_
static struct help_item_resource  otaset_helpinfo[] =
{
    {IM_MEDIAPLAY_HELPBUTTON_SWAP,    RS_MP_HELP_SWITCH},
    {IM_MEDIAPLAY_HELPBUTTON_EXIT,    RS_HELP_EXIT},
};
#define HELP_CNT    (sizeof(otaset_helpinfo)/sizeof(struct help_item_resource))
#endif

LDEF_MENU_ITEM_SEL(win_otaupg_con,wota_con0,&wota_con1,wota_txt0,wota_num0,wota_line0,1,7,2,\
            CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_INFO_SATELLITE, STRING_PROC, 0, 0, NULL)

LDEF_MENU_ITEM_SEL(win_otaupg_con,wota_con1,&wota_con2,wota_txt1,wota_num1,wota_line1,2,1,3,\
            CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,RS_INFO_TRANSPONDER, STRING_PROC, 0, 0, NULL)

LDEF_MENU_ITEM_EDF(win_otaupg_con,wota_con2,&wota_con3,wota_txt2,wota_num2,wota_line2,3,2,4,\
            CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W, CON_H, RS_DISPLAY_FREQUENCY, \
            NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_pat, NULL,fre_sub,otaset_display_strs[0])

LDEF_MENU_ITEM_EDF(win_otaupg_con,wota_con3,&wota_con4,wota_txt3,wota_num3,wota_line3,4,3,5,\
            CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_INFO_SYMBOL, NORMAL_EDIT_MODE, \
            CURSOR_SPECIAL, symb_pat,NULL,symb_sub,otaset_display_strs[1])

LDEF_MENU_ITEM_SEL(win_otaupg_con,wota_con4,&wota_con5,wota_txt4,wota_num4,wota_line4,5,4,6,\
            CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_INFO_POLARITY, STRING_ID, 0, 2, tp_polarity_ids)

LDEF_MENU_ITEM_EDF(win_otaupg_con,wota_con5,&wota_txt6,wota_txt5,wota_num5,wota_line5,6,5,7,\
            CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H, RS_UPGRADE_PID, NORMAL_EDIT_MODE, \
            CURSOR_NO, pid_pat,NULL,NULL,otaset_display_strs[2])

LDEF_TXT_BTN(win_otaupg_con,wota_txt6,&wota_line6, wota_line6, 7,6,1,\
            CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H,RS_COMMON_START)

LDEF_WIN(win_otaupg_con,&wota_con0,W_L, W_T,W_W,W_H,1)

static UINT16 ota_model_type_ids[] =
{
    RS_CONSTELLATION_16,
    RS_CONSTELLATION_32,
    RS_CONSTELLATION_64,
    RS_CONSTELLATION_128,
    RS_CONSTELLATION_256,
};

static char fre_pat_dvbt[]  = "r100000~858000";
static char symb_pat_dvbt[]  = "r1000~45000";
static char fre_pat_dvbc[]  = "f32";
static char symb_pat_dvbc[]  = "s5";

static UINT16 bandwidth_ids[] =
{
    RS_6MHZ,
    RS_7MHZ,
    RS_8MHZ,
};

static POBJECT_HEAD otaset_items[]=
{
    (POBJECT_HEAD)&wota_con0,
    (POBJECT_HEAD)&wota_con1,
    (POBJECT_HEAD)&wota_con2,
    (POBJECT_HEAD)&wota_con3,
    (POBJECT_HEAD)&wota_con4,
    (POBJECT_HEAD)&wota_con5,
    (POBJECT_HEAD)&wota_txt6
};

static OTASET_OBJECT_ATTR  otaset_dvbs_menu[]=
{
    {{CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W, CON_H},C_ATTR_ACTIVE,RS_INFO_SATELLITE},
    {{CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H},C_ATTR_ACTIVE,RS_INFO_TRANSPONDER},
    {{CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H},C_ATTR_ACTIVE,RS_DISPLAY_FREQUENCY},
    {{CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H},C_ATTR_ACTIVE,RS_INFO_SYMBOL},
    {{CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H},C_ATTR_ACTIVE,RS_INFO_POLARITY},
    {{CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H},C_ATTR_ACTIVE,RS_UPGRADE_PID},
    {{CON_L, CON_T + (CON_H + CON_GAP)*6, CON_W, CON_H},C_ATTR_ACTIVE,RS_COMMON_START},
};

static OTASET_OBJECT_ATTR  otaset_dvbt_menu[]=
{
    {{CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H},C_ATTR_HIDDEN,RS_INFO_SATELLITE},
    {{CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H},C_ATTR_HIDDEN,RS_INFO_TRANSPONDER},
    {{CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H},C_ATTR_ACTIVE,RS_DISPLAY_FREQUENCY},
    {{CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H},C_ATTR_HIDDEN,RS_INFO_SYMBOL},
    {{CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H},C_ATTR_ACTIVE,RS_INSTALLATION_BANDWIDTH},
    {{CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H},C_ATTR_ACTIVE,RS_UPGRADE_PID},
    {{CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H},C_ATTR_ACTIVE,RS_COMMON_START},
};

static OTASET_OBJECT_ATTR  otaset_dvbc_menu[]=
{
    {{CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H},C_ATTR_HIDDEN,RS_INFO_SATELLITE},
    {{CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H},C_ATTR_HIDDEN,RS_INFO_TRANSPONDER},
    {{CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H},C_ATTR_ACTIVE,RS_DISPLAY_FREQUENCY},
    {{CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H},C_ATTR_ACTIVE,RS_INFO_SYMBOL},
    {{CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H},C_ATTR_ACTIVE,RS_INSTALLATION_QAM_MODE},
    {{CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H},C_ATTR_ACTIVE,RS_UPGRADE_PID},
    {{CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H},C_ATTR_ACTIVE,RS_COMMON_START},
};

static ID ota_turn_timer_id = OSAL_INVALID_ID;
static char pid_pat[]  = "r32~8191";
static UINT16 ota_presat_id=0;
static UINT16 ota_satpos=0;
static UINT16 ota_pol=0;
static UINT32 ota_freq	=11200;
static UINT32 ota_symb	=6875;
UINT16 ota_pid	= 7004;
static UINT32 ota_modulation=QAM128;
static UINT16 ota_bandwidth=0;
static OTA_CFG m_ota_cfg;
static UINT32 cur_front_type = FRONTEND_TYPE_C;
static UINT8 ota_is_init = 0;

static void win_otaset_tpturn_handler(UINT32 nouse)
{
    //BOOL send_flag=FALSE;

    if(ota_turn_timer_id != OSAL_INVALID_ID)
    {
        ota_turn_timer_id = OSAL_INVALID_ID;
        //send_flag=
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,(UINT32)&win_otaupg_con,FALSE);
    }
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION  otaset_btn_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT  otaset_btn_callback(POBJECT_HEAD pobj,VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact=0;
    POBJECT_HEAD pmenu = NULL;
    INT32 func_ret=0;

	MEMSET(&pmenu,0,sizeof(POBJECT_HEAD));
    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
#if defined(_NV_PROJECT_SUPPORT_) & !defined(_BUILD_UPG_LOADER_)
            /* set upg flag & save ota cfg */
            if(SUCCESS != otaset_switch_to_upgloader())
            {
                libc_printf("switch to upgloader fail!\n");
                ret = PROC_LOOP;
                break;
            }
#else
             func_ret=win_otaset_get_freq_symb_pid_display(cur_front_type);

            pmenu = (POBJECT_HEAD)&g_win2_otaupg ;
            if(osd_obj_open(pmenu, MENU_OPEN_TYPE_MENU + EDIT_TP_START_ID) != PROC_LEAVE)
            {
                menu_stack_push(pmenu);
            }

            ret = PROC_LOOP;
#endif
        }
        break;
    default:
        break;
    }

    return ret;
}

static VACTION  otaset_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

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

static PRESULT  otaset_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid __MAYBE_UNUSED__=0;
    UINT32 sel __MAYBE_UNUSED__=0;
    UINT16 *uni_str __MAYBE_UNUSED__=NULL;
    VACTION unact=0;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    S_NODE s_node;
    MEMSET(&s_node,0,sizeof(S_NODE));
#endif

    bid = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
    #if (SYS_PROJECT_FE == PROJECT_FE_DVBS|| SYS_PROJECT_FE == PROJECT_FE_DVBS2)
        if(EDIT_TP_SATE_ID == bid)
        {
        #ifdef _BUILD_OTA_E_
            com_asc_str2uni((UINT8 *)"Fixed Sat", uni_str);
        #else
            get_tuner_sat_name(OTA_TUNER_IDX,sel,uni_str);
        #endif
        }
        else if(EDIT_TP_TPIDX_ID == bid)
        {            
        #ifdef  _BUILD_OTA_E_
            com_asc_str2uni((UINT8 *)"Fixed TP", uni_str);
        #else
            get_tuner_sat(OTA_TUNER_IDX,SAT_POS,&s_node);
            get_tp_name(s_node.sat_id,sel,uni_str);
        #endif
        }
    #endif
    break;
    case EVN_POST_CHANGE:
        sel = param1;

    #if 0// (SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
        if(EDIT_TP_SATE_ID == bid)
            win_otaset_load_sat_setting(sel,TRUE);
        else if(EDIT_TP_TPIDX_ID == bid)
            win_otaset_load_tp_setting(sel,TRUE);
    #endif
        api_stop_timer(&ota_turn_timer_id);
        ota_turn_timer_id = api_start_timer(OTA_TURN_TIMER_NAME, OTA_TURN_TIMER_TIME, win_otaset_tpturn_handler);
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_POP_UP == unact)
        {
    #if 0//(SYS_PROJECT_FE == PROJECT_FE_DVBS || SYS_PROJECT_FE == PROJECT_FE_DVBS2)
            COM_POP_LIST_TYPE_T list_type=0;
            COM_POP_LIST_PARAM_T param;
            OSD_RECT rect;
            MEMSET(&param,0,sizeof(param));
            MEMSET(&rect,0,sizeof(rect));
            ret = PROC_LOOP;
            osd_set_rect2(&rect,&pobj->frame);
            param.selecttype = POP_LIST_SINGLESELECT;
            switch(bid)
            {
            case EDIT_TP_SATE_ID:
                rect.u_left -= 40;
                rect.u_width += 40;
                list_type = POP_LIST_TYPE_SAT;
                rect.u_height = 300;//230;
                param.id = OTA_TUNER_IDX;
                param.cur = SAT_POS;
                break;
            case EDIT_TP_TPIDX_ID:
                list_type = POP_LIST_TYPE_TP;
                rect.u_height = 300;//230;
                get_tuner_sat(OTA_TUNER_IDX,SAT_POS,&s_node);
                param.id = s_node.sat_id;
                param.cur = TP_POS;
                break;
            default:
                list_type = 0xFF;
                break;
            }

            if(0xff == list_type)
                break;
            UINT16 cur_idx=0;
            cur_idx = win_com_open_sub_list(list_type,&rect,&param);
            if(( LIST_INVALID_SEL_IDX==cur_idx ) ||(cur_idx == param.cur))
                break;

            if(EDIT_TP_SATE_ID== bid )
            {
                SAT_POS = cur_idx;
                osd_track_object((POBJECT_HEAD)&wota_con0, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                win_otaset_load_sat_setting((UINT32)cur_idx, TRUE);
            }
            else if( EDIT_TP_TPIDX_ID==bid )
            {
                TP_POS = cur_idx;
                osd_track_object((POBJECT_HEAD)&wota_con1,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                win_otaset_load_tp_setting((UINT32)cur_idx,TRUE);
            }
    #endif
            api_stop_timer(&ota_turn_timer_id);
            ota_turn_timer_id = api_start_timer(OTA_TURN_TIMER_NAME, OTA_TURN_TIMER_TIME, win_otaset_tpturn_handler);
        }
        break;
        default:
            break;
    }

    return ret;
}

static VACTION  otaset_item_edf_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
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

static PRESULT  otaset_item_edf_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid=0;
    UINT8 status=0;
    EDIT_FIELD *edf=NULL;

    bid = osd_get_obj_id(pobj);

    edf = (EDIT_FIELD*)pobj;

    switch(event)
    {
    case EVN_POST_CHANGE:
        status = osd_get_edit_field_status(edf);
        if(SELECT_STATUS == status)
        {
            if(( EDIT_TP_FREQ_ID==bid)  || (EDIT_TP_SYMB_ID==bid ) )
            {
                api_stop_timer(&ota_turn_timer_id);
                ota_turn_timer_id = api_start_timer(OTA_TURN_TIMER_NAME,OTA_TURN_TIMER_TIME, win_otaset_tpturn_handler);
            }
        }
        break;
    case EVN_PARENT_FOCUS_POST_LOSE:
        if((EDIT_TP_FREQ_ID==bid  ) || (EDIT_TP_SYMB_ID==bid ))
        {
            if(win_otaset_get_freq_symb_pid_display(cur_front_type) != NUM_ZERO)
            {
                api_stop_timer(&ota_turn_timer_id);
                ota_turn_timer_id = api_start_timer(OTA_TURN_TIMER_NAME,OTA_TURN_TIMER_TIME, win_otaset_tpturn_handler);
            }
        }


        break;
    default:
        break;
    }

    return ret;
}

static VACTION  otaset_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT  otaset_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION  otaset_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

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
#ifdef    _BUILD_OTA_E_
        act = VACT_PASS;
#else
        act = VACT_CLOSE;
#endif
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT  otaset_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	//PRESULT func_ret = PROC_PASS;
	POBJECT_HEAD    pfocusobj = NULL;
	UINT8 lock = 0;

	MEMSET(&pfocusobj,0,sizeof(POBJECT_HEAD));

	switch(event)
	{
		case EVN_PRE_OPEN:
			ota_presat_id = 0xFFFF;
			wincom_open_title_ext((POBJECT_HEAD)&pobj,(UINT8*)GACAS_UPG_TITLE_OTA,0);
			win_otaset_menu_config(cur_front_type);
			if(ota_is_init == 0)
			{
				if(RET_SUCCESS == gacas_loader_db_get_ota_cab(&ota_pid,&ota_freq,&ota_symb,&ota_modulation))
				{
					ota_freq /= 10;
					ota_modulation += QAM16;
				}
				ota_is_init = 1;
			}
			/*	pfocusobj = osd_get_focus_object(pobj);
			if(pfocusobj != NULL)
			{
				if(pfocusobj->b_attr != C_ATTR_ACTIVE)
				{
					pfocusobj = osd_get_adjacent_object(pfocusobj,VACT_CURSOR_DOWN);
					if(pfocusobj != NULL)
					{
						func_ret=osd_change_focus(pobj,pfocusobj->b_id,0);
					}
				}
			}*/
			osd_change_focus(pobj,7,0);
			win_otaset_load_freq_symb_pid_display(cur_front_type);
			break;
		case EVN_POST_DRAW:
			win_signal_open_ex(pobj,pobj->frame.u_left,pobj->frame.u_top + LIST_ITEMCNT*(CON_H + CON_GAP) +20);
			win_otaset_turn_frntend_ex(cur_front_type);
			api_stop_timer(&ota_turn_timer_id);
			ota_turn_timer_id = api_start_timer(OTA_TURN_TIMER_NAME,OTA_TURN_TIMER_TIME, win_otaset_tpturn_handler);
			lock = win_otaset_signal_refresh_combo(cur_front_type);
			break;
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//dm_set_onoff(DM_NIMCHECK_ON);
			api_stop_timer(&ota_turn_timer_id);
			break;
		case EVN_MSG_GOT:
			if(CTRL_MSG_SUBTYPE_STATUS_SIGNAL == param1)
			{
				lock = win_otaset_signal_refresh_combo(cur_front_type);
				win_otaset_force_to_ota(lock);
			}
			else if(CTRL_MSG_SUBTYPE_CMD_TP_TURN == param1)
			{
				win_otaset_turn_frntend_ex(cur_front_type);
			}
			break;
		default:
			break;
	}
	return ret;
}
void win_otaset_get_ota_info(UINT16 *p_pid,UINT32 *p_freq,UINT32 *p_symb,UINT32 *p_mod)
{
	*p_pid 		= ota_pid;
	*p_freq 		= ota_freq;
	*p_symb 	= ota_symb;
	*p_mod 		= ota_modulation;	
}
void win_otaset_force_to_ota(UINT8 is_lock_signal)
{
	UINT32 hkey;

	if(is_lock_signal == 0)
		return;
	if(ota_is_init == 1 && gacas_loader_db_is_ota_force() == TRUE && win_otaupg_con.focus_object_id == 7)
	{
		ap_vk_to_hk(0, V_KEY_ENTER, &hkey);
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
		ota_is_init = 2;
	}
	if(win_otaupg_con.focus_object_id != 7)
	{
		ota_is_init = 3;
	}
}

void win_otaset_load_freq_symb_pid_display(UINT32 fe_type)
{
    EDIT_FIELD *edf=NULL;
    MULTISEL *msel=NULL;
    BOOL func_flag=FALSE;

    if(FRONTEND_TYPE_S == fe_type)
    {
        edf = &wota_num2;
        func_flag=osd_set_edit_field_content(edf, STRING_NUMBER, (UINT32)ota_freq);

        edf = &wota_num3;
        func_flag=osd_set_edit_field_content(edf, STRING_NUMBER, (UINT32)ota_symb);
      
        edf = &wota_num5;
        func_flag=osd_set_edit_field_content(edf, STRING_NUMBER, (UINT32)ota_pid);

        msel = &wota_num4;
        osd_set_multisel_sel(msel, ota_pol);
    }
    else
    {
        win_otaset_load_freq_symb_pid_display_other(fe_type);
    }
}

INT32 win_otaset_get_freq_symb_pid_display(UINT32 fe_type)
{
    EDIT_FIELD *edf=NULL;
    MULTISEL *msel=NULL;
    UINT32 val=0;
    INT32 ret = 0;
    BOOL func_flag=FALSE;

    if(FRONTEND_TYPE_S == fe_type)
    {
        edf = &wota_num2;
        func_flag=osd_get_edit_field_int_value(edf, &val);
        if(ota_freq != val)
        {
            ota_freq = val;
            ret = -1;
        }

        edf = &wota_num3;
        func_flag=osd_get_edit_field_int_value(edf, &val);
        if(ota_symb != val)
        {
            ota_symb = val;
            ret = -1;
        }

        edf = &wota_num5;
        func_flag=osd_get_edit_field_int_value(edf, &val);
        ota_pid = (UINT16)val;

        msel = &wota_num4;
        val = osd_get_multisel_sel(msel);
        if(ota_pol != val)
        {
            ota_pol = val;
            ret = -1;
        }

        msel = &wota_num0;
        ota_satpos = osd_get_multisel_sel(msel);
        m_ota_cfg.pid = (UINT32)ota_pid;

        return ret;
    }
    else
    {
        ret =win_otaset_get_freq_symb_pid_display_other(fe_type);
    }
    return ret;
}

static void win_otaset_turn_frntend_ex(UINT32 fe_type)
{
    UINT8 back_saved=0;
    S_NODE s_node;
    T_NODE t_node;
    struct ft_antenna antenna;
    union ft_xpond xponder;
    struct nim_device *nim = NULL;

    UINT32 frequency=0;
    INT32  nim_id = 0;
    INT32 func_ret=0;

    MEMSET(&xponder, 0x0, sizeof (xponder));
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));
    MEMSET(&antenna, 0x0, sizeof (antenna));

    func_ret=win_otaset_get_freq_symb_pid_display(fe_type);
    if(FRONTEND_TYPE_S == fe_type)
    {
        cur_tuner_idx = OTA_TUNER_IDX;
#ifdef _BUILD_OTA_E_
        MEMCPY(&s_node, &m_ota_cfg.s_node, sizeof(s_node));
#else
        get_tuner_sat(OTA_TUNER_IDX, ota_satpos, &s_node);
        MEMCPY(&m_ota_cfg.s_node, &s_node, sizeof(s_node));
        t_node.frq = ota_freq;
        t_node.sym = ota_symb;
        t_node.pol = ota_pol;
        MEMCPY(&m_ota_cfg.t_node, &t_node, sizeof(t_node));
        m_ota_cfg.pid = (UINT32)ota_pid;
#endif
        cur_tuner_idx = s_node.tuner1_valid ? 0 : 1;
        MEMSET(&antenna, 0, sizeof(struct ft_antenna));
        MEMSET(&xponder, 0, sizeof(union ft_xpond));
    #ifdef DVBS_SUPPORT
        sat2antenna(&s_node, &antenna);
    #endif

        antenna.antenna_enable = 1;
        xponder.s_info.type = fe_type;
        xponder.s_info.frq = ota_freq;
        xponder.s_info.sym = ota_symb;
        xponder.s_info.pol = ota_pol;
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, \
    (s_node.tuner1_valid ? 0 : (s_node.tuner2_valid ? 1 : 0)));
        frontend_tuning(nim, &antenna, &xponder, 1);
        if(NUM_ZERO == cur_tuner_idx) //TUNER1 <--> TSI_TS_A <--> DMX0
        {
            tsi_dmx_src_select(TSI_DMX_0,TSI_TS_A);
            tsi_select(TSI_TS_A,ts_route_get_nim_tsiid(0));
        }
        else//TUNER2 <--> TSI_TS_B <--> DMX0
        {
            tsi_dmx_src_select(TSI_DMX_0,TSI_TS_B);
            tsi_select(TSI_TS_B, ts_route_get_nim_tsiid(1));
        }
        wincom_dish_move_popup_open(ota_presat_id, \
    s_node.sat_id,cur_tuner_idx,&back_saved);
        ota_presat_id = s_node.sat_id;
    }
    else if((FRONTEND_TYPE_T == fe_type) || (FRONTEND_TYPE_ISDBT == fe_type))
    {
        //board config isdbt and dvbt is same.
        if(FRONTEND_TYPE_T == board_get_frontend_type(0))
        {
            nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
            nim_id = 0;
        }
        else if(FRONTEND_TYPE_T == board_get_frontend_type(1))
        {
            nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
            nim_id = 1;
        }

        frequency  = ota_freq;
        t_node.frq = ota_freq;
        t_node.bandwidth = (ota_bandwidth + 6);

        MEMCPY(&m_ota_cfg.t_node, &t_node, sizeof(t_node));
        m_ota_cfg.pid = (UINT32)ota_pid;

        if(NULL != nim)
        {
            MEMSET(&xponder, 0, sizeof(xponder));
            xponder.t_info.type = FRONTEND_TYPE_T;
            xponder.t_info.frq = frequency;
            xponder.t_info.band_width = (ota_bandwidth + 6)*1000;//kHZ
            xponder.t_info.usage_type = (UINT8)USAGE_TYPE_AUTOSCAN;
            //frontend_set_nim(nim, NULL, &xpond, 1);
            //modify for ISDBT project,
        //in ota_upgrade menu, sometime system reset 2011 11 26
            if(FRONTEND_TYPE_ISDBT == fe_type)
            {
                if((OTA_TUNE_MIN_FREQUECY<=xponder.t_info.frq )\
            &&(OTA_TUNE_MAX_FREQUECY>=xponder.t_info.frq))
                    {
                    frontend_tuning(nim, NULL, &xponder, 1);
                    }
        }
        else
        {
        frontend_tuning(nim, NULL, &xponder, 1);
        }
        }

    }
    else if(FRONTEND_TYPE_C == fe_type)
    {
        if(fe_type == (UINT32)board_get_frontend_type(0))
        {
            nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
            nim_id=0;
        }
        else if(fe_type == (UINT32)board_get_frontend_type(1))
        {
            nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
            nim_id=1;
        }   

        t_node.frq = ota_freq;
        t_node.sym = ota_symb;
        t_node.pol = ota_pol;
        t_node.fec_inner = ota_modulation;
        MEMCPY(&m_ota_cfg.t_node, &t_node, sizeof(t_node));
        m_ota_cfg.pid = (UINT32)ota_pid;
		m_ota_cfg.fe_type = FRONTEND_TYPE_C ;
        if(NULL != nim)
        {
            MEMSET(&xponder, 0, sizeof(xponder));
            xponder.c_info.type = fe_type;
            xponder.c_info.frq = ota_freq;
            xponder.c_info.sym = ota_symb;
            xponder.c_info.modulation = ota_modulation;
            //frontend_set_nim(nim, NULL, &xpond, 1);
            frontend_tuning(nim, NULL, &xponder, 1);
        }
    }
    if(FRONTEND_TYPE_S != fe_type)
    {
        if(NUM_ZERO == nim_id) //TUNER1 <--> TSI_TS_A <--> DMX0
        {
            tsi_dmx_src_select(TSI_DMX_0,TSI_TS_A);
            tsi_select(TSI_TS_A,ts_route_get_nim_tsiid(0));
        }
        else//TUNER2 <--> TSI_TS_B <--> DMX0
        {
            tsi_dmx_src_select(TSI_DMX_0,TSI_TS_B);
            tsi_select(TSI_TS_B, ts_route_get_nim_tsiid(1));
        }
    }
}

//#else

void win_otaset_load_freq_symb_pid_display_other(UINT32 fe_type)
{
    EDIT_FIELD *edf=NULL;
    MULTISEL *msel=NULL;
    UINT32 freq_def=0;
    UINT8 dot_pos=0;
    __MAYBE_UNUSED__ int sel = 0;
    BOOL func_flag=FALSE;

    /* FREQ */
    edf = &wota_num2;
    wincom_mbs_to_i_with_dot(otaset_display_strs[0], &freq_def, &dot_pos);
    if(NUM_ZERO==freq_def)
    {
        wincom_i_to_mbs_with_dot(otaset_display_strs[0],\
                             ota_freq, DVBC_OTA_EDIT_LEN - 1, 0x02);
    }
    func_flag=osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" KHz");
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    if(FRONTEND_TYPE_C == fe_type)
    {
#ifdef DVBC_SUPPORT
        /*FREQ*/
        edf = &wota_num2;
        wincom_mbs_to_i_with_dot(otaset_display_strs[0], &freq_def, &dot_pos);
        wincom_i_to_mbs_with_dot(otaset_display_strs[0], ota_freq, DVBC_OTA_EDIT_LEN - 1, 0x02);
        osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)"MHz");
        osd_set_edit_field_style(edf, NORMAL_EDIT_MODE | SELECT_STATUS);

        /*SYMBOL*/
        edf = &wota_num3;
        osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)"KBps");
        wincom_i_to_mbs(otaset_display_strs[1], ota_symb, DVBC_OTA_EDIT_LEN - 1);
        osd_set_edit_field_style(edf, NORMAL_EDIT_MODE | SELECT_STATUS);
        /*QAM*/
        msel = &wota_num4;
        sel = ota_modulation - QAM16;
        sel = sel > 0 ? sel : 0;
        osd_set_multisel_sel(msel, sel);
#endif
    }
    else if((FRONTEND_TYPE_ISDBT == fe_type) || (FRONTEND_TYPE_T == fe_type))
    {
        /*freq*/
        edf = &wota_num2;
        func_flag=osd_set_edit_field_content(edf, STRING_NUMBER, (UINT32)ota_freq);

        /*band*/
        msel = &wota_num4;
        osd_set_multisel_sel(msel, ota_bandwidth);
    }

    /* PID */
    edf = &wota_num5;
    func_flag=osd_set_edit_field_content(edf, STRING_NUMBER, (UINT32)ota_pid);

}

INT32 win_otaset_get_freq_symb_pid_display_other(UINT32 fe_type)
{
    EDIT_FIELD *edf=NULL;
    MULTISEL  *msel=NULL;
    UINT32 val=0;
    INT32 ret = 0;
    UINT8   dot_pos=0;
    UINT32 freq=0;
    UINT32 symbol=0;
    UINT32 constellation=0;
    BOOL func_flag=FALSE;

	if(FRONTEND_TYPE_C == fe_type)
    {
        /* FREQ */
        edf = &wota_num2;
        wincom_mbs_to_i_with_dot(otaset_display_strs[0], &freq, &dot_pos);
        //OSD_GetEditFieldIntValue(edf, &val);
        if(ota_freq != freq)
        {
            ota_freq = freq;
            ret = -1;
        }

        /* SYMBOL */
        edf = &wota_num3;
        symbol = wincom_mbs_to_i(otaset_display_strs[1]);
        //OSD_GetEditFieldIntValue(edf, &val);
        if(ota_symb != symbol)
        {
            ota_symb = symbol;
            ret = -1;
        }

        /* QAM */
        msel = &wota_num4;
        constellation = osd_get_multisel_sel(msel)+QAM16;
        if(ota_modulation!= constellation)
        {
            ota_modulation = constellation;
            ret = -1;
        }
    }
    else
    {
        edf = &wota_num2;
        func_flag=osd_get_edit_field_int_value(edf, &val);
        if(ota_freq != val)
        {
            ota_freq = val;
            ret = -1;
        }

        msel = &wota_num4;
        val = osd_get_multisel_sel(msel);

        if(ota_bandwidth != val)
        {
            ota_bandwidth = val;
            ret = -1;
        }
    }
    /* PID */
    edf = &wota_num5;
    func_flag=osd_get_edit_field_int_value(edf, &val);
    ota_pid =(UINT16) val;
    m_ota_cfg.pid = (UINT32)ota_pid;

    return ret;
}
//#endif
static POBJECT_HEAD win_otaset_get_child_obj(POBJECT_HEAD root, UINT16 obj_type)
{
    OBJLIST *ol= NULL;
    CONTAINER *con= NULL;
    POBJECT_HEAD oh = NULL;
    POBJECT_HEAD pobj = NULL;
    INT16  i=0;
    INT16 cnt=0;

    MEMSET(&oh,0,sizeof(POBJECT_HEAD));
    MEMSET(&pobj,0,sizeof(POBJECT_HEAD));
    if(OT_CONTAINER == root->b_type)
    {
        con = (CONTAINER*)root;
        oh = (POBJECT_HEAD )(con->p_next_in_cntn);
        while(oh)
        {
            if(oh->b_type == obj_type)
            {
                pobj = oh;
                break;
            }
            oh = oh->p_next;
        }
    }
    else if(OT_OBJLIST == root->b_type)
    {
        ol = (OBJLIST*)root;
        cnt = osd_get_obj_list_count(ol);
        for(i = 0; i < cnt; i++)
        {
            oh = ol->p_list_field[i];
            if((oh!=NULL) &&( oh->b_type == obj_type))
            {
                pobj = oh;
                break;
            }
        }
    }

    return pobj;
}

static BOOL win_otaset_menu_config_dvbs(UINT32 fe_type)
{
	POBJECT_HEAD    pobj = NULL;
	BOOL            ret = TRUE;
	UINT16          i = 0;
	PEDIT_FIELD  pedit = NULL;
	PMULTISEL psel = NULL;
	PTEXT_FIELD ptxt = NULL;

	if(FRONTEND_TYPE_S == fe_type)
	{
		for(i = 0; i < sizeof(otaset_dvbs_menu)/sizeof(otaset_dvbs_menu[0]); i++)
		{
			pobj = otaset_items[i]; 
			osd_move_object(pobj,otaset_dvbs_menu[i].obj_rect.u_left,otaset_dvbs_menu[i].obj_rect.u_top,0);
			osd_set_attr(pobj,otaset_dvbs_menu[i].obj_active);
			switch(i)
			{
				case 1:
				{
				pedit = (PEDIT_FIELD)win_otaset_get_child_obj(pobj,OT_EDITFIELD);
				if(pedit != NULL)
				{
				pedit->pcs_pattern = lnb_pat;
				}
				}
				break;
				case 2:
				{
				pedit = (PEDIT_FIELD)win_otaset_get_child_obj(pobj,OT_EDITFIELD);
				if(pedit != NULL)
				{
				pedit->pcs_pattern = fre_pat;
				}
				}
				break;
				case 3:
				{
				pedit = (PEDIT_FIELD)win_otaset_get_child_obj(pobj,OT_EDITFIELD);
				if(pedit != NULL)
				{
				pedit->pcs_pattern = symb_pat;
				}
				}
				break;
				case 4:
				{
				psel = (PMULTISEL)win_otaset_get_child_obj(pobj,OT_MULTISEL);
				if(psel != NULL)
				{
				psel->n_count = 2;
				psel->p_sel_table = tp_polarity_ids;
				}
				ptxt = (PTEXT_FIELD)win_otaset_get_child_obj(pobj,OT_TEXTFIELD);
				if(ptxt != NULL)
				{
				ptxt->w_string_id = otaset_dvbs_menu[i].obj_strid;
				}
				}
				break;
				default:
				break;
			}
		}
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

static BOOL win_otaset_menu_config_dvbt(UINT32 fe_type)
{
    POBJECT_HEAD    pobj = NULL;
    BOOL            ret = TRUE;
    UINT16          i = 0;
    PEDIT_FIELD  pedit = NULL;
    PMULTISEL psel = NULL;
    PTEXT_FIELD ptxt = NULL;

    if((FRONTEND_TYPE_T == fe_type) ||( FRONTEND_TYPE_ISDBT == fe_type))
    {
        for(i = 0;i < sizeof(otaset_dvbt_menu)/sizeof(otaset_dvbt_menu[0]);i++)
        {
            pobj = otaset_items[i];
            osd_move_object(pobj,otaset_dvbt_menu[i].obj_rect.u_left,otaset_dvbt_menu[i].obj_rect.u_top,0);
            osd_set_attr(pobj,otaset_dvbt_menu[i].obj_active);
            switch(i)
            {
                case 2:
                    {
                    pedit = (PEDIT_FIELD)win_otaset_get_child_obj(pobj,OT_EDITFIELD);
                    if(pedit != NULL)
                    {
                        pedit->pcs_pattern = fre_pat_dvbt;
                    }
                    }
                    break;
                case 3:
                    {
                    pedit = (PEDIT_FIELD)win_otaset_get_child_obj (pobj,OT_EDITFIELD);
                    if(pedit != NULL)
                    {
                        pedit->pcs_pattern = symb_pat_dvbt;
                    }
                    }
                    break;
                case 4:
                    {
                        psel = (PMULTISEL)win_otaset_get_child_obj(pobj,OT_MULTISEL);
                        if(psel != NULL)
                        {
                            psel->n_count = 3;
                            psel->p_sel_table = bandwidth_ids;
                        }
                        ptxt = (PTEXT_FIELD)win_otaset_get_child_obj(pobj,OT_TEXTFIELD);
                        if(ptxt != NULL)
                        {
                            ptxt->w_string_id = otaset_dvbt_menu[i].obj_strid;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        ret = FALSE;
    }
    return ret;
}

static BOOL win_otaset_menu_config_dvbc(UINT32 fe_type)
{
    POBJECT_HEAD    pobj = NULL;
    BOOL            ret = TRUE;
    UINT16          i = 0;
    PEDIT_FIELD  pedit = NULL;
    PMULTISEL psel = NULL;
    PTEXT_FIELD ptxt = NULL;


    MEMSET(&pobj,0,sizeof(POBJECT_HEAD));
    if(FRONTEND_TYPE_C == fe_type)
    {
        for(i = 0;i < sizeof(otaset_dvbc_menu)/sizeof(otaset_dvbc_menu[0]);i++)
        {
            pobj = otaset_items[i];
            osd_move_object(pobj,otaset_dvbc_menu[i].obj_rect.u_left,otaset_dvbc_menu[i].obj_rect.u_top,0);
            osd_set_attr(pobj,otaset_dvbc_menu[i].obj_active);
            switch(i)
            {
                case 2:
                    {
                    pedit = (PEDIT_FIELD)win_otaset_get_child_obj(pobj,OT_EDITFIELD);
                    if(pedit != NULL)
                    {
                        pedit->pcs_pattern = fre_pat_dvbc;
                    }
                    }
                    break;
                case 3:
                    {
                    pedit = (PEDIT_FIELD)win_otaset_get_child_obj(pobj,OT_EDITFIELD);
                    if(pedit != NULL)
                    {
                        pedit->pcs_pattern = symb_pat_dvbc;
                    }

                    }
                    break;
                case 4:
                    {
                        psel = (PMULTISEL)win_otaset_get_child_obj(pobj,OT_MULTISEL);
                        if(psel != NULL)
                        {
                            psel->n_count = 5;
                            psel->p_sel_table = ota_model_type_ids;
                        }
                        ptxt =(PTEXT_FIELD)win_otaset_get_child_obj(pobj,OT_TEXTFIELD);
                        if(ptxt != NULL)
                        {
                            ptxt->w_string_id = otaset_dvbc_menu[i].obj_strid;
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    }
    else
    {
        ret = FALSE;
    }
    return ret;
}

static void win_otaset_menu_config(UINT32 fe_type)
{
   BOOL func_flag=FALSE;

	if(FRONTEND_TYPE_S == fe_type)
    {
        func_flag=win_otaset_menu_config_dvbs(fe_type);
    }
    else if((FRONTEND_TYPE_T == fe_type)||(FRONTEND_TYPE_ISDBT == fe_type))
    {
        func_flag=win_otaset_menu_config_dvbt(fe_type);
    }
    else if(FRONTEND_TYPE_C == fe_type)
    {
        func_flag=win_otaset_menu_config_dvbc(fe_type);
    }
}

static UINT8 win_otaset_signal_refresh_combo(UINT32 fe_type)
{
    UINT16 level = 0;
    UINT8 quality = 0;
    UINT8 lock = 0;
    struct nim_device *nim_dev = NULL;
    //BOOL func_flag=FALSE;
    S_NODE  s_node;
    INT32 func_ret=0;

	MEMSET(&s_node, 0x0, sizeof (s_node));
    if(FRONTEND_TYPE_S == fe_type)
    {

#ifdef _BUILD_OTA_E_
        MEMCPY(&s_node, &m_ota_cfg.s_node, sizeof(s_node));
#else
        get_tuner_sat(OTA_TUNER_IDX, ota_satpos, &s_node);
#endif

		nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (s_node.tuner1_valid ? 0 : (s_node.tuner2_valid ? 1 : 0)));

    }
    else if((FRONTEND_TYPE_T ==fe_type)|| (FRONTEND_TYPE_ISDBT ==fe_type))
    {
        //board config isdbt and dvbt is same.
        if(FRONTEND_TYPE_T == board_get_frontend_type(0))
        {
            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
        }
        else if(FRONTEND_TYPE_T == board_get_frontend_type(1))
        {
            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
        }
    }
    else if(FRONTEND_TYPE_C == fe_type)
    {
        if(fe_type == (UINT32)board_get_frontend_type(0))
        {
            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
        }
        else if(fe_type == (UINT32)board_get_frontend_type(1))
        {
            nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
        }
    }
     func_ret=nim_get_lock(nim_dev, &lock);
     func_ret=nim_get_snr(nim_dev, &quality);
    if((NULL != nim_dev) && (NULL != nim_dev->do_ioctl))
    {
      nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&level);
    }

    dem_signal_to_display(lock,(UINT8*)&level,&quality);
    win_signal_set_level_quality(level,quality,lock);
    win_signal_update();
	return lock;
}
void ota_set_front_type(void)
{
    cur_front_type=board_get_frontend_type(0);//add by Iris for AUTO OTA
}

