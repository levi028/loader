#include <sys_config.h>

#ifdef NETWORK_SUPPORT
#include <basic_types.h>
#include <hld/hld_dev.h>
#include <hld/net/net.h>
#include <api/libnet/libnet.h>
#include <api/libtcpip/lwip/ip_addr.h>
#include <api/libtcpip/lwip/netif.h>
#include <api/libwifi/lib_wifi.h>
#include <api/libusb3g/lib_usb3g.h>
#include <api/libhilink/lib_hilink.h>
#include <api/libnic/lib_nic.h>

#define HILINK_WAIT_TIME 15000
#define WIFI_WAIT_TIME   15000
#define WIFIDIRECT_WAIT_TIME 20000
unsigned int g_ulnetdeviceuse = 0;

extern struct netif *netif_find_by_dev(struct net_device * net_device);
extern void get_local_ip_cfg(PIP_LOC_CFG pcfg);

int ethernet_open( )
{
    struct net_device *pstrnetdev = NULL;
    struct netif *pstrnetif = NULL;
    IP_LOC_CFG stripcfg ;

    MEMSET(&stripcfg,0,sizeof(stripcfg));
    pstrnetdev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
    if (pstrnetdev == NULL)
    {
        LIBNIC_PRINT("%s:%d:NULL DEVICE.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }

    if (pstrnetdev->flags & HLD_DEV_STATS_ATTACHED)
    {
        LIBNIC_PRINT("%s:%d:already attached.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_OK;
    }

    pstrnetif = sys_add_netif2(pstrnetdev,IP_ADDR_ANY,IP_ADDR_ANY,IP_ADDR_ANY);
    if (NULL == pstrnetif)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }
    
    get_local_ip_cfg(&stripcfg);
#ifdef _INVW_JUICE
    if (0)
#else
    if (stripcfg.dhcp_on == 1)
#endif
    {
        libnet_dhcp_on2(pstrnetif);
    }
    else
    {
        libnet_set_ipconfig(&stripcfg);
    }

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_ETHERNET);
    return LIBNIC_OK;
}

int ethernet_close()
{
    struct netif *pstrnetif = NULL;
    struct net_device *pstrnetdev = NULL;

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_NONE);
    pstrnetdev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
    if (pstrnetdev == NULL)
    {
        LIBNIC_PRINT("%s:%d:NULL DEVICE.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_OK;
    }

    pstrnetif = netif_find_by_dev(pstrnetdev);
    if (pstrnetif)
    {
        #ifdef _INVW_JUICE
            if (0)
        #else
            //if(ip_cfg.dhcp_on == 1)
        #endif
            {
                libnet_dhcp_off2(pstrnetif);
            }

            netif_remove(pstrnetif);
            netif_free(pstrnetif);
    }

    net_close(pstrnetdev);
    pstrnetdev->flags &= ~(HLD_DEV_STATS_ATTACHED);

    return LIBNIC_OK;
}

#ifdef WIFI_SUPPORT
int wifi_open()
{
    int slret = 0;
    unsigned int ultotal = 0;
    struct net_device *pstrnetdev = NULL;

    pstrnetdev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
    if (NULL == pstrnetdev)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }
    if(pstrnetdev->get_plug_status == NULL || FALSE == pstrnetdev->get_plug_status(pstrnetdev))
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }
    
    slret = api_wifi_netif_function(TRUE);
    if (TRUE != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }

    while (0 == (pstrnetdev->flags & HLD_DEV_STATS_ATTACHED) )
    {
        osal_task_sleep(20);
        ultotal += 20;
        if (WIFI_WAIT_TIME < ultotal)
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_TIMEOUT_ERR;
        }
    }

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_WIFI);
    return LIBNIC_OK;
}

int wifi_close()
{
    int slret = 0;
    unsigned int ultotal = 0;
    struct net_device *pstrnetdev = NULL;

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_NONE);
    pstrnetdev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
    if (NULL == pstrnetdev)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }
    
    slret = api_wifi_netif_function(FALSE);
    if (TRUE != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }

    while (pstrnetdev->flags & HLD_DEV_STATS_ATTACHED) 
    {
        osal_task_sleep(20);
        ultotal += 20;
        if (WIFI_WAIT_TIME < ultotal)
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_TIMEOUT_ERR;
        }
    }
    
    return LIBNIC_OK;
}

