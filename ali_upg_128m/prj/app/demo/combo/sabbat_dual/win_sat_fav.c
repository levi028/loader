/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_sat_fav.c
*
*    Description: favorite program nemu(press botton "FAV").
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
#include <api/libtsi/sec_pmt.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_hde.h>
#else
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#endif
#include <hld/dis/vpo.h>

//#include <api/libdb/db_config.h>
#include <api/libdb/db_return_value.h>
#ifndef COMBOUI
//#include <api/libdb/db_node_s.h>
#else
#include <api/libdb/db_node_combo.h>
#endif
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>

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
#include "control.h"
#include "win_sat_fav.h"

#ifndef MAX_GROUP_NUM
#define MAX_GROUP_NUM    (1+MAX_SAT_NUM + MAX_FAVGROUP_NUM+MAX_LOCAL_GROUP_NUM)
#endif
/*******************************************************************************
*   Objects definition
*******************************************************************************/

#define LIST_CNT    10

//extern CONTAINER    g_win_satfav_list;


#define WIN_SH_IDX    WSTL_WIN_SLIST_02_8BIT
#define WIN_HL_IDX    WIN_SH_IDX
#define WIN_SL_IDX    WIN_SH_IDX
#define WIN_GRY_IDX   WIN_SH_IDX

#define TITLE_SH_IDX    WSTL_MIXBACK_IDX_01_8BIT

#define LIST_SH_IDX     WSTL_NOSHOW_IDX
#define LIST_HL_IDX     WSTL_NOSHOW_IDX
#define LIST_SL_IDX     WSTL_NOSHOW_IDX
#define LIST_GRY_IDX    WSTL_NOSHOW_IDX

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT

#define LIST_BAR_MID_RECT_IDX       WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT

#define CON_SH_IDX   WSTL_TEXT_15_8BIT
#define CON_HL_IDX   WSTL_BUTTON_05_8BIT
#define CON_SL_IDX   WSTL_TEXT_15_8BIT
#define CON_GRY_IDX  WSTL_TEXT_14

#define TXTI_SH_IDX   WSTL_TEXT_15_8BIT
#define TXTI_HL_IDX   WSTL_TEXT_28_8BIT
#define TXTI_SL_IDX   WSTL_TEXT_15_8BIT
#define TXTI_GRY_IDX  WSTL_TEXT_14

#define TXTN_SH_IDX   WSTL_TEXT_15_8BIT
#define TXTN_HL_IDX   WSTL_TEXT_28_8BIT
#define TXTN_SL_IDX   WSTL_TEXT_15_8BIT
#define TXTN_GRY_IDX  WSTL_TEXT_14

#define BMP_SH_IDX   WSTL_LINE_MENU_HD//sharon WSTL_MAINMENU_LINE_8BIT
#define BMP_HL_IDX   BMP_SH_IDX
#define BMP_SL_IDX   BMP_SH_IDX
#define BMP_GRY_IDX  BMP_SH_IDX

#ifndef SD_UI
#define W_L     50//sharon 100
#define W_T     60
#define W_W     409//sharon 372
#define W_H     562//sharon 536

#define TITLE_L     (W_L + 100)
#define TITLE_T     (W_T + 6)
#define TITLE_W     (W_W - 200)
#define TITLE_H     40

#define LST_L   (W_L + 2)
#define LST_T   (W_T + 68)
#define LST_W   260//sharon 230
#define LST_H   320//sharon 298

#define CON_L   (W_L + 20)
#define CON_T   (W_T + 60)
#define CON_W   (W_W - 60)//sharon 300
#define CON_H   40
#define CON_GAP 4

#define SCB_L (CON_L + CON_W + 10)
#define SCB_T CON_T
#define SCB_W 12//18
#define SCB_H 462//sharon 436

#define SCB_GAP 4

#define BMP_L_OF        8
#define BMP_T_OF        ((CON_H - BMP_H)/2)
#define BMP_W           30
#define BMP_H           28

