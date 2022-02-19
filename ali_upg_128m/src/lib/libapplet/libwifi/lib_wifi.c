/************************************************************************************************************ 
* Copyright (C) 2008 ALi Corp. All Rights Reserved.All rights reserved.
* 
* File: cec_proc.c 
*
* Description¡GThis is a simple sample file to illustrate format of file 
*             prologue.
*
* History:
*     Date         By          Reason           Ver.
*   ==========  =========    ================= ======
************************************************************************************************************/

/*******************
* INCLUDE FILES    *
********************/
#include <sys_config.h>

#ifdef WIFI_SUPPORT
#include <api/libwifi/lib_wifi.h>
#include <api/libnet/libnet.h>
#include <hld/wifi/wifi_api.h>

//#define LIB_WIFI_DEBUG
#ifdef LIB_WIFI_DEBUG
#define WIFI_PRINTF libc_printf
#else
#define WIFI_PRINTF(...)
#endif

#ifdef _INVW_JUICE
#define	LIB_WIFI_MAX_AP_NUM		100
#else
#define	LIB_WIFI_MAX_AP_NUM		50
#endif

typedef enum {
	CMD_DEVICE_ENABLE = 0,
	CMD_DEVICE_DISABLE,
	CMD_AP_CONNECT,
	CMD_AP_DISCONNECT,
	CMD_AP_SCAN,
#ifdef WIFI_P2P_SUPPORT
    CMD_WIFI_DIRECT_DEVICE_ENABLE,
    CMD_WIFI_DIRECT_DEVICE_DISABLE,
	CMD_P2P_ENABLE,
	CMD_P2P_DISABLE,	
#endif	
    CMD_NETIF_ENABLE,
    CMD_NETIF_DISABLE,
} E_WIFI_CMD;


typedef struct{
	E_WIFI_CMD 			type;
	struct wifi_ap_info		ap_info;
} WIFI_CMD_T;


typedef struct
{
	// LIB_WIFI Task info (No Need to Reset)
	OSAL_ID 			thread_id;			//Task ID
	OSAL_ID 			msgque_id;			//MSG Queue ID
	WIFI_DISP_FUNC_T	event_callback;
	ID					cyclic_timer_id;

	// LIB_WIFI Preference Configuration (No Need To Reset)
	BOOL				b_auto_scan_mode;		// 1: AutoScan 0: No AutoScan
	BOOL				b_auto_dhcp_mode;		// 1: AutoDhcp 0: No AutoDhcp

	// LIB_WIFI Internal State (must reset when plug_in or plug_out)
	UINT8				apscan_num;			// Scanned AP Num
	struct wifi_ap_info 	apscan_list[LIB_WIFI_MAX_AP_NUM];	//Scanned AP List
	struct wifi_ap_info 	previous_apinfo;		// Previous Connected AP Iofo
	struct wifi_ap_info 	connect_apinfo;		// Connecting AP Iofo
	BOOL				b_plugged;			// 1: Plugged	0: UnPlugged	
	BOOL				b_device_enabled;		// 1: Enabled	0: Disabled
	BOOL				b_device_ready;		// 1: Ready	0: Not Ready
	BOOL				b_connected;			// 1: Connected 0: Disconnected
	IP_LOC_CFG 			local_ip_cfg;
	
} WIFI_CFG_T;

WIFI_CFG_T			g_wifi_config;

BOOL				g_prev_plug_state;
BOOL				g_prev_device_enable_state;
BOOL				g_prev_connect_state;
UINT32				g_prev_ipaddr=0;

struct wifi_ap_info		g_tmp_connect_apinfo;


/*******************************************************************************
*	Globe vriable & function declare
*******************************************************************************/
static void wifi_auto_scan_cyclic_timer_handler(UINT32 param);
static BOOL _api_wifi_reset_device_default_value(void);
BOOL wifi_act_do_netif_ablestatus(BOOL b_enable);

UINT32 api_wifi_convert_signal_quality_to_percentage(UINT32 quality)
{
    #if 1 //Barry fix mixed ENCRYPT_MODE
    if(quality)
        quality = quality-1;
    #else
    quality = (quality*100);
	quality = quality/255;
    #endif

	return	(UINT32 )quality;
}

BOOL api_wifi_enable_device_function(BOOL b_enable)
{
	WIFI_CMD_T		command;
	
	WIFI_PRINTF("api_wifi_enable_device_function(bEnable=%d)\n", b_enable);

	if(b_enable)
	{
		command.type	= 	CMD_DEVICE_ENABLE;
	}
	else
	{
		command.type	= 	CMD_DEVICE_DISABLE;
	}
		
	if( osal_msgqueue_send(	g_wifi_config.msgque_id,	&command,	sizeof(WIFI_CMD_T), OSAL_WAIT_FOREVER_TIME) != OSAL_E_OK )
	{
		WIFI_PRINTF(" osal_msgqueue_send fail\n");
		return FALSE;	
	}
	else
	{
		return TRUE;	
	}
}

#ifdef WIFI_P2P_SUPPORT

