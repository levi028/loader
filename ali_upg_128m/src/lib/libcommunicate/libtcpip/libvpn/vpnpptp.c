
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
#include <api/libtcpip/lwip/inet.h>
#include <api/libtcpip/lwip/netif.h>
#include <api/libtcpip/lwip/sockets.h>

#include "vpnpptp.h"
#include "pptp_msg.h"

//#define PPTP_DEBUG(...)
#define PPTP_DEBUG libc_printf
#define PPTP_ERROR libc_printf

#define PPTP_GENERAL_ERROR_NONE                 0
#define PPTP_GENERAL_ERROR_NOT_CONNECTED        1
#define PPTP_GENERAL_ERROR_BAD_FORMAT           2
#define PPTP_GENERAL_ERROR_BAD_VALUE            3
#define PPTP_GENERAL_ERROR_NO_RESOURCE          4
#define PPTP_GENERAL_ERROR_BAD_CALLID           5
#define PPTP_GENERAL_ERROR_PAC_ERROR            6

typedef enum
{
    PPTP_CSTATE_IDLE,
    PPTP_CSTATE_WAIT_CTL_REPLY,
    PPTP_CSTATE_WAIT_CALL_REPLY,
    PPTP_CSTATE_ESTABLISHED,
    PPTP_CSTATE_ERROR
} PPTP_CSTATE;

typedef enum
{
    PPTP_STATE_NONE,
    PPTP_STATE_OPEN,
    PPTP_STATE_CLOSE,
} PPTP_STATE;

#define KEPP_LIVE_TMO (1000*20)
#define KEEP_LIVE_CNT  3
#define PPTP_READ_TMO  1000
#define EINPROGRESS 115 /* Operation now in progress */
#define INVALID_CALLID 0xffff

static inline UINT16 get_callid()
{
    UINT16 callid = 0x4000;
    extern struct netif *netif_default;
    struct netif *nf = netif_default;

    if (nf == NULL) { nf = netif_find("en"); }
    if (nf)
    {
        callid = ntohl(nf->ip_addr.addr) & 0xffff;
    }
    else
    {
        static UINT16 callids[] = {0x0, 0x4000, 0x8000, 0xc000};
        static int _callid_idx = -1;
        if (++_callid_idx >= 4) { _callid_idx = 0; }
        callid =  callids[_callid_idx];
    }
    return callid;
}

static int _pptp_close_socket(vpn_pptp *pptp)
{
    if (pptp->sock != -1)
    {
        closesocket(pptp->sock);
        pptp->sock = -1;
    }
    return 0;
}

static int _pptp_bind(vpn_pptp *pptp)
{
    static UINT16 vpnport = 1600;
    int ret = 0;
    struct sockaddr_in locl;

    vpnport++;
    if (vpnport > 1610) { vpnport = 1600; }

    MEMSET(&locl, 0, sizeof(locl));
    locl.sin_family = AF_INET;
    locl.sin_port = htons(vpnport);

    ret = bind(pptp->sock, (struct sockaddr *)&locl, sizeof(locl));
    if (ret < 0)
    {
        libc_printf("%d: bind port error:%d\n", __LINE__, lwip_socket_errno);
        return -1;
    }
    return 0;
}

