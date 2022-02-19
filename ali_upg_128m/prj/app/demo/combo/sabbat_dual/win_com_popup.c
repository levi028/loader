/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_popup.c
*
*    Description: The common function of popup
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
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libtsi/db_3l.h>
#include <api/libdiseqc/lib_diseqc.h>
#include <math.h>

#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "vkey.h"
#include "key.h"
#include "control.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_popup_smsg.h"
#if defined( SUPPORT_CAS9)
#include "conax_ap/win_ca_mmi.h"
#endif

#if defined(SUPPORT_CAS7)
#include "conax_ap7/win_ca_mmi.h"
#endif
#define POP_PRINTF PRINTF
#define POPUP_MAX_TITLE_LEN 30

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
#include "c1700a_ca/win_c1700a_mmi.h"
#endif

#include "win_com_popup_inner.h"

/*******************************************************************************
*   WINDOW's objects declaration
*******************************************************************************/
CONTAINER    g_win_popup;
                //modify for adding welcome page when only open dvbt 2011 10 17

static UINT16 msg_title[POPUP_MAX_MSG_LEN] ={0};
//modify for adding welcome page when only open dvbt 2011 10 17
static TEXT_CONTENT popup_mtxt_content_timer = {STRING_ID, {0}};
static UINT16 msg_timer_str[POPUP_MAX_TIMER_STR_LEN] = {0};
static ID popup_timer_id = OSAL_INVALID_ID;
static UINT16 cnt_poptime = 0;

#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif
static VACTION popup_btn_text_keymap(POBJECT_HEAD p_obj, UINT32 vkey);
static PRESULT popup_btn_text_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION popup_con_keymap(POBJECT_HEAD p_obj, UINT32 vkey);
static PRESULT popup_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static void win_compopup_timer_countdown(void);

/*******************************************************************************
*   WINDOW's objects defintion MACRO
*******************************************************************************/

#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUP_TITLE_SH_IDX, POPUP_TITLE_SH_IDX, 0,0,   \
    NULL, NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_MULTITEXT(root,var_msg,nxt_obj,l,t,w,h)  \
    DEF_MULTITEXT(var_msg,root,nxt_obj,C_ATTR_ACTIVE,0,   \
        0,0,0,0,0, l,t,w,h,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,\
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER,1, 0,0,w,h, NULL,&popup_mtxt_content)

//modify for adding welcome page when only open dvbt 2011 10 17
#define LDEF_MULTITEXT_TIMER(root,var_msg,nxt_obj,l,t,w,h)  \
    DEF_MULTITEXT(var_msg,root,nxt_obj,C_ATTR_ACTIVE,0,   \
    0,0,0,0,0, l,t,w,h,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,  \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER,1, 0,0,w,h, NULL,&popup_mtxt_content_timer)
//modify end

#define LDEF_TXT_BTM(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX, POPUPHL_IDX, 0,0,   \
    popup_btn_text_keymap, popup_btn_text_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_CONTAINER(var_con,nxt_obj,l,t,w,h,focus_id)   \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUPWIN_IDX,POPUPWIN_IDX,0,0,   \
    popup_con_keymap,popup_con_callback,  \
    nxt_obj, focus_id,0)

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
LDEF_TXT_TITLE(&g_win_popup,win_popup_title_txt,&win_popup_msg_mbx, W_L, W_T, W_W, TITLE_H,0,msg_title)
//modify for adding welcome page when only open dvbt 2011 10 17
LDEF_MULTITEXT(&g_win_popup, win_popup_msg_mbx, &win_popup_msg_mbx_timer, MSG_L,MSG_T,MSG_W,MSG_H)
LDEF_MULTITEXT_TIMER(&g_win_popup, win_popup_msg_mbx_timer, &win_popup_yes, MSG_L,MSG_T+MSG_H,MSG_W,MSG_H)
//modify for adding welcome page when only open dvbt 2011 10 17
LDEF_TXT_BTM(&g_win_popup, win_popup_yes, &win_popup_no,    1, 3, 2, 1, 1, BTM_L,BTM_T, BTM_W, BTM_H,  \
    RS_COMMON_YES, NULL)
LDEF_TXT_BTM(&g_win_popup, win_popup_no, &win_popup_cancel, 2, 1, 3, 2, 2, BTM_L + (BTM_W + BTM_GAP)*1,BTM_T, \
    BTM_W, BTM_H,  RS_COMMON_NO, NULL)
LDEF_TXT_BTM(&g_win_popup, win_popup_cancel, NULL,          3, 2, 1, 3, 3, BTM_L + (BTM_W + BTM_GAP)*2,BTM_T, \
    BTM_W, BTM_H,  RS_COMMON_CANCEL, NULL)

LDEF_CONTAINER(g_win_popup, &win_popup_title_txt, W_L, W_T, W_W, W_H, 1)
/*******************************************************************************
*   Local Variable & Function declare
*******************************************************************************/
static struct rect timer_rect;
                //modify for adding welcome page when only open dvbt 2011 10 17

#if (defined(SUPPORT_BC_STD) || defined(SUPPORT_BC))
extern UINT32 gdw_wm_dur;
#endif

static INT8 set_change_focus = -1;

static UINT16 msg_btn_str[3][10] = {{0,},};

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
#include "menus_root.h"
#include <api/libcas/c1700a/c1700a.h>
#include <api/libcas/c1700a/c1700a_mmi.h>

//#define ABEL_PRINTF   libc_printf
#define ABEL_PRINTF

extern TEXT_FIELD win_popup_item_name;
extern TEXT_FIELD win_popup_item_val;
UINT16 msg_popup_item_name[POPUP_MAX_MSG_LEN];
UINT16 msg_popup_item_val[POPUP_MAX_MSG_LEN];

#define ITEM_TXT_L  W_L + MSG_LOFFSET
#define ITEM_TXT_T  BTM_T-BTM_H
#define ITEM_TXT_W  (W_W - MSG_LOFFSET * 2)/2
#define ITEM_TXT_H  TITLE_H

#define ITEM_VAL_L  ITEM_TXT_L+ITEM_TXT_W
#define ITEM_VAL_T  (W_T + W_H - BTM_H*2)
#define ITEM_VAL_W  ITEM_TXT_W
#define ITEM_VAL_H  ITEM_TXT_H

