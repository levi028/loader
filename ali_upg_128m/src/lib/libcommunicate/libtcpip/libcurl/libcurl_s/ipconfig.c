/* -*- DHCP set can be manual of automatical. -*- */

//#include "config.h"
#include <api/libtcpip/lwip/netif.h>
#include <api/libtcpip/lwip/dns.h>
#include <api/libtcpip/lwip/inet.h>
#include <api/libnet/libnet.h>
#ifndef LIB_W5300
#include <api/libtcpip/netif/etharp.h>
#include <api/libtcpip/lwip/netifapi.h>
#include <api/libtcpip/lwip/tcpip.h>
#endif
#include <hld/hld_dev.h>
#include <hld/net/net.h>

#ifndef WIN32
//#define LIBNET_DEBUG(fmt, args...) libc_printf("net-> " fmt, ##args)
#define LIBNET_DEBUG(...)	do{}while(0)
#else
#define LIBNET_DEBUG printf
#endif

#ifdef NETWORK_SUPPORT
#ifndef LIB_W5300
extern int ethernet_open( );
extern err_t ethernetif_init(struct netif *netif);
extern err_t tcpip_input(struct pbuf *p, struct netif *inp);
extern libnet_callback libnet_cb;
extern struct netif *netif_find_by_dev(struct net_device * net_device);

struct netif * sys_add_netif2(struct net_device *dev, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw)   // WiFi Support (need to call this function directly...)
{
	struct netif *new_netif = NULL;
    struct ethernetif *ethif = NULL;
	struct netif *ret = NULL;
    
    if(dev == NULL) 
	{
		return NULL;
	}
    new_netif = netif_find_by_dev(dev);
    if (new_netif) //already exist
    {
        return new_netif;
    }
    new_netif = MALLOC(sizeof(struct netif));
    if (new_netif == NULL)
    {
        return new_netif;
    }
    
    ethif = MALLOC(sizeof(struct ethernetif));
    if (ethif == NULL)
    {
        FREE(new_netif);
        return NULL;
    }
    
    MEMSET(new_netif, 0, sizeof(struct netif));
    MEMSET(ethif, 0, sizeof(struct ethernetif));
    ethif->dev = dev;
    
    netif_set_name(new_netif, "en");
	ret = netif_add(new_netif, ipaddr, netmask, gw, (void *)ethif, ethernetif_init, tcpip_input);
    if (NULL == ret)
	{
        FREE(new_netif);
        FREE(ethif);
		LWIP_DEBUGF(SYS_DEBUG, ("%s: Add network interface failed!\n", __FUNCTION__));
		return NULL;
	}
#ifdef _INVW_JUICE
#else
	netif_set_up(new_netif);
#endif
    netif_set_default(new_netif);
    dev->flags |= HLD_DEV_STATS_ATTACHED;
	return new_netif;
}