static int _pptp_connect(vpn_pptp *pptp)
{
    int             ret = 0;
    struct timeval  to;
    struct sockaddr_in dest;
    int tmo = 20 * 1000;
    fd_set          fdset;

    PPTP_DEBUG("connect server, timeout:%ds\n", tmo / 1000);
    if (_pptp_bind(pptp) < 0)
    {
        PPTP_ERROR("%d: bind port error:%d\n", __LINE__, lwip_socket_errno);
        return -1;
    }
    MEMSET(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(pptp->sport);
    dest.sin_addr.s_addr = pptp->saddr;
    ret = connect(pptp->sock, (struct sockaddr *) &dest, sizeof(dest));
    if (ret < 0 && lwip_socket_errno != EINPROGRESS)
    {
        PPTP_ERROR("%d: connect error:%d\n", __LINE__, lwip_socket_errno);
        return -1;
    }
    if (ret == 0) // the connection is completed
    {
        return ret;
    }

    while (pptp->state == PPTP_STATE_OPEN && tmo > 0)
    {
        FD_ZERO(&fdset);
        FD_SET(pptp->sock, &fdset);
        to.tv_sec = 0;
        to.tv_usec = 100 * 1000;
        ret = lwip_select(pptp->sock + 1, NULL, &fdset, NULL, &to);
        if (ret > 0)
        {
            int error   = 0;
            int len     = sizeof(error);
            lwip_getsockopt(pptp->sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
            ret = error ? -1 : 0;
            PPTP_ERROR("connect ret:%d (err:%d, %d)\n", ret, lwip_socket_errno, error);
            break;
        }
        else if (ret == 0) //timeout
        {
            tmo -= 100;
        }
        else // error occured
        {
            ret = -1;
            PPTP_ERROR("%d: connect error:%d\n", __LINE__, lwip_socket_errno);
            break;
        }
    }
    if (tmo <= 0)
    {
        PPTP_ERROR("connect server timeout!!!\n");
    }
    return ret;
}
static int _pptp_open_socket(vpn_pptp *pptp)
{
    //struct sockaddr_in dest;

    if (pptp->sock == -1)
    {
        if (-1 == (pptp->sock = socket(AF_INET, SOCK_STREAM, 0)))
        {
            PPTP_ERROR("create socket failed*\n");
            return -1;
        }
        lwip_fcntl(pptp->sock, F_SETFL, O_NONBLOCK);
        if (_pptp_connect(pptp) < 0)
        {
            PPTP_ERROR("connect server failed+\n");
            _pptp_close_socket(pptp);
            return -1;
        }
        PPTP_DEBUG("connect server success\n");
        return 1;
    }
    return 1;
}

static int _pptp_read_packet(vpn_pptp  *pptp, char *buffer, int len, int tmo)
{
    int ret = 0;
    int loop = tmo / 10;

    if (loop == 0) { loop = 1; }
    while (loop-- > 0 && pptp->state == PPTP_STATE_OPEN)
    {
        ret = recv_tmo(pptp->sock, buffer , len, 0, 10);
        if (ret > 0 || ret < 0)
        {
            break;
        }
    }
    return ret;
}

static int _pptp_write_packet(vpn_pptp *pptp, char *buffer, int len)
{
    int ret = 0;
    int loop = 5000 / 100;
    int off = 0;

    if (pptp->sock == -1) { return -1; }
    if (loop == 0) { loop = 1; }
    do
    {
        ret = send_tmo(pptp->sock, buffer + off , len - off, 0, 100);
        if (ret > 0) //send success
        {
            off += ret;
        }
        else if (ret < 0) //error
        {
            break;
        }
        else //timeout
        {
            loop--;
        }
    }
    while (loop > 0 && pptp->state == PPTP_STATE_OPEN && off < len);
    return (ret > 0 ? ret : -1);
}

static int _pptp_send_stop_req(vpn_pptp *pptp)
{
    struct pptp_call_clear_rqst rqst1 =
    {
        PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_RQST), hton16(pptp->call_id), 0
    };

    struct pptp_stop_ctrl_conn rqst2 =
    {
        PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RQST),
        hton8(PPTP_STOP_LOCAL_SHUTDOWN), 0, 0
    };

    _pptp_write_packet(pptp, (char *)&rqst1, sizeof(rqst1));
    _pptp_write_packet(pptp, (char *)&rqst2, sizeof(rqst2));
    PPTP_DEBUG("PPTP send stop request\n");
    return 0;
}

