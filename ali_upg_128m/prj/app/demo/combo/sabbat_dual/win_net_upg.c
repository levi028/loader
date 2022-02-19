 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_net_upg.c
*
*    Description:   The realize of network upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef NETWORK_SUPPORT
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
#include "win_net_upg.h"

#include <hal/hal_gpio.h>
#include <api/libchunk/chunk.h>
#include <hld/sto/sto_dev.h>
#include <api/libnet/libnet.h>
#include <api/libtcpip/lwip/inet.h>
#include <api/libupg/net_upgrade.h>
#include <api/libchunk/chunk.h>
#ifdef _GEN_CA_ENABLE_
#include <api/librsa/flash_cipher.h>
#include <boot/boot_common.h>
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION netupg_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT netupg_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  netupg_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT  netupg_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION netupg_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT netupg_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION netupg_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT netupg_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static PRESULT  netupg_msg_proc(UINT32 msg_type, UINT32 msg_code);

extern UINT8 get_ip_cfg_last_active();
extern void get_remote_ip_cfg(PIP_REMOTE_CFG pcfg);
extern void get_remote_ip_dft_cfg(PIP_REMOTE_CFG pcfg);
extern void ap_send_msg(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer);
extern void power_on_process(void);
extern BOOL power_off_process(BOOL ota_scan __MAYBE_UNUSED__);
extern void key_pan_display(char *data, UINT32 len);
extern void set_remote_ip_cfg(PIP_REMOTE_CFG pcfg);
extern void set_remote_ip_cfg(PIP_REMOTE_CFG pcfg);
extern unsigned long libnet_perform_test(char *full_url, unsigned char *buffer,
                               unsigned int buf_len, unsigned int offset);
extern unsigned int mg_table_driven_crc(register unsigned int crc,
                register unsigned char *bufptr, register int len);

#ifdef _GEN_CA_ENABLE_
UINT32 get_code_version(UINT8 *data_addr, UINT32 data_len);
#endif

//////////////////////////////////

#define C_SECTOR_SIZE   0x10000

static UINT16 protocol_type[];
static UINT16 url_type[];

#define NETUPG_MSG_MAX_LINE 2
static char netupg_msg_buf[NETUPG_MSG_MAX_LINE+1][64];
static UINT32 netupg_msg_line;

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

#define MAINCODE_CHUNK_ID   0x01FE0101
#define FETCH_LONG(p)  ((((*(p)) << 24)) | ((*(p + 1)) << 16) | ((*(p + 2)) << 8) | (*(p + 3)))
#define VACT_ABORT (VACT_PASS + 1)
#define VACT_BLUE  (VACT_PASS + 2)

#define DOUBLE_DIGIT_MAX 99
#define SITEMS_NUM_MAX 3
#define LENGTH_2BYTES  16
#define LENGTH_3BYTES  24

#define MAIN_CODE_CHUNK_ID      0x01FE0101
#define SEE_CODE_CHUNK_ID       	0x06F90101
#define DECRPT_KEY_ID           		0x22DD0100

enum
{
    IDC_NETUPG_TYPE = 1,
    IDC_NETUPG_URL_TYPE,
    IDC_NETUPG_URL,
    IDC_NETUPG_USER,
    IDC_NETUPG_PASSWORD,
    IDC_NETUPG_START,
};

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    netupg_item_con_keymap,netupg_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,max_len,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    netupg_item_edf_keymap,netupg_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,max_len,cursormode,pre,sub,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    netupg_item_sel_keymap,netupg_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,max_len,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL/*&varLine*/   ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,max_len,\
                        cursormode,pat,pre,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_SEL_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_TXT_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_INFOCON(var_con,nxt_obj,l,t,w,h)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,UPG_INFO_SH_IDX,   \
    NULL,NULL,  \
    &nxt_obj, 0,0)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(var_bar, &root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX,\
        NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
        rl,rt , rw, rh, 1, 100, 100, 0)

#define LDEF_PROGRESS_TXT(root,var_txt,nxt_obj,l,t,w,h)       \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[6])

#define LDEF_TXT_MSG(root,var_txt)       \
    DEF_TEXTFIELD(var_txt,&root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, MSG_SH_IDX,0,0,0,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[7])

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    netupg_con_keymap,netupg_con_callback,  \
    nxt_obj, focus_id,0)

static char ip_pattern[] = "i00";
static char string_pattern[] = "s17";
static char pwd_pattern[] = "m-*15";

LDEF_SEL_MENU_ITEM(g_win_netupg,netupg_item_con1, &netupg_item_con2,netupg_item_txtname1,netupg_item_tcpip_type,\
                netupg_item_line1,1,6,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, RS_NET_PROTOCOL,\
                STRING_ID,0,2,protocol_type)

LDEF_SEL_MENU_ITEM(g_win_netupg,netupg_item_con2, &netupg_item_con3,netupg_item_txtname2,netupg_item_url_type,\
                netupg_item_line2,2,1,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, RS_NET_URL_TYPE,\
                STRING_ID,0,2,url_type)

LDEF_EDIT_MENU_ITEM(g_win_netupg,netupg_item_con3, &netupg_item_con4,netupg_item_txtname3,netupg_item_url,\
                netupg_item_line3,3,2,4,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, RS_NET_URL,\
                NORMAL_EDIT_MODE,MAX_URL_LEN,CURSOR_NORMAL,string_pattern,NULL,NULL,display_strs[0])

LDEF_EDIT_MENU_ITEM(g_win_netupg,netupg_item_con4, &netupg_item_con5,netupg_item_txtname4,netupg_item_user,\
                netupg_item_line4,4,3,5,CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H, RS_INFO_LNB_USER,\
                NORMAL_EDIT_MODE,MAX_IP_USER_NAME,CURSOR_NORMAL,string_pattern,NULL,NULL,display_strs[1])

LDEF_EDIT_MENU_ITEM(g_win_netupg,netupg_item_con5, &netupg_item_con6,netupg_item_txtname5,netupg_item_pwd,\
                netupg_item_line5,5,4,6,CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, RS_NET_PASSWORD,\
                NORMAL_EDIT_MODE,MAX_IP_PWD_LEN,CURSOR_NORMAL,pwd_pattern,NULL,NULL,display_strs[2])

