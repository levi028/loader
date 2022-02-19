/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_closecaption.c
*
*    Description: The realize the control of CC
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

#ifdef AUDIO_DESCRIPTION_SUPPORT
#include <hld/dmx/dmx.h>
#include <api/libsi/psi_pmt.h>
#include "win_audio_description.h"
#endif

#include "osdobjs_def.h"
#include "string.id"
//#include "images.h"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_closedcaption.h"
#include "mobile_input.h"
#include "control.h"

#ifdef CC_BY_OSD
#ifdef CC_MONITOR_CS  //vicky20110210
#include <api/libclosecaption/lib_closecaption.h>
#include"copper_common/com_api.h"
#endif
#endif

#if 0
#define AUDIO_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#else
#define AUDIO_DEBUG(...)    do{} while(0)
#endif

#ifdef CC_BY_OSD
/*******************************************************************************
*    Objects definition
*******************************************************************************/
CONTAINER g_win_closedcaption;

static VACTION cc_item_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT cc_item_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);

static VACTION cc_list_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT cc_list_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);

static VACTION cc_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT cc_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_SLIST_02_8BIT
//WSTL_INFO_01_8BIT//WSTL_ZOOM_WIN_01
#define WIN_HL_IDX    WIN_SH_IDX//WSTL_ZOOM_WIN_01
#define WIN_SL_IDX    WIN_SH_IDX//WSTL_ZOOM_WIN_01
#define WIN_GRY_IDX   WIN_SH_IDX//WSTL_ZOOM_WIN_01

#define TITLE_SH_IDX     WSTL_MIXBACK_IDX_01_8BIT////WSTL_MIXBACK_BLACK_IDX
#define MODE_SH_IDX        WSTL_AUDIO_WIN_01 //WSTL_VOLUME_01_BG_8BIT //

#define LST_SH_IDX        WSTL_NOSHOW_IDX
#define LST_HL_IDX        WSTL_NOSHOW_IDX
#define LST_SL_IDX        WSTL_NOSHOW_IDX
#define LST_GRY_IDX    WSTL_NOSHOW_IDX
/*
#define CON_SH_IDX   WSTL_TEXT_17
#define CON_HL_IDX   //WSTL_BUTTON_05
#define CON_SL_IDX   WSTL_TEXT_17//WSTL_BUTTON_01
#define CON_GRY_IDX  WSTL_BUTTON_07
*/
#define CON_SH_IDX   WSTL_TEXT_15_8BIT
#define CON_HL_IDX   WSTL_BUTTON_05_8BIT
#define CON_SL_IDX   WSTL_TEXT_15_8BIT
#define CON_GRY_IDX  WSTL_TEXT_14


/*
#define TXT_SH_IDX   WSTL_TEXT_17
#define TXT_HL_IDX   WSTL_BUTTON_02_FG
#define TXT_SL_IDX   WSTL_TEXT_17//WSTL_BUTTON_01_FG
#define TXT_GRY_IDX  WSTL_BUTTON_07
*/

#define TXT_SH_IDX   WSTL_TEXT_15_8BIT
#define TXT_HL_IDX   WSTL_TEXT_28_8BIT
#define TXT_SL_IDX   WSTL_TEXT_15_8BIT
#define TXT_GRY_IDX  WSTL_TEXT_14

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//WSTL_SCROLLBAR_01
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//WSTL_SCROLLBAR_01

#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT //WSTL_SCROLLBAR_02

#define MARK_SH_IDX        WSTL_MIXBACK_IDX_06_8BIT  //WSTL_MIXBACK_BLACK_IDX
#define MARK_HL_IDX        MARK_SH_IDX             //WSTL_MIXBACK_BLACK_IDX
#define MARK_SL_IDX        MARK_SH_IDX             //WSTL_MIXBACK_BLACK_IDX
#define MARK_GRY_IDX    MARK_SH_IDX             //WSTL_MIXBACK_BLACK_IDX

#ifndef SD_UI
#define    W_L      74//210
#define    W_T      60//110
#define    W_W      250//260// 400//260        //vicky20110210
#define    W_H      310//314//350//420//250    //vicky20110210

