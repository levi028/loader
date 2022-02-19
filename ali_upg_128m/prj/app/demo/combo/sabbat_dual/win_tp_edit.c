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
#include <api/libosd/osd_lib.h>
//#include <lib/libui/libgui2/osd/Osd_lib.h>
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "ctrl_popup_msg.h"
#include "control.h"
#include "win_com.h"

#define DEB_PRINT    soc_printf

/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_tp_edit,g_win_tplist;
extern OBJLIST     tplst_olist;

extern CONTAINER tpe_item1;    /* Freq */
extern CONTAINER tpe_item2;    /* Symb */
extern CONTAINER tpe_item3;    /* Pol */
extern CONTAINER tpe_item4;    /* Search Mode */
extern CONTAINER tpe_item5;    /* Search */

extern TEXT_FIELD tpe_title;
extern TEXT_FIELD tpe_txt1;
extern TEXT_FIELD tpe_txt2;
extern TEXT_FIELD tpe_txt3;
extern TEXT_FIELD tpe_txt4;
extern TEXT_FIELD tpe_txt5;
extern TEXT_FIELD tpe_btntxt_ok;
extern TEXT_FIELD tpe_btntxt_cancel;

extern EDIT_FIELD tpe_edt1;    /* Freq */
extern EDIT_FIELD tpe_edt2;    /* Symb */
extern MULTISEL   tpe_msel3;    /* Pol */
extern MULTISEL   tpe_msel4;    /* Search Mode */

#ifdef PLSN_SUPPORT
extern  CONTAINER   tpe_item6;  /* Plsn type -- 0:root, 1:gold*/
extern  CONTAINER   tpe_item7;  /* Plsn Value*/
extern  TEXT_FIELD  tpe_txt6;
extern  TEXT_FIELD  tpe_txt7;
extern  MULTISEL    tpe_msel5;
extern  EDIT_FIELD  tpe_edt3;
#endif

static VACTION tpe_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpe_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tpe_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpe_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tpe_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpe_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tpe_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpe_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION tpe_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tpe_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_POP_WIN_01_HD
#define WIN_HL_IDX      WSTL_POP_WIN_01_HD
#define WIN_SL_IDX      WSTL_POP_WIN_01_HD
#define WIN_GRY_IDX  WSTL_POP_WIN_01_HD

#define CON_SH_IDX   WSTL_TEXT_04_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_TEXT_04_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define TTILE_SH_IDX    WSTL_POP_TXT_SH_HD
#define TTILE_HL_IDX    TTILE_SH_IDX
#define TTILE_SL_IDX    TTILE_SH_IDX
#define TTILE_GRY_IDX   TTILE_SH_IDX


#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_04_HD//WSTL_BUTTON_08
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX    WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX    WSTL_BUTTON_POP_HL_HD

#ifndef SD_UI
#define    W_L     324//460//150
#define    W_T     140//180//110
#define    W_W     510
#ifdef PLSN_SUPPORT
#define     W_H     340
#else
#define    W_H     280
#endif

#define TITLE_L     (W_L + 20)
#define TITLE_T        (W_T + 20)
#define TITLE_W     (W_W - 40)
#define TITLE_H     40

#define CON_L        (W_L + 20)
#define CON_T        (TITLE_T +  TITLE_H + 4)
#define CON_W        (W_W - 40)
#define CON_H        40
#define CON_GAP        0

#define TXT_L_OF      10
#define TXT_W          200
#define TXT_H        40
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W - TXT_W-20)
#define SEL_H        40
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2)
#ifdef PLSN_SUPPORT
#define BTN_T_OF    ((CON_H + CON_GAP)*5 + 14)
#else
#define BTN_T_OF    ((CON_H + CON_GAP)*3 + 14)//100
#endif
#define BTN_W        150
#define BTN_H        38
#define BTN_GAP     60
#else
#define    W_L     150//460//150
#define    W_T     140//180//110
#define    W_W     400
#define    W_H     230


#define TITLE_L     (W_L + 20)
#define TITLE_T        (W_T + 20)
#define TITLE_W     (W_W - 40)
#define TITLE_H     32

#define CON_L        (W_L + 20)
#define CON_T        (TITLE_T +  TITLE_H + 4)
#define CON_W        (W_W - 40)
#define CON_H        32
#define CON_GAP        0

