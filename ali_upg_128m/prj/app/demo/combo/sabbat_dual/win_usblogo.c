/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_usblogo.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef USB_LOGO_TEST

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libpub29/lib_hde.h>
//#include <api/libpub29/lib_pub27.h>
//#include <api/libtsi/si_monitor.h>
#include <api/libmp/lib_mp.h>
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>
#include <api/libmp/lib_mp.h>


#include <api/libosd/osd_lib.h>

//#include <bus/usb/usb.h>
#include <bus/usb2/usb.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "win_com_menu_define.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "usb_tpl.h"

#define USB_SUPPORT_2PORT

UINT32  usb_if_test_flg = IF_TEST_NULL;


/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER    g_win_usb_logo;

extern CONTAINER    usb_vid_con;
extern TEXT_FIELD    usb_vid_title;
extern MULTISEL    usb_vid_sel;
extern CONTAINER    enum_pid_con;
extern TEXT_FIELD    enum_pid_title;
extern MULTISEL    enum_pid_sel;
extern CONTAINER    usb_enum_con;
extern TEXT_FIELD    usb_logo_enum;
extern CONTAINER    usb_pid_con;
extern TEXT_FIELD    usb_pid_title;
extern MULTISEL        usb_pid_sel;
extern CONTAINER    usb_btn_con;
extern TEXT_FIELD    usb_logo_btn;
extern CONTAINER    usb_port_con;
extern TEXT_FIELD    usb_port_title;
extern MULTISEL    usb_port_sel;

extern TEXT_FIELD usblogo_msg;

enum
{
#ifdef USB_SUPPORT_2PORT
    USB_PORT_ID = 1,
    USB_VID_ID,
#else
    USB_VID_ID = 1,
#endif
    ENUM_PID_ID,
    USB_ENUM_ID,
    USB_PID_ID,
    USB_START_ID,
};


/* USB_DT_DEVICE: Device descriptor */
struct usb_device_descriptor
{
    UINT8  b_length;
    UINT8  b_descriptor_type;

    UINT16 bcd_usb;
    UINT8  b_device_class;
    UINT8  b_device_sub_class;
    UINT8  b_device_protocol;
    UINT8  b_max_packet_size0;
    UINT16 id_vendor;
    UINT16 id_product;
    UINT16 bcd_device;
    UINT8  i_manufacturer;
    UINT8  i_product;
    UINT8  i_serial_number;
    UINT8  b_num_configurations;

}__attribute__((aligned(32)));

#ifdef EHCI_USB_LOGO_TEST
       extern struct usb_device_descriptor dev_detail;
    extern int usb_logo_device_pulg_in;
    extern int test_jk;
#else
    static struct usb_device_descriptor dev_detail;
#endif

char *str_usbs[]=
{
    "Front USB",
    "Rear USB",
};

UINT32 hex_vids[]=
{
    0x1a0a
};

#define pid_test_se0 0x0101
#define pid_test_j 0x0102
#define pid_test_k 0x0103
#define pid_test_packet 0x0104
#define pid_test_suspend 0x0106
#define pid_test_get_desc 0x0107
#define pid_test_get_desc_data  0x0108
#define pid_test_bus_enumeration 0x0109

#define OPEN_USB_INFO    1
#define CLOSE_USB_INFO    0

UINT32 hex_pids[]=
{
    pid_test_se0,
    pid_test_j,
    pid_test_k,
    pid_test_packet,
    pid_test_suspend,
    pid_test_get_desc,
    pid_test_get_desc_data,
    pid_test_bus_enumeration,
};

char *str_pids[]=
{
    "PID_test_se0",
    "PID_test_J",
    "PID_test_K",
    "PID_test_packet",
    "PID_test_suspend",
    "PID_test_get_desc",
    "PID_get_desc_data",
};

