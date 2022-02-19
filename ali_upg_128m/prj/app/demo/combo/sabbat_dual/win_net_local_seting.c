 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_net_local_setting.c
*
*    Description:   The setting of local network environment
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

#include <hld/net/net.h>
#include <api/libtcpip/lwip/inet.h>
#include <api/libtcpip/lwip/sockets.h>
#include <api/libtcpip/lwip/netdb.h>
#include <api/libtcpip/lwip/err.h>
#include <api/libtcpip/lwip/netif.h>
#include <api/libnet/libnet.h>
#ifdef SAT2IP_SUPPORT
#include "sat2ip/sat2ip_upnp.h"
#endif
#include "win_net_local_seting.h"


/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION localset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT localset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION  localset_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT  localset_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION localset_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT localset_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION localset_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT localset_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static UINT16 win_com_onoff_ids[];

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

#define TXT_L_OF    TXTN_L_OF
#define TXT_W       TXTN_W
#define TXT_H       TXTN_H
#define TXT_T_OF    TXTN_T_OF

#define SEL_L_OF    TXTS_L_OF
#define SEL_W       TXTS_W
#define SEL_H       TXTS_H
#define SEL_T_OF    TXTS_T_OF

enum
{
    IDC_LOCALSET_DHCP = 1,
    IDC_LOCALSET_IP_ADDR,
    IDC_LOCALSET_SUBNET_MASK,
    IDC_LOCALSET_GATEWAY,
    IDC_LOCALSET_DNS1,
    IDC_LOCALSET_DNS2,
    IDC_LOCALSET_APPLY,
};

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    localset_item_con_keymap,localset_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,pre,sub,str)    \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    localset_item_edf_keymap,localset_item_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    localset_item_sel_keymap,localset_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL/*&varLine*/ ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cursormode,\
                        pat,pre,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_SEL_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_TXT_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL/*&varLine*/,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    localset_con_keymap,localset_con_callback,  \
    nxt_obj, focus_id,0)

static char ip_pattern[] = "i00";

LDEF_SEL_MENU_ITEM(g_win_localsetting,localset_item_con1, &localset_item_con2,localset_item_txtname1,\
                localset_item_dhcp,localset_item_line1,1,7,2,CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H, \
                RS_NET_DHCP,STRING_ID,0,2,win_com_onoff_ids)

LDEF_EDIT_MENU_ITEM(g_win_localsetting,localset_item_con2, &localset_item_con3,localset_item_txtname2,\
                localset_item_ip_addr,localset_item_line2,2,1,3,CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H, \
                RS_NET_IP_ADDR,NORMAL_EDIT_MODE,CURSOR_NORMAL,ip_pattern,NULL,NULL,display_strs[0])

LDEF_EDIT_MENU_ITEM(g_win_localsetting,localset_item_con3, &localset_item_con4,localset_item_txtname3,\
                localset_item_subnet_mask,localset_item_line3,3,2,4,CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H, \
                RS_NET_SUBNET_MASK, NORMAL_EDIT_MODE,CURSOR_NORMAL,ip_pattern,NULL,NULL,display_strs[1])

LDEF_EDIT_MENU_ITEM(g_win_localsetting,localset_item_con4, &localset_item_con5,localset_item_txtname4,\
                localset_item_gateway,localset_item_line4,4,3,5,CON_L, CON_T + (CON_H + CON_GAP)*3,CON_W,CON_H,\
                RS_NET_GATEWAY,NORMAL_EDIT_MODE,CURSOR_NORMAL,ip_pattern,NULL,NULL,display_strs[2])

LDEF_EDIT_MENU_ITEM(g_win_localsetting,localset_item_con5, &localset_item_con6,localset_item_txtname5,\
                localset_item_dns1,localset_item_line5,5,4,6,CON_L, CON_T + (CON_H + CON_GAP)*4,CON_W,CON_H, \
                RS_NET_DNS_PREFERRED,NORMAL_EDIT_MODE,CURSOR_NORMAL,ip_pattern,NULL,NULL,display_strs[3])