#define LDEF_TXT_ITEM_VAL(root,var_txt,nxt_obj,l,t,w,h,res_id,str)     \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_TEXT_29_HD, WSTL_TEXT_29_HD, 0,0,   \
    NULL, NULL,  \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXT_ITEM_NAME(root,var_txt,nxt_obj,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUPSTR_IDX_SD, POPUPSTR_IDX_SD, 0,0,   \
    NULL, NULL,  \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

LDEF_TXT_ITEM_NAME ( &g_win_popup, win_popup_item_name, &win_popup_yes, ITEM_TXT_L, ITEM_TXT_T, ITEM_TXT_W, \
    ITEM_TXT_H, 0, NULL )
LDEF_TXT_ITEM_VAL ( &g_win_popup, win_popup_item_val, &win_popup_yes, ITEM_VAL_L, ITEM_VAL_T, ITEM_VAL_W, \
    ITEM_VAL_H, 0, NULL )

#define VACT_TVPIN_DEL_ONE_CHAR (VACT_PASS+1)

UINT8           g_input_tvpin_pos = 0;
UINT8           g_input_tvpin_chars[8];
UINT8           g_target_tvpin_for_confirm_ui[8];
UINT8           g_target_parental_control_level_for_confirm_ui;
UINT8           g_tvpin_display[16];
BOOL            g_tvpin_confirm_mode=FALSE;
BOOL            g_tvpin_confirm_proc_exec_result=FALSE;
UINT16          val_xoffset = 0;
BOOL                    g_enter_wrong_pin_mmi = FALSE;

static void _update_popup_item_tvpin_display(void)
{
    switch(g_input_tvpin_pos)
    {
        case 0:
            snprintf(g_tvpin_display,16, "----");
            break;
        case 1:
            snprintf(g_tvpin_display,16,"*---");
            break;
        case 2:
            snprintf(g_tvpin_display,16,"**--");
            break;
        case 3:
            snprintf(g_tvpin_display,16,"***-");
            break;
        case 4:
            snprintf(g_tvpin_display,16,"****");
            break;
    }
    osd_set_text_field_content((POBJECT_HEAD)&win_popup_item_val, STRING_ANSI, g_tvpin_display);
    osd_draw_object((POBJECT_HEAD)&win_popup_item_val,C_UPDATE_ALL);
}
#endif

//modify for adding welcome page when only open dvbt 2011 10 17
static char *win_popup_timer_count[] =
{
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
};
//modify end
static OBJECT_HEAD *win_popup_btm_objs[] =
{
    (OBJECT_HEAD*)&win_popup_yes,
    (OBJECT_HEAD*)&win_popup_no,
    (OBJECT_HEAD*)&win_popup_cancel
};

static UINT16 btn_str_ids[] =
{
    RS_COMMON_YES,
    RS_COMMON_NO,
    RS_COMMON_CANCEL
};

/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
VACTION popup_con_keymap(POBJECT_HEAD p_obj, UINT32 vkey)
{
#ifndef _BUILD_OTA_E_
    UINT32  hwkey __MAYBE_UNUSED__ = 0;
#endif
    VACTION act = VACT_PASS;

    if(win_popup_type!= WIN_POPUP_TYPE_SMSG)
    {
        switch(vkey)
        {
        case V_KEY_MENU:
        case V_KEY_EXIT:
            win_pp_choice = WIN_POP_CHOICE_NULL;
            act = VACT_CLOSE;
            break;
        case V_KEY_LEFT:
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
            if(win_popup_type == WIN_POPUP_TYPE_INPUTPIN)
            {
               act = VACT_TVPIN_DEL_ONE_CHAR;
            }
            else
            {
                act = VACT_CURSOR_LEFT;
            }
#else      
            act = VACT_CURSOR_LEFT;
#endif
            break;
        case V_KEY_RIGHT:
            act = VACT_CURSOR_RIGHT;
            break;
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
            case V_KEY_0:
            case V_KEY_1:
            case V_KEY_2:
            case V_KEY_3:
            case V_KEY_4:
            case V_KEY_5:
            case V_KEY_6:
            case V_KEY_7:
            case V_KEY_8:
            case V_KEY_9:
                act = VACT_NUM_0 + ( vkey - V_KEY_0 );
                break;
#endif
        default:
            break;
        }
    }
    else
    {
     #if    defined( SUPPORT_CAS9) ||defined(SUPPORT_CAS7)
        if(menu_stack_get(0) == (POBJECT_HEAD)&g_win_mainmenu)
        {
            if((V_KEY_EXIT == vkey)||(V_KEY_MENU == vkey)||(V_KEY_LEFT == vkey)\
                    ||(V_KEY_UP == vkey)||(V_KEY_DOWN == vkey))
            {
                cas_pop_stop_timer();
                clean_mmi_cur_msg();
                clean_mmi_msg(CA_MMI_PRI_01_SMC,1);
                act = VACT_CLOSE;
            }
        }
        else
        {
            if((V_KEY_EXIT == vkey)||(V_KEY_UP == vkey)||(V_KEY_DOWN == vkey)||\
                   (V_KEY_MENU == vkey))
            {
                cas_pop_stop_timer();
                clean_mmi_cur_msg();
                clean_mmi_msg(CA_MMI_PRI_01_SMC,1);
                act = VACT_CLOSE;
                if(V_KEY_EXIT != vkey)
                {
                    ap_vk_to_hk(0, vkey, &hwkey);
                    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hwkey, FALSE);
                                    //if exit key got,repatch the messages again
                }
            }
        }
        #else
        if((vkey == V_KEY_EXIT)||(vkey == V_KEY_MENU))
        {
            act = VACT_CLOSE;
        }
        #endif
    }
    return act;
}

PRESULT popup_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    UINT8 unact, input, save;
    UINT8 id = osd_get_obj_id(p_obj);
    UINT16 vkey;
    POBJECT_HEAD new_menu;
    UINT8 i;
    UINT8 ret;
    UINT32 input_tvpin_pos_3 = 3;
    UINT32 input_tvpin_pos_4 = 4;

    vkey = (UINT16)param1;

    if((vkey==V_KEY_EXIT)||(vkey ==  V_KEY_MENU))
    {
        win_compopup_set_enter_wrong_pin_mmi_status(FALSE);
        return PROC_LEAVE;
    }
#endif

    switch(event)
    {
    case EVN_PRE_DRAW:
        if(set_change_focus != -1)
        {
            osd_set_container_focus((PCONTAINER)p_obj, set_change_focus);
        }
        break;
    case EVN_PRE_OPEN:
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        if(menu_stack_get_top())
        {
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
            if( (!api_c1700a_mmi_check_mainmenu_on_top())&&(0 == api_c1700a_osd_get_cas_msg_num()))
#endif
            {
                *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            }
        }
        break;

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
        case EVN_POST_CLOSE:
            on_event_cmsg_popup_is_closed();
            win_compopup_reset_target_tvpin_value();
            break;
        case EVN_UNKNOWN_ACTION:
            unact = ( VACTION ) ( param1 >> 16 ) ;
            if(g_tvpin_confirm_mode)
            {
                if ( unact <= VACT_NUM_9 )
                {
                    if(g_input_tvpin_pos <= input_tvpin_pos_3)
                    {
                        g_input_tvpin_chars[g_input_tvpin_pos] = unact-VACT_NUM_0+'0';
                        g_input_tvpin_pos++;
                        _update_popup_item_tvpin_display();
                    }

                    if(input_tvpin_pos_4 == g_input_tvpin_pos)
                    {
                        if(api_abel_mmi_check_mainmenu_on_top())
                        {
                            if(menu_stack_get_top()==&g_win_change_tv_pin)
                            {
                                ABEL_PRINTF("-[UI] Change TV PIN\n");

                                for(i=0; i<8; i++)
                                {
                                    ABEL_PRINTF("g_target_tvpin_for_confirm_ui[%d]=%c (%d)\n", i, \
                                        g_target_tvpin_for_confirm_ui[i], g_target_tvpin_for_confirm_ui[i]);
                                }

                                if(kms_change_pin(g_input_tvpin_chars, g_target_tvpin_for_confirm_ui)==SUCCESS)
                                {
                                    g_tvpin_confirm_proc_exec_result=TRUE;
                                    ABEL_PRINTF("kms_change_pin()::SUCCESS\n");
                                    ret = SUCCESS;
                                }
                                else
                                {
                                    g_tvpin_confirm_proc_exec_result=FALSE;
                                    ABEL_PRINTF("kms_change_pin()::FAIL\n");
                                    ret = !SUCCESS;
                                }
                            }
                            else if(menu_stack_get_top()==&g_win_parental_control)
                            {
                                ABEL_PRINTF("-[UI] Parental Control\n");

                                if(parental_control(g_input_tvpin_chars,g_target_parental_control_level_for_confirm_ui)\
                                    ==SUCCESS)
                                {
                                    g_tvpin_confirm_proc_exec_result=TRUE;
                                    ABEL_PRINTF("kms_change_pin()::SUCCESS\n");
                                    ret = SUCCESS;
                                }
                                else
                                {
                                    g_tvpin_confirm_proc_exec_result=FALSE;
                                    ABEL_PRINTF("kms_change_pin()::FAIL\n");
                                    ret = !SUCCESS;
                                }
                            }
                            else
                            {
                                ABEL_PRINTF("-[UI] Unknown (should be preview menu)\n");

                                if(user_confirm(TV_PIN, g_input_tvpin_chars)==SUCCESS)              
                                {
                                    g_tvpin_confirm_proc_exec_result=TRUE;
                                    ABEL_PRINTF("kms_change_pin()::SUCCESS\n");
                                    ret = SUCCESS;
                                }
                                else
                                {
                                    g_tvpin_confirm_proc_exec_result=FALSE;
                                    ABEL_PRINTF("kms_change_pin()::FAIL\n");
                                    ret = !SUCCESS;
                                }
                            }
                        }
                        else
                        {
                            ABEL_PRINTF("-[UI] Full Screen Display\n");
                            if(user_confirm(TV_PIN, g_input_tvpin_chars)==SUCCESS)
                            {
                                g_tvpin_confirm_proc_exec_result=TRUE;
                                ABEL_PRINTF("user_confirm()::SUCCESS\n");
                                ret = SUCCESS;
                            }
                            else
                            {
                                g_tvpin_confirm_proc_exec_result=FALSE;
                                ABEL_PRINTF("user_confirm()::FAIL\n");
                                ret = !SUCCESS;
                            }
                        }

                        if(ret != SUCCESS)
                        {
                            g_input_tvpin_pos = 0;
                            _update_popup_item_tvpin_display();
                            return PROC_LOOP;
                        }
                        return PROC_LEAVE;
                    }
                }
                else if(unact == VACT_TVPIN_DEL_ONE_CHAR)
                {
                    if(g_input_tvpin_pos>0)
                    {
                        g_input_tvpin_pos--;
                        g_input_tvpin_chars[g_input_tvpin_pos]=0;
                        _update_popup_item_tvpin_display();
                    }
                }
            }
            return PROC_LOOP;
        break;
#endif
        default :
            break;
    }

    return PROC_PASS;
}
/*******************************************************************************
*   BUTTON --- proc
********************************************************************************/

