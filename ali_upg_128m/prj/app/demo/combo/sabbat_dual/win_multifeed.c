/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_multifeed.c
*
*    Description: multi_feed menu.
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

#include "mobile_input.h"
#include <api/libsi/lib_multifeed.h>
#include "win_multifeed.h"
#include "control.h"

#if defined(SUPPORT_CAS9)|| defined(SUPPORT_CAS7)
#include "conax_ap/win_ca_mmi.h"
#endif

#ifdef MULTIFEED_SUPPORT

#define DEB_PRINT   soc_printf

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION multifeed_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT multifeed_item_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

static VACTION multifeed_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT multifeed_list_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

static VACTION multifeed_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT multifeed_callback(POBJECT_HEAD p_obj, VEVENT event,  UINT32 param1, UINT32 param2);

/*#define WIN_SH_IDX      WSTL_ZOOM_WIN_01
#define WIN_HL_IDX    WSTL_ZOOM_WIN_01
#define WIN_SL_IDX    WSTL_ZOOM_WIN_01
#define WIN_GRY_IDX  WSTL_ZOOM_WIN_01

#define TITLE_SH_IDX    WSTL_MIXBACK_BLACK_IDX
#define MODE_SH_IDX WSTL_AUDIO_WIN_01

#define LST_SH_IDX      WSTL_NOSHOW_IDX
#define LST_HL_IDX      WSTL_NOSHOW_IDX
#define LST_SL_IDX      WSTL_NOSHOW_IDX
#define LST_GRY_IDX WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_TEXT_17
#define CON_HL_IDX   WSTL_BUTTON_05
#define CON_SL_IDX   WSTL_TEXT_17//WSTL_BUTTON_01
#define CON_GRY_IDX  WSTL_BUTTON_07

#define TXT_SH_IDX   WSTL_TEXT_17
#define TXT_HL_IDX   WSTL_BUTTON_02_FG
#define TXT_SL_IDX   WSTL_TEXT_17//WSTL_BUTTON_01_FG
#define TXT_GRY_IDX  WSTL_BUTTON_07

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02

#define MARK_SH_IDX WSTL_MIXBACK_BLACK_IDX
#define MARK_HL_IDX WSTL_MIXBACK_BLACK_IDX
#define MARK_SL_IDX WSTL_MIXBACK_BLACK_IDX
#define MARK_GRY_IDX    WSTL_MIXBACK_BLACK_IDX


#define W_L     74//210
#define W_T     (60-40)//110
#define W_W     200//260
#define W_H     (350-36)//420//250

#define TITLE_L  (W_L)
#define TITLE_T  (W_T + 6)
#define TITLE_W ((W_W - 20)/2)
#define TITLE_H  48//26

#define LST_L   (W_L + 20)
#define LST_T   (W_T+54)
#define LST_W   (W_W - 56)
#define LST_H   (W_H - TITLE_H-15)//- 36 - 26)//240//200 //180

#define SCB_L (LST_L + LST_W + 10)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H//(LST_H - 36)

#define CON_L   LST_L
#define CON_T   LST_T
#define CON_W   LST_W
#define CON_H   40
#define CON_GAP 0

#define TXT_L_OF    30
#define TXT_W       ((CON_W - 34)/2)
#define TXT_H       28
#define TXT_T_OF    ((CON_H - TXT_H)/2)
*/
#define WIN_SH_IDX    WSTL_WIN_SLIST_02_8BIT//WSTL_ZOOM_WIN_01
#define WIN_HL_IDX    WIN_SH_IDX//WSTL_ZOOM_WIN_01
#define WIN_SL_IDX    WIN_SH_IDX//WSTL_ZOOM_WIN_01
#define WIN_GRY_IDX   WIN_SH_IDX//WSTL_ZOOM_WIN_01

#define TITLE_SH_IDX    WSTL_MIXBACK_BLACK_IDX
#define MODE_SH_IDX WSTL_AUDIO_WIN_01

#define LST_SH_IDX      WSTL_NOSHOW_IDX
#define LST_HL_IDX      WSTL_NOSHOW_IDX
#define LST_SL_IDX      WSTL_NOSHOW_IDX
#define LST_GRY_IDX WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_TEXT_15_8BIT//WSTL_TEXT_17
#define CON_HL_IDX   WSTL_BUTTON_05_8BIT//WSTL_BUTTON_05
#define CON_SL_IDX   WSTL_TEXT_15_8BIT//WSTL_TEXT_17//WSTL_BUTTON_01
#define CON_GRY_IDX  WSTL_TEXT_14//WSTL_BUTTON_07