BOOL api_wifi_direct_enable_device_function(BOOL b_enable)
{
	WIFI_CMD_T		command;
	
	WIFI_PRINTF("api_wifi_enable_device_function(bEnable=%d)\n", b_enable);

	if(b_enable)
	{
		command.type	= 	CMD_WIFI_DIRECT_DEVICE_ENABLE;
	}
	else
	{
		command.type	= 	CMD_WIFI_DIRECT_DEVICE_DISABLE;
	}
		
	if( osal_msgqueue_send(	g_wifi_config.msgque_id,	&command,	sizeof(WIFI_CMD_T), OSAL_WAIT_FOREVER_TIME) != OSAL_E_OK )
	{
		WIFI_PRINTF(" osal_msgqueue_send fail\n");
		return FALSE;	
	}
	else
	{
		return TRUE;	
	}
}

BOOL api_wifi_enable_p2p_function(BOOL b_enable)
{
	WIFI_CMD_T		command;
	
	WIFI_PRINTF("api_wifi_enable_device_function(bEnable=%d)\n", b_enable);

	if(b_enable)
	{
		command.type	= 	CMD_P2P_ENABLE;
	}
	else
	{
		command.type	= 	CMD_P2P_DISABLE;
	}
		
	if( osal_msgqueue_send(	g_wifi_config.msgque_id,	&command,	sizeof(WIFI_CMD_T), OSAL_WAIT_FOREVER_TIME) != OSAL_E_OK )
	{
		WIFI_PRINTF(" osal_msgqueue_send fail\n");
		return FALSE;	
	}
	else
	{
		return TRUE;	
	}
}

BOOL api_wifi_p2p_get_enable_status(void)
{
    BOOL ret = FALSE;
    struct net_device *net_dev = NULL;
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_P2P); 
    if(net_dev != NULL)
    {
        ret = p2p_get_enable_status(net_dev);
    }
    return ret;
}

BOOL api_wifi_check_p2p_support(void)
{
    BOOL ret_code = FALSE;
    struct net_device *net_dev = NULL;
    net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
    if((net_dev != NULL) && (net_dev->set_p2p_enable_status != NULL))
    {
        ret_code = TRUE;
    }
    return ret_code;
}

static void _api_wifi_direct_act_do_device_enable(void)
{
	struct net_device *dev;
		
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(dev)
		wi_fi_set_enable_status(dev, TRUE);
}

static void _api_wifi_direct_act_do_device_disable(void)
{
	struct net_device *dev;

	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(dev)
		wi_fi_set_enable_status(dev, FALSE);
}


static void _api_wifi_p2p_act_do_enable(void)
{
    p2p_set_enable_status(NULL,TRUE);
#if 0 /* Barry test : GO mode */
    char *arg1[] = {"iwpriv", "p2p0", "set", "p2pLinkDown=1"};
    char *arg2[] = {"iwpriv", "p2p0", "set", "p2pReset=1"};
    char *arg3[] = {"iwpriv", "usb-wifi", "set", "WfdDevType=1"};
    char *arg4[] = {"iwpriv", "usb-wifi", "set", "WfdSessionAvail=1"};
    char *arg5[] = {"iwpriv", "usb-wifi", "set", "WfdMaxThroughput=300"};
    char *arg6[] = {"iwpriv", "usb-wifi", "set", "WfdEnable=1"};
    char *arg7[] = {"iwpriv", "p2p0", "set", "p2pOpCh=1"};
    char *arg8[] = {"iwpriv", "p2p0", "set", "p2pLisCh=1"};
    char *arg9[] = {"iwpriv", "p2p0", "set", "P2pOpMode=1"};
    char *arg10[] = {"iwpriv", "p2p0", "set", "WPAPSK=12345678"};
    char *arg11[] = {"iwpriv", "p2p0", "set", "p2pSigmaEnable=0"};
    char *arg12[] = {"iwpriv", "p2p0", "set", "p2pWscMode=2"};
    char *arg13[] = {"iwpriv", "p2p0", "set", "p2pWscConf=3"};
    char *arg14[] = {"iwpriv", "p2p0", "set", "WscMode=2"};
    char *arg15[] = {"iwpriv", "p2p0", "set", "WscConfMode=5"};     
    char *arg16[] = {"iwpriv", "p2p0", "set", "p2pConfirmByUI=1"};
    char *arg17[] = {"iwpriv", "p2p0", "set", "SSID=barry-p2p"};
    char *arg18[] = {"iwpriv", "usb-wifi", "set", "WfdDevType=1"};
    char *arg19[] = {"iwpriv", "usb-wifi", "set", "WfdSessionAvail=1"};
    char *arg20[] = {"iwpriv", "usb-wifi", "set", "WfdCP=1"};
#else /* Barry test : device mode */

    char *arg1[] = {"iwpriv", "p2p0", "set", "p2pLinkDown=1"};
    char *arg2[] = {"iwpriv", "p2p0", "set", "p2pReset=1"};
    char *arg3[] = {"iwpriv", "usb-wifi", "set", "WfdDevType=1"};
    char *arg4[] = {"iwpriv", "usb-wifi", "set", "WfdSessionAvail=1"};
    char *arg5[] = {"iwpriv", "usb-wifi", "set", "WfdMaxThroughput=300"};
    char *arg6[] = {"iwpriv", "usb-wifi", "set", "WfdEnable=1"};
    char *arg7[] = {"iwpriv", "p2p0", "set", "p2pOpCh=1"};
    char *arg8[] = {"iwpriv", "p2p0", "set", "p2pLisCh=1"};
    char *arg9[] = {"iwpriv", "p2p0", "set", "p2pGoInt=0"};
    char *arg10[] = {"iwpriv", "p2p0", "set", "p2pSigmaEnable=0"};
    char *arg11[] = {"iwpriv", "p2p0", "set", "p2pWscMode=2"};
    char *arg12[] = {"iwpriv", "p2p0", "set", "p2pWscConf=3"};
    char *arg13[] = {"iwpriv", "p2p0", "set", "WscMode=2"};
    char *arg14[] = {"iwpriv", "p2p0", "set", "WscConfMode=5"};     
    char *arg15[] = {"iwpriv", "p2p0", "set", "p2pConfirmByUI=1"};
    char *arg16[] = {"iwpriv", "usb-wifi", "set", "WfdDevType=1"};
    char *arg17[] = {"iwpriv", "usb-wifi", "set", "WfdSessionAvail=1"};
    char *arg18[] = {"iwpriv", "usb-wifi", "set", "WfdCP=1"};
    char *arg19[] = {"iwpriv", "p2p0", "set", "WscGetConf=1"};
    char *arg20[] = {"iwpriv", "p2p0", "set", "p2pScan=1"};
#endif
    //struct iwreq		wrq;
    //unsigned char buffer[32] = "ALI Wifi Direct";///"Barry-(^@.@^)-";
    //unsigned char buffer[32] = "B2S-F";///fulan;
    //unsigned char buffer[32] = "B2S-G";///Gospel;
    //unsigned char buffer[32] = "B2S-S";///SMART;    
    iwpriv(4, arg1);
    iwpriv(4, arg2);
    iwpriv(4, arg3);
    iwpriv(4, arg4);
    iwpriv(4, arg5);
    iwpriv(4, arg6);
    iwpriv(4, arg7);
    iwpriv(4, arg8);
    iwpriv(4, arg9);
    iwpriv(4, arg10);
    iwpriv(4, arg11);
    iwpriv(4, arg12);
    iwpriv(4, arg13);
    iwpriv(4, arg14);
    iwpriv(4, arg15);
    iwpriv(4, arg16);
    iwpriv(4, arg17);
    iwpriv(4, arg18);
    iwpriv(4, arg19);
    iwpriv(4, arg20);    
}