#define TXT_L_OF      10
#define TXT_W          120
#define TXT_H        32
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W - TXT_W-20)
#define SEL_H        32
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/2-10)
#define BTN_T_OF    ((CON_H + CON_GAP)*3 + 14)//100
#define BTN_W        150
#define BTN_H        30
#define BTN_GAP     40

#endif

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    tpe_item_keymap,tpe_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TTILE_SH_IDX,TTILE_HL_IDX,TTILE_SL_IDX,TTILE_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
    tpe_btn_keymap,tpe_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)


#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,EDT_HL_IDX,EDT_SL_IDX,EDT_GRY_IDX,   \
    tpe_item_edt_keymap,tpe_item_edt_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,NULL,sub,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    tpe_item_sel_keymap,tpe_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur_mode,pat,sub,str)

#define LDEF_MENU_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HLSUB_IDX,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    tpe_keymap,tpe_callback,  \
    nxt_obj, focus_id,0)

#ifdef PLSN_SUPPORT
UINT16 plsn_type_ids[] =
{
    RS_PLSN_ROOT,
    RS_PLSN_GOLD,
};

LDEF_TITLE(g_win_tp_edit, tpe_title, &tpe_item1, \
		TITLE_L, TITLE_T, TITLE_W, TITLE_H, 0)
LDEF_MENU_ITEM_EDT(g_win_tp_edit,tpe_item1,&tpe_item2,tpe_txt1,tpe_edt1, 1, 6, 2, \
		CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_DISPLAY_FREQUENCY, NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_pat,fre_sub, display_strs[0])
LDEF_MENU_ITEM_EDT(g_win_tp_edit,tpe_item2,&tpe_item3,tpe_txt2,tpe_edt2, 2, 1, 3, \
		CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INFO_SYMBOL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, symb_pat,symb_sub, display_strs[1])
LDEF_MENU_ITEM_SEL(g_win_tp_edit,tpe_item3,&tpe_item6,tpe_txt3,tpe_msel3, 3, 2, 4, \
		CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_INFO_POLARITY, STRING_ID, 0, 2, tp_polarity_ids)
LDEF_MENU_ITEM_SEL(g_win_tp_edit,tpe_item6,&tpe_item7,tpe_txt6,tpe_msel5, 4, 3, 5, \
		CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_PLSN_FLAG, STRING_ID, 0, 2, plsn_type_ids)
LDEF_MENU_ITEM_EDT(g_win_tp_edit,tpe_item7,&tpe_btntxt_ok,tpe_txt7,tpe_edt3, 5, 4, 6, \
		CON_L, CON_T + (CON_H + CON_GAP)*4, CON_W, CON_H, RS_PLSN_NUM, NORMAL_EDIT_MODE, CURSOR_SPECIAL, symb_pat,symb_sub, display_strs[2])
LDEF_TXT_BTN(g_win_tp_edit, tpe_btntxt_ok,&tpe_btntxt_cancel,6,7,7,5,1,	\
			CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)
LDEF_TXT_BTN(g_win_tp_edit, tpe_btntxt_cancel, NULL            ,7,6,6,5,1,	\
			CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)
LDEF_WIN(g_win_tp_edit, &tpe_title,W_L, W_T,W_W,W_H,1)
#define TP_FREQ_ID	1
#define TP_SYMB_ID	2
#define TP_POL_ID	3
#define PLSN_TYPE   4
#define PLSN_NUM	5
#define BTN_OK_ID	6
#define BTN_CANCEL_ID	7

#else

LDEF_TITLE(g_win_tp_edit, tpe_title, &tpe_item1, \
        TITLE_L, TITLE_T, TITLE_W, TITLE_H, 0)

LDEF_MENU_ITEM_EDT(g_win_tp_edit,tpe_item1,&tpe_item2,tpe_txt1,tpe_edt1, 1, 4, 2, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_DISPLAY_FREQUENCY, NORMAL_EDIT_MODE, CURSOR_SPECIAL, fre_pat,fre_sub, display_strs[0])

LDEF_MENU_ITEM_EDT(g_win_tp_edit,tpe_item2,&tpe_item3,tpe_txt2,tpe_edt2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INFO_SYMBOL, NORMAL_EDIT_MODE, CURSOR_SPECIAL, symb_pat,symb_sub, display_strs[1])