err_t sys_add_netif(struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw)   // WiFi Support (need to call this function directly...)
{
    struct net_device *dev = NULL;
	
	dev =  (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if(dev == NULL 
		|| !(dev->get_plug_status && dev->get_plug_status(dev) == TRUE 
				&& dev->get_enable_status && dev->get_enable_status(dev) == TRUE))
    {
        dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
    }
    
    if(dev == NULL)
	{
		return ERR_MEM;
	}
	if(sys_add_netif2(dev, ipaddr, netmask, gw))
	{
		return ERR_OK;
	}
    return ERR_MEM;
}

void sys_del_netif(struct net_device *dev)
{
	struct netif *netif = NULL;
	
    if(NULL == dev)
	{
		return;
	}
    netif = netif_find_by_dev(dev);
    if(NULL == netif)
    {    
        return;
    }    
    #ifdef AIRPLAY_ENABLE
    avahi_network_notify(1);
    #endif
    libnet_dhcp_off2(netif);
    netif_remove(netif);
    netif_free(netif);
    dev->flags &= ~(HLD_DEV_STATS_ATTACHED);
}

int network_init(struct libnet_config *pcfg)
{
	tcpip_init(NULL, NULL, &pcfg->lwip_cfg);

#ifdef _INVW_JUICE
	return 0; /* sys_add_netif - deferred - called by Juice initialisation */
#endif
   
#ifdef WIFI_SUPPORT
    if(ERR_OK == wifi_open())
    {
        return 0;
    }
#endif

    if (ERR_OK != ethernet_open())
    {
        return -1;
    }
    
	return 0;
}


//obsolete api, should use new api "libnet_get_ipconfig2"
int libnet_get_ipconfig(struct ip_local_cfg *cfg)
{
    struct netif *netif = NULL;
	
	netif = netif_find("en0");
    return libnet_get_ipconfig2(netif, cfg);
}

int libnet_get_ipconfig2(struct netif *netif, struct ip_local_cfg *cfg)
{
	if (netif == NULL)
	{	
		LIBNET_DEBUG("Can not find netif named <en>!\n");
		return -1;
	}	
	cfg->ip_addr = netif->ip_addr.addr;
	cfg->gateway = netif->gw.addr;
	cfg->subnet_mask = netif->netmask.addr;	
	cfg->dns = dns_getserver(0).addr;
	cfg->dns2 = dns_getserver(1).addr;
	return 0;
}

//obsolete api, should use new api "libnet_set_ipconfig2"
int libnet_set_ipconfig(const struct ip_local_cfg *cfg)
{
    struct netif *netif = NULL;
	
	netif = netif_find("en0");
    return libnet_set_ipconfig2(netif, cfg);
}

int libnet_set_ipconfig2(struct netif *netif, const struct ip_local_cfg *cfg)
{
	struct ip_addr ipaddr, gw, netmask;

	if (NULL == netif)
	{
		LIBNET_DEBUG("Can not find netif named <en>!\n");
		return -1;
	}
	//set the ip address	
	ipaddr.addr = cfg->ip_addr;
	netmask.addr = cfg->subnet_mask;
	gw.addr = cfg->gateway;
	netif_set_addr(netif, &ipaddr, &netmask, &gw);

	//set the dns server
	if(netif_is_default(netif)) // only set default netif
    {
    	ipaddr.addr = cfg->dns;
    	dns_setserver(0, &ipaddr);
    	ipaddr.addr = cfg->dns2;
    	dns_setserver(1, &ipaddr);
	}
    #ifdef AIRPLAY_ENABLE
    avahi_network_notify(0);
    #endif
	return 0;
	
}

//obsolete api, should use new api "libnet_set_ipaddress2"
int libnet_set_ipaddress(const char *sipaddr, const char *sgw, const char *snetmask)
{
    struct netif *netif = NULL;
	
	netif = netif_find("en0");
    return libnet_set_ipaddress2(netif, sipaddr, sgw, snetmask);
}

int libnet_set_ipaddress2(struct netif *netif, const char *sipaddr, const char *sgw, const char *snetmask)
{
	struct ip_addr ipaddr, gw, netmask;
    
	if (netif == NULL)
	{
		LIBNET_DEBUG("Can not find netif named <en>!\n");
		return -1;
	}

	//set the ip address	
	ipaddr.addr = inet_addr(sipaddr);
	netmask.addr = inet_addr(snetmask);
	gw.addr = inet_addr(sgw);
	netif_set_addr(netif, &ipaddr, &netmask, &gw);
	return 0;	
}

void dhcp_task(UINT32 para1, UINT32 param2)
{
	struct netif *netif = NULL;	
	int time_out = 3*60  * 1000; //default timeout time 1 minutes
	int cnt = 0; 
	int cb_code = NET_ERR_DHCP_FAILED;
	
	netif = (struct netif *)para1;
	if(netif == NULL)
	{
		return;
	}
	do
	{
		if ((!ip_addr_cmp(&netif->ip_addr, IP_ADDR_ANY))
			&& (!ip_addr_cmp(&netif->netmask, IP_ADDR_ANY))
			&& (!ip_addr_cmp(&netif->gw, IP_ADDR_ANY)))
		{
			cb_code = NET_ERR_DHCP_SUCCESS;
			LIBNET_DEBUG("DHCP get ip addresses successfully!\n");
            #ifdef AIRPLAY_ENABLE
            avahi_network_notify(0);
            #endif
			break;
		}
		else
		{
			osal_task_sleep(200);
			cnt += 200;
			if(cnt > time_out)
			{
				LIBNET_DEBUG("DHCP task dhcp_off!\n");
				break;
			}
		}
	}while (netif->flags&NETIF_FLAG_DHCP);

	LIBNET_DEBUG("dhcp_task exit!\n");
	if(netif->flags&NETIF_FLAG_DHCP)
	{
		libnet_cb(NET_MSG_DHCP, cb_code);
	}
}

//obsolete api, should use new api "libnet_dhcp_on2"
int libnet_dhcp_on(void)
{
    struct netif *netif = NULL;
	
	netif = netif_find("en0");
    return libnet_dhcp_on2(netif);
}

int libnet_dhcp_on2(struct netif *netif)
{
	LIBNET_DEBUG("enter function %s\n", __FUNCTION__);
	if (netif == NULL)
	{
		LIBNET_DEBUG("Can not find netif named <en>!\n");
		return -1;
	}
	//firstly set the IP address = IP_ADDR_ANY
	netif_set_addr(netif, IP_ADDR_ANY, IP_ADDR_ANY, IP_ADDR_ANY);
	//start the DHCP
#if DHCP_SUCCESS_CALLBACK
	if (ERR_OK != netifapi_dhcp_start_callback(netif, NULL))
#else
	if (ERR_OK != netifapi_dhcp_start(netif))
#endif
	{
		return -1;
	}
   
	//start the DHCP task
	OSAL_T_CTSK t_ctsk;
	OSAL_ID tsk_id;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.para1 = (UINT32)netif;
	t_ctsk.para2 = (UINT32)NULL;
	t_ctsk.name[0] = 'D';
	t_ctsk.name[1] = 'H';
	t_ctsk.name[2] = 'C';
	t_ctsk.task = dhcp_task;
	tsk_id = osal_task_create(&t_ctsk);
	LIBNET_DEBUG("tsk_id = %d\n", tsk_id);
	ASSERT(OSAL_INVALID_ID != tsk_id);
	return 0;
}


//obsolete api, should use new api "libnet_dhcp_off2"
int libnet_dhcp_off()
{
    struct netif *netif = NULL;
	
	netif = netif_find("en0");
	return libnet_dhcp_off2(netif);
}

int libnet_dhcp_off2(struct netif *netif)
{
	if (netif == NULL || netif->dhcp == NULL)
	{
		return -1;
	}

	if (ERR_OK != netifapi_dhcp_release(netif))
	{
		return -1;
	}
	netifapi_dhcp_stop(netif);
    netif_set_up(netif); /* Bring up the interface */
	return 0;
}


int libnet_dhcp_get_status()
{
    struct netif *netif = NULL;
	
	netif = netif_find("en0");
	return libnet_dhcp_get_status2(netif);
}

int libnet_dhcp_get_status2(struct netif *netif)
{
    if(netif)
	{
        return (netif->flags&NETIF_FLAG_DHCP);
	}
	else
	{
        return 0;
	}
}

void libnet_reg_ipdetect_callback(void (*dup_ip_callback)(void))
{
	struct netif *new_netif = NULL;

	new_netif = netif_find("en0");
	if (new_netif == NULL)
	{
		LIBNET_DEBUG("Can not find netif named <en>!\n");
		return;
	}
	
	new_netif->dup_ip_callback = dup_ip_callback;
}
void libnet_ipaddr_broadcast()
{
	struct netif *new_netif = NULL;

	new_netif = netif_find("en0");
	if (new_netif == NULL)
	{
		LIBNET_DEBUG("Can not find netif named <en>!\n");
		return;
	}
	netif_send_arp(new_netif);
}


#else //W5300
#include <errno.h>
int libnet_dhcp_on(void)
{
	return -ENOTSUP;
}


int libnet_dhcp_off()
{
	return -ENOTSUP;
}

static UINT32 __inet_aton(UINT8 *ipa)
{
	int i;
	UINT32 ipv = 0;
	for(i = 0; i < 4; i++){
		ipv = (ipv << 8) | ipa[i];
	}
	return ipv;
}

static void __inet_ntoa(UINT32 ipv, UINT8 *ipa)
{
	int i;
	for(i = 0; i < 4; i++)
		ipa[i] = (ipv >> (i * 8)) & 0xff;

}


int libnet_get_ipconfig(struct ip_local_cfg *cfg)
{
	UINT8 ip[5];
	w5300_ioctl(0, W5300_GET_IP, (void *)ip);
	cfg->ip_addr = __inet_aton(ip);

	w5300_ioctl(0, W5300_GET_GW, (void *)ip);
	cfg->gateway = __inet_aton(ip);

	w5300_ioctl(0, W5300_GET_NM, (void *)ip);
	cfg->subnet_mask = __inet_aton(ip);

	cfg->dns = 0;
	cfg->dns2 = 0;

	return 0;
}

int libnet_set_ipconfig(const struct ip_local_cfg *cfg)
{
	UINT8 ip[5];

	__inet_ntoa(cfg->ip_addr, ip);
	w5300_ioctl(0, W5300_SET_IP, (void *)ip);

	__inet_ntoa(cfg->gateway, ip);
	w5300_ioctl(0, W5300_SET_GW, (void *)ip);

	__inet_ntoa(cfg->subnet_mask, ip);
	w5300_ioctl(0, W5300_SET_NM, (void *)ip);


}



#endif
#endif