static void _api_wifi_p2p_act_do_disable(void)
{
    p2p_set_enable_status(NULL,FALSE);    
}




#endif


BOOL api_wifi_do_ap_connect(struct wifi_ap_info* p_ap_info)
{
	WIFI_CMD_T		command;
	
	WIFI_PRINTF("api_wifi_do_ap_connect(p_ap_info=0x%08X)\n", p_ap_info);

	if(p_ap_info)
	{
		command.type	= 	CMD_AP_CONNECT;
		memcpy(&command.ap_info, p_ap_info, sizeof(struct wifi_ap_info));
		
		if( osal_msgqueue_send(	g_wifi_config.msgque_id,	&command,	sizeof(WIFI_CMD_T), OSAL_WAIT_FOREVER_TIME) != OSAL_E_OK )
		{
			WIFI_PRINTF("osal_msgqueue_send fail\n");	
			return FALSE;
		}
		else
		{
			return TRUE;	
		}
	}
	return FALSE;
}


BOOL api_wifi_do_ap_disconnect(void)
{
	WIFI_CMD_T		command;
	
	WIFI_PRINTF("api_wifi_do_ap_disconnect()\n");

	command.type	= 	CMD_AP_DISCONNECT;
	
	if( osal_msgqueue_send(	g_wifi_config.msgque_id,	&command,	sizeof(WIFI_CMD_T), OSAL_WAIT_FOREVER_TIME) != OSAL_E_OK )
	{
		WIFI_PRINTF(" osal_msgqueue_send fail\n");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

UINT8 api_wifi_get_ap_scan_num(void)
{
	return g_wifi_config.apscan_num;
}

BOOL api_wifi_get_ap_scan_list(struct wifi_ap_info* p_ap_list, UINT8* act_apnum, UINT8 max_apnum)
{

	if(g_wifi_config.apscan_num>max_apnum)
	{
		memcpy(p_ap_list, g_wifi_config.apscan_list, max_apnum*sizeof(struct wifi_ap_info));
		*act_apnum=max_apnum;
	}
	else
	{
		memcpy(p_ap_list, g_wifi_config.apscan_list, g_wifi_config.apscan_num*sizeof(struct wifi_ap_info));
		*act_apnum=g_wifi_config.apscan_num;
	}
	return TRUE;
}

BOOL api_wifi_get_device_plugged_status(void)
{
	return g_wifi_config.b_plugged;
}	

BOOL api_wifi_get_device_connected_status(void)
{
	return g_wifi_config.b_connected;
}

static void api_wifi_set_device_enabled_flag(BOOL b_flag)
{	
	g_wifi_config.b_device_enabled = b_flag;	
}

BOOL api_wifi_get_device_enabled_flag(void)
{
	return g_wifi_config.b_device_enabled;	
}

static void api_wifi_set_device_ready_flag(BOOL b_flag)
{	
	g_wifi_config.b_device_ready = b_flag;	
}

BOOL api_wifi_get_device_ready_flag(void)
{
	return g_wifi_config.b_device_ready;	
}

BOOL api_wifi_get_connecting_ap_info(struct wifi_ap_info* p_ap_info)
{
	memcpy(p_ap_info, &g_wifi_config.connect_apinfo, sizeof(struct wifi_ap_info));
	return TRUE;
}

INT32 api_wifi_get_connected_ap_signal_quality(void)
{
	return g_wifi_config.connect_apinfo.quality;
}

void api_wifi_set_auto_scan_mode(BOOL b_flag)
{	
	g_wifi_config.b_auto_scan_mode = b_flag;	
}

BOOL api_wifi_get_auto_scan_mode_flag(void)
{
	return g_wifi_config.b_auto_scan_mode;	
}

void api_wifi_set_auto_dhcp_mode(BOOL b_flag)
{	
	g_wifi_config.b_auto_dhcp_mode = b_flag;	
}

BOOL api_wifi_get_auto_dhcp_mode_flag(void)
{
	return g_wifi_config.b_auto_dhcp_mode;	
}

BOOL api_wifi_register_ui_display_callback(WIFI_DISP_FUNC_T wifi_disp_func)
{
	g_wifi_config.event_callback = wifi_disp_func;
	return TRUE;	
}

/*******************************************************************************
*	Local variable & function
*******************************************************************************/
static BOOL _api_wifi_check_current_and_previous_ap_info_match(void)
{
	if(memcmp(&g_wifi_config.previous_apinfo, &g_wifi_config.connect_apinfo, sizeof(struct wifi_ap_info ))==0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static BOOL __api_wifi_copy_ap_info(struct wifi_ap_info* p_dst_ap, struct wifi_ap_info* p_src_ap)
{
	memcpy((void*)p_dst_ap, (void*)p_src_ap, sizeof(struct wifi_ap_info));
	return TRUE;
}


static BOOL _api_wifi_backup_temp_connect_ap_info(struct wifi_ap_info* p_ap_info)
{
	return __api_wifi_copy_ap_info(&g_tmp_connect_apinfo, p_ap_info);
}


static BOOL _api_wifi_update_previous_connect_ap_info(void)
{
	return __api_wifi_copy_ap_info(&g_wifi_config.previous_apinfo, &g_tmp_connect_apinfo);
}


static BOOL _api_wifi_update_current_connect_ap_info(void)
{
	return __api_wifi_copy_ap_info(&g_wifi_config.connect_apinfo, &g_tmp_connect_apinfo);
}


static BOOL __api_wifi_reset_ap_info(struct wifi_ap_info* p_ap_info)
{
	memset(p_ap_info, 0, sizeof(struct wifi_ap_info));
	return TRUE;
}

static BOOL _api_wifi_reset_previous_ap_info(void)
{
	return __api_wifi_reset_ap_info(&g_wifi_config.previous_apinfo);
}

static BOOL _api_wifi_reset_current_ap_info(void)
{
	return __api_wifi_reset_ap_info(&g_wifi_config.connect_apinfo);
}


static void wifi_device_event_on_ap_scan_finish(void)
{
	WIFI_PRINTF("wifi_device_event_on_ap_scan_finish(), ap_num=%d\n", g_wifi_config.apscan_num);

	if(api_wifi_get_device_plugged_status()&&api_wifi_get_device_enabled_flag())
	{
    		if(g_wifi_config.event_callback) //modify ui_issue_on_off
    	    {
        		// For Display Callback
        		g_wifi_config.event_callback(WIFI_MSG_DEVICE_AP_SCAN_FINISH, 0);
    	    }
	}
	else
	{
		_api_wifi_reset_device_default_value();
	}
}


static void wifi_device_event_on_device_enabled(void)
{
	WIFI_PRINTF("wifi_device_event_on_device_enabled()\n");
	
	if(g_wifi_config.event_callback)
	{
		// For Display Callback
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_ON, 0);
	}
}


static void wifi_device_event_on_device_disabled(void)
{	
	WIFI_PRINTF("wifi_device_event_on_device_disabled()\n");
	
	if(g_wifi_config.event_callback)
	{
		// For Display Callback
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_OFF, 0);
	}
	_api_wifi_reset_device_default_value();
}


static void wifi_device_event_on_ipv4_address_retrieved(void)
{
	WIFI_PRINTF("wifi_device_event_on_ipv4_address_retrieved()\n");
	
	if(g_wifi_config.event_callback)
	{
		// For Display Callback
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_IPV4_ADDR_RETRIEVED, 0);
	}

}

static void wifi_device_event_on_ap_connected(void)
{
	WIFI_PRINTF("wifi_device_event_on_ap_connected()\n");
	
	if(g_wifi_config.event_callback)
	{
		// For Display Callback
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_AP_CONNECTED, 0);
	}
	
	_api_wifi_update_current_connect_ap_info();

	if(!_api_wifi_check_current_and_previous_ap_info_match())
	{
		//Current AP does NOT match Previous AP
		_api_wifi_update_previous_connect_ap_info();
	}

	if(api_wifi_get_auto_dhcp_mode_flag())
	{
		WIFI_PRINTF("[WiFi]: initial AUTO_DHCP mode\n");
	
#ifdef _INVW_JUICE
		if(!libnet_dhcp_on())
#else
		if(libnet_dhcp_on())
#endif
		{
			WIFI_PRINTF("[WiFi]: DHCP success!(Finished)\n");
		}
		else
		{
			WIFI_PRINTF("[WiFi]: DHCP Fail! (operation Not Finished)\n");
		}
	}
}

