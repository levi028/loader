/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: ctrl_network.c
 *
 *    Description: This source file is application network relate process.
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
#include <hld/ge/ge.h>
#include <api/libnet/libnet.h>
#include <hld/net/net.h>
#include "copper_common/com_api.h"
#include <api/libtcpip/ali_network.h>
#include "control.h"
#include "usb_tpl.h"
#include "menus_root.h"
#include "ctrl_network.h"
#include "win_com_popup.h"

#ifdef NETWORK_SUPPORT
#include <api/libnic/lib_nic.h>
#include "win_net_choose.h"
#endif
#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#endif

#ifdef _INVW_JUICE
extern  inview_wifi_message(int type);
#endif

#ifdef NETWORK_SUPPORT

#ifdef _MHEG5_IC_ICS_SUPPORT_
void ap_network_connect_status(void)
{
    struct net_device *ndev = NULL;
    char linked = -1;
    static int32_t prevStatus=0;
    ndev = dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
    if (ndev == NULL)
    {
        libc_printf("net_device: Can not find net device!\n");
    }
    ndev->get_info(ndev, NET_GET_LINK_STATUS, &linked);
    if ((NET_LINK_CONNECTED == linked) && (prevStatus != 0))
    {
    	prevStatus = 0;
        MHEG_IC_GLUE_ICStatusChangeNotify(0, NULL);
    }
    else if  ((NET_LINK_CONNECTED != linked) && (prevStatus != 1))
    {
    	prevStatus = 1;
        MHEG_IC_GLUE_ICStatusChangeNotify(1,NULL);
    }
}    
#endif
void network_callback(UINT32 event_type, UINT32 param)
{
    //libc_printf("send network message: (%d, %d)\n", event_type, param);
    IP_LOC_CFG  net_local_cfg;
    BOOL        clear = TRUE;
    UINT8       * __MAYBE_UNUSED__ p = NULL;

    if ((NET_MSG_DHCP == event_type) && (NET_ERR_DHCP_SUCCESS == param))    // dhcp and success
    {
        libnet_get_ipconfig(&net_local_cfg);
        net_local_cfg.dhcp_on = 1;
        set_local_ip_cfg(&net_local_cfg);
        libc_printf("DHCP OK:\n");
        p = (unsigned char*)&net_local_cfg.ip_addr;
        libc_printf("IP: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
    }

    switch (event_type)
    {
    case NET_MSG_DHCP:
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DHCP, param, clear);
        break;

    case NET_MSG_DOWNLOAD_START:
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_START, param, clear);
        break;

    case NET_MSG_DOWNLOAD_PROGRESS:
        clear = FALSE;
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_PROGRESS, param, clear);
        break;

    case NET_MSG_DOWNLOAD_FINISH:
        //libc_printf("network_callback NET_MSG_DOWNLOAD_FINISH \n");
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_FINISH, param, clear);
        break;

    case NET_MSG_DOWNLOAD_RESUME:
        //libc_printf("network_callback NET_MSG_DOWNLOAD_RESUME \n");
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_RESUME, param, clear);
        break;

    case NET_MSG_DOWNLOAD_WAITDATA:
        //libc_printf("network_callback NET_MSG_DOWNLOAD_WAITDATA \n");
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_WAITDATA, param, clear);
        break;

    case NET_MSG_DOWNLOAD_NETERROR:
        //libc_printf("network_callback NET_MSG_DOWNLOAD_NETERROR \n");
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NETERROR, param, clear);
        break;

    case NET_MSG_DOWNLOAD_SPEED:
        //libc_printf("network_callback NET_MSG_DOWNLOAD_SPEED \n");
        ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_SPEEDINFO, param, clear);
        break;

    case NET_MSG_DOWNLOAD_INFO:
        //libc_printf("network_callback NET_MSG_DOWNLOAD_INFO \n");
        if (param == NET_DOWNLOAD_INFO_NOSEEK)
        {
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_DOWNLOAD_NOSEEK, param, clear);
        }
        break;
        
		case CTRL_MSG_PING_PROGRESS:
            //libc_printf("network_callback NET_MSG_DOWNLOAD_INFO \n");
        	ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PING_PROGRESS, param, clear);
        break;		
    default:
        break;
    }
}
#endif
#ifdef WIFI_SUPPORT
static void win_wifi_popup_display(UINT8 *string)
{
    UINT8       back_saved;
    __MAYBE_UNUSED__ sys_state_t system_state = SYS_STATE_INITIALIZING;
    system_state = api_get_system_state();

    if (NULL == string)
    {
        return;
    }

#ifdef _INVW_JUICE
    if (menu_stack_get_top() != (POBJECT_HEAD) & g_win_inviewinput)
    {
#endif
        win_msg_popup_close();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg((char *)string, NULL, 0);
#ifdef MULTIVIEW_SUPPORT
        if ((system_state == SYS_STATE_9PIC) || (system_state == SYS_STATE_4_1PIC))
        {
            win_compopup_set_frame(420, 250, 448, 100);
        }
#endif
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
#ifdef _INVW_JUICE
    }
#endif
}

