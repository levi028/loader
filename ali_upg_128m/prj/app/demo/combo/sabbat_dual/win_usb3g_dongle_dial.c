/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_usb3g_dongle_dial.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef USB3G_DONGLE_SUPPORT  /* --Doy.Dong, 2013-1-14*/
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
#include <api/libusb3g/lib_usb3g.h>

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

#include "mobile_input.h"

#define DEB_PRINT    libc_printf

/*******************************************************************************
*    Objects definition
*******************************************************************************/

CONTAINER g_win_dial_up;

CONTAINER dial_item1;    /* apn */
CONTAINER dial_item2;    /* dial number */
CONTAINER dial_item3;    /* user name */
CONTAINER dial_item4;    /* password */


TEXT_FIELD dial_title;
TEXT_FIELD dial_txt1;
TEXT_FIELD dial_txt2;
TEXT_FIELD dial_txt3;
TEXT_FIELD dial_txt4;

TEXT_FIELD dial_btntxt_dial;
TEXT_FIELD dial_btntxt_exit;
TEXT_FIELD dial_btntxt_caps;
TEXT_FIELD dial_btntxt_del;

BITMAP   dial_btnbmp_caps;
BITMAP   dial_btnbmp_del;

EDIT_FIELD dial_edt1;    /* apn */
EDIT_FIELD dial_edt2;    /* dial number */
EDIT_FIELD dial_edt3;    /* user name */
EDIT_FIELD dial_edt4;    /* password */

static char name_pat[];
//static char orbit_pat[] =  "f31";

//static UINT16 orbit_sub[];

static UINT16 btn_bmp_ids_truecolor[][4];
static UINT8 btn_txt_widths[];
static char *btn_txt_strs[] =
{
    "CAPS", "DEL", "OK", "CANCEL"
};
static struct usb3gdongle *dialdongle = NULL;
static int disable_press = 0;

static VACTION dial_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT dial_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION dial_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT dial_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION dial_btn_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT dial_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION dial_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT dial_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static void set_capsdel_okcancel_btncontent(BOOL param);
extern void win_dongle_popup_display(char *string, UINT32 show_ticks);
extern void sys_data_set_def_ispinfo(struct isp_info *ispinfo);


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

#define EDT_SH_IDX   WSTL_TEXT_04_HD
#define EDT_HL_IDX   WSTL_BUTTON_08_HD//WSTL_TEXT_04
#define EDT_NAME_HL_IDX   WSTL_BUTTON_08_HD//WSTL_BUTTON_09
#define EDT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_TEXT_04_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD

#define POPUPSH_IDX    WSTL_BUTTON_POP_SH_HD
#define POPUPHL_IDX    WSTL_BUTTON_POP_HL_HD

#define    W_L         340//430
#define    W_T         150//200
#define    W_W         500
#define    W_H         360

#define TITLE_L     (W_L + 40)
#define TITLE_T         (W_T + 20)
#define TITLE_W     (W_W - 80)
#define TITLE_H     40

#define CON_L        TITLE_L
#define CON_T        (TITLE_T +  TITLE_H)
#define CON_W        (W_W - 80)
#define CON_H        40
#define CON_GAP        4

#define TXT_L_OF      10
#define TXT_W          160
#define TXT_H        40
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          260
#define SEL_H        40 //28
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/3)
#define BTN_T_OF    (CON_H+CON_GAP)*4+14
#define BTN_W        160
#define BTN_H        38
#define BTN_GAP        40

#define COLBTN_L    (W_L + 60)
#define COLBTN_T    (W_T + W_H - 60)
#define COLBTN_GAP    4

#define CAPSFLG_L    (COLBTN_L+170+28*2)
#define CAPSFLG_T    (COLBTN_T)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
                  dial_item_keymap,dial_item_callback,  \
                  conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TITLE(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, WSTL_POP_TXT_SH_HD,WSTL_POP_TXT_SH_HD,WSTL_POP_TXT_SH_HD,WSTL_POP_TXT_SH_HD,   \
                  NULL,NULL,  \
                  C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_TXT_BTN(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
                  ID,idl,idr,idu,idd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
                  dial_btn_keymap,dial_btn_callback,  \
                  C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)
#define LDEF_BMP_BTN(root,var_bmp,nxt_obj,ID,idl,idr,idu,idd,l,t,icon_id) \
    DEF_BITMAP(var_bmp, root, nxt_obj, C_ATTR_ACTIVE,0,     \
               ID,idl,idr,idu,idd, l, t, 28, 40, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT,    \
               NULL, NULL,    \
               C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, icon_id)
#define LDEF_TXT_BTN_EXT(root,var_txt,nxt_obj,l,t,w,h,str)    \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)



#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h, hl,align,style,cursormode,pat,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,hl,EDT_SL_IDX,EDT_GRY_IDX,   \
                  dial_item_edt_keymap,dial_item_edt_callback,  \
                  align, 8,0,style,pat,17,cursormode,NULL,sub,str)