static void wifi_device_event_on_ap_disconnected(void)
{
	WIFI_PRINTF("wifi_device_event_on_ap_disconnected()\n");
	if(g_wifi_config.event_callback)
	{
		// For Display Callback
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_AP_DISCONNECTED, 0);
	}
	_api_wifi_reset_current_ap_info();		
}

static void wifi_device_event_on_device_plug_in(void)
{
	WIFI_PRINTF("wifi_device_event_on_device_plug_in()\n");

	if(g_wifi_config.event_callback)
	{
		// For Display Callback
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_PLUG_IN, 0);
	}
	_api_wifi_reset_device_default_value();
	#ifdef _INVW_JUICE
	api_wifi_set_device_enabled_flag(TRUE);
	#else
	#if 1
	api_wifi_set_device_enabled_flag(FALSE);
	#else
	api_wifi_set_device_enabled_flag(TRUE);
	#endif
	#endif
	#ifdef _INVW_JUICE
	api_wifi_enable_device_function(TRUE);
	#else
	//api_wifi_enable_device_function(TRUE);
	#endif
}

static void wifi_device_event_on_device_plug_out(void)
{
	WIFI_PRINTF("wifi_device_event_on_device_plug_out()\n");

	if(g_wifi_config.event_callback)
	{
		g_wifi_config.event_callback(WIFI_MSG_DEVICE_PLUG_OUT, 0);
	}
	_api_wifi_reset_device_default_value();
}