LDEF_TXT_MENU_ITEM(g_win_netupg,netupg_item_con6, &netupg_info_con,netupg_item_txtname6,netupg_item_line6,\
                6,5,1,CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, RS_COMMON_START)

LDEF_INFOCON(netupg_info_con, netupg_bar_progress,CON_L,CON_T + (CON_H + CON_GAP)*6, CON_W, 150)

LDEF_PROGRESS_BAR(netupg_info_con,netupg_bar_progress,&netupg_txt_progress, \
    BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W,BAR_H-10)

LDEF_PROGRESS_TXT(netupg_info_con, netupg_txt_progress, NULL, \
        BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_TXT_MSG(netupg_info_con,netupg_txt_msg)

LDEF_WIN(g_win_netupg,&netupg_item_con1,W_L, W_T, W_W, W_H, 1)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static UINT16 protocol_type[] =
{
    RS_NET_HTTP,
    RS_NET_FTP,
};
static UINT16 url_type[] =
{
    RS_NET_URL_TYPE_INT,
    RS_NET_URL_TYPE_STRING,
};

static struct help_item_resource  netupg_helpinfo[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_COM_CAPS},
    {IM_EPG_COLORBUTTON_GREEN,     RS_HELP_DELETE},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_NET_ABORT},
};

#if 0
static POBJECT_HEAD netupg_objs[] =
{
    (POBJECT_HEAD)&netupg_item_tcpip_type,
    (POBJECT_HEAD)&netupg_item_url,
    (POBJECT_HEAD)&netupg_item_user,
    (POBJECT_HEAD)&netupg_item_pwd,
};
#endif

static IP_REMOTE_CFG net_remote_cfg;
static UINT8 use_last_ip_cfg;

static int download_started = 0;
static UINT32 upgrade_handle = 0; // used to abort download
static UINT32 net_file_type;  //0 - config file, 1 - upgrade abs file
static UINT32 net_file_length; //the file length of the download file

#define DEFULT_CONFIG_FILE "download_setting.cfg"

static UINT8* buffer_for_cfgfile = (UINT8*)((__MM_PVR_VOB_BUFFER_ADDR&0x0fffffff)|0x80000000);
static UINT32 buflen_for_cfgfile = 512 * 1024;
static UINT8* buffer_for_upgradefile =  (UINT8*)(((__MM_PVR_VOB_BUFFER_ADDR&0x0fffffff)|0x80000000) + 1024 * 1024);
static UINT32 buflen_for_upgradefile = 8 * 1024 * 1024;

static UINT8*  __MAYBE_UNUSED__ buffer_for_decrypt = (UINT8*)(((__MM_PVR_VOB_BUFFER_ADDR&0x0fffffff)|0x80000000) + 9* 1024 * 1024);
static UINT32 buflen_for_decrypt __MAYBE_UNUSED__= 6*1024*1024;

static void net_mi_callback(UINT8* string);
static int  network_performance_test = 0;
#if 0
static mobile_input_type_t mit_normal =
{
    MOBILE_INPUT_NORMAL,
    MOBILE_CAPS_INIT_LOW,
    17,
    0,
    net_mi_callback,
};

static mobile_input_type_t mit_pwd =
{
    MOBILE_INPUT_PWD,
    MOBILE_CAPS_INIT_LOW,
    17,
    0,
    net_mi_callback,
};
#endif

static void netupg_mobile_input_init(UINT8 id)
{
    mobile_input_init(&netupg_item_pwd, NULL);
}


static void win_netupg_process_update(UINT32 process)
{

    if(process > PERCENT_MAX)
        process = 0;

    osd_set_progress_bar_pos(&netupg_bar_progress,(INT16)process);
    osd_set_text_field_content(&netupg_txt_progress, STRING_NUM_PERCENT, (UINT32)process);

    osd_draw_object( (POBJECT_HEAD)&netupg_bar_progress, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&netupg_txt_progress, C_UPDATE_ALL);
}

static void win_netupg_msg_update(const char *msg)
{
    UINT32 i;
    OSD_RECT rect;

    if(NETUPG_MSG_MAX_LINE == netupg_msg_line)
    {
        for(i = 0; i < NETUPG_MSG_MAX_LINE - 1; i++)
            strncpy(netupg_msg_buf[i], netupg_msg_buf[i + 1], 63);
        strncpy(netupg_msg_buf[i], (char *)msg, 63);
    }
    else
    {
        strncpy(netupg_msg_buf[netupg_msg_line++], (char *)msg, 63);
    }

    //draw msg infos
    for(i=0; i<NETUPG_MSG_MAX_LINE; i++)
    {
        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&netupg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&netupg_txt_msg, STRING_ANSI, (UINT32)netupg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&netupg_txt_msg, C_UPDATE_ALL);
    }
}

static void win_netupg_msg_clear()
{
    UINT8 i;
    OSD_RECT rect;

    for(i=0;i<NETUPG_MSG_MAX_LINE;i++)
    {
        netupg_msg_buf[i][0] = 0;

        rect.u_left  = MSG_L;
        rect.u_top   = MSG_T+(MSG_H+MSG_GAP)*i;
        rect.u_width = MSG_W;
        rect.u_height= MSG_H;

        osd_set_rect2(&netupg_txt_msg.head.frame, &rect);
        osd_set_text_field_content(&netupg_txt_msg, STRING_ANSI, (UINT32)netupg_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&netupg_txt_msg, C_UPDATE_ALL);
    }

    netupg_msg_line = 0;
}


static void netupg_set_url(BOOL update)
{
    if(net_remote_cfg.url_type == 0)
    {
        // url type is ip address, change to ip pattern
        osd_set_edit_field_pattern(&netupg_item_url,ip_pattern);
        osd_set_edit_field_content(&netupg_item_url, STRING_NUMBER, ntohl(net_remote_cfg.url_int));
    }
    else
    {
        // url type is string, change to string pattern and enable mobile input.
        osd_set_edit_field_pattern(&netupg_item_url,string_pattern);
        osd_set_edit_field_content(&netupg_item_url, STRING_ANSI, (UINT32)net_remote_cfg.url_str);
    }
    if(update)
        osd_draw_object((POBJECT_HEAD)&netupg_item_con3,C_UPDATE_ALL);
}