LDEF_MENU_ITEM_SEL(g_win_tp_edit,tpe_item3,&tpe_btntxt_ok,tpe_txt3,tpe_msel3, 3, 2, 4, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_INFO_POLARITY, STRING_ID, 0, 2, tp_polarity_ids)


LDEF_TXT_BTN(g_win_tp_edit, tpe_btntxt_ok,&tpe_btntxt_cancel,4,5,5,3,1,    \
            CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_SAVE)

LDEF_TXT_BTN(g_win_tp_edit, tpe_btntxt_cancel, NULL            ,5,4,4,3,1,    \
            CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CANCEL)

LDEF_WIN(g_win_tp_edit, &tpe_title,W_L, W_T,W_W,W_H,1)


#define TP_FREQ_ID    1
#define TP_SYMB_ID    2
#define TP_POL_ID    3
#define BTN_OK_ID    4
#define BTN_CANCEL_ID    5
//#define TP_SRCHMODE_ID    4
//#define TP_SRCH_ID    5

#endif

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/



typedef enum
{
    EDIT_TP_MODE_EDIT = 0,
    EDIT_TP_MODE_ADD,
}EDIT_TP_MODE_T;

enum {
	PLS_TYPE_ROOT = 0,
	PLS_TYPE_GOLD,
};

UINT8  edit_tp_ok = 0; /* 0 - cancel : 1 save*/
UINT8  edit_tp_mode =0;
UINT32 edit_tp_sat_id =0;
UINT32 edit_tp_id =0;
UINT32 edit_tp_exist =0;
UINT8  is_exist[] = "TP already exists! Select [YES] Jump exist tp,Select [NO] Continue Add";
win_popup_choice_t edit_tp_yesno = 0;
static void win_tpe_set_mode(UINT32 mode,UINT32 sat_id,UINT32 tp_id);
static void win_tpe_load_setting(void);
static UINT32 win_tpe_check_input(void);
#ifdef PLSN_SUPPORT
static void change_pls_number(UINT32 pls_type);
#endif

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION tpe_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_0:    case V_KEY_1:    case V_KEY_2:    case V_KEY_3:
    case V_KEY_4:    case V_KEY_5:    case V_KEY_6:    case V_KEY_7:
    case V_KEY_8:    case V_KEY_9:
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


static PRESULT tpe_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    default:
        break;
    }

    return ret;
}

static VACTION tpe_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT tpe_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_CHANGE:
#ifdef PLSN_SUPPORT
        if (osd_get_obj_id((POBJECT_HEAD)(p_obj->p_root)) == PLSN_TYPE)
        {
            change_pls_number(*((UINT32 *)param1));
        }
#endif
        break;
    case EVN_POST_CHANGE:
		break;
    default:
        break;
    }

    return ret;
}

static VACTION tpe_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT tpe_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION tpe_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if(key == V_KEY_ENTER)
        act = VACT_ENTER;

    return act;

}

static PRESULT tpe_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8	b_id = 0;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        if(b_id == BTN_OK_ID)
        {
            edit_tp_exist = 0;
            if((edit_tp_exist = win_tpe_check_input()) == 0)
            {
                edit_tp_ok = 1;
                ret = PROC_LEAVE;
            }

            if(edit_tp_yesno == WIN_POP_CHOICE_YES)
            {
                edit_tp_ok = 0;
                ret = PROC_LEAVE;
            }
            else if(edit_tp_yesno == WIN_POP_CHOICE_NO)//NO
            {
                edit_tp_ok = 0;
                edit_tp_exist = 0;
            }



        }
        else if(b_id == BTN_CANCEL_ID)
            ret = PROC_LEAVE;

		break;
    default:
        break;
    }

    return ret;
}

static VACTION tpe_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
      case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
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

static PRESULT tpe_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        win_tpe_load_setting();
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

static void win_tpe_set_mode(UINT32 mode,UINT32 sat_id,UINT32 tp_id)
{
    if(mode == 0)
        edit_tp_mode = EDIT_TP_MODE_EDIT;
    else
        edit_tp_mode = EDIT_TP_MODE_ADD;

    edit_tp_sat_id     = sat_id;
    edit_tp_id         = tp_id;
}