static void _api_wifi_act_do_device_enable(void)
{
	struct net_device *dev;
	
	
	
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(dev)
		wi_fi_set_enable_status(dev, TRUE);

    api_wifi_set_device_enabled_flag(TRUE); //modify ui_issue_on_off
	
}

static void _api_wifi_act_do_device_disable(void)
{
	struct net_device *dev;
	
	api_wifi_set_device_enabled_flag(FALSE);

	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(dev)
		wi_fi_set_enable_status(dev, FALSE);
}

static BOOL _api_wifi_act_do_ap_scan(void)
{
	UINT32	i;
	INT32	ap_scan_num;
	
	struct net_device *dev;
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);

	if(dev)
	{
		ap_scan_num = wi_fi_scan_ap(dev);
		if(ap_scan_num>=0)
		{
			WIFI_PRINTF("ap_scan_num=%d\n", ap_scan_num);

			for(i=0; i<LIB_WIFI_MAX_AP_NUM; i++)
			{
				__api_wifi_reset_ap_info(&g_wifi_config.apscan_list[i]);
			}

            if(ap_scan_num > LIB_WIFI_MAX_AP_NUM)
                ap_scan_num = LIB_WIFI_MAX_AP_NUM; //ap_scan_num must <= LIB_WIFI_MAX_AP_NUM
			g_wifi_config.apscan_num=(UINT8)ap_scan_num;
			wi_fi_get_aplist(dev, g_wifi_config.apscan_list, g_wifi_config.apscan_num);

			wifi_device_event_on_ap_scan_finish();

			return TRUE;
		}
		else
		{
			WIFI_PRINTF("_api_wifi_act_do_ap_scan(): FAIL (ap_scan_num=%d, driver fails to return AP List)\n", ap_scan_num);
			return FALSE;
		}
	}
	else
	{
		WIFI_PRINTF("_api_wifi_act_do_ap_scan(): FAIL (device NOT exist)\n");
		return FALSE;
	}
}

