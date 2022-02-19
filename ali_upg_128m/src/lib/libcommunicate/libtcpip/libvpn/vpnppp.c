
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
#include <api/libtcpip/lwip/sio.h>
#include <api/libtcpip/lwip/ip_addr.h>

//#define GREPPP_DEBUG(...)
#define GREPPP_DEBUG libc_printf
#define GREPPP_ERROR libc_printf

#define MAX_PAKCET_CNT   100
#define VPN_PPP_TIMEOUT  (2*60*1000)

#define VPN_NAME_LEN   128
#define VPN_PASSWD_LEN 128

extern void ccp_mppe(int unit, int enable);
extern void ppp_set_netifname(int pd, char *name);
extern void pppInit(void);
extern void pppSetAuth(int type, const char *user, const char *passwd);
extern int pppOverSerialOpen(sio_fd_t fd, void (*linkStatusCB)(void *ctx, int errCode, void *arg), void *linkStatusCtx);
extern void ppp_set_netifname(int pd, char *name);
extern int pppClose(int pd);
extern int ppp_state(int pd);
//extern void pcap_stop(struct netif *netif);
//extern char *pcap_start(struct netif *netif, char *save_dir);
extern void dns_setserver(u8_t numdns, ip_addr_t *dnsserver);
extern void netif_auto_default();


#if 1 /* copy from lwip ppp.h*/
#define PPPERR_NONE      0 /* No error. */
#define PPPERR_PARAM    -1 /* Invalid parameter. */
#define PPPERR_OPEN     -2 /* Unable to open PPP session. */
#define PPPERR_DEVICE   -3 /* Invalid I/O device for PPP. */
#define PPPERR_ALLOC    -4 /* Unable to allocate resources. */
#define PPPERR_USER     -5 /* User interrupt. */
#define PPPERR_CONNECT  -6 /* Connection lost. */
#define PPPERR_AUTHFAIL -7 /* Failed authentication challenge. */
#define PPPERR_PROTOCOL -8 /* Failed to meet protocol. */

enum pppAuthType
{
    PPPAUTHTYPE_NONE,
    PPPAUTHTYPE_ANY,
    PPPAUTHTYPE_PAP,
    PPPAUTHTYPE_CHAP
};

struct ppp_addrs
{
    UINT32 our_ipaddr, his_ipaddr, netmask, dns1, dns2;
};
#endif

enum ppp_state
{
    PPP_STATE_NONE = 0,
    PPP_STATE_OPEN,
    PPP_STATE_CLOSE,
};
//ppp state
enum ppp_conn_state
{
    ppp_cstate_none = 0,
    ppp_cstate_connectting,
    ppp_cstate_connect_err,
    ppp_cstate_disconnectting,
    ppp_cstate_connected,
};

typedef struct
{
    struct list_head listpointer;
    char *buffer;
    int   size;
} ppp_packet;

typedef struct
{
    struct list_head input;
    struct list_head output;

    int input_cnt;
    int output_cnt;
    ID mstate;

    ID ilock;
    ID olock;

    int ppp_status;

    int state;
    int pppfd;

    char usrname[VPN_NAME_LEN];
    char passwd[VPN_PASSWD_LEN];

    struct siofd sio;
} vpnPPP;

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

static void _vpn_ppp_freedata(vpnPPP *ppp)
{
    struct list_head *entry = NULL;
    ppp_packet *data = NULL;

    osal_mutex_lock(ppp->ilock, OSAL_WAIT_FOREVER_TIME);
    while (!list_empty(&ppp->input))
    {
        entry = ppp->input.next;
        data = list_entry(entry, ppp_packet, listpointer);
        list_del(entry);
        FREE(data);
    }
    ppp->input_cnt = 0;
    osal_mutex_unlock(ppp->ilock);

    osal_mutex_lock(ppp->olock, OSAL_WAIT_FOREVER_TIME);
    while (!list_empty(&ppp->output))
    {
        entry = ppp->output.next;
        data = list_entry(entry, ppp_packet, listpointer);
        list_del(entry);
        FREE(data);
    }
    ppp->output_cnt = 0;
    osal_mutex_unlock(ppp->olock);
}

static void _vpn_ppp_status(vpnPPP *ppp, int status)
{
    osal_mutex_lock(ppp->mstate, OSAL_WAIT_FOREVER_TIME);
    ppp->ppp_status = status;
    osal_mutex_unlock(ppp->mstate);
}