PRESULT ap_wifi_disp_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    UINT8       content[200];
    IP_LOC_CFG  local_ip_cfg;

    UINT32 netdevicetype = 0;
    UINT32 net_choose_flag = 0;
    UINT8 backsaved = 0;
    PRESULT result = PROC_PASS;

#ifdef _INVW_JUICE
    if (!inview_paused())
    {
#if !defined(_INVIEW_DEBUG)
        return;
#endif
    }
#endif /* INVIEW_JUICE */

    if(msg_type==CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG)
    {
        switch(msg_code)
        {
            case WIFI_MSG_DEVICE_PLUG_IN:
                snprintf((char *)content, 200,"[WiFi]: Device Plug IN");
            #ifdef WIFI_DIRECT_SUPPORT
                if(sys_data_get_wifi_direct_onoff() && api_wifi_check_p2p_support())
                {
                    UINT8 back_saved;
                    win_compopup_init(WIN_POPUP_TYPE_SMSG); 
                    win_compopup_set_msg_ext("waiting,enable wifi direct~!!! ", NULL, 0);
                    win_compopup_open_ext(&back_saved);   

                    net_choose_flag = sys_data_get_net_choose_flag();
                    if(NET_CHOOSE_WIFI_DIRECT_ID == net_choose_flag)
                    {
                        nic_getcur_netdevice(&netdevicetype);
                        nic_switch_netdevice(netdevicetype,net_choose_flag);
                    }

                    //                    //api_wifi_enable_device_function(TRUE);//enable wifi.
                    //                    api_wifi_direct_enable_device_function(TRUE);
                    //                    api_wifi_enable_p2p_function(TRUE); // enaable wifi direct. 
                    //                    
                    //                    INT32 time_cnt = 20000;                    
                    //                    while(1) ///(time_cnt > 0)
                    //                    {
                    //                        if(api_wifi_p2p_get_enable_status() != FALSE)
                    //                        {
                    //                            break;
                    //                        }
                    //                        osal_task_sleep(1);
                    //                        time_cnt--;
                    //                        //libc_printf("time:%d \n",time_cnt);
                    //                    }                    
                    wifi_direct_set_device_name("ALI WIFI Direct");
                    //osal_task_sleep(1000);
                    win_compopup_smsg_restoreback();                    
                }
            #endif                
            break;
            case WIFI_MSG_DEVICE_PLUG_OUT:
                snprintf((char *)content, 200,"[WiFi]: Device Plug Out");
                break;

        case WIFI_MSG_DEVICE_AP_CONNECTED:
            snprintf((char *)content, 200,"[WiFi]: AP Connected!");
            break;

        case WIFI_MSG_DEVICE_AP_DISCONNECTED:
            snprintf((char *)content, 200,"[WiFi]: AP Disconnected!");
            break;

        case WIFI_MSG_DEVICE_IPV4_ADDR_RETRIEVED:
            libnet_get_ipconfig(&local_ip_cfg);
            if (local_ip_cfg.ip_addr)
            {
                snprintf((char *)content, 200,"[WiFi]: IP Addr=%03lu.%03lu.%03lu.%03lu",
                        (local_ip_cfg.ip_addr) & 0xFF, (local_ip_cfg.ip_addr >> 8) & 0xFF,
                        (local_ip_cfg.ip_addr >> 16) & 0xFF,
                        (local_ip_cfg.ip_addr >> 24) & 0xFF);
            }
            else
            {
                return result;
            }
            break;
        }

#ifdef _INVW_JUICE
#if defined(_INVIEW_DEBUG)
        if (!inview_paused())
        {
            return result;
        }
#endif
#endif
        win_wifi_popup_display(content);	

#ifdef ALICAST_SUPPORT
        //update for this case: When enter alicast menu, don't enable wifi auto connecting function.  
        extern UINT8 win_alicast_open_flag;

        if(1 == win_alicast_open_flag)  
        {
            return result;
        }
#endif

        if(WIFI_MSG_DEVICE_PLUG_IN == msg_code)
        {
            net_choose_flag = sys_data_get_net_choose_flag();

            if(NET_CHOOSE_WIFI_MANAGER_ID == net_choose_flag)
            {
            #if defined(SAT2IP_SERVER_SUPPORT)||defined(PVR2IP_SERVER_SUPPORT)
                // disconnect all streaming session, to avoid sending data when ethX no work.
                extern int rtsp_session_disconnect_by_type(int type);
                rtsp_session_disconnect_by_type(0); // disconnect all
            #endif
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg_ext("WiFi Opening...,Please Don't Plug Out!", NULL, 0);
                win_compopup_open_ext(&backsaved);	     

                nic_getcur_netdevice((unsigned int *)&netdevicetype);
                nic_switch_netdevice(netdevicetype,net_choose_flag);

                win_compopup_smsg_restoreback();

                ap_wifi_auto_connect();
            }
        }
    }

    return result;
}