static void win_tpe_load_setting(void)
{
	EDIT_FIELD	*edf = NULL;
	MULTISEL	*msel = NULL;
    T_NODE     t_node;

    MEMSET(&t_node, 0, sizeof(t_node));
    edit_tp_ok = 0;
    edit_tp_exist = 0;
    if(edit_tp_mode == EDIT_TP_MODE_EDIT)
        get_tp_by_id(edit_tp_id, &t_node);
    else
    {
        t_node.frq = 3000;
        t_node.sym = 1000;
        t_node.pol = 0;
    }

    edf = &tpe_edt1;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" MHz");
    osd_set_edit_field_content(edf, STRING_NUMBER, t_node.frq);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);


    edf = &tpe_edt2;
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)" KS/s");
    osd_set_edit_field_content(edf, STRING_NUMBER, t_node.sym);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

    msel = &tpe_msel3;
    osd_set_multisel_sel(msel, t_node.pol);
    osd_set_edit_field_style(edf,NORMAL_EDIT_MODE | SELECT_STATUS);

#ifdef PLSN_SUPPORT
    msel = &tpe_msel5;
    osd_set_multisel_sel(msel, 1);
    edf = &tpe_edt3;
    osd_set_edit_field_pattern(edf, "r0~524288");
    osd_set_edit_field_suffix(edf, STRING_ANSI, (UINT32)"");
    osd_set_edit_field_content(edf, STRING_NUMBER, t_node.pls_num);
    osd_set_edit_field_style(edf, NORMAL_EDIT_MODE | SELECT_STATUS);
#endif
}
static UINT32 win_tpe_check_input(void)
{
	EDIT_FIELD	*edf = NULL;
	MULTISEL	*msel = NULL;
    T_NODE     t_node;
	UINT32 freq =0,symb=0,pol=0;
	UINT32 i=0,tp_cn=0;
	UINT8 back_saved=0;
#ifdef MULTISTREAM_SUPPORT
    UINT8 isid = 0xFF;
#endif
#ifdef PLSN_SUPPORT
    UINT32 plsn_type = 0;
    UINT32 plsn_val = 0;
#endif

    MEMSET(&t_node, 0 , sizeof(t_node));
    edit_tp_yesno = WIN_POP_CHOICE_NULL;
    edf = &tpe_edt1;
    osd_get_edit_field_int_value(edf, &freq);

    edf = &tpe_edt2;
    osd_get_edit_field_int_value(edf, &symb);

    msel = &tpe_msel3;
    pol  = osd_get_multisel_sel(msel);

#ifdef PLSN_SUPPORT
    msel = &tpe_msel5;
    plsn_type = osd_get_multisel_sel(msel);

    edf = &tpe_edt3;
    osd_get_edit_field_int_value(edf, &plsn_val);
#endif

    if(edit_tp_mode == EDIT_TP_MODE_EDIT)
    {
        get_tp_by_id(edit_tp_id, &t_node);
    #ifdef PLSN_SUPPORT
        if ((t_node.frq == freq) && (t_node.sym = symb) && (t_node.pol == pol) && (t_node.pls_num == plsn_val))
    #else
		if((t_node.frq == freq) &&(t_node.sym == symb) && (t_node.pol == pol))
    #endif
            return 0;
        else
            goto CHECK_DUPLICATE;
    }
    else
        goto CHECK_DUPLICATE;

CHECK_DUPLICATE:
    tp_cn = get_tp_num_sat(edit_tp_sat_id);
    for(i=0;i<tp_cn;i++)
    {
        get_tp_at(edit_tp_sat_id, i, &t_node);
		if((edit_tp_mode == EDIT_TP_MODE_EDIT) && (t_node.tp_id == edit_tp_id))
            continue;
    #ifdef MULTISTREAM_SUPPORT
        if((t_node.frq == freq) && (t_node.sym == symb) && (t_node.pol == pol) && (t_node.plp_id == isid))
    #else
		if((t_node.frq == freq) && (t_node.sym == symb) && (t_node.pol == pol))
    #endif
        {
            //win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            //win_compopup_set_msg(NULL,NULL,RS_POPUP_TP_EXIST);
            win_compopup_set_msg((char *)is_exist,NULL,0);
            edit_tp_yesno = win_compopup_open_ext(&back_saved);
            if(edit_tp_yesno == WIN_POP_CHOICE_YES)
            //when choice is no, tp add menu is still on the tplist menu,so changefocus result in UI destroy for SD_UI.
            {
                //modify bomur 2011 08 31
                //instruction:when the tplist has own the new editted tp, the focus will point to the old tp
                osd_change_focus((POBJECT_HEAD)&tplst_olist, i, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
                //modify end
            }
            return (i+1);///1;
        }
    }

    return 0;
}



UINT32 win_tpe_open(UINT32 mode,UINT32 sat_id,UINT32 tp_id)
{

	CONTAINER* win = NULL;
    PRESULT ret = PROC_LOOP;
	UINT32 hkey = 0;
	TEXT_FIELD* txt = NULL;
	UINT16 str_id =0;

    BOOL old_value = ap_enable_key_task_get_key(TRUE);
     txt = &tpe_title;
    if(mode == 0)
        str_id = RS_INFO_EDIT_TP;
    else
        str_id = RS_INFO_ADD_TP;
    osd_set_text_field_content(txt, STRING_ID,(UINT32)str_id);

    win = &g_win_tp_edit;
    osd_set_container_focus(win, TP_FREQ_ID);
    win_tpe_set_mode(mode,sat_id,tp_id);

    osd_obj_open((POBJECT_HEAD)win, 0);

    while(ret != PROC_LEAVE)
    {
        hkey = ap_get_popup_msg(C_POPUP_MSG_PASS_CI);
        if((hkey == INVALID_HK) || (hkey == INVALID_MSG))
            continue;

        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY<<16),hkey, 0);
    }
    ap_enable_key_task_get_key(old_value);

    return edit_tp_ok;
}

