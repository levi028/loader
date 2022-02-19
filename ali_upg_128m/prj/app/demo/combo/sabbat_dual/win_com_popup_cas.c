/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_popup_cas.c
*
*    Description: The common CAS function of popup
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include <basic_types.h>
#include <api/libosd/osd_lib.h>

#include "menus_root.h"
#include "osd_config.h"
#include "control.h"

#if defined( SUPPORT_CAS9)
#include "conax_ap/win_ca_mmi.h"
#endif

#if defined(SUPPORT_CAS7)
#include "conax_ap7/win_ca_mmi.h"
#endif

#if defined(SUPPORT_C0200A)
#include "c0200a_ap/win_c0200a_mmi.h"
#endif

#include "win_signalstatus.h"
#include "win_com_popup_inner.h"
#include "win_prog_input.h"

#ifdef SUPPORT_CAS7
#define BARGING_OFFSET 15
#endif

BOOL mmipopup_is_closed=TRUE;
static struct rect mmi_pos __MAYBE_UNUSED__;
static struct rect mmi_pos_msg __MAYBE_UNUSED__;

#if defined(SUPPORT_CAS7) ||defined(SUPPORT_CAS9)
void win_mmicompopup_set_msg(char *str,char *unistr,UINT16 str_id)
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
    CONTAINER  *win = &g_win_popup;

    title_flag = (win->p_next_in_cntn == txt)? 0 : 1;

    popup_mtxt_content.b_text_type = STRING_UNICODE;
    popup_mtxt_content.text.p_string   = pp_msg_str;

    if(str!=NULL)
    {
        com_asc_str2uni((UINT8 *)str, pp_msg_str);
    }
    else if(unistr!=NULL)
    {
        com_uni_str_copy_char( (UINT8*)pp_msg_str,(UINT8 *)unistr);
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
        lines = (totalw + w - 1) / w;
        if (lines <= 1)
        {
            lines = 1;
            break;
        }
        else
        {
            if (MAX_LEN_CONAX_ONE_LINE != w)
            {
                w = MAX_LEN_CONAX_ONE_LINE;
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

    //for jade hd osd
    UINT16 max_h = 144;

    if(h<max_h)
    {
        h = max_h;
    }

    if(title_flag)
    {
        h += TITLE_H + 10;
    }
    l = (OSD_MAX_WIDTH - w)/2;
    t = (OSD_MAX_HEIGHT - h)/2;
    t = (t + 1) /2 * 2 - MSG_VOFFSET_NOBTN;
    win_compopup_set_frame(l,t,w,h);
}

BOOL win_mmipopup_is_closed(void)
{
    return mmipopup_is_closed;
}

void win_mmipopup_open(void)
{
    if((CA_MMI_PRI_01_SMC == get_mmi_showed())&& (TRUE ==get_channel_scramble()))
    {
        api_disp_blackscreen(TRUE, TRUE);
    }
    if(CA_MMI_PRI_05_MSG == get_mmi_showed())
    {
        osd_set_color((POBJECT_HEAD)&g_win_popup,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX);
        osd_set_color((POBJECT_HEAD)&win_popup_msg_mbx,WSTL_TEXT_07_HD,WSTL_TEXT_07_HD,WSTL_TEXT_07_HD,WSTL_TEXT_07_HD);
        mmi_pos_msg.u_start_x = g_win_popup.head.frame.u_left;
        mmi_pos_msg.u_start_y = g_win_popup.head.frame.u_top;
        mmi_pos_msg.u_width = g_win_popup.head.frame.u_width;
        mmi_pos_msg.u_height = g_win_popup.head.frame.u_height;
    }

    if(CA_MMI_PRI_05_MSG != get_mmi_showed())
    {
        osd_set_color((POBJECT_HEAD)&win_popup_msg_mbx,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD);
    }
    osd_draw_object((POBJECT_HEAD)&g_win_popup, C_UPDATE_ALL);
    mmipopup_is_closed=FALSE;
    mmi_pos.u_start_x = g_win_popup.head.frame.u_left;
  #ifdef CAS9_V6 //large_char
    mmi_pos.u_start_y = g_win_popup.head.frame.u_top;
  #elif defined(SUPPORT_CAS7)
    mmi_pos.u_start_y = g_win_popup.head.frame.u_top;
  #else
    mmi_pos.u_start_x = g_win_popup.head.frame.u_top;
  #endif
    mmi_pos.u_width = g_win_popup.head.frame.u_width;
    mmi_pos.u_height = g_win_popup.head.frame.u_height;

    if(CA_MMI_PRI_05_MSG == get_mmi_showed())
    {
        //win_popup_msg_mbx.head.bType=OT_MULTITEXT;
        osd_set_color((POBJECT_HEAD)&g_win_popup,POPUPWIN_IDX_SD, POPUPWIN_IDX_SD,0,0);
        osd_set_color((POBJECT_HEAD)&win_popup_msg_mbx,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD,POPUPSTR_IDX_SD);
    }
}

#ifdef SUPPORT_CAS7
void win_mmipopup_close_ex(UINT8 mmi_flag)
{
    if(get_mmi_showed()!=CA_MMI_PRI_03_MAT&&get_mmi_showed()!=CA_MMI_PRI_04_PPV)
    {
        if((mmipopup_is_closed==FALSE))
        {
            osd_clear_object((POBJECT_HEAD)&g_win_popup, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
        }
    }
}
#endif
void win_mmipopup_close(UINT8 mmi_flag)
{
    if((CA_MMI_PRI_05_MSG == get_mmi_showed())&&(CA_MMI_PRI_05_MSG != mmi_flag))
    {
        return;
    }
    if((CA_MMI_PRI_03_MAT != get_mmi_showed())&&(CA_MMI_PRI_04_PPV != get_mmi_showed()))
    {
        #ifdef CAS9_V6
        /* avoid mail icon overlap with host msg, in v6 we change the msg's LTWH for large character */
        show_bmail_osdon_off(0);
        #endif

        if(menu_is_mmi10()&&(FALSE == mmipopup_is_closed))
        {
            g_win_popup.head.frame.u_left = mmi_pos.u_start_x;
            #ifdef CAS9_V6 //large_char
            g_win_popup.head.frame.u_top = mmi_pos.u_start_y;
            #elif defined(SUPPORT_CAS7)
            g_win_popup.head.frame.u_top = mmi_pos.u_start_y;
            #else
            g_win_popup.head.frame.u_top = mmi_pos.u_start_x;
            #endif
            g_win_popup.head.frame.u_width = mmi_pos.u_width;
            g_win_popup.head.frame.u_height = mmi_pos.u_height;
            osd_clear_object((POBJECT_HEAD)&g_win_popup, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
        }
    }
    if(CA_MMI_PRI_01_SMC == get_mmi_showed())
    {
        api_disp_blackscreen(FALSE, FALSE);
    }
}

#ifdef SUPPORT_CAS7
void win_mmipopup_msg_close(UINT8 mmi_flag,UINT8 b_force)
{
    //struct vpo_device *vpo_dev = NULL;
    //date_time int_utc_time;
    if((get_mmi_showed()==CA_MMI_PRI_05_MSG)&&(mmi_flag!=CA_MMI_PRI_05_MSG))
    {
        return;
    }
    if((get_mmi_showed()!=CA_MMI_PRI_03_MAT)&&(get_mmi_showed()!=CA_MMI_PRI_04_PPV))
    {
        if(menu_is_mmi10()&&(mmipopup_is_closed==FALSE))
        {
            g_win_popup.head.frame.u_left = mmi_pos_msg.u_start_x - BARGING_OFFSET;
            g_win_popup.head.frame.u_top = mmi_pos_msg.u_start_y- BARGING_OFFSET;
            g_win_popup.head.frame.u_width = mmi_pos_msg.u_width + BARGING_OFFSET;
            g_win_popup.head.frame.u_height = mmi_pos_msg.u_height + BARGING_OFFSET;
            osd_clear_object((POBJECT_HEAD)&g_win_popup, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
        }
    }
    if(CA_MMI_PRI_01_SMC == get_mmi_showed())
    {
        api_disp_blackscreen(FALSE, FALSE);
    }
}
#endif

#ifdef SUPPORT_CAS9
void win_mmipopup_close_ex(UINT8 mmi_flag,UINT8 b_force)
{
    POBJECT_HEAD    topmenu = NULL;

    if((CA_MMI_PRI_05_MSG == get_mmi_showed())&&(CA_MMI_PRI_05_MSG != mmi_flag))
    {
        return;
    }
    if((CA_MMI_PRI_03_MAT != get_mmi_showed())&&(CA_MMI_PRI_04_PPV != get_mmi_showed()))
    {
        #ifdef CAS9_V6
        /* avoid mail icon overlap with host msg, in v6 we change the msg's LTWH for large character */
        show_bmail_osdon_off(0);
        #endif

        if(menu_is_mmi10()&&(FALSE == mmipopup_is_closed))
        {
            //use saved popup frame to fix bug: when new popup after message,
            //message frame changed to new value,
            //can't clear after new popup exit
            g_win_popup.head.frame.u_left = mmi_pos.u_start_x;
            #ifdef CAS9_V6 //large_char
            g_win_popup.head.frame.u_top = mmi_pos.u_start_y;
            #else
            g_win_popup.head.frame.u_top = mmi_pos.u_start_x;
            #endif
            g_win_popup.head.frame.u_width = mmi_pos.u_width;
            g_win_popup.head.frame.u_height = mmi_pos.u_height;

            osd_clear_object((POBJECT_HEAD)&g_win_popup, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
        }
        else if((FALSE == mmipopup_is_closed) && (FALSE == menu_is_mmi10()) && (TRUE == b_force))
        {
            topmenu = menu_stack_get_top();
            while(NULL != topmenu)
            {
                osd_obj_close(topmenu, C_CLOSE_CLRBACK_FLG);
                menu_stack_pop();
                topmenu = menu_stack_get_top();
            }

            osd_clear_object((POBJECT_HEAD)&g_win_popup, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
        }  
    }
    if(CA_MMI_PRI_01_SMC == get_mmi_showed())//&&menu_is_mmi10())
    {
        api_disp_blackscreen(FALSE, FALSE);
    }
}
#endif

void win_pop_msg_open(UINT16 msg_id)
{
    if((CA_MMI_PRI_01_SMC == get_mmi_showed()) && (TRUE ==get_channel_scramble()))
    {
        api_disp_blackscreen(TRUE, TRUE);
    }

    win_pop_msg_set_display(msg_id);
    osd_draw_object((POBJECT_HEAD)&win_pop_msg_con, C_UPDATE_ALL);

    mmipopup_is_closed=FALSE;
    mmi_pos.u_start_x = g_win_popup.head.frame.u_left;
    #ifdef CAS9_V6 //large_char
    mmi_pos.u_start_y = g_win_popup.head.frame.u_top;
    #else
    mmi_pos.u_start_x = g_win_popup.head.frame.u_top;
    #endif
    mmi_pos.u_width = g_win_popup.head.frame.u_width;
    mmi_pos.u_height = g_win_popup.head.frame.u_height;
}

void win_pop_msg_close(UINT8 mmi_flag)
{
    if((CA_MMI_PRI_05_MSG == get_mmi_showed())&&(CA_MMI_PRI_05_MSG != mmi_flag))
    {
        return;
    }
    if((CA_MMI_PRI_03_MAT != get_mmi_showed())&&(CA_MMI_PRI_04_PPV != get_mmi_showed()))
    {
        if(menu_is_mmi10()&&(FALSE == mmipopup_is_closed))
        {
            osd_clear_object((POBJECT_HEAD)&win_pop_msg_con, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
        }
    }
    if(CA_MMI_PRI_01_SMC == get_mmi_showed())//&&menu_is_mmi10())
    {
        api_disp_blackscreen(FALSE, FALSE);
    }
}

static win_popup_choice_t win_compopup_open_cas(void)
{
    CONTAINER *con =&g_win_popup;
    PRESULT b_result = PROC_LOOP;
    UINT32 hkey = 0;

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

    win_popup_init();
    BOOL old_value = FALSE;

    old_value = ap_enable_key_task_get_key(TRUE);
    while(b_result != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();
        if((INVALID_HK == hkey) || (INVALID_MSG == hkey))
        {
            continue;
        }
        b_result = osd_obj_proc((POBJECT_HEAD)con, (MSG_TYPE_KEY<<16),hkey,0);
    }
    ap_enable_key_task_get_key(old_value);
    return win_pp_choice;
}

static UINT8 cas_pop_open = FALSE;

UINT8 get_cas_pop_status(void)
{
    return cas_pop_open;
}

win_popup_choice_t win_compopup_open_ext_cas(UINT8 *back_saved)
{
    win_popup_choice_t ret = WIN_POP_CHOICE_NO;

    wincom_backup_region(&g_win_popup.head.frame);
    cas_pop_open = TRUE;
    ret = win_compopup_open_cas();

    *back_saved = 1;
    if(win_pp_opened)
    {
        win_msg_popup_close();
    }
    wincom_restore_region();

    cas_pop_open = FALSE;
    return ret;
}
#endif

#ifdef SUPPORT_C0200A
void win_pop_msg_open_nvcak(UINT16 msg_id)
{   
    	POBJECT_HEAD    topmenu=menu_stack_get_top();
	POBJECT_HEAD    firstmenu = menu_stack_get(0);
    if((CA_MMI_PRI_01_SMC == get_mmi_showed()) && (TRUE ==get_channel_scramble()))  
    {
	#ifdef _C0200A_CA_ENABLE_
	if(topmenu!= NULL && firstmenu == &g_win_mainmenu)
	{
		;
	}
	else
	#endif
	{
        	api_disp_blackscreen(TRUE, TRUE);
	}
    }
    if(firstmenu == &g_win_mainmenu)
    {
    	wincom_backup_region(&win_pop_msg_con.head.frame);
    }
    win_pop_msg_set_display(msg_id);
    osd_draw_object((POBJECT_HEAD)&win_pop_msg_con, C_UPDATE_ALL);

    mmipopup_is_closed=FALSE;
    mmi_pos.u_start_x = g_win_popup.head.frame.u_left;
	mmi_pos.u_start_x = g_win_popup.head.frame.u_top;
    mmi_pos.u_width = g_win_popup.head.frame.u_width;
    mmi_pos.u_height = g_win_popup.head.frame.u_height;
}
    
void win_pop_msg_close_nvcak(UINT8 mmi_flag)
{
	POBJECT_HEAD    topmenu=menu_stack_get_top();
	POBJECT_HEAD    firstmenu = menu_stack_get(0);
	if((CA_MMI_PRI_05_MSG == get_mmi_showed())&&(CA_MMI_PRI_05_MSG != mmi_flag))
	{
        return;
	}
	if((CA_MMI_PRI_03_MAT != get_mmi_showed())&&(CA_MMI_PRI_04_PPV != get_mmi_showed()))
    {
        if(menu_is_mmi10()&&(FALSE == mmipopup_is_closed) || (FALSE == mmipopup_is_closed)&&(firstmenu == &g_win_mainmenu)&&(1 == get_mmi_showed()))
        {
            osd_clear_object((POBJECT_HEAD)&win_pop_msg_con, C_CLOSE_CLRBACK_FLG);
            mmipopup_is_closed=TRUE;
	    if(firstmenu == &g_win_mainmenu)
	    {
	    	wincom_restore_region();
	    }
        }        
    }
    if(CA_MMI_PRI_01_SMC == get_mmi_showed())//&&menu_is_mmi10())
    {
    	#ifdef _C0200A_CA_ENABLE_
		if(topmenu!= NULL && firstmenu == &g_win_mainmenu)
		{
			;
		}
		else
	#endif
		{
        		api_disp_blackscreen(FALSE, FALSE);    
		}
    }
}
#endif

#if(defined(MULTI_CAS)&&defined(SUPPORT_CAS_A))
void win_compopup_set_enter_wrong_pin_mmi_status(BOOL value)
{
    g_enter_wrong_pin_mmi = value;
}

BOOL  win_compopup_get_enter_wrong_pin_mmi_status()
{
    return g_enter_wrong_pin_mmi;
}

BOOL win_compopup_set_target_parental_control_level(UINT8 target_parental_control_level_code)
{
    g_target_parental_control_level_for_confirm_ui = target_parental_control_level_code;
}

BOOL win_compopup_set_target_tvpin_value(UINT8* target_tvpin)
{
    MEMCPY(g_target_tvpin_for_confirm_ui, target_tvpin, 8);
}

BOOL win_compopup_reset_target_tvpin_value(void)
{
    MEMSET(g_target_tvpin_for_confirm_ui, 0xFF, 8);
}

BOOL win_compopup_get_confirm_proc_exec_result(void)
{
    return g_tvpin_confirm_proc_exec_result;
}

BOOL win_compopup_reset_confirm_proc_exec_result(void)
{
    g_tvpin_confirm_proc_exec_result=FALSE;
}

void win_compopup_set_tvpin_input_confirm_mode(BOOL b_mode)
{
    g_tvpin_confirm_mode = b_mode;
}

void win_compopup_set_item_name(char *str, char *unistr, UINT16 str_id)
{
    TEXT_FIELD *txt;
    OBJECT_HEAD *obj= &win_popup_msg_mbx;
    OBJECT_HEAD *pobj_next;

    //libc_printf("win_compopup_set_item_name()\n");
    txt = &win_popup_item_name;
    osd_set_text_field_str_point(txt, msg_popup_item_name);
    if (str != NULL)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else if (unistr != NULL)
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
    }
    else
    {
        if (str_id != 0)
        {
            osd_set_text_field_str_point(txt, NULL);
            osd_set_text_field_content(txt, STRING_ID, (UINT32)str_id);
        }
        else
        {
            return ;
        }
    }
    osd_set_objp_next(obj, txt);

    pobj_next = (win_popup_btm_num[win_popup_type] <= 0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
    //libc_printf("-msg_popup_item_name->pObjNext=%d\n", pObjNext);
    osd_set_objp_next(txt, pobj_next);
}

void win_compopup_set_item_val(char *str, char *unistr, UINT16 str_id, UINT16 val_offset)
{
    TEXT_FIELD *txt;
    OBJECT_HEAD *obj= &win_popup_msg_mbx;
    OBJECT_HEAD *pobj_next=&win_popup_item_name;

    //libc_printf("win_compopup_set_item_val()\n");
    txt = &win_popup_item_val;
    osd_set_text_field_str_point(txt, msg_popup_item_val);
    if (str != NULL)
    {
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    }
    else if (unistr != NULL)
    {
        osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
    }
    else
    {
        if (str_id != 0)
        {
            osd_set_text_field_str_point(txt, NULL);
            osd_set_text_field_content(txt, STRING_ID, (UINT32)str_id);
        }
        else
        {
            return ;
        }
    }

    obj=&win_popup_msg_mbx;
    if(osd_get_objp_next(obj)==&win_popup_item_name)
    {
        obj=&win_popup_item_name;
        osd_set_objp_next(obj, txt);
    }
    else
    {
        osd_set_objp_next(obj, txt);
    }

    pobj_next = (win_popup_btm_num[win_popup_type] <= 0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
    osd_set_objp_next(txt, pobj_next);
    val_xoffset = val_offset;
}
#endif

