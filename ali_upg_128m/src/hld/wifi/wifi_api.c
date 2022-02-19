#include <sys_config.h>
#include <hld/net/net.h>
#include <hld/wifi/wifi_api.h>
#include <api/libnet/libnet.h>
#include <api/libtcpip/lwip/ip_addr.h>
#ifdef USB3G_DONGLE_SUPPORT
#include <api/libusb3g/lib_usb3g.h>
#endif


typedef INT32 (*RT5370_INIT_CALLBACK)(UINT32);

INT32 (*rt5370_init_func)(UINT32);
INT32 (*mt7601_init_func)(UINT32);

ID	wifi_switch_mutex;

#ifdef WIFI_SUPPORT
/* Function Implement */
int wi_fi_get_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->get_info)
	{
		return dev->get_info(dev,info_type,info_buf);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}
}

int wi_fi_set_info(struct net_device *dev, UINT32 info_type, char*  arg)  //cpl_0312
{
	/*
		This API can set your own mac address for your dervice,
		Note the this API can only called before you calling "WiFi_SwtichMedia" 
		In other words, the mac address should be set before initializing the wifi device , or the initial sequence will fail
	*/

	struct net_device *net_dev;
	//INT	is, mac_len;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;

	if(dev->set_info)
	{
		return dev->set_info(dev,info_type,arg);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}

	
}


int wi_fi_scan_ap(struct net_device *dev)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->scan_ap)
	{
		return dev->scan_ap(dev);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}
}

int wi_fi_get_aplist(struct net_device *dev, struct wifi_ap_info ap_list [ ], int num_ap)
{
	struct net_device *net_dev;

	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}
	if(dev == NULL)
		return -1;
	
	if(dev->get_ap_list)
	{
		return dev->get_ap_list(dev, (UINT32)ap_list, num_ap);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}
}

void wi_fi_connect_ap(struct net_device *dev, struct wifi_ap_info *ap_info)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}
	if(dev == NULL)
		return ;
	
	if(dev->connect_ap)
	{
		dev->connect_ap(dev, (UINT32)ap_info);
	}
	else
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
	return;
}

void wi_fi_dis_connect_ap(struct net_device *dev, struct wifi_ap_info *ap_info)  //cpadd_110421
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return;
	
	if(dev->disconnect_ap)
	{
		dev->disconnect_ap(dev, (UINT32)ap_info);
	}
	else
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
	return;
}
int wi_fi_get_conn_status(struct net_device *dev)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->get_conn_status)
	{
		return dev->get_conn_status(dev);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}
}

int wi_fi_get_plug_status(struct net_device *dev)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->get_plug_status)
	{
		return dev->get_plug_status(dev);
	}
	else {
		//WiFi_PRINTF("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		//return -1;
		return WIFI_MEDIA_DISCONNECTED;
	}	
}

int wi_fi_set_enable_status(struct net_device *dev, int enable)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->set_enable_status)
	{
		if(enable) {
			//dev->set_enable_status(dev, TRUE);
			#if 0//cpl_0312 (sample for set mac cstm assigned address)
			wi_fi_set_info(dev,0,"7c:dd:90:02:3d:11" );  
			#endif
            #ifdef USB3G_DONGLE_SUPPORT/*because only support one net interface in same time.*/
            usb3g_disconnect_all_dongle();	
            #endif
			wi_fi_swtich_media(WIFI_SWITCH_MAC_TO_WIFI);
		}else {
			wi_fi_swtich_media(WIFI_SWITCH_WIFI_TO_MAC);
			//dev->set_enable_status(dev, FALSE);
		}
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}

    return 0;
}

int wi_fi_get_enable_status(struct net_device *dev)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->get_enable_status)
	{
		return dev->get_enable_status(dev);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}	
}

int wi_fi_get_ready_status(struct net_device *dev)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->get_ready_status)
	{
		return dev->get_ready_status(dev);
	}
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}	
}

