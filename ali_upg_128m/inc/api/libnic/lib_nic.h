#ifndef _LIB_NIC_H_
#define _LIB_NIC_H_

#ifdef NETWORK_SUPPORT
//#define LIBNIC_DEBUG
#ifdef LIBNIC_DEBUG
    #define LIBNIC_PRINT libc_printf
#else
    #define LIBNIC_PRINT(...) 
#endif

enum
{
    LIBNIC_OK = 0,
    LIBNIC_PARA_ERR,
    LIBNIC_NETDEVICE_ERR,
    LIBNIC_NETIF_ERR,
    LIBNIC_ADDNETIF_ERR,
    LIBNIC_TIMEOUT_ERR,
    LIBNIC_MAX_ERR
};

enum
{
    LIBNIC_DEVICEUSE_NONE = 0,
#ifdef WIFI_OS_SUPPORT
    #ifdef WIFI_SUPPORT
    LIBNIC_DEVICEUSE_WIFI, // accordance with UI
    #endif    
    LIBNIC_DEVICEUSE_ETHERNET,
    #ifdef USB3G_DONGLE_SUPPORT
    LIBNIC_DEVICEUSE_USB3G,
    #endif
    #ifdef HILINK_SUPPORT
    LIBNIC_DEVICEUSE_HILINK,
    #endif
    #ifdef WIFI_SUPPORT
    #ifdef WIFI_P2P_SUPPORT
    LIBNIC_DEVICEUSE_WIFIDIRECT,
    #endif 
    #endif
#else    
    LIBNIC_DEVICEUSE_ETHERNET,
    
    #ifdef WIFI_SUPPORT
    LIBNIC_DEVICEUSE_WIFI,
    #endif
    
    #ifdef USB3G_DONGLE_SUPPORT
    LIBNIC_DEVICEUSE_USB3G,
    #endif
    
    #ifdef HILINK_SUPPORT
    LIBNIC_DEVICEUSE_HILINK,
    #endif

    #ifdef WIFI_SUPPORT
    #ifdef WIFI_P2P_SUPPORT
    LIBNIC_DEVICEUSE_WIFIDIRECT,
    #endif
    #endif
#endif
    LIBNIC_DEVICEUSE_MAX
};

enum
{
    LIBNIC_NETDEVICE_NOEXIST = 0,
    LIBNIC_NETDEVICE_EXIST,
    LIBNIC_NETDEVICE_MAXEXIST
};

int nic_setcur_netdevice(unsigned int ulnetdevicetype);
int nic_getcur_netdevice(unsigned int *pulnetdevicetype);
int nic_switch_netdevice(unsigned int uloldnetdevicetype,unsigned int ulnewnetdevicetype);
int nic_getcard_isexist(unsigned int ulnetdevicetype,unsigned int *pulexiststate);
//int nic_setuser_netdevice(unsigned int ulnetdevicetype);
//int nic_getuser_netdevice(unsigned int *pulnetdevicetype);
//int nic_save_wifiinfo(char *pcapname,unsigned int ulpasswdtype,char *pcpasswd);
//int nic_get_wifiinfo(unsigned int ulindex,char **ppcapname,unsigned int *pulpasswdtype,char **ppcpasswd);

#endif

#endif