VACTION popup_btn_text_keymap(POBJECT_HEAD p_obj, UINT32 vkey)
{
    VACTION act = VACT_PASS;

    if(V_KEY_ENTER == vkey)
    {
        act = VACT_ENTER;
    }

    return act;
}

PRESULT popup_btn_text_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    VACTION act = 0;
    UINT16  vkey = 0;
    UINT8   id = 0;

	if(0 == vkey)
	{
		;
	}
	if(EVN_UNKNOWN_ACTION == event)
    {
        id      = p_obj->b_id;
        act     = (UINT8)(param1>>16);
        vkey    = (UINT16)param1;

        if(VACT_ENTER == act)
        {
            switch (id)
            {
                case POPUP_YES_ID:
                    win_pp_choice = WIN_POP_CHOICE_YES;
                    break;
                case POPUP_NO_ID:
                    win_pp_choice = WIN_POP_CHOICE_NO;
                    break;
                case POPUP_CANCEL_ID:
                    win_pp_choice = WIN_POP_CHOICE_CANCEL;
                    break;
                default:
                    win_pp_choice = WIN_POP_CHOICE_NULL;
                    break;
            }
            return PROC_LEAVE;
        }
    }
    return PROC_PASS;
}
/*******************************************************************************
*
*   Window  open,proc
*
*******************************************************************************/
#if !(defined( _BUILD_OTA_E_)||defined( SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
static win_popup_type_t cur_popup_type  = WIN_POPUP_TYPE_SMSG;
#endif

void win_msg_popup_open(void)
{
#if !(defined( _BUILD_OTA_E_) ||defined( SUPPORT_CAS7) || defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_msg_popup_smsg_open();
        return;
    }
#endif

#ifdef TRUE_COLOR_HD_OSD
    if(get_signal_stataus_show())
    {
        show_signal_status_osdon_off(0);
    }
    if(NULL == menu_stack_get_top())
    {
        if(win_pp_opened)
        {
            win_msg_popup_close();
        }
        win_popup_init();
    }
#else
    LPVSCR apvscr;

    apvscr = osd_get_task_vscr(osal_task_get_current_id());
    osd_update_vscr(apvscr);/*Update layer1 vscr*/
    osd_set_device_handle((HANDLE)g_osd_dev2);/*Switch to layer2 device*/
    api_osd_mode_change(OSD_MSG_POPUP);
    osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_ON);
    api_set_deo_layer(1);

    win_popup_init();
    osd_update_vscr(apvscr);
    osd_set_device_handle((HANDLE)g_osd_dev);
#endif
}

void win_compopup_close(void)
{
    OBJECT_HEAD *obj = NULL;

    #if !(defined(_BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_close();
        cur_popup_type = !WIN_POPUP_TYPE_SMSG;
        return;
    }
    #endif
    obj= (OBJECT_HEAD*)&g_win_popup;
    osd_obj_close(obj,C_CLOSE_CLRBACK_FLG);
    win_pp_opened = FALSE;

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    on_event_c1700a_mmi_close();
#endif
}

void win_compopup_refresh(void)
{
#if !(defined( _BUILD_OTA_E_)||defined( SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_refresh();
        return;
    }
#endif
    if(win_pp_opened)
    {
        osd_draw_object((OBJECT_HEAD*)&g_win_popup, C_UPDATE_ALL);
    }
}

void win_msg_popup_close(void)
{
#ifdef _NV_PROJECT_SUPPORT_
    show_signal_status_enable();
#endif
#if !(defined(_BUILD_OTA_E_) ||defined(SUPPORT_CAS7) || defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_msg_popup_smsg_close();
        cur_popup_type = !WIN_POPUP_TYPE_SMSG;
        return;
    }
#endif

#ifdef TRUE_COLOR_HD_OSD
    if(win_pp_opened)
    {
        win_compopup_close();
    }
#else
    LPVSCR apvscr;

    apvscr = osd_get_task_vscr(osal_task_get_current_id());
    osd_update_vscr(apvscr);/*Update layer1 vscr*/
    osd_set_device_handle((HANDLE)g_osd_dev2);/*Switch to layer2 device*/
    api_osd_mode_change(OSD_MSG_POPUP);

    win_compopup_close();
    osd_update_vscr(apvscr);
    api_set_deo_layer(0);
    osddrv_show_on_off((HANDLE)g_osd_dev2,OSDDRV_OFF);
    osd_set_device_handle((HANDLE)g_osd_dev);
#endif
}

BOOL win_msg_popup_opend(void)
{
    return win_pp_opened;
}

/*******************************************************************************
*
*   exported APIs
*
*******************************************************************************/
static UINT8 win_com_popup_frame_set = 0;
void win_compopup_init(win_popup_type_t type)
{
    OBJECT_HEAD *obj = NULL;
    UINT8 i = 0;
    UINT8 btn_num = 0;
    TEXT_FIELD *txt = NULL;
    CONTAINER *win = NULL;
    POBJECT_HEAD pobj_next = NULL;
    UINT8 second_btn = 2;
    UINT8 third_btn = 3;

    #ifdef _INVW_JUICE
        if(menu_stack_get_top() == NULL)
        {
                ap_osd_int_restore();
        }
    #endif
    #if !(defined(_BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY)||(defined(_NV_PROJECT_SUPPORT_)&&!defined(_C0200A_CA_DAL_TEST_)))
    cur_popup_type = type;
    if(cur_popup_type==WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_init(type);
        return;
    }
    #endif
    win = &g_win_popup;
    pp_msg_str[0] = 0x0000;

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    on_event_win_compopup_init();
    g_input_tvpin_pos=0;
    g_tvpin_confirm_mode                =   FALSE;
    g_tvpin_confirm_proc_exec_result    =   FALSE;
    MEMSET(g_input_tvpin_chars, 0xFF, 8);
#endif

    win_popup_type = type;
    win_popup_type = (type<=WIN_POPUP_TYPE_OKNOCANCLE)?type:WIN_POPUP_TYPE_SMSG;
    win_pp_choice = WIN_POP_CHOICE_NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    cnt_poptime = 0;
                //modify for adding welcome page when only open dvbt 2011 10 17
    popup_mtxt_content_timer.text.p_string   = NULL;
    win_pp_choice = WIN_POP_CHOICE_NULL;
    btn_num = win_popup_btm_num[win_popup_type];

    osd_set_container_focus(win, 1);
    win->p_next_in_cntn = (OBJECT_HEAD*)&win_popup_title_txt;
    win_compopup_set_title("Message", NULL, 0);


    obj = (OBJECT_HEAD*)&win_popup_title_txt;
    pobj_next = (POBJECT_HEAD)&win_popup_msg_mbx;
    osd_set_objp_next(obj, pobj_next);
    
    /* Link Window's objects */
    obj = (OBJECT_HEAD*)&win_popup_msg_mbx;
    pobj_next = (POBJECT_HEAD)&win_popup_msg_mbx_timer;
    osd_set_objp_next(obj, pobj_next);

    obj = (OBJECT_HEAD*)&win_popup_msg_mbx_timer;
    pobj_next = (btn_num<=0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
    osd_set_objp_next(obj, pobj_next);

    obj = win_popup_btm_objs[0];
    pobj_next = (btn_num<=1) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[1];
    osd_set_objp_next(obj, pobj_next);

    txt = (TEXT_FIELD*)win_popup_btm_objs[0];
    obj = win_popup_btm_objs[1];
    pobj_next = (btn_num<=2) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[2];
    osd_set_objp_next(obj, pobj_next);

    if(1 == btn_num)
    {
        osd_set_id(win_popup_btm_objs[0], 1, 1, 1, 1, 1);
    }
    else if(second_btn == btn_num)
    {
        osd_set_id(win_popup_btm_objs[0], 1, 2, 2, 1, 1);
        osd_set_id(win_popup_btm_objs[1], 2, 1, 1, 2, 2);
    }
    else if(third_btn == btn_num)
    {
        osd_set_id(win_popup_btm_objs[0], 1, 3, 2, 1, 1);
        osd_set_id(win_popup_btm_objs[1], 2, 1, 3, 2, 2);
        osd_set_id(win_popup_btm_objs[2], 3, 2, 1, 3, 3);
    }

    for(i=0;i<3;i++)
    {
        txt = (TEXT_FIELD*)win_popup_btm_objs[i];
        txt->p_string = NULL;
        osd_set_text_field_content(txt, STRING_ID, (UINT32)btn_str_ids[i]);
    }
    win_com_popup_frame_set = 0;
    win_compopup_set_frame(W_MSG_L,W_MSG_T,W_MSG_W,W_MSG_H);
    win_com_popup_frame_set = 1;
    set_change_focus = -1;
}

void win_compopup_set_title(char *str,char *unistr,UINT16 str_id)
{
    TEXT_FIELD *txt = NULL;
    CONTAINER *win = &g_win_popup;

#if !(defined(_BUILD_OTA_E_) ||defined(SUPPORT_CAS7)  || defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_set_title(str,unistr,str_id);
        return;
    }
    #endif
    txt = &win_popup_title_txt;
    osd_set_text_field_str_point(txt, msg_title);
    if(str!=NULL)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else if(unistr!=NULL)
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
    }
    else
    {
        if(str_id != 0)
        {
            osd_set_text_field_str_point(txt, NULL);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str_id);
        }
        else
        {
            win->p_next_in_cntn = (OBJECT_HEAD*)&win_popup_msg_mbx;
            return;
        }
    }

    osd_set_container_next_obj(win,txt);
}

