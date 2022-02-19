/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_usb3g_dongle.c
*
*    Description: The function realize USB 3g
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB3G_DONGLE_SUPPORT

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <api/libpub/lib_frontend.h>
#include <api/libusb3g/lib_usb3g.h>
#include <api/libtcpip/lwip/netdb.h>
#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_net_choose.h"
#include <api/libnic/lib_nic.h>

CONTAINER g_win_usb3g_dongle;

OBJLIST     usb3g_dongle_olist;

CONTAINER usb3g_dongle0;
CONTAINER usb3g_dongle1;
CONTAINER usb3g_dongle2;
CONTAINER usb3g_dongle3;
CONTAINER usb3g_dongle4;
CONTAINER usb3g_dongle5;

TEXT_FIELD usb3g_dongle_idx0;
TEXT_FIELD usb3g_dongle_idx1;
TEXT_FIELD usb3g_dongle_idx2;
TEXT_FIELD usb3g_dongle_idx3;
TEXT_FIELD usb3g_dongle_idx4;
TEXT_FIELD usb3g_dongle_idx5;

TEXT_FIELD usb3g_dongle_name0;
TEXT_FIELD usb3g_dongle_name1;
TEXT_FIELD usb3g_dongle_name2;
TEXT_FIELD usb3g_dongle_name3;
TEXT_FIELD usb3g_dongle_name4;
TEXT_FIELD usb3g_dongle_name5;

TEXT_FIELD usb3g_dongle_line0;
TEXT_FIELD usb3g_dongle_line1;
TEXT_FIELD usb3g_dongle_line2;
TEXT_FIELD usb3g_dongle_line3;
TEXT_FIELD usb3g_dongle_line4;
TEXT_FIELD usb3g_dongle_line5;

TEXT_FIELD     dongle_line;

SCROLL_BAR    usb3g_dongle_list_scb;

TEXT_FIELD     dongle_info_txt;

#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX   WSTL_WIN_BODYRIGHT_01_HD

#define LST_SH_IDX        WSTL_NOSHOW_IDX
#define LST_HL_IDX        WSTL_NOSHOW_IDX
#define LST_SL_IDX        WSTL_NOSHOW_IDX
#define LST_GRY_IDX        WSTL_NOSHOW_IDX

//#define LST_SATIP_DEV_SH_IDX     WSTL_TEXT_11_HD

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXTN_GRY_IDX  WSTL_BUTTON_01_FG_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_01_HD

#define TXTI_SH_IDX   WSTL_TEXT_09_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_SL_IDX   WSTL_TEXT_09_HD
#define TXTI_GRY_IDX  WSTL_TEXT_09_HD

#define TXTC_SH_IDX   WSTL_TEXT_09_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTC_SL_IDX   WSTL_TEXT_09_HD
#define TXTC_GRY_IDX  WSTL_TEXT_09_HD

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT

#define PROGRESSBAR_SH_IDX            WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX        WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX    WSTL_BAR_04_HD
#define PROGRESS_TXT_SH_IDX    WSTL_BUTTON_01_HD

#define MSG_SH_IDX WSTL_TEXT_09_HD

#define    W_L         248//384
#define    W_T         98//138
#define    W_W         692
#define    W_H         488


#define CON_L        (W_L + 10)
#define CON_H        40
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 70 - (SCB_W + 4))
#define CON_GAP     12

#define LIST_ITEMCNT    6//10


#define SCB_L         (W_L + 20)
#define SCB_T         (W_T + 12)//150
#define SCB_W         12//18
#define SCB_H         LST_H-LST_GAP//390

#define ITEM_GAP    6//8
#define ITEM_H        CON_H

#define LST_L        (CON_L + 10)
#define LST_T        CON_T
#define LST_W        (W_W - 40)
#define LST_GAP     4
#define LST_H        ((ITEM_H+ITEM_GAP)*LIST_ITEMCNT+LST_GAP)

#define ITEM_L        (LST_L + 0)
#define ITEM_T        (LST_T + 2)
#define ITEM_W        (LST_W - 0)

#define ITEM_IDX_L  20
#define ITEM_IDX_W  40
#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  320//280//180

#define LINE_L_OF    0
#define LINE_T_OF      (ITEM_H+2)
#define LINE_W        ITEM_W
#define LINE_H         4


#define BAR_L    (CON_L+5)
#define BAR_T    (CON_T + (ITEM_H + ITEM_GAP)*LIST_ITEMCNT+10)
#define BAR_W    (CON_W-BAR_TXT_W-10)
#define BAR_H    24

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W    100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H    36//24