static BOOL _api_wifi_act_do_ap_connect(struct wifi_ap_info* p_ap_info)
{
	struct net_device *dev;
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);

	if(dev)
	{
		if(p_ap_info)
		{
			WIFI_PRINTF("_api_wifi_act_do_ap_connect()\n");
			WIFI_PRINTF("-SSID[]=%s\n", p_ap_info->ssid);
			WIFI_PRINTF("-EncryptMode=%d\n", p_ap_info->encrypt_mode);
			WIFI_PRINTF("-keyIdx=%d\n", p_ap_info->key_idx);
			WIFI_PRINTF("-pwd[]=%s\n", p_ap_info->pwd);
			WIFI_PRINTF("-numCharInPwd=%d\n", p_ap_info->num_char_in_pwd);

			
			if(((p_ap_info->encrypt_mode==WIFI_ENCRYPT_MODE_OPEN_WEP)||(p_ap_info->encrypt_mode==WIFI_ENCRYPT_MODE_SHARED_WEP))&&((p_ap_info->key_idx!=1)&&(p_ap_info->key_idx!=2)&&(p_ap_info->key_idx!=3)&&(p_ap_info->key_idx!=4)))
			{
				WIFI_PRINTF("KeyIndex invalid (%d)\n",p_ap_info->key_idx);
				return FALSE;
			}
	
			wi_fi_connect_ap(dev, p_ap_info);

			_api_wifi_backup_temp_connect_ap_info(p_ap_info);
			
			if(1==wi_fi_get_conn_status(dev))
			{
				WIFI_PRINTF("_api_wifi_act_do_ap_connect(): OK\n");
				return TRUE;
			}
			else
			{
				WIFI_PRINTF("_api_wifi_act_do_ap_connect(): FAIL (operation Not Finished)\n");
				return FALSE;
			}
		}
#ifdef _INVW_JUICE
		else
		{
			WIFI_PRINTF("_api_wifi_act_do_ap_connect(): FAIL (no AP info)\n");
			return FALSE;
		}
#endif

	}

	WIFI_PRINTF("_api_wifi_act_do_ap_connect(): FAIL (device NOT exist)\n");
	return FALSE;
}

static BOOL _api_wifi_act_do_ap_disconnect(void)
{
	struct net_device *dev;
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);

	if(dev)
	{
		//if(api_wifi_get_device_connected_status())
		{
			
			wi_fi_dis_connect_ap(dev, &g_wifi_config.connect_apinfo);
			if(1==wi_fi_get_conn_status(dev))
			{
				WIFI_PRINTF("_api_wifi_act_do_ap_disconnect(): FAIL (operation Not Finished)\n");
				return FALSE;			
			}
			else
			{
				WIFI_PRINTF("_api_wifi_act_do_ap_disconnect(): OK\n");
				return TRUE;
			}
		}
	}

	WIFI_PRINTF("_api_wifi_act_do_ap_disconnect(): FAIL (device NOT exist)\n");
	return FALSE;
}

static BOOL _api_wifi_reset_device_default_value(void)
{
	UINT32 i;
	
	//g_wifi_config.cyclic_timer_id = OSAL_INVALID_ID;

	//LIB_WIFI Internal State (except fot plug_in/ plug_out state)
	g_wifi_config.apscan_num=0;
	
	for(i=0; i<LIB_WIFI_MAX_AP_NUM; i++)
	{
		__api_wifi_reset_ap_info(&g_wifi_config.apscan_list[i]);
	}
	__api_wifi_reset_ap_info(&g_wifi_config.connect_apinfo);
	__api_wifi_reset_ap_info(&g_wifi_config.previous_apinfo);

	g_wifi_config.b_connected=FALSE;
	g_wifi_config.b_device_enabled=FALSE;
	g_wifi_config.b_device_ready=FALSE;
	memset(&g_wifi_config.local_ip_cfg, 0, sizeof(IP_LOC_CFG));

	//LIB_WIFI Global Variable
	g_prev_connect_state=FALSE;		// connected or disconnected
	g_prev_device_enable_state=FALSE;	// enabled or disabled
	g_prev_ipaddr=0;

	__api_wifi_reset_ap_info(&g_tmp_connect_apinfo);

	return TRUE;
}

static BOOL _api_wifi_update_device_status(void)
{
	struct net_device *dev;
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);

	if(dev)
	{
		g_wifi_config.b_plugged	= ((WIFI_DEV_PLUG_IN==wi_fi_get_plug_status(dev))?TRUE:FALSE);	

		if(g_wifi_config.b_plugged)
		{
			if(g_wifi_config.b_device_enabled)
			{
				g_wifi_config.b_device_ready= ((WIFI_DEV_READY == wi_fi_get_ready_status(dev))?TRUE:FALSE);
				g_wifi_config.b_connected	= ((WIFI_MEDIA_CONNECTED==wi_fi_get_conn_status(dev))?TRUE:FALSE);
				if(g_wifi_config.b_connected)
				{
					g_wifi_config.connect_apinfo.quality=wi_fi_get_apquality(dev);

					libnet_get_ipconfig(&g_wifi_config.local_ip_cfg);				
				}
			}
			else
			{
				g_wifi_config.b_device_ready = FALSE;
				g_wifi_config.b_connected = FALSE;		
			}
		}
		else
		{
			g_wifi_config.b_device_ready = FALSE;
			g_wifi_config.b_connected = FALSE;
		}
	}
	else 
	{
		g_wifi_config.b_plugged = FALSE;
		g_wifi_config.b_device_enabled = FALSE;
		g_wifi_config.b_device_ready = FALSE;
		g_wifi_config.b_connected = FALSE;	
	}
	return TRUE;
}