LDEF_EDIT_MENU_ITEM(g_win_localsetting,localset_item_con6, &localset_item_con7,localset_item_txtname6,\
                localset_item_dns2,localset_item_line6,6,5,7, CON_L, CON_T + (CON_H + CON_GAP)*5,CON_W,CON_H, \
                RS_NET_DNS_ALTERNATE,NORMAL_EDIT_MODE,CURSOR_NORMAL,ip_pattern,NULL,NULL,display_strs[4])

LDEF_TXT_MENU_ITEM(g_win_localsetting,localset_item_con7, NULL,localset_item_txtname7,localset_item_line7,7,6,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*6,CON_W,CON_H, RS_NET_APPLY)

LDEF_WIN(g_win_localsetting,&localset_item_con1,W_L, W_T, W_W, W_H, 1)


/*******************************************************************************
*   Local functions & variables define
*******************************************************************************/
static UINT16 win_com_onoff_ids[] =
{
    RS_COMMON_OFF,
    RS_COMMON_ON,
};

static PCONTAINER localset_cons[] =
{
    &localset_item_con2,
    &localset_item_con3,
    &localset_item_con4,
    &localset_item_con5,
    &localset_item_con6
};

static IP_LOC_CFG net_local_cfg;
#define VACT_CAPTURE VACT_PASS+1
#define VACT_PING   VACT_PASS+2
static UINT8 g_netif_pcap = 0;

extern void get_local_ip_cfg(PIP_LOC_CFG pcfg);
extern struct netif *netif_find_by_type(int dev_type);
extern void set_local_ip_cfg(PIP_LOC_CFG pcfg);
extern char *netif_pcap(struct netif *netif, char *dir, UINT8 start);

