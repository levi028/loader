#ifndef LIBNET_H
#define LIBNET_H
#include <sys_config.h>
#include <api/libtcpip/arch/sys_arch.h>
#include <hld/net/net.h>

#ifdef __cplusplus
extern "C"
{
#endif

//callback message and message code
enum
{
    NET_MSG_DHCP = 1,
    NET_MSG_DOWNLOAD_START,
    NET_MSG_DOWNLOAD_PROGRESS,
    NET_MSG_DOWNLOAD_FINISH,
    NET_MSG_DOWNLOAD_RESUME,
    NET_MSG_DOWNLOAD_WAITDATA,
    NET_MSG_DOWNLOAD_NETERROR,
    NET_MSG_DOWNLOAD_SPEED,
    NET_MSG_DOWNLOAD_INFO,
	NET_MSG_DOWNLOAD_PLAYLIST,
	CTRL_MSG_PING_PROGRESS,
};

enum
{
    NET_DOWNLOAD_INFO_NOSEEK = 1,
    NET_DOWNLOAD_INFO_BYTESEEK,
};

enum
{
    NET_ERR_DHCP_SUCCESS = 1,
    NET_ERR_DHCP_FAILED,
    NET_ERR_CONNECT_FAILED,
    NET_ERR_FILE_NOT_FOUND,
    NET_ERR_LOGIN_FAILED,
    NET_ERR_HTTP_SERVER_ERROR,
    NET_ERR_USER_ABORT,
    NET_ERR_FTP_SERVER_ERROR,
    NET_ERR_OPERATION_NOT_PERMIT,
    NET_ERR_FILE_SAVE_ERROR,

};


typedef void (*libnet_callback)(UINT32 msg, UINT32 code);
typedef unsigned long NETFILE;



//========================================================================================
//  network config function
typedef struct ip_local_cfg
{
    UINT32 dhcp_on;                    // 1: DHCP on, 0: DHCP off
    UINT32 ip_addr;
    UINT32 subnet_mask;
    UINT32 gateway;
    UINT32 dns;
    UINT32 dns2;

}IP_LOC_CFG, *PIP_LOC_CFG;



struct libnet_config
{
    LWIP_CONFIG lwip_cfg;
    libnet_callback cb;
};

#ifdef NETWORK_SUPPORT
extern libnet_callback libnet_cb;
#endif

int libnet_init(struct libnet_config *pcfg);
void libnet_notify(UINT32 msg, UINT32 msgcode);

int network_init(struct libnet_config *pcfg);
int libnet_get_ipconfig(struct ip_local_cfg *cfg);
int libnet_set_ipconfig(const struct ip_local_cfg *cfg);
int libnet_dhcp_on(void);
int libnet_dhcp_off();
int libnet_set_ipaddress(const char *sipaddr, const char *sgw, const char *snetmask);

int libnet_dhcp_off2(struct netif *netif);
int libnet_dhcp_on2(struct netif *netif);
struct netif * sys_add_netif2(struct net_device *dev, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw);
int libnet_get_ipconfig2(struct netif *netif, struct ip_local_cfg *cfg);
int libnet_set_ipconfig2(struct netif *netif, const struct ip_local_cfg *cfg);
int libnet_set_ipaddress2(struct netif *netif, const char *sipaddr, const char *sgw, const char *snetmask);
int libnet_dhcp_get_status2(struct netif *netif);




//=========================================================================================
// http/ftp client

/* Ftp_URL is like "ftp://user:password@192.168.0.1/xxxx", user and password can be NULL*/
/* Http_URL is like "http://192.168.0.1/xxxx", outfile is like "/mnt/uda1/1.rar"(full path)*/
unsigned long libnet_download(char *full_url, char *outfile);
unsigned long libnet_download_to_mem(char *full_url, unsigned char* buffer, unsigned int buf_len,  unsigned int offset);
void libnet_download_abort(unsigned long handle);
void libnet_set_sock_to_url(int sock);
int  libnet_get_sock_to_url(void);
void libnet_set_wait_flag(BOOL set);
BOOL libnet_get_wait_flag(void);
BOOL get_net_dl_status(void);
void set_net_dl_status(BOOL status);
void libnet_set_url_timeout(UINT32 timout);//in ms
void libnet_set_stop_rtp(int value);
int libnet_get_stop_rtp(void);

void sys_del_netif(struct net_device *dev);

#ifdef __cplusplus
}
#endif

#endif