#define TITLE_L  (W_L)
#define TITLE_T  (W_T + 6)
#define TITLE_W  (W_W - 20)
#define TITLE_H  48//26
/*
#define MODE_L  (W_L+14)
#define MODE_T  (W_T+54) //(TITLE_T + TITLE_H + 2)
#define MODE_W  (W_W-28)
#define MODE_H  36//40
*/
#define LST_L   (W_L + 20)
#define LST_T   (TITLE_T + TITLE_H + 2) //(MODE_T +  MODE_H)    //vicky20110210
#define LST_W   (W_W - 56)
#define LST_H   (W_H - TITLE_H  - 26)
//(W_H - TITLE_H - MODE_H - 26)
//240//200 //180     //vicky20110210

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H//(LST_H - 36)

#define CON_L    LST_L
#define CON_T    LST_T
#define CON_W    LST_W
#define CON_H    40
#define CON_GAP   0

#define TXT_L_OF    30
#define TXT_W       (CON_W - 34)
#define TXT_H       CON_H //28
#define TXT_T_OF    ((CON_H - TXT_H)/2)
#else
#define    W_L      74//210
#define    W_T      60//110
#define    W_W      180//260// 400//260        //vicky20110210
#define    W_H      280//314//350//420//250    //vicky20110210

#define TITLE_L  (W_L)
#define TITLE_T  (W_T + 6)
#define TITLE_W  (W_W - 20)
#define TITLE_H  48//26
/*
#define MODE_L  (W_L+14)
#define MODE_T  (W_T+54) //(TITLE_T + TITLE_H + 2)
#define MODE_W  (W_W-28)
#define MODE_H  36//40
*/
#define LST_L   (W_L + 20)
#define LST_T   (TITLE_T + TITLE_H + 2) //(MODE_T +  MODE_H)    //vicky20110210
#define LST_W   (W_W - 56)
#define LST_H   (W_H - TITLE_H  - 26)
//(W_H - TITLE_H - MODE_H - 26)
//240//200 //180     //vicky20110210

#define SCB_L (LST_L + LST_W + 4)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H LST_H//(LST_H - 36)

#define CON_L    LST_L
#define CON_T    LST_T
#define CON_W    LST_W
#define CON_H    32
#define CON_GAP    0

#define TXT_L_OF      30
#define TXT_W          (CON_W - 34)
#define TXT_H       CON_H //28
#define TXT_T_OF    ((CON_H - TXT_H)/2)
#endif


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,\
    idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, \
    CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    cc_item_keymap,cc_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_TOP, 6,0,res_id,NULL)

#define LDEF_TXT_MODE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MODE_SH_IDX,MODE_SH_IDX,MODE_SH_IDX,MODE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,str)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, \
    LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE,\
    page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)        \
  DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,\
    style,dep,count,flds,sb,mark,selary)  \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    cc_list_keymap,cc_list_callback,    \
    flds,sb,mark,style,dep,count,selary)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    cc_keymap,cc_callback,  \
    nxt_obj, focus_id,0)



LDEF_TITLE(g_win_closedcaption, cc_title, &cc_ol, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_CC_CAPTION)    //vicky20110210

LDEF_MENU_ITEM(g_win_closedcaption, cc_item1, &cc_item2,cc_txt1,1,6,2,
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H,display_strs[0])
LDEF_MENU_ITEM(g_win_closedcaption, cc_item2, &cc_item3,cc_txt2,2,1,3,
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,display_strs[1])
LDEF_MENU_ITEM(g_win_closedcaption, cc_item3, &cc_item4,cc_txt3,3,2,4,
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H,display_strs[2])
LDEF_MENU_ITEM(g_win_closedcaption, cc_item4, &cc_item5,cc_txt4,4,3,5,
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H,display_strs[3])
LDEF_MENU_ITEM(g_win_closedcaption, cc_item5, &cc_item6,cc_txt5,5,4,6,
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H,display_strs[4])
LDEF_MENU_ITEM(g_win_closedcaption, cc_item6,NULL             ,cc_txt6,6,5,1,
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H,display_strs[5])