int wifi_plug_isexist(struct net_device *pstrnetdev,unsigned int *pulexiststate)
{
    if ((NULL == pstrnetdev)||(NULL == pulexiststate))
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }
    
    if (pstrnetdev->get_plug_status)
    {
        if (TRUE == pstrnetdev->get_plug_status(pstrnetdev))
        {
            *pulexiststate = LIBNIC_NETDEVICE_EXIST;
        }
        else
        {
            *pulexiststate = LIBNIC_NETDEVICE_NOEXIST;
        }
    }
    else
    {
        *pulexiststate = LIBNIC_NETDEVICE_NOEXIST;
    }

    return LIBNIC_OK;
}

#ifdef WIFI_P2P_SUPPORT
int wifidirect_open()
{
    int slret = 0;
    unsigned int ultotal = 0;
    struct net_device *pstrnetdev = NULL;
    
    slret = api_wifi_direct_enable_device_function(TRUE);
    if (TRUE != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }
    
    slret = api_wifi_enable_p2p_function(TRUE); 
    if (TRUE != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }
                      
    while(1) 
    {
        if(api_wifi_p2p_get_enable_status() != FALSE)
        {
            break;
        }
        
        osal_task_sleep(20);
        ultotal += 20;
        if (WIFIDIRECT_WAIT_TIME < ultotal)
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_TIMEOUT_ERR;
        }
    }  

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_WIFIDIRECT);
    return LIBNIC_OK;
}

int wifidirect_close()
{
    int slret = 0;
    unsigned int ultotal = 0;
    struct net_device *pstrnetdev = NULL;

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_NONE);
    slret = api_wifi_enable_p2p_function(FALSE); 
    if (TRUE != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }
    
    slret = api_wifi_direct_enable_device_function(FALSE);
    if (TRUE != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }

    pstrnetdev = dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
    if (NULL == pstrnetdev)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }

    while (pstrnetdev->flags & HLD_DEV_STATS_ATTACHED) 
    {
        osal_task_sleep(20);
        ultotal += 20;
        if (WIFIDIRECT_WAIT_TIME < ultotal)
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_TIMEOUT_ERR;
        }
    }

    return LIBNIC_OK;
}

int wifidirect_isexist(struct net_device *pstrnetdev,unsigned int *pulexiststate)
{
    int slret = 0;
    unsigned int ulexist = 0;
    
    if ((NULL == pstrnetdev)||(NULL == pulexiststate))
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }

    (void)wifi_plug_isexist(pstrnetdev, &ulexist);
    if (LIBNIC_NETDEVICE_EXIST == ulexist)
    {
        slret = api_wifi_check_p2p_support();
        if (TRUE == slret)
        {
            *pulexiststate = LIBNIC_NETDEVICE_EXIST;
        }
        else
        {
            *pulexiststate = LIBNIC_NETDEVICE_NOEXIST;
        }
    }
    else
    {
        *pulexiststate = LIBNIC_NETDEVICE_NOEXIST;
    }

    return LIBNIC_OK;
}
#endif
#endif

#ifdef USB3G_DONGLE_SUPPORT
int usb3g_open()
{
    int slret = 0;
    int slcount = 0;    
    struct usb3gdongle *pstrusb3gdongle = NULL;

    slcount = usb3g_dongle_count();
    if (0 == slcount)
    {
        LIBNIC_PRINT("%s:%d:count = 0.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }

    pstrusb3gdongle = usb3g_get_dongle_by_index(0);
    if (NULL == pstrusb3gdongle)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_NETDEVICE_ERR;
    }

    slret = usb3g_start_dail(pstrusb3gdongle);
    if (0 == slret)
    {
       LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_USB3G);
    return LIBNIC_OK;
}

int usb3g_close()
{
    nic_setcur_netdevice(LIBNIC_DEVICEUSE_NONE);
    usb3g_disconnect_all_dongle();
    return LIBNIC_OK;
}
#endif

#ifdef HILINK_SUPPORT
int hilink_open()
{
    int slret = 0;
    unsigned int ulInitflag = 0xff;
    unsigned int ultotal = 0;

    hilink_set_useflag(HILINK_USEFLAG_USE);
    slret = hilink_init();
    if (LIBNIC_OK != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_ADDNETIF_ERR;
    }

    (void)hilink_get_initflag(&ulInitflag);
    while (1 != ulInitflag)
    {
        osal_task_sleep(20);
        ultotal += 20;
        if (HILINK_WAIT_TIME < ultotal)
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_TIMEOUT_ERR;
        }
        (void)hilink_get_initflag(&ulInitflag);
    }

    nic_setcur_netdevice(LIBNIC_DEVICEUSE_HILINK);
    return LIBNIC_OK; 
}