static void localset_hdcp_on(BOOL update, UINT32 on)
{
    UINT8 action;
    UINT8 i;

    action = on? C_ATTR_INACTIVE : C_ATTR_ACTIVE;

    if( action == C_ATTR_INACTIVE )
    {
        UINT8 b_id = g_win_localsetting.focus_object_id;
        if((b_id != IDC_LOCALSET_APPLY) && (b_id != IDC_LOCALSET_DHCP))
        {
            g_win_localsetting.focus_object_id = IDC_LOCALSET_DHCP;
        }
    }

    for(i = 0; i < ARRAY_SIZE(localset_cons); i++)
    {
        set_container_active(localset_cons[i], action);
        if(update)
            osd_draw_object( (POBJECT_HEAD)localset_cons[i], C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
    }
}


static void localset_init()
{
    get_local_ip_cfg(&net_local_cfg);
//  libc_printf("start dhcp: %d\n", net_local_cfg.dhcp_on);
    osd_set_multisel_sel(&localset_item_dhcp, net_local_cfg.dhcp_on);
    localset_hdcp_on(FALSE, net_local_cfg.dhcp_on);
    osd_set_edit_field_content(&localset_item_ip_addr, STRING_NUMBER,
                            ntohl(net_local_cfg.ip_addr));
    osd_set_edit_field_content(&localset_item_subnet_mask, STRING_NUMBER,
                            ntohl(net_local_cfg.subnet_mask));
    osd_set_edit_field_content(&localset_item_gateway, STRING_NUMBER,
                            ntohl(net_local_cfg.gateway));
    osd_set_edit_field_content(&localset_item_dns1, STRING_NUMBER,
                            ntohl(net_local_cfg.dns));
    osd_set_edit_field_content(&localset_item_dns2, STRING_NUMBER,
                            ntohl(net_local_cfg.dns2));

    /*because only support one net interface in same time.*/
    do
    {
        if(netif_find_by_type(HLD_DEV_TYPE_3G))
        {
            set_container_active(&localset_item_con7, C_ATTR_INACTIVE);
        }
        else
        {
            set_container_active(&localset_item_con7, C_ATTR_ACTIVE);
        }
    }while(0);
    /******************************************************/

}

static void localset_update()
{
    get_local_ip_cfg(&net_local_cfg);
//  libc_printf("start dhcp: %d\n", net_local_cfg.dhcp_on);
    osd_set_multisel_sel(&localset_item_dhcp, net_local_cfg.dhcp_on);
    localset_hdcp_on(FALSE, net_local_cfg.dhcp_on);
    osd_set_edit_field_content(&localset_item_ip_addr, STRING_NUMBER,
                            ntohl(net_local_cfg.ip_addr));
    osd_draw_object((POBJECT_HEAD)&localset_item_ip_addr, C_UPDATE_ALL);

    osd_set_edit_field_content(&localset_item_subnet_mask, STRING_NUMBER,
                            ntohl(net_local_cfg.subnet_mask));
    osd_draw_object((POBJECT_HEAD)&localset_item_subnet_mask, C_UPDATE_ALL);

    osd_set_edit_field_content(&localset_item_gateway, STRING_NUMBER,
                            ntohl(net_local_cfg.gateway));
    osd_draw_object((POBJECT_HEAD)&localset_item_gateway, C_UPDATE_ALL);

    osd_set_edit_field_content(&localset_item_dns1, STRING_NUMBER,
                            ntohl(net_local_cfg.dns));
    osd_draw_object((POBJECT_HEAD)&localset_item_dns1, C_UPDATE_ALL);

    osd_set_edit_field_content(&localset_item_dns2, STRING_NUMBER,
                            ntohl(net_local_cfg.dns2));
    osd_draw_object((POBJECT_HEAD)&localset_item_dns2, C_UPDATE_ALL);

}

typedef enum
{
    LOCAL_CFG_ERR_NONE = 0,
    LOCAL_CFG_ERR_IP,
    LOCAL_CFG_ERR_SUBNET,
    LOCAL_CFG_ERR_IP_SUBNET,
    LOCAL_CFG_ERR_GATEWAY,
    LOCAL_CFG_ERR_DNS_PREFER,
    LOCAL_CFG_ERR_DNS_ALTER,
    LOCAL_CFG_ERR_MAX /* MUST be the last one */
} LOCAL_CFG_ERR_CODE;

void win_localset_popup_display(UINT8 *string, UINT32 show_ticks)
{
    UINT8 back_saved;
    win_msg_popup_close();
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg((char *)string, NULL, 0);
    win_compopup_open_ext(&back_saved);
    osal_task_sleep(show_ticks);
    win_compopup_smsg_restoreback();
}

static LOCAL_CFG_ERR_CODE localset_cfg_save(void)
{
    IP_LOC_CFG last_local_cfg;

    /* check address validity */
    if (net_local_cfg.dhcp_on == 0)
    {
        if ((ntohl(net_local_cfg.ip_addr) & 0xFF000000) == 0)
            return LOCAL_CFG_ERR_IP;

        if ((ntohl(net_local_cfg.subnet_mask) & 0xFF000000) == 0)
            return LOCAL_CFG_ERR_SUBNET;

        if (ntohl(net_local_cfg.ip_addr & (~net_local_cfg.subnet_mask)) == 0)
            return LOCAL_CFG_ERR_IP_SUBNET;

        if ((ntohl(net_local_cfg.gateway) & 0xFF000000) == 0)
            return LOCAL_CFG_ERR_GATEWAY;

        if ((ntohl(net_local_cfg.dns) & 0xFF000000) == 0)
            return LOCAL_CFG_ERR_DNS_PREFER;

        //if ((ntohl(net_local_cfg.dns2) & 0xFF000000) == 0)
        //  return LOCAL_CFG_ERR_DNS_ALTER;
    }

    get_local_ip_cfg(&last_local_cfg);
    if (/*(0 == libnet_dhcp_get_status()) &&*/ (1 == net_local_cfg.dhcp_on))
    {
        libc_printf("set DHCP on!\n");
        libnet_dhcp_on();
    }
    else if (/*(1 == last_local_cfg.dhcp_on ) && */(0 == net_local_cfg.dhcp_on ))
    {
        libc_printf("set DHCP off!\n");
        libnet_dhcp_off();
    }

    if (0 == net_local_cfg.dhcp_on )
    {
        libc_printf("set local IP\n");
        libnet_set_ipconfig(&net_local_cfg);
    }
    set_local_ip_cfg(&net_local_cfg);
    sys_data_save(1);
#ifdef SAT2IP_SUPPORT
    if (0 == net_local_cfg.dhcp_on )
    {
        ap_upnp_network_update();
    }
#endif
#ifdef DLNA_SUPPORT
    if(0 == net_local_cfg.dhcp_on )
    {
        dlnaNetIfAddNotify();
        dlnaNetAddressAddNotify();
    }
#endif

//  libc_printf("save dhcp: %d\n", net_local_cfg.dhcp_on);
    return LOCAL_CFG_ERR_NONE;
}

/*******************************************************************************
*   key mapping and event callback definition
*******************************************************************************/
static VACTION localset_item_sel_keymap(POBJECT_HEAD p_obj, UINT32 key)
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

static PRESULT localset_item_sel_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch(event)
    {
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        net_local_cfg.dhcp_on = param1;
        localset_hdcp_on(TRUE, param1);
        break;
    case EVN_REQUEST_STRING:
        break;

    case EVN_UNKNOWN_ACTION:

        break;
    }
    return ret;
}