static POBJECT_HEAD cc_list_item[] =
{
    (POBJECT_HEAD)&cc_item1,
    (POBJECT_HEAD)&cc_item2,
    (POBJECT_HEAD)&cc_item3,
    (POBJECT_HEAD)&cc_item4,
    (POBJECT_HEAD)&cc_item5,
    (POBJECT_HEAD)&cc_item6,
};

LDEF_LISTBAR(cc_ol,cc_scb,6,SCB_L,SCB_T, SCB_W, SCB_H)

LDEF_MARKBMP(cc_ol,cc_mark, CON_L + 8, CON_T, 20, CON_H, 0);

#define LIST_STYLE (LIST_VER|LIST_SINGLE_SLECT\
        |LIST_ITEMS_NOCOMPLETE| LIST_SCROLL|LIST_GRID\
    |LIST_FOCUS_FIRST|LIST_PAGE_KEEP_CURITEM|LIST_FULL_PAGE)

LDEF_OL(g_win_closedcaption,cc_ol,NULL,\
        LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 6, 0,\
    cc_list_item,&cc_scb,(POBJECT_HEAD)&cc_mark,NULL)

LDEF_WIN(g_win_closedcaption, &cc_title,W_L, W_T,W_W,W_H,1)

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

static TEXT_FIELD *cc_text_items[] =
{
    &cc_txt1,
    &cc_txt2,
    &cc_txt3,
    &cc_txt4,
    &cc_txt5,
    &cc_txt6,
};
static UINT8 bscnt=0;
static UINT32 dwls1=0;
static UINT32 dwls2=0;
static UINT8 bcslist[32]={0};
static ID polling_timer_id = OSAL_INVALID_ID;
static UINT8 win_cc_load(BOOL update);
static void win_cc_set_display(void);
static void win_cc_change(void);

static PRESULT     win_cc_message_proc(UINT32 msg_type, UINT32 msg_code);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION cc_item_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT cc_item_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION cc_list_keymap(POBJECT_HEAD pobj, UINT32 key)
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

static PRESULT cc_list_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT16 oldsel=0;
    UINT16 newsel=0;

    switch(event)
    {
    case EVN_PRE_DRAW:
        win_cc_set_display();
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
        if(oldsel!=newsel)
        {
            win_cc_change();
        }
        //libc_printf("Proc EVN_POST_CHANGE");
        api_dec_wnd_count();
        //libc_printf("End\n");
        ret= PROC_LEAVE;
        break;
    default:
        break;
    }

    return ret;
}


static VACTION cc_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
    case V_KEY_AUDIO:
    case V_KEY_BLUE:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static void get_cs_list(UINT8 *blist)
{
    UINT8 bi=0;
    UINT8 bcnt=0;
    UINT32 dwt1=0;
    UINT32 dwt2=0;

    bscnt=get_dtv_cs_number();
    if(bscnt>=NUM_ONE)
    {
        MEMSET(blist,0,32);
        dwt1=get_dtv_cs_list1();
        dwls1=get_dtv_cs_list1();
        dwt2=get_dtv_cs_list2();
        dwls2=get_dtv_cs_list2();
        //libc_printf("[%x %x]\n",dwls2,dwls1);
        if ((0 == dwls1) || (0 == dwls2))
        {
            return;
        }

        for(bi=0;bi<32;bi++)
        {
            if(dwt1&0x01)
            {
                blist[bcnt]=bi+1;
                bcnt++;
            }
            dwt1>>=1;
        }
        for(bi=0;bi<32;bi++)
        {
            if(dwt2&0x01)
            {
                blist[bcnt]=bi+1+32;
                bcnt++;
            }
            dwt2>>=1;
        }
    }
    /*
      libc_printf("UI(%d)",bscnt);
      for(bi=0;bi<bscnt;bi++)
      {
      libc_printf("[%d]",blist[bi]);
      }
      libc_printf("\n");
    */
}

