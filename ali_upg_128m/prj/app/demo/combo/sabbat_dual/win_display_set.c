/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_display_set.c
*
*    Description:   The realize of display setting
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_com_list.h"
#include "menus_root.h"
#include "win_display_set.h"
#ifdef DISPLAY_SETTING_SUPPORT

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static UINT32 m_dis_set_range[][3];
static VACTION dis_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT dis_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION dis_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT dis_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION dis_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT dis_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD

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


#define TXT_L_OF    TXTN_L_OF
#define TXT_W       TXTN_W
#define TXT_H       TXTN_H
#define TXT_T_OF    TXTN_T_OF

#define SEL_L_OF    TXTS_L_OF
#define SEL_W       TXTS_W
#define SEL_H       TXTS_H
#define SEL_T_OF    TXTS_T_OF

#define MIN_BRIGHTNESS_VALUE    0
#define MAX_BRIGHTNESS_VALUE    100
#define MAX_BRIGHTNESS_NUM      101

enum
{
    IDC_DIS_BRIGHT = 1,
    IDC_DIS_CONTRAST,
    IDC_DIS_SATURATION,
    IDC_DIS_HUE,
    IDC_DIS_SHARP,
    IDC_DIS_MAX,
};

#define LDEF_CON(root, var_con,nxt_obj,id,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    dis_item_con_keymap,dis_item_con_callback,  \
    conobj, focus_id,1)


#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, id, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    dis_item_sel_keymap,dis_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,id,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,id,id,id,idu,idd,l,t,w,h,&var_txt,id)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/,id, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    dis_con_keymap,dis_con_callback,  \
    nxt_obj, focus_id,0)

LDEF_MENU_ITEM(g_win_display_set,dis_item_con1, &dis_item_con2,dis_item_txtname1,dis_item_txtset1,dis_item_line1,\
                1,IDC_DIS_SHARP,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_SYSTEM_DISPLAY_SET_BRIGHTNESS,\
                STRING_NUMBER,0,MAX_BRIGHTNESS_NUM,m_dis_set_range[IDC_DIS_BRIGHT])

LDEF_MENU_ITEM(g_win_display_set,dis_item_con2, &dis_item_con3,dis_item_txtname2,dis_item_txtset2,dis_item_line2,\
                2,1,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_SYSTEM_DISPLAY_SET_CONTRAST,\
                STRING_NUMBER,0,MAX_BRIGHTNESS_NUM,m_dis_set_range[IDC_DIS_CONTRAST])

LDEF_MENU_ITEM(g_win_display_set,dis_item_con3, &dis_item_con4,dis_item_txtname3,dis_item_txtset3,dis_item_line3,\
                3,2,4,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_SYSTEM_DISPLAY_SET_SATURATION,\
                STRING_NUMBER,0,MAX_BRIGHTNESS_NUM,m_dis_set_range[IDC_DIS_SATURATION])

LDEF_MENU_ITEM(g_win_display_set,dis_item_con4, &dis_item_con5,dis_item_txtname4,dis_item_txtset4,dis_item_line4,\
                4,3,5,  CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_SYSTEM_DISPLAY_SET_HUE,\
                STRING_NUMBER,0,MAX_BRIGHTNESS_NUM,m_dis_set_range[IDC_DIS_HUE])

LDEF_MENU_ITEM(g_win_display_set,dis_item_con5, NULL,dis_item_txtname5,dis_item_txtset5,dis_item_line5,\
                5,4,1,CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_SYSTEM_DISPLAY_SET_SHARPNESS,\
                STRING_NUMBER,0,11,m_dis_set_range[IDC_DIS_SHARP])

LDEF_WIN(g_win_display_set,&dis_item_con1,W_L, W_T, W_W, W_H, 1)
/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static CONTAINER *dis_items[] =
{
    &dis_item_con1,
    &dis_item_con2,
    &dis_item_con3,
    &dis_item_con4,
    &dis_item_con5,
};

