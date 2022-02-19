#ifndef __ALI_NETWORK_H__
#define __ALI_NETWORK_H__
/**
 * This header file add some structure and type porting from linux
 */
#include <osal/osal.h>
#include <sys_config.h>
#include <api/libtcpip/lwip/sockets.h>
#include <api/libfs2/types.h>
#include <api/libtcpip/lwip/inet.h>

#ifdef __cplusplus
extern "C" {
#endif


#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#define MSG_DONTROUTE MSG_MORE
#define SOMAXCONN  128
#define IPPROTO_IPV6 41
#define IPV6_MULTICAST_IF 17
#define IPV6_MULTICAST_HOPS 18

//#define stat fs_stat

#define IN6_IS_ADDR_LINKLOCAL(a) \
    ((((__const uint32_t *) (a))[0] & htonl (0xffc00000))== htonl (0xfe800000))

typedef unsigned short in_port_t;
typedef unsigned short sa_family_t;
typedef unsigned int in_addr_t;
#define AI_PASSIVE      1
#define AI_CANONNAME    2
#define AI_NUMERICHOST  4
struct sockaddr_storage {
    unsigned char ss_len;
    unsigned char ss_family;
    //sa_family_t ss_family; // Don't use sa_family_t at TDS, change to unsigned char
    unsigned long int __ss_align;
    char __ss_padding[(128 - (2 * sizeof (unsigned long int)))];
};
struct in6_addr {
    UINT8 s6_addr[16];            /* IPv6 address */
};

struct sockaddr_in6 {
    sa_family_t sin6_family;         /* AF_INET6 */
    in_port_t sin6_port;               /* transport layer port # */
    UINT32 sin6_flowinfo;           /* IPv6 traffic class & flow info */
    struct in6_addr sin6_addr;    /* IPv6 address */
    UINT32 sin6_scope_id;        /* set of interfaces for a scope */
};
struct ifmap
{
    unsigned long int mem_start;
    unsigned long int mem_end;
    unsigned short int base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
    /* 3 bytes spare */
};

#define    __caddr_t void *

struct ifconf
{
    int ifc_len;                        /* Size of buffer.  */
    union
    {
        __caddr_t ifcu_buf;
        struct ifreq *ifcu_req;
    } ifc_ifcu;
};
#define IF_NAMESIZE 16

struct ifreq
{
#define IFHWADDRLEN    6
#ifndef IFNAMSIZ
#define IFNAMSIZ       IF_NAMESIZE
#endif
	union
	{
		char ifrn_name[IFNAMSIZ];       /* Interface name, e.g. "en0".  */
	} ifr_ifrn;

    union
    {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_dstaddr;
        struct sockaddr ifru_broadaddr;
        struct sockaddr ifru_netmask;
        struct sockaddr ifru_hwaddr;
        short int ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        struct ifmap ifru_map;
        char ifru_slave[IFNAMSIZ];      /* Just fits the size */
        char ifru_newname[IFNAMSIZ];
        __caddr_t ifru_data;
    } ifr_ifru;
};

#ifdef __cplusplus
}
#endif


#define ifr_name	ifr_ifrn.ifrn_name	/* interface name 	*/
#define ifr_addr ifr_ifru.ifru_addr /* address */
#define ifr_broadaddr ifr_ifru.ifru_broadaddr /* broadcast address */
#define ifr_netmask ifr_ifru.ifru_netmask /* interface net mask */
#define ifr_flags ifr_ifru.ifru_flags /* flags */
#define ifr_hwaddr ifr_ifru.ifru_hwaddr /* MAC address */
#define ifr_metric ifr_ifru.ifru_metric /* metric */
#define ifr_mtu ifr_ifru.ifru_mtu /* mtu */


#define SIOCGIFADDR _IOW('s', 102, struct ifreq) /* Get if addr */

#ifndef UINT16_MAX
#define UINT16_MAX 0xFFFF
#endif



#endif //__ALI_NETWORK_H__