#define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,\
                           l,t,w,h,edthl,res_id,align,style,cur_mode,pat,sub,str) \
LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,CON_HL_IDX,&var_txt,ID)   \
LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
LDEF_EDIT(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,edthl,align,style,cur_mode,pat,sub,str)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
                  dial_keymap,dial_callback,  \
                  nxt_obj, focus_id,0)


LDEF_TITLE(g_win_dial_up, dial_title, &dial_item1, \
           TITLE_L, TITLE_T, TITLE_W, TITLE_H, 0)


LDEF_MENU_ITEM_EDT(g_win_dial_up, dial_item1, &dial_item2, dial_txt1, dial_edt1, 1, 4, 2, \
                   CON_L, CON_T + (CON_H + CON_GAP) * 0, CON_W, CON_H, EDT_NAME_HL_IDX, RS_USB_3G_APN,  C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, name_pat, NULL, display_strs[0])
LDEF_MENU_ITEM_EDT(g_win_dial_up, dial_item2, &dial_item3, dial_txt2, dial_edt2, 2, 1, 3, \
                   CON_L, CON_T + (CON_H + CON_GAP) * 1, CON_W, CON_H, EDT_NAME_HL_IDX, RS_USB_3G_DIAL_NUMBER,  C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, name_pat, NULL, display_strs[1])
LDEF_MENU_ITEM_EDT(g_win_dial_up, dial_item3, &dial_item4, dial_txt3, dial_edt3, 3, 2, 4, \
                   CON_L, CON_T + (CON_H + CON_GAP) * 2, CON_W, CON_H, EDT_NAME_HL_IDX, RS_USB_3G_USER_NAME,  C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, name_pat, NULL, display_strs[2])
LDEF_MENU_ITEM_EDT(g_win_dial_up, dial_item4, &dial_btntxt_dial, dial_txt4, dial_edt4, 4, 3, 5, \
                   CON_L, CON_T + (CON_H + CON_GAP) * 3, CON_W, CON_H, EDT_NAME_HL_IDX, RS_USB_3G_PASSWORD,  C_ALIGN_LEFT | C_ALIGN_VCENTER, NORMAL_EDIT_MODE, CURSOR_NORMAL, name_pat, NULL, display_strs[3])

LDEF_TXT_BTN(g_win_dial_up, dial_btntxt_dial, &dial_btntxt_exit, 5, 1, 6, 4, 6,    \
             CON_L + BTN_L_OF, CON_T + BTN_T_OF, BTN_W, BTN_H, RS_COMMON_CONNECT)

LDEF_TXT_BTN(g_win_dial_up, dial_btntxt_exit, &dial_btnbmp_caps , 6, 5, 1, 5, 1,    \
             CON_L + BTN_L_OF + BTN_W + BTN_GAP, CON_T + BTN_T_OF, BTN_W - 40, BTN_H, RS_COMMON_CANCEL)

LDEF_BMP_BTN(&g_win_dial_up, dial_btnbmp_caps, &dial_btntxt_caps, 0, 0, 0, 0, 0, \
             COLBTN_L, COLBTN_T, IM_EPG_COLORBUTTON_RED)


LDEF_TXT_BTN_EXT(&g_win_dial_up, dial_btntxt_caps, &dial_btnbmp_del, \
                 COLBTN_L + 28 * 1, COLBTN_T, 100, 40, display_strs[4])

LDEF_BMP_BTN(&g_win_dial_up, dial_btnbmp_del, &dial_btntxt_del, 0, 0, 0, 0, 0, \
             COLBTN_L + 100 + 28 * 1, COLBTN_T, IM_EPG_COLORBUTTON_GREEN)



LDEF_WIN(g_win_dial_up, &dial_title, W_L, W_T, W_W, W_H, 1)


#define SAT_NAME_ID    1
#define SAT_ORBIT_ID    2
#define BTN_OK_ID        3
#define BTN_CANCEL_ID    4

/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

//char name_pat[]  = "s17";

UINT16 sat_name_str[50];
UINT16 orbit_sub[10];

UINT8  edit_sat_mode;
UINT16 edit_sat_id;

