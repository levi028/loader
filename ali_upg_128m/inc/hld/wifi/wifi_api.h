#ifndef _WIFI_API_H_
#define _WIFI_API_H_

#include <hld/hld_dev.h>
#include <hld/net/net.h>

//#define WiFi_PRINTF	libc_printf
#define wi_fi_printf(...)	do{}while(0)	

//#define DBG_WIFI_API
#ifdef DBG_WIFI_API
#define DBG_WIFI_PRINTF	libc_printf
#else
#define DBG_WIFI_PRINTF(...)	do{}while(0)	
#endif

static UINT8 wifi_dev_name[HLD_MAX_NAME_SIZE] __attribute__((unused))= "usb-wifi";
static UINT8 p2p_dev_name[HLD_MAX_NAME_SIZE] __attribute__((unused))= "p2p0";

#define WIFI_MAX_SSID_LEN	32
#define WIFI_MAX_PWD_LEN	136

#if 1 //Barry fix mixed ENCRYPT_MODE
#define ENCRYPT_NONE "NONE"
#define ENCRYPT_WEP "WEP"
#define ENCRYPT_WPA_TKIP "WPAPSK/TKIP"
#define ENCRYPT_WPA_AES "WPAPSK/AES"
#define ENCRYPT_WPA2_TKIP "WPA2PSK/TKIP"
#define ENCRYPT_WPA2_AES "WPA2PSK/AES"
#define encrypt_wpa_mixed_1 "WPAPSK/TKIPAES"
#define encrypt_wpa_mixed_2 "WPA2PSK/TKIPAES"
#define encrypt_wpa_mixed_3 "WPA1PSKWPA2PSK/TKIP"
#define encrypt_wpa_mixed_4 "WPA1PSKWPA2PSK/AES"
#define encrypt_wpa_mixed_5 "WPA1PSKWPA2PSK/TKIPAES"
#endif //end Barry fix mixed ENCRYPT_MODE

typedef enum _WIFI_CONNECT_STATUS
{
	WIFI_MEDIA_DISCONNECTED = 0,
	WIFI_MEDIA_CONNECTED,
} WIFI_CONNECT_STATUS;

typedef enum _WIFI_READY_STATUS
{
	WIFI_DEV_NOT_READY = 0,
	WIFI_DEV_READY,
} WIFI_READY_STATUS;

typedef enum _WIFI_PLUG_STATUS
{
	WIFI_DEV_PLUG_OUT = 0,
	WIFI_DEV_PLUG_IN,
} WIFI_PLUG_STATUS;

typedef enum _WIFI_ENCRYPT_MODE
{
	WIFI_ENCRYPT_MODE_NONE = 0,
	WIFI_ENCRYPT_MODE_OPEN_WEP,
	WIFI_ENCRYPT_MODE_SHARED_WEP,
	WIFI_ENCRYPT_MODE_WPAPSK_TKIP,
	WIFI_ENCRYPT_MODE_WPAPSK_AES,
	WIFI_ENCRYPT_MODE_WPA2PSK_TKIP,
	WIFI_ENCRYPT_MODE_WPA2PSK_AES,
} WIFI_ENCRYPT_MODE;

typedef enum _WIFI_SWITCH_MEDIA_TYPE
{
	WIFI_SWITCH_MAC_TO_WIFI = 0,
	WIFI_SWITCH_WIFI_TO_MAC,
	WIFI_SWITCH_INIT,
	WIFI_SWITCH_NETIF,
} WIFI_SWITCH_MEDIA_TYPE;

typedef enum _WIFI_SWITCH_MEDIA_TYPE2
{
    WIFI_SWITCH_TO_WIFI = 0,
    WIFI_SWITCH_TO_OTHER,

    WIFI_SWITCH_MAX,
}WIFI_SWITCH_MEDIA_TYPE2;