static void wifi_internal_message_proc(void)
{
	
	WIFI_CMD_T		wifi_msg;
	UINT32			msg_len;

	//WIFI_PRINTF("wifi_internal_message_proc()\n");

	if (osal_msgqueue_receive(&wifi_msg, (INT*)&msg_len, g_wifi_config.msgque_id, 100) == E_OK)
	{
		switch (wifi_msg.type)
		{
			case CMD_DEVICE_ENABLE:
				WIFI_PRINTF("-CMD_DEVICE_ENABLE\n");			
				_api_wifi_act_do_device_enable();
				break;
				
			case CMD_DEVICE_DISABLE:
				WIFI_PRINTF("-CMD_DEVICE_DISABLE\n");
				if(api_wifi_get_device_connected_status())
				{
					_api_wifi_act_do_ap_disconnect();
				}
				_api_wifi_act_do_device_disable();
				break;

			case CMD_AP_CONNECT:
				WIFI_PRINTF("-CMD_AP_CONNECT\n");			
				_api_wifi_act_do_ap_connect(&wifi_msg.ap_info);
				break;

			case CMD_AP_DISCONNECT:
				WIFI_PRINTF("-CMD_AP_DISCONNECT\n");			
				_api_wifi_act_do_ap_disconnect();
				break;
				
			case CMD_AP_SCAN:	
				WIFI_PRINTF("-CMD_AP_SCAN\n");			
				_api_wifi_act_do_ap_scan();
				break;
#ifdef WIFI_P2P_SUPPORT
            case CMD_WIFI_DIRECT_DEVICE_ENABLE:
				WIFI_PRINTF("-CMD_DEVICE_ENABLE\n");			
				_api_wifi_direct_act_do_device_enable();
				break;
            case CMD_WIFI_DIRECT_DEVICE_DISABLE:
				WIFI_PRINTF("-CMD_DEVICE_DISABLE\n");
				_api_wifi_direct_act_do_device_disable();
				break;
            case CMD_P2P_ENABLE:
                _api_wifi_p2p_act_do_enable();
                break;
            case CMD_P2P_DISABLE:
                _api_wifi_p2p_act_do_disable();
                break;
#endif  
            case CMD_NETIF_ENABLE:
                wifi_act_do_netif_ablestatus(TRUE);
                break;
                
            case CMD_NETIF_DISABLE:
				if(api_wifi_get_device_connected_status())
				{
					_api_wifi_act_do_ap_disconnect();
				}
				wifi_act_do_netif_ablestatus(FALSE);
                break;
                
			default:
				break;									
		}
	}	
}


static void wifi_device_status_monitor_proc(void)
{
	_api_wifi_update_device_status();
}

static void wifi_device_event_dispatch_proc(void)
{
	//WIFI_PRINTF("wifi_device_event_dispatch_proc()\n");

	// Device PLUG_IN or PLUG_OUT
	if(g_wifi_config.b_plugged!=g_prev_plug_state)
	{
		//WIFI_PRINTF("-PLUG_EVENT\n");			

		g_prev_plug_state = g_wifi_config.b_plugged;

		// here we monitor device PLUG_IN / PLUG_OUT events
		if(g_wifi_config.b_plugged)
		{
			wifi_device_event_on_device_plug_in();
		}
		else
		{
			wifi_device_event_on_device_plug_out();
		}
	}
	
	// Device ENABLED or DISABLED
	if(g_wifi_config.b_device_enabled!=g_prev_device_enable_state)
	{		
		if(api_wifi_get_device_plugged_status())
		{
			//WIFI_PRINTF("-ON_OFF_EVENT\n");			

			g_prev_device_enable_state = g_wifi_config.b_device_enabled;

			// here we only monitor Device Enabled/ Disabled event

			if(g_wifi_config.b_device_enabled)
			{
				wifi_device_event_on_device_enabled();
			}
			else
			{
				wifi_device_event_on_device_disabled();
			}
		}
	}
	
	// Device CONNECTED or DISCONNECTED
	if(g_wifi_config.b_connected!=g_prev_connect_state)
	{				
		if(api_wifi_get_device_plugged_status()&&api_wifi_get_device_enabled_flag())
		{

			//WIFI_PRINTF("-CONNECT_EVENT\n");	

			g_prev_connect_state = g_wifi_config.b_connected;

			// Only Monitor Connection Status when WiFi device is plugged IN
			// here we only monitor Connected/ Disconnected event

			if(g_wifi_config.b_connected)
			{
				wifi_device_event_on_ap_connected();
			}
			else
			{
				wifi_device_event_on_ap_disconnected();
			}
		}
	}


	if(g_wifi_config.local_ip_cfg.ip_addr!=g_prev_ipaddr)
	{
		g_prev_ipaddr=g_wifi_config.local_ip_cfg.ip_addr;

		if(g_wifi_config.local_ip_cfg.ip_addr)
		{
			wifi_device_event_on_ipv4_address_retrieved();
		}
	}
	
}

static void wifi_auto_scan_cyclic_timer_handler(UINT32 param)
{

	if(	(api_wifi_get_device_plugged_status()) &&
		(api_wifi_get_device_enabled_flag())	&& 
		(api_wifi_get_device_ready_flag())	&& 
		(!api_wifi_get_device_connected_status())
	)
	{
		if(api_wifi_get_auto_scan_mode_flag())
		{
			_api_wifi_act_do_ap_scan();
		}
	}
}