#ifdef PLSN_SUPPORT
UINT32 pls_root_to_gold(UINT32 root)
{
    UINT32 val = root;

    nim_io_control(g_nim_dev, NIM_DRIVER_PLSN_ROOT_TO_GOLD, &val);
    return val;
}

UINT32 pls_gold_to_root(UINT32 gold)
{
    UINT32 val = gold;

    nim_io_control(g_nim_dev, NIM_DRIVER_PLSN_GOLD_TO_ROOT, &val);
    return val;
}

static void change_pls_number(UINT32 pls_type)
{
    EDIT_FIELD *edf = &tpe_edt3;
	UINT32 pls_val = 0, pls_tmp = 0;
	osd_get_edit_field_int_value(edf, &pls_val);
	if (pls_type == PLS_TYPE_ROOT)
	{
		pls_tmp = pls_gold_to_root(pls_val);
	}
	else if (pls_type == PLS_TYPE_GOLD)
	{
		pls_tmp = pls_root_to_gold(pls_val);
	}
	osd_set_edit_field_content(edf, STRING_NUMBER, pls_tmp);
	osd_draw_object((POBJECT_HEAD)edf, C_UPDATE_ALL);
}

void win_tpe_get_setting(UINT32* freq, UINT32* symb, UINT32* pol, UINT32* pls)
{
	EDIT_FIELD	*edf;
	MULTISEL	*msel;
	INT16 pls_type = -1;
	UINT32 pls_val = 0;
	edf = &tpe_edt1;
	osd_get_edit_field_int_value(edf, freq);
	edf = &tpe_edt2;
	osd_get_edit_field_int_value(edf, symb);
	msel = &tpe_msel3;
	*pol  = osd_get_multisel_sel(msel);	
	msel = &tpe_msel5;
	pls_type = osd_get_multisel_sel(msel);
	edf = &tpe_edt3;
	osd_get_edit_field_int_value(edf, &pls_val);
	if (pls_type == PLS_TYPE_ROOT)
	{
		*pls = pls_root_to_gold(pls_val);
	}
	else if (pls_type == PLS_TYPE_GOLD)
	{
		*pls = pls_val;
	}
}


#else
void win_tpe_get_setting(UINT32* freq, UINT32* symb, UINT32* pol)
{
	EDIT_FIELD	*edf = NULL;
	MULTISEL	*msel = NULL;


    edf = &tpe_edt1;
    osd_get_edit_field_int_value(edf, freq);

    edf = &tpe_edt2;
    osd_get_edit_field_int_value(edf, symb);

    msel = &tpe_msel3;
    *pol  = osd_get_multisel_sel(msel);
}
#endif