#define DONGLE_LINE_L    (LST_L)
#define DONGLE_LINE_T      (LST_T + LST_H + 14)
#define DONGLE_LINE_W    (LST_W-20)
#define DONGLE_LINE_H   2

#define MSG_L    (LST_L+20)
#define MSG_T    (DONGLE_LINE_T + DONGLE_LINE_H + 10)//(BAR_T)//(BAR_T + BAR_H+20)
#define MSG_W    (LST_W-20)
#define MSG_H    36
#define MSG_GAP    4

extern UINT32 sys_data_get_net_choose_flag(void);
extern UINT32 win_dial_open(struct usb3gdongle *dongle);
extern void sys_data_get_def_ispinfo(struct isp_info *ispinfo);
extern void ap_send_msg(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer);
static VACTION usb3g_dongle_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT win_usb3g_dongle_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION usb3g_dongle_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usb3g_dongle_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION usb3g_dongle_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usb3g_dongle_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
                  usb3g_dongle_list_item_con_keymap,usb3g_dongle_list_item_con_callback,  \
                  conobj, 1,1)

#define LDEF_TXT_IDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXT_OTHER(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
                  ID,idl,idr,idu,idd, l,t,w,h, TXTC_SH_IDX,TXTC_HL_IDX,TXTC_SL_IDX,TXTC_GRY_IDX,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,var_idx,var_ssid,var_line,ID,l,t,w,h,idx_str,ssid_str)    \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_idx,1)    \
    LDEF_TXT_IDX(&var_con,var_idx,&var_ssid ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idx_str)    \
    LDEF_TXT_OTHER(&var_con,var_ssid, NULL/*&varLine*/,0,0,0,0,0,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,ssid_str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,ITEM_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
                  0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
                  NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
                  0, 20, w, h - 40, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
    DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
                   1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
                   usb3g_dongle_list_keymap,usb3g_dongle_list_callback,    \
                   flds,sb,mark,style,dep,count,selary)



#define LDEF_TXT_MSG(root,var_txt)        \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0,  0,0,0,0, MSG_SH_IDX,0,0,0,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[21])

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
                  usb3g_dongle_keymap,win_usb3g_dongle_callback,  \
                  nxt_obj, focus_id,0)

#define LDEF_DONGLE_LINE(root, var_txt, nxt_obj, l, t, w, h, str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
                  0, 0, 0, 0, 0, l, t, w, h, WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,WSTL_LINE_MENU_HD,   \
                  NULL,NULL,  \
                  C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

LDEF_LIST_ITEM(usb3g_dongle_olist, usb3g_dongle0, usb3g_dongle_idx0, usb3g_dongle_name0, usb3g_dongle_line0, 1, \
               ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP) * 0, ITEM_W, ITEM_H, display_strs[0], display_strs[10])
LDEF_LIST_ITEM(usb3g_dongle_olist, usb3g_dongle1, usb3g_dongle_idx1, usb3g_dongle_name1, usb3g_dongle_line1, 2, \
               ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP) * 1, ITEM_W, ITEM_H, display_strs[1], display_strs[11])
LDEF_LIST_ITEM(usb3g_dongle_olist, usb3g_dongle2, usb3g_dongle_idx2, usb3g_dongle_name2, usb3g_dongle_line2, 3, \
               ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP) * 2, ITEM_W, ITEM_H, display_strs[2], display_strs[12])
LDEF_LIST_ITEM(usb3g_dongle_olist, usb3g_dongle3, usb3g_dongle_idx3, usb3g_dongle_name3, usb3g_dongle_line3, 4, \
               ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP) * 3, ITEM_W, ITEM_H, display_strs[3], display_strs[13])
LDEF_LIST_ITEM(usb3g_dongle_olist, usb3g_dongle4, usb3g_dongle_idx4, usb3g_dongle_name4, usb3g_dongle_line4, 5, \
               ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP) * 4, ITEM_W, ITEM_H, display_strs[4], display_strs[14])
LDEF_LIST_ITEM(usb3g_dongle_olist, usb3g_dongle5, usb3g_dongle_idx5, usb3g_dongle_name5, usb3g_dongle_line5, 6, \
               ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP) * 5, ITEM_W, ITEM_H, display_strs[5], display_strs[15])


//LDEF_LISTBAR(usb3g_dongle_olist, usb3g_dongle_list_scb, LIST_ITEMCNT, SCB_L, SCB_T, SCB_W, SCB_H)



