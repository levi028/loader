/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_cc.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#if (ISDBT_CC==1)
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libpub29/lib_pub27.h>
//#include <api/libpub29/lib_as.h>

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

#if (ISDBT_CC==1)
//#include <hld/sdec/sdec.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#endif

//#define ICC_PRINTF     libc_printf

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_cc;
extern TEXT_FIELD isdbt_cc_title;
extern OBJLIST     isdbt_cc_ol;
extern SCROLL_BAR     audio_scb;
extern BITMAP    isdbt_cc_mark;

extern CONTAINER isdbt_cc_item1;
extern CONTAINER isdbt_cc_item2;
extern CONTAINER isdbt_cc_item3;
extern CONTAINER isdbt_cc_item4;
extern CONTAINER isdbt_cc_item5;
extern CONTAINER isdbt_cc_item6;

extern TEXT_FIELD isdbt_cc_txt1;
extern TEXT_FIELD isdbt_cc_txt2;
extern TEXT_FIELD isdbt_cc_txt3;
extern TEXT_FIELD isdbt_cc_txt4;
extern TEXT_FIELD isdbt_cc_txt5;
extern TEXT_FIELD isdbt_cc_txt6;

static VACTION cc_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT cc_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION cc_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT cc_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION cc_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT cc_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX      WSTL_WIN_SLIST_02_8BIT
#define WIN_HL_IDX      WIN_SH_IDX
#define WIN_SL_IDX      WIN_SH_IDX
#define WIN_GRY_IDX  WIN_SH_IDX

#define TITLE_SH_IDX     WSTL_MIXBACK_IDX_01_8BIT

#define LST_SH_IDX        WSTL_NOSHOW_IDX
#define LST_HL_IDX        WSTL_NOSHOW_IDX
#define LST_SL_IDX        WSTL_NOSHOW_IDX
#define LST_GRY_IDX        WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_TEXT_15_8BIT    //WSTL_TEXT_12
#define CON_HL_IDX   WSTL_BUTTON_05_8BIT    //WSTL_BUTTON_05
#define CON_SL_IDX   WSTL_TEXT_15_8BIT    //WSTL_TEXT_12
#define CON_GRY_IDX  WSTL_TEXT_14
#define TXT_SH_IDX   WSTL_TEXT_15_8BIT
#define TXT_HL_IDX   WSTL_TEXT_28_8BIT
#define TXT_SL_IDX   WSTL_TEXT_15_8BIT
#define TXT_GRY_IDX  WSTL_TEXT_14

#define MARK_SH_IDX        WSTL_MIXBACK_IDX_06_8BIT  //WSTL_MIXBACK_BLACK_IDX
#define MARK_HL_IDX        MARK_SH_IDX             //WSTL_MIXBACK_BLACK_IDX
#define MARK_SL_IDX        MARK_SH_IDX             //WSTL_MIXBACK_BLACK_IDX
#define MARK_GRY_IDX    MARK_SH_IDX             //WSTL_MIXBACK_BLACK_IDX

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT

#ifdef SD_UI
//SD UI
#define    W_L              60
#define    W_T              60
#define    W_W              200
#define    W_H              280

#define TITLE_L             (W_L )
#define TITLE_T             (W_T + 6)
#define TITLE_W             (W_W - 8)
#define TITLE_H             48

#define LST_L        (W_L + 10)
#define LST_T        (W_T + 66)
#define LST_W        (W_W - 36)
#define LST_H        (W_H - TITLE_H - 36)

#define SCB_L         (LST_L + LST_W+4 )
#define SCB_T         LST_T
#define SCB_W         12
#define SCB_H         LST_H

#define CON_L        LST_L
#define CON_T        LST_T
#define CON_W        LST_W
#define CON_H        32
#define CON_GAP        0

#define TXT_L_OF        30
#define TXT_W           (CON_W - 14)
#define TXT_H           CON_H
#define TXT_T_OF        ((CON_H - TXT_H)/2)
#else
#define    W_L             74
#define    W_T             60
#define    W_W            400
#define    W_H             324

#define TITLE_L         (W_L )
#define TITLE_T        (W_T + 6)
#define TITLE_W         (W_W - 20)
#define TITLE_H         48

#define LST_L        (W_L + 20)
#define LST_T        (W_T + 66)
#define LST_W        (W_W - 56)
#define LST_H        (W_H - TITLE_H - 36)

#define SCB_L         (LST_L + LST_W+4 )
#define SCB_T         LST_T
#define SCB_W         12
#define SCB_H         LST_H

#define CON_L        LST_L
#define CON_T        LST_T
#define CON_W        LST_W
#define CON_H        40
#define CON_GAP        0

#define TXT_L_OF      30
#define TXT_W        (CON_W - 34)
#define TXT_H        28
#define TXT_T_OF        ((CON_H - TXT_H)/2)
#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
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

#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,str)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

#define LDEF_MARKBMP(root,var_bmp,l,t,w,h,icon)        \
  DEF_BITMAP(var_bmp,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SL_IDX,MARK_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    cc_list_keymap,cc_list_callback,    \
    flds,sb,mark,style,dep,count,selary)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    cc_keymap,cc_callback,  \
    nxt_obj, focus_id,0)