static int _pptp_send_call_req(vpn_pptp  *pptp)
{
    static UINT16 call_serial_number = 10000;
    int ret = 0;
    /* Send off the call request */
    struct pptp_out_call_rqst packet =
    {
        PPTP_HEADER_CTRL(PPTP_OUT_CALL_RQST),
        0, 0, /*call_id, sernum */
        hton32(PPTP_BPS_MIN), hton32(PPTP_BPS_MAX),
        hton32(PPTP_BEARER_CAP), hton32(PPTP_FRAME_CAP),
        hton16(PPTP_WINDOW), 0, 0, 0, {0}, {0}
    };

    /* Initialize call structure */
    pptp->call_id   = get_callid();
    pptp->sernum    = call_serial_number++;
    packet.call_id = htons(pptp->call_id);
    packet.call_sernum = htons(pptp->sernum);

    /* fill in the phone number if it was specified */
    if (pptp->phonenr[0])
    {
        strncpy((char *)packet.phone_num, (char *)pptp->phonenr, sizeof(packet.phone_num));
        packet.phone_len = strlen((char *)pptp->phonenr);
        if (packet.phone_len > sizeof(packet.phone_num))
        {
            packet.phone_len = sizeof(packet.phone_num);
        }
        packet.phone_len = hton16(packet.phone_len);
    }
    ret = _pptp_write_packet(pptp, (char *)&packet, sizeof(packet));
    pptp->cstate = (ret >= 0) ? PPTP_CSTATE_WAIT_CALL_REPLY : PPTP_CSTATE_ERROR;
    PPTP_DEBUG("PPTP send out-going call request, ret:%d\n", ret);
    return 0;
}

static int _pptp_set_link_info(vpn_pptp  *pptp, UINT16 peer_call_id)
{
    /* if we need to send a set_link packet because of buggy
       hardware or pptp server, do it now */
    struct pptp_set_link_info packet =
    {
        PPTP_HEADER_CTRL(PPTP_SET_LINK_INFO),
        hton16(peer_call_id),
        0,
        0xffffffff,
        0xffffffff
    };
    _pptp_write_packet(pptp, (char *)&packet, sizeof(packet));
    return 0;
}

static int _pptp_send_conf_req(vpn_pptp  *pptp)
{
    int ret = 0;
    struct pptp_start_ctrl_conn packet =
    {
        PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
        hton16(PPTP_VERSION), 0, 0,
        hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
        hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION),
        PPTP_HOSTNAME, PPTP_VENDOR
    };

    pptp->call_serial_number = 0;
    ret = _pptp_write_packet(pptp, (char *)&packet, sizeof(packet));
    pptp->cstate = (ret >= 0) ? PPTP_CSTATE_WAIT_CTL_REPLY : PPTP_CSTATE_ERROR;
    PPTP_DEBUG("pptp send config request, ret:%d\n", ret);
    return ret;
}

static int _pptp_recv_conf_rep(vpn_pptp  *pptp, char *buffer, int len)
{
    struct pptp_start_ctrl_conn *packet = (struct pptp_start_ctrl_conn *) buffer;
    PPTP_DEBUG("Received Start Control Connection Reply\n");
    if (pptp->cstate == PPTP_CSTATE_WAIT_CTL_REPLY)
    {
        if (ntoh16(packet->version) != PPTP_VERSION)
        {
            return 0;
        }
        if (ntoh8(packet->result_code) != 1 && ntoh8(packet->result_code) != 0)
        {
            PPTP_DEBUG("Negative reply received to our Start Control Connection Request\n");
            return 0;
        }

        pptp->version      = ntoh16(packet->version);
        pptp->firmware_rev = ntoh16(packet->firmware_rev);
        memcpy(pptp->hostname, packet->hostname, sizeof(pptp->hostname));
        memcpy(pptp->vendor, packet->vendor, sizeof(pptp->vendor));

        return _pptp_send_call_req(pptp);
    }
    return 0;
}