#define TXT_SH_IDX   WSTL_TEXT_15_8BIT//WSTL_TEXT_17
#define TXT_HL_IDX   WSTL_TEXT_28_8BIT//WSTL_BUTTON_02_FG
#define TXT_SL_IDX   WSTL_TEXT_15_8BIT//WSTL_TEXT_17//WSTL_BUTTON_01_FG
#define TXT_GRY_IDX  WSTL_TEXT_14//WSTL_BUTTON_07

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//WSTL_SCROLLBAR_01
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//WSTL_SCROLLBAR_01

#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT//WSTL_SCROLLBAR_02

#define MARK_SH_IDX WSTL_MIXBACK_BLACK_IDX
#define MARK_HL_IDX WSTL_MIXBACK_BLACK_IDX
#define MARK_SL_IDX WSTL_MIXBACK_BLACK_IDX
#define MARK_GRY_IDX    WSTL_MIXBACK_BLACK_IDX

#ifndef SD_UI
#define W_L     74//40
#define W_T     60
#define W_W     400//260
#define W_H     380//(250-26)

#define TITLE_L (W_L + 10)
#define TITLE_T  (W_T + 6)
#define TITLE_W (W_W - 20)
#define TITLE_H 48//26

#define LST_L   (W_L + 20)//(W_L + 2)
#define LST_T   (TITLE_T + TITLE_H + 30)
#define LST_W   (W_W - 56)//(W_W - 20)
#define LST_H   (W_H - TITLE_H - 60)//180

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H

#define CON_L   LST_L
#define CON_T   LST_T
#define CON_W   LST_W
#define CON_H   40//28
#define CON_GAP 0

#define TXT_L_OF    30
#define TXT_W       (CON_W - 34)
#define TXT_H       28
#define TXT_T_OF    ((CON_H - TXT_H)/2)
#else
#define W_L     35//40
#define W_T     35
#define W_W    250//260
#define W_H     320//(250-26)

#define TITLE_L (W_L + 10)
#define TITLE_T  (W_T + 6)
#define TITLE_W (W_W - 20)
#define TITLE_H 48//26

#define LST_L   (W_L + 20)//(W_L + 2)
#define LST_T   (TITLE_T + TITLE_H + 30)
#define LST_W   (W_W - 56)//(W_W - 20)
#define LST_H   (W_H - TITLE_H - 60)//180

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H

#define CON_L   LST_L
#define CON_T   LST_T
#define CON_W   LST_W
#define CON_H   32//28
#define CON_GAP 0

#define TXT_L_OF    30
#define TXT_W       (CON_W - 34)
#define TXT_H       28
#define TXT_T_OF    ((CON_H - TXT_H)/2)
#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,\
    multifeed_item_keymap,multifeed_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,  \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_TOP, 6,0,0,str)



#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,str)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)      \
  DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)  \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    multifeed_list_keymap,multifeed_list_callback,    \
    flds,sb,mark,style,dep,count,selary)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    multifeed_keymap,multifeed_callback,  \
    nxt_obj, focus_id,0)



LDEF_TITLE(g_win_multifeed, multifeed_title, &multifeed_ol, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, display_strs[6])

LDEF_MENU_ITEM(g_win_multifeed, multifeed_item1, &multifeed_item2,multifeed_txt1,1,6,2,
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H,display_strs[0])
LDEF_MENU_ITEM(g_win_multifeed, multifeed_item2, &multifeed_item3,multifeed_txt2,2,1,3,
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,display_strs[1])
LDEF_MENU_ITEM(g_win_multifeed, multifeed_item3, &multifeed_item4,multifeed_txt3,3,2,4,
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H,display_strs[2])
LDEF_MENU_ITEM(g_win_multifeed, multifeed_item4, &multifeed_item5,multifeed_txt4,4,3,5,
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H,display_strs[3])
LDEF_MENU_ITEM(g_win_multifeed, multifeed_item5, &multifeed_item6,multifeed_txt5,5,4,6,
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H,display_strs[4])
LDEF_MENU_ITEM(g_win_multifeed, multifeed_item6,NULL             ,multifeed_txt6,6,5,1,
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H,display_strs[5])


POBJECT_HEAD multifeed_list_item[] =
{
    (POBJECT_HEAD)&multifeed_item1,
    (POBJECT_HEAD)&multifeed_item2,
    (POBJECT_HEAD)&multifeed_item3,
    (POBJECT_HEAD)&multifeed_item4,
    (POBJECT_HEAD)&multifeed_item5,
    (POBJECT_HEAD)&multifeed_item6,
};

LDEF_LISTBAR(multifeed_ol,multifeed_scb,6,SCB_L,SCB_T, SCB_W, SCB_H)