static void win_polling_timer_countdown(void)
{
    //libc_printf("Send CTRL_MSG_SUBTYPE_CMD_CS_UPDTAED\n");
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_CS_UPDTAED,\
                CTRL_MSG_SUBTYPE_CMD_CS_UPDTAED,FALSE);
}
static PRESULT cc_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;


    switch(event)
    {
    case EVN_PRE_OPEN:
        if( (NUM_ZERO==get_dtv_cs_number()) && (FALSE==get_cc_control_byte()))
        {
        #ifdef SUPPORT_BC
            UINT8 back_saved=0;
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_DISPLAY_NO_DATA);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
        #else
            win_com_popup_open(WIN_POPUP_TYPE_SMSG,\
        NULL, RS_DISPLAY_NO_DATA);
            osal_task_sleep(1000);
            win_compopup_close();
        #endif
                 api_dec_wnd_count();
            return PROC_LEAVE;
        }
        polling_timer_id=api_start_cycletimer(NULL, \
    1000, (OSAL_T_TIMER_FUNC_PTR)win_polling_timer_countdown);
            api_inc_wnd_count();
        if(NUM_ONE==win_cc_load(FALSE))
        {
            ret = PROC_LEAVE;
        }
        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        //libc_printf("Delete polling timer--");
        api_stop_timer(&polling_timer_id);
        //libc_printf("--End\n");
        break;
    case EVN_POST_CLOSE:
        api_dec_wnd_count();
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    case EVN_MSG_GOT:
        ret = win_cc_message_proc(param1,param2);
        break;
    default:
        break;
    }
    return ret;
}

static BOOL cc_checking(void)
{
    BOOL fchange=FALSE;
    UINT8 bcurcnt=0;
    UINT32 dwt1=0;
    UINT32 dwt2=0;
    UINT8  __MAYBE_UNUSED__ bcmd=0;

    bcurcnt=get_dtv_cs_number();
    if(bscnt!=bcurcnt)
    {
        fchange=TRUE;
        //libc_printf("bscnt(%d),bcurcnt(%d)\n",bscnt,bcurcnt);
    }
    else if((bscnt>0)||(bcurcnt>0))
    {
        dwt1=get_dtv_cs_list1();
        dwt2=get_dtv_cs_list2();

        if(dwls1!=dwt1)
        {
            fchange=TRUE;
            //libc_printf("dwls1(%x),dwt1(%x)\n",dwls1,dwt1);
        }
        if(dwls2!=dwt2)
        {
            fchange=TRUE;
            //libc_printf("dwls2(%x),dwt2(%x)\n",dwls2,dwt2);
        }
    }

    //debug vicky20110216
    bcmd=get_cc_control_byte();
    //libc_printf("CC(%d),Totla CS(%d)\n",bcmd,bcurcnt);
    //end
    /*
    if(fchange)
    {
        ;//libc_printf("Send CTRL_MSG_SUBTYPE_CMD_CS_UPDTAED\n");
    }
    */
    return fchange;
}
static PRESULT     win_cc_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT    ret = PROC_LOOP;
    OBJLIST *ol = &cc_ol;

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_CS_UPDTAED:
        if(TRUE==cc_checking())
        {
            win_cc_load(TRUE);
            if((POBJECT_HEAD)&g_win_closedcaption == menu_stack_get_top() )
            {
                osd_track_object((POBJECT_HEAD)ol,    \
                        C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            }
        }
        break;
    default:
        ret = PROC_PASS;
        break;
    }

    return ret;
}

static UINT8 win_cc_load(BOOL update)
{
    UINT8 ret = 0;
    TEXT_FIELD *  __MAYBE_UNUSED__ txt_mode=NULL;
    OBJLIST *ol=NULL;

    UINT16 cnt=0;
    UINT16 sel=0;
    UINT16 top_idx=0;
    UINT16 cur_idx=0;
    UINT16 dep=0;

    txt_mode = &cc_txt_mode;
    ol = &cc_ol;

    UINT8 bcc=0;
    UINT8 bcs=0;
    UINT8 bi=0;

    cnt=9;
    get_cs_list(bcslist);
    cnt+=bscnt;

    bcc=sys_data_get_cc_control();
    bcs=sys_data_get_dtvcc_service()    ;
    sel=0;

    if(bcc>NUM_ZERO)
    {
        sel=bscnt+bcc;
    }
    else if(bcs>0)
    {
        for(bi=0;bi<bscnt;bi++)
        {
            if(bcslist[bi]==bcs)
            {
                sel=bi+1;
                break;
            }
        }
        //error handle
        if(NUM_ZERO==sel)
        {
            set_dtv_cc_service(0);  //i.e. clear bcc
            set_cc(0);
            sys_data_set_dtvcc_service(0);  //i.e. clear bcs
        }
    }

    dep = osd_get_obj_list_page(ol);
    cur_idx = 0;
    top_idx = 0;
    if(sel < cnt)
    {
        cur_idx = sel;
    }
    top_idx = cur_idx / dep * dep;

    osd_set_obj_list_count(ol, cnt);
    osd_set_obj_list_single_select(ol, sel);
    osd_set_obj_list_top(ol, top_idx);
    osd_set_obj_list_cur_point(ol, cur_idx);
    osd_set_obj_list_new_point(ol, cur_idx);
    return ret;
}