#define TXTI_L_OF   0
#define TXTI_T_OF   0
#define TXTI_W      50
#define TXTI_H      CON_H

#define TXTN_L_OF   (TXTI_L_OF + TXTI_W + 10)
#define TXTN_T_OF  0
#define TXTN_W      (CON_W - TXTN_L_OF-10)
#define TXTN_H     CON_H
#else
#define W_L     40//sharon 100
#define W_T     40
#define W_W     300//sharon 372
#define W_H     410//sharon 536

#define TITLE_L     (W_L + 45)
#define TITLE_T     (W_T + 2)
#define TITLE_W     (W_W - 100)
#define TITLE_H     40

#define LST_L   (W_L + 2)
#define LST_T   (W_T + 68)
#define LST_W   260//sharon 230
#define LST_H   320//sharon 298

#define CON_L   (W_L + 15)
#define CON_T   (W_T + 50)
#define CON_W   (W_W - 50)//sharon 300
#define CON_H   32
#define CON_GAP 2

#define SCB_L (CON_L + CON_W + 10)
#define SCB_T CON_T
#define SCB_W 12//18
#define SCB_H 350//sharon 436

#define SCB_GAP 4

#define BMP_L_OF        8
#define BMP_T_OF        ((CON_H - BMP_H)/2)
#define BMP_W           30
#define BMP_H           28

#define TXTI_L_OF   0
#define TXTI_T_OF   0
#define TXTI_W      50
#define TXTI_H      CON_H

#define TXTN_L_OF   (TXTI_L_OF + TXTI_W + 10)
#define TXTN_T_OF  0
#define TXTN_W      (CON_W - TXTN_L_OF-10)
#define TXTN_H     CON_H
#endif

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | \
                    LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    satfav_item_con_keymap,satfav_item_con_callback,  \
    conobj, ID,1)

#define LDEF_BMP(root,varbmp,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,bmp_id)       \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define ldef_satfav_item(root,var_con,nxt_obj,var_bmp,var_txtidx,var_txt_name,ID,idu,idd,l,t,w,h,icon,idxstr,namestr) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txtidx,1) \
    LDEF_TXTIDX(&var_con,var_txtidx,&var_txt_name ,0,0,0,0,0,l + TXTI_L_OF, t + TXTI_T_OF,TXTI_W,TXTI_H,0,idxstr)   \
    LDEF_TXTNAME(&var_con,var_txt_name,NULL/*&varBmp*/ ,1,1,1,1,1,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,0,namestr)\
    LDEF_BMP(&var_con, var_bmp, NULL, 0,0,0,0,0,l, t + h, w, SCB_GAP, 0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 18, w, h - 36, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark) \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
        1,1,1,1,1, l,t,w,h,LIST_SH_IDX,LIST_HL_IDX,0,0,   \
        satfav_item_lst_keymap,satfav_item_lst_callback,    \
        flds,sb,mark,style,dep,count,mark)

static VACTION satfav_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satfav_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION satfav_item_lst_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satfav_item_lst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION satfav_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT satfav_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);



ldef_satfav_item(satfav_item_list,satfav_item_con1,&satfav_item_con2,satfav_item_bmp1,satfav_item_txtidx1,\
    satfav_item_txtnam1, 1,10, 2, CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,0, display_strs[0],display_strs[15])
ldef_satfav_item(satfav_item_list,satfav_item_con2,&satfav_item_con3,satfav_item_bmp2,satfav_item_txtidx2,\
    satfav_item_txtnam2, 2,1, 3, CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, 0, display_strs[1],display_strs[16])
ldef_satfav_item(satfav_item_list,satfav_item_con3,&satfav_item_con4,satfav_item_bmp3,satfav_item_txtidx3,\
    satfav_item_txtnam3, 3,2, 4, CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, 0, display_strs[2],display_strs[17])