POBJECT_HEAD usb3g_dongle_list_items[] =
{
    (POBJECT_HEAD) &usb3g_dongle0,
    (POBJECT_HEAD) &usb3g_dongle1,
    (POBJECT_HEAD) &usb3g_dongle2,
    (POBJECT_HEAD) &usb3g_dongle3,
    (POBJECT_HEAD) &usb3g_dongle4,
    (POBJECT_HEAD) &usb3g_dongle5,
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  \
                    LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_usb3g_dongle, usb3g_dongle_olist, &dongle_line, LST_L, LST_T, LST_W, LST_H, LIST_STYLE, \
        LIST_ITEMCNT, 0, usb3g_dongle_list_items, &usb3g_dongle_list_scb, NULL, NULL)

LDEF_TXT_MSG(g_win_usb3g_dongle, dongle_info_txt)

LDEF_WIN(g_win_usb3g_dongle, &usb3g_dongle_olist, W_L, W_T, W_W, W_H, 1)

LDEF_DONGLE_LINE(g_win_usb3g_dongle, dongle_line, NULL, DONGLE_LINE_L, DONGLE_LINE_T, DONGLE_LINE_W, DONGLE_LINE_H, NULL)

#define USB3G_DONGLE_MAX_NUM        LIST_ITEMCNT
#define USB3G_DONGLE_INFO_MAX_LEN    32

static UINT8    usb3g_dongle_num = 0;
static UINT16    usb3g_dongle_list_pos = 0;
static UINT16    usb3g_dongle_list_top = 0;
static UINT16    usb3g_dongle_list_item_sel = 0;

static UINT8 usb3g_dongle_list[USB3G_DONGLE_MAX_NUM][USB3G_DONGLE_INFO_MAX_LEN];

#define DONGLE_INFO_MAX_LINE 4
static char dongle_info_buf[DONGLE_INFO_MAX_LINE + 1][64];
//static UINT32 dongle_info_line;

#define VACT_CAPTURE VACT_PASS+1
static UINT8 g_usb3g_pcap = 0;

static char *_strcat (char *dst, const char *src)
{
    char *cp = dst;

    while (*cp)
    {
        cp++;    /* find end of dst */
    }

    while (*src)
    {
        *cp++ = *src++;/* Copy src to end of dst */
    }

    return(dst);/* return dst */
}
void win_dongle_popup_display(char *string, UINT32 show_ticks)
{
    UINT8 back_saved;
    win_msg_popup_close();
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg(string, NULL, 0);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(show_ticks);
    win_compopup_smsg_restoreback();
}

void usb3g_dongle_event_callback(enum USB3G_DONGLE_EVENT event, struct usb3gdongle *dongle)
{
    switch (event)
    {
        case USB3G_DONGLE_HOTPLUG_IN:
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_IN, (UINT32)dongle, FALSE);
            break;
        case USB3G_DONGLE_HOTPLUG_OUT:
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_OUT, (UINT32)dongle, FALSE);
            break;
        case USB3G_DONGLE_STATE_CHANGED:
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_3GDONGLE_STATE_CHANGED, (UINT32)dongle, FALSE);
            break;
        case USB3G_DONGLE_DO_CONNECT:
            break;
        default:
            break;
    }
}

static void win_dongle_update_list()
{
    int i = 0;
    struct usb3gdongle *dongle = NULL;
    usb3g_dongle_num = usb3g_dongle_count();

    for (i = 0; i < USB3G_DONGLE_MAX_NUM; i++)
    {
        MEMSET(usb3g_dongle_list[i], 0, USB3G_DONGLE_INFO_MAX_LEN);
        dongle = usb3g_get_dongle_by_index(i);

        if (dongle)
        {
            snprintf((char*)usb3g_dongle_list[i], USB3G_DONGLE_INFO_MAX_LEN,"%s %s", dongle->vendor_info,
                    dongle->product_info);
        }
    }
}