void win_compopup_set_msg(char *str,char *unistr,UINT16 str_id)
{
    UINT8 btn_num = 0;
    UINT8 lines = 0;
    UINT8 *pstr = NULL;
    POBJECT_HEAD txt = (POBJECT_HEAD)&win_popup_msg_mbx;
    UINT16 totalw = 0;
    UINT16 mw = 0;
    UINT16 mh = 0;
    UINT16 l = 0;
    UINT16 t = 0;
    UINT16 w = 0;
    UINT16 h = 0;
    UINT8   title_flag = 0;
    CONTAINER *win = &g_win_popup;

    #if !(defined(_BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY)||(defined(_NV_PROJECT_SUPPORT_)&&!defined(_C0200A_CA_DAL_TEST_)))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_set_msg(str,unistr,str_id);
        return;
    }
    #endif
    title_flag = (win->p_next_in_cntn == txt)? 0 : 1;

    popup_mtxt_content.b_text_type = STRING_UNICODE;
    popup_mtxt_content.text.p_string   = pp_msg_str;

    if(str!=NULL)
    {
        com_asc_str2uni((UINT8 *)str, pp_msg_str);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char( (UINT8*)pp_msg_str,(UINT8*)unistr);
    }
    else
    {
        popup_mtxt_content.b_text_type        = STRING_ID;
        popup_mtxt_content.text.w_string_id   = str_id;
    }

    btn_num = win_popup_btm_num[win_popup_type];
    if(str != NULL || unistr!= NULL)
    {
        pstr = (UINT8*)pp_msg_str;
    }
    else
    {
        pstr = osd_get_unicode_string(str_id);
    }
    totalw = osd_multi_font_lib_str_max_hw(pstr,txt->b_font,&mw,&mh,0);
    totalw += MSG_LOFFSET*2;

    if(btn_num <= 1)
    {
        w = MIN_WIDHT_1ORNOBTN;
    }
    else
    {
        w = MIN_WIDTH_2BTN;
    }

    while (1)
    {
        if ((totalw + w - 1) % w != 0)
        {
            lines = (totalw + w - 1) / w + 1;
        }
        else
        {
            lines = (totalw + w - 2) / w;
        }
        if (lines <= 1)
        {
            lines = 1;
            break;
        }
        else
        {
            if (MAX_LEN_ONE_LINE != w)
            {
                w = MAX_LEN_ONE_LINE;
                continue;
            }
            break;
        }
    }

    if(mh < DEFAULT_FONT_H)
    {
        mh = DEFAULT_FONT_H;
    }
    h =  mh * lines + (lines  - 1)*4 ;
    if(0 == btn_num)
    {
        h += MSG_VOFFSET_NOBTN*2;
    }
    else
    {
        h += (MSG_VOFFSET_WITHBTN + MSG_BUTTON_TOPOFFSET + MSG_BUTTON_BOTOFFSET + BTM_H);
    }

    if(title_flag)
    {
        h += TITLE_H;
    }
	#if 0//((defined(SUPPORT_CAS9)||defined(SUPPORT_CAS7) || defined(SUPPORT_CAS_A) ||defined(SUPPORT_BC)||defined(FTA_ONLY)||(defined(_NV_PROJECT_SUPPORT_)&&!defined(_C0200A_CA_DAL_TEST_))) && (!defined SD_UI) )
        l = 290;
        t = 250;
    #else
        l = (OSD_MAX_WIDTH - w)/2;
        t = (OSD_MAX_HEIGHT - h)/2;
        t = (t + 1) /2 * 2;
    #endif

    win_compopup_set_frame(l,t,w,h);
}

void win_compopup_set_msg_ext(char *str,char *unistr,UINT16 str_id)
{
    UINT8 *pstr = NULL;
    UINT16 *retval = NULL;

	if(NULL == retval)
	{
		;
	}
	if(NULL == pstr)
	{
		;
	}
	#if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_set_msg_ext(str,unistr,NULL,NULL,str_id);
        return;
    }
    #endif
    popup_mtxt_content.b_text_type = STRING_UNICODE;
    popup_mtxt_content.text.p_string   = pp_msg_str;

    if(str!=NULL)
    {
        retval = com_str2uni_str_ext(pp_msg_str, str, POPUP_MAX_MSG_LEN-1);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char_n((UINT8*)pp_msg_str, (UINT8*)unistr, POPUP_MAX_MSG_LEN-1);
    }
    else
    {
        popup_mtxt_content.b_text_type        = STRING_ID;
        popup_mtxt_content.text.w_string_id   = str_id;
    }

    if(str != NULL || unistr!= NULL)
    {
        pstr = (UINT8*)pp_msg_str;
    }
    else
    {
        pstr = osd_get_unicode_string(str_id);
    }
}

void win_compopup_set_msg_utf(char *str,char *unistr,char *utf8, char *utf16,UINT16 str_id)
{
    UINT8 *pstr = NULL;
    UINT16 *retval = NULL;

	if(NULL == pstr)
	{
		;
	}
	if(NULL == retval)
	{
		;
	}
	#if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_set_msg_ext(str,unistr,utf8,utf16,str_id);
        return;
    }
    #endif
    popup_mtxt_content.b_text_type = STRING_UNICODE;
    popup_mtxt_content.text.p_string   = pp_msg_str;

    if(str!=NULL)
    {
        retval = com_str2uni_str_ext(pp_msg_str, str, POPUP_MAX_MSG_LEN-1);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char_n((UINT8*)pp_msg_str, (UINT8*)unistr, POPUP_MAX_MSG_LEN-1);
    }
    else if(utf8!=NULL)
    {
    com_utf8str2uni((UINT8*)utf8, pp_msg_str);
    }
    else
    {
        popup_mtxt_content.b_text_type        = STRING_ID;
        popup_mtxt_content.text.w_string_id   = str_id;
    }

    if(str != NULL || unistr!= NULL)
    {
        pstr = (UINT8*)pp_msg_str;
    }
    else
    {
        pstr = osd_get_unicode_string(str_id);
    }
}

