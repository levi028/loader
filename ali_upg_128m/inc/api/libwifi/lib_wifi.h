#ifndef _LIB_WIFI_H_
#define _LIB_WIFI_H_

#include <types.h>
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <hld/wifi/wifi_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct wifi_ap_info*		gReConnectAPInfo;

typedef enum
{
    WIFI_NO_MSG = 0,
    WIFI_MSG_DEVICE_PLUG_IN,
    WIFI_MSG_DEVICE_PLUG_OUT,
    WIFI_MSG_DEVICE_ON,
    WIFI_MSG_DEVICE_OFF,
    WIFI_MSG_DEVICE_AP_CONNECTED,
    WIFI_MSG_DEVICE_AP_DISCONNECTED,
    WIFI_MSG_DEVICE_AP_SCAN_FINISH,
    WIFI_MSG_DEVICE_IPV4_ADDR_RETRIEVED,
} E_WIFI_MSG_TYPE;

typedef void (*WIFI_DISP_FUNC_T)(E_WIFI_MSG_TYPE type, UINT32 param);

BOOL    api_wifi_enable_device_function(BOOL b_enable);
BOOL    api_wifi_do_ap_connect(struct wifi_ap_info* p_ap_info);
BOOL    api_wifi_do_ap_disconnect(void);
UINT8    api_wifi_get_ap_scan_num(void);
BOOL    api_wifi_get_ap_scan_list(struct wifi_ap_info* p_ap_list, UINT8* act_apnum, UINT8 max_apnum);
BOOL    api_wifi_get_device_plugged_status(void);
BOOL    api_wifi_get_device_connected_status(void);
BOOL    api_wifi_get_device_enabled_flag(void);
BOOL    api_wifi_get_connecting_ap_info(struct wifi_ap_info* p_ap_info);
INT32    api_wifi_get_connected_ap_signal_quality(void);
UINT32    api_wifi_convert_signal_quality_to_percentage(UINT32 quality);
void    api_wifi_set_auto_scan_mode(BOOL b_flag);
BOOL    api_wifi_get_auto_scan_mode_flag(void);
void    api_wifi_set_auto_dhcp_mode(BOOL b_flag);
BOOL    api_wifi_get_auto_dhcp_mode_flag(void);
BOOL    api_wifi_register_ui_display_callback(WIFI_DISP_FUNC_T wifi_disp_func);
BOOL    api_wifi_get_device_ready_flag(void);

#ifdef WIFI_P2P_SUPPORT
BOOL api_wifi_direct_enable_device_function(BOOL bEnable);
BOOL api_wifi_enable_p2p_function(BOOL bEnable);
BOOL api_wifi_p2p_get_enable_status(void);
BOOL api_wifi_check_p2p_support(void);
#endif


INT32 wifi_module_init(WIFI_DISP_FUNC_T wifi_disp_func);

BOOL api_wifi_netif_function(BOOL bEnable);

#ifdef __cplusplus
}
#endif

#endif//_LIB_WIFI_H_