static VACTION localset_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;
    switch (key)
    {
        case V_KEY_ENTER:
            act = VACT_ENTER;
            break;
        default:
            act = VACT_PASS;
            break;
    }
    return act;
}

static PRESULT localset_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
    UINT8 b_id = osd_get_obj_id(p_obj);

    switch( event)
    {
        case EVN_UNKNOWN_ACTION:
            unact = (VACTION)(param1 >> 16);
            if ((b_id == IDC_LOCALSET_APPLY) && (unact == VACT_ENTER))
            {
                char hint[64];
                UINT8 b_new_focus_id = IDC_LOCALSET_APPLY, back_saved;
                LOCAL_CFG_ERR_CODE err_code = localset_cfg_save();
                switch (err_code)
                {
                    case LOCAL_CFG_ERR_NONE:
                        strncpy(hint, "Apply network local setting...", 63);
                        b_new_focus_id = IDC_LOCALSET_APPLY;
                        break;
                    case LOCAL_CFG_ERR_IP:
                        strncpy(hint, "Invalid IP address", 63);
                        b_new_focus_id = IDC_LOCALSET_IP_ADDR;
                        break;
                    case LOCAL_CFG_ERR_SUBNET:
                        strncpy(hint, "Invalid subnet mask", 63);
                        b_new_focus_id = IDC_LOCALSET_SUBNET_MASK;
                        break;
                    case LOCAL_CFG_ERR_IP_SUBNET:
                        strncpy(hint, "Invalid IP address or subnet mask",63);
                        b_new_focus_id = IDC_LOCALSET_IP_ADDR;
                        break;
                    case LOCAL_CFG_ERR_GATEWAY:
                        strncpy(hint, "Invalid gateway", 63);
                        b_new_focus_id = IDC_LOCALSET_GATEWAY;
                        break;
                    case LOCAL_CFG_ERR_DNS_PREFER:
                        strncpy(hint, "Invalid preferred DNS", 63);
                        b_new_focus_id = IDC_LOCALSET_DNS1;
                        break;
                    case LOCAL_CFG_ERR_DNS_ALTER:
                        strncpy(hint, "Invalid alternate DNS", 63);
                        b_new_focus_id = IDC_LOCALSET_DNS2;
                        break;
                    default:
                        err_code = LOCAL_CFG_ERR_MAX; /* don't show anything */
                        break;
                }

                if (err_code < LOCAL_CFG_ERR_MAX)
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg(hint, NULL, 0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(1000);
                    win_compopup_smsg_restoreback();
                    osd_container_chg_focus(&g_win_localsetting, b_new_focus_id,
                                          C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

static VACTION  localset_item_edf_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    default:
        act = VACT_PASS;
    }

    return act;
}

static PRESULT  localset_item_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 b_id;
    //UINT8 status;
    EDIT_FIELD*  __MAYBE_UNUSED__ edf;
    UINT8 back_save;
    UINT32 value;
    //char tmp_str[32];

    b_id = osd_get_obj_id(p_obj);

    edf = (EDIT_FIELD*)p_obj;

    switch(event)
    {
    case EVN_PRE_CHANGE:

        break;
    case EVN_POST_CHANGE:
        value = osd_get_edit_field_content((PEDIT_FIELD)p_obj);
        switch(b_id)
        {
            case IDC_LOCALSET_IP_ADDR:
                net_local_cfg.ip_addr = htonl(value);
                break;
            case IDC_LOCALSET_SUBNET_MASK:
                net_local_cfg.subnet_mask = htonl(value);
                break;
            case IDC_LOCALSET_GATEWAY:
                net_local_cfg.gateway = htonl(value);
                break;
            case IDC_LOCALSET_DNS1:
                net_local_cfg.dns = htonl(value);
                break;
            case IDC_LOCALSET_DNS2:
                net_local_cfg.dns2 = htonl(value);
                break;
        }
        //libc_printf("POST: ip %xh, get value = %xh\n",param1,value);
        break;
    case EVN_PARENT_FOCUS_POST_LOSE:
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

static VACTION localset_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    case V_KEY_RED:
        act = VACT_CAPTURE;
        break;
    case V_KEY_BLUE:
        act = VACT_PING;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}


static PRESULT  localset_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    UINT8 back_save;

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_DHCP:
        if (msg_code == (UINT32)NET_ERR_DHCP_SUCCESS)
        {
            localset_update();
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg("DHCP On OK.", NULL, 0);
            win_compopup_open_ext(&back_save);
        }
        else
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg("DHCP On fail.", NULL, 0);
            win_compopup_open_ext(&back_save);
        }

    }

    return ret;
}


