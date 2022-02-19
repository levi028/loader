
#include <sys_config.h>

#ifdef VPN_ENABLE
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/list.h>
#include <api/libtcpip/lwip/sockets.h>
#include <api/libtcpip/lwip/inet.h>
#include <api/libvpn/vpn.h>
#include "ppp_fcs.h"
#include "pptp_msg.h"
#include "pptp_gre.h"

#define HDLC_FLAG         0x7E
#define HDLC_ESCAPE       0x7D
#define HDLC_TRANSPARENCY 0x20
#define IP_MTU 14
#define MISSING_WINDOW 300
#define WRAPPED( curseq, lastseq) ((((curseq) & 0xffffff00) == 0) && (((lastseq) & 0xffffff00 ) == 0xffffff00))
#define INVALID_CALLID 0xffff

typedef int (*callback_t)(vpn_gre *vpngre, int cl, void *pack, UINT32 len);

extern UINT32 vpn_ppp_open(char *usr, char *passwd);
extern void vpn_ppp_close(UINT32 fd);
extern int vpn_write_pppdata(int fd, char *buff, int len);
extern int vpn_read_pppdata(int fd, char *buff, int len);
extern int vpn_ppp_select(int fd);

static void pptp_gre_state(vpn_gre *gre, int state)
{
    if (gre)
    {
        osal_mutex_lock(gre->slock, OSAL_WAIT_FOREVER_TIME);
        switch (state)
        {
            case PPTP_GRE_STATE_NONE:
                if (gre->state == PPTP_GRE_STATE_STOP)
                {
                    gre->state = state;
                }
                break;
            case PPTP_GRE_STATE_RUNNING:
                if (gre->state == PPTP_GRE_STATE_NONE)
                {
                    gre->state = state;
                }
                break;
            case PPTP_GRE_STATE_STOP:
                if (gre->state == PPTP_GRE_STATE_RUNNING)
                {
                    gre->state = state;
                }
                break;
        }
        osal_mutex_unlock(gre->slock);
    }
}

void print_packet(int fd, void *pack, UINT32 len)
{
    UINT8 *b = (UINT8 *)pack;
    UINT32 i, j;

    libc_printf("-- begin packet (%u) --\n", len);
    for (i = 0; i < len; i += 16)
    {
        for (j = 0; j < 8; j++)
            if (i + 2 * j + 1 < len)
            {
                libc_printf("%02x%02x ", (UINT32) b[i + 2 * j], (UINT32) b[i + 2 * j + 1]);
            }
            else if (i + 2 * j < len)
            {
                libc_printf("%02x ", (UINT32) b[i + 2 * j]);
            }
        libc_printf("\n");
    }
    libc_printf("-- end packet --\n");
}

//static int gettimeofday(struct timeval *tv)
//{
//    tv->tv_sec = osal_get_time();
//    tv->tv_usec = osal_get_tick();
//    return(0);
//}

UINT64 time_usecs(void)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
}

static int vpn_gre_select(vpn_gre *vpngre, int maxfdp1, fd_set *readset,
                          struct timeval *timeout, int pty_fd)
{
    int ret = 0;
    struct timeval tm;
    int tmo = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

    if (vpngre->callid == INVALID_CALLID || vpngre->peer_callid == INVALID_CALLID)
    {
        osal_task_sleep(100);
        return 0;
    }
    do
    {
        if (vpn_ppp_select(pty_fd) > 0)
        {
            ret = 1 << 8;
        }
        tm.tv_sec = 0;
        tm.tv_usec = ret > 0 ? 0 : 10 * 1000;
        ret |= lwip_select(maxfdp1 + 1, readset, NULL, NULL, &tm);
        if (ret > 0)
        {
            return ret;
        }
        tmo -= 10;
    }
    while (tmo > 0);
    return 0;
}