static void win_dongle_list_update_display(BOOL refresh)
{
    OBJLIST *ol;
    UINT16 cnt, page, top, curitem;
    UINT16 i, index;
    UINT32 valid_idx;
    CONTAINER *item;
    TEXT_FIELD *txt;
    ol = &usb3g_dongle_olist;
    // Fake server list
    //    OSD_SetObjListCount(ol, 4);    /* Set count */
    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);
    usb3g_dongle_list_item_sel = curitem;
    usb3g_dongle_list_top = top;
    usb3g_dongle_list_pos = usb3g_dongle_list_item_sel - usb3g_dongle_list_top;

    for (i = 0; i < page; i++)
    {
        item = (CONTAINER *)usb3g_dongle_list_items[i];
        index = top + i;

        if (index < cnt)
        {
            valid_idx = 1;
        }
        else
        {
            valid_idx = 0;
        }

        /*dongle index*/
        txt = (PTEXT_FIELD)osd_get_container_next_obj(item);

        if (valid_idx)
        {
            osd_set_text_field_content(txt, STRING_NUMBER, (UINT32) (index + 1));
        }
        else
        {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
        }

        /* dongle name*/
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);

        if (valid_idx)
        {
            if (STRLEN((char*)usb3g_dongle_list[index]))
            {
                osd_set_text_field_content(txt, STRING_ANSI, (UINT32)usb3g_dongle_list[index]);
            }
            else
            {
                osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
            }
        }
        else
        {
            osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
        }
    }

    if (refresh)
    {
        osd_track_object((POBJECT_HEAD)&usb3g_dongle_olist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}

static void win_dongle_info_txt_update()
{
    UINT32 i;
    OSD_RECT rect;
    struct usb3gdongle *dongle = NULL;
    dongle = usb3g_get_dongle_by_index(usb3g_dongle_list_item_sel);
    MEMSET(dongle_info_buf, 0 , 64 * DONGLE_INFO_MAX_LINE);

    if (dongle)
    {
        snprintf(dongle_info_buf[0], 64, "Model: %s %s",
                dongle->vendor_info, dongle->product_info);
        snprintf(dongle_info_buf[1], 64,"IMSI:   %s", dongle->imsi);
        snprintf(dongle_info_buf[2], 64,"APN:   %s  Dail number: %s",
                dongle->config.isp.apn, dongle->config.isp.phone_num);

        if (dongle->simready)
        {
            switch (dongle->state)
            {
                case dongle_state_idle:
                    strncpy(dongle_info_buf[3], "State:  ready", (64-1));
                    break;
                case dongle_state_disconnectting:
                    strncpy(dongle_info_buf[3], "State:  disconnectting...", (64-1));
                    break;
                case dongle_state_connectting:
                    strncpy(dongle_info_buf[3], "State:  do connecting...", (64-1));
                    break;
                case dongle_state_connect_err:
                    strncpy(dongle_info_buf[3], "State:  connect faild, retry later", (64-1));
                    break;
                case dongle_state_connected:
                    strncpy(dongle_info_buf[3], "State:  connected!!", (64-1));
                    break;
            }
        }
        else
        {
            strncpy(dongle_info_buf[3], "** sim card error **", (64-1));
        }
    }

    //draw msg infos
    for (i = 0; i < DONGLE_INFO_MAX_LINE; i++)
    {
        rect.u_left     = MSG_L;
        rect.u_top      = MSG_T + (MSG_H + MSG_GAP) * i;
        rect.u_width = MSG_W;
        rect.u_height = MSG_H;
        osd_set_rect2(&dongle_info_txt.head.frame, &rect);
        osd_set_text_field_content(&dongle_info_txt, STRING_ANSI, (UINT32)dongle_info_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&dongle_info_txt, C_UPDATE_ALL);
    }
}

static void win_update_all_dongle_menu(BOOL refresh)
{
    OBJLIST *ol;
    //CONTAINER *item;
    SCROLL_BAR *sb;
    ol = &usb3g_dongle_olist;
    sb = &usb3g_dongle_list_scb;
    win_dongle_update_list();

    if ((usb3g_dongle_list_pos + usb3g_dongle_list_top) >= usb3g_dongle_num)
    {
        usb3g_dongle_list_top = 0;
        usb3g_dongle_list_item_sel = 0;
        usb3g_dongle_list_pos = 0;
    }

    if (usb3g_dongle_list_item_sel >= usb3g_dongle_num)
    {
        usb3g_dongle_list_item_sel = 0;
    }

    osd_set_obj_list_count(ol, usb3g_dongle_num);    /* Set count */
    osd_set_obj_list_cur_point(ol, usb3g_dongle_list_item_sel);
    osd_set_obj_list_new_point(ol, usb3g_dongle_list_item_sel);
    osd_set_obj_list_top(ol, usb3g_dongle_list_top);
    osd_set_scroll_bar_max(sb, usb3g_dongle_num);
    osd_set_scroll_bar_pos(sb, usb3g_dongle_list_top);
    win_dongle_list_update_display(refresh);
    win_dongle_info_txt_update();
}

static PRESULT win_usb3g_dongle_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    //UINT8 i;
    //UINT8 serv_name[24];
    //POBJECT_HEAD usb3g_dongle_satlst_menu;
    //INT32 vscr_idx = osal_task_get_current_id();
    //LPVSCR ap_vscr = osd_get_task_vscr(vscr_idx);

    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_IN:
            win_update_all_dongle_menu(TRUE);
            break;
        case CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_OUT:
            win_update_all_dongle_menu(TRUE);
            break;
        case CTRL_MSG_SUBTYPE_CMD_3GDONGLE_STATE_CHANGED:
            win_update_all_dongle_menu(TRUE);
            break;
    }

    return ret;
}

