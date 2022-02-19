#include <sys_config.h>

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_popup.h"


/*******************************************************************************
* Objects declaration
*******************************************************************************/
extern CONTAINER g_win_multiviewset;

extern CONTAINER multiviewset_item0;
extern TEXT_FIELD multiviewset_txt0;
extern TEXT_FIELD multiviewset_line0;
extern MULTISEL  multiviewset_sel0;


static VACTION multiviewset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT multiviewset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION multiviewset_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT multiviewset_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION multiviewset_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT multiviewset_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-320)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    multiviewset_item_keymap,multiviewset_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    multiviewset_item_sel_keymap,multiviewset_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    multiviewset_keymap,multiviewset_callback,  \
    nxt_obj, focus_id,0)

typedef enum
{
    MULTIVIEWSET_ID = 1,
}MULTIVIEWSETTING_ID;

static UINT16 multiviewset_ids[] =
{
    RS_COMMON_3x3,
    RS_COMMON_41
};

LDEF_MENU_ITEM(g_win_multiviewset,multiviewset_item0,NULL,multiviewset_txt0,multiviewset_sel0,multiviewset_line0, 1, 1, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_MULTIVIEW_SETTING, \
        STRING_ID, 0, sizeof(multiviewset_ids)/sizeof(UINT16), multiviewset_ids)
LDEF_WIN(g_win_multiviewset,&multiviewset_item0,W_L, W_T, W_W, W_H, 1)

CONTAINER *multiviewset_menu_items[] =
{
    &multiviewset_item0,
};


/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/
static VACTION multiviewset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {
          case V_KEY_LEFT:
            act = VACT_DECREASE;
            break;
        case V_KEY_RIGHT:
            act = VACT_INCREASE;
            break;
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            act = VACT_PASS;
            break;
    }

    return act;
}
void win_multiview_enter_key(POBJECT_HEAD p_obj,UINT8 id)
{
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
	__MAYBE_UNUSED__ UINT16 sel = 0;
    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&param, 0 ,sizeof(param));
    rect.u_left=CON_L+TXTS_L_OF;
    rect.u_width=TXTS_W;
    rect.u_height=250;
    param.selecttype = POP_LIST_SINGLESELECT;

    switch(id)
    {
        case MULTIVIEWSET_ID:
#ifdef MULTIVIEW_SUPPORT
            #ifndef SD_UI
            rect.u_height = 300;
            rect.u_top=CON_T + (CON_H + CON_GAP)*1;
            #else
            rect.u_height = 250;
            rect.u_top=CON_T + (CON_H)*1;
            #endif
            param.cur = sys_data_get_multiviewmode();
            sel  = win_com_open_sub_list(POP_LIST_MULTIVIEW_SET,&rect,&param);
            if (sel == param.cur)
            {
                break;
            }
            sys_data_set_multiviewmode(sel);
            osd_set_multisel_sel(&multiviewset_sel0, sel);
            osd_notify_event(p_obj,EVN_POST_CHANGE,(UINT32)sel, 0);
            osd_track_object((POBJECT_HEAD)&multiviewset_item0,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
#endif
            break;
        default:
            break;

    }
}


static PRESULT multiviewset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT32 sel =0 ;
	UINT8 b_id = 0;
	UINT8 unact = 0;
	//UINT32 draw_cmd = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL;

    b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
        case EVN_PRE_CHANGE:
            sel = *((UINT32*)param1);
            break;
        case EVN_POST_CHANGE:
            sel = param1;
            if (b_id == MULTIVIEWSET_ID)
            {
				//UINT8 action = sel ? C_ATTR_ACTIVE : C_ATTR_INACTIVE;
                sys_data_set_multiviewmode(sel);

            }
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16) ;
            if(unact == VACT_ENTER)
            {
                win_multiview_enter_key(p_obj,b_id);
            }

            break;
        default:
            break;
    }
     return ret;
}

static VACTION multiviewset_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT multiviewset_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION multiviewset_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT multiviewset_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	//CONTAINER* root = (CONTAINER*) p_obj;

    switch(event)
    {
        case EVN_PRE_OPEN:
            osd_set_multisel_sel(&multiviewset_sel0, sys_data_get_multiviewmode());
            wincom_open_title(p_obj, RS_MULTIVIEW_SETTING, 0);
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_PRE_CLOSE:
            sys_data_save(1);
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG; /* Make OSD not flickering */
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_MSG_GOT:
             break;
        default:
            break;
    }

    return ret;
}

UINT8 win_multiview_get_num()
{
    UINT8 ID=osd_get_focus_id((POBJECT_HEAD)&g_win_multiviewset);
    UINT8 num=0;

    switch(ID)
    {
        case MULTIVIEWSET_ID:
            num=2;
            break;
        default:
            break;
    }
    return num;
}
PRESULT comlist_menu_multiview_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	INT16 i=0,w_top=0;
	UINT8 id=0;
	//char str_len=0;
    OBJLIST* ol= NULL;

    PRESULT cb_ret = PROC_PASS;

    ol = (OBJLIST*)p_obj;

    if(event==EVN_PRE_DRAW)
    {
        id=osd_get_focus_id((POBJECT_HEAD)&g_win_multiviewset);
        switch(id)
        {
            case MULTIVIEWSET_ID:
                    w_top = osd_get_obj_list_top(ol);
			    	for(i=0;((i<ol->w_dep) && ((i+w_top)<ol->w_count));i++)
                    {
                        win_comlist_set_str(i + w_top,NULL,NULL,multiviewset_ids[i + w_top]);
                    }
                break;
            default:
                break;
        }
    }
    else if(event == EVN_POST_CHANGE)
    {
        cb_ret = PROC_LEAVE;
    }

    return cb_ret;
}