UINT8  edit_sat_ok; /* 0 - cancel : 1 save*/
UINT8  sat_orbit_direct; /* 0 - 'E' , 1 - 'W' */

extern char *longitute_sub_chars[2];

extern void draw_caps_flag(UINT16 x, UINT16 y, UINT32 mode); /*  mode : 0 -hide, 1 -draw*/
extern void draw_caps_del_colbtn(UINT16 x, UINT16 y, UINT32 mode);
extern void draw_ok_cancel_colbtn(UINT16 x, UINT16 y, UINT32 mode);


/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

#define VACT_CHANGE_SUB    (VACT_PASS + 1)
#define VACT_OK                (VACT_PASS + 2)
#define VACT_CANCEL        (VACT_PASS + 3)


static VACTION dial_item_edt_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    //UINT8 b_id;
    UINT8 b_style;
    EDIT_FIELD    *edf;
    edf = (EDIT_FIELD *)p_obj;
    //b_id = osd_get_obj_id(p_obj);
    b_style = edf->b_style;

    switch (key)
    {
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
            act = key - V_KEY_0 + VACT_NUM_0;
            break;
        case V_KEY_LEFT:
        case V_KEY_RIGHT:
            act = (key == V_KEY_LEFT) ? VACT_EDIT_LEFT : VACT_EDIT_RIGHT;
            break;
        case V_KEY_ENTER:
            act = VACT_EDIT_ENTER;
            break;
        case V_KEY_EXIT:

            if (b_style & EDIT_STATUS)
            {
                act = VACT_EDIT_SAVE_EXIT;
            }
            else
            {
                act = VACT_PASS;
            }

            break;
        case V_KEY_RED:
            act = VACT_CAPS;
            break;
        case V_KEY_GREEN:
            act = VACT_DEL;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static VACTION dial_btn_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch (key)
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
        case V_KEY_ENTER:
            act = VACT_OK;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT dial_btn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8    b_id;
    VACTION unact;
    unact = (VACTION)(param1 >> 16);
    b_id = osd_get_obj_id(p_obj);
    //libc_printf("unact: %x\n", unact);


    if (unact == VACT_CLOSE)
    {
        ret = PROC_LEAVE;
    }

    if (unact == VACT_OK && !disable_press)
    {
        if (b_id == 5) //connect button
        {
            if (dialdongle->state == dongle_state_idle)
            {
                com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt1),
                               dialdongle->config.isp.apn);
                com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt2),
                               dialdongle->config.isp.phone_num);
                com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt3),
                               dialdongle->config.isp.user);
                com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt4),
                               dialdongle->config.isp.passwd);

                if (STRLEN(dialdongle->config.isp.apn) == 0
                        || STRLEN(dialdongle->config.isp.phone_num) == 0)
                {
                    win_dongle_popup_display("Invalid dial setting!!\n", 1000);
                }
                else
                {
                    disable_press = 1;
                    sys_data_set_def_ispinfo(&dialdongle->config.isp);
                    sys_data_save(1);
                    usb3g_start_dail(dialdongle);
                }
            }
            else
            {
                disable_press = 1;
                sys_data_set_def_ispinfo(&dialdongle->config.isp);
                sys_data_save(1);
                usb3g_stop_dail(dialdongle);
            }

            ret = PROC_LEAVE;
        }

        if (b_id == 6)
        {
            ret = PROC_LEAVE;
        }
    }

    return ret;
}

static PRESULT dial_item_edt_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT8 b_id;
    //UINT8 unact;
    UINT8 caps;
    mobile_input_type_t mobile_input_type;
    caps = mobile_input_get_caps();
    mobile_input_type.type         = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = caps ? MOBILE_CAPS_INIT_UP : MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback    = NULL;
    //b_id = osd_get_obj_id(p_obj);

    switch (event)
    {
        case EVN_KEY_GOT:
            ret = mobile_input_proc((EDIT_FIELD *)p_obj, (VACTION)(param1 >> 16), param1 & 0xFFFF, param2);

            if (ret == PROC_LOOP)
            {
                draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 1);
            }

            break;
        case EVN_PRE_CHANGE:
            break;
        case EVN_POST_CHANGE:
            break;
        case EVN_UNKNOWN_ACTION:
            //unact = (VACTION)(param1 >> 16);
            break;
        case EVN_FOCUS_PRE_GET:
        case EVN_PARENT_FOCUS_PRE_GET:
            mobile_input_init((EDIT_FIELD *)p_obj, &mobile_input_type);
            draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 1);
            draw_caps_del_colbtn(COLBTN_L, COLBTN_T, 1);
            break;
        case EVN_FOCUS_PRE_LOSE:
        case EVN_PARENT_FOCUS_PRE_LOSE:
            mobile_input_init((EDIT_FIELD *)p_obj, &mobile_input_type);
            draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 0);
            draw_caps_del_colbtn(COLBTN_L, COLBTN_T, 0);
            break;
    }

    return ret;
}