static VACTION usb3g_dongle_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch (key)
    {
        case V_KEY_EXIT:
        case V_KEY_MENU:
        case V_KEY_LEFT:
            act = VACT_CLOSE;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT win_usb3g_dongle_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch (event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title((POBJECT_HEAD)&g_win_usb3g_dongle, RS_USB_3G_DONGLE, 0);
            break;
        case EVN_POST_OPEN:
            win_update_all_dongle_menu(TRUE);
            break;
        case EVN_PRE_CLOSE:
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_MSG_GOT:

            if ((param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT) ||
                    (param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT_ALL))
            {
                ret = PROC_LEAVE;
            }
            else
            {
                win_usb3g_dongle_message_proc(param1, param2);
            }

            break;
    }

    return ret;
}


static PRESULT win_usb3g_dongle_list_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    //UINT8 back_saved;
    //POBJECT_HEAD usb3g_dongle_satlst_menu;
    //OBJLIST *ol;
    //INT32 usb3g_dongle_ret = 0;
    //UINT8 *buffer;
    //UINT32 buffer_len;
    //UINT8 url[128] = {0};
    struct usb3gdongle *dongle = NULL;
    //ol = &usb3g_dongle_olist;

    switch (act)
    {
        case VACT_CAPTURE:
            {
                char *path;
                char msg[128];
                g_usb3g_pcap = !g_usb3g_pcap;
                path = usb3g_pcap("/mnt/uda1", g_usb3g_pcap);
                g_usb3g_pcap = (path == NULL ? 0 : 1);
                MEMSET(msg, 0, 128);
                snprintf(msg, 128,"%s capture 3G data %s", g_usb3g_pcap ? "Start" : "Stop", g_usb3g_pcap ? path : "");
                win_dongle_popup_display(msg, 2000);
                break;
            }
        case VACT_ENTER:
            dongle = usb3g_get_dongle_by_index(usb3g_dongle_list_item_sel);
            if (dongle)
            {
            if (!dongle->simready)
            {
                UINT8 back_saved;
                win_msg_popup_close();
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg("check SIM card!!!!", NULL, 0);
                win_compopup_open_ext(&back_saved);
                usb3g_dongle_simready(dongle);
                win_compopup_smsg_restoreback();

                if (!dongle->simready)
                {
                    win_dongle_popup_display("SIM card is error!!!!", 1000);
                    break;
                }
            }

            if (STRLEN(dongle->config.isp.phone_num) == 0)
            {
                struct isp_info *isp = usb3g_get_isp_service_by_imsi(dongle->imsi);

                if (isp)
                {
                    MEMCPY(&dongle->config.isp, isp, sizeof(struct isp_info));
                }
                else
                {
                    struct isp_info defisp;
                    sys_data_get_def_ispinfo(&defisp);
                    MEMCPY(&dongle->config.isp, &defisp, sizeof(struct isp_info));
                }
            }

            win_dial_open(dongle);
            }
            break;
        default:
            break;
    }

    return ret;
}

static VACTION usb3g_dongle_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        case V_KEY_P_UP:
            act = VACT_CURSOR_PGUP;
            break;
        case V_KEY_P_DOWN:
            act = VACT_CURSOR_PGDN;
            break;
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        case V_KEY_RED:
            act = VACT_CAPTURE;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT usb3g_dongle_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;

    switch (event)
    {
        case EVN_PRE_DRAW:
            win_dongle_list_update_display(FALSE);
            break;
        case EVN_POST_DRAW:
            break;
        case EVN_ITEM_PRE_CHANGE:
            break;
        case EVN_ITEM_POST_CHANGE:
            usb3g_dongle_list_item_sel = osd_get_obj_list_new_point(&usb3g_dongle_olist);
            win_dongle_info_txt_update();
            break;
        case EVN_PRE_CHANGE:
            break;
        case EVN_POST_CHANGE:
            break;
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1 >> 16);
            ret = win_usb3g_dongle_list_unkown_act_proc(unact);
            break;
        default:
            break;
    }

    return ret;
}