int wi_fi_get_apquality(struct net_device *dev)
{
	struct net_device *net_dev;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;
	
	if(dev->get_ap_quality) //cpadd_110420_add_quality
		return dev->get_ap_quality(dev);
	else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}
}





#if 0
static int wi_fi_attach_netif(struct net_device *net_dev)
{
	if(net_dev == NULL)
		return -1;

    if(net_dev->flags & HLD_DEV_STATS_ATTACHED)
    {
        wi_fi_printf("%s already attached!\n", net_dev->name);
        return 0;
    }
    
	if (sys_add_netif(IP_ADDR_ANY, IP_ADDR_ANY, IP_ADDR_ANY))
	{
		wi_fi_printf("[ERR] %s: add netif fail...\n", __FUNCTION__);
		return -1;
	}

	IP_LOC_CFG ip_cfg;
	get_local_ip_cfg(&ip_cfg);

#ifdef _INVW_JUICE   
	if(0)
#else
	if(ip_cfg.dhcp_on == 1)
#endif		
	{	
		libnet_dhcp_on();
	}
	else
	{
		libnet_set_ipconfig(&ip_cfg);
	}

    net_dev->flags |= HLD_DEV_STATS_ATTACHED;

	return 0;
}

static int wi_fi_detach_netif(struct net_device *net_dev)
{
	struct netif * cur_netif = NULL;
	
	if(net_dev == NULL)
		return -1;
		
    if((net_dev->flags & HLD_DEV_STATS_ATTACHED) != HLD_DEV_STATS_ATTACHED)
    {
        wi_fi_printf("%s not attached!\n", net_dev->name);
        return 0;
    }
	
	net_close(net_dev);
    net_dev->flags &= ~(HLD_DEV_STATS_ATTACHED);
    
	cur_netif = netif_find("en0");  	
	if(cur_netif) {

	IP_LOC_CFG ip_cfg;
	get_local_ip_cfg(&ip_cfg);

#ifdef _INVW_JUICE
	if(0)
#else
	if(ip_cfg.dhcp_on == 1)
#endif	
	{	
		libnet_dhcp_off();
	}
		
		netif_remove(cur_netif);
        netif_free(cur_netif);
		cur_netif = NULL;	
	}else {
		wi_fi_printf("[ERR] %s: no netif found...\n", __FUNCTION__);
		return -1;
	}

	return 0;
}
#endif

static int wi_fi_attach_netif(struct net_device *net_dev)
{
	if(net_dev == NULL)
		return -1;

    if(net_dev->flags & HLD_DEV_STATS_ATTACHED)
    {
        wi_fi_printf("%s already attached!\n", net_dev->name);
        return 0;
    }

	if (!sys_add_netif2(net_dev,IP_ADDR_ANY, IP_ADDR_ANY, IP_ADDR_ANY))
	{
		wi_fi_printf("[ERR] %s: add netif fail...\n", __FUNCTION__);
		return -1;
	}

    net_dev->flags |= HLD_DEV_STATS_ATTACHED;

	return 0;
}

static int wi_fi_detach_netif(struct net_device *net_dev)
{
	//struct netif * cur_netif = NULL;
	
	if(net_dev == NULL)
		return -1;
		
    if((net_dev->flags & HLD_DEV_STATS_ATTACHED) != HLD_DEV_STATS_ATTACHED)
    {
        wi_fi_printf("%s not attached!\n", net_dev->name);
        return 0;
    }
	
	net_close(net_dev); 
    sys_del_netif(net_dev);

	return 0;
}