LDEF_TITLE(g_win_cc, isdbt_cc_title, &isdbt_cc_ol, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_CC_CAPTION)

LDEF_MENU_ITEM(g_win_cc, isdbt_cc_item1, &isdbt_cc_item2,isdbt_cc_txt1,1,6,2,
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H,display_strs[0])
LDEF_MENU_ITEM(g_win_cc, isdbt_cc_item2, &isdbt_cc_item3,isdbt_cc_txt2,2,1,3,
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,display_strs[1])
LDEF_MENU_ITEM(g_win_cc, isdbt_cc_item3, &isdbt_cc_item4,isdbt_cc_txt3,3,2,4,
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H,display_strs[2])
LDEF_MENU_ITEM(g_win_cc, isdbt_cc_item4, &isdbt_cc_item5,isdbt_cc_txt4,4,3,5,
        CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H,display_strs[3])
LDEF_MENU_ITEM(g_win_cc, isdbt_cc_item5, &isdbt_cc_item6,isdbt_cc_txt5,5,4,6,
        CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H,display_strs[4])
LDEF_MENU_ITEM(g_win_cc, isdbt_cc_item6,NULL             ,isdbt_cc_txt6,6,5,1,
        CON_L, CON_T + (CON_H + CON_GAP)*5, CON_W, CON_H,display_strs[5])

POBJECT_HEAD isdbt_cc_list_item[] =
{
    (POBJECT_HEAD)&isdbt_cc_item1,
    (POBJECT_HEAD)&isdbt_cc_item2,
    (POBJECT_HEAD)&isdbt_cc_item3,
    (POBJECT_HEAD)&isdbt_cc_item4,
    (POBJECT_HEAD)&isdbt_cc_item5,
    (POBJECT_HEAD)&isdbt_cc_item6,
};
//need to use definition to assign page num
LDEF_LISTBAR(isdbt_cc_ol,isdbt_cc_scb,6,SCB_L,SCB_T, SCB_W, SCB_H)

LDEF_MARKBMP(isdbt_cc_ol,isdbt_cc_mark, CON_L + 8, CON_T, 20, CON_H,IM_MSELECT );    // IM_ICON_SELECT

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_cc,isdbt_cc_ol,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 6, 0,isdbt_cc_list_item,&isdbt_cc_scb,(POBJECT_HEAD)&isdbt_cc_mark,NULL)

LDEF_WIN(g_win_cc, &isdbt_cc_title,W_L, W_T,W_W,W_H,1)

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

static TEXT_FIELD* text_items[] =
{
    &isdbt_cc_txt1,
    &isdbt_cc_txt2,
    &isdbt_cc_txt3,
    &isdbt_cc_txt4,
    &isdbt_cc_txt5,
    &isdbt_cc_txt6,
};
static PRESULT win_cc_load(void);
static void win_cc_set_display(void);
static void win_cc_change(void);
static PRESULT win_cc_message_proc(UINT32 msg_type, UINT32 msg_code);

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION cc_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    return act;
}

static PRESULT cc_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    return ret;
}

static VACTION cc_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {
        case V_KEY_UP:        act = VACT_CURSOR_UP;        break;
        case V_KEY_DOWN:    act = VACT_CURSOR_DOWN;    break;
        case V_KEY_P_UP:    act = VACT_CURSOR_PGUP;        break;
        case V_KEY_P_DOWN:    act = VACT_CURSOR_PGDN;        break;
        case V_KEY_ENTER:    act = VACT_SELECT;            break;
        default:                act = VACT_PASS;
    }
    return act;
}

static PRESULT cc_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    volatile UINT16 oldsel,newsel;

    switch(event)
    {
        case EVN_PRE_DRAW:
            win_cc_set_display();
            break;
        case EVN_POST_DRAW:            break;
        case EVN_ITEM_PRE_CHANGE:        break;
        case EVN_ITEM_POST_CHANGE:        break;
        case EVN_PRE_CHANGE:            break;
        case EVN_POST_CHANGE:
            oldsel = (UINT16)param1;
            newsel = (UINT16)param2;
            if(oldsel!=newsel)
            {
                win_cc_change();
            }
                  api_dec_wnd_count();
            return PROC_LEAVE;
            break;
        default:            ;
    }
    return ret;
}

static VACTION cc_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
		case V_KEY_RED:		act = VACT_CLOSE;			break;
        default:            act = VACT_PASS;
    }
    return act;
}