static int _vpn_sio_write(sio_fd_t fd, char *buff, int len)
{
    struct siofd *sio = (struct siofd *) fd;
    vpnPPP *ppp = NULL;
    ppp_packet *data = NULL;

    if (sio == NULL || sio->priv == NULL)
    {
        return 0;
    }

    ppp = (vpnPPP *)sio->priv;
    if (ppp->output_cnt >= MAX_PAKCET_CNT)
    {
        GREPPP_DEBUG("VPNPPP: output list full\n");
        return 0;
    }
    data = MALLOC(sizeof(ppp_packet) + len);
    if (data == NULL)
    {
        GREPPP_ERROR("%s:%d  malloc failed\n", __FUNCTION__, __LINE__);
        return 0;
    }
    MEMSET(data, 0, sizeof(ppp_packet) + len);
    data->buffer = (char *)data + sizeof(ppp_packet);
    data->size = len;
    MEMCPY(data->buffer, buff, len);

    osal_mutex_lock(ppp->olock, OSAL_WAIT_FOREVER_TIME);
    list_add_tail(&data->listpointer, &ppp->output);
    ppp->output_cnt++;
    osal_mutex_unlock(ppp->olock);

    //GREPPP_DEBUG("%s:%d  write data:%d\n", __FUNCTION__, __LINE__ , len);
    return len;
}

static int _vpn_sio_read(sio_fd_t fd, char *buff, int len)
{
    int cpylen = 0;
    struct siofd *sio = (struct siofd *) fd;
    vpnPPP *ppp = NULL;
    struct list_head *entry = NULL;
    ppp_packet *data = NULL;

    if (sio == NULL || sio->priv == NULL)
    {
        return 0;
    }

    ppp = (vpnPPP *)sio->priv;
    if (!list_empty(&ppp->input))
    {
        entry = ppp->input.next;
        data = list_entry(entry, ppp_packet, listpointer);
        if (data->size <= len)
        {
            cpylen = data->size;
            MEMCPY(buff, data->buffer, cpylen);
            osal_mutex_lock(ppp->ilock, OSAL_WAIT_FOREVER_TIME);
            list_del(entry);
            ppp->input_cnt--;
            osal_mutex_unlock(ppp->ilock);
            FREE(data);
        }
        else
        {
            cpylen = len;
            MEMCPY(buff, data->buffer, cpylen);
            data->buffer += len;
            data->size -= len;
        }
    }
    else
    {
        osal_task_sleep(10);
    }
    //GREPPP_DEBUG("%s:%d  read data:%d\n", __FUNCTION__, __LINE__ , cpylen);
    return cpylen;
}

static void _vpn_ppp_free(vpnPPP *ppp)
{
    _vpn_ppp_freedata(ppp);
    if (ppp->mstate != INVALID_ID)
    {
        osal_mutex_delete(ppp->mstate);
        ppp->mstate = INVALID_ID;
    }
    if (ppp->ilock != INVALID_ID)
    {
        osal_mutex_delete(ppp->ilock);
        ppp->ilock = INVALID_ID;
    }
    if (ppp->olock != INVALID_ID)
    {
        osal_mutex_delete(ppp->olock);
        ppp->olock = INVALID_ID;
    }
    FREE(ppp);
}

static vpnPPP *_vpn_ppp_alloc()
{
    vpnPPP *ppp = MALLOC(sizeof(vpnPPP));
    if (ppp == NULL)
    {
        return NULL;
    }

    MEMSET(ppp, 0, sizeof(vpnPPP));
    INIT_LIST_HEAD(&ppp->input);
    INIT_LIST_HEAD(&ppp->output);
    ppp->mstate = osal_mutex_create();
    ppp->ilock = osal_mutex_create();
    ppp->olock = osal_mutex_create();

    if ((ppp->mstate == INVALID_ID) || (ppp->ilock == INVALID_ID) || (ppp->olock == INVALID_ID))
    {
        _vpn_ppp_free(ppp);
        return NULL;
    }

    ppp->sio.sio_read = _vpn_sio_read;
    ppp->sio.sio_write = _vpn_sio_write;
    ppp->sio.priv = ppp;
    ppp->pppfd = -1;
    return ppp;
}