USB_DEVICE_TPL usb_tpls[]={
                0x0781,0x5530,"Flash" ,"sandisk cruzer" ,                    //QT-UB-LOGO-F01
        0x0951,0x1653,"Flash", "KingSton ",                           //QT-UB-LOGO-F02
                0x0781,0x5535, "Flash" ,"SanDisk" ,                           //QT-UB-LOGO-F03
        0x1058,0x1021,"HDD",  "Western Digital"   ,                    //QT-UB-LOGO-HD01
        0x0bc2,0x50a1,"HDD","STX-GFD-USB3.0",                         //QT-UB-LOGO-HD02
                };



/*
USB_DEVICE_TPL usb_tpls[]={
                0x045E,0x00F6,"Mouse","MicroSoft 1068",
                0x067B,0x2506,"HDD","ydstar",
                0x04CF,0x8818,"HDD","YuanXing 3.5'",
                0x058F,0x6390,"HDD","ydstar x212 2.5",
                0x067B,0x2507,"HDD","DATASTORAGE PD2500",
                //0x04B4,0x6830,"HDD","Hammer-2500",
                0x04B4,0x6830,"HDD","HP 2.5'",
                0x152D,0x2338,"HDD","Z-TEKUSB2.0",
                0x14CD,0x6600,"HDD","USB 2.0 TO IDE CABLE (2.5'/3.5')",
                0x0951,0x1603,"Flash","kingston DataTraveler",
                0x058F,0x6387,"Flash","UNIbit",
                0x1307,0x0163,"Flash","Teclast",
                0x0DD8,0x0C00,"Flash","Netac",
                0x0,0x0,"Flash","PNY Black",
                0x1043,0x8012,"Flash","THUNIS",
                0x1687,0x0163,"Flash","KINGMAX",
                0x3538,0x0054,"Flash","agio",
                0x0951,0x1605,"Flash","KingSton",
                0x0781,0x5406,"Flash","Cruzer",
                0x054C,0x0243,"Flash","SONY",
                0x0930,0x6540,"Flash","TOSHIBA",
                0x05E3,0x0608,"Hub","D-Link DUB-H4",
                0x058F,0x9254,"Hub","BELKIN 4-PORT Mini Hub(Model# F5U217zhMOB-OE)",
                0x046D,0x08AF,"QuickCam","Logitech ver.100",
                0xFFFF,0xFFFF,"Unknow device"," ",
                0x01da,0x0158,"card ader","s card ",
                0x01da,0x0158,"car reader","s cad ",
                0x0bda,0x0158,"card reader0","sd card ",
                0x090c,0x1000,"Flash","LG",
                0x05e3,0x0718,"HDD","SAMSUNG",
                0x043e,0x70d4,"Flash","XTICK",
                0x13fd,0x1840,"HDD","DATAAGE",
                0x152D,0x2339,"HDD","BUSlink",
                };
    */

UINT32 usb_tpl_arry_cnt = (sizeof(usb_tpls))/(sizeof(usb_tpls[0]));


#define USBLOGO_MAX_MSG_LINE 4
UINT8 usblogo_msg_line;

static VACTION usblogo_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usblogo_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION usblogo_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usblogo_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION usblogo_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT usblogo_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_HD//WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD


#define TXT_L_OF      TXTN_L_OF
#define TXT_W          TXTN_W
#define TXT_H        TXTN_H
#define TXT_T_OF    TXTN_T_OF

#define SEL_L_OF     TXTS_L_OF
#define SEL_W          TXTS_W
#define SEL_H        TXTS_H
#define SEL_T_OF    TXTS_T_OF

#define USBLOGO_MSG_GAP    CON_GAP
#define USBLOGO_MSG_L    CON_L
#define USBLOGO_MSG_T    CON_T
#define USBLOGO_MSG_W    CON_W
#define USBLOGO_MSG_H    CON_H



#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,hl,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    usblogo_item_keymap,usblogo_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TXT1(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl)    \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    usblogo_sel_keymap,usblogo_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_ITEM_SEL(root,var_con,nxt_obj,var_txt,var_num,ID,idu,idd,l,t,w,h,\
                        hl,res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)
#define LDEF_ITEM_TXT(root,var_con,nxt_obj,var_txt,ID,idu,idd,l,t,w,h,\
                        hl,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,hl,&var_txt,ID)   \
    LDEF_TXT1(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,w,TXT_H,res_id)    \