int wi_fi_swtich_media(UINT8 type)
{
	struct net_device *net_dev=NULL;
	struct netif * cur_netif __MAYBE_UNUSED__= NULL; 
	struct netif * m_p_netif_wifi __MAYBE_UNUSED__= NULL; 
    struct net_device *p2p_dev __MAYBE_UNUSED__= NULL;

	osal_mutex_lock(wifi_switch_mutex, OSAL_WAIT_FOREVER_TIME);
	
	switch(type) {
		case WIFI_SWITCH_MAC_TO_WIFI:
			net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
			if(net_dev) {
				wi_fi_detach_netif(net_dev);

				#ifdef _INVW_JUICE
				osal_delay(1000);
				#endif
				
				DBG_WIFI_PRINTF(" detach Mac  ------------------>\n");
			}else
				DBG_WIFI_PRINTF(" no need to detach Mac  ------------------>\n");

			net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
			if(net_dev) {
				if(net_dev->set_enable_status)
					net_dev->set_enable_status(net_dev, TRUE);
				wi_fi_attach_netif(net_dev);

				#ifdef _INVW_JUICE
				osal_delay(1000);
				#endif
				
				DBG_WIFI_PRINTF("<------------------  attach WiFi \n");
			}else
				DBG_WIFI_PRINTF("<------------------  can't attach WiFi \n");
			
			break;
			
		case WIFI_SWITCH_WIFI_TO_MAC:
			net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
#ifdef WIFI_P2P_SUPPORT //first close p2p dev
            p2p_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_P2P);
            if(p2p_dev!=NULL){
                if(p2p_get_enable_status(p2p_dev))
                    p2p_set_enable_status( p2p_dev, FALSE);
    }
#endif
			if(net_dev) {
				wi_fi_detach_netif(net_dev);
				if(net_dev->set_enable_status)
					net_dev->set_enable_status(net_dev, FALSE);

				#ifdef _INVW_JUICE
				osal_delay(1000);
				#endif
				
				DBG_WIFI_PRINTF(" detach WiFi  ------------------>\n");
			}else
				DBG_WIFI_PRINTF(" no need to detach WiFi  ------------------>\n");

            #if 0
			net_dev = dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
			if(net_dev) {
				wi_fi_attach_netif(net_dev);

				#ifdef _INVW_JUICE
				osal_delay(1000);
				#endif
				
				DBG_WIFI_PRINTF("<------------------  attach Mac \n");
			}else
				DBG_WIFI_PRINTF("<------------------  no need to attach Mac \n");
			#endif
			break;
			
		default:
			wi_fi_printf("[ERR] %s: wrong swtich type...\n", __FUNCTION__);
			break;
	}

	osal_mutex_unlock(wifi_switch_mutex);

	return 0;
}

RET_CODE wi_fi_detach(struct net_device *dev)
{
	wi_fi_printf("rt5370_detach: Detach net device driver.\n");
	
	#ifdef DUAL_ENABLE
	dev_en_remote(0);
	#endif
	dev_free(dev);
	#ifdef DUAL_ENABLE
	dev_en_remote(1);
	#endif
	dev = NULL ;

	osal_mutex_delete(wifi_switch_mutex);
	
	return RET_SUCCESS;
}

RET_CODE wi_fi_attach(void)
{
	struct net_device *dev;

	//alloc memory space for net device.
	dev = (struct net_device *)dev_alloc((INT8*)wifi_dev_name, HLD_DEV_TYPE_WIFI, sizeof(struct net_device));
	if (dev == NULL)
	{
		wi_fi_printf("[ERR] %s: alloc net device error!\n", __FUNCTION__);
		return ERR_NO_MEM;
	}

	
	dev->next = NULL;

	//init function point (some functions assigned latter)
	dev->attach = (void *)wi_fi_attach;
	dev->detach = wi_fi_detach;
	dev->open = NULL;
	dev->close = NULL;
	dev->ioctl = NULL;
	dev->send_packet = NULL;
	dev->callback = NULL;
	dev->get_info = NULL;
	dev->set_info = NULL;
	dev->scan_ap = NULL;
	dev->get_ap_list = NULL;
	dev->connect_ap = NULL;
	dev->get_conn_status = NULL;

	/* Add this device to queue */ 
	#ifdef DUAL_ENABLE
	dev_en_remote(0);
	#endif
	if (dev_register(dev) != RET_SUCCESS)
	{
		wi_fi_printf("Register net device error!\n");
		dev_free(dev);
		#ifdef DUAL_ENABLE
		dev_en_remote(1);
		#endif
		return RET_FAILURE;
	}
	#ifdef DUAL_ENABLE
	dev_en_remote(1);
	#endif
	wifi_switch_mutex = osal_mutex_create();
	wi_fi_printf("Register WiFi net dev:%08x (priv: %08x)\n", dev, dev->priv);

    return 0;
}