static void netupg_sel_protocol(UINT32 protocol, BOOL update)
{
    UINT32 n_cmd_draw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL;
    UINT8 action = (protocol == 0) ? C_ATTR_INACTIVE : C_ATTR_ACTIVE; // 0: http
    set_container_active(&netupg_item_con4, action); /* User */
    set_container_active(&netupg_item_con5, action); /* Password */

    if (update)
    {
        osd_draw_object((POBJECT_HEAD)&netupg_item_con4, n_cmd_draw);
        osd_draw_object((POBJECT_HEAD)&netupg_item_con5, n_cmd_draw);
    }
}

static void netupg_init()
{
    UINT16 block_cnt;

    //init the blocks
    netupg_block_free();
    netupg_block_init(&block_cnt);

    // get last remote setting.
    use_last_ip_cfg = get_ip_cfg_last_active();

    if(use_last_ip_cfg)
        get_remote_ip_cfg(&net_remote_cfg);
    else
        get_remote_ip_dft_cfg(&net_remote_cfg);

    // set the tcpip type
    osd_set_multisel_sel(&netupg_item_tcpip_type, net_remote_cfg.protocol_type);

    netupg_sel_protocol(net_remote_cfg.protocol_type, FALSE);

    // set the url type
    osd_set_multisel_sel(&netupg_item_url_type, net_remote_cfg.url_type);

    // set the url
    netupg_set_url(FALSE);

    // user is alway have mobile input type.
    osd_set_edit_field_content(&netupg_item_user, STRING_ANSI, (UINT32)net_remote_cfg.user);

    // to support any key password, it must be mobile input type.
    osd_set_edit_field_content(&netupg_item_pwd, STRING_ANSI, (UINT32)net_remote_cfg.pwd);

    osd_set_progress_bar_pos(&netupg_bar_progress,(INT16)0);
    osd_set_text_field_content(&netupg_txt_progress, STRING_NUM_PERCENT, (UINT32)0);
    osd_set_container_focus(&g_win_netupg,IDC_NETUPG_TYPE);
    netupg_mobile_input_init(IDC_NETUPG_URL);
}

static void win_get_netupg_param()
{
    UINT32 value;

    // get protocol type
    net_remote_cfg.protocol_type = osd_get_multisel_sel(&netupg_item_tcpip_type);

    // get url type
    net_remote_cfg.url_type = osd_get_multisel_sel(&netupg_item_url_type);

    // get url
    value = osd_get_edit_field_content(&netupg_item_url);
    if(net_remote_cfg.url_type == 0)
        net_remote_cfg.url_int = htonl(value);
    else
        com_uni_str_to_asc((UINT8 *)value,(char *)net_remote_cfg.url_str);

    // get user name
    value = osd_get_edit_field_content(&netupg_item_user);
    com_uni_str_to_asc((UINT8*)value, (char *)net_remote_cfg.user);

    // get user password
    value = osd_get_edit_field_content(&netupg_item_pwd);
    com_uni_str_to_asc((UINT8*)value, (char *)net_remote_cfg.pwd);
}

//static LIB_NUC lib_nuc;
static UINT32 net_ui_callback(UINT32 type, UINT32 code)
{
    ap_send_msg(type, code, FALSE);
    return 0;
}

static UINT32 net_user_stop()
{
    return 0;
}

static void generate_url(char *url, UINT32 url_buf_len)
{
    //UINT32 value;
    //int pos;
    UINT32 t_len = 0;

    win_get_netupg_param();

    if (net_remote_cfg.protocol_type == 0)
    {
        if (net_remote_cfg.url_type == 0)
        {
            UINT8 *ptr = (UINT8 *)&net_remote_cfg.url_int;
            snprintf(url, url_buf_len, "http://%d.%d.%d.%d/", ptr[0], ptr[1], ptr[2], ptr[3]);
        }
        else
        {
            snprintf(url, url_buf_len,"http://%s/", net_remote_cfg.url_str);
        }
    }
    else
    {
        snprintf(url, url_buf_len, "ftp://%s:%s@", net_remote_cfg.user,
                                     net_remote_cfg.pwd);
        if (net_remote_cfg.url_type == 0)
        {
            char tmp_string[32];
            UINT8 *ptr = (UINT8 *)&net_remote_cfg.url_int;
            snprintf(tmp_string, url_buf_len, "%d.%d.%d.%d/", ptr[0], ptr[1], ptr[2], ptr[3]);
            t_len = url_buf_len - strlen(url);
            if (t_len > strlen(tmp_string))
            {
                strncat(url, tmp_string, t_len-1);
            }
        }
        else
        {
            t_len = url_buf_len - strlen(url);
            if (t_len > strlen((const char *)net_remote_cfg.url_str))
            {
                strncat(url, (const char *)net_remote_cfg.url_str, t_len-1);
                t_len -= strlen((const char *)net_remote_cfg.url_str);
                strncat(url, "/", t_len-1);
            }
        }
    }
}

static UINT32 netupg_download_configfile()
{
    UINT32 handle;
    char *url = MALLOC(1024);

    if (url == NULL)
    {
        win_netupg_msg_update("No enough memory! Download failed!");
        download_started = 0;
        return 0;
    }
    else
    {
        //sprintf(url, "/%s to be downloaded...", DEFULT_CONFIG_FILE);
        win_netupg_msg_update("start to download config file...");
    }
    MEMSET(url, 0, 1024);
    generate_url(url, 1024);
    if(1 == network_performance_test)
    {
        strncat(url, "test_download.bin", (1024-1-strlen(url)));
        handle = libnet_perform_test(url, buffer_for_cfgfile, buflen_for_cfgfile, 0);
    }
    else
    {
        strncat(url, DEFULT_CONFIG_FILE, (1024-1-strlen(url)));
        net_file_type = 0; // config file
        handle = libnet_download_to_mem(url, buffer_for_cfgfile, buflen_for_cfgfile, 0);
    }
    FREE(url);
    return handle;
}

static UINT32 netupg_download_upgradefile(const char *file)
{
    UINT32 handle;
    char *url = MALLOC(1024);

    if (url == NULL)
    {
        win_netupg_msg_update("No enough memory! Download failed!");
        download_started = 0;
        return 0;
    }
    else
    {
        //sprintf(url, "/%s to be downloaded...", file);
        win_netupg_msg_update("start to download firmware...");
    }
    MEMSET(url, 0, 1024);
    generate_url(url, 1024);
    strncat(url, file, (1024-1-strlen(url)));
    net_file_type = 1; // upgrade abs file
    handle = libnet_download_to_mem(url, buffer_for_upgradefile, buflen_for_upgradefile, 0);

    FREE(url);
    return handle;
}