static int _pptp_recv_call_rep(vpn_pptp  *pptp, char *buffer, int len)
{
    struct pptp_out_call_rply *packet = (struct pptp_out_call_rply *)buffer;

    if (pptp->cstate == PPTP_CSTATE_WAIT_CALL_REPLY)
    {
        PPTP_DEBUG("Received Outgoing Call Reply.\n");
        /* check for errors */
        if (packet->result_code != 1)
        {
            /* An error.  Log it verbosely. */
            PPTP_DEBUG("Our outgoing call request [callid %d] has not been accepted.\n", (int)  pptp->call_id);
        }
        else
        {
            /* connection established */
            pptp->peer_call_id = ntoh16(packet->call_id);
            pptp->speed        = ntoh32(packet->speed);
            pptp->cstate = PPTP_CSTATE_ESTABLISHED;

            /* call pptp_set_link. unless the user specified a quirk
               and this quirk has a set_link hook, this is a noop */
            _pptp_set_link_info(pptp, pptp->peer_call_id);

            PPTP_DEBUG("Outgoing call established (call ID %u, peer's call ID %u).\n",
                       pptp->call_id, pptp->peer_call_id);
        }
    }
    return 0;
}

static int _pptp_recv_echo_req(vpn_pptp  *pptp, char *buffer, int len)
{
    struct pptp_echo_rqst *packet = (struct pptp_echo_rqst *) buffer;
    struct pptp_echo_rply reply =
    {
        PPTP_HEADER_CTRL(PPTP_ECHO_RPLY),
        packet->identifier, /* skip hton32(ntoh32(id)) */
        hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0
    };
    _pptp_write_packet(pptp, (char *)&reply, sizeof(reply));
    return 0;
}

static int _pptp_recv_set_link(vpn_pptp  *pptp, char *buffer, int len)
{
    return 0;
}

static int _pptp_recv_echo_rep(vpn_pptp  *pptp, char *buffer, int len)
{
    return 0;
}

static int _pptp_send_echo_req(vpn_pptp  *pptp)
{
    int ret = 0;
    struct pptp_echo_rqst rqst =
    {
        PPTP_HEADER_CTRL(PPTP_ECHO_RQST), hton32(pptp->ka_id)
    };
    ret = _pptp_write_packet(pptp, (char *)&rqst, sizeof(rqst));
    if (ret < 0)
    {
        pptp->cstate = PPTP_CSTATE_ERROR;
    }
    return 0;
}

static int _pptp_keep_alive(vpn_pptp  *pptp)
{
    if (pptp->cstate == PPTP_CSTATE_ESTABLISHED)
    {
        if (pptp->kpcnt <= 0)
        {
            PPTP_DEBUG("PPTP Timeout!\n");
            pptp->cstate = PPTP_CSTATE_ERROR;
        }
        if (pptp->kptmo > 0)
        {
            pptp->kptmo -= PPTP_READ_TMO;
        }
        if (pptp->kptmo <= 0)
        {
            _pptp_send_echo_req(pptp);
            pptp->kpcnt--;
            pptp->kptmo = KEPP_LIVE_TMO;
            PPTP_DEBUG("pptp keep live(%d)...\n", pptp->kpcnt);
        }
    }
    return 0;
}