struct wifi_ap_info {
	char 	ssid[WIFI_MAX_SSID_LEN ];     // AP's SSID
	WIFI_ENCRYPT_MODE	encrypt_mode;   // Encrypt mode
	char 	key_idx;                                 // for WEP key index
	char		pwd[WIFI_MAX_PWD_LEN];      // Password
	char 	num_char_in_pwd;        		    // Number of characters in pwd
	int 		quality;	//rssi strength
};

enum WIFI_DIRECT_ASSOCIATION_STATUS{ 
	 RT_P2P_DEVICE_FIND=					    0x010A
	,RT_P2P_RECV_PROV_REQ=						0x010B
	,RT_P2P_RECV_PROV_RSP=						0x010C
	,RT_P2P_RECV_INVITE_REQ=					0x010D
	,RT_P2P_RECV_INVITE_RSP=					0x010E
	,RT_P2P_RECV_GO_NEGO_REQ=					0x010F
	,RT_P2P_RECV_GO_NEGO_RSP=					0x0110
	,RT_P2P_GO_NEG_COMPLETED=					0x0111
	,RT_P2P_GO_NEG_FAIL	=						0x0112
	,RT_P2P_WPS_COMPLETED=						0x0113
	,RT_P2P_CONNECTED=						    0x0114
	,RT_P2P_DISCONNECTED=					    0x0115
	,RT_P2P_CONNECT_FAIL=				        0x0116
	,RT_P2P_LEGACY_CONNECTED=				    0x0117
	,RT_P2P_LEGACY_DISCONNECTED=				0x0118
	,RT_P2P_AP_STA_CONNECTED=			        0x0119
	,RT_P2P_AP_STA_DISCONNECTED=				0x011A
	,RT_P2P_AP_STA_OUT=				            0x011D
    };

#define SIOCGIFHWADDR	0x8927		/* Get hardware address		*/
#define RT_OID_WSC_PIN_CODE							0x0752
#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x01)
#define	OID_GET_SET_TOGGLE			0x8000
#define OID_802_11_P2P_DEVICE_NAME			0x0802
#define RT_OID_802_11_P2P_DEVICE_NAME		(OID_GET_SET_TOGGLE + OID_802_11_P2P_DEVICE_NAME)


/* Function Prototype */
int wi_fi_scan_ap(struct net_device *dev);
int wi_fi_get_info(struct net_device *dev, UINT32 info_type, void *info_buf);
int wi_fi_set_info(struct net_device *dev, UINT32 info_type, char*  arg);  //cpl_0312
int wi_fi_get_aplist(struct net_device *dev, struct wifi_ap_info ap_list [ ], int num_ap);
void wi_fi_connect_ap(struct net_device *dev, struct wifi_ap_info *ap_info);
void wi_fi_dis_connect_ap(struct net_device *dev, struct wifi_ap_info *ap_info);
int wi_fi_get_conn_status(struct net_device *dev);
int wi_fi_get_plug_status(struct net_device *dev);
int wi_fi_set_enable_status(struct net_device *dev, int enable);
int wi_fi_get_enable_status(struct net_device *dev);
int wi_fi_get_ready_status(struct net_device *dev);
int wi_fi_get_apquality(struct net_device *dev);
int wi_fi_swtich_media(UINT8 type);
int wi_fi_device_enable(BOOL b_enable);//cpadd_110531
int wi_fi_set_device_pid_vid(UINT16 pid,UINT16 vid);
RET_CODE wi_fi_detach(struct net_device *dev);
RET_CODE wi_fi_attach(void);
RET_CODE p2p_attach(void);
int p2p_set_enable_status(struct net_device *dev, int enable);
int wi_fi_device_ioctl(const char * ifname, int request, struct iwreq *pwrq);
void p2p_register_send_event_callback(void *callback);
int p2p_get_enable_status(struct net_device *dev);
int p2p_set_device_name(struct net_device *dev, void *buffer);

int wi_fi_set_enable_status2(struct net_device *dev, int enable);
#endif