#if 1
int wi_fi_swtich_media2(UINT8 type)
{
    struct net_device *net_dev=NULL;

    if (WIFI_SWITCH_MAX <= type)
    {
        wi_fi_printf("%s:%d:error.\r\n",__FUNCTION__,__LINE__);
        return -1;
    }

    osal_mutex_lock(wifi_switch_mutex, OSAL_WAIT_FOREVER_TIME);
    switch(type) 
    {
        case WIFI_SWITCH_TO_WIFI:
        {
            net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
            if(net_dev) 
            {
                if(net_dev->set_enable_status)
                {
                    net_dev->set_enable_status(net_dev, TRUE);
                }
                wi_fi_attach_netif(net_dev);

                #ifdef _INVW_JUICE
                osal_delay(1000);
                #endif
            }
            else
            {
                DBG_WIFI_PRINTF("<------------------  can't attach WiFi \n");
            }
        }
        break;

    case WIFI_SWITCH_TO_OTHER:
    {
        net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);

        if(net_dev) 
        {
            wi_fi_detach_netif(net_dev);
            if(net_dev->set_enable_status)
            {
                net_dev->set_enable_status(net_dev, FALSE);
            }

            #ifdef _INVW_JUICE
            osal_delay(1000);
            #endif

            DBG_WIFI_PRINTF(" detach WiFi  ------------------>\n");
        }
        else
        {
            DBG_WIFI_PRINTF(" no need to detach WiFi  ------------------>\n");
        }
    }
    break;

    default:
        wi_fi_printf("[ERR] %s: wrong swtich type...\n", __FUNCTION__);
    break;
    }
    osal_mutex_unlock(wifi_switch_mutex);

    return 0;
}