//call the C library funtion to split the string
static char *_strsep(char **stringp, const char *delim)
{
    register char *tmp = *stringp;
    register char *tmp2 = tmp;
    register const char *tmp3;
    if (!*stringp)
        return 0;
    for (tmp2 = tmp; *tmp2; ++tmp2)
    {
        for (tmp3 = delim; *tmp3; ++tmp3)
            if (*tmp2 == *tmp3)
            {   /* delimiter found */
                *tmp2 = 0;
                *stringp = tmp2 + 1;
                return tmp;
            }
    }
    *stringp = 0;
    return tmp;
}

static int calc_version(const char *sver)
{
    int v = 0;
    int field = 0;
    int len = STRLEN((char *)sver);
    int i;

    //follow the "xx.xx.xx" format
    for(i = 0; i < len; i++)
    {
        if((sver[i] >= '0') && (sver[i] <= '9'))
        {
            field = field * 10 + sver[i] - '0';

            if(field > DOUBLE_DIGIT_MAX)
            {
                return 0;
            }
        }
        else if (sver[i] == '.')
        {
            v = v * 100 + field;
            field = 0;
        }
        else
        {
            //illegal character
            return 0;

        }
    }

    v = v * 100 + field;
    return (v > 999999 ? 0 : v);
}

static int check_hw_module(char* nm)
{
    char module[20];
    MEMSET(module,0,sizeof(module));
    
    if(ALI_S3503 == sys_ic_get_chip_id())
    {
        strncpy(module,"M3503",sizeof(module));
    }
    else if(ALI_S3821== sys_ic_get_chip_id())
    {
        strncpy(module,"M3821",sizeof(module));
    }
    else
    {
        strncpy(module,"Mxxx",sizeof(module));
    }
    if(NULL == strstr(nm,module))
    {
        return 0;
    }
    return 1;
}

static int check_config_item(char *line, char **sver, char **sfile, const char *model)
{
    int i;
    char *sitems[] = {NULL, NULL, NULL, NULL}; /* model, version, file */

    i = 0;
    char *token = _strsep(&line, ";");
    while(token)
    {
        sitems[i++] = token;
        if(SITEMS_NUM_MAX == i )
            break;
        token = _strsep(&line, ";");
    }

    if(i != SITEMS_NUM_MAX)
    {
        //not enough field, return 0
        return 0;
    }

    //split the STB model version
    //1=35001-01001
    char *s = sitems[0];
    token = _strsep(&s, "=");
    if(token == 0)
    {
        //not follow the format
        return 0;
    }
    //check with the ver in bootloader
    /*
    CHUNK_HEADER blk_header;
    unsigned long id = 0;
    sto_chunk_goto(&id, 0, 1);
    sto_get_chunk_header(id, &blk_header);
    */

    libc_printf("STB module = %s\n", s);
    libc_printf("version = %s, %d\n", sitems[1], calc_version(sitems[1]));
    libc_printf("file = %s\n", sitems[2]);
    libc_printf("model version = %s\n", model);

    if(STRCMP(s, (char *)model) != 0)
    {
        #ifdef _GEN_CA_ENABLE_
        if(!check_hw_module(s))
        #endif
        {
            //not for the model
            return 0;
        }
    }

    if(sver)
        *sver =  sitems[1];
    if(sfile)
        *sfile = sitems[2];

    return 1;
}

static int check_config_file(char *buffer, int len)
{
    //firstly split with "\r\n"
    char *line;
    int ver = 0, maxver;

    char *sfile = NULL;
    char *stmpfile = NULL;
    char *sver = NULL, *stmpver = NULL;
    char *ptr = buffer;
    char verbuf[32];

    CHUNK_HEADER chunk_hdr;
    UINT32 id;

    MEMSET(verbuf,0,sizeof(verbuf));

    if (len <= 0)
    {
        win_netupg_msg_update("No valid firmware!");
        download_started = 0;
        return -1;
    }

    /* get version in bootloader */
    id = 0;
    sto_chunk_goto(&id, 0, 1);
    sto_get_chunk_header(id, &chunk_hdr);

    maxver = 0;
    line = _strsep(&ptr, "\n");
    while(line != NULL)
    {
        //split the items
        if(0 != check_config_item(line, &stmpver, &stmpfile, (const char *)chunk_hdr.version))
        {
            ver = calc_version(stmpver);

            if(maxver < ver)
            {
                maxver = ver;
                sver = stmpver;
                sfile = stmpfile;
         
            }
        }
        line = _strsep(&ptr, "\n");
    }

    if (sfile)
    {
        id = MAINCODE_CHUNK_ID;
        sto_get_chunk_header(id, &chunk_hdr);

        #ifdef _GEN_CA_ENABLE_
        snprintf(verbuf, sizeof(verbuf), "%d.%d.%d",chunk_hdr.version[5]-'0',chunk_hdr.version[6]-'0',\
                                chunk_hdr.version[7]-'0');
        #else
        strncpy(verbuf,(const char *)chunk_hdr.version,sizeof(verbuf));
        #endif

        libc_printf("current version = %s\n", chunk_hdr.version);
        libc_printf("upgrade version = %s\n", sver);
        libc_printf("upgrade file = %s\n", sfile);

        UINT8 back_saved;
        win_popup_choice_t choice;
//        int burnflash_ret;
        char tmp[256];

        win_compopup_init(WIN_POPUP_TYPE_OKNO);
#if 0
        sprintf(tmp, "Current firmware version: %s", chunk_hdr.version);
        win_netupg_msg_update(tmp);
        sprintf(tmp, "Newest firmware version: %s", sver);
        win_netupg_msg_update(tmp);
        sprintf(tmp, "Are you sure to download the newest firmware?");
#else
        snprintf(tmp, 256, "Current firmware version: %s\n"
                     "Newest firmware version: %s\n\n"
                     "download the newest one?",
                     verbuf, sver);
#endif
        win_compopup_set_msg(tmp, NULL, 0);
        win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
        choice = win_compopup_open_ext(&back_saved);
        if (choice == WIN_POP_CHOICE_YES)
        {
            upgrade_handle = netupg_download_upgradefile(sfile);
        }
        else
        {
            win_netupg_msg_update("No firmware to be downloaded.");
            download_started = 0;
        }
    }
    else
    {
        win_netupg_msg_update("No valid firmware!");
        download_started = 0;
    }

    return 0;

}