ldef_satfav_item(satfav_item_list,satfav_item_con4,&satfav_item_con5,satfav_item_bmp4,satfav_item_txtidx4,\
    satfav_item_txtnam4, 4,3, 5, CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, 0, display_strs[3],display_strs[18])
ldef_satfav_item(satfav_item_list,satfav_item_con5,&satfav_item_con6,satfav_item_bmp5,satfav_item_txtidx5,\
    satfav_item_txtnam5, 5,4, 6, CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, 0, display_strs[4],display_strs[19])
ldef_satfav_item(satfav_item_list,satfav_item_con6,&satfav_item_con7,satfav_item_bmp6,satfav_item_txtidx6,\
    satfav_item_txtnam6, 6,5, 7, CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, 0, display_strs[5],display_strs[20])
ldef_satfav_item(satfav_item_list,satfav_item_con7,&satfav_item_con8,satfav_item_bmp7,satfav_item_txtidx7,\
    satfav_item_txtnam7, 7,6, 8, CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, 0, display_strs[6],display_strs[21])
ldef_satfav_item(satfav_item_list,satfav_item_con8,&satfav_item_con9,satfav_item_bmp8,satfav_item_txtidx8,\
    satfav_item_txtnam8, 8,7, 9, CON_L, CON_T + (CON_H + CON_GAP)*7,CON_W,CON_H, 0, display_strs[7],display_strs[22])
ldef_satfav_item(satfav_item_list,satfav_item_con9,&satfav_item_con10,satfav_item_bmp9,satfav_item_txtidx9,\
    satfav_item_txtnam9, 9,8, 10, CON_L, CON_T + (CON_H + CON_GAP)*8,CON_W,CON_H,   0, display_strs[8],display_strs[23])
ldef_satfav_item(satfav_item_list,satfav_item_con10,NULL  ,satfav_item_bmp10,satfav_item_txtidx10,\
    satfav_item_txtnam10,10,9, 1, CON_L, CON_T + (CON_H + CON_GAP)*9,CON_W,CON_H,   0, display_strs[9],display_strs[24])


LDEF_LISTBAR(satfav_item_list,satfav_list_bar,LIST_CNT,SCB_L,SCB_T, SCB_W, SCB_H)


LDEF_OL(g_win_satfav_list,satfav_item_list,NULL, CON_L, CON_T, CON_W, CON_H,    \
    LIST_STYLE, LIST_CNT, 0, satfav_items,&satfav_list_bar,NULL)

DEF_TEXTFIELD(satfav_title,&g_win_satfav_list,&satfav_item_list,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,TITLE_L,TITLE_T ,TITLE_W,TITLE_H, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL/*display_strs[30]*/)

DEF_CONTAINER(g_win_satfav_list,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    satfav_con_keymap,satfav_con_callback,  \
    (POBJECT_HEAD)&satfav_title, 1,0)

/*******************************************************************************
*   Local vriable & function declare
*******************************************************************************/

POBJECT_HEAD satfav_items[] =
{
    (POBJECT_HEAD)&satfav_item_con1,
    (POBJECT_HEAD)&satfav_item_con2,
    (POBJECT_HEAD)&satfav_item_con3,
    (POBJECT_HEAD)&satfav_item_con4,
    (POBJECT_HEAD)&satfav_item_con5,
    (POBJECT_HEAD)&satfav_item_con6,
    (POBJECT_HEAD)&satfav_item_con7,
    (POBJECT_HEAD)&satfav_item_con8,
    (POBJECT_HEAD)&satfav_item_con9,
    (POBJECT_HEAD)&satfav_item_con10,
};

static UINT8 sat_fav_list_flag = 0; /* 0 - SAT, 1 - FAV */

static void win_satfav_load_groupcount(void);
static void win_satfav_set_display(void);

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/

static VACTION satfav_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT satfav_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION satfav_item_lst_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        act = VACT_SELECT;  /* Select */
        break;
    default:
        act = VACT_PASS;
    break;

    }

    return act;
}

