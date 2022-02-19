#include <sys_config.h>

#ifdef NETWORK_SUPPORT
#include "api/libtcpip/lwip/opt.h"
#include "api/libtcpip/lwip/mem.h"
#include "api/libtcpip/lwip/raw.h"
#include "api/libtcpip/lwip/icmp.h"
#include "api/libtcpip/lwip/netif.h"
#include "api/libtcpip/lwip/sys.h"
#include "api/libtcpip/lwip/inet_chksum.h"
#include "api/libtcpip/lwip/sockets.h"
#include "api/libtcpip/lwip/inet.h"
#include "api/libtcpip/lwip/netdb.h"
#include "api/libtcpip/lwip/ip.h"
#include "api/libnet/libnet.h"

extern void network_callback(UINT32 event_type, UINT32 param);
extern UINT32 ping_test_flag;
char ping_str[8][60] = {""};
int ping_count_write,ping_count_read = 0;

static char *_inet_ntoa(UINT32 s_addr)
{
    static char str[16];
    char inv[3];
    char *rp;
    UINT8 *ap;
    UINT8 rem;
    UINT8 n;
    UINT8 i;
    rp = str;
    ap = (UINT8 *)&s_addr;

    for (n = 0; n < 4; n++)
    {
        i = 0;

        do
        {
            rem = *ap % (UINT8)10;
            *ap /= (UINT8)10;
            inv[i++] = '0' + rem;
        }
        while (*ap);

        while (i--)
        {
            *rp++ = inv[i];
        }

        *rp++ = '.';
        ap++;
    }

    *--rp = 0;
    return str;
}

