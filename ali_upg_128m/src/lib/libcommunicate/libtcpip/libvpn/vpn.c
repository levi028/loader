
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
#include <api/libtcpip/lwip/inet.h>
#include <api/libvpn/vpn.h>
#include <hld/hld_dev.h>

#include "pptp_gre.h"
#include "vpnpptp.h"

#define VPN_DEBUG libc_printf
//#define VPN_DEBUG(...)

typedef enum
{
    VPN_STATE_NONE = 0,
    VPN_STATE_OPEN,
    VPN_STATE_CLOSE,
} VPN_STATE;

typedef enum
{
    VPN_CONNECT_STATE_NONE = 0,
    VPN_CONNECT_STATE_START,
    VPN_CONNECT_STATE_CALLID,
    VPN_CONNECT_STATE_PPP,
    VPN_CONNECT_STATE_CONNECTED,
} VPN_CONNECT_STATE;

typedef struct
{
    ID  taskid;
    vpn_gre gre;
    vpn_pptp pptp;
    vpn_cfg vpn;
    ID  mutex;
    int state; // VPN_STATE
    int cstate; //VPN_CONNECT_STATE
    vpn_notify notify;
} vpnmgr;


static vpnmgr *g_vpnmgr = NULL;

extern int vpn_gre_open(vpn_cfg *vpncfg, vpn_gre *vpngre);
extern int vpn_mppe_enbale(int fd, int enable);
extern void vpn_gre_close(vpn_gre *vpngre);
extern int vpn_ppp_state(int fd);
extern void pppInit(void);
extern void netif_auto_default();
extern struct netif *netif_find(char *name);
extern void pcap_stop(struct netif *netif);
extern char *pcap_start(struct netif *netif, char *save_dir);

static void _vpn_notify(int msg)
{
    if (g_vpnmgr && g_vpnmgr->notify)
    {
        g_vpnmgr->notify(msg);
    }
}

static void _vpn_task(UINT32 param1, UINT32 param2)
{
    int ret = 0;
    int tmo = 0;

    g_vpnmgr->state = VPN_STATE_OPEN;
    g_vpnmgr->cstate = VPN_CONNECT_STATE_START;

    _vpn_notify(VPN_MSG_START_CONNECTTING);
    while (g_vpnmgr->state == VPN_STATE_OPEN)
    {
        switch (g_vpnmgr->cstate)
        {
            case VPN_CONNECT_STATE_START:
                if (tmo > 0)
                {
                    osal_task_sleep(100);
                    tmo -= 100;
                    break;
                }
                //
                ret = vpn_gre_open(&g_vpnmgr->vpn, &g_vpnmgr->gre);
                if (ret == 0)
                {
                    tmo = 5 * 1000;
                    break;
                }
                vpn_mppe_enbale(g_vpnmgr->gre.pty_fd, g_vpnmgr->vpn.encryt);
                
                MEMSET(&g_vpnmgr->pptp, 0, sizeof(vpn_pptp));
                g_vpnmgr->pptp.saddr = g_vpnmgr->vpn.saddr;
                g_vpnmgr->pptp.sport = 1723;
                ret = vpn_pptp_start(&g_vpnmgr->pptp);
                if (ret == 0)
                {
                    tmo = 5 * 1000;
                    break;
                }
                //
                tmo = 30 * 1000;
                g_vpnmgr->cstate = VPN_CONNECT_STATE_CALLID;
                break;

            case VPN_CONNECT_STATE_CALLID:
                ret = vpn_pptp_getcallid(&g_vpnmgr->pptp, &g_vpnmgr->gre.callid, &g_vpnmgr->gre.peer_callid);
                if (ret)
                {
                    tmo = 30 * 1000;
                    g_vpnmgr->cstate = VPN_CONNECT_STATE_PPP;
                    VPN_DEBUG("Start PPP Authenicate! Timeout:%ds\n", tmo / 1000);
                    break;
                }

                osal_task_sleep(5);
                tmo -= 5;
                if (tmo <= 0 || vpn_pptp_error(&g_vpnmgr->pptp))
                {
                    _vpn_notify(VPN_MSG_CONNECT_SERVER_FAILED);
                    tmo = 10 * 1000;
                    VPN_DEBUG("PPTP connect timeout, try again after %ds\n", tmo / 1000);
                    g_vpnmgr->cstate = VPN_CONNECT_STATE_START;
                    vpn_gre_close(&g_vpnmgr->gre);
                    vpn_pptp_close(&g_vpnmgr->pptp);
                }
                break;

            case VPN_CONNECT_STATE_PPP:
                if (vpn_ppp_state(g_vpnmgr->gre.pty_fd))
                {
                    _vpn_notify(VPN_MSG_VPN_SUCESS);
                    g_vpnmgr->cstate = VPN_CONNECT_STATE_CONNECTED;
                    VPN_DEBUG("PPP authenicate success\n");
                    break;
                }

                if (tmo <= 0 || vpn_pptp_error(&g_vpnmgr->pptp))
                {
                    _vpn_notify(VPN_MSG_AUTHENTICATE_FAILED);
                    tmo = 10 * 1000;
                    VPN_DEBUG("PPP authenicate timeout, Resart after %ds!\n", tmo / 1000);
                    g_vpnmgr->cstate = VPN_CONNECT_STATE_START;
                    vpn_gre_close(&g_vpnmgr->gre);
                    vpn_pptp_close(&g_vpnmgr->pptp);
                    break;
                }

                tmo -= 100;
                osal_task_sleep(100);
                break;

            case VPN_CONNECT_STATE_CONNECTED:
            default:
                if (vpn_pptp_error(&g_vpnmgr->pptp))
                {
                    _vpn_notify(VPN_MSG_CONNECTION_LOST);
                    tmo = 10 * 1000;
                    VPN_DEBUG("VPN Connection lost, Restart after %ds!\n", tmo / 1000);
                    g_vpnmgr->cstate = VPN_CONNECT_STATE_START;
                    vpn_gre_close(&g_vpnmgr->gre);
                    vpn_pptp_close(&g_vpnmgr->pptp);
                    break;
                }
                osal_task_sleep(100);
                break;
        }
    }

    _vpn_notify(VPN_MSG_VPN_CLOSED);
    vpn_gre_close(&g_vpnmgr->gre);
    vpn_pptp_close(&g_vpnmgr->pptp);
    g_vpnmgr->cstate = VPN_CONNECT_STATE_NONE;
    g_vpnmgr->state = VPN_STATE_NONE;
    g_vpnmgr->taskid = INVALID_ID;
    VPN_DEBUG("VPN has been closed\n");
}

