
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libnet/libnet.h>
#include <api/libcurl/curl.h>
#include <api/libcurl/easy.h>
#include <api/libcurl/urlfile.h>
#include <api/libcurl/rbuffer.h>

//#include <api/libtcpip/lwip/sockets.h>
//#include <api/libfs2/stdio.h>
//#include <api/libfs2/fcntl.h>
//#include "rtp_pe_cache.h"


#define MAX_PKT_SIZE    1400

typedef struct RTP_HEADER 
{ 
	unsigned short	cc        : 4; 
	unsigned short	extension : 1; 
	unsigned short	padding   : 1; 
	unsigned short	version   : 2; 
	unsigned short	payload   : 7; 
	unsigned short	marker    : 1; 
	unsigned short	seqnum; 
	unsigned long	timestamp; 
	unsigned long	ssrc; 
} RTP_HEADER; 

#define SEQNUM(A) (((A & 0x00FF) << 8) | ((A & 0xFF00) >> 8))


struct uf_rtppriv
{
    int sock;
    int port;
    UINT16 seqnum;
};

static int uf_rtp_init(urlfile *file)
{
    struct uf_rtppriv *priv = NULL;
    struct sockaddr_in sa;
    char   *buffer = NULL;
    int     ret = 0;
    RTP_HEADER *rtp = NULL;
    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    static int lose_packet = 0; //flag = 0, size_request_tmp = 0, remind_size = 0;
    //int total_bytes_read = 0;
    //int size_request = 0;
    UINT32 ul = 1;
    struct timeval timeout;
    fd_set rfd;
    int timeout_cnt = 0;

    if (file->private == NULL)
    {
        priv = MALLOC(sizeof(struct uf_rtppriv));
        if (priv == NULL) { return -1; }
        file->private = priv;
    }

    priv->port = 42030;

    if ((priv->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        libc_printf("Create RTP by socket failed \n");
        return -1;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(priv->port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(priv->sock, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        libc_printf("Create RTP bind error \n");
        closesocket(priv->sock);
        return -1;
    }
    ioctlsocket(priv->sock, FIONBIO, &ul);

    buffer = MALLOC(MAX_PKT_SIZE);
    if (buffer == NULL)
    {
    	 FREE(buffer);
        return -1;
    }

    file->size=-1;//unknow filesize
    uf_state(file, UF_TRANS);
    while (file->state == UF_TRANS)
    {
        FD_ZERO(&rfd);
        timeout.tv_sec = 0;//RTP_TIMEOUT;
        timeout.tv_usec = 100*1000;
        //timeout.tv_sec = 20*1000;//RTP_TIMEOUT;
       // timeout.tv_usec =0;
        FD_SET(priv->sock, &rfd);//set onetime,to select one socket
        ret = select(priv->sock + 1, &rfd, 0, 0, &timeout); //RTP_TIMEOUT
        if (ret == 0)
        {
            timeout_cnt += 100;
            if (timeout_cnt == 10 * 1000)
            {
            	timeout_cnt = 0;
            }
        }
        else if (ret > 0)
        {
            timeout_cnt = 0;
            ret = recvfrom(priv->sock, (void *)buffer, MAX_PKT_SIZE, 0, (struct sockaddr *)&si_other, (u32_t *)&slen);
            if (ret <= 0)
            {
            	  FREE(buffer);
                return -1;
            }

            if (ret > MAX_PKT_SIZE)
            {
                libc_printf("PKT size  error,sixe=%d \n", ret);
                //return 0;
            }

            if (ret > 0)
            {
                rtp = (RTP_HEADER *)buffer;
                //  libc_printf("%d ", SEQNUM(rtp->seqnum));
                if (SEQNUM(rtp->seqnum) != (priv->seqnum + 1))
                {
                    lose_packet++;
                    libc_printf("L: %d\n", lose_packet); // we lost rtp package
                }
                priv->seqnum = SEQNUM(rtp->seqnum);
                //  libc_printf("%d ", seqnum);
                //libc_printf("Jack bytes_read = %d \n",bytes_read);
                //              osal_mutex_lock(pe_cache_mutex_id, OSAL_WAIT_FOREVER_TIME);
                ret = ret - 12;
                uf_store_data(file, buffer+12, ret);
            }
        }
    }

    closesocket(priv->sock);
    priv->sock = -1;
    FREE(buffer);
    return -1;
}

static void uf_rtp_release(urlfile *file)
{
    struct uf_rtppriv *priv = (struct uf_rtppriv *)file->private;
    if (priv)
    {
        if (priv->sock >= 0)
        {
            closesocket(priv->sock);
        }
        file->private = NULL;
        FREE(priv);
    }
}

ufprotocol uf_rtp =
{
    .next = NULL,
    .name = "rtp",
    .init = (void *)uf_rtp_init,
    .release = (void *)uf_rtp_release,
    .seek = NULL,
    .ioctl = NULL,
    .curl_retproc = NULL,
    .curl_progcb = NULL,
    .data_recved = NULL,
};