LDEF_MARKBMP(multifeed_ol,multifeed_mark, CON_L + 8, CON_T, 20, CON_H, IM_MSELECT/*IM_ICON_SELECT*/);

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | \
                    LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_multifeed,multifeed_ol,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 6, 0,multifeed_list_item,&multifeed_scb,\
        (POBJECT_HEAD)&multifeed_mark,NULL)

LDEF_WIN(g_win_multifeed, &multifeed_title,W_L, W_T,W_W,W_H,1)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

#define MULTIFEED_TIMER_NAME "mft"
#define MULTIFEED_TIMER_DURATION 4000
static ID multifeed_timer = OSAL_INVALID_ID;
static UINT8 gs_is_multifeed_show = 0;
static TEXT_FIELD* text_items[] =
{
    &multifeed_txt1,
    &multifeed_txt2,
    &multifeed_txt3,
    &multifeed_txt4,
    &multifeed_txt5,
    &multifeed_txt6,
};

void win_multifeed_load(void);
void win_multifeed_set_display(void);
static PRESULT  win_multifeed_message_proc(UINT32 msg_type, UINT32 msg_code);
void multifeed_timer_func(UINT unused);
void win_multifeed_change_feed();

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
void multifeed_timer_func(UINT unused)
{
    api_stop_timer(&multifeed_timer);
    ap_send_msg(CTRL_MSG_SUBTYPE_EXIT, multifeed_timer, TRUE);
}

static VACTION multifeed_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT multifeed_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION multifeed_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    case V_KEY_ENTER:
        act = VACT_SELECT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT multifeed_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT16 oldsel = 0;
    UINT16 newsel = 0;
    UINT16 cur_channel = sys_data_get_cur_group_cur_mode_channel();
    P_NODE p_node;
    UINT16 i = 0;
    struct MULTIFEED_INFO multifeed_info;
    struct FEED_INFO *p = NULL;

    MEMSET(&p_node,0, sizeof(P_NODE));
    MEMSET(&multifeed_info,0,sizeof(struct MULTIFEED_INFO));
    switch(event)
    {
    case EVN_PRE_DRAW:
        win_multifeed_set_display();
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        oldsel = (UINT16)param1;
        newsel = (UINT16)param2;
        /*! Check feed selected changed or not */
        if(oldsel!=newsel)
        {
            if (api_pvr_is_record_active())
            {
                /*!
                 * if Current Service is recording, and subfeed is not
                 * the same TP with current Service, then can't switch.
                 */
                get_prog_at(cur_channel,&p_node);
                multifeed_get_info(p_node.prog_id, &multifeed_info);
                p = multifeed_info.feed_head;
                for (i=0; i<multifeed_info.num; i++)
                {
                    if (i == newsel)
                    {
                        break;
                    }
                    p = p->next;
                }
                if (p->p_node.tp_id != p_node.tp_id )
                {
                    /*!
                     * Recording and Not the same TP
                     * Popup Message to inform User,
                     * and keep List status to previous.
                     */
                    UINT8 back_saved = 0; 
                    osd_set_obj_list_single_select(&multifeed_ol, oldsel);
                    osd_track_object((POBJECT_HEAD)&multifeed_ol, C_UPDATE_ALL);
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_frame(300, 200, 500, 200);
                    win_compopup_set_msg_ext("Can not switch to The Feed locate in other TP because Recording!",NULL,0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(2000);
                    win_compopup_smsg_restoreback();
                    break;
                }
            }
            /*! can switch */
            win_multifeed_change_feed();
        }
        break;
    case EVN_KEY_GOT:
//        api_stop_timer(&multifeed_timer);
//        multifeed_timer = api_start_timer(MULTIFEED_TIMER_NAME, MULTIFEED_TIMER_DURATION,multifeed_timer_func);
        break;
    default:
        break;
    }

    return ret;
}


static VACTION multifeed_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_PIP_LIST:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;

}

static PRESULT  win_multifeed_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    OBJLIST * ol = NULL;

    switch( msg_type )
    {
    case CTRL_MSG_SUBTYPE_CMD_MULTIFEED:
        if (gs_is_multifeed_show)
        {
            win_multifeed_load();
            win_multifeed_set_display();
            ol = &multifeed_ol;
            osd_track_object( (POBJECT_HEAD)ol, C_UPDATE_ALL );
        }
        break;
    default:
        ret = PROC_PASS;
        break;
    }
    return ret;
}

