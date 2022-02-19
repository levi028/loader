/*****************************************************************************
*    Copyright (C) 2008 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: win_net_upg.c
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2008/12/4      Roman        1.0.0          Create
*
*****************************************************************************/
#include <sys_config.h>

#ifdef VPN_ENABLE
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
#include <api/libtcpip/lwip/inet.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_com_menu_define.h"
#include "mobile_input.h"


/*******************************************************************************
*   Objects definition
*******************************************************************************/
extern CONTAINER g_win_vpn_setting;

extern CONTAINER vpnset_item_con1;
extern CONTAINER vpnset_item_con2;
extern CONTAINER vpnset_item_con3;
extern CONTAINER vpnset_item_con4;
extern CONTAINER vpnset_item_con5;
extern CONTAINER vpnset_item_con6;

extern TEXT_FIELD vpnset_item_txtname1;
extern TEXT_FIELD vpnset_item_txtname2;
extern TEXT_FIELD vpnset_item_txtname3;
extern TEXT_FIELD vpnset_item_txtname4;
extern TEXT_FIELD vpnset_item_txtname5;
extern TEXT_FIELD vpnset_item_txtname6;

TEXT_FIELD vpn_btntxt_connect;
TEXT_FIELD vpn_btntxt_disconnect;
TEXT_FIELD vpn_btntxt_caps;
TEXT_FIELD vpn_btntxt_del;
BITMAP   vpn_btnbmp_caps;
BITMAP   vpn_btnbmp_del;

extern MULTISEL vpnset_item_tcpip_type;
extern MULTISEL vpnset_item_encrypt;
extern EDIT_FIELD vpnset_item_server;
extern EDIT_FIELD vpnset_item_user;
extern EDIT_FIELD vpnset_item_pwd;
extern MULTISEL vpnset_item_txtset5;
extern MULTISEL vpnset_item_txtset6;

extern CONTAINER vpnset_info_con;
extern TEXT_FIELD vpnset_txt_progress;
extern PROGRESS_BAR vpnset_bar_progress;
extern TEXT_FIELD vpnset_txt_msg;

extern TEXT_FIELD vpnset_item_line1;
extern TEXT_FIELD vpnset_item_line2;
extern TEXT_FIELD vpnset_item_line3;
extern TEXT_FIELD vpnset_item_line4;
extern TEXT_FIELD vpnset_item_line5;
extern TEXT_FIELD vpnset_item_line6;


extern void ap_send_msg(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer);
extern int vpn_start(vpn_cfg *vpn);
extern int vpn_init(vpn_notify notify);
extern void sys_data_set_vpn_setting(vpn_cfg *vpncfg);
extern void vpn_stop();
extern void draw_caps_del_colbtn(UINT16 x, UINT16 y, UINT32 mode);
extern void draw_caps_flag(UINT16 x, UINT16 y, UINT32 mode);
extern int vpn_state();
extern void sys_data_get_vpn_setting(vpn_cfg *vpncfg);
static VACTION vpnset_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT vpnset_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  vpnset_item_edf_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT  vpnset_item_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION vpnset_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT vpnset_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION vpnset_win_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT vpnset_win_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION vpn_btn_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT vpn_btn_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static void vpn_notify_cb(int msg);

static UINT16 vpn_onoff[];

#define NETUPG_MSG_MAX_LINE 2
//static char vpnset_msg_buf[NETUPG_MSG_MAX_LINE + 1][64];
//static UINT32 vpnset_msg_line;

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

#define EDF_SH_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_HL_IDX   WSTL_BUTTON_04_HD
#define EDF_SL_IDX   WSTL_BUTTON_01_FG_HD
#define EDF_GRY_IDX  WSTL_BUTTON_07_HD

#define PROGRESSBAR_SH_IDX          WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX      WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX WSTL_BAR_04_HD
#define PROGRESS_TXT_SH_IDX WSTL_BUTTON_01_HD

#define MSG_SH_IDX WSTL_TEXT_09_HD//WSTL_TEXT_04_HD
#define UPG_INFO_SH_IDX WSTL_TEXT_09_HD

#define TXT_L_OF    TXTN_L_OF
#define TXT_W       TXTN_W
#define TXT_H       TXTN_H
#define TXT_T_OF    TXTN_T_OF

#define SEL_L_OF    TXTS_L_OF
#define SEL_W       TXTS_W
#define SEL_H       TXTS_H
#define SEL_T_OF    TXTS_T_OF