#define LDEF_TXT_MSG(root,var_txt)        \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, \
    TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[4])


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    usblogo_keymap,usblogo_callback,  \
    nxt_obj, focus_id,0)

#ifdef USB_SUPPORT_2PORT
LDEF_ITEM_SEL(g_win_usb_logo,usb_port_con,&usb_vid_con,usb_port_title,usb_port_sel,\
    1,6,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,CON_HL_IDX,display_strs[0],STRING_PROC,0,0,NULL)
LDEF_ITEM_SEL(g_win_usb_logo,usb_vid_con,&enum_pid_con,usb_vid_title,usb_vid_sel,\
    2,1,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,CON_HL_IDX,display_strs[1],STRING_PROC,0,0,NULL)
LDEF_ITEM_SEL(g_win_usb_logo,enum_pid_con,&usb_enum_con,enum_pid_title,enum_pid_sel,\
    3,2,4,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL_IDX,display_strs[2],STRING_PROC,0,0,NULL)
LDEF_ITEM_TXT(g_win_usb_logo,usb_enum_con,&usb_pid_con,usb_logo_enum,\
    4,3,5,CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H,CON_HL_IDX,RS_USB_ENUM_BUS)
LDEF_ITEM_SEL(g_win_usb_logo,usb_pid_con,&usb_btn_con,usb_pid_title,usb_pid_sel,\
    5,4,6,CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H,CON_HL_IDX,display_strs[3],STRING_PROC,0,0,NULL)
LDEF_ITEM_TXT(g_win_usb_logo,usb_btn_con,NULL,usb_logo_btn,\
    6,5,1,CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H,CON_HL_IDX,RS_COMMON_START)

LDEF_TXT_MSG(g_win_usb_logo,usblogo_msg)

LDEF_WIN(g_win_usb_logo,&usb_port_con,W_L,W_T,W_W,W_H,1)
#else
LDEF_ITEM_SEL(g_win_usb_logo,usb_vid_con,&enum_pid_con,usb_vid_title,usb_vid_sel,\
    1,5,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,CON_HL_IDX,display_strs[0],STRING_PROC,0,0,NULL)
LDEF_ITEM_SEL(g_win_usb_logo,enum_pid_con,&usb_enum_con,enum_pid_title,enum_pid_sel,\
    2,1,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H,CON_HL_IDX,display_strs[1],STRING_PROC,0,0,NULL)
LDEF_ITEM_TXT(g_win_usb_logo,usb_enum_con,&usb_pid_con,usb_logo_enum,\
    3,2,4,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H,CON_HL_IDX,RS_USB_ENUM_BUS)
LDEF_ITEM_SEL(g_win_usb_logo,usb_pid_con,&usb_btn_con,usb_pid_title,usb_pid_sel,\
    4,3,5,CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H,CON_HL_IDX,display_strs[2],STRING_PROC,0,0,NULL)
LDEF_ITEM_TXT(g_win_usb_logo,usb_btn_con,NULL,usb_logo_btn,\
    5,4,1,CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H,CON_HL_IDX,RS_COMMON_START)

LDEF_TXT_MSG(g_win_usb_logo,usblogo_msg)

LDEF_WIN(g_win_usb_logo,&usb_vid_con,W_L,W_T,W_W,W_H,1)
#endif

/*******************************************************************************/
static void usblogo_init()
{
    PTEXT_FIELD ptxt;
    PMULTISEL psel;
    POBJECT_HEAD p_con;

    usblogo_msg_line = 0;
    ptxt = &usblogo_msg;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"");

#ifdef USB_SUPPORT_2PORT
    ptxt = &usb_port_title;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"USB PORT");
#endif
    ptxt = &usb_vid_title;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"VID");
    ptxt = &usb_pid_title;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)str_pids[0]);
    ptxt = &enum_pid_title;
    osd_set_text_field_content(ptxt,STRING_ANSI,(UINT32)"PID");