void ap_wifi_display_callback(E_WIFI_MSG_TYPE type, UINT32 param)
{
#ifdef _INVW_JUICE
    PRESULT proc_ret = PROC_PASS;
#endif
    UINT32  msg_code;

    //libc_printf("ap_wifi_display_callback()\n");
    msg_code = type;

    // special to do
    switch (type)
    {
    case WIFI_MSG_DEVICE_PLUG_IN:
        //            libc_printf("-WIFI_MSG_DEVICE_PLUG_IN\n");
        // for UI Popup Message
#ifdef _INVW_JUICE
        inview_resume(0);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) & g_win_inviewinput, TRUE);
#else
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG, msg_code, FALSE);
#endif
        break;

    case WIFI_MSG_DEVICE_PLUG_OUT:
        //            libc_printf("-WIFI_MSG_DEVICE_PLUG_OUT\n");
        // for UI Popup Message
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG, msg_code, FALSE);
        break;

    case WIFI_MSG_DEVICE_ON:
        //            libc_printf("-WIFI_MSG_DEVICE_ON\n");
        break;

    case WIFI_MSG_DEVICE_OFF:
        //            libc_printf("-WIFI_MSG_DEVICE_OFF\n");
        break;

    case WIFI_MSG_DEVICE_AP_CONNECTED:
        //            libc_printf("-WIFI_MSG_DEVICE_AP_CONNECTED\n");
        // for UI Popup Message
#ifdef _INVW_JUICE
#else
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG, msg_code, FALSE);
#endif
        break;

    case WIFI_MSG_DEVICE_AP_DISCONNECTED:
        //            libc_printf("-WIFI_MSG_DEVICE_AP_DISCONNECTED\n");
        // for UI Popup Message
#ifdef _INVW_JUICE
#else
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG, msg_code, FALSE);
#endif
        break;

    case WIFI_MSG_DEVICE_IPV4_ADDR_RETRIEVED:
        //            libc_printf("-WIFI_MSG_DEVICE_IPV4_ADDR_RETRIEVED\n");
        // for UI Popup Message
#ifdef _INVW_JUICE
#else
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_DISP_MSG, msg_code, FALSE);
#endif
        break;
    default:
        libc_printf("not handle msg = %d \n",type);
        break;
    }

    // for Menu On the TOP
    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_WIFI_CTRL_MSG, msg_code, FALSE);
#ifdef _INVW_JUICE
    inview_wifi_message(type);  /* TODO: wrong level? */
#endif
}
#endif