#ifdef _GEN_CA_ENABLE_
#ifdef NETUPG_ONLY_MS_CODE
//signature check

INT32 verify_data_by_chunk(UINT8 key_pos,UINT32 chunk_id)
{
    UINT32 ret = 0;
    UINT32 data_len = 0;
    UINT32 enc_len = 0;
    UINT32 sig_len = 0;
    UINT32 chunk_addr=0;
    UINT8* decrypt_addr = buffer_for_decrypt;
    UINT8* data = NULL;

    //chunk_id = MAIN_CODE_CHUNK_ID;
    chunk_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if (chunk_addr)
    {
        data = (UINT8 *)chunk_addr;
        data_len = fetch_long(data + CHUNK_LENGTH) - CHUNK_HEADER_SIZE + CHUNK_NAME;
        enc_len = ((data_len + 0x1f)&(~0x1f)) + SIG_LEN;
        sig_len= enc_len + CHUNK_HEADER_SIZE;
        MEMCPY(decrypt_addr,data,CHUNK_HEADER_SIZE);
        data += CHUNK_HEADER_SIZE;
        ret = aes_cbc_decrypt_ram_chunk(key_pos, &decrypt_addr[CHUNK_HEADER_SIZE], data, enc_len);
        if ((NUM_ZERO == ret) && (NUM_ZERO != data_len))
            ret = get_code_version(decrypt_addr, sig_len);
    }

    return ret;
}

static int check_upgrade_file(unsigned char* buffer, int buf_len )
{
    int ret = 0;
    UINT8 key_pos = 0;
    UINT32 key_id = 0;
    UINT32 bl_size = 0 ;
    UINT8* data_base = buffer;
    
    if(ALI_S3821==sys_ic_get_chip_id())
    {
        key_id = (NEW_HEAD_ID&NEW_HEAD_MASK)|FIRST_FLASH_KEY;
    }
    else
    {
        key_id = DECRPT_KEY_ID;
    }
    if (decrypt_universal_key(&key_pos, key_id) < NUM_ZERO)
    {
        return 1;
    }
    get_boot_total_area_len(&bl_size);
    if(bl_size >= buf_len)
    {
        return 2;
    }
    data_base +=bl_size;    
    chunk_init((UINT32)data_base, (buf_len-bl_size));
    ret = verify_data_by_chunk(key_pos,MAIN_CODE_CHUNK_ID);
    if(ret<=0)
    {
        return 3;
    }
    ret = verify_data_by_chunk(key_pos,SEE_CODE_CHUNK_ID);
    if(ret<=0)
    {
        return 4;
    }
    return 0;
}
#else
//crc check
static int check_upgrade_file(unsigned char* buffer, int buf_len )
{
    //UINT32 i;
    UINT32 data_id;
    UINT32 data_len;
    UINT32 data_off;
    UINT32 data_crc;
    UINT32 crc;
    UINT32 pointer = 0;
    struct sto_device *flash_dev;

    if (buf_len <= 0)
        return -1;

    if ((flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        return -1;
    }

    get_boot_total_area_len(&pointer);
    do
    {
        data_id = FETCH_LONG((buffer + pointer) + CHUNK_ID);
        data_len = FETCH_LONG((buffer + pointer) + CHUNK_LENGTH);
        data_off = FETCH_LONG((buffer + pointer) + CHUNK_OFFSET);
        data_crc = FETCH_LONG((buffer + pointer) + CHUNK_CRC);

        if ((data_off != 0) && (data_len > data_off))
        {
            libc_printf("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", \
                pointer, data_id, data_len, data_off);
            return 1;
        }
        
        if(data_id == BOOTLOADER_CHUNK_ID)
        {
            pointer += data_off;
            continue;
        }

        if (pointer != 0)
        {
            if (((data_id >> LENGTH_2BYTES) & 0xFF) + ((data_id >> LENGTH_3BYTES) & 0xFF) != 0xFF)
                return 2;
        }

        if (data_off >(UINT32)(flash_dev->totol_size))
        {
            libc_printf("@pointer[%08X] id[%08X] data_off[%08X] > flash size\n", \
                pointer, data_id, data_off);
            return 3;
        }
        
        if (data_crc != NO_CRC)
        {
            crc = mg_table_driven_crc(0xFFFFFFFF, buffer + pointer + 16, data_len);
            if (data_crc != crc)
            {
                libc_printf("@pointer[%08X] id[%08X] crc[%08X] != data_crc[%08X]\n", \
                    pointer, data_id, crc, data_crc);
                return 4;
            }
        }
        pointer += data_off;
        if (pointer + CHUNK_HEADER_SIZE >(UINT32)(flash_dev->totol_size))
        {
            libc_printf("@pointer[%08X] no space for header\n", pointer);
            return 5;
        }

    } while (data_off);

    libc_printf("the upgrade file passed check!\n");
    return 0;
}
#endif
#else
static int check_upgrade_file(unsigned char* buffer, int buf_len )
{
    //UINT32 i;
    UINT32 data_id;
    UINT32 data_len;
    UINT32 data_off;
    UINT32 data_crc;
    UINT32 crc;
    UINT32 pointer = 0;
    struct sto_device *flash_dev;

    if (buf_len <= 0)
        return -1;

    if ((flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        return -1;
    }

    do
    {
        data_id  = FETCH_LONG((buffer + pointer) + CHUNK_ID);
        data_len = FETCH_LONG((buffer + pointer) + CHUNK_LENGTH);
        data_off = FETCH_LONG((buffer + pointer) + CHUNK_OFFSET);
        data_crc = FETCH_LONG((buffer + pointer) + CHUNK_CRC);

        if (pointer != 0)
        {
            if (((data_id >> LENGTH_2BYTES) & 0xFF) + ((data_id >> LENGTH_3BYTES) & 0xFF) != 0xFF)
                return 1;
        }

        if (data_off >(UINT32)(flash_dev->totol_size))
        {
            libc_printf("@pointer[%08X] id[%08X] data_off[%08X] > flash size\n", \
                pointer, data_id, data_off);
            return 2;
        }
        if ((data_off != 0) && (data_len > data_off))
        {
            libc_printf("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", \
                pointer, data_id, data_len, data_off);
            return 3;
        }
        if (data_crc != NO_CRC)
        {
            crc = mg_table_driven_crc(0xFFFFFFFF, buffer + pointer + 16, data_len);
            if (data_crc != crc)
            {
                libc_printf("@pointer[%08X] id[%08X] crc[%08X] != data_crc[%08X]\n", \
                    pointer, data_id, crc, data_crc);
                return 4;
            }
        }
        pointer += data_off;
        if (pointer + CHUNK_HEADER_SIZE >(UINT32)(flash_dev->totol_size))
        {
            libc_printf("@pointer[%08X] no space for header\n", pointer);
            return 5;
        }

    } while (data_off);

    libc_printf("the upgrade file passed check!\n");
    return 0;
}
#endif


static void netupg_start()
{
    INT32 vscr_idx = osal_task_get_current_id();
    LPVSCR ap_vscr = osd_get_task_vscr(vscr_idx);

    // get current upgrade parmeters.
    win_get_netupg_param();

    // save current setting
    set_remote_ip_cfg(&net_remote_cfg);

    win_netupg_process_update(0);
    win_netupg_msg_clear();
    osd_update_vscr(ap_vscr);

    IP_LOC_CFG local_cfg;
    libnet_get_ipconfig(&local_cfg);
    if ((ntohl(local_cfg.ip_addr) & 0xFF000000) == 0)
    {
        win_netupg_msg_update("Invalid local IP address!");
        osd_update_vscr(ap_vscr);
        download_started = 0;
        return;
    }

    if (net_remote_cfg.url_type == 0 &&
        (ntohl(net_remote_cfg.url_int) & 0xFF000000) == 0)
    {
        win_netupg_msg_update("Invalid server IP address!");
        osd_update_vscr(ap_vscr);
        osd_container_chg_focus(&g_win_netupg, IDC_NETUPG_URL,
                              C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
        download_started = 0;
        return;
    }

    /* download config file */
    upgrade_handle = netupg_download_configfile(); /* it will download upgrade file */
    return;

#if 0
    // ...
    MEMCPY(&lib_nuc.local_cfg, &local_cfg, sizeof(local_cfg));
    MEMCPY(&lib_nuc.rmt_cfg, &net_remote_cfg, sizeof(net_remote_cfg));
    lib_nuc.pfn_ui_connect = net_ui_callback;
    lib_nuc.pfn_user_stop = net_user_stop;
    lib_nuc.file_addr = __MM_PVR_VOB_BUFFER_ADDR;
    lib_nuc.file_len = 0x400000;
    lib_net_upg_start(&lib_nuc);
#endif
}

static void net_mi_callback(UINT8* string)
{
}
/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION netupg_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT netupg_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_POST_CHANGE:
        if(IDC_NETUPG_URL_TYPE == b_id)
        {
            net_remote_cfg.url_type = param1;
            // change the display of url;
            netupg_set_url(TRUE);
        }
        else if (IDC_NETUPG_TYPE == b_id)
        {
            netupg_sel_protocol(param1, TRUE);
            net_remote_cfg.protocol_type = param1;
        }
        break;
    default:
        break;
    }
    return ret;
}

static VACTION netupg_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;
    switch(key)
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

static PRESULT netupg_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
    UINT8 b_id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1 >> 16);
        if ((VACT_ABORT == unact  ) && (1 == download_started ))
        {
            libnet_download_abort(upgrade_handle);
            upgrade_handle = 0;
            download_started = 0;
            win_netupg_msg_update("User abort!");
        }
        if ((IDC_NETUPG_START == b_id ) &&(VACT_ENTER == unact ) && (0 == download_started ))
        {
            download_started = 1;
            netupg_start();
        }
        else if(VACT_BLUE == unact)
        {
            download_started = 1;
            network_performance_test = 1;
            netupg_start();
            network_performance_test =0;
        }

        break;
    default:
        break;
    }
    return ret;
}