#define BAR_L   (CON_L+5)
#define BAR_T   (CON_T + (CON_H + CON_GAP)*6+10)
#define BAR_W   (CON_W-BAR_TXT_W-10)
#define BAR_H   24

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W   100//sharon 60
#define BAR_TXT_T ( BAR_T - 6)
#define BAR_TXT_H   36//24 

#define MSG_L   (CON_L+5)
#define MSG_T   (BAR_T + BAR_H+20)
#define MSG_W   (CON_W-10)
#define MSG_H   36
#define MSG_GAP 4

#define BTN_L_OF    ((W_W - BTN_W*2 - BTN_GAP)/3)
#define BTN_T_OF    (CON_H+CON_GAP)*4+14
#define MTXT_L      (W_L + 20)
#define MTXT_T      (W_T + 30)
#define MTXT_W      (W_W - 60)
#define MTXT_H      200

#define BTN_L1      (W_L + 250)
//#define BTN_L2        (BTN_L1 + BTN_W + 40)
#define BTN_T       (MTXT_T + MTXT_H + 30)
#define BTN_W       160
#define BTN_H       40

#define BTN_SH_IDX   WSTL_BUTTON_SHOR_HD
#define BTN_HL_IDX   WSTL_BUTTON_SHOR_HI_HD
#define BTN_SL_IDX   WSTL_BUTTON_01_FG_HD
#define BTN_GRY_IDX  WSTL_BUTTON_SHOR_GRY_HD

#define COLBTN_L    (W_L + 60)
#define COLBTN_T    (W_T + W_H -82)
#define COLBTN_GAP  4

#define CAPSFLG_L   (COLBTN_L+170+28*2)
#define CAPSFLG_T   (COLBTN_T)
#define POPUPSH_IDX WSTL_BUTTON_POP_SH_HD//WSTL_BUTTON_POP_SH
#define POPUPHL_IDX WSTL_BUTTON_POP_HL_HD//WSTL_BUTTON_POP_HL


#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)     \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
                  ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
                  vpnset_item_con_keymap,vpnset_item_con_callback,  \
                  conobj, focusID,1)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)      \
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_EDIT(root, varNum, nxtObj, ID, l, t, w, h,style,max_len,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
                  ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
                  vpnset_item_edf_keymap,vpnset_item_edf_callback,  \
                  C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,max_len,cursormode,pre,sub,str)

#define LDEF_MSEL(root, varNum, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
                 ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
                 vpnset_item_sel_keymap,vpnset_item_sel_callback,  \
                 C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT_MENU_ITEM(root,varCon,nxtObj,varTxt,varNum,varLine,ID,IDu,IDd,l,t,w,h,\
                            resID,style,max_len,cursormode,pat,pre,sub,str) \
LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
LDEF_EDIT(&varCon,varNum,NULL/*&varLine*/   ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,max_len,cursormode,pat,pre,sub,str)\
LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_SEL_MENU_ITEM(root,varCon,nxtObj,varTxt,varNum,varLine,ID,IDu,IDd,l,t,w,h,\
                           resID,style,cur,cnt,ptbl) \
LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
LDEF_MSEL(&varCon,varNum,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_TXT_MENU_ITEM(root,varCon,nxtObj,varTxt,varLine,ID,IDu,IDd,l,t,w,h,resID) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)\
    LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_INFOCON(varCon,nxtObj,l,t,w,h)     \
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,   \
                  NULL,NULL,  \
                  &nxtObj, 0,0)

#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(varBar, &root, nxtObj, C_ATTR_ACTIVE, 0, \
                    0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX,\
                    NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
                    rl,rt , rw, rh, 1, 100, 100, 0)

#define LDEF_PROGRESS_TXT(root,varTxt,nxtObj,l,t,w,h)       \
    DEF_TEXTFIELD(varTxt,&root,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[6])

#define LDEF_TXT_MSG(root,varTxt)       \
    DEF_TEXTFIELD(varTxt,&root,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0,  0,0,0,0, MSG_SH_IDX,0,0,0,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[7])

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)     \
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
                  vpnset_win_keymap,vpnset_win_callback,  \
                  nxtObj, focusID,0)