static PRESULT satfav_item_lst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    OBJLIST *ol = (OBJLIST*)p_obj;
    UINT16 sel = 0;
    UINT8 group_idx = 0;
    UINT8 av_flag = 0;

#ifdef MULTI_DESCRAMBLE
    UINT16 last_group_prog = 0xffff;
    P_NODE p_node_temp;

    MEMSET(&p_node_temp , 0,sizeof(P_NODE));
#endif
    switch(event)
    {
    case EVN_PRE_DRAW:
        win_satfav_set_display();
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_ITEM_POST_CHANGE:
        break;
    case EVN_POST_CHANGE:

        sel = osd_get_obj_list_single_select(ol);
        if(sel < sys_data_get_group_num() )
        {
            av_flag = sys_data_get_cur_chan_mode();

            group_idx = (UINT8)sel;
            if(1 == sat_fav_list_flag)
            {
                group_idx += sys_data_get_sate_group_num(av_flag);
                group_idx++;
                if(group_idx > MAX_GROUP_NUM)
                {
                    PRINTF("group_idx(%d) > MAX_GROUP_NUM(%d)\n", group_idx, MAX_GROUP_NUM);
                }
            }
#ifdef MULTI_DESCRAMBLE
            last_group_prog = sys_data_get_cur_group_cur_mode_channel();
            get_prog_at(last_group_prog,&p_node_temp);
            libc_printf("%s,prog_pos=%d,prog_id=%d\n",__FUNCTION__, last_group_prog,p_node_temp.prog_id);
            /*stop pre channel's filter,ts_stream and so on*/
            #if !(defined(CAS9_V6) && defined(CAS9_PVR_SID))
            if(FALSE == api_mcas_get_prog_record_flag(p_node_temp.prog_id))
            #endif
               //if the program is recording,don't stop descramble service
            {
                libc_printf("%s,stop service\n",__FUNCTION__);
                api_mcas_stop_service_multi_des(p_node_temp.prog_id,0xffff);
            }
#endif

            sys_data_change_group(group_idx);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_light_chanlist, TRUE);

            ret = PROC_LEAVE;
        }
        break;
    default:
        break;
    }

    return ret;
}


static VACTION satfav_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
    case V_KEY_SAT:
    case V_KEY_FAV:
        act  = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT satfav_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 vkey = 0xFF;
    UINT8 av_flag = 0;
    UINT8 grp_cnt = 0;
    UINT8 back_saved = 0;

    switch(event)
    {
    case EVN_PRE_OPEN:
        api_inc_wnd_count();
        if(MENU_OPEN_TYPE_KEY == (param2 & MENU_OPEN_TYPE_MASK))
        {
            vkey =  param2 & MENU_OPEN_PARAM_MASK;
        }
        if((V_KEY_SAT == vkey) || (V_KEY_FAV == vkey))
        {
            av_flag = sys_data_get_cur_chan_mode();

            sat_fav_list_flag = (V_KEY_SAT == vkey)? 0 : 1;
            if( 0 == sat_fav_list_flag)
            {
                grp_cnt = sys_data_get_sate_group_num(av_flag);
            }
            else
            {
                grp_cnt = sys_data_get_fav_group_num(av_flag);
            }
            if(0 == grp_cnt)
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg("No FAV Channels", NULL, 0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
                ret = PROC_LEAVE;
                api_dec_wnd_count();
                break;
            }
        }
        else
        {
            ret = PROC_LEAVE;
            api_dec_wnd_count();
            break;
        }

        win_satfav_load_groupcount();

        break;
    case EVN_POST_OPEN:
        break;
    case EVN_PRE_CLOSE:
        break;
    case EVN_POST_CLOSE:
        api_dec_wnd_count();
        break;
    case EVN_UNKNOWN_ACTION:
        break;
    default:
        break;
    }

    return ret;
}