static void _vpn_ppp_link_status(void *ctx, int errCode, void *arg)
{
    vpnPPP *ppp = (vpnPPP *)ctx;
    struct ppp_addrs *addrs = arg;
    int newstate = ppp_cstate_connect_err;

    switch (errCode)
    {
        case PPPERR_NONE:
            newstate = ppp_cstate_connected;
            GREPPP_DEBUG("ip_addr = %s\n", _inet_ntoa(addrs->our_ipaddr));
            GREPPP_DEBUG("netmask = %s\n", _inet_ntoa(addrs->netmask));
            GREPPP_DEBUG("dns1    = %s\n", _inet_ntoa(addrs->dns1));
            GREPPP_DEBUG("dns2    = %s\n", _inet_ntoa(addrs->dns2));
            dns_setserver(0, (ip_addr_t *)&addrs->dns1);
            dns_setserver(1, (ip_addr_t *)&addrs->dns2);
            netif_auto_default();
            break;
        case PPPERR_PARAM:
            GREPPP_DEBUG("ppp connection failed: Invalid parameter.\n");
            break;
        case PPPERR_OPEN:
            GREPPP_DEBUG("ppp connection failed: Unable to open PPP session.\n");
            break;
        case PPPERR_DEVICE:
            GREPPP_DEBUG("ppp connection failed: Invalid I/O device for PPP.\n");
            break;
        case PPPERR_ALLOC:
            GREPPP_DEBUG("ppp connection failed: Unable to allocate resources.\n");
            break;
        case PPPERR_USER:
            GREPPP_DEBUG("ppp connection failed: User interrupt.\n");
            break;
        case PPPERR_CONNECT:
            if (ppp->ppp_status == ppp_cstate_connected)
            {
                GREPPP_DEBUG("ppp Connection lost.\n");
                newstate = ppp_cstate_connectting;
            }
            else
            {
                newstate = ppp->ppp_status;
            }
            break;
        case PPPERR_AUTHFAIL:
            GREPPP_DEBUG("ppp connection failed: Failed authentication challenge.\n");
            break;
        case PPPERR_PROTOCOL:
            GREPPP_DEBUG("ppp connection failed: Failed to meet protocol.\n");
            break;
        default:
            GREPPP_DEBUG("ppp unknown errcode.\n");
            break;
    }

    _vpn_ppp_status(ppp, newstate);
}

static void _vpn_stop_ppp(vpnPPP *ppp)
{
    if (ppp->pppfd >= 0)
    {
        pppClose(ppp->pppfd);

        //wait ppp stack release resource
        while (ppp_state(ppp->pppfd))
        {
            osal_task_sleep(100);
        }

        ppp->pppfd = -1;
    }
}

static int _vpn_start_ppp(vpnPPP *ppp)
{
    if (STRLEN(ppp->usrname) > 0)
    {
        pppSetAuth(PPPAUTHTYPE_CHAP, ppp->usrname, ppp->passwd);
    }
    else
    {
        pppSetAuth(PPPAUTHTYPE_NONE, NULL, NULL);
    }
    int pd = pppOverSerialOpen((sio_fd_t)&ppp->sio, _vpn_ppp_link_status, ppp);
    ppp_set_netifname(pd, "vp");
    return pd;
}

static void _vpn_pppd(UINT32 param1, UINT32 param2)
{
    vpnPPP *ppp = (vpnPPP *)param1;
    int tmo = 0;

    ppp->state = PPP_STATE_OPEN;
    _vpn_ppp_status(ppp, ppp_cstate_connectting);

    while (ppp->state == PPP_STATE_OPEN)
    {
        switch (ppp->ppp_status)
        {
            case ppp_cstate_connectting:
                _vpn_ppp_freedata(ppp);

                if (ppp->pppfd >= 0)
                {
                    _vpn_stop_ppp(ppp);
                }

                //maybe the state has been changed in _vpn_ppp_link_status
                //so need ensure the state is ppp_state_connectting
                _vpn_ppp_status(ppp, ppp_cstate_connectting);
                ppp->pppfd = _vpn_start_ppp(ppp);

                //GREPPP_DEBUG("PPP data IN : %d\n", ppp->input_cnt);
                //GREPPP_DEBUG("PPP data OUT: %d\n", ppp->output_cnt);

                if (ppp->pppfd < 0)
                {
                    GREPPP_DEBUG("ppp open errror[ %d]\n", ppp->pppfd);
                    _vpn_ppp_status(ppp, ppp_cstate_connect_err);
                }
                else
                {
                    //check timeout
                    tmo = VPN_PPP_TIMEOUT;
                    while ((tmo > 0) && (ppp->state == PPP_STATE_OPEN) && ppp->ppp_status == ppp_cstate_connectting)
                    {
                        osal_task_sleep(100);
                        tmo -= 100;
                    }
                    if (tmo <= 0)
                    {
                        GREPPP_DEBUG("ppp open timeout[%d], restartting ...\n", ppp->pppfd);
                        _vpn_stop_ppp(ppp);
                        _vpn_ppp_status(ppp, ppp_cstate_connectting);
                    }
                }
                break;

            case ppp_cstate_connect_err:
                osal_task_sleep(1000); //restart after 1s.
                _vpn_ppp_status(ppp, ppp_cstate_connectting);
                break;

            case ppp_cstate_connected:
            default:
                osal_task_sleep(100); //just sleep
                break;
        }
    }

    //stop ppp
    _vpn_stop_ppp(ppp);
    _vpn_ppp_free(ppp);
}