#ifdef USB_SUPPORT_2PORT
    psel = &usb_port_sel;
    osd_set_multisel_count(psel,2);
    osd_set_multisel_sel(psel,0);
#endif
    psel = &usb_vid_sel;
    osd_set_multisel_count(psel,1);
    osd_set_multisel_sel(psel,0);
    psel = &usb_pid_sel;
    osd_set_multisel_count(psel,7);
    osd_set_multisel_sel(psel,0);
    psel = &enum_pid_sel;
    osd_set_multisel_count(psel,0);
    osd_set_multisel_sel(psel,0);

    p_con = (POBJECT_HEAD)&enum_pid_con;
    osd_set_attr(p_con, C_ATTR_INACTIVE);//When do not enumerate bus,invalid the item

    osd_set_container_focus(&g_win_usb_logo,1);
}

static void usbenum_init()
{
    POBJECT_HEAD p_con;
    PMULTISEL psel;

    p_con = (POBJECT_HEAD)&enum_pid_con;
    osd_set_attr(p_con, C_ATTR_ACTIVE);

    psel = &enum_pid_sel;
    osd_set_multisel_count(psel,1);
    osd_set_multisel_sel(psel,0);
    osd_draw_object((POBJECT_HEAD)psel, C_UPDATE_ALL);
    psel = &usb_vid_sel;
    osd_draw_object((POBJECT_HEAD)psel, C_UPDATE_ALL);
}

static void usblogo_set_item_attr(UINT8 b_attr)
{
    UINT string_id;
    PTEXT_FIELD ptxt;
    PCONTAINER pcon;

    if(b_attr == C_ATTR_ACTIVE)
        string_id = RS_COMMON_START;
    else
        string_id = RS_DISEQC12_MOVEMENT_STOP;

    ptxt = &usb_logo_btn;
    osd_set_text_field_content(ptxt,STRING_ID,string_id);
    osd_track_object((POBJECT_HEAD)ptxt,C_UPDATE_ALL);

    pcon = &usb_vid_con;
    osd_set_attr((POBJECT_HEAD)pcon,b_attr);
    osd_draw_object((POBJECT_HEAD)pcon,C_UPDATE_ALL);
    pcon = &usb_pid_con;
    osd_set_attr((POBJECT_HEAD)pcon,b_attr);
    osd_draw_object((POBJECT_HEAD)pcon,C_UPDATE_ALL);
}


void usblogo_show_string(char* str)
{
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg((char *)str,NULL,0);
        #ifndef SD_UI
        win_compopup_set_frame(300, 430, 550, 130);
        #endif
        win_compopup_open();
        osal_task_sleep(2000);
}