#define LDEF_TXT_BTN(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID)       \
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
                  ID,IDl,IDr,IDu,IDd, l,t,w,h, POPUPSH_IDX,POPUPHL_IDX,POPUPSH_IDX,POPUPSH_IDX,   \
                  vpn_btn_keymap,vpn_btn_callback,  \
                  C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_BMP_BTN(root,varBmp,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,iconID) \
    DEF_BITMAP(varBmp, root, nxtObj, C_ATTR_ACTIVE,0,   \
               ID,IDl,IDr,IDu,IDd, l, t, 28, 40, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT, WSTL_TEXT_04_8BIT,    \
               NULL, NULL,  \
               C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, iconID)
#define LDEF_TXT_BTN_EXT(root,varTxt,nxtObj,l,t,w,h,str)    \
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
                  0,0,0,0,0, l,t,w,h, WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,WSTL_TEXT_04_8BIT,   \
                  NULL,NULL,  \
                  C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

static char ip_pattern[] = "i00";
static char string_pattern[] = "s17";
static char pwd_pattern[] = "m-*15";

LDEF_EDIT_MENU_ITEM(g_win_vpn_setting, vpnset_item_con1, &vpnset_item_con2, vpnset_item_txtname1, vpnset_item_server, vpnset_item_line2, 1, 5, 2,    \
                    CON_L, CON_T + (CON_H + CON_GAP) * 0, CON_W, CON_H, RS_VPN_SERVER, \
                    NORMAL_EDIT_MODE, MAX_URL_LEN, CURSOR_NORMAL, ip_pattern, NULL, NULL, display_strs[0])

LDEF_EDIT_MENU_ITEM(g_win_vpn_setting, vpnset_item_con2, &vpnset_item_con3, vpnset_item_txtname2, vpnset_item_user, vpnset_item_line3, 2, 1, 3,  \
                    CON_L, CON_T + (CON_H + CON_GAP) * 1, CON_W, CON_H, RS_VPN_USRNAME, \
                    NORMAL_EDIT_MODE, MAX_IP_USER_NAME, CURSOR_NORMAL, string_pattern, NULL, NULL, display_strs[1])

LDEF_EDIT_MENU_ITEM(g_win_vpn_setting, vpnset_item_con3, &vpnset_item_con4, vpnset_item_txtname3, vpnset_item_pwd, vpnset_item_line4, 3, 2, 4,    \
                    CON_L, CON_T + (CON_H + CON_GAP) * 2, CON_W, CON_H, RS_VPN_PASSWD, \
                    NORMAL_EDIT_MODE, MAX_IP_PWD_LEN, CURSOR_NORMAL, pwd_pattern, NULL, NULL, display_strs[2])

LDEF_SEL_MENU_ITEM(g_win_vpn_setting, vpnset_item_con4, &vpn_btntxt_connect, vpnset_item_txtname4, vpnset_item_encrypt, vpnset_item_line5, 4, 3, 5,   \
                   CON_L, CON_T + (CON_H + CON_GAP) * 3, CON_W, CON_H, RS_VPN_ENCRYPT, \
                   STRING_ID , 0, 2, vpn_onoff)

LDEF_TXT_BTN(g_win_vpn_setting, vpn_btntxt_connect, &vpn_btntxt_disconnect, 5, 4, 1, 4, 1, BTN_L1, BTN_T, BTN_W, BTN_H, RS_COMMON_CONNECT)

 
LDEF_WIN(g_win_vpn_setting, &vpnset_item_con1, W_L, W_T, W_W, W_H, 1)


#define VACT_ABORT (VACT_PASS + 1)
#define VACT_BLUE  (VACT_PASS + 2)
#define VACT_OK    (VACT_PASS + 3)

/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static UINT16 vpn_onoff[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
};

//static int _vpn_pcap_ = 0;
static vpn_cfg g__vpnsetting;

void win_vpn_popup_display(char *string, UINT32 show_ticks)
{
    UINT8 back_saved;
    win_msg_popup_close();
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg(string, NULL, 0);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(show_ticks);
    win_compopup_smsg_restoreback();
}

static void vpnset_mobile_input_init(UINT8 id)
{
    mobile_input_init(&vpnset_item_pwd, NULL);
}