int vpn_init(vpn_notify notify)
{
    vpnmgr *ptr = NULL;
    if (g_vpnmgr) { return 1; }

    ptr = MALLOC(sizeof(vpnmgr));
    if (ptr == NULL)
    {
        VPN_DEBUG("VPN init failed\n");
        return 0;
    }

    MEMSET(ptr, 0, sizeof(vpnmgr));
    ptr->notify = notify;
    ptr->mutex = osal_mutex_create();
    ptr->gre.slock = osal_mutex_create();
    if (OSAL_INVALID_ID == ptr->mutex || OSAL_INVALID_ID == ptr->gre.slock)
    {
        if (ptr->mutex != OSAL_INVALID_ID)
        {
            osal_mutex_delete(ptr->mutex);
        }
        if (ptr->gre.slock != OSAL_INVALID_ID)
        {
            osal_mutex_delete(ptr->gre.slock);
        }
        FREE(ptr);
        VPN_DEBUG("VPN init failed\n");
        return 0;
    }
    pppInit();
    VPN_DEBUG("VPN init success\n");
    g_vpnmgr = ptr;
    return 1;
}

int vpn_start(vpn_cfg *vpn)
{
    int ret = 0;
    OSAL_T_CTSK ptask;

    if (g_vpnmgr == NULL)
    {
        VPN_DEBUG("VPN is not init!\n");
        return 0;
    }
    //pcap_start(netif_find_by_type(HLD_DEV_TYPE_NET), "/mnt/uda1");

    osal_mutex_lock(g_vpnmgr->mutex, OSAL_WAIT_FOREVER_TIME);
    if (g_vpnmgr->state == VPN_STATE_NONE)
    {
        memcpy(&g_vpnmgr->vpn, vpn, sizeof(vpn_cfg));
        ptask.task = (FP)_vpn_task;
        ptask.itskpri = 20;
        ptask.quantum = 10;
        ptask.stksz = 0x2000;
        ptask.para1 = 0;
        ptask.para2 =  0;
        ptask.name[0] = 'v';
        ptask.name[1] = 'p';
        ptask.name[2] = 'n';
        g_vpnmgr->taskid = osal_task_create(&ptask);
        if (OSAL_INVALID_ID == g_vpnmgr->taskid)
        {
            ret = 0;
        }
        else
        {
            while (g_vpnmgr->state == VPN_STATE_NONE)
            {
                osal_task_sleep(100);
            }
            ret = 1;
        }
    }
    else
    {
        ret = 1;
        VPN_DEBUG("VPN is running\n");
    }
    osal_mutex_unlock(g_vpnmgr->mutex);

    return ret;
}

void vpn_stop()
{
    if (g_vpnmgr == NULL)
    {
        VPN_DEBUG("VPN is not init!\n");
        return;
    }

    osal_mutex_lock(g_vpnmgr->mutex, OSAL_WAIT_FOREVER_TIME);
    if (g_vpnmgr->state == VPN_STATE_OPEN)
    {
        g_vpnmgr->state = VPN_STATE_CLOSE;
        while (g_vpnmgr->state == VPN_STATE_CLOSE)
        {
            osal_task_sleep(100);
        }
    }
    osal_mutex_unlock(g_vpnmgr->mutex);
    netif_auto_default();
    //pcap_stop(netif_find_by_type(HLD_DEV_TYPE_NET));
}

int vpn_state()
{
    return (g_vpnmgr && g_vpnmgr->state == VPN_STATE_OPEN);
}

char *vpn_pcap(char *save_dir, UINT8 start)
{
    struct netif *nf = netif_find("vp");

    if(nf)
    {
        if(start) 
            return pcap_start(nf, save_dir);
        else 
            pcap_stop(nf);
    }
    return NULL;
}
#endif