UINT32 vpn_ppp_open(char *usr, char *passwd)
{
    vpnPPP *ppp = NULL;
    OSAL_T_CTSK ptask;

    ppp = _vpn_ppp_alloc();
    if (ppp == NULL)
    {
        return 0xFFFFFFFF;
    }

    strncpy(ppp->usrname, usr, VPN_NAME_LEN);
    strncpy(ppp->passwd, passwd, VPN_PASSWD_LEN);

    ptask.task = (FP)_vpn_pppd;
    ptask.itskpri = 20;
    ptask.quantum = 10;
    ptask.stksz = 0x2000;
    ptask.para1 = (int)ppp;
    ptask.para2 =  0;
    ptask.name[0] = 'p';
    ptask.name[1] = 'p';
    ptask.name[2] = 'p';

    if (OSAL_INVALID_ID == osal_task_create(&ptask))
    {
        _vpn_ppp_free(ppp);
        return 0xFFFFFFFF;
    }

    while (ppp->state == PPP_STATE_NONE)
    {
        osal_task_sleep(100);
    }
    return (UINT32)ppp;
}

void vpn_ppp_close(UINT32 fd)
{
    vpnPPP *ppp = (vpnPPP *)fd;

    if (fd == 0 || (int)fd == -1)
    {
        return;
    }
    ppp->state = PPP_STATE_CLOSE;
    GREPPP_DEBUG("%s:%d\n", __FUNCTION__, __LINE__);
}

int vpn_read_pppdata(int fd, char *buff, int len)
{
    int cpylen = 0;
    vpnPPP *ppp = (vpnPPP *)fd;
    struct list_head *entry = NULL;
    ppp_packet *data = NULL;

    if (fd == 0 || fd == -1)
    {
        return 0;
    }
    if (!list_empty(&ppp->output))
    {
        entry = ppp->output.next;
        data = list_entry(entry, ppp_packet, listpointer);
        if (data->size <= len)
        {
            cpylen = data->size;
            MEMCPY(buff, data->buffer, cpylen);
            osal_mutex_lock(ppp->olock, OSAL_WAIT_FOREVER_TIME);
            list_del(entry);
            ppp->output_cnt--;
            osal_mutex_unlock(ppp->olock);
            FREE(data);
        }
        else
        {
            cpylen = len;
            MEMCPY(buff, data->buffer, cpylen);
            data->buffer += len;
            data->size -= len;
        }
    }

    //GREPPP_DEBUG("%s:%d  read data:%d\n", __FUNCTION__, __LINE__ , cpylen);
    return cpylen;
}

int vpn_write_pppdata(int fd, char *buff, int len)
{
    vpnPPP *ppp = (vpnPPP *)fd;
    ppp_packet *data = NULL;

    if (fd == 0 || fd == -1)
    {
        return 0;
    }
    if (ppp->input_cnt >= MAX_PAKCET_CNT)
    {
        GREPPP_DEBUG("VPNPPP: input list full\n");
        return 0;
    }
    data = MALLOC(sizeof(ppp_packet) + len);
    if (data == NULL)
    {
        return 0;
    }
    MEMSET(data, 0, sizeof(ppp_packet) + len);
    data->buffer = (char *)data + sizeof(ppp_packet);
    data->size = len;
    MEMCPY(data->buffer, buff, len);
    osal_mutex_lock(ppp->ilock, OSAL_WAIT_FOREVER_TIME);
    list_add_tail(&data->listpointer, &ppp->input);
    ppp->input_cnt++;
    osal_mutex_unlock(ppp->ilock);

    //GREPPP_DEBUG("%s:%d  write data:%d\n", __FUNCTION__, __LINE__ , len);
    return len;
}

int vpn_ppp_select(int fd)
{
    vpnPPP *ppp = (vpnPPP *)fd;

    if (fd == 0 || fd == -1)
    {
        return 0;
    }
    return ppp->output_cnt;
}

int vpn_ppp_state(int fd)
{
    vpnPPP *ppp = (vpnPPP *)fd;

    if (fd == 0 || fd == -1)
    {
        return 0;
    }
    return (ppp->ppp_status == ppp_cstate_connected);
}

int vpn_mppe_enbale(int fd, int enable)
{
    vpnPPP *ppp = (vpnPPP *)fd;

    if (fd == 0 || fd == -1)
    {
        return 0;
    }
    ccp_mppe(ppp->pppfd, enable);
    return 1;
}

#endif