static void vpn_cfg_init()
{
    sys_data_get_vpn_setting(&g__vpnsetting);
    //    g__vpnsetting.encryt=1;
    //    g__vpnsetting.saddr=inet_addr("175.100.167.213");
    //    strcpy(g__vpnsetting.usr,"alexzhu");
    //    strcpy(g__vpnsetting.pwd,"CenbongTelecom");


    //    g__vpnsetting.encryt=1;
    //    g__vpnsetting.saddr=inet_addr("220.133.102.4");
    //    strcpy(g__vpnsetting.usr,"alex");
    //    strcpy(g__vpnsetting.pwd,"alex");


    osd_set_edit_field_content(&vpnset_item_server, STRING_NUMBER, ntohl(g__vpnsetting.saddr));
    osd_set_edit_field_content(&vpnset_item_user, STRING_ANSI, (UINT32)g__vpnsetting.usr);
    osd_set_edit_field_content(&vpnset_item_pwd, STRING_ANSI, (UINT32)g__vpnsetting.pwd);
    osd_set_multisel_sel(&vpnset_item_encrypt, g__vpnsetting.encryt);
}

static int vpn_get_values()
{
    g__vpnsetting.encryt = osd_get_multisel_sel(&vpnset_item_encrypt);
    g__vpnsetting.saddr  = htonl(osd_get_edit_field_content((PEDIT_FIELD)&vpnset_item_server));
    com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&vpnset_item_user), g__vpnsetting.usr);
    com_uni_str_to_asc((UINT8 *)osd_get_edit_field_content(&vpnset_item_pwd), g__vpnsetting.pwd);

    if (g__vpnsetting.saddr == 0 || STRLEN(g__vpnsetting.usr) == 0 || STRLEN(g__vpnsetting.pwd) == 0)
    {
        win_vpn_popup_display("Invalid Input", 1000);
        return 0;
    }
    libc_printf("VPN Setting:\n    Server:%s\n    User  :%s\n    Passwd:%s\n    Enc   :%d\n",
                inet_ntoa(g__vpnsetting.saddr), g__vpnsetting.usr, g__vpnsetting.pwd, g__vpnsetting.encryt);
    return 1;
}

static void vpn_update_info()
{
    if (vpn_state())
    {
        osd_set_text_field_content(&vpn_btntxt_connect, STRING_ID, RS_COMMON_DISCONNECT);
    }
    else
    {
        osd_set_text_field_content(&vpn_btntxt_connect, STRING_ID, RS_COMMON_CONNECT);
    }
    osd_track_object((POBJECT_HEAD)&g_win_vpn_setting, C_UPDATE_ALL);

#if 0 /* test  --Doy.Dong, 2014-10-16*/
    POBJECT_HEAD pObj = OSD_GetFocusObject((POBJECT_HEAD)&g_win_vpn_setting);
    if(pObj == (POBJECT_HEAD)&vpnset_item_con2 || pObj == (POBJECT_HEAD)&vpnset_item_con3)
    {
        libc_printf("focus..............\n");
        draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 1);
        draw_caps_del_colbtn(COLBTN_L, COLBTN_T, 1);
    }
    else
    {
        draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 0);
        draw_caps_del_colbtn(COLBTN_L, COLBTN_T, 0);
    }    
#endif /****test****/
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/


static VACTION vpnset_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch (key)
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

static PRESULT vpnset_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    return ret;
}

static VACTION vpnset_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
    VACTION act;
    switch (key)
    {
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        case V_KEY_YELLOW:
            act = VACT_ABORT;
            break;
        case V_KEY_BLUE:
            act = VACT_BLUE;
            break;
        default:
            act = VACT_PASS;
    }

    return act;
}

static PRESULT vpnset_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
//  VACTION unact;
//  UINT8 bID = osd_get_obj_id(pObj);

    switch (event)
    {
        case EVN_UNKNOWN_ACTION:
            break;
        default:
            break;
    }
    return ret;
}