static PRESULT localset_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact;
    //IP_LOC_CFG last_local_cfg;

    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_localsetting,RS_NET_LOCAL_SETTING, 0);
        localset_init();
        break;
    case EVN_PRE_CLOSE:
        //localset_cfg_save();
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1 >> 16);
        if(unact == VACT_CAPTURE)
        {
            char *path;
            char msg[128];
            struct netif *netif = netif_find("en0");
            if(netif == NULL)
                break;
            g_netif_pcap = (netif && netif->pcap);
            g_netif_pcap = !g_netif_pcap;
            path = netif_pcap(netif, "/mnt/uda1", g_netif_pcap);
            g_netif_pcap = (path == NULL ? 0 : 1);
            MEMSET(msg, 0, 128);
            snprintf(msg, 128,"%s capture data %s", g_netif_pcap ? "Start" : "Stop", g_netif_pcap ? path : "");
            win_localset_popup_display((UINT8 *)msg, 2000);
            break;
        }
		#if 0
        else if( unact == VACT_PING)
        {
            lwip_ping(inet_ntoa(net_local_cfg.dns), 64, 10);
            lwip_ping("www.baidu.com", 64, 10);
        }
		#endif
        break;
    case EVN_MSG_GOT:
        ret = localset_message_proc(param1,param2);
        break;
    default:
        break;
    }

    return ret;
}


/*******************************************************************************
*   other functions  definition
*******************************************************************************/

#endif /* NETWORK_SUPPORT */