void win_compopup_set_btnstr(UINT32 btn_idx, UINT16 str_id)
{
    TEXT_FIELD *txt = NULL;
    UINT32 max_btn_idx = 3;

    if(btn_idx < max_btn_idx)
    {
        txt = (TEXT_FIELD*)win_popup_btm_objs[btn_idx];
        osd_set_text_field_content(txt, STRING_ID, (UINT32)str_id);
    }
}

void win_compopup_set_btnstr_ext(UINT32 btn_idx, char *str)
{
    TEXT_FIELD *txt = NULL;
    char temp_str[10] = {0};
    UINT32 max_btn_idx = 3;

    if(btn_idx < max_btn_idx)
    {
        txt = (TEXT_FIELD*)win_popup_btm_objs[btn_idx];
        snprintf(temp_str,10, "%s", str);
        txt->p_string = msg_btn_str[btn_idx];
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)temp_str);
    }
}

void win_compopup_set_frame(UINT16 x,UINT16 y,UINT16 w,UINT16 h)
{
    OBJECT_HEAD *obj = NULL;
    UINT8 i = 0;
    UINT8 btn_num = 0;
    UINT16 btn_left = 0;
    UINT16 btn_top = 0;
    UINT16 btm_width = 0;
    UINT16 topoffset = 0;
    UINT16 botoffset = 0;
    const UINT8 second_btn = 2;

    OBJECT_HEAD *objmbx = NULL;
    OBJECT_HEAD *objmbx_timer = NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    MULTI_TEXT *mbx = NULL;
    MULTI_TEXT *mbx_timer = NULL;
                //modify for adding welcome page when only open dvbt 2011 10 17
    UINT8   title_flag = 0;
    CONTAINER *win = &g_win_popup;
    TEXT_FIELD *txt_title = &win_popup_title_txt;

    #if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_set_frame(x,y,w,h);
        return;
    }
    #endif
#ifdef SD_UI
    if(x+w>=OSD_MAX_WIDTH)
    {
        w = OSD_MAX_WIDTH-x-80;
    }
    if(y+h>=OSD_MAX_HEIGHT)
    {
        h = OSD_MAX_HEIGHT-y-80;
    }
#endif

    title_flag = (osd_get_container_next_obj(win) == (POBJECT_HEAD)txt_title)?1:0;
    if(title_flag)
    {
        osd_set_rect(&txt_title->head.frame, x, y, w, TITLE_H);
    }

    obj = (OBJECT_HEAD*)&g_win_popup;
    osd_set_rect(&obj->frame, x,y,w,h);
    btn_num = win_popup_btm_num[win_popup_type];
    objmbx = (OBJECT_HEAD*)&win_popup_msg_mbx;
    objmbx_timer = (OBJECT_HEAD*)&win_popup_msg_mbx_timer;
                //modify for adding welcome page when only open dvbt 2011 10 17
    mbx = &win_popup_msg_mbx;
    mbx_timer = &win_popup_msg_mbx_timer;
                //modify for adding welcome page when only open dvbt 2011 10 17

    if(0 == btn_num)
    {
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
        if(win_popup_type == WIN_POPUP_TYPE_INPUTPIN)
        {
            mbx->b_align = C_ALIGN_LEFT| C_ALIGN_TOP;
        }
        else
        {
            mbx->b_align = C_ALIGN_CENTER | C_ALIGN_VCENTER;
        }
#else
        mbx->b_align = C_ALIGN_CENTER | C_ALIGN_VCENTER;
#endif
        topoffset = MSG_VOFFSET_NOBTN;
        if(h - topoffset*2 < DEFAULT_FONT_H)
        {
            topoffset = (h - DEFAULT_FONT_H)/2;
        }

        botoffset=30;
        if(title_flag)
        {
            topoffset = TITLE_H;
        }
    }
    else
    {
        mbx->b_align = C_ALIGN_CENTER | C_ALIGN_VCENTER;
        topoffset = MSG_VOFFSET_NOBTN;//h/5;
        botoffset = 30;
        if(title_flag)
        {
            topoffset = TITLE_H;
        }
    }

    osd_set_obj_rect(objmbx,x + MSG_LOFFSET,y + topoffset,w - MSG_LOFFSET*2, h - botoffset - topoffset);

    //modify for adding welcome page when only open dvbt 2011 10 17
    timer_rect.u_start_x = x + MSG_LOFFSET+5;
    timer_rect.u_start_y = y + topoffset;
    timer_rect.u_width  = w - MSG_LOFFSET*2-10;
    timer_rect.u_height = h - botoffset - topoffset;
    osd_set_obj_rect(objmbx_timer,x + MSG_LOFFSET+5,y + topoffset+40,\
                    w - MSG_LOFFSET*2-10,0);
    //modify end
    mbx->rc_text.u_width  = objmbx->frame.u_width;
    mbx->rc_text.u_height = objmbx->frame.u_height;
    //modify for adding welcome page when only open dvbt 2011 10 17
    mbx_timer->rc_text.u_width  = objmbx->frame.u_width;
    mbx_timer->rc_text.u_height = 40/*objmbx->frame.uHeight*/;
    //modify end

    for(i=0;i<btn_num;i++)
    {
        obj = win_popup_btm_objs[i];
        btm_width = BTM_W;

        if(1 == btn_num)
        {
            btn_left = x + (w - btm_width)/2;
        }
        else if(second_btn == btn_num)
        {
            btn_left = x + ( w - btm_width*btn_num - BTM_GAP)/2 + (btm_width + BTM_GAP) *i ;
        }
        else
        {
            btn_left = x +  w - (btm_width + BTM_GAP) * ( btn_num - i) - 40;
        }
        btn_top = h - MSG_BUTTON_BOTOFFSET - BTM_H;
        osd_set_rect(&obj->frame, btn_left,(y + btn_top),btm_width,BTM_H);
    }

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    TEXT_FIELD *txt_item_name   = &win_popup_item_name;
    TEXT_FIELD *txt_item_val        = &win_popup_item_val;

    if(osd_get_objp_next(objmbx)==&win_popup_item_name)
    {
        // ITEM_NAME_W = 2/3 Width, ITEM_VAL_W = 1/3 Width
        osd_set_rect(&txt_item_name->head.frame, x + MSG_LOFFSET, y + h - \
        MSG_BUTTON_BOTOFFSET-BTM_H-BTM_GAP,((w-MSG_LOFFSET*2)*2)/3,ITEM_TXT_H);

        obj = &win_popup_item_name;
        if(osd_get_objp_next(obj)==&win_popup_item_val)
        {
            osd_set_rect(&txt_item_val->head.frame, x + MSG_LOFFSET+ ((w - MSG_LOFFSET * 2)*2)/3 - val_xoffset, \
                y + h - MSG_BUTTON_BOTOFFSET - BTM_H  - BTM_GAP ,  ((w - MSG_LOFFSET * 2))/3, ITEM_VAL_H);
        }
    }
    else if(osd_get_objp_next(objmbx)==&win_popup_item_val)
    {
        osd_set_rect(&txt_item_val->head.frame, x + MSG_LOFFSET+(w - MSG_LOFFSET * 2)/2, \
            y + h - MSG_BUTTON_BOTOFFSET - BTM_H*2 - BTM_GAP, (w - MSG_LOFFSET * 2)/2, TITLE_H);
    }
#endif
}

void win_compopup_set_pos(UINT16 x,UINT16 y)
{
    #if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_set_pos(x,y);
        return;
    }
    #endif
    osd_move_object((POBJECT_HEAD)&g_win_popup, x, y, FALSE);
}

void win_compopup_set_default_choice(win_popup_choice_t choice)
{
    win_pp_choice = choice;
    g_win_popup.focus_object_id = win_pp_choice ;
}

 win_popup_choice_t win_compopup_open(void)
{
    CONTAINER *con =&g_win_popup;
    PRESULT b_result = PROC_LOOP;
    UINT32 hkey = 0;
    UINT ct =cnt_poptime;
    BOOL old_value = FALSE;

    #if !(defined(_BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_open();
        return WIN_POP_CHOICE_NULL;
    }
    #endif

    win_msg_popup_close();