static VACTION  vpnset_item_edf_keymap(POBJECT_HEAD pObj, UINT32 key)
{
    VACTION act;
    //UINT8 bID;
    UINT8 bStyle;
    EDIT_FIELD  *edf;
    edf = (EDIT_FIELD *)pObj;
    //bID = osd_get_obj_id(pObj);
    bStyle = edf->b_style;

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

            if (bStyle & EDIT_STATUS)
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

static PRESULT  vpnset_item_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT8 bID;
    //UINT8 unact;
    UINT8 caps;
//  UINT32 value;

    mobile_input_type_t mobile_input_type;
    caps = mobile_input_get_caps();
    mobile_input_type.type      = MOBILE_INPUT_NORMAL;
    mobile_input_type.caps_flag = caps ? MOBILE_CAPS_INIT_UP : MOBILE_CAPS_INIT_LOW;
    mobile_input_type.maxlen    = 17;
    mobile_input_type.fixlen_flag = 0;
    mobile_input_type.callback  = NULL;
    //bID = osd_get_obj_id(pObj);

    switch (event)
    {
        case EVN_KEY_GOT:
            if (pObj != (POBJECT_HEAD)&vpnset_item_server)
            {
                ret = mobile_input_proc((EDIT_FIELD *)pObj, (VACTION)(param1 >> 16), param1 & 0xFFFF, param2);
                if (ret == PROC_LOOP)
                {
                    draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 1);
                }
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
            if (pObj == (POBJECT_HEAD)&vpnset_item_user || pObj == (POBJECT_HEAD)&vpnset_item_pwd)
            {
                mobile_input_init((EDIT_FIELD *)pObj, &mobile_input_type);
                draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 1);
                draw_caps_del_colbtn(COLBTN_L, COLBTN_T, 1);
            }
            break;
        case EVN_FOCUS_PRE_LOSE:
        case EVN_PARENT_FOCUS_PRE_LOSE:
            draw_caps_flag(CAPSFLG_L, CAPSFLG_T, 0);
            draw_caps_del_colbtn(COLBTN_L, COLBTN_T, 0);
            break;
    }

    return ret;
}

static VACTION vpnset_win_keymap(POBJECT_HEAD pObj, UINT32 key)
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

static PRESULT vpnset_win_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
//    VACTION unact;

    switch (event)
    {
        case EVN_PRE_OPEN:
            wincom_open_title((POBJECT_HEAD)&g_win_vpn_setting, RS_VPN, 0);
            vpn_cfg_init();
            break;
        case EVN_POST_OPEN:
            break;
        case EVN_PRE_CLOSE:
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

static VACTION vpn_btn_keymap(POBJECT_HEAD pObj, UINT32 key)
{
    VACTION act = VACT_PASS;

    if (key == V_KEY_ENTER)
    {
        act = VACT_ENTER;
    }

    return act;
}

static PRESULT vpn_btn_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT8   bID;
    VACTION unact;
    unact = (VACTION)(param1 >> 16);
    //bID = osd_get_obj_id(pObj);

    if (unact == VACT_CLOSE)
    {
        ret = PROC_LEAVE;
    }

    if (unact == VACT_ENTER)
    {
        if (vpn_state())
        {
            vpn_stop();
        }
        else
        {
            if (vpn_get_values())
            {
                sys_data_set_vpn_setting(&g__vpnsetting);
                sys_data_save(1);
                vpn_init(vpn_notify_cb);
                vpn_start(&g__vpnsetting);
            }
        }
        ret = PROC_LOOP;
    }
    vpn_update_info();
    return ret;
}


/*******************************************************************************
*   other functions  definition
*******************************************************************************/
static void vpn_notify_cb(int msg)
{
    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_VPN_MSG, msg, FALSE);
    //    libc_printf("vpn_notify_cb: %d\n", msg);
}

void vpnset_msg_proc(UINT32 msg_type, UINT32 msg_code)
{
    //int state = 1;
    switch (msg_code)
    {
        case VPN_MSG_START_CONNECTTING:
            win_vpn_popup_display("VPN Startting ...", 1000);
            break;
        case VPN_MSG_CONNECT_SERVER_FAILED:
            win_vpn_popup_display("VPN Connect Server Failed", 1000);
            //vpn_stop();
            break;
        case VPN_MSG_AUTHENTICATE_FAILED:
            win_vpn_popup_display("VPN Authenicate Failed", 1000);
            //vpn_stop();
            break;
        case VPN_MSG_VPN_SUCESS:
            win_vpn_popup_display("VPN Connect Success", 1000);
            break;
        case VPN_MSG_CONNECTION_LOST:
            win_vpn_popup_display("VPN Connection Lost ...", 1000);
            break;
        case VPN_MSG_VPN_CLOSED:
            //state = 0;
            win_vpn_popup_display("VPN Closed", 1000);
            break;
    }

    if (menu_stack_get_top() == (POBJECT_HEAD)&g_win_vpn_setting)
    {
        vpn_update_info();
    }
}

#endif /* VPN_ENBALE */

