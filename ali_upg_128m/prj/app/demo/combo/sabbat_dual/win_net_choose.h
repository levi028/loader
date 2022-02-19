#ifndef _WIN_NET_CHOOSE_H_
#define _WIN_NET_CHOOSE_H_

#ifdef NETWORK_SUPPORT
//#define WIN_NETCHOOSE_DEBUG
#ifdef WIN_NETCHOOSE_DEBUG
    #define NETCHOOSE_PRINT libc_printf
#else
    #define NETCHOOSE_PRINT(...) 
#endif

typedef enum
{
    NET_CHOOSE_NONE,
#ifndef WIFI_OS_SUPPORT
	NET_CHOOSE_ETHERNET_ID,
#ifdef WIFI_SUPPORT
	NET_CHOOSE_WIFI_MANAGER_ID,
	#endif
#else
    #ifdef WIFI_SUPPORT
	NET_CHOOSE_WIFI_MANAGER_ID,
    #endif
    NET_CHOOSE_ETHERNET_ID,
#endif
#ifdef USB3G_DONGLE_SUPPORT
	NET_CHOOSE_3G_DONGLE_ID,
#endif
#ifdef HILINK_SUPPORT
	NET_CHOOSE_HILINK_ID,
#endif
#ifdef WIFI_DIRECT_SUPPORT
    NET_CHOOSE_WIFI_DIRECT_ID,
#endif
}NET_CHOOSE_ID;

#ifdef WIFI_SUPPORT
void ap_wifi_auto_connect(void);
#endif


UINT8 win_net_choose_get_card_num(void);
PRESULT comlist_menu_net_choose_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

#endif

#endif