static void _pptp_task(UINT32 param1, UINT32 param2)
{
#define BUFF_LEN 1024
    char buffer[BUFF_LEN];
    int   len = 0;
    vpn_pptp  *pptp = (vpn_pptp *) param1;
    struct pptp_header *header = (struct pptp_header *)buffer;

    pptp->state = PPTP_STATE_OPEN;
    if (_pptp_open_socket(pptp) < 0)
    {
        pptp->cstate = PPTP_CSTATE_ERROR;
    }
    else
    {
        _pptp_send_conf_req(pptp);
    }

    while (pptp->state == PPTP_STATE_OPEN)
    {
        if (pptp->cstate == PPTP_CSTATE_ERROR)
        {
            osal_task_sleep(100); //error occured, wait close
            continue;
        }

        len = _pptp_read_packet(pptp, buffer, BUFF_LEN, PPTP_READ_TMO);
        if (len > 0)
        {
            pptp->kpcnt = KEEP_LIVE_CNT;
            pptp->kptmo = KEPP_LIVE_TMO;
            switch (ntoh16(header->ctrl_type))
            {
                case PPTP_START_CTRL_CONN_RPLY:
                    _pptp_recv_conf_rep(pptp, buffer, len);
                    break;
                case PPTP_OUT_CALL_RPLY:
                    _pptp_recv_call_rep(pptp, buffer, len);
                    break;
                case PPTP_CALL_CLEAR_NTFY:
                    pptp->cstate = PPTP_CSTATE_ERROR;
                    PPTP_DEBUG("vpn server notify: disconnect\n");
                    break;
                case PPTP_ECHO_RPLY:
                    _pptp_recv_echo_rep(pptp, buffer, len);
                    break;
                case PPTP_ECHO_RQST:
                    _pptp_recv_echo_req(pptp, buffer, len);
                    break;
                case PPTP_SET_LINK_INFO:
                    _pptp_recv_set_link(pptp, buffer, len);
                    break;
                default:
                    osal_task_sleep(10);
                    break;
            }
        }
        else if (len == 0)
        {
            _pptp_keep_alive(pptp);
        }
        else if (len < 0) //error
        {
            pptp->cstate = PPTP_CSTATE_ERROR;
            PPTP_ERROR("PPTP error occured: errno:%d\n", lwip_socket_errno);
        }
    }

    _pptp_send_stop_req(pptp);
    PPTP_DEBUG("PPTP has been closed\n");
    _pptp_close_socket(pptp);
    pptp->state = PPTP_STATE_NONE;
}

int vpn_pptp_getcallid(vpn_pptp *pptp, UINT16 *call_id, UINT16 *peer_call_id)
{
    if (pptp && pptp->cstate == PPTP_CSTATE_ESTABLISHED)
    {
        if (call_id)
        {
            *call_id = pptp->call_id;
        }
        if (peer_call_id)
        {
            *peer_call_id = pptp->peer_call_id;
        }
        return 1;
    }
    return 0;
}

int vpn_pptp_error(vpn_pptp *pptp)
{
    return (pptp == NULL || pptp->cstate == PPTP_CSTATE_ERROR);
}

int vpn_pptp_start(vpn_pptp *pptp)
{
    if (pptp && pptp->state == PPTP_STATE_NONE)
    {
        OSAL_T_CTSK ptask;
        ptask.task = (FP)_pptp_task;
        ptask.itskpri = 20;
        ptask.quantum = 10;
        ptask.stksz = 0x2000;
        ptask.para1 = (int)pptp;
        ptask.para2 =  0;
        ptask.name[0] = 'p';
        ptask.name[1] = 't';
        ptask.name[2] = 'p';

        pptp->sock = -1;
        pptp->peer_call_id = INVALID_CALLID;
        pptp->call_id = INVALID_CALLID;
        pptp->kpcnt = KEEP_LIVE_CNT;
        pptp->kptmo = KEPP_LIVE_TMO;

        if (OSAL_INVALID_ID == osal_task_create(&ptask))
        {
            return 0;
        }

        while (pptp->state == PPTP_STATE_NONE)
        {
            osal_task_sleep(100);
        }
    }
    return 1;
}

int vpn_pptp_close(vpn_pptp *pptp)
{
    if (pptp && pptp->state == PPTP_STATE_OPEN)
    {
        pptp->state = PPTP_STATE_CLOSE;

        while (pptp->state == PPTP_STATE_CLOSE)
        {
            osal_task_sleep(100);
        }
    }
    return 1;
}

#endif