int pptp_gre_open(struct in_addr inetaddr)
{
    struct sockaddr_in src_addr;
    int s = lwip_socket(AF_INET, SOCK_RAW, PPTP_PROTO);
    if (s < 0)
    {
        libc_printf("socket create failed: %d\n", lwip_socket_errno);
        return -1;
    }
    src_addr.sin_family = AF_INET;
    src_addr.sin_addr   = inetaddr;
    src_addr.sin_port   = 0;
    if (lwip_connect(s, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0)
    {
        libc_printf("connect failed: %d\n", lwip_socket_errno);
        lwip_close(s);
        return -1;
    }
    return s;
}


static int decaps_ppp(vpn_gre *vpngre, int fd, callback_t cb, int cl)
{
    UINT32 start = 0, end;
    int status;

    if ((end = vpn_read_pppdata(fd, (char*)vpngre->decaps_hdlc_buffer,
                                sizeof(vpngre->decaps_hdlc_buffer))) <= 0)
    {
        int saved_errno = lwip_socket_errno;
        libc_printf("lwip_read :%d\n", lwip_socket_errno);
        switch (saved_errno)
        {
            case EMSGSIZE:
            {
                socklen_t optval, optlen = sizeof(optval);
                if (getsockopt(fd, IPPROTO_IP, IP_MTU, &optval, &optlen) < 0)
                {
                    libc_printf("getsockopt: %d\n", lwip_socket_errno);
                }
                libc_printf("getsockopt: IP_MTU: %d\n", optval);
                return 0;
            }
            case EIO:
                libc_printf("ppp read err:%d\n", lwip_socket_errno);
                break;
        }
        return 0;
    }

    if (!vpngre->decaps_hdlc_checkedsync)
    {
        vpngre->decaps_hdlc_checkedsync = 1;
    }

    if (vpngre->syncppp)
    {
        while (start + 8 < end)
        {
            vpngre->decaps_hdlc_len = ntoh16(*(short int *)(vpngre->decaps_hdlc_buffer + start + 6)) + 4;

            if (start + vpngre->decaps_hdlc_len <= end)
            {
                if ((status = cb(vpngre, cl, vpngre->decaps_hdlc_buffer + start, vpngre->decaps_hdlc_len)) < 0)
                {
                    return status;
                }
            }
            start += vpngre->decaps_hdlc_len;
        }
        return 0;
    }

    while (start < end)
    {
        while (vpngre->decaps_hdlc_buffer[start] != HDLC_FLAG)
        {
            if ((vpngre->decaps_hdlc_escape == 0) && vpngre->decaps_hdlc_buffer[start] == HDLC_ESCAPE)
            {
                vpngre->decaps_hdlc_escape = HDLC_TRANSPARENCY;
            }
            else
            {
                if (vpngre->decaps_hdlc_len < PACKET_MAX)
                {
                    vpngre->decaps_hdlc_copy [vpngre->decaps_hdlc_len++] = vpngre->decaps_hdlc_buffer[start] ^ vpngre->decaps_hdlc_escape;
                }
                vpngre->decaps_hdlc_escape = 0;
            }
            start++;
            if (start >= end)
            {
                return 0;
            }
        }

        start++;

        if ((vpngre->decaps_hdlc_len < 4) || (vpngre->decaps_hdlc_escape != 0))
        {
            vpngre->decaps_hdlc_len = 0;
            vpngre->decaps_hdlc_escape = 0;
            continue;
        }

        if (pppfcs16(PPPINITFCS16, vpngre->decaps_hdlc_copy, vpngre->decaps_hdlc_len) != PPPGOODFCS16)
        {
            libc_printf("Bad Frame Check Sequence\n");
        }
        vpngre->decaps_hdlc_len -= sizeof(UINT16);

        if ((status = cb(vpngre, cl, vpngre->decaps_hdlc_copy, vpngre->decaps_hdlc_len)) < 0)
        {
            return status;
        }

        vpngre->decaps_hdlc_len = 0;
        vpngre->decaps_hdlc_escape = 0;
    }
    return 0;
}

static int encaps_ppp(vpn_gre *vpngre, int fd, void *pack, UINT32 len)
{
    UINT8 *source = (UINT8 *)pack;
    UINT32 pos = 0, i;
    UINT16 fcs;

    if (vpngre->syncppp)
    {
        return vpn_write_pppdata(fd, (char*)source, len);
    }

    fcs = pppfcs16(PPPINITFCS16, source, len) ^ 0xFFFF;
    vpngre->encaps_hdlc_buffer[pos++] = HDLC_FLAG;

    for (i = 0; i < len + 2; i++)
    {

        UINT8 c =
            (i < len) ? source[i] : (i == len) ? (fcs & 0xFF) : ((fcs >> 8) & 0xFF);

        if (pos >= sizeof(vpngre->encaps_hdlc_buffer))
        {
            break;
        }
        if ((c < 0x20) || (c == HDLC_FLAG) || (c == HDLC_ESCAPE))
        {
            vpngre->encaps_hdlc_buffer[pos++] = HDLC_ESCAPE;
            if (pos < sizeof(vpngre->encaps_hdlc_buffer))
            {
                vpngre->encaps_hdlc_buffer[pos++] = c ^ 0x20;
            }
        }
        else
        {
            vpngre->encaps_hdlc_buffer[pos++] = c;
        }
    }

    if (pos < sizeof(vpngre->encaps_hdlc_buffer))
    {
        vpngre->encaps_hdlc_buffer[pos++] = HDLC_FLAG;
    }

    return vpn_write_pppdata(fd, (char*)vpngre->encaps_hdlc_buffer, pos);
}


static int decaps_gre(vpn_gre *vpngre, int fd, callback_t callback, int cl)
{
    struct pptp_gre_header *header;
    int status, ip_len = 0;
    UINT32 headersize;
    UINT32 payload_len;
    UINT32 seq;

    if ((status = lwip_read(fd, vpngre->decaps_gre_buffer, sizeof(vpngre->decaps_gre_buffer))) <= 0)
    {
        libc_printf("lwip_read err: %d\n", lwip_socket_errno);
        vpngre->stats.rx_errors++;
        return 0;
    }

    if ((vpngre->decaps_gre_buffer[0] & 0xF0) == 0x40)
    {
        ip_len = (vpngre->decaps_gre_buffer[0] & 0xF) * 4;
    }
    header = (struct pptp_gre_header *)(vpngre->decaps_gre_buffer + ip_len);

    if (((ntoh8(header->ver) & 0x7F) != PPTP_GRE_VER) ||
            (ntoh16(header->protocol) != PPTP_GRE_PROTO) ||
            PPTP_GRE_IS_C(ntoh8(header->flags)) ||
            PPTP_GRE_IS_R(ntoh8(header->flags)) ||
            (!PPTP_GRE_IS_K(ntoh8(header->flags))) ||
            ((ntoh8(header->flags) & 0xF) != 0))
    {
        libc_printf("Discarding GRE: %X %X %X %X %X %X\n",
                    ntoh8(header->ver) & 0x7F, ntoh16(header->protocol),
                    PPTP_GRE_IS_C(ntoh8(header->flags)),
                    PPTP_GRE_IS_R(ntoh8(header->flags)),
                    PPTP_GRE_IS_K(ntoh8(header->flags)),
                    ntoh8(header->flags) & 0xF);
        vpngre->stats.rx_invalid++;
        return 0;
    }
    if (ntoh16(header->call_id) != vpngre->callid)
    {
        return 0;
    }
    if (PPTP_GRE_IS_A(ntoh8(header->ver)))
    {
        UINT32 ack = (PPTP_GRE_IS_S(ntoh8(header->flags))) ? header->ack : header->seq;
        ack = ntoh32(ack);
        if (ack > vpngre->ack_recv)
        {
            vpngre->ack_recv = ack;
        }
        if (WRAPPED(ack, vpngre->ack_recv))
        {
            vpngre->ack_recv = ack;
        }
        if (vpngre->ack_recv == vpngre->stats.pt.seq)
        {
            int rtt = time_usecs() - vpngre->stats.pt.time;
            vpngre->stats.rtt = (vpngre->stats.rtt + rtt) / 2;
        }
    }
    if (!PPTP_GRE_IS_S(ntoh8(header->flags)))
    {
        return 0;
    }
    headersize  = sizeof(struct pptp_gre_header);
    payload_len = ntoh16(header->payload_len);
    seq         = ntoh32(header->seq);
    if (!PPTP_GRE_IS_A(ntoh8(header->ver)))
    {
        headersize -= sizeof(header->ack);
    }
    if (status - headersize < payload_len)
    {
        vpngre->stats.rx_truncated++;
        return 0;
    }
    if (vpngre->decaps_gre_first || (seq == vpngre->seq_recv + 1))
    {
        vpngre->stats.rx_accepted++;
        vpngre->decaps_gre_first = 0;
        vpngre->seq_recv = seq;
        return callback(vpngre, cl, vpngre->decaps_gre_buffer + ip_len + headersize, payload_len);
    }
    else if (seq < vpngre->seq_recv + 1 || WRAPPED(vpngre->seq_recv, seq))
    {
        vpngre->stats.rx_underwin++;
    }
    else if (seq < vpngre->seq_recv + MISSING_WINDOW || WRAPPED(seq, vpngre->seq_recv + MISSING_WINDOW))
    {
        vpngre->stats.rx_buffered++;
        if (vpngre->disable_buffer)
        {
            vpngre->seq_recv = seq;
            vpngre->stats.rx_lost += seq - vpngre->seq_recv - 1;
            return callback(vpngre, cl, vpngre->decaps_gre_buffer + ip_len + headersize, payload_len);
        }
    }
    else
    {
        vpngre->stats.rx_overwin++;
    }
    return 0;
}

static int encaps_gre(vpn_gre *vpngre, int fd, void *pack, UINT32 len)
{
    UINT32 header_len;
    int rc;
    struct pptp_gre_header *header = (struct pptp_gre_header *)vpngre->encaps_gre_buffer;

    header->flags       = hton8(PPTP_GRE_FLAG_K);
    header->ver         = hton8(PPTP_GRE_VER);
    header->protocol    = hton16(PPTP_GRE_PROTO);
    header->payload_len = hton16(len);
    header->call_id     = hton16(vpngre->peer_callid);

    if (pack == NULL)
    {
        if (vpngre->ack_sent != vpngre->seq_recv)
        {
            header->ver |= hton8(PPTP_GRE_FLAG_A);
            header->payload_len = hton16(0);

            header->seq = hton32(vpngre->seq_recv);
            vpngre->ack_sent = vpngre->seq_recv;
            rc = lwip_write(fd, header, sizeof(struct pptp_gre_header) - sizeof(header->seq));
            if (rc < 0)
            {
                rc = 0;
                vpngre->stats.tx_failed++;
            }
            else if ((size_t)rc < sizeof(struct pptp_gre_header) - sizeof(header->seq))
            {
                vpngre->stats.tx_short++;
            }
            else
            {
                vpngre->stats.tx_acks++;
            }
            return rc;
        }
        else
        {
            return 0;
        }

    }

    header->flags |= hton8(PPTP_GRE_FLAG_S);
    header->seq    = hton32(vpngre->encaps_gre_seq);
    if (vpngre->ack_sent != vpngre->seq_recv)
    {
        header->ver |= hton8(PPTP_GRE_FLAG_A);
        header->ack  = hton32(vpngre->seq_recv);
        vpngre->ack_sent = vpngre->seq_recv;
        header_len = sizeof(struct pptp_gre_header);
    }
    else
    {
        header_len = sizeof(struct pptp_gre_header) - sizeof(header->ack);
    }
    if (header_len + len >= sizeof(vpngre->encaps_gre_buffer))
    {
        vpngre->stats.tx_oversize++;
        return 0;
    }

    memcpy(vpngre->encaps_gre_buffer + header_len, pack, len);

    vpngre->seq_sent = vpngre->encaps_gre_seq;
    vpngre->encaps_gre_seq++;

    rc = lwip_write(fd, vpngre->encaps_gre_buffer, header_len + len);
    if (rc < 0)
    {
        rc = 0;
        vpngre->stats.tx_failed++;
    }
    else if ((size_t)rc < header_len + len)
    {
        vpngre->stats.tx_short++;
    }
    else
    {
        vpngre->stats.tx_sent++;
        vpngre->stats.pt.seq  = vpngre->seq_sent;
        vpngre->stats.pt.time = time_usecs();
    }
    return rc;
}

static int vpn_gre_callid(vpn_gre *gre)
{
    struct sockaddr_in loc_addr;
    memset(&loc_addr, 0, sizeof(loc_addr));
    loc_addr.sin_family = AF_INET;
    loc_addr.sin_addr.s_addr = gre->callid;
    libc_printf("GRE(%d) Bind CallID:%d\n", gre->gre_fd, gre->callid);
    return bind(gre->gre_fd, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
}

static void vpn_gre_proc(vpn_gre *vpngre)
{
    int no_ack = 2 * 1000;
    int max_fd;

    vpngre->ack_sent = 0;
    vpngre->ack_recv = 0;
    vpngre->seq_sent = 0;
    vpngre->seq_recv = 0;
    max_fd = vpngre->gre_fd;

    while (vpngre->state == PPTP_GRE_STATE_RUNNING)
    {
        if (vpngre->callid == INVALID_CALLID) // can not access one socket in two task, because of semaphore can not free tiwce
        {
            osal_task_sleep(5);
        }
        else
        {
            vpn_gre_callid(vpngre);
            break;
        }
    }

    while (vpngre->state == PPTP_GRE_STATE_RUNNING)
    {
        struct timeval tv = {0, 0};
        struct timeval *tvp;
        fd_set rfds;
        int retval;

        int block_usecs = -1;

        FD_ZERO(&rfds);
        FD_SET(vpngre->gre_fd, &rfds);

        if (vpngre->ack_sent != vpngre->seq_recv)
        {
            if (vpngre->ack_sent + 1 == vpngre->seq_recv)
            {
                block_usecs = 500000;
            }
            else
            {
                block_usecs = 1;
            }
        }

        if (block_usecs < 10 * 1000)
        {
            block_usecs = 10 * 1000;
        }
        if (block_usecs == -1)
        {
            tvp = NULL;
        }
        else
        {
            tvp = &tv;
            tv.tv_usec = block_usecs;
            tv.tv_sec  = tv.tv_usec / 1000000;
            tv.tv_usec %= 1000000;
        }
        retval = vpn_gre_select(vpngre, max_fd + 1, &rfds, tvp, vpngre->pty_fd);
        if (retval >> 8 == 1)
        {
            decaps_ppp(vpngre, vpngre->pty_fd, encaps_gre, vpngre->gre_fd);
        }
        else if (retval == 0 && vpngre->ack_sent != vpngre->seq_recv)
        {
            if (no_ack <= 0)
            {
                encaps_gre(vpngre, vpngre->gre_fd, NULL, 0);
                no_ack = 2 * 1000;
            }
            else
            {
                no_ack -= tvp->tv_sec * 1000 + tvp->tv_usec / 1000;
            }
        }
        if (retval > 0 && FD_ISSET(vpngre->gre_fd, &rfds))
        {
            no_ack = 2 * 1000;
            decaps_gre(vpngre, vpngre->gre_fd, encaps_ppp, vpngre->pty_fd);
        }
    }
}

static void pptp_gre_reset(vpn_gre *vpngre)
{
    vpngre->syncppp = 0;
    vpngre->log_level = 1;
    vpngre->disable_buffer = 1;
    vpngre->test_type = 0;
    vpngre->test_rate = 100;
    vpngre->rtmark = 0;
    vpngre->ack_sent = 0;
    vpngre->ack_recv = 0;
    vpngre->seq_sent = 0;
    vpngre->seq_recv = 0;

    vpngre->decaps_gre_first = 1;
    vpngre->encaps_gre_seq = 1;
    MEMSET(&vpngre->stats, 0, sizeof(gre_stats_t));
}

static void vpn_gre_task(UINT32 param1, UINT32 param2)
{
    vpn_gre *vpngre = (vpn_gre *)param1;

    pptp_gre_state(vpngre, PPTP_GRE_STATE_RUNNING);
    vpn_gre_proc(vpngre);
    pptp_gre_state(vpngre, PPTP_GRE_STATE_NONE);
    vpngre->taskid = INVALID_ID;
    libc_printf("GRE has been closed\n");
}

void vpn_gre_close(vpn_gre *vpngre)
{
    pptp_gre_state(vpngre, PPTP_GRE_STATE_STOP);
    while (vpngre->state == PPTP_GRE_STATE_STOP)
    {
        osal_task_sleep(100);
    }

    if (vpngre->pty_fd != -1)
    {
        vpn_ppp_close(vpngre->pty_fd);
        vpngre->pty_fd = -1;
    }
    if (vpngre->gre_fd != -1)
    {
        lwip_close(vpngre->gre_fd);
        vpngre->gre_fd = -1;
    }
}

int vpn_gre_open(vpn_cfg *vpncfg, vpn_gre *vpngre)
{
    OSAL_T_CTSK ptask;
    struct in_addr inetaddr;

    if (vpngre == NULL || vpncfg == NULL)
    {
        return 0;
    }

    if (vpngre->state == PPTP_GRE_STATE_STOP)
    {
        return 0;
    }
    if (vpngre->state == PPTP_GRE_STATE_RUNNING)
    {
        return 1;
    }

    pptp_gre_reset(vpngre);
    inetaddr.s_addr = vpncfg->saddr;
    vpngre->callid = INVALID_CALLID;
    vpngre->peer_callid = INVALID_CALLID;
    pptp_gre_state(vpngre, PPTP_GRE_STATE_NONE);
    vpngre->gre_fd = pptp_gre_open(inetaddr);
    vpngre->pty_fd = (int)vpn_ppp_open(vpncfg->usr, vpncfg->pwd);
    if (vpngre->gre_fd == -1 || vpngre->pty_fd == -1)
    {
        vpn_gre_close(vpngre);
        return 0;
    }

    ptask.task = (FP)vpn_gre_task;
    ptask.itskpri = 20;
    ptask.quantum = 10;
    ptask.stksz = 0x1000;
    ptask.para1 = (int)vpngre;
    ptask.para2 =  0;
    ptask.name[0] = 'g';
    ptask.name[1] = 'r';
    ptask.name[2] = 'e';

    vpngre->taskid = osal_task_create(&ptask);
    if (OSAL_INVALID_ID == vpngre->taskid)
    {
        vpn_gre_close(vpngre);
        return 0;
    }
    libc_printf("vpn gre start\n");
    return 1;
}

#endif