static ID _start_cycletimer(char *name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler)
{
    OSAL_T_CTIM     t_dalm;
    ID                alarmid = INVALID_ID;

    t_dalm.callback = handler;
    t_dalm.type = OSAL_TIMER_CYCLE;
    t_dalm.time  = interval;

    alarmid = osal_timer_create(&t_dalm);
    if(OSAL_INVALID_ID != alarmid)
    {
        //libc_printf("----------------start %s timer succeed\n", name);
        osal_timer_activate(alarmid, 1);
        return alarmid;
    }
    else
    {
        //libc_printf("----------------start %s timer failed\n",name);
        return OSAL_INVALID_ID;
    }
}

void wifi_main_task(UINT32 param1, UINT32 param2)
{

	WIFI_PRINTF("wifi_main_task()\n");

#ifdef _INVW_JUICE   
	api_wifi_set_auto_scan_mode(FALSE);
#else
	api_wifi_set_auto_scan_mode(TRUE);
#endif
	api_wifi_set_auto_dhcp_mode(TRUE);

	g_wifi_config.cyclic_timer_id= _start_cycletimer("LIB_WIFI_AUTO_SCAN", 2000, wifi_auto_scan_cyclic_timer_handler);	
		
	while(TRUE)
	{	
		WIFI_PRINTF("-----------------\n");

		//monitor device status & dispatch event
		wifi_device_status_monitor_proc();
		wifi_device_event_dispatch_proc();

		//process incomming message command
		wifi_internal_message_proc();

		osal_task_sleep(200);
	}
}



INT32 wifi_module_init(WIFI_DISP_FUNC_T wifi_disp_func)
{
	WIFI_PRINTF("wifi_module_init(wifi_disp_func=0x%08X)\n", wifi_disp_func);

	OSAL_T_CTSK t_ctsk;
	OSAL_T_CMBF t_cmbf;


	/* Reset WIFI Config */
	memset(&g_wifi_config,0, sizeof(WIFI_CFG_T));

	if(wifi_disp_func)
	{
		g_wifi_config.event_callback = wifi_disp_func;
	}

	wi_fi_device_enable(TRUE);

	/* create massage buffer */
	t_cmbf.bufsz = 16*sizeof(WIFI_CMD_T);
	t_cmbf.maxmsz = sizeof(WIFI_CMD_T);
	g_wifi_config.msgque_id= osal_msgqueue_create(&t_cmbf);
	if (OSAL_INVALID_ID == g_wifi_config.msgque_id)
	{
		WIFI_PRINTF("[LIB_WIFI]: osal_msgqueue_create(), error!\n");
		ASSERT(0);
	}
	
	/* create main thread */	
	t_ctsk.task = wifi_main_task;
#ifdef _INVW_JUICE   
	t_ctsk.itskpri= OSAL_PRI_HIGH; //OSAL_PRI_NORMAL; // 20120816-Mark
	t_ctsk.quantum=20; //10; // 20120816-Mark
#else
	t_ctsk.itskpri=OSAL_PRI_NORMAL;
	t_ctsk.quantum=10;
#endif
	t_ctsk.stksz=0x2000;
	t_ctsk.para1=0;
	t_ctsk.para2=0;
	t_ctsk.name[0] = 'W';
	t_ctsk.name[1] = 'I';
	t_ctsk.name[2] = 'F';
 
	g_wifi_config.thread_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == g_wifi_config.thread_id)
	{
		WIFI_PRINTF("[LIB_WIFI]: osal_task_create(), failed\n");
		return ERR_FAILUE;
	}		
	
	//WiFi_SetDevicePidVid(0x7711,0x7392); //example

	return RET_SUCCESS;	
}	
#endif

void wifi_device_enable(void)
{
	_api_wifi_act_do_device_enable();
}

void wifi_device_disable(void)
{
	_api_wifi_act_do_device_disable();
}

BOOL wifi_disconnect_current_ap(void)
{
	return _api_wifi_act_do_ap_disconnect();
}

BOOL wifi_act_do_netif_ablestatus(BOOL b_enable)
{
	struct net_device *dev;

    if (FALSE == b_enable)
    {
        api_wifi_set_device_enabled_flag(b_enable);
    }
	
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(dev)
	{
		wi_fi_set_enable_status2(dev, b_enable);
	}

    if (TRUE == b_enable)
    {
        api_wifi_set_device_enabled_flag(b_enable);
    }

    return TRUE;
}

BOOL api_wifi_netif_function(BOOL b_enable)
{
	WIFI_CMD_T		command;
	
	WIFI_PRINTF("api_wifi_enable_device_function(bEnable=%d)\n", b_enable);

	if(b_enable)
	{
		command.type	= 	CMD_NETIF_ENABLE;
	}
	else
	{
		command.type	= 	CMD_NETIF_DISABLE;
	}
		
	if( osal_msgqueue_send(	g_wifi_config.msgque_id,	&command,	sizeof(WIFI_CMD_T), OSAL_WAIT_FOREVER_TIME) != OSAL_E_OK )
	{
		WIFI_PRINTF(" osal_msgqueue_send fail\n");
		return FALSE;	
	}
	else
	{
		return TRUE;	
	}
}