static VACTION usb3g_dongle_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    return act;
}

static PRESULT usb3g_dongle_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
//  UINT8  b_id;

    switch (event)
    {
        case EVN_FOCUS_PRE_GET:
            break;
        case EVN_FOCUS_PRE_LOSE:
            break;
        case EVN_UNKNOWN_ACTION:
            break;
    }

    return ret;
}

void ap_usb3g_dongle_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    struct usb3gdongle *dongle = (struct usb3gdongle *)msg_code;;
    struct isp_info defisp;

    UINT32 netdevicetype = 0;
    UINT32 net_choose_flag = 0;

    switch (msg_type)
    {
        case CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_IN:
            win_dongle_popup_display("a 3G dongle plug in", 1000);
            
            net_choose_flag = sys_data_get_net_choose_flag();
            nic_getcur_netdevice((unsigned int *)&netdevicetype);
            if(NET_CHOOSE_3G_DONGLE_ID == net_choose_flag)
            {
            	nic_switch_netdevice(netdevicetype,net_choose_flag);
            }
            
            sys_data_get_def_ispinfo(&defisp);

            if (MEMCMP(defisp.isp_code, dongle->imsi, STRLEN(defisp.isp_code))  == 0
                    && STRLEN(defisp.phone_num) > 0)
            {
                MEMCPY(&dongle->config.isp, &defisp, sizeof(struct isp_info));
            }

            //refresh window
            if (menu_stack_get_top() == (POBJECT_HEAD)&g_win_usb3g_dongle)
            {
                win_update_all_dongle_menu(TRUE);
            }
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_USB3G_DONGLE, USB3G_STATUS_PLUGIN, FALSE);
            //usb3g_start_dail(dongle);
            break;
        case CTRL_MSG_SUBTYPE_CMD_3GDONGLE_HOTPLUG_OUT:
            win_dongle_popup_display("a 3G dongle plug out", 1000);

            //refresh window
            if (menu_stack_get_top() == (POBJECT_HEAD)&g_win_usb3g_dongle)
            {
                if (usb3g_dongle_count() == 0)
                {
                    osd_obj_close(menu_stack_get_top(), C_CLOSE_CLRBACK_FLG);
                    menu_stack_pop();

                    if (menu_stack_get_top() != NULL)
                    {
                        osd_obj_open(menu_stack_get_top(), 0);
                    }
                }
                else
                {
                    win_update_all_dongle_menu(TRUE);
                }
            }
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_USB3G_DONGLE, USB3G_STATUS_PULLOUT, FALSE);
            break;
        case CTRL_MSG_SUBTYPE_CMD_3GDONGLE_STATE_CHANGED:

            switch (dongle->state)
            {
                case dongle_state_connected:
                    win_dongle_popup_display("3G dongle dial connected", 1000);
#if 0/* test code  --Doy.Dong, 2013-1-23*/

                    do
                    {
                        //lwip_ping("119.132.103.148", 64, 4);
                        //lwip_ping("www.baidu.com", 32, 4);
                        //lwip_ping("www.sina.com.cn", 32, 4,);
                        //lwip_ping("www.google.com.hk", 32, -1);
                        //http_download("http://mirror.yongbok.net/nongnu/lwip/lwip-1.4.1.zip");
                        //http_download("http://www.poray.com.cn/Upfiles/down/R50E_UserManual_Chinese.pdf");
                    }
                    while (0);

#endif /****test code****/
                    break;
                case dongle_state_connectting:
                    win_dongle_popup_display("3G dongle start connectting ...", 1000);
                    break;
                case dongle_state_disconnectting:
                    win_dongle_popup_display("3G dongle disconnectting ...", 1000);
                    break;
                case dongle_state_connect_err:
                    win_dongle_popup_display("3G dongle connect failed!", 1000);
                    break;
                default:
                    break;
            }

            //refresh window
            if (menu_stack_get_top() == (POBJECT_HEAD)&g_win_usb3g_dongle)
            {
                win_update_all_dongle_menu(TRUE);
            }

            break;
    }
}


#endif // USB3G_DONGLE_SUPPORT