static VACTION  netupg_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_EDIT_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_EDIT_RIGHT;
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

static PRESULT  netupg_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 b_id;
    //UINT8 status;
    EDIT_FIELD* edf;
    UINT8 back_save;
    //UINT32 value;

    b_id = osd_get_obj_id(p_obj);

    edf = (EDIT_FIELD*)p_obj;
    //libc_printf("pObj=%xh, event=%x, param1=%x, param2=%x\n",pObj, event, param1, param2);

    switch(event)
    {
    case EVN_KEY_GOT:
        if(((b_id == IDC_NETUPG_URL )&& (net_remote_cfg.url_type != 0)) ||
            (b_id == IDC_NETUPG_USER) ||
            (b_id == IDC_NETUPG_PASSWORD))
            ret = mobile_input_proc(edf,(VACTION)(param1>>16), param1 & 0xFFFF, param2);
        break;

    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        //libc_printf("POST: ip %xh, get value = %xh\n",param1,OSD_GetEditFieldContent((PEDIT_FIELD)pObj));
        break;

    case EVN_FOCUS_PRE_GET:
    case EVN_PARENT_FOCUS_PRE_GET:
        netupg_mobile_input_init(b_id);
        break;

    case EVN_FOCUS_PRE_LOSE:
    case EVN_PARENT_FOCUS_PRE_LOSE:

        //netupg_mobile_input_init(bID);
        break;

    case EVN_DATA_INVALID:
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg("Invalid address number!", NULL, 0);
        win_compopup_open_ext(&back_save);
        ret = PROC_LOOP;
        break;

    }

    return ret;
}

static VACTION netupg_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

    if (1 == download_started )
    {
        act = VACT_PASS;
    }

    return act;
}

static PRESULT netupg_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //VACTION unact;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_netupg,RS_NET_UPGRADE, 0);
        netupg_init();
        win_netupg_msg_clear();
        download_started = 0;
        upgrade_handle = 0;
        break;
    case EVN_POST_OPEN:
        wincom_open_help(p_obj,netupg_helpinfo , ARRAY_SIZE(netupg_helpinfo));
        break;
    case EVN_PRE_CLOSE:
        win_get_netupg_param();
        set_remote_ip_cfg(&net_remote_cfg);
        sys_data_save(1);
        wincom_close_help();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        netupg_block_free();
        break;
    case EVN_MSG_GOT:
        ret = netupg_msg_proc(param1,param2);
        break;
    default:
        break;
    }

    return ret;
}