static VACTION dial_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    return act;
}

static PRESULT dial_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    return ret;
}

static VACTION dial_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch (key)
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
        case V_KEY_YELLOW:
            act = VACT_OK;
            break;
        case V_KEY_BLUE:
            act = VACT_CANCEL;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT dial_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //VACTION unact;
    mobile_input_type_t mobile_input_type;
    mobile_input_type.type         = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback    = NULL;

    switch (event)
    {
        case EVN_PRE_OPEN:
            mobile_input_init(&dial_edt1, &mobile_input_type);
            mobile_input_init(&dial_edt2, &mobile_input_type);
            mobile_input_init(&dial_edt3, &mobile_input_type);
            mobile_input_init(&dial_edt4, &mobile_input_type);
            break;
        case EVN_POST_OPEN:
            draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 1);
            //    draw_caps_del_colbtn(COLBTN_L, COLBTN_T,1);
            //    draw_ok_cancel_colbtn(COLBTN_L + 220 , COLBTN_T,1);
            break;
        case EVN_PRE_CLOSE:
            /* Make OSD not flickering */
            *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
            com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt1), dialdongle->config.isp.apn);
            com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt2), dialdongle->config.isp.phone_num);
            com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt3), dialdongle->config.isp.user);
            com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&dial_edt4), dialdongle->config.isp.passwd);
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_UNKNOWN_ACTION:
            //unact = (VACTION)(param1 >> 16);
            break;
    }

    return ret;
}


UINT32 win_dial_open(struct usb3gdongle *dongle)
{
    CONTAINER    *win;
    PRESULT ret = PROC_LOOP;
    UINT32 hkey;
    BOOL old_value = ap_enable_key_task_get_key(TRUE);
    osd_set_text_field_content(&dial_title, STRING_ID, RS_USB_3G_DIAL_SETTING);

    if (dongle == NULL)
    {
        return 0;
    }

    dialdongle = dongle;

    if (dongle->state == dongle_state_idle)
    {
        osd_set_text_field_content(&dial_btntxt_dial, STRING_ID, RS_COMMON_CONNECT);
    }
    else
    {
        osd_set_text_field_content(&dial_btntxt_dial, STRING_ID, RS_COMMON_DISCONNECT);
    }

    ///
    osd_set_edit_field_content(&dial_edt1, STRING_ANSI, (UINT32)dongle->config.isp.apn);
    osd_set_edit_field_content(&dial_edt2, STRING_ANSI, (UINT32)dongle->config.isp.phone_num);
    osd_set_edit_field_content(&dial_edt3, STRING_ANSI, (UINT32)dongle->config.isp.user);
    osd_set_edit_field_content(&dial_edt4, STRING_ANSI, (UINT32)dongle->config.isp.passwd);
    //
    osd_set_text_field_content(&dial_btntxt_caps, STRING_ANSI, (UINT32)btn_txt_strs[0]);
    osd_set_text_field_content(&dial_btntxt_del, STRING_ANSI, (UINT32)btn_txt_strs[1]);
    osd_set_text_field_content(&dial_btntxt_dial, STRING_ANSI, (UINT32)btn_txt_strs[2]);
    osd_set_text_field_content(&dial_btntxt_exit, STRING_ANSI, (UINT32)btn_txt_strs[3]);
    win = &g_win_dial_up;

    if (strlen(dongle->config.isp.apn) == 0)
    {
        osd_set_container_focus(win, 1);
    }
    else
    {
        osd_set_container_focus(win, 5);
    }

    disable_press = 0;
    wincom_backup_region(&(win->head.frame));
    osd_obj_open((POBJECT_HEAD)win, 0);

    while (ret != PROC_LEAVE)
    {
        hkey = ap_get_key_msg();

        if (hkey == INVALID_HK || hkey == INVALID_MSG)
        {
            continue;
        }
        ret = osd_obj_proc((POBJECT_HEAD)win, (MSG_TYPE_KEY << 16), hkey, 0);
    }

    wincom_restore_region();
    ap_enable_key_task_get_key(old_value);
    return 0;
}

#endif /****USB3G_DONGLE_SUPPORT****/