int hilink_close()
{
    unsigned int ulInitflag = 0xff;
    unsigned int ultotal = 0;
    
    nic_setcur_netdevice(LIBNIC_DEVICEUSE_NONE);
    hilink_set_useflag(HILINK_USEFLAG_NOUSE);
    (void)hilink_get_initflag(&ulInitflag);
    while (0 != ulInitflag)
    {
        osal_task_sleep(20);
        ultotal += 20;
        if (HILINK_WAIT_TIME < ultotal)
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_TIMEOUT_ERR;
        }
        (void)hilink_get_initflag(&ulInitflag);
    }

    return LIBNIC_OK;
}
#endif

int nic_setcur_netdevice(unsigned int ulnetdevicetype)
{
    if (LIBNIC_DEVICEUSE_MAX <= ulnetdevicetype)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }
    
    g_ulnetdeviceuse = ulnetdevicetype;
    return LIBNIC_OK;
}

int nic_getcur_netdevice(unsigned int *pulnetdevicetype)
{
    if (NULL == pulnetdevicetype)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }

    *pulnetdevicetype = g_ulnetdeviceuse;
    return LIBNIC_OK;
}

int nic_switch_netdevice(unsigned int uloldnetdevicetype,unsigned int ulnewnetdevicetype)
{
    int slret = 0;
    
    if ((uloldnetdevicetype >= LIBNIC_DEVICEUSE_MAX)||(ulnewnetdevicetype >= LIBNIC_DEVICEUSE_MAX))
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }
#ifdef WIFI_OS_SUPPORT
    if(uloldnetdevicetype == ulnewnetdevicetype)
    {
        return LIBNIC_OK;
    }
#endif    
    LIBNIC_PRINT("%s:old=%d,new=%d.\r\n",__FUNCTION__,uloldnetdevicetype,ulnewnetdevicetype);
    switch (uloldnetdevicetype)
    {
        case LIBNIC_DEVICEUSE_NONE:
        break;

        case LIBNIC_DEVICEUSE_ETHERNET:
            slret = ethernet_close();
        break;

        #ifdef WIFI_SUPPORT
        case LIBNIC_DEVICEUSE_WIFI:
            slret = wifi_close();
        #ifdef WIFI_OS_SUPPORT
            WiFi_DeviceEnable(FALSE);
        #endif
        break;
        #endif

        #ifdef USB3G_DONGLE_SUPPORT
        case LIBNIC_DEVICEUSE_USB3G:
            slret = usb3g_close();
        break;
        #endif

        #ifdef HILINK_SUPPORT
        case LIBNIC_DEVICEUSE_HILINK:
            slret = hilink_close();
        break;
        #endif

        #ifdef WIFI_SUPPORT
        #ifdef WIFI_P2P_SUPPORT 
        case LIBNIC_DEVICEUSE_WIFIDIRECT:
            slret = wifidirect_close();
        break;
        #endif
        #endif

        default:
        break;
    }

    if (LIBNIC_OK != slret)
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return slret;
    }

    switch(ulnewnetdevicetype)
    {
        case LIBNIC_DEVICEUSE_NONE:
            slret = nic_setcur_netdevice(ulnewnetdevicetype);
        break;

        case LIBNIC_DEVICEUSE_ETHERNET:
            slret = ethernet_open();
        break;

        #ifdef WIFI_SUPPORT
        case LIBNIC_DEVICEUSE_WIFI:
            slret = wifi_open();
        break;
        #endif

        #ifdef USB3G_DONGLE_SUPPORT
        case LIBNIC_DEVICEUSE_USB3G:
            slret = usb3g_open();
        break;
        #endif

        #ifdef HILINK_SUPPORT
        case LIBNIC_DEVICEUSE_HILINK:
            slret = hilink_open();
        break;
        #endif

        #ifdef WIFI_SUPPORT
        #ifdef WIFI_P2P_SUPPORT
        case LIBNIC_DEVICEUSE_WIFIDIRECT:
            slret = wifidirect_open();
        break;
        #endif
        #endif

        default:
        break;
    }

    return slret;
}