static PRESULT cc_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //VACTION unact;

    switch(event)
    {
    case EVN_PRE_OPEN:
        api_inc_wnd_count();
        if(PROC_LEAVE==win_cc_load())    // No CC Data, Force Exit CC menu
        {
        #ifdef SUPPORT_BC
            UINT8 back_saved;
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_msg_ext(NULL, NULL, RS_DISPLAY_NO_DATA);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(1000);
            win_compopup_smsg_restoreback();
        #else
            win_com_popup_open(WIN_POPUP_TYPE_SMSG,NULL, RS_DISPLAY_NO_DATA);
            osal_task_sleep(1000);
            win_compopup_close();
        #endif
                  api_dec_wnd_count();
            return PROC_LEAVE;
        }
        break;
    case EVN_POST_OPEN:        break;
    case EVN_PRE_CLOSE:        break;
    case EVN_POST_CLOSE:
        //libc_printf("\nCon-do EVN_POST_CLOSE!!");
        api_dec_wnd_count();
        break;
    }
    return ret;
}

 PRESULT win_cc_load(void)
{
    OBJLIST* ol;
    UINT8 bcnt,bsel,btop_idx,bcur_idx,bdep;
    UINT8 b_cc_lang=0,b_lang_num=0,b_show=0;
    SYSTEM_DATA* sys_data;

#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang*cclanglst;
    isdbtcc_get_language(&cclanglst, &b_lang_num);
#endif

    if(0==b_lang_num)
        return PROC_LEAVE;

    bcnt=b_lang_num+1;
    sys_data = sys_data_get();
    b_show=sys_data->osd_set.subtitle_display;
    b_cc_lang=sys_data->osd_set.subtitle_lang;
    ol = &isdbt_cc_ol;

    if(0==b_show)
        bsel=bcnt-1;
    else
    {
        if(b_cc_lang<bcnt)
            bsel=b_cc_lang;
        else
            bsel=0;
    }
    bdep = osd_get_obj_list_page(ol);
    bcur_idx=bsel;
    btop_idx = bcur_idx / bdep * bdep;

    osd_set_obj_list_count(ol, bcnt);
    osd_set_obj_list_single_select(ol, bsel);
    osd_set_obj_list_top(ol, btop_idx);
    osd_set_obj_list_cur_point(ol, bcur_idx);
    osd_set_obj_list_new_point(ol, bcur_idx);
    return PROC_PASS;
}

UINT8 ccto_upper(UINT8 b_char)
{
    if((b_char>=(UINT8)'a') && (b_char<=(UINT8)'z'))
        return b_char-(UINT8)'a'+(UINT8)'A';
    else
        return b_char;
}
void win_cc_set_display(void)
{
    TEXT_FIELD *txt;
    OBJLIST* ol;
    //UINT16 i,cnt,top_idx,cur_idx,dep,str_len;
    UINT16 i,cnt,top_idx,cur_idx,dep;
    UINT16 cc_pid;
    UINT8 b_lang_num;
    char lang[4];
    char str[30];

#if (ISDBT_CC == 1)
    //struct t_isdbtcc_lang *cclanglst,*cclst;
	struct t_isdbtcc_lang *cclanglst;

    isdbtcc_get_language(&cclanglst, &b_lang_num);

    ol = &isdbt_cc_ol;
    cnt = osd_get_obj_list_count(ol);
    dep = osd_get_obj_list_page(ol);
    top_idx = osd_get_obj_list_top(ol);

    for(i=0;i<dep;i++)
    {
        cur_idx = top_idx + i;
        txt = text_items[i];

        if((cur_idx+1) < cnt )
        {
            cc_pid =cclanglst[i].pid;
            MEMSET(lang,0,sizeof(lang));
            MEMCPY(lang, cclanglst[i].lang, 3);
            if(STRLEN(lang) == 0)
                snprintf(str,30," CC %d  (%d) ",cur_idx + 1,cc_pid & 0x1FFF);
             else
             {
                 UINT8 j;
                for(j=0;j<3;j++)
                    lang[j]=ccto_upper(lang[j]);
                #if 0
                sprintf(str," CC%d %s  (%d)",cur_idx+1,lang,cc_pid & 0x1FFF);
                #else
                 snprintf(str,30," %s  (%d)",lang,cc_pid & 0x1FFF);
                #endif
             }
            //str_len = STRLEN(str);
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }
        else if((cur_idx+1)==cnt)
        {
            snprintf(str,30," %s","CC OFF");
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
        }
        else
        {    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)""); }
    }
#endif
}
void win_cc_change(void)    //monitor new pid for cc or Display off
{
    OBJLIST* ol=&isdbt_cc_ol;
    UINT8 bsel;
    UINT8  __MAYBE_UNUSED__ b_cc_lang,b_lang_num,b_show;
    SYSTEM_DATA* sys_data=sys_data_get();

#if (ISDBT_CC == 1)
    struct t_isdbtcc_lang*cclanglst;
    isdbtcc_get_language(&cclanglst, &b_lang_num);
#endif
    b_show=sys_data->osd_set.subtitle_display;
    b_cc_lang=sys_data->osd_set.subtitle_lang;

    bsel = osd_get_obj_list_single_select(ol);
    if(b_lang_num==bsel)
    {
        sys_data->osd_set.subtitle_display = 0;/*select off option*/
        sys_data->osd_set.subtitle_lang = 0xFF;
    }
    else if(b_lang_num>bsel)
    {
        sys_data->osd_set.subtitle_display = 1;/*subtitle on*/
        sys_data->osd_set.subtitle_lang = bsel;/*set_sub_lang_idx is used in api_osd_mode_change function*/
    }
    api_osd_mode_change(OSD_NO_SHOW);
}

#endif

