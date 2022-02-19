#include <sys_config.h>
#include <sys_config.h>
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
#include <hld/decv/decv.h>
#include <api/libwifi/lib_wifi.h>

#ifdef WIFI_DIRECT_SUPPORT

//#define WIFI_DIRECT_PRT

#ifdef WIFI_DIRECT_PRT
    #define WFD_PRINTF          libc_printf
#else
    #define WFD_PRINTF(...)     do{}while(0)
#endif


void wifi_direct_set_device_name(char *name);

#if defined(WIFI_P2P_SUPPORT) && defined(WIFI_DIRECT_SUPPORT)

static void wifi_direct_msg_callback(UINT32 handle,UINT32 event_type, UINT32 wrqu, UINT32 data)
{
    union	iwreq_data  *ireq_data = NULL;
    WFD_PRINTF("1:0x%x,2:%d,3:0x%x,4:0x%x \n",handle,event_type,wrqu,data);
    if(event_type == IWEVCUSTOM )//RT_P2P_AP_STA_OUT
    {
        ireq_data = (union	iwreq_data *)wrqu;
        if((ireq_data != NULL) && (ireq_data->data.flags == RT_P2P_AP_STA_OUT))
        {
            if(sys_data_get_wifi_direct_onoff())
            {
                #if 0
                api_wifi_enable_p2p_function(FALSE); //disable p2p
                //api_wifi_enable_device_function(FALSE); //disable wifi
                api_wifi_direct_enable_device_function(FALSE);
                #endif
            }   
        }
        else if((ireq_data != NULL) && (ireq_data->data.flags == RT_P2P_DISCONNECTED))
        {
            struct netif      *net_if = NULL;
            struct net_device *net_dev = (struct net_device *)handle;  
            
            if(net_dev != NULL)
            {
                net_if = netif_find_by_dev(net_dev);
                if(net_if != NULL)
                {
                    netif_set_down(net_if);
                }
            }            
        }
        else if((ireq_data != NULL) && (ireq_data->data.flags == RT_P2P_AP_STA_CONNECTED))
        {
            struct netif      *net_if = NULL;
            struct net_device *net_dev = (struct net_device *)handle;
            
            if(net_dev != NULL)
            {
                net_if = netif_find_by_dev(net_dev);
                if(net_if != NULL)
                {
                    netif_set_up(net_if);
                }
                if(net_if!=NULL&&sys_data_get_wifi_direct_onoff())
                {
                    libnet_dhcp_on2(net_if);
                }
            }
        }
    }
}

void wifi_direct_register_callback(void)
{    
    p2p_register_send_event_callback((void *)wifi_direct_msg_callback);
}

void wifi_direct_set_device_name(char *name)
{
    struct net_device *net_dev = NULL;
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_P2P);    
    if((name != NULL) && (net_dev !=NULL))
    {
        p2p_set_device_name(net_dev,name);
    }
}

#endif


#endif