static int _recvfrom(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr,
                     socklen_t *addrlen, unsigned int timeout_ms)
{
    int             ret = 0;
    struct timeval  to;
    fd_set          fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);
    to.tv_sec = timeout_ms / 1000;
    to.tv_usec = (timeout_ms % 1000) * 1000;
    ret = lwip_select(sockfd + 1, &fdset, NULL, NULL, timeout_ms == 0 ? NULL : &to);

    if (ret > 0)
    {
        if (FD_ISSET(sockfd, &fdset))
        {
            ret = lwip_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

            if (ret <= 0)
            {
                ret = -1;
            }
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
}

void lwip_ping(char *ip_domain, int pktsize, unsigned int send_times)
{
    int isdomain = 0;
    int seqno = 0;
    int sock = -1;

    UINT32 send_cnt = 0;
    UINT32 recv_cnt = 0;
    UINT32 timeout_ms = 3000;
    __MAYBE_UNUSED__ UINT32 ping_tick = 0;
    __MAYBE_UNUSED__ UINT32 recv_tick = 0;
    struct sockaddr_in from;
    struct sockaddr_in to;
    struct icmp_echo_hdr *echo;
    int ipaddr = inet_addr(ip_domain);
    socklen_t addr_len  = sizeof(struct sockaddr_in);
    int buff_len  = sizeof(struct icmp_echo_hdr) + pktsize;
    char *recvbuf = NULL;
	libnet_callback notify = (libnet_callback)network_callback;
	
    if (ipaddr == (int)INADDR_NONE)
    {
        struct hostent *host = lwip_gethostbyname(ip_domain);

        if (host == NULL)
        {
            libc_printf("can not resolve domain name:%s\n", ip_domain);
			if(ping_count_write == 7)
			{
				ping_count_write = 0;
			}
			if(1 == ping_test_flag)
			{
				sprintf(ping_str[ping_count_write++],"can not resolve domain name:%s\n", ip_domain);
				notify(CTRL_MSG_PING_PROGRESS, 0);
			}
			osal_task_sleep(10);
			ping_test_flag = 0;
            return;
        }

        ipaddr = *(int *)host->h_addr;
        isdomain = 1;
    }

    if ((sock = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
    {
        libc_printf("create socket failed!\n");
		if(ping_count_write == 7)
		{
			ping_count_write = 0;
		}
		if(1 == ping_test_flag)
		{
			sprintf(ping_str[ping_count_write++],"create socket failed!\n");
			notify(CTRL_MSG_PING_PROGRESS, 0);
		}
		osal_task_sleep(10);
		//lwip_close(sock);
		ping_test_flag = 0;
        return;
    }

    echo = MALLOC(buff_len);

    if (echo == NULL)
    {
        libc_printf("malloc failed, request to malloc size:0x%x\n", buff_len);
		if(ping_count_write == 7)
		{
			ping_count_write = 0;
		}
		if(1 == ping_test_flag)
		{
			sprintf(ping_str[ping_count_write++],"malloc failed, request to malloc size:0x%x\n", buff_len);
			notify(CTRL_MSG_PING_PROGRESS, 0);
		}
		osal_task_sleep(10);
		ping_test_flag = 0;
        lwip_close(sock);
        return ;
    }

    recvbuf = MALLOC(buff_len + sizeof(struct ip_hdr));

    if (recvbuf == NULL)
    {
        libc_printf("malloc failed, request to malloc size:0x%x\n", buff_len);
		if(ping_count_write == 7)
		{
			ping_count_write = 0;
		}
		if(1 == ping_test_flag)
		{
			sprintf(ping_str[ping_count_write++],"malloc failed, request to malloc size:0x%x\n", buff_len);
			notify(CTRL_MSG_PING_PROGRESS, 0);
		}
    	FREE(echo);
        lwip_close(sock);
		osal_task_sleep(10);
		ping_test_flag = 0;
        return ;
    }

    to.sin_addr.s_addr = ipaddr;
    to.sin_family = AF_INET;
    to.sin_len = sizeof(to);

    if (isdomain)
    {
        libc_printf("\n\nPinging %s[%s] with %d bytes of data:\n", ip_domain, _inet_ntoa(ipaddr), pktsize);
		if(ping_count_write == 7)
		{
			ping_count_write = 0;
		}
		if(1 == ping_test_flag)
		{
		sprintf(ping_str[ping_count_write++],"Pinging %s[%s] with %d bytes of data:", ip_domain, _inet_ntoa(ipaddr), pktsize);
		notify(CTRL_MSG_PING_PROGRESS, 0);
		}

    }
    else
    {
        libc_printf("\n\nPinging %s with %d bytes of data:\n", ip_domain, pktsize);
		if(ping_count_write == 7)
		{
			ping_count_write = 0;
		}
		if(1 == ping_test_flag)
		{
        	sprintf(ping_str[ping_count_write++],"Pinging %s with %d bytes of data:", ip_domain, pktsize);
			//libc_printf("\n %s\n",ping_str[ping_count_write-1]);
			notify(CTRL_MSG_PING_PROGRESS, 0);
		}
    }

    while (send_cnt++ < send_times || send_times == 0)
    {
        ICMPH_TYPE_SET(echo, ICMP_ECHO);
        ICMPH_CODE_SET(echo, 0);
        echo->chksum = 0;
        echo->id     = 0xAFAF;
        echo->seqno  = htons(++seqno);
        echo->chksum = inet_chksum(echo, buff_len);
        ping_tick = osal_get_tick();
		if(ping_test_flag == 2)
		{
			break;
		}
			
        if (lwip_sendto(sock, (char *)echo, buff_len, 0, (struct sockaddr *)&to, addr_len) > 0)
        {
            if (_recvfrom(sock, recvbuf, buff_len + sizeof(struct ip_hdr), 0, (struct sockaddr *)&from, &addr_len, timeout_ms) >
                    (int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
            {
                recv_tick = osal_get_tick();
                struct ip_hdr *iphdr = (struct ip_hdr *)recvbuf;
                struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)(recvbuf + (IPH_HL(iphdr) * 4));

                if ((iecho->id == 0xAFAF) && (iecho->seqno == echo->seqno))
                {
                    recv_cnt++;
                    libc_printf("Reply from %s: bytes=%d time:%ldms TTL=255\n",
                                _inet_ntoa(from.sin_addr.s_addr), pktsize, recv_tick - ping_tick);
					if(ping_count_write == 7)
					{	
						ping_count_write = 0;
					}
					if(1 == ping_test_flag)
					{
						sprintf(ping_str[ping_count_write++],"Reply from %s: bytes=%d time:%ldms TTL=255",
                                _inet_ntoa(from.sin_addr.s_addr), pktsize, recv_tick - ping_tick);
						notify(CTRL_MSG_PING_PROGRESS, 0);
					}
                }
            }
            else
            {
                libc_printf("Request timed out.\n");
				if(ping_count_write == 7)
				{
					ping_count_write = 0;
				}
				if(1 == ping_test_flag)
				{
					sprintf(ping_str[ping_count_write++],"Request timed out.");
					notify(CTRL_MSG_PING_PROGRESS, 0);
				}
            }
        }
        else
        {
            libc_printf("Ping %s error!!\n", _inet_ntoa(ipaddr));
			if(ping_count_write == 7)
			{
				ping_count_write = 0;
			}
			if(1 == ping_test_flag)
			{
				sprintf(ping_str[ping_count_write++],"Ping %s error!!", _inet_ntoa(ipaddr));
				notify(CTRL_MSG_PING_PROGRESS, 0);
			}
        }

        osal_task_sleep(1000);
    }

    libc_printf("----------------------------------------------------------\n"\
                "Ping statistics for %s:\n"\
                "Packets: Sent = %d, Received = %d, Lost = %d (%d%% loss)\n",
                _inet_ntoa(ipaddr), send_times, recv_cnt, (send_times - recv_cnt),
                (send_times - recv_cnt) * 100 / send_times);
	if(ping_count_write == 7)
	{
		ping_count_write = 0;
	}
	if(1 == ping_test_flag)
	{
		sprintf(ping_str[ping_count_write++],"--------------------------------------------------------------------------------------------");
		notify(CTRL_MSG_PING_PROGRESS, 0);	
	}
	osal_task_sleep(10);
	if(ping_count_write == 7)
	{
		ping_count_write = 0;
	}
	if(1 == ping_test_flag)
	{
		sprintf(ping_str[ping_count_write++],"Ping statistics for %s:",_inet_ntoa(ipaddr));
		notify(CTRL_MSG_PING_PROGRESS, 0);
	}
	osal_task_sleep(100);
	if(ping_count_write == 7)
	{
		ping_count_write = 0;
	}
	if(1 == ping_test_flag)
	{
    	sprintf(ping_str[ping_count_write++],"Packets: Sent = %d, Received = %ld, Lost = %ld (%ld%% loss)",send_times, recv_cnt, (send_times - recv_cnt),
                (send_times - recv_cnt) * 100 / send_times);             
		notify(CTRL_MSG_PING_PROGRESS, 0);
	}
	osal_task_sleep(10);
    lwip_close(sock);
    FREE(echo);
    FREE(recvbuf);
	ping_test_flag = 0;
	//libc_printf("ping_test_flag:%d %s\n", ping_test_flag,__FUNCTION__);
}

#endif