static void win_cc_set_display(void)
{
    TEXT_FIELD *txt=NULL;
    OBJLIST *ol=NULL;
    P_NODE p_node;
    UINT16 i=0;
    UINT16 cnt=0;

    UINT16 top_idx=0;
    UINT16 cur_idx=0;
    UINT16 dep=0;
    UINT8 temp=0;

    char str[30]={0};

    MEMSET(str,0,sizeof(str));
    MEMSET(&p_node,0,sizeof(P_NODE));
    ol = &cc_ol;
    cnt = osd_get_obj_list_count(ol);
    dep = osd_get_obj_list_page(ol);
    top_idx = osd_get_obj_list_top(ol);

    for(i=0;i<dep;i++)
    {
        cur_idx = top_idx + i;
        txt = cc_text_items[i];

        if(NUM_ZERO==cur_idx)
        {
            snprintf(str,30," %s","OFF");
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }
        else if(cur_idx < cnt )
        {
            temp=bscnt+4;
            if(cur_idx>temp)
            {
                snprintf(str,30," Text %d",cur_idx-bscnt-4);
            }
            else if(cur_idx>bscnt)
            {
                snprintf(str,30," CC %d",cur_idx-bscnt);
            }
            else
            {
                snprintf(str,30," CS %d",bcslist[cur_idx-1]);
            }
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }
        else
    {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
    }
    }

}

//idx: is the index of the UI CC list.
static void cc_change(UINT8 idx)
{
    UINT8 bcc=0;
    UINT8 bcs=0;
    UINT8 bcsfirst=FALSE;
    SYSTEM_DATA *sys_data=sys_data_get();

    if(NUM_ZERO==idx)
    {
        bcc=0;
        bcs=0;
        //libc_printf("Index(%d):Set OFF",idx);
    }
    else
    {
        get_cs_list(bcslist);
        if(idx>bscnt)
        {
            bcs=0;
            bcsfirst=TRUE;
            bcc=idx-bscnt;
            //libc_printf("Index(%d):Set CC (%d)",idx,bcc);
        }
        else
        {
            bcc=0;
            bcs=bcslist[idx-1];
            set_dtv_cc_service(bcslist[idx-1]);
            //libc_printf("Index(%d):Set CS (%d)",idx,bcs);
        }
    }

    if(bcsfirst)
    {
        set_dtv_cc_service(bcs);
        set_cc(bcc);
    }
    else
    {
        set_cc(bcc);
        set_dtv_cc_service(bcs);
    }

    if((bcc>NUM_ZERO) || (bcs>NUM_ZERO))
    {
        sys_data->osd_set.cc_display = 1;/*cc on*/   //vicky20110216
    }
    else
    {
        sys_data->osd_set.cc_display = 0;/*cc off*/  //vicky20110216
    }
    sys_data_set_cc_control(bcc);
    sys_data_set_dtvcc_service(bcs);
    api_osd_mode_change(OSD_NO_SHOW);
}

static void win_cc_change(void)
{
    OBJLIST *ol = &cc_ol;
    UINT16 sel=0;

    if (NUM_ZERO == osd_get_obj_list_count(ol))
    {
        return;
    }

    sel = osd_get_obj_list_single_select(ol);
    cc_change(sel);
}
#endif