int wi_fi_set_enable_status2(struct net_device *dev, int enable)
{
    struct net_device *net_dev;

    net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
    if(net_dev!=dev)
    {
        dev=net_dev;
    }

    if(dev == NULL)
    return -1;

    if(dev->set_enable_status)
    {
        if(enable)
        {
            wi_fi_swtich_media2(WIFI_SWITCH_TO_WIFI);
        }
        else 
        {
            wi_fi_swtich_media2(WIFI_SWITCH_TO_OTHER);
            //dev->set_enable_status(dev, FALSE);
        }
    }
    else 
    {
        wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

#endif

#ifdef WIFI_P2P_SUPPORT
void p2p_register_send_event_callback(void *callback)
{

	struct net_device *dev;

	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_P2P);
	if (dev == NULL)
	{
		return -1;
	} 
	if(callback)
	{
		dev->p2p_send_event_callback = callback;
	}
}

int p2p_get_enable_status(struct net_device *dev)
{
    BOOL ret = FALSE;
	if(dev == NULL)
		return -1;
	
	if(dev->get_p2p_enable_status)
	{
		return dev->get_p2p_enable_status(dev);
	}
	else 
	{
		wi_fi_printf("[ERR]%s: WiFi P2P device not enable\n", __FUNCTION__);
		return ret;
	}	
}

RET_CODE p2p_attach(void)
{
	struct net_device *dev;

	//alloc memory space for net device.
	dev = dev_alloc(p2p_dev_name, HLD_DEV_TYPE_P2P, sizeof(struct net_device));
	if (dev == NULL)
	{
		wi_fi_printf("[ERR] %s: alloc net device error!\n", __FUNCTION__);
		return ERR_NO_MEM;
	}

	
	dev->next = NULL;

	//init function point (some functions assigned latter)
	dev->attach = p2p_attach;
	dev->detach = wi_fi_detach;
	dev->open = NULL;
	dev->close = NULL;
	dev->ioctl = NULL;
	dev->send_packet = NULL;
	dev->callback = NULL;
	dev->get_info = NULL;
	dev->set_info = NULL;
	dev->scan_ap = NULL;
	dev->get_ap_list = NULL;
	dev->connect_ap = NULL;
	dev->get_conn_status = NULL;

	/* Add this device to queue */ 
	#ifdef DUAL_ENABLE
	dev_en_remote(0);
	#endif
	if (dev_register(dev) != RET_SUCCESS)
	{
		wi_fi_printf("Register net device error!\n");
		dev_free(dev);
		#ifdef DUAL_ENABLE
		dev_en_remote(1);
		#endif
		return RET_FAILURE;
	}
	#ifdef DUAL_ENABLE
	dev_en_remote(1);
	#endif
	wifi_switch_mutex = osal_mutex_create();
	wi_fi_printf("Register WiFi net dev:%08x (priv: %08x)\n", dev, dev->priv);
	
}

int p2p_attach_netif(struct net_device *net_dev)
{
    struct netif *netif = NULL;
    
	if(net_dev == NULL)
		return -1;

    netif = netif_find_by_dev(net_dev);
    if(netif)
    {
        wi_fi_printf("%s already attached!\n", net_dev->name);
        return 0;
    }
    if(net_dev->flags & HLD_DEV_STATS_ATTACHED)
    {
        wi_fi_printf("%s already attached!\n", net_dev->name);
        return 0;
    }
	netif = sys_add_netif2(net_dev, IP_ADDR_ANY, IP_ADDR_ANY, IP_ADDR_ANY);
    if(netif==NULL)
	{
		wi_fi_printf("[ERR] %s: add netif fail...\n", __FUNCTION__);
		return -1;
	}

	//IP_LOC_CFG ip_cfg;
	//get_local_ip_cfg(&ip_cfg);

	//if(ip_cfg.dhcp_on == 1)		
	//{	
	//	libnet_dhcp_on2(netif);
	//}
	//else
	//{
	//	libnet_set_ipconfig2(netif, &ip_cfg);
	//}
    net_dev->flags |= HLD_DEV_STATS_ATTACHED;

	return 0;
}

static int p2p_detach_netif(struct net_device *net_dev)
{
	struct netif * cur_netif = NULL;

	if(net_dev == NULL)
		return -1;

    net_close(net_dev);    
    sys_del_netif(net_dev);

	return 0;
}

int p2p_set_enable_status(struct net_device *dev, int enable)
{
    struct net_device *net_dev;
    int ret=0;
	
	net_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_P2P);
	if(net_dev!=dev)
	{
		dev=net_dev;
	}

	if(dev == NULL)
		return -1;

	if(dev->set_p2p_enable_status)
	{
		if(enable) 
        {
            if(p2p_get_enable_status(dev) == 0){//0:p2p disable,1:enable,net_open
                ret=p2p_attach_netif(dev); 	
                if(ret!=0)
                {
                	libc_printf("p2p  AttachNetif  fail\n");
                	return -1;
                }
            }
            return dev->set_p2p_enable_status(dev, TRUE);
	    }
        else 
        {
            if(p2p_get_enable_status(dev))//1:enable,net_close
                p2p_detach_netif(dev);
            return dev->set_p2p_enable_status(dev, FALSE);
        }
	}
    else {
		wi_fi_printf("[ERR]%s: WiFi device plug out or not init done\n", __FUNCTION__);
		return -1;
	}

}

int wi_fi_device_ioctl(const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
	struct net_device	*dev;
	int ret;
	
	dev = dev_get_by_name((INT8 *)ifname);
	if(dev == NULL)
		return -1;
    if(dev->ioctl)
	{
	    ret = dev->ioctl(dev, request, (UINT32)pwrq);
    }
	/* Do the request */
	return (ret)?-1:0;
}