#ifdef CI_PLUS_SUPPORT
    if(is_ciplus_menu_exist())
    {
        return WIN_POP_CHOICE_NULL;
    }
#endif
#ifdef _MHEG5_SUPPORT_
    if(mheg_app_avaliable())
    {
        return WIN_POP_CHOICE_NULL;
    }
#endif

#if (defined(SUPPORT_BC_STD) && defined(BC_PATCH_UI) )
                              //closing windows when show order pin or check pin
    extern UINT8 g_order_pin_dialog;
    DWORD start_tick;
    start_tick=osal_get_tick();
#elif defined(SUPPORT_BC)
    extern UINT8 g_order_pin_dialog;
    DWORD start_tick;
    start_tick=osal_get_tick();
#endif

    win_popup_init();
    if(win_popup_type!=WIN_POPUP_TYPE_SMSG)
    {
        old_value = FALSE;
        old_value  = ap_enable_key_task_get_key(TRUE);
        while(b_result != PROC_LEAVE)
        {
#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
            if(api_c1700a_mmi_check_mmi_in_progress()&&(menu_stack_get_top()==\
                (POBJECT_HEAD)&g_win2_progname))
            {
            /* process win_progname first, then process abel user confirm mmi :
               solution: clear g_win_progname first then process user
               confirm mmi. */
                osal_task_sleep(1500);
                osd_clear_object((POBJECT_HEAD)&g_win2_progname,C_UPDATE_ALL);
                menu_stack_pop();
            }
#endif
        #if (defined(SUPPORT_BC_STD) || defined(SUPPORT_BC))
        osm_api_osd_display_window_control_proc();
        #endif

            hkey = ap_get_key_msg();
            if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
            {
                continue;
            }

            #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
            if(CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER == hkey)
            {
                //libc_printf("%s-%d Got Msg CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_NR_OSM_TIMER\n",__FILE__,__LINE__);
                handle_nonremovable_osm_about_timer();
            }
            else if (CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER == hkey)
            {
                //libc_printf("%s-%d Got Msg CTRL_MSG_SUBTYPE_CMD_OSM_HANDLE_WATERMARKING_TIMER\n",__FILE__,__LINE__);
                _osm_api_osd_display_window_open();
            }
            else if (CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING == hkey)
            {
                //libc_printf("%s-%d Got Msg CTRL_MSG_SUBTYPE_CMD_OSM_WATERMARKING\n",__FILE__,__LINE__);
                watermarking(gdw_wm_dur);
            }
            #endif

            #if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
                UINT32 vkey;
                ap_hk_to_vk(0, hkey, &vkey);
            #endif

            #if (defined(SUPPORT_BC_STD)&& defined(BC_PATCH_UI))
            DWORD tick_threshold = 600;

            if((osal_get_tick()-start_tick<tick_threshold)||(g_order_pin_dialog!=TRUE))
            {                  //closing windows when show order pin or check pin
                    if ((V_KEY_UP == vkey) || (V_KEY_DOWN == vkey))
                    {
                        vkey=V_KEY_NULL;
                    }
            }
            #elif defined(SUPPORT_BC)
            DWORD tick_threshold = 600;

            if((osal_get_tick()-start_tick<tick_threshold)||(g_order_pin_dialog!=TRUE))
            {              //closing windows when show order pin or check pin
                if ((V_KEY_UP == vkey) || (V_KEY_DOWN == vkey))
                {
                    vkey=V_KEY_NULL;
                }
            }
            #endif
            //modify for adding welcome page when only open dvbt 2011 10 16
            if ((CTRL_MSG_TYPE_POPUP_TIMER == hkey) || (CTRL_MSG_SUBTYPE_CMD_PVR_AUTO_STANDBY_TIMER == hkey))
            {
                if (ct != 0)
                {
                    com_asc_str2uni((UINT8 *)win_popup_timer_count[ct-1], msg_timer_str);
                    popup_mtxt_content_timer.b_text_type = STRING_ANSI; //STRING_UNICODE;
                    popup_mtxt_content_timer.text.p_string   = msg_timer_str;
                    osd_track_object((POBJECT_HEAD)&g_win_popup,C_UPDATE_ALL);
                    ct--;
                }
                else
                {
                    popup_mtxt_content_timer.text.p_string   = NULL;
                    osd_track_object((POBJECT_HEAD)&g_win_popup,C_UPDATE_ALL);
                }

                if (0 == ct)
                {
                    api_stop_timer(&popup_timer_id);
                    ap_enable_key_task_get_key(old_value);
                    if (CTRL_MSG_TYPE_POPUP_TIMER == hkey)
                    {
                        return WIN_POP_CHOICE_NO;
                    }
                    else
                    {
                        return win_pp_choice;
                    }
                }
            }
        #if defined(SUPPORT_BC_STD)
            else if((CTRL_MSG_SUBTYPE_CMD_EXIT == hkey) || (vkey == V_KEY_UP) || (vkey==V_KEY_DOWN))
            {   //issue: Order not remove when event finish
                #if defined(BC_PATCH_UI)
                if(g_order_pin_dialog==TRUE)   //closing windows when show order pin or check pin
                {
                #endif
                if (vkey == V_KEY_UP)
                {
                    change_channel(1);
                }
                else if (vkey == V_KEY_DOWN)
                {
                    change_channel(-1);
                }
                #if defined(BC_PATCH_UI)
                }
                #endif
                ap_enable_key_task_get_key(old_value);
                return WIN_POP_CHOICE_NO;
            }
        #elif defined(SUPPORT_BC)
            else if((CTRL_MSG_SUBTYPE_CMD_EXIT == hkey) || (vkey == V_KEY_UP) || (vkey==V_KEY_DOWN))
            { //issue: Order not remove when event finish
                if(g_order_pin_dialog==TRUE)   //closing windows when show order pin or check pin
                {
                    if (vkey == V_KEY_UP)
                    {
                        change_channel(1);
                    }
                    else if (vkey == V_KEY_DOWN)
                    {
                        change_channel(-1);
                    }
                }
                ap_enable_key_task_get_key(old_value);
                return WIN_POP_CHOICE_NO;
            }
        #endif
            else
            {
                b_result = osd_obj_proc((POBJECT_HEAD)con, (MSG_TYPE_KEY<<16),hkey,0);
            }
        }
        api_stop_timer(&popup_timer_id);
        //modify end
        ap_enable_key_task_get_key(old_value);
    }

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
    on_event_c1700a_mmi_close();
#endif

    return win_pp_choice;
}

win_popup_choice_t win_com_popup_open(win_popup_type_t type,char *str,UINT16 str_id)
{
    #if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_com_popup_smsg_open(type,str,str_id);
        return WIN_POP_CHOICE_NULL;
    }
    #endif
    win_compopup_init(type);
    win_compopup_set_msg(str,NULL,str_id);
    return win_compopup_open();
}

win_popup_choice_t win_compopup_open_ext(UINT8 *back_saved)
{
    win_popup_choice_t ret = WIN_POP_CHOICE_NO;
    win_popup_type_t  type = win_popup_type;

#ifdef _NV_PROJECT_SUPPORT_
    show_signal_status_disable();
#endif

  #if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_open_ext();
        return WIN_POP_CHOICE_NULL;
    }
    #endif
    wincom_backup_region(&g_win_popup.head.frame);
    ret = win_compopup_open();
    if(type != WIN_POPUP_TYPE_SMSG)
    {
        *back_saved = 1;
        if(win_pp_opened)
        {
            win_msg_popup_close();
        }
        wincom_restore_region();
    }
    else
    {
        *back_saved = 0;
    }

    return ret;
}

