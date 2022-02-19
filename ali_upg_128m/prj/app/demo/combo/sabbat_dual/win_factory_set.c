/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_factory_set.c
*
*    Description:   The menu for user set to default setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
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
#ifdef CAS9_VSC
#include <vsc/vsc.h>
#endif
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_factory_set.h"
#include "win_pause.h"
#include "win_mute.h"
#include "control.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

/*******************************************************************************
* Objects declaration
*******************************************************************************/
static VACTION factoryset_btn_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT factoryset_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION factoryset_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT factoryset_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define MTXT_SH_IDX     WSTL_TEXT_09_HD

#define BTN_SH_IDX   WSTL_BUTTON_SHOR_HD
#define BTN_HL_IDX   WSTL_BUTTON_SHOR_HI_HD
#define BTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define BTN_GRY_IDX  WSTL_BUTTON_SHOR_GRY_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define CON_L       W_L
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 46)
#define CON_H       40
#define CON_GAP     12

#define MTXT_L      (W_L + 20)
#define MTXT_T      (W_T + 30)
#define MTXT_W      (W_W - 60)
#define MTXT_H      200

#define BTN_L1      (W_L + 120)
#define BTN_L2      (BTN_L1 + BTN_W + 40)
#define BTN_T       (MTXT_T + MTXT_H + 30)
#define BTN_W       160
#define BTN_H       40
#else
#define W_L         105//384
#define W_T         57//138
#define W_W         482
#define W_H         370

#define CON_L       W_L
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 26)
#define CON_H       30
#define CON_GAP     8

#define MTXT_L      (W_L + 20)
#define MTXT_T      (W_T + 30)
#define MTXT_W      (W_W - 60)
#define MTXT_H      150

#define BTN_L1      (W_L + 60)
#define BTN_L2      (BTN_L1 + BTN_W + 40)
#define BTN_T       (MTXT_T + MTXT_H + 30)
#define BTN_W       100
#define BTN_H       32
#endif

#define MTXT_CTN_CNT  1//(sizeof(factoryset_mtxt_content)/sizeof(TEXT_CONTENT))
#define BTN_YES_ID  1
#define BTN_NO_ID   2

#define LDEF_MTXT(root,var_mtxt,next_obj,l,t,w,h,cnt,content) \
    DEF_MULTITEXT(var_mtxt,&root,next_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, MTXT_SH_IDX,MTXT_SH_IDX,MTXT_SH_IDX,MTXT_SH_IDX,   \
        NULL,NULL,  \
        C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,NULL,content)

#define LDEF_TXT_BTN(root,var_txt,next_obj,l,t,w,h,ID,idl,idr,res_id)      \
    DEF_TEXTFIELD(var_txt,&root,next_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,ID,ID, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,   \
    factoryset_btn_keymap,factoryset_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,res_id,NULL)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    factoryset_keymap,factoryset_callback,  \
    nxt_obj, focus_id,0)

LDEF_MTXT(win_factoryset_con,factoryset_mtxt,&factoryset_btn_ok, \
        MTXT_L, MTXT_T,MTXT_W,MTXT_H,MTXT_CTN_CNT,factoryset_mtxt_content)

LDEF_TXT_BTN(win_factoryset_con,factoryset_btn_ok,&factoryset_btn_cancel, \
        BTN_L1, BTN_T, BTN_W, BTN_H,1,2,2,RS_COMMON_YES)

LDEF_TXT_BTN(win_factoryset_con,factoryset_btn_cancel,NULL, \
        BTN_L2, BTN_T, BTN_W, BTN_H,2,1,1,RS_COMMON_NO)

LDEF_WIN(win_factoryset_con,&factoryset_mtxt,W_L, W_T,W_W,W_H,2)
/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/
TEXT_CONTENT factoryset_mtxt_content[] =
{
    {STRING_ID, {RS_MSG_THIS_OPERATION_WILL_ERASE_ALL}},
};

/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
static VACTION factoryset_btn_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(V_KEY_ENTER == key)
    {
        act = VACT_ENTER;
    }
    return act;
}