void usblogo_info_show(UINT32 vid,UINT32 pid,unsigned char b_is_to_popup)
{
    UINT8 back_saved = 0;
    UINT16 len_display_str[80] = {0};

    //sreturn;

     switch(pid)
    {
        case pid_test_se0:
            #ifdef EHCI_USB_LOGO_TEST
            if (usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please remove USB device !");
            else
            #endif
            snprintf(len_display_str, 80, "%s", "Test SE0_NAK...");
            break;
        case pid_test_j:
            #ifdef EHCI_USB_LOGO_TEST
            if (usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please remove USB device !");
            else
            #endif
            snprintf(len_display_str, 80, "%s", "Test J...");
            break;
        case pid_test_k:
            #ifdef EHCI_USB_LOGO_TEST
            if (usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please remove USB device !");
            else
            #endif
            snprintf(len_display_str, 80, "%s", "Test K...");
            break;
        case pid_test_packet:
            #ifdef EHCI_USB_LOGO_TEST
            if (usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please remove USB device !");
            else
            #endif
            snprintf(len_display_str, 80, "%s", "Test packet...");
            break;
        case pid_test_suspend:
            #ifdef EHCI_USB_LOGO_TEST
            if (test_jk)
                snprintf(len_display_str, 80, "%s", "Please do Power off/on again !");
            else if (!usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please insert USB device !");
            else
            #endif
                snprintf(len_display_str, 80, "%s", "TEST_HOST_SUSPEND_RESUME...");
            break;
        case pid_test_get_desc:
            #ifdef EHCI_USB_LOGO_TEST
             if (test_jk)
                snprintf(len_display_str, 80, "%s", "Please do Power off/on again !");
            else if (!usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please insert USB device !");
            else
            #endif
                snprintf(len_display_str, 80, "%s", "TEST_STEP_GET_DEV_DESC...");
            break;
        case pid_test_get_desc_data:
            #ifdef EHCI_USB_LOGO_TEST
            if (test_jk)
                snprintf(len_display_str, 80, "%s", "Please do Power off/on again !");
            else if (!usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please insert USB device !");
            else
            #endif
                snprintf(len_display_str, 80, "%s", "TEST_STEP_GET_DEV_DESC_DATA...");
            break;
        case pid_test_bus_enumeration:
            #ifdef EHCI_USB_LOGO_TEST
            if (test_jk)
                snprintf(len_display_str, 80, "%s", "Please do Power off/on again !");
            else if (!usb_logo_device_pulg_in)
                snprintf(len_display_str, 80, "%s", "Please insert USB device !");
            else
            #endif
                snprintf(len_display_str, 80, "%s", "test_bus_enumeration...");
            break;
        default:
            snprintf(len_display_str, 80,"%s %04x %s %04x","PID err   VID:",vid," PID:",pid);
            break;
     }

    if(b_is_to_popup ==OPEN_USB_INFO)
    {
        //open the info msg box
        usblogo_show_string(len_display_str);
    }
    else
    {
        //close the info msg box,and updata
        osd_track_object((POBJECT_HEAD)&g_win_usb_logo,C_UPDATE_ALL);
    }

    return;
}



static PRESULT usblogo_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    CONTAINER* cont = (CONTAINER*)p_obj;
    UINT8    id = osd_get_obj_id(p_obj);

    switch(event)
    {
        case EVN_PRE_OPEN:
            usblogo_init();
                   wincom_open_title((POBJECT_HEAD)&g_win_usb_logo,RS_USB_LOGO_TEST, 0);
            break;
        case EVN_POST_OPEN:
            //TODO:test enter mode
            usb_test_enter();
            usb_if_test_flg = IF_TEST_ENTER ;
            break;
        case EVN_PRE_CLOSE:
            if(usb_logo_btn.w_string_id == RS_DISEQC12_MOVEMENT_STOP)
            {
                usblogo_set_item_attr(C_ATTR_ACTIVE);
                usb_test_stop();
            }
            break;
        case EVN_POST_CLOSE:
            usb_test_exit();
            break;
        case EVN_MSG_GOT:
            if(param1 ==  CTRL_MSG_SUBTYPE_CMD_EXIT)
            {
                ret = PROC_LEAVE;
            }
            break;
        default:
            break;
    }
    return ret;
}

static VACTION usblogo_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

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

static VACTION usblogo_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
    {
    case V_KEY_ENTER:
    case V_KEY_RIGHT:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
    }

    return act;
}
static PRESULT usblogo_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    char *enum_str = NULL;
    UINT8 b_id = 0, b_attr = 0;
    UINT16 idx = 0, usb_port = 0;
    PRESULT ret;
    VACTION unact;

    ret = PROC_PASS;
    b_id = osd_get_obj_id(p_obj);
    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1>>16);
            if(unact == VACT_ENTER)
            {
                if(b_id == USB_START_ID)
                {
                    b_attr = (usb_logo_btn.w_string_id == RS_DISEQC12_MOVEMENT_STOP)?C_ATTR_ACTIVE : C_ATTR_INACTIVE;
                    usblogo_set_item_attr(b_attr);
                    osd_track_object((POBJECT_HEAD)&usb_btn_con,C_UPDATE_ALL);


                    if(b_attr == C_ATTR_ACTIVE)
                    {
                        usblogo_info_show(hex_vids[0],hex_pids[idx],CLOSE_USB_INFO);
                        usb_test_stop();
                    }
                    else
                    {
                        idx = osd_get_multisel_sel(&usb_pid_sel);
                    #ifdef USB_SUPPORT_2PORT
                        usb_port = osd_get_multisel_sel(&usb_port_sel);

                    #ifdef    EHCI_USB_LOGO_TEST
                        //libc_printf("%s(): usb_port=%u \n",__FUNCTION__,usb_port);
                        if (hex_pids[idx] >= pid_test_suspend)
                            usb_port |= 0x80;
                        else
                            usb_port &= 0x7f;
                    #endif
                        usb_test_set_port(usb_port);    //this func is in usb_logo_test.c
                    #endif
                        usblogo_info_show(hex_vids[0],hex_pids[idx],OPEN_USB_INFO);

                        usb_test_start(hex_vids[0],hex_pids[idx]);
                    }

                }
                else if(b_id == USB_ENUM_ID)
                {
                #ifdef USB_SUPPORT_2PORT
                    usb_port = osd_get_multisel_sel(&usb_port_sel);
                #ifdef    EHCI_USB_LOGO_TEST
                    usb_port |= 0x80;
                #endif
                    usb_test_set_port(usb_port);    //this func is in usb_logo_test.c
                #endif
                    usblogo_info_show(hex_vids[0],pid_test_bus_enumeration,OPEN_USB_INFO);

                    usb_test_bus_enum(&dev_detail);
                    //usblogo_if_test_start(hex_vids[0],PID_test_bus_enumeration);
                    usbenum_init();

                    usblogo_info_show(hex_vids[0],pid_test_bus_enumeration,CLOSE_USB_INFO);
                }
            }
            break;
        default:
            break;
    }

    return ret;
}

static VACTION usblogo_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
        default:
            act = VACT_PASS;
    }

    return act;
}
static PRESULT usblogo_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    char str_buff[16] = {0};
    UINT8 b_id = 0;
    UINT16 sel,item_cnt,*uni_str;
    PRESULT ret;
    VACTION unact;
    PMULTISEL psel;

    ret = PROC_PASS;
    b_id = osd_get_obj_id(p_obj);
    switch(event)
    {
        case EVN_REQUEST_STRING:
            sel = param1;
            uni_str= (UINT16*)param2;
            if(b_id == USB_VID_ID)
            {
                psel = &enum_pid_sel;
                item_cnt = osd_get_multisel_count(psel);

                if(item_cnt == 0)
                {
                    snprintf(str_buff, 16,"0x%04x",hex_vids[0]);
                    com_asc_str2uni(str_buff,uni_str);
                }
                else
                {
                    snprintf(str_buff, 16,"0x%04x",dev_detail.id_vendor);
                    com_asc_str2uni(str_buff,uni_str);
                }
            }
            else if(b_id == ENUM_PID_ID)
            {
                item_cnt = osd_get_multisel_count((PMULTISEL)p_obj);

                if(item_cnt == 0)
                {
                    com_asc_str2uni("Invlid",uni_str);
                }
                else
                {
                    snprintf(str_buff, 16,"0x%04x",dev_detail.id_product);
                    com_asc_str2uni(str_buff,uni_str);
                }
            }
            else if(b_id == USB_PID_ID)
            {
                snprintf(str_buff, 16,"0x%04x",hex_pids[sel]);
                com_asc_str2uni(str_buff,uni_str);

                osd_set_text_field_content(&usb_pid_title,STRING_ANSI,(UINT32)str_pids[sel]);
            }
#ifdef USB_SUPPORT_2PORT
            else if(b_id == USB_PORT_ID)
            {
                if(SYS_MAIN_BOARD== BOARD_DB_M3811_01V01)
                    com_asc_str2uni(str_usbs[0],uni_str);
                else
                    com_asc_str2uni(str_usbs[sel],uni_str);
                }
#endif
            else
            {
                soc_printf("Unpredicted conditions occur!!\n");
            }
            break;
        case EVN_POST_CHANGE:
            if(b_id==USB_PID_ID)
                osd_track_object((POBJECT_HEAD)&usb_pid_con,C_UPDATE_ALL);
            break;
        default:
            break;
    }

    return ret;
}
#endif