win_popup_choice_t win_compopup_open_enh(POBJECT_HEAD root_obj, UINT32 draw_mode, UINT32 cmd)
{
    win_popup_choice_t ret = WIN_POP_CHOICE_NO;
    struct osdrect *frame_ptr = NULL;
    POBJECT_HEAD obj = NULL;
    UINT16 x = 0;
    UINT16 y = 0;
    BOOL adjust_xy = FALSE;

    if(NULL == root_obj)
    {
        ASSERT(0);
        return WIN_POP_CHOICE_NULL;
    }

    frame_ptr = &(root_obj->frame);
    obj = (POBJECT_HEAD)&g_win_popup;

    if((obj->frame.u_height > frame_ptr->u_height) || (obj->frame.u_width > frame_ptr->u_width))
    {
        return WIN_POP_CHOICE_NULL;
    }

    x = obj->frame.u_left;
    y = obj->frame.u_top;
    if(obj->frame.u_top < frame_ptr->u_top)
    {
        x = frame_ptr->u_top;
        adjust_xy = TRUE;
    }
    if(obj->frame.u_left < frame_ptr->u_left)
    {
        y = frame_ptr->u_left;
        adjust_xy = TRUE;
    }
    if(adjust_xy)
    {
        win_compopup_set_frame(x,y,obj->frame.u_width,obj->frame.u_height);
    }

    ret = win_compopup_open();

    //if(root_obj)
    //{
        if(draw_mode&0x01)
        {
            osd_track_object(root_obj,cmd);
        }
        else// if(!(draw_mode&0x01))
        {
            osd_draw_object(root_obj,cmd);
        }
    //}
    return ret;
}

void win_compopup_smsg_restoreback(void)
{
#ifdef _NV_PROJECT_SUPPORT_
    show_signal_status_enable();
#endif
    #if !(defined( _BUILD_OTA_E_)||defined(SUPPORT_CAS7)||defined(SUPPORT_CAS9)||defined(SUPPORT_BC)||defined(FTA_ONLY))
    if(cur_popup_type == WIN_POPUP_TYPE_SMSG)
    {
        win_compopup_smsg_smsg_restoreback();
        return;
    }
    #endif
    if(win_pp_opened)
    {
        win_msg_popup_close();
    }
    wincom_restore_region();
}

/////////////////////////////////////////////
static ID wincom_dish_move_timer_id = OSAL_INVALID_ID;
static BOOL movetime_expire = FALSE;
static void wincom_dish_move_timerhandler(void)
{
    if(wincom_dish_move_timer_id!= OSAL_INVALID_ID)
    {
        movetime_expire = TRUE;
    }
}

static BOOL wincom_dish_move_position_check(S_NODE *pcur_s_node, UINT8 positioner_type, UINT8 position,
    UINT8 *back_saved, UINT8 *s_back)
{
    usals_local local;
    double __MAYBE_UNUSED__ sat_orbit = 0;
    double __MAYBE_UNUSED__ local_longitude = 0;
    double __MAYBE_UNUSED__ local_latitude = 0;
    double xx_angle = 0;
    const UINT8 type_diseqc1_3 = 2;
    const double angle_degree_90 = 90;

    MEMSET(&local, 0x0, sizeof(usals_local));
    if(0 == positioner_type)
    {
        return FALSE;
    }
    else if(1 == positioner_type)  /* DisEqc 1.2 */
    {
        if(0 == position)   /* NULL*/
        {
            return FALSE;
        }
    }
    else if(type_diseqc1_3 == positioner_type)   /* DisEqc 1.3 */
    {
        sys_data_get_local_position(&local);
        sat_orbit = api_usals_get_sat_orbit(pcur_s_node->sat_orbit);
        local_longitude = api_usals_local_word2double(local.local_longitude);
        local_latitude  = api_usals_local_word2double(local.local_latitude);
        xx_angle = api_usals_target_formula_calcuate(sat_orbit,local_longitude, local_latitude);

        if(xx_angle > angle_degree_90)
        {
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(150, 120, 300, 80);
            win_compopup_set_msg(NULL, NULL, RS_MSG_OUT_RANGE);
            win_compopup_open_ext(s_back);
            *back_saved = *s_back;
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void wincom_dish_move_popup_open(UINT16 pre_sat_id,UINT16 cur_sat_id, UINT32 tuner_idx,UINT8 *back_saved)
{
    S_NODE pre_s_node;
    S_NODE cur_s_node;
    UINT16 str[100] = {0};
    UINT16 n_strlen = 0;
    UINT8 save_back = 0;
    UINT32 hkey = INVALID_HK;
    UINT32 vkey = V_KEY_NULL;
    UINT8 signal_lock = 0;
    UINT32 wait_time = 0;
    INT32 preret = 0;
    INT32 curret = 0;
    UINT32 show_pre_sat = 0;
    struct nim_device *nim_dev = NULL;
    BOOL bret = FALSE;
    const UINT8 type_diseqc1_3 = 2;
    const UINT32 max_wait_time = 20;
    BOOL old_value = 0;

    MEMSET(&pre_s_node,0, sizeof(S_NODE));
    MEMSET(&cur_s_node,0, sizeof(S_NODE));
#ifdef NEW_DEMO_FRAME
    struct ft_antenna antenna_pre;
    struct ft_antenna antenna_cur;

    MEMSET(&antenna_pre,0, sizeof(struct ft_antenna));
    MEMSET(&antenna_cur,0, sizeof(struct ft_antenna));
#else
    struct cc_antenna_info antenna_pre;
    struct cc_antenna_info antenna_cur;

    MEMSET(&antenna_pre,0, sizeof(struct cc_antenna_info));
    MEMSET(&antenna_cur,0, sizeof(struct cc_antenna_info));
#endif

    /* Signal detect */
    if(0 == tuner_idx)
    {
        nim_dev = g_nim_dev;
    }
    else
    {
        nim_dev = g_nim_dev2;
    }
    if(FRONTEND_TYPE_S != dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM))
    {
        return;
    }

    *back_saved = 1;

    if((pre_sat_id==cur_sat_id)||(0xFFFF == cur_sat_id))
    {       /*||pre_sat_pos==INVALID_POS_NUM*/
        return;
    }

    preret = get_sat_by_id(pre_sat_id,&pre_s_node);
    curret = get_sat_by_id(cur_sat_id,&cur_s_node);
    if(curret != SUCCESS)
    {
        return;
    }

#ifdef DVBS_SUPPORT
  #ifdef NEW_DEMO_FRAME
    sat2antenna(&cur_s_node, &antenna_cur);
  #else
    sat2antenna_ext(&cur_s_node, &antenna_cur, tuner_idx);
  #endif
#endif

    bret = wincom_dish_move_position_check(&cur_s_node, antenna_cur.positioner_type, antenna_cur.position,
        back_saved, &save_back);
    if (TRUE != bret)
    {
        return;
    }

    if((0xFFFF != pre_sat_id) && (SUCCESS == preret))
    {
#ifdef DVBS_SUPPORT
  #ifdef NEW_DEMO_FRAME
      sat2antenna(&pre_s_node, &antenna_pre);
  #else
      sat2antenna_ext(&pre_s_node, &antenna_pre, tuner_idx);
  #endif
#endif

        if(1 == antenna_pre.positioner_type)  /* DisEqc 1.2 */
        {
            if((1 == antenna_cur.positioner_type) && (antenna_pre.position == antenna_cur.position))
            {
                return;
            }
            else if(antenna_pre.position != 0)
            {
                show_pre_sat = 1;
            }
        }
        else if(type_diseqc1_3 == antenna_pre.positioner_type)   /* DisEqc 1.3 */
        {
            if((type_diseqc1_3 == antenna_cur.positioner_type) && (pre_s_node.sat_orbit == cur_s_node.sat_orbit))
            {
                return;
            }
            else
            {
                show_pre_sat = 1;
            }
        }
   }

    old_value = ap_enable_key_task_get_key(TRUE);

    MEMSET(str, 0, sizeof(str));
#ifndef DB_USE_UNICODE_STRING
    strncpy(str, "Moving dish,please wait...      ",99);
    n_strlen = STRLEN(str);
    if(pre_sat_id != 0xFFFF && pre_s_node.positioner_type!=0)
    {
        strncpy(&str[n_strlen], pre_s_node.sat_name, (99-n_strlen));
        n_strlen = STRLEN(str);
    }
    strncpy(&str[n_strlen], "-->>", (99-n_strlen));
    n_strlen = STRLEN(str);
    strncpy(&str[n_strlen], cur_s_node.sat_name, (99-n_strlen));
#else
    com_asc_str2uni((UINT8 *)"Moving dish,please wait...      ",str);
    n_strlen = com_uni_str_len(str);
    if(show_pre_sat)
    {
        com_uni_str_copy_char((UINT8*)&str[n_strlen], pre_s_node.sat_name);
        n_strlen = com_uni_str_len(str);
    }
    if(!show_pre_sat)
    {
        com_asc_str2uni((UINT8 *)"\n-->>",&str[n_strlen]);
    }
    else
    {
        com_asc_str2uni((UINT8 *)"-->>",&str[n_strlen]);
    }
    n_strlen = com_uni_str_len(str);
    com_uni_str_copy_char((UINT8*)&str[n_strlen], cur_s_node.sat_name);
    n_strlen = com_uni_str_len(str);
#endif

    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_frame(100, 140,400, 80);
#ifndef DB_USE_UNICODE_STRING
    win_compopup_set_msg("Moving dish...", NULL,0);
#else
    win_compopup_set_msg("Moving dish...", NULL,0);
#endif

    movetime_expire = FALSE;
    wincom_dish_move_timer_id = api_start_timer("dish move", 20000, \
                                (OSAL_T_TIMER_FUNC_PTR)wincom_dish_move_timerhandler);

    win_compopup_open_ext(back_saved);

    while(!( ((V_KEY_EXIT == vkey) || (V_KEY_MENU == vkey) || (V_KEY_RIGHT == vkey) || (V_KEY_LEFT == vkey))  \
                ||(0 != signal_lock) ||movetime_expire))
    {
        hkey = ap_get_key_msg();    /* wait for 50ms */
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }
        ap_hk_to_vk(0,hkey, &vkey);

        if(0 == cur_tuner_idx)
        {
            nim_dev = g_nim_dev;
        }
        else
        {
            nim_dev = g_nim_dev2;
        }

        if(wait_time>=max_wait_time)
        {
            nim_get_lock(nim_dev, &signal_lock);
        }
        wait_time++;
    }

    if((V_KEY_RIGHT == vkey) || (V_KEY_LEFT == vkey))
    {
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);
    }

    api_stop_timer(&wincom_dish_move_timer_id);
    win_compopup_smsg_restoreback();

    *back_saved = save_back;
    ap_enable_key_task_get_key(old_value);
}