static MULTISEL *dis_item_set[] =
{
    &dis_item_txtset1,
    &dis_item_txtset2,
    &dis_item_txtset3,
    &dis_item_txtset4,
    &dis_item_txtset5,
};

static UINT32 m_dis_set_range[][3] =
{
    {0, 0, 0},
    {MIN_BRIGHTNESS_VALUE,MAX_BRIGHTNESS_VALUE,1},
    {MIN_BRIGHTNESS_VALUE,MAX_BRIGHTNESS_VALUE,1},
    {MIN_BRIGHTNESS_VALUE,MAX_BRIGHTNESS_VALUE,1},
    {MIN_BRIGHTNESS_VALUE,MAX_BRIGHTNESS_VALUE,1},
    {MIN_BRIGHTNESS_VALUE,10,1},
 };

static BOOL init = FALSE;

static void win_dis_load_setting(void);
static void win_dis_init_items(void);
static void win_dis_enter_key(UINT8 id);
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION dis_item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
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
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        break;
    }

    return act;
}

static PRESULT dis_item_sel_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 bid= 0;
    UINT32 sel= 0;
    UINT16 *uni_str = NULL;
    SYSTEM_DATA *sys_data = NULL;

	if(NULL == sys_data)
	{
		;
	}
	if(NULL == uni_str)
	{
		;
	}
	bid = osd_get_obj_id(pobj);
    sys_data = sys_data_get();
    switch(event)
    {
    case EVN_PRE_CHANGE:
        sel = *((UINT32*)param1);
        break;
    case EVN_POST_CHANGE:
        sel = param1;
        switch(bid)
        {
        case IDC_DIS_BRIGHT:
            sys_data_set_brightness(sel);
            break;
        case IDC_DIS_CONTRAST:
            sys_data_set_contrast(sel);
            break;
        case IDC_DIS_SATURATION:
            sys_data_set_saturation(sel);
            break;
        case IDC_DIS_HUE:
            sys_data_set_hue(sel);
            break;
        case IDC_DIS_SHARP:
            sys_data_set_sharpness(sel);
            break;
        default:
            break;
        }

        break;
    case EVN_REQUEST_STRING:
        sel = param1;
        uni_str= (UINT16*)param2;
        switch(bid)
        {
        case IDC_DIS_BRIGHT:
            break;
        case IDC_DIS_CONTRAST:
            break;

//      case IDC_DIS_HUE:           break;
        case IDC_DIS_SHARP:
            break;
        default:
            break;
        }
        break;

    case EVN_UNKNOWN_ACTION:
        win_dis_enter_key(bid);

        break;
    default:
        break;
    }
    return ret;
}

static VACTION dis_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    return VACT_PASS;
}

static PRESULT dis_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    return PROC_PASS;
}

static VACTION dis_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

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

static PRESULT dis_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_display_set,RS_SYSTEM_DISPLAY_SET, 0);
        win_dis_load_setting();
        win_dis_init_items();
        break;
    case EVN_PRE_CLOSE:
        sys_data_save(1);
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    default:
        break;
    }

    return ret;
}


/*******************************************************************************
*   other functions  definition
*******************************************************************************/

static void win_dis_init_items(void)
{
    if(!init)
    {
        init = TRUE;
    }
}


static void win_dis_load_setting(void)
{
    MULTISEL *msel = NULL;
    SYSTEM_DATA *sys_data = NULL;
    UINT32 sel= 0;
   
    sys_data = sys_data_get();


    /* Brightness */
    msel = dis_item_set[IDC_DIS_BRIGHT - 1];
    sel = sys_data->avset.brightness;
    osd_set_multisel_sel(msel, sel);

    /* Contrast */
    msel = dis_item_set[IDC_DIS_CONTRAST - 1];
    sel = sys_data->avset.contrast;
    osd_set_multisel_sel(msel, sel);

    /* Saturation */
    msel = dis_item_set[IDC_DIS_SATURATION - 1];
    sel = sys_data->avset.saturation;
    osd_set_multisel_sel(msel, sel);

    /* Hue */
    msel = dis_item_set[IDC_DIS_HUE - 1];
    sel = sys_data->avset.hue;
    osd_set_multisel_sel(msel, sel);

    /* Sharpness */
    msel = dis_item_set[IDC_DIS_SHARP - 1];
    sel = sys_data->avset.sharpness;
    osd_set_multisel_sel(msel, sel);

#ifdef _S3281_
    CONTAINER *huecon = dis_items[IDC_DIS_HUE - 1];
    CONTAINER *sharpcon = dis_items[IDC_DIS_SHARP - 1];

    osd_set_attr(huecon,C_ATTR_HIDDEN);
    osd_set_attr(sharpcon,C_ATTR_HIDDEN);
#endif

}