int nic_getcard_isexist(unsigned int ulnetdevicetype,unsigned int *pulexiststate)
{
    unsigned int uldevicetype = 0;
    struct net_device *pstrnetdev = NULL;
#ifdef WIFI_SUPPORT
    unsigned int ulexisttemp = 0;
#endif

    #if 0
    if ((LIBNIC_DEVICEUSE_NONE == ulnetdevicetype)||(LIBNIC_DEVICEUSE_MAX <= ulnetdevicetype)||(NULL == pulexiststate))
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }
    #endif
    if ((LIBNIC_DEVICEUSE_MAX <= ulnetdevicetype)||(NULL == pulexiststate))
    {
        LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return LIBNIC_PARA_ERR;
    }

    switch (ulnetdevicetype)
    {
        case LIBNIC_DEVICEUSE_NONE:
        {
             *pulexiststate = LIBNIC_NETDEVICE_EXIST;   
             return LIBNIC_OK;
        }
        break;
        
        case LIBNIC_DEVICEUSE_ETHERNET:
        {
            uldevicetype = HLD_DEV_TYPE_NET;
        }
        break;

        #ifdef WIFI_SUPPORT
        case LIBNIC_DEVICEUSE_WIFI:
        {
            uldevicetype = HLD_DEV_TYPE_WIFI;
        }
        break;
        #endif

        #ifdef USB3G_DONGLE_SUPPORT
        case LIBNIC_DEVICEUSE_USB3G:
        {
            uldevicetype = HLD_DEV_TYPE_3G;
        }
        break;
        #endif

        #ifdef HILINK_SUPPORT
        case LIBNIC_DEVICEUSE_HILINK:
        {
            uldevicetype = HLD_DEV_TYPE_USBNET;
        }
        break;
        #endif

        #ifdef WIFI_SUPPORT
        #ifdef WIFI_P2P_SUPPORT
        case LIBNIC_DEVICEUSE_WIFIDIRECT:
        {
            uldevicetype = HLD_DEV_TYPE_WIFI;
        }
        break;
        #endif
        #endif

        default:
        {
            LIBNIC_PRINT("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
            return LIBNIC_PARA_ERR   ;
        }   
    }

    pstrnetdev = (struct net_device *)dev_get_by_type(NULL, uldevicetype);
    if (NULL == pstrnetdev)
    {
        *pulexiststate = LIBNIC_NETDEVICE_NOEXIST;
    }
    else
    {
        #ifdef WIFI_SUPPORT
        if (LIBNIC_DEVICEUSE_WIFI == ulnetdevicetype)
        {
            (void)wifi_plug_isexist(pstrnetdev,&ulexisttemp);
            *pulexiststate = ulexisttemp;
        }
        #ifdef WIFI_P2P_SUPPORT
        else if (LIBNIC_DEVICEUSE_WIFIDIRECT == ulnetdevicetype)
        {
            (void)wifidirect_isexist(pstrnetdev,&ulexisttemp);
            *pulexiststate = ulexisttemp;
        } 
        #endif
        else
        #endif
        {
            *pulexiststate = LIBNIC_NETDEVICE_EXIST;
        }
    }

    return LIBNIC_OK;
}

#if 0
int nic_setuser_netdevice(unsigned int ulnetdevicetype)
{
    ;
}

int nic_getuser_netdevice(unsigned int *pulnetdevicetype)
{
    ;
}

int nic_save_wifiinfo(char *pcapname,unsigned int ulpasswdtype,char *pcpasswd)
{
    ;
}

int nic_get_wifiinfo(unsigned int ulindex,char **ppcapname,unsigned int *pulpasswdtype,char **ppcpasswd)
{
    ;
}
#endif

#endif