static PRESULT multifeed_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
   // VACTION unact = VACT_PASS;
    TEXT_FIELD *title = NULL;
    char t[20] = {0};

    switch(event)
    {
    case EVN_PRE_OPEN:
//api_inc_wnd_count();
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
        if((CA_MMI_PRI_01_SMC == get_mmi_showed())||(CA_MMI_PRI_06_BASIC == get_mmi_showed()))
        {
            //win_mmipopup_close(1);
                #if defined(SUPPORT_CAS7)//SUPPORT_CAS7 alone
                    win_mmipopup_close(CA_MMI_PRI_01_SMC);
                #else
                    win_pop_msg_close(CA_MMI_PRI_01_SMC);
                #endif
            //win_fingerpop_close();
        }
        if((CA_MMI_PRI_00_NO==get_mmi_showed()) && (get_signal_stataus_show()))
        {
            show_signal_status_osdon_off(0);
        }
        #endif
        title=&multifeed_title;
        strncpy(t,"Multifeed", (20-1));
        osd_set_text_field_content(title, STRING_ANSI, (UINT32)t);
        win_multifeed_load();
        break;
    case EVN_POST_OPEN:
        //multifeed_timer = api_start_timer(MULTIFEED_TIMER_NAME,MULTIFEED_TIMER_DURATION,multifeed_timer_func);
        gs_is_multifeed_show = TRUE;
        break;
    case EVN_PRE_CLOSE:
        //api_stop_timer(&multifeed_timer);
        gs_is_multifeed_show = FALSE;
        break;
    case EVN_POST_CLOSE:
        #if defined(SUPPORT_CAS9) || defined(SUPPORT_CAS7)
            restore_ca_msg_when_exit_win();
        #endif
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_EXIT == param1)
        {
            ret = PROC_LEAVE;
        }
        else
        {
            ret = win_multifeed_message_proc( param1, param2 );
        }
        break;
    default:
        break;
    }

    return ret;
}

void win_multifeed_load(void)
{
    OBJLIST* ol = NULL;
    P_NODE p_node;
    UINT16 channel = 0;
    UINT16 cnt = 0,sel = 0;
    UINT16 top_idx = 0;
    UINT16 cur_idx = 0;
    UINT16 dep = 0;
    struct MULTIFEED_INFO multifeed_info;
    INT32  __MAYBE_UNUSED__ ret = 0;

    MEMSET(&p_node,0, sizeof(P_NODE));
    MEMSET(&multifeed_info,0,sizeof(struct MULTIFEED_INFO));
    ol = &multifeed_ol;

    channel = sys_data_get_cur_group_cur_mode_channel();

    get_prog_at(channel,&p_node);
    MEMSET(&multifeed_info, 0, sizeof(struct MULTIFEED_INFO));
    ret = multifeed_get_info(p_node.prog_id, &multifeed_info);

    sel = 0xFFFF;
    cnt = multifeed_info.num;

    if(multifeed_info.idx < cnt)
    {
        sel= multifeed_info.idx;
    }

    dep = osd_get_obj_list_page(ol);
    cur_idx = 0;
    top_idx = 0;
    if(sel<multifeed_info.num)
    {
        cur_idx = sel;
    }
    top_idx = cur_idx / dep * dep;

    osd_set_obj_list_count(ol, cnt);
    osd_set_obj_list_single_select(ol, sel);
    osd_set_obj_list_top(ol, top_idx);
    osd_set_obj_list_cur_point(ol, cur_idx);
    osd_set_obj_list_new_point(ol, cur_idx);
}


void win_multifeed_set_display(void)
{
    TEXT_FIELD *txt = NULL;
    OBJLIST* ol = NULL;
    UINT16 i = 0;
    UINT16 cnt = 0;
    UINT16 top_idx = 0;
    UINT16 cur_idx = 0;
    UINT16 dep = 0;
    P_NODE p_node;
    UINT16 channel = 0;
    UINT8 name[32] = {0};

    MEMSET(&p_node,0,sizeof(P_NODE));
    channel = sys_data_get_cur_group_cur_mode_channel();

    get_prog_at(channel,&p_node);

    ol = &multifeed_ol;
    cnt = osd_get_obj_list_count(ol);
    dep = osd_get_obj_list_page(ol);
    top_idx = osd_get_obj_list_top(ol);

    for(i=0;i<dep;i++)
    {
        cur_idx = top_idx + i;
        txt = text_items[i];

        if(cur_idx < cnt )
        {
            multifeed_get_feed_name(p_node.prog_id, cur_idx, name);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)name);
        }
        else
        {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
        }
    }

}


void win_multifeed_change_feed()
{
    OBJLIST* ol = NULL;
    UINT16 sel = 0;
    UINT16 cur_channel = sys_data_get_cur_group_cur_mode_channel();

    ol = &multifeed_ol;
    if(0 == osd_get_obj_list_count(ol))
    {
        return;
    }
    sel = osd_get_obj_list_single_select(ol);

    if(SUCCESS == multifeed_change_feed(cur_channel, sel))
    {
        api_play_channel(cur_channel, TRUE, TRUE,FALSE);
    }
}
#endif