int p2p_set_device_name(struct net_device *dev, void *buffer)
{
    struct iwreq		wrq;
    unsigned char local_device_name[33];
    memset( local_device_name, 0, sizeof(local_device_name));
    
	if(dev == NULL || buffer == NULL)
		return -1;
    snprintf( local_device_name, sizeof(local_device_name), "%s", buffer );
	wrq.u.data.pointer = &local_device_name;
    wrq.u.data.length = (unsigned short)strlen(local_device_name);
    wrq.u.data.flags = RT_OID_802_11_P2P_DEVICE_NAME;
    if(wi_fi_device_ioctl( "p2p0", RT_PRIV_IOCTL, &wrq)==0)
        return 0;
    else{
        wi_fi_printf("p2p set device name error!\n");
        return -1;
    }
}

#endif

#else

int wi_fi_swtich_media(UINT8 type)
{
	return RET_FAILURE;
}
#endif

#ifdef WIFI_RT5370_SUPPORT
INT32 rt5370_init(UINT32  p_node);
#endif
#ifdef WIFI_MT7601_SUPPORT
INT32 mt7601_init(UINT32  p_node);
#endif
#ifdef WIFI_RTL8188_SUPPORT
INT32 rtl8188_init(UINT32  p_node);
#endif
int wi_fi_device_enable(BOOL b_enable)
{

	struct net_device *dev;

	//alloc memory space for net device.
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if (dev == NULL)
	{
		//WiFi_PRINTF("[ERR] %s: alloc net device error!\n", __FUNCTION__);
		return ERR_NO_MEM;
	} 

#ifdef WIFI_SUPPORT
#ifdef WIFI_MT7601_SUPPORT
    dev->mt7601_callback= (void*)mt7601_init;
#endif
#ifdef WIFI_RT5370_SUPPORT
    dev->rt5370_callback=(void*)rt5370_init;
#endif
#ifdef WIFI_RTL8188_SUPPORT
	dev->rtl8188_callback=(void*)rtl8188_init;
#endif
    if(sys_ic_get_chip_id() == ALI_S3503 || sys_ic_get_chip_id() == ALI_S3821 \
		|| sys_ic_get_chip_id() == ALI_C3505 || sys_ic_get_chip_id() == ALI_C3702 \
		|| sys_ic_get_chip_id() == ALI_C3711C){
#ifdef WIFI_MT7601_SUPPORT
        dev->mt7601_callback(TRUE);
#endif
#ifdef WIFI_RT5370_SUPPORT
        dev->rt5370_callback(TRUE);
#endif
#ifdef WIFI_RTL8188_SUPPORT
        dev->rtl8188_callback(TRUE);
#endif
    }
#else
    dev->mt7601_callback=NULL;
	dev->rt5370_callback=NULL;
#endif

	dev->wifi_device_enable = b_enable;

    return 0;
	
}


int wi_fi_set_device_pid_vid(UINT16 pid,UINT16 vid)
{

	struct net_device *dev;
	int cnt=0;
	//alloc memory space for net device.
	dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if (dev == NULL)
	{
		//WiFi_PRINTF("[ERR] %s: alloc net device error!\n", __FUNCTION__);
		return ERR_NO_MEM;
	} 

	if(cnt>10)
	{
		libc_printf("only support max 10 customer device pid vid\n");
		return FALSE;
	}

	for(cnt=0; cnt<10 ; cnt++)
	{
		if(dev->customer_vid[cnt]==0)  
			break;
	}

 	dev->customer_vid[cnt]=vid;
	dev->customer_pid[cnt]=pid;
	
	libc_printf("cust = %d ; vid = 0x%x ; pid = 0x%x \n",cnt,dev->customer_vid[cnt],dev->customer_pid[cnt]);

	return TRUE;

}