#ifdef PVR_AUTO_STANDBY_SUPPORT
static void win_compopup_pvr_auto_standby_timer_countdown(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PVR_AUTO_STANDBY_TIMER, CTRL_MSG_SUBTYPE_CMD_PVR_AUTO_STANDBY_TIMER,FALSE);
}
#endif
//modify for adding welcome page when only open dvbt2011 10 17
// unit of time as ms, unit of TimeOut as Sec.
void win_compopup_set_timer(UINT32 time, UINT32 msgtype, UINT16 timeout)
{

    if(CTRL_MSG_TYPE_POPUP_TIMER==msgtype)
    {
        osd_set_obj_rect((OBJECT_HEAD*)&win_popup_msg_mbx_timer,\
                timer_rect.u_start_x,timer_rect.u_start_y+40,timer_rect.u_width,40);
        popup_timer_id = api_start_cycletimer(NULL, time, (OSAL_T_TIMER_FUNC_PTR)win_compopup_timer_countdown);
    }
#ifdef PVR_AUTO_STANDBY_SUPPORT
    else if(CTRL_MSG_SUBTYPE_CMD_PVR_AUTO_STANDBY_TIMER==msgtype)
    {
        osd_set_obj_rect((OBJECT_HEAD*)&win_popup_msg_mbx_timer,\
                timer_rect.u_start_x,timer_rect.u_start_y+40,timer_rect.u_width,40);
        popup_timer_id = api_start_cycletimer(NULL, time, \
                (OSAL_T_TIMER_FUNC_PTR)win_compopup_pvr_auto_standby_timer_countdown);
    }
#endif
    cnt_poptime=timeout;
}

static void win_compopup_timer_countdown(void)
{
    ap_send_msg(CTRL_MSG_TYPE_POPUP_TIMER,CTRL_MSG_TYPE_POPUP_TIMER,FALSE);
}

void win_compopup_auto_standby_timer_countdown(void)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_AUTO_STANDBY, CTRL_MSG_SUBTYPE_CMD_AUTO_STANDBY_TIMER, FALSE);
}

static void send_key_timer_handler(UINT32 vkey)
{
    UINT32 hkey = 0;

    ap_vk_to_hk(0, vkey, &hkey);
    ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
}

ID win_compopup_start_send_key_timer(UINT32 vkey, UINT32 interval, UINT16 timer_type)
{
    OSAL_T_CTIM  t_dalm;
    ID           timer_id = 0;
    SYSTEM_DATA *p_sys_data = sys_data_get();

    if (0xffffffff == interval)
    {
        interval = p_sys_data->osd_set.time_out * 1000;
    }

    MEMSET(&t_dalm, 0, sizeof(OSAL_T_CTIM));
    t_dalm.callback = send_key_timer_handler;
    t_dalm.type = timer_type;//TIMER_CYCLIC;
    t_dalm.time = interval;
    t_dalm.param = vkey;

    timer_id = osal_timer_create(&t_dalm);
    if (OSAL_INVALID_ID == timer_id)
    {
        return OSAL_INVALID_ID;
    }

    osal_timer_activate(timer_id, 1);
    return timer_id;
}

#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
void win_otacompopup_set_msg(char* str,char* unistr,UINT16 str_id)
{
    UINT8 btn_num,lines;
    UINT8* pstr;
    POBJECT_HEAD txt = (POBJECT_HEAD)&win_popup_msg_mbx;
    UINT16 totalw,mw,mh;
    UINT16 l,t,w,h;
    UINT8   title_flag;
    CONTAINER* win = &g_win_popup;

    title_flag = (win->p_next_in_cntn == txt)? 0 : 1;

    popup_mtxt_content.b_text_type = STRING_UNICODE;
    popup_mtxt_content.text.p_string   = pp_msg_str;

    if(str!=NULL)
    {
        com_asc_str2uni(str, pp_msg_str);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char( (UINT8*)pp_msg_str,unistr);
    }
    else
    {
        popup_mtxt_content.b_text_type        = STRING_ID;
        popup_mtxt_content.text.w_string_id   = str_id;
    }

    btn_num = win_popup_btm_num[win_popup_type];
    if(str != NULL || unistr!= NULL)
    {
        pstr = (UINT8*)pp_msg_str;
    }
    else
    {
        pstr = osd_get_unicode_string(str_id);
    }
    totalw = osd_multi_font_lib_str_max_hw(pstr,txt->b_font,&mw,&mh,0);

    totalw += MSG_LOFFSET*2;

    if(btn_num <= 1)
    {
        w = MIN_WIDHT_1ORNOBTN;
    }
    else
    {
        w = MIN_WIDTH_2BTN;
    }
CHECK_LINES:
    lines = (totalw + w - 1) / w;
    if(lines <= 1)
        lines = 1;
    else
    {
        if(w != MAX_LEN_ONE_LINE)
        {
            w = MAX_LEN_ONE_LINE;
            goto CHECK_LINES;
        }
    }

    if(mh < DEFAULT_FONT_H)
    {
        mh = DEFAULT_FONT_H;
    }
    h =  mh * lines + (lines  - 1)*4 ;
    if(btn_num == 0)
    {
        h += MSG_VOFFSET_NOBTN*2;
    }
    else
    {
        h += (MSG_VOFFSET_WITHBTN + MSG_BUTTON_TOPOFFSET + MSG_BUTTON_BOTOFFSET + BTM_H);
    }

    if(title_flag)
    {
        h += TITLE_H + 10;
    }
#ifdef HDOSD
    l = (OSD_HD_MAX_WIDTH - w)>>1;
    t = ((OSD_HD_MAX_HEIGHT - h)>>1) - 40;
#else
    l = (OSD_MAX_WIDTH - w)>>1;
    t = ((OSD_MAX_HEIGHT - h)>>1) - 20;
#endif
    t = (t + 1) /2 * 2;
    win_compopup_set_frame(l,t,w,h);
}

#endif