static void win_satfav_load_groupcount(void)
{
    UINT8 av_flag = 0;
    OBJLIST *ol = NULL;
    UINT16 group_cnt = 0;
    UINT16 group_idx = 0;
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    UINT16 channel = 0;
    UINT16 select = 0;
    UINT16 page = 0;
    UINT16 title_id = 0;
    TEXT_FIELD *txt = NULL;
	BOOL   bret = FALSE;
	
    ol = &satfav_item_list;

    select = INVALID_POS_NUM;

    av_flag = sys_data_get_cur_chan_mode();
    group_idx = sys_data_get_cur_group_index();

    bret = sys_data_get_cur_mode_group_infor( group_idx, &group_type,&group_pos, &channel);

	if(bret)
	{
	    if(0 == sat_fav_list_flag)  /* SAT */
	    {
	        group_cnt = sys_data_get_sate_group_num(av_flag);
	        group_cnt++;
	        if(FAV_GROUP_TYPE == group_type)
	        {
	            group_idx = 0;
	        }
	        else
	        {
	            select = group_idx;
	        }
	    }
	    else    /* FAV */
	    {
	        group_cnt = sys_data_get_fav_group_num(av_flag);
	        if(group_type != FAV_GROUP_TYPE)
	        {
	            group_idx = 0;
	        }
	        else
	        {
	            group_idx -= sys_data_get_sate_group_num(av_flag);
	            group_idx--;
	            select = group_idx;
	        }
	    }
	}

    page = osd_get_obj_list_page(ol);

    osd_set_obj_list_count(ol, group_cnt);
    osd_set_obj_list_cur_point(ol, group_idx);
    osd_set_obj_list_new_point(ol, group_idx);
    osd_set_obj_list_top(ol,group_idx / page*page);
    osd_set_obj_list_single_select(ol, select);

    title_id = (0 == sat_fav_list_flag)?  RS_INFO_SATELLITE : RS_FAVORITE;
    txt = &satfav_title;
    osd_set_text_field_content(txt, STRING_ID,title_id);
}

static void win_satfav_set_display(void)
{
    UINT32 i = 0;
    UINT16 top = 0;
    UINT16 cnt = 0;
    UINT16 page = 0;
    UINT16 idx = 0;

    OBJLIST *ol = &satfav_item_list;
    TEXT_FIELD *txt = NULL;
    CONTAINER *item = NULL;
    UINT32 valid_idx = 0;
    UINT16 unistr[50] = {0};
    UINT8 group_type = 0;
    UINT8 group_idx = 0;
    UINT8 av_flag = 0;

    av_flag = sys_data_get_cur_chan_mode();

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)satfav_items[i];

        idx = top + i;
        group_idx = (UINT8)(idx);
        if(1 == sat_fav_list_flag)
        {
            group_idx += sys_data_get_sate_group_num(av_flag);
            group_idx++;
            if(group_idx > MAX_GROUP_NUM)
            {
                PRINTF("group_idx(%d) > MAX_GROUP_NUM(%d)\n", group_idx, MAX_GROUP_NUM);
            }
        }
        if(idx< cnt)
        {
            valid_idx = 1;
        }
        else
        {
            valid_idx = 0;
        }
        if(valid_idx)
        {
            get_chan_group_name((char *)unistr,(50*2),group_idx,&group_type);
        }

        /*IDX */
        txt = (TEXT_FIELD*)osd_get_container_next_obj(item);
        if(valid_idx)
        {
            osd_set_text_field_content(txt,STRING_NUMBER, idx + 1);
        }
        else
        {
            osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
        }
        /*Name*/
        txt = (TEXT_FIELD*)osd_get_objp_next(txt);
        if(valid_idx)
        {
            osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)unistr);
        }
        else
        {
            osd_set_text_field_content(txt,STRING_ANSI,(UINT32)"");
        }
    }

}