/*******************************************************************************
*   other functions  definition
*******************************************************************************/

#ifdef CI_PLUS_SUPPORT
static BOOL netupg_allcode_check(UINT8 *pbuff, UINT32 length)
{
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id = 0xFFFFFFFF;
    CHUNK_HEADER chunk_header;
    UINT8 *data_addr = NULL;
    UINT32 data_len = 0;
    BOOL ret = TRUE;

    if ((NULL == pbuff) ||( 0 == length))
    {
        ASSERT(0);
        return FALSE;
    }

    chunk_init(pbuff, length);
    // step 2: maincode check
    // get maincode raw data address & length
    chunk_id = MAINCODE_ID;
    if(!get_chunk_header(chunk_id, &chunk_header))
    {
        ret = FALSE;
        return ret;
    }
    data_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    data_len = chunk_header.len - CHUNK_HEADER_SIZE + CHUNK_NAME;

    if (0 != test_rsa_ram((UINT32)data_addr, data_len)) // this function operate RAW Data and RAW length
    {
        ret = FALSE;
        return ret;
    }

    // step 3: seecode check
    // get raw data address & length from memory
    chunk_id = SEECODE_ID;
    if (!get_chunk_header(chunk_id, &chunk_header))
    {
        ret = FALSE;
        return ret;
    }
    data_addr = chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    data_len = chunk_header.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
    if (0 != test_rsa_ram((UINT32)data_addr, data_len))
    {
        ret = FALSE;
        return ret;
    }

    return ret;
}
#endif

static void win_netupg_process_ms_update(UINT32 process,UINT32 mode)
{    
    static UINT32  base = 0;
    const UINT8    up_cnt = 2;
    
    if(process > PERCENT_MAX)
        process = 0;
    if(mode == 0)
    {
        if(process < 100)
        {
            process = base + process/up_cnt;
        }
        else
        {
            base += 100/up_cnt;
            process = base;
        }        
    }
    osd_set_progress_bar_pos(&netupg_bar_progress,(INT16)process);
    osd_set_text_field_content(&netupg_txt_progress, STRING_NUM_PERCENT, (UINT32)process);

    osd_draw_object( (POBJECT_HEAD)&netupg_bar_progress, C_UPDATE_ALL);
    osd_draw_object( (POBJECT_HEAD)&netupg_txt_progress, C_UPDATE_ALL);
}

RET_CODE netupg_burn_by_chunk(UINT32 chunk_id,UINT8 *buffer, UINT32 len,UINT8 *move)
{
    RET_CODE ret = SUCCESS;
    UINT32 f_chunk_addr = 0;
    UINT32 f_chunk_offset = 0;
    UINT32 sector = 0;
    UINT32 burn_len = 0 ;

    UINT32 m_chunk_addr = 0;
    UINT32 m_chunk_offset = 0;

    f_chunk_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    f_chunk_offset = sto_fetch_long((UINT32)f_chunk_addr + CHUNK_OFFSET);

    m_chunk_addr = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    m_chunk_offset=fetch_long((UINT8*)(m_chunk_addr + CHUNK_OFFSET));

    netupg_set_process_callback(win_netupg_process_ms_update);
    if(m_chunk_offset == f_chunk_offset)
    {
        sector = f_chunk_addr/C_SECTOR_SIZE;
        ret = netupg_ms_burn_by_chunk(sector,(UINT8*)m_chunk_addr, m_chunk_offset, 0);
    }
    else
    {
        if(NULL != move)
            *move = 1;

        sector = f_chunk_addr/C_SECTOR_SIZE;
        burn_len = len - (m_chunk_addr - (UINT32)buffer);
        burn_len = (burn_len + C_SECTOR_SIZE) /C_SECTOR_SIZE * C_SECTOR_SIZE;
        ret = netupg_ms_burn_by_chunk(sector,(UINT8*)m_chunk_addr, burn_len, 1);
    }    

    return ret;
}

static RET_CODE netupg_ms_burn(UINT8 *buffer, UINT32 len)
{
    RET_CODE ret = SUCCESS;
    UINT32 bl_size = 0;
    UINT8 *data_base = buffer;
    UINT32 data_len = len;
    UINT8 bmove = 0;
    
    get_boot_total_area_len(&bl_size);
    if(bl_size >= len)
    {
        return 1;
    }
    data_base +=bl_size; 
    data_len -= bl_size;
    chunk_init((UINT32)data_base, data_len);

    ret = netupg_burn_by_chunk(MAIN_CODE_CHUNK_ID,data_base,data_len,&bmove);
    if(!bmove)
    {
        ret = netupg_burn_by_chunk(SEE_CODE_CHUNK_ID,data_base,data_len,&bmove);
    }

    return ret;
}