static PRESULT factoryset_btn_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //PRESULT ret = PROC_LEAVE;
    VACTION unact= PROC_PASS;
    UINT8 bid= 0;
    UINT8 back_saved= 0;
    UINT32 chunk_id= 0;
    UINT32 default_db_addr= 0;
    UINT32 default_db_len= 0;
    #ifdef CAS9_V6
            UINT8 cache_pin[4]={0};
    #endif

    bid = osd_get_obj_id(pobj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            ret = PROC_LEAVE;
            if(BTN_NO_ID == bid)
            {
                break;
            }

#ifdef SUPPORT_CAS9
        #ifdef MULTI_DESCRAMBLE
        UINT8 i = 0;
        for(i = 0;i < 3;i++)  //stop filter all the TP
        {
            api_mcas_stop_transponder_multi_des(i);
        }
        #else
            api_mcas_stop_transponder();
        #endif
#endif
#if defined(SUPPORT_BC_STD)
            api_mcas_stop_transponder();
#elif defined(SUPPORT_BC)
            UINT8 i = 0;
            for(i = 0;i < 3;i++)  //stop filter all the TP
            {
                api_mcas_stop_transponder_multi_des(i);
            }
#endif

#ifdef DVBC_MODE_CHANGE
            SYSTEM_DATA *sys_data = NULL;
            UINT8 dvbc_mode = 0 ;

            sys_data = sys_data_get();
            dvbc_mode = sys_data->dvbc_mode ;
#endif
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg(NULL,NULL, RS_MSG_SAVING_DATA);
                win_compopup_open_ext(&back_saved);

                chunk_id = 0x03FC0100;
                if(!api_get_chunk_add_len(chunk_id, &default_db_addr,  &default_db_len))
                {
                    PRINTF("api_get_chunk_add_len() failed\n");
                }
                //here may reset CI+ Service Shunning parameter, 20100803
                #ifdef CI_PLUS_SERVICESHUNNING
                ci_service_shunning_reset();
                #endif
                if(DBERR_DEFAULT == set_default_value(DEFAULT_ALL_PROG,default_db_addr + 16))
				{
					win_compopup_smsg_restoreback();
					libc_printf("%s() in line(%d) error!\n",__FUNCTION__,__LINE__);
					break;
                }
                //init_default_value();
            #ifdef CAS9_VSC    //don't reset backup vsc store config
                VSC_STORE_CONFIG config;

                MEMSET(&config, 0, sizeof(config));
                sys_data_get_back_up_vsc_config(&config);
            #endif

            #ifdef CAS9_V6
            MEMSET(cache_pin,0,4);
            sys_data_get_cached_ca_pin(cache_pin);
            #endif
            
                sys_data_factroy_init();

            #ifdef _MHEG5_SUPPORT_
                mheg_fs_format_dev();
                mheg_mem_register_callback();
            #endif
            
            #ifdef CAS9_VSC
                sys_data_set_back_up_vsc_config(config);
            #endif
                sys_data_set_factory_reset(FALSE);
                sys_data_set_cur_chan_mode(TV_CHAN);
                sys_data_save(TRUE);
                sys_data_load();

            #ifdef CAS9_V6
            sys_data_set_cached_ca_pin(cache_pin);
            sys_data_save(TRUE);            
            #endif                
            
                if(get_mute_state())
                {
                    set_mute_on_off(FALSE);
                }
                if(get_pause_state())
                {
                    set_pause_on_off(FALSE);
                }
                win_compopup_smsg_restoreback();
#ifdef BIDIRECTIONAL_OSD_STYLE
                osd_clear_screen();
                osd_draw_object((POBJECT_HEAD)&g_win_mainmenu, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
#endif
                //ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_palntsc,FALSE);
            }
#ifdef _INVW_JUICE
                            inview_factory_reset();
#endif

#ifdef DVBC_MODE_CHANGE
            sys_data->dvbc_mode = dvbc_mode ;
            sys_data_save(TRUE);
#endif

    #ifdef SHOW_WELCOME_SCREEN
            POBJECT_HEAD submenu;
//modify for the title of language submenu show welcome 2011 11 24
//welcome page has been solve by other method, not use the language submenu anymore
            //extern void set_win_lang_as_welcome(BOOL iswelcome);
            //set_win_lang_as_welcome(TRUE);
            submenu = (POBJECT_HEAD) & win_lan_con;
            if (osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
            {
                menu_stack_pop();
                menu_stack_push(submenu);
                ret = PROC_LOOP;
            }
    #endif
        }
        break;
    default:
        break;
    }
    return ret;
}

static VACTION factoryset_keymap(POBJECT_HEAD pobj, UINT32 vkey)
{
    VACTION act = VACT_PASS;

    switch(vkey)
    {
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act= VACT_CLOSE;
    break;
    default:
        break;
    }
    return act;
}

static PRESULT factoryset_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    CONTAINER *cont = (CONTAINER*)pobj;

    switch(event)
    {
        case EVN_PRE_OPEN:
            osd_set_container_focus(cont, BTN_NO_ID);
            wincom_open_title((POBJECT_HEAD)&win_factoryset_con,RS_TOOLS_FACTORY_SETTING, 0);
            break;

        case EVN_POST_OPEN:
            break;

        case EVN_PRE_CLOSE:
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