UINT8 win_display_set_get_item_num(void)
{
    UINT8 id = osd_get_focus_id((POBJECT_HEAD)&g_win_display_set);

    if (IDC_DIS_SHARP == id)
    {
        return 11;
    }
    return MAX_BRIGHTNESS_NUM;
}

PRESULT comlist_display_set_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i= 0;
    UINT16 wtop= 0;
    UINT8 id= 0;
    OBJLIST *ol = NULL;
    UINT8 temp[4]={0};

    PRESULT cbret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    if(EVN_PRE_DRAW == event)
    {
        id = osd_get_focus_id((POBJECT_HEAD)&g_win_display_set);
        switch(id)
        {
        case IDC_DIS_BRIGHT:
        case IDC_DIS_CONTRAST:
        case IDC_DIS_SATURATION:
        case IDC_DIS_HUE:
        case IDC_DIS_SHARP:
            wtop = osd_get_obj_list_top(ol);
            for(i=0;((i<ol->w_dep) && ((i+wtop)<ol->w_count));i++)
            {
                //sprintf(temp,"%d", i+wtop+m_dis_set_range[id][0]);
                snprintf((char *)temp, 4, "%lu", i+wtop+m_dis_set_range[id][0]);
                win_comlist_set_str(i + wtop,(char *)temp,NULL,0);
            }
            break;
        default:
            break;
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }
    return cbret;
}


static void win_dis_enter_key(UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT16 sel = 0;
    POBJECT_HEAD pobj = NULL;
    POBJECT_HEAD pobjitem= NULL;
    MULTISEL *msel = NULL;

    MEMSET(&rect, 0 ,sizeof(rect));
    MEMSET(&param, 0 ,sizeof(param));
    msel = dis_item_set[id - 1];
    pobj = (POBJECT_HEAD)msel;
    pobjitem = (POBJECT_HEAD)dis_items[id - 1];

    rect.u_left  = CON_L+SEL_L_OF;
    rect.u_width = SEL_W;
    rect.u_top = pobj->frame.u_top;

    param.selecttype = POP_LIST_SINGLESELECT;

    switch(id)
    {
    case IDC_DIS_BRIGHT:
        sel = p_sys_data->avset.brightness;
        break;
    case IDC_DIS_CONTRAST:
        sel = p_sys_data->avset.contrast;
        break;
    case IDC_DIS_SATURATION:
        sel = p_sys_data->avset.saturation;
        break;
    case IDC_DIS_HUE:
        sel = p_sys_data->avset.hue;
        break;
    case IDC_DIS_SHARP:
        sel = p_sys_data->avset.sharpness;
        break;
    default:
        break;
    }
#ifndef SD_UI
    rect.u_height = 340;
#else
    rect.u_height = 200;
#endif
    param.cur = sel - m_dis_set_range[id][0];
    sel = win_com_open_sub_list(POP_LIST_MENU_DISPLAY_SET,&rect,&param) + m_dis_set_range[id][0];

    osd_set_multisel_sel(msel, sel);

    PRESULT ret = 0;

    ret = osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);

    if((ret != PROC_PASS) && (ret != PROC_LOOP) \
        && (ret != PROC_LEAVE) )
    {
        return;
    }

    osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

}

#endif