static PRESULT netupg_msg_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    //libc_printf("Enter %s....\n",__FUNCTION__);
    char msg[128];

    INT32 vscr_idx = osal_task_get_current_id();
    LPVSCR ap_vscr = osd_get_task_vscr(vscr_idx);
    int valid_upgrade_file = 0;

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_SCAN:
        osd_set_progress_bar_pos(&netupg_bar_progress, msg_code);
        osd_draw_object((POBJECT_HEAD)&netupg_bar_progress,C_UPDATE_ALL);
        if( PERCENT_MAX == msg_code)
        {
            UINT8 back_saved;
            win_popup_choice_t choice;
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg(NULL, NULL, RS_USBUPG_SURE_BURN_FLASH);
            choice = win_compopup_open_ext(&back_saved);
            if(choice == WIN_POP_CHOICE_YES)
            {
                //burning flash
                msg_code = 0;
                while(1)
                {
                    msg_code+=5;
                    if(msg_code >= PERCENT_MAX)
                        break;
                    osd_set_progress_bar_pos(&netupg_bar_progress, msg_code);
                    osd_draw_object((POBJECT_HEAD)&netupg_bar_progress,C_UPDATE_ALL);
                    osal_task_sleep(300);
                }

            }

        }
        break;

    case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_START:
        win_netupg_process_update(0);
        net_file_length = msg_code;
        snprintf(msg, 128, "file length = %d bytes", (int)msg_code);
        win_netupg_msg_update(msg);
        osd_update_vscr(ap_vscr);
        break;

    case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_PROGRESS:
        if (net_file_length == 0)
        {
            win_netupg_process_update(100);
        }
        else
        {
            win_netupg_process_update(msg_code * 100 / net_file_length);
        }
        osd_update_vscr(ap_vscr);
        break;

    case CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH:
        upgrade_handle = 0;
        if((int)msg_code >= 0)
        {
            int bar_pos = osd_get_progress_bar_pos(&netupg_bar_progress);
            if (PERCENT_MAX== bar_pos)
            {
                snprintf(msg, 128,"download finished! length = %d bytes", (int)msg_code);
            }
            else
            {
                snprintf(msg, 128,"download abort! length = %d bytes", (int)msg_code);
            }
            win_netupg_msg_update(msg);
            osd_update_vscr(ap_vscr);

            unsigned char *  __MAYBE_UNUSED__ buffer;
            if(net_file_type == 0)
                buffer = buffer_for_cfgfile;
            else
                buffer = buffer_for_upgradefile;

            /*
            unsigned char md5[17];
            int i;
            md5_do_data(md5, buffer, msg_code);

            for(i = 0; i < 16; i++)
            {
                libc_printf("%x", md5[i]);
            }
            libc_printf("\n");
            */

            if(net_file_type == 0)
            {
                if (PERCENT_MAX == bar_pos )
                {
                    buffer_for_cfgfile[msg_code] = 0;
                    win_netupg_process_update(0);
                    win_netupg_msg_clear();
                    osd_update_vscr(ap_vscr);

                    win_netupg_msg_update("Checking configuration file..");
                    check_config_file((char *)buffer_for_cfgfile, msg_code);
                    osd_update_vscr(ap_vscr);
                }
                else
                {
                    download_started = 0;
                }
            }
            else
            {
                libc_printf("check upgrade file!\n");
                if (PERCENT_MAX == bar_pos )
                {
                    if (check_upgrade_file(buffer_for_upgradefile, msg_code) == 0)
                    {
                        valid_upgrade_file = 1;
                    }
                    else
                    {
                        win_netupg_msg_update("Invalid firmware!");
                        osd_update_vscr(ap_vscr);
                        download_started = 0;
                    }
                }
                else
                {
                    download_started = 0;
                }
            }
        }
        else
        {
            switch (-msg_code)
            {
                case NET_ERR_CONNECT_FAILED:
                    strncpy(msg, "Connect to server failed!", 127);
                    break;
                case NET_ERR_USER_ABORT:
                    strncpy(msg, "User abort!", 127);
                    break;
                case NET_ERR_LOGIN_FAILED:
                    strncpy(msg, "Invalid user or password, login failed!", 127);
                    break;
                /*
                case NET_ERR_FILE_NOT_FOUND:
                case NET_ERR_HTTP_SERVER_ERROR:
                case NET_ERR_FTP_SERVER_ERROR:
                case NET_ERR_OPERATION_NOT_PERMIT:
                case NET_ERR_FILE_SAVE_ERROR:
                */
                default:
                    strncpy(msg, "Download failed!", 127);
                    break;
            }
            win_netupg_msg_update(msg);
            osd_update_vscr(ap_vscr);
            download_started = 0;
        }

        /* update firmware */
        if (valid_upgrade_file)
        {
            UINT8 back_saved;
            win_popup_choice_t choice;
            int burnflash_ret = -1;

            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Are you sure to burn the firmware?", NULL, 0);
            win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            choice = win_compopup_open_ext(&back_saved);
            if (choice == WIN_POP_CHOICE_YES)
            {
                win_get_netupg_param();
                set_remote_ip_cfg(&net_remote_cfg);
                sys_data_save(1);

                win_netupg_process_update(0);
                win_netupg_msg_update("Burning flash, don't power off.");
                osd_update_vscr(ap_vscr);

                ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_POOLING);
              //  osal_task_dispatch_off();
                sto_flash_disable_protect();
                netupg_set_upgrade_bloader_flag(1);
#ifdef _GEN_CA_ENABLE_
                burnflash_ret = netupg_ms_burn(buffer_for_upgradefile, msg_code);
#else
#ifdef CI_PLUS_SUPPORT
                if (TRUE == netupg_allcode_check(buffer_for_upgradefile, msg_code))
                {
                    netupg_set_upgrade_bloader_flag(0); // skip bootloader
#endif
                    burnflash_ret = netupg_allcode_burn(buffer_for_upgradefile, msg_code, win_netupg_process_update);
#ifdef CI_PLUS_SUPPORT
                }
#endif
#endif
                osd_update_vscr(ap_vscr);
         //       osal_task_dispatch_on();
                ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_INTERRUPT);

                if(burnflash_ret == 0)
                {
                    download_started = 0;
                    key_pan_display("OFF ",4);
                    osal_task_sleep(500);
                    power_off_process(0);

                    sys_data_factroy_init();
                    sys_data_set_factory_reset(TRUE);
                    sys_data_set_cur_chan_mode(TV_CHAN);

                    power_on_process();
                }
                else
                {
                    download_started = 0;
                    win_netupg_msg_update("Burning flash error!");
                    osd_update_vscr(ap_vscr);
                }
            }
            else
            {
                download_started = 0;
            }
        }
        break;
    }

    //libc_printf("Exit %s....\n",__FUNCTION__);

    return ret;
}

#endif /* NETWORK_SUPPORT */

#ifdef _INVW_JUICE
int inview_oadupgrade_flashwrite(unsigned char* buffer, int buf_len)
{
    int burnflash_ret;
        ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_POOLING);
      //  osal_task_dispatch_off();
        sto_flash_disable_protect();
        netupg_set_upgrade_bloader_flag(1);
        //added 0 to disable progress bar display
        burnflash_ret = netupg_allcode_burn(buffer, buf_len, win_netupg_process_update);

 //       osal_task_dispatch_on();
        ge_io_ctrl(g_ge_dev, GE_IO_SET_SYNC_MODE, GE_SYNC_MODE_INTERRUPT);

        if(burnflash_ret == 0)
        {
            download_started = 0;
            key_pan_display("OFF ",4);
            osal_task_sleep(500);
            power_off_process();
            sys_data_factroy_init();
            sys_data_set_factory_reset(TRUE);
            sys_data_set_cur_chan_mode(TV_CHAN);
            power_on_process();
            return 1;
        }
        else
        {
            printf("BURNING FLASH - OAD UPRGADE FAILED - SORRY - ERROR %d\n", burnflash_ret);
            return 0;
        }

}
#endif
