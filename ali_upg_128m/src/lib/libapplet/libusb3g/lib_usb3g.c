/******************************************************************************
 * Ali Corp. All Rights Reserved. 2012 Copyright (C)
 *
 * File: lib_usb3g.c
 *
 * Description: -
 *     USB 3G Dongle Api
 * History
 * --------------------
 * 1. 2012-12-25, Doy.Dong written
 * --------------------
 *
 ******************************************************************************/

#include <sys_config.h>

#ifdef USB3G_DONGLE_SUPPORT
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <hld/ali_3g_mdm/ali_3g_mdm_api.h>
#include <hld/ali_3g_mdm/termios.h>
#include "api/libusb3g/lib_usb3g.h"
#include "api/libtcpip/lwip/lwipopts.h"
#include <api/libtcpip/lwip/netif.h>
#include <api/libtcpip/lwip/dns.h>
#include <api/libnet/libnet.h>
#include "hld/net/net.h"
#include <hld/hld_dev.h>

#define SPRINTF   sprintf

#define USB3G_DEBUG libc_printf
#define USB3G_ERROR libc_printf

typedef struct
{
    ID dongle_mutex;
    ID isp_mutex;
    int dongle_num;     //dongle count
    struct usb3gdongle *dongle_list;
    struct isp_info *isplist;
    usb3g_dongle_cb callback;
} usb3gdongle_mgr;

static usb3gdongle_mgr *g_usb3g_dongle_mgr = NULL;


#if 1 /* copy from lwip ppp.h  --Doy.Dong, 2012-12-31*/
#define PPPERR_NONE      0 /* No error. */
#define PPPERR_PARAM    -1 /* Invalid parameter. */
#define PPPERR_OPEN     -2 /* Unable to open PPP session. */
#define PPPERR_DEVICE   -3 /* Invalid I/O device for PPP. */
#define PPPERR_ALLOC    -4 /* Unable to allocate resources. */
#define PPPERR_USER     -5 /* User interrupt. */
#define PPPERR_CONNECT  -6 /* Connection lost. */
#define PPPERR_AUTHFAIL -7 /* Failed authentication challenge. */
#define PPPERR_PROTOCOL -8 /* Failed to meet protocol. */

enum ppp_auth_type
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

extern void pppInit(void);
extern void pppSetAuth(enum ppp_auth_type authType, const char *user, const char *passwd);
extern int pppOverSerialOpen(sio_fd_t fd, void (*linkStatusCB)(void *ctx, int errCode, void *arg), void *linkStatusCtx);
extern void ppp_set_netifname(int pd, char *name);
extern int pppClose(int pd);
extern int ppp_state(int pd);
extern void pcap_stop(struct netif *netif);
extern char *pcap_start(struct netif *netif, char *save_dir);
extern void dns_setserver(u8_t numdns, struct ip_addr *dnsserver);

#endif /****copy from lwip ppp.h****/

static void _usb3g_dongle_set_state(struct usb3gdongle *dongle, int newstate);
static int _modem_can_read(usb3gdongle_port *modem, int timeout);

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

static char *_strstr(char *s1, int len, char *s2)
{
    char *tmp = s1;

    while (tmp < s1 + len)
    {
        if (*tmp == *s2 && MEMCMP(tmp, s2, STRLEN(s2)) == 0)
        {
            return tmp;
        }

        tmp++;
    }

    return NULL;
}

static char *_indexof(const char *s, char c, int index)
{
    char *tmp = (char *)s;
    int idx = 0;

    while (tmp && *tmp)
    {
        if ( *tmp == c)
        {
            idx++;
        }

        if (idx == index)
        {
            return tmp;
        }

        tmp++;
    }

    return NULL;
}

static void _replace(char *s, int len, char c1, char c2)
{
    int i = 0;

    while ( i < len)
    {
        if (*(s + i) == c1)
        {
            *(s + i) = c2;
        }

        i++;
    }
}
static void _dump_hex(const char *buff , int len, char *prestr)
{
    int i = 0, j = 0;
    int idx = 0;
    int hexlen = 4 * len;
    char c = 0;
    int pre_7d = 0;

    if (len == 0)
    {
        return;
    }

    if (hexlen < 128)
    {
        hexlen = 128;
    }

    char *hex = MALLOC(hexlen);

    if (hex == NULL)
    {
        return ;
    }

    MEMSET(hex, 0, hexlen);
    sprintf(hex, "%s(%d):\n", prestr, len);
    idx = STRLEN(hex);

    for (i = 0; i < len; i++)
    {
        c = buff[i];

        if (c == 0x7d)
        {
            pre_7d = 1;
            continue;
        }

        if (pre_7d == 1)
        {
            switch (c)
            {
                case 0x5d:
                    c = 0x7d;
                    break;
                case 0x5e:
                    c = 0x7e;
                    break;
                default:
                    c ^= 0x20;
                    break;
            }
        }

        j++;
        sprintf(hex + idx, "%02x ", c & 0xff);
        pre_7d = 0;
        idx += 3;

        if (j % 16 == 0)
        {
            hex[idx] = '\n';
            idx++;
        }
    }

    USB3G_DEBUG("%s\n", hex);
    FREE(hex);
}

static int _read(int fd, char *buff, int len)
{
    int ret = 0;
    struct ali_3g_modem_dev *dev = NULL;
    usb3gdongle_port *modem = (usb3gdongle_port *)fd;

    if (modem->hdl == -1 || modem->private == NULL || modem->valid != 1)
    {
        return 0;
    }

    dev = (struct ali_3g_modem_dev *)modem->private;
    ret = dev->read(modem->hdl, buff, len);
    return ret;
}

static int _write(int fd, char *buff, int len)
{
    struct ali_3g_modem_dev *dev = NULL;
    usb3gdongle_port *modem = (usb3gdongle_port *)fd;

    if (modem->hdl == -1 || modem->private == NULL || modem->valid != 1)
    {
        return 0;
    }

    dev = (struct ali_3g_modem_dev *)modem->private;
    return dev->write(modem->hdl, buff, len);
}

static int _usb_sio_write(sio_fd_t fd, char *buff, int len)
{
    struct siofd *sio = (struct siofd *) fd;
    usb3gdongle_port *modem = NULL;
    
    if (sio == 0 || sio->priv == NULL)
    {
        return 0;
    }
    //_dump_hex(buff, len, "write");
    modem = (usb3gdongle_port *)sio->priv;
    return _write((int)modem, buff, len);
}

static int _usb_sio_read(sio_fd_t fd, char *buff, int len)
{
    int ret = 0;
    struct siofd *sio= (struct siofd *)fd;
    usb3gdongle_port *modem = NULL;
    
    if (sio == 0 || sio->priv == NULL)
    {
        return 0;
    }

    modem = (usb3gdongle_port *)sio->priv;
    if (_modem_can_read(modem, 0) > 0)
    {
        ret = _read((int)modem, buff, len);
        //_dump_hex(buff, ret, "read");
    }

    return ret;
}

static int _modem_can_read(usb3gdongle_port *modem, int timeout)
{
    int dotry = timeout / 10;
    int ret = -1;
    struct ali_3g_modem_dev *dev = NULL;

    if (modem == NULL || modem->private == NULL || modem->valid != 1)
    {
        return ret;
    }

    dev = (struct ali_3g_modem_dev *)(modem->private);
    ret = dev->ioctl(modem->hdl, ALI_MDM_IOCTL_READABLE, 0);

    while (ret == 0 && dotry-- > 0)
    {
        osal_task_sleep(10);
        ret = dev->ioctl(modem->hdl, ALI_MDM_IOCTL_READABLE, 0);
    }

    if (ret < 0)
    {
        modem->valid = 0;
    }

    return ret;
}

static int _modem_write(usb3gdongle_port *modem, char *buff, int len)
{
    int ret = -1;

    if (modem == NULL || buff == NULL || len == 0 || modem->valid != 1)
    {
        return -1;
    }

    ret = _write((int)modem, buff, len);

    if (ret < 0)
    {
        modem->valid = 0;
    }

    return ret;
}

static int _modem_read(usb3gdongle_port *modem, char *buff, int len)
{
    int ret = 0;
    int ret2 = 0;

    if (modem == NULL || buff == NULL || len == 0 || modem->valid != 1)
    {
        return 0;
    }

    if (_modem_can_read(modem, 1000) > 0)
    {
        ret += (ret2 = _read((int)modem, buff + ret, len - ret));

        if (ret2 < 0)
        {
            modem->valid = 0;
        }

        while (ret2 > 0 && modem->valid == 1 && _modem_can_read(modem, 100) > 0 && ret < len )
        {
            ret += (ret2 = _read((int)modem, buff + ret, len - ret));

            if (ret2 < 0)
            {
                modem->valid = 0;
                break;
            }
        }
    }

    return ret;
}

int modem_sendat(usb3gdongle_port *modem, char *atcmd, char *response, int reslen)
{
    #define RETRY_SEND 4
    int ret = 0;
    int recv_response = 0;
    int offset = 0;
    char buff[512];
    int buff_len = 512;
    int checkval = 0;
    int retry = 1;
    int moredata = 0;

    if (atcmd == NULL || STRLEN(atcmd) == 0 || modem == NULL || modem->valid != 1)
    {
        return 0;
    }

    checkval = (response != NULL && *response != 0);

    do
    {
        //make modem happy
        _modem_write(modem, "\r\r\r\r\r", 5);
        //send AT command to modem
        _modem_write(modem, atcmd, STRLEN(atcmd));
        USB3G_DEBUG("modem send: %s\n", atcmd);

        if (response == NULL) //needn't receive response
        {
            return 1;
        }

        do
        {
            moredata = 0;
            MEMSET(buff, 0, buff_len);
            ret = _modem_read(modem, buff, buff_len - 2);
            _replace(buff, buff_len - 2, '\r', ' ');
            _replace(buff, buff_len - 2, '\n', ' ');
            USB3G_DEBUG("        [%d].recv[%d]: %s\n", moredata, ret, buff);

            if (ret == buff_len - 2)
            {
                moredata = 1;
            }

            if (checkval) //check response value
            {
                if (!recv_response  && _strstr(buff, buff_len - 2, response))
                {
                    recv_response = 1;
                }
            }
            else //save response data
            {
                if (offset >= reslen)
                {
                    break;
                }

                if (reslen - offset < (int)STRLEN(buff))
                {
                    MEMCPY(response + offset, buff, reslen - offset);
                    break;
                }
                else
                {
                    STRCPY(response + offset, buff);
                    offset += STRLEN(buff);

                    if (offset >= reslen)
                    {
                        break;
                    }
                }
            }
        }
        while (moredata && modem->valid == 1);

        //receive response success, needn't retry
        if (recv_response || offset > 0)
        {
            break;
        }
    }
    while (retry++ < RETRY_SEND && modem->valid == 1);

    return (offset || recv_response);
}


static void _modem_reset(usb3gdongle_port *modem)
{
    struct ali_3g_modem_dev *dev = NULL;
    unsigned int portdata;

    if (modem == NULL || modem->valid != 1)
    {
        return;
    }

    dev = (struct ali_3g_modem_dev *)(modem->private);
    portdata = TIOCM_DTR | TIOCM_RTS;

    if ( dev->ioctl(modem->hdl, ALI_MDM_IOCTL_TIOCMBIS, (UINT32)&portdata) < 0)
    {
        USB3G_DEBUG("modem hangup: ALI_MDM_IOCTL_TIOCMGET Failed\n");
        return;
    }

    _modem_write(modem, "\r\r\r\r\r", 5);
    _modem_write(modem, "\r\r\r\r\r", 5);
    _modem_write(modem, "\r\r\r\r\r", 5);
    //reset
    modem_sendat(modem, "ATZ\r", "OK", 0);
    USB3G_DEBUG("modem reset!!!\n");
    _modem_write(modem, "\r\r\r\r\r", 5);
}

void _modem_setup(INT32 m_fd)
{
    struct termios options;
    // get the current options
    ali_3g_modem_tcgetattr(m_fd, &options);
    /* Set termios to raw mode */
    /* input modes - clear indicated ones giving: no break, no CR to NL,
        no parity check, no strip char, no start/stop output (sic) control */
    options.c_iflag &= ~(BRKINT  | INPCK | ISTRIP | IUCLC | IGNCR | ICRNL | INLCR | IXON | PARMRK);
    /* output modes - clear giving: no post processing such as NL to CR+NL */
    options.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    options.c_cflag |= (CS8);
    /* local modes - clear giving: echoing off, canonical off (no erase with  backspace, ^U,...),
        no extended functions, no signal chars (^Z,^C) */
    options.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer */
    options.c_cc[VMIN] = 5;
    options.c_cc[VTIME] = 8; /* after 5 bytes or .8 seconds after first byte seen  */
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0; /* immediate - anything       */
    options.c_cc[VMIN] = 2;
    options.c_cc[VTIME] = 0; /* after two bytes, no timer  */
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 8; /* after a byte or .8 seconds */
    ali_3g_modem_cfsetospeed (&options, B115200);
    ali_3g_modem_cfsetispeed (&options, B115200);
    ali_3g_modem_tcsetattr(m_fd, TCSAFLUSH,  &options);
    ali_3g_modem_tcflush(m_fd, TCIOFLUSH);
}

static int _modem_hangup(usb3gdongle_port *modem)
{
    struct ali_3g_modem_dev *dev = NULL;
    unsigned int portdata;
    char buff[512];
    int buff_len = 512;

    if (modem == NULL || modem->valid != 1)
    {
        return 0;
    }

    dev = (struct ali_3g_modem_dev *)(modem->private);
    portdata = TIOCM_DTR;

    if ( dev->ioctl(modem->hdl, ALI_MDM_IOCTL_TIOCMBIC, (UINT32)&portdata) < 0)
    {
        USB3G_DEBUG("modem hangup: ALI_MDM_IOCTL_TIOCMBIC Failed\n");
        return 0;
    }

    if (_modem_read(modem, buff, buff_len - 2) > 0)
    {
        USB3G_DEBUG("Receive: %s\n", buff);
    }

    modem_sendat(modem, "ATH\r", "OK", 0);
    USB3G_DEBUG("modem hangup!!!\n");
    return 0;
}

static int _modem_dial( usb3gdongle_port *modem, char *phone_num)
{
    char cmd[128];

    if (modem == NULL || modem->valid != 1)
    {
        return 0;
    }

    MEMSET(cmd, 0, sizeof(cmd));
    SPRINTF(cmd, "ATDT%s\r", phone_num);
    return modem_sendat(modem, cmd, "CONNECT", 0);
}


static int _modem_get_vendor_id(usb3gdongle_port *modem, char *vendor_id, int vendor_id_len)
{
    char *ptr = NULL;
    char *s = "AT+CGMI\r";
    char tmpbuf[512];

    if (modem == NULL || vendor_id == NULL || vendor_id_len == 0 || modem->valid != 1)
    {
        return 0;
    }

    MEMSET(tmpbuf, 0, sizeof(tmpbuf));
    modem_sendat(modem, s, tmpbuf, sizeof(tmpbuf));
    ptr = _strstr(tmpbuf, sizeof(tmpbuf), "AT+CGMI");

    if (ptr)
    {
        ptr += 7;

        while (*ptr == ' ')
        {
            ptr++;
        }

        char *ptr2 = _indexof(ptr, ' ', 1);

        if (ptr2)
        {
            MEMCPY(vendor_id, ptr, (ptr2 - ptr) > vendor_id_len ? vendor_id_len : (ptr2 - ptr));
            return 1;
        }
    }

    return 0;
}

static int _modem_get_device_id(usb3gdongle_port *modem, char *device_id, int device_id_len)
{
    char *ptr = NULL;
    char *s = "AT+CGMM\r";
    char tmpbuf[512];

    if (modem == NULL || device_id == NULL || device_id_len == 0 || modem->valid != 1)
    {
        return 0;
    }

    MEMSET(tmpbuf, 0, sizeof(tmpbuf));
    modem_sendat(modem, s, tmpbuf, sizeof(tmpbuf));
    ptr = _strstr(tmpbuf, sizeof(tmpbuf), "AT+CGMM");

    if (ptr)
    {
        ptr += 7;

        while (*ptr == ' ')
        {
            ptr++;
        }

        char *ptr2 = _indexof(ptr, ' ', 1);

        if (ptr2)
        {
            MEMCPY(device_id, ptr, (ptr2 - ptr) > device_id_len ? device_id_len : (ptr2 - ptr));
            return 1;
        }
    }

    return 0;
}

static int _modem_set_apn(usb3gdongle_port *modem, char *apn)
{
    char s[512];

    if (modem == NULL || modem->valid != 1)
    {
        return 1;
    }

    MEMSET(s, 0, sizeof(s));

    if (apn == NULL || STRLEN(apn) == 0)
    {
        STRCPY(s, "AT+CGDCONT=1,\"IP\"\r");
    }
    else
    {
        SPRINTF(s, "AT+CGDCONT=1,\"IP\",\"%s\"\r", apn);
    }

    return modem_sendat(modem, s, "OK", 0);
}

static int _modem_get_signal_strength(usb3gdongle_port *modem, int *quality, int *simerr)
{
    char buff[512];
    char *ptr = NULL;
    char *atcmd = "AT+CSQ\r";

    if (modem == NULL || quality == NULL || modem->valid != 1)
    {
        return 0;
    }

    //send AT command to modem
    MEMSET(buff, 0, sizeof(buff)); //must clean the buff
    modem_sendat(modem, atcmd, buff, sizeof(buff));
    ptr = _strstr(buff, sizeof(buff), "+CSQ:");

    if (ptr) //check response data
    {
        char *s = NULL;

        if ((s = _indexof(ptr, ':', 1)))
        {
            *quality = ATOI(s + 1);
        }

        if ((s = _indexof(ptr, ',', 1)))
        {
            if (simerr)
            {
                *simerr = (ATOI(s + 1) != 99);
            }
        }

        return 1;
    }

    return 0;
}

static int _modem_get_imsi(usb3gdongle_port *modem, char *imsi)
{
    int retry = 0;
    char *ptr = NULL;
    char *s = "AT+CIMI\r";
    char tmpbuf[512];

    if (modem == NULL || imsi == NULL || modem->valid != 1)
    {
        return 0;
    }

    while (retry++ < 3 && !ptr)
    {
        MEMSET(tmpbuf, 0, sizeof(tmpbuf));
        modem_sendat(modem, s, tmpbuf, sizeof(tmpbuf));
        ptr = _strstr(tmpbuf, sizeof(tmpbuf), "OK");

        if (ptr)
        {
            int flag = 1;
            ptr = _strstr(tmpbuf, sizeof(tmpbuf), "AT+CIMI");

            if (ptr == NULL)
            {
                if (_strstr(tmpbuf, sizeof(tmpbuf), "^BOOT"))
                {
                    ptr = _strstr(tmpbuf, sizeof(tmpbuf), "OK");
                    flag = -1;
                }
            }

            if (ptr == NULL)
            {
                ptr = tmpbuf;
            }

            while (*ptr < 0x30 || *ptr > 0x39)
            {
                ptr += (flag * 1);
            }

            if (flag == -1)
            {
                ptr -= 14;
            }

            MEMCPY(imsi, ptr, MCC_LEN + MNC_LEN + 10);

            if (imsi[MCC_LEN + MNC_LEN + 10 - 1] > '9' || imsi[MCC_LEN + MNC_LEN + 10 - 1] < '0')
            {
                imsi[MCC_LEN + MNC_LEN + 10 - 1] = 0;
            }

            USB3G_DEBUG("get imsi: %s\n", imsi);
            return 1;
        }
        else if (_strstr(tmpbuf, sizeof(tmpbuf), "required")) //maybe need puk code
        {
            break;
        }
    }

    return 0;
}

static void _usb3g_dongle_hangup(struct usb3gdongle *dongle)
{
    int i = 0;
    usb3gdongle_port *modem = NULL;

    if (dongle == NULL || dongle->valid != 1)
    {
        return;
    }

    for (i = 0; i < MAX_DONGLE_PORT_CNT; i++)
    {
        modem = &(dongle->ports[i]);

        if (modem->valid != 1)
        {
            continue;
        }

        _modem_hangup(modem);
    }
}

static usb3gdongle_port *_usb3g_get_free_port(struct usb3gdongle *dongle)
{
    int i = 0;
    usb3gdongle_port *port = NULL;

    if (dongle == NULL || dongle->valid != 1)
    {
        return NULL;
    }

    for (i = 0; i < MAX_DONGLE_PORT_CNT; i++)
    {
        if (dongle->ports[i].valid != 1)
        {
            port = &dongle->ports[i];
            MEMSET(port, 0, sizeof(usb3gdongle_port));
            break;
        }
    }

    return port ;
}



static void _usb3g_dongle_link_status(void *ctx, int err_code, void *arg)
{
    struct usb3gdongle *dongle = (struct usb3gdongle *)ctx;
    struct ppp_addrs *addrs = arg;
    int newstate = dongle_state_connect_err;

    switch (err_code)
    {
        case PPPERR_NONE:
            newstate = dongle_state_connected;
            USB3G_DEBUG("ip_addr = %s\n", _inet_ntoa(addrs->our_ipaddr));
            USB3G_DEBUG("netmask = %s\n", _inet_ntoa(addrs->netmask));
            USB3G_DEBUG("dns1    = %s\n", _inet_ntoa(addrs->dns1));
            USB3G_DEBUG("dns2    = %s\n", _inet_ntoa(addrs->dns2));
            dns_setserver(0, (ip_addr_t *)&addrs->dns1);
            dns_setserver(1, (ip_addr_t *)&addrs->dns1);
#if 0/*because only support one net interface in same time.*/

            do
            {
                struct netif *netif = netif_find_by_type(HLD_DEV_TYPE_3G);

                if (netif)
                {
                    IP_LOC_CFG cfg;
                    struct ip_addr  ipaddr;
                    cfg.ip_addr = netif->ip_addr.addr;
                    cfg.gateway = netif->gw.addr;
                    cfg.subnet_mask = netif->netmask.addr;
                    ipaddr = dns_getserver(0);
                    cfg.dns = ipaddr.addr;
                    ipaddr = dns_getserver(1);
                    cfg.dns2 = ipaddr.addr;
                    cfg.dhcp_on = 1;
                    set_local_ip_cfg(&cfg);
                }
            }
            while (0);

#endif
            break;
        case PPPERR_PARAM:
            USB3G_DEBUG("ppp connection failed: Invalid parameter.\n");
            break;
        case PPPERR_OPEN:
            USB3G_DEBUG("ppp connection failed: Unable to open PPP session.\n");
            break;
        case PPPERR_DEVICE:
            USB3G_DEBUG("ppp connection failed: Invalid I/O device for PPP.\n");
            break;
        case PPPERR_ALLOC:
            USB3G_DEBUG("ppp connection failed: Unable to allocate resources.\n");
            break;
        case PPPERR_USER:
            USB3G_DEBUG("ppp connection failed: User interrupt.\n");
            break;
        case PPPERR_CONNECT:

            if (dongle->state == dongle_state_connected)
            {
                USB3G_DEBUG("ppp Connection lost.\n");
                newstate = dongle_state_connectting;
            }
            else
            {
                newstate = dongle->state;
            }

            break;
        case PPPERR_AUTHFAIL:
            USB3G_DEBUG("ppp connection failed: Failed authentication challenge.\n");
            break;
        case PPPERR_PROTOCOL:
            USB3G_DEBUG("ppp connection failed: Failed to meet protocol.\n");
            break;
        default:
            USB3G_DEBUG("ppp unknown errcode.\n");
            break;
    }

    if (dongle->do_connect)
    {
        _usb3g_dongle_set_state(dongle, newstate);
    }
}

static usb3gdongle_port *_usb3g_dongle_dial(struct usb3gdongle *dongle)
{
    int i = 0;
    usb3gdongle_port *modem = NULL;

    for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->do_connect && dongle->valid == 1; i++)
    {
        modem = &(dongle->ports[i]);

        if (!modem->valid || dongle->valid != 1)
        {
            continue;
        }

        modem->used = 1;
        if (dongle->do_connect && dongle->valid == 1)
        {
            _modem_hangup(modem);
        }

        if (dongle->do_connect && dongle->valid == 1)
        {
            _modem_reset(modem);
        }

        if (dongle->do_connect && dongle->valid == 1)
        {
            _modem_set_apn(modem, dongle->config.isp.apn);
        }

        if (dongle->do_connect && dongle->valid == 1)
        {
            modem_sendat(modem, "ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0\r", "OK", 0);
        }

        //add this callback for customer do extension.
        //maybe some special dongles need do special action before dailup.
        if (dongle->do_connect && dongle->valid == 1 && g_usb3g_dongle_mgr->callback)
        {
            g_usb3g_dongle_mgr->callback(USB3G_DONGLE_DO_CONNECT, dongle);
        }

        if (dongle->do_connect && dongle->valid == 1 && _modem_dial(modem, dongle->config.isp.phone_num))
        {
            return modem;
        }
        modem->used = 0;
    }

    return NULL;
}

static int _usb3g_start_ppp(struct usb3gdongle *dongle, usb3gdongle_port *modem)
{
    int pd = 0;

    if (dongle == NULL || modem == NULL || dongle->valid != 1)
    {
        return -1;
    }
    pppSetAuth(PPPAUTHTYPE_NONE, dongle->config.isp.user, dongle->config.isp.passwd);
    if (STRLEN(dongle->config.isp.user) > 0)
    {
        pppSetAuth(PPPAUTHTYPE_CHAP, dongle->config.isp.user, dongle->config.isp.passwd);
    }
    
    dongle->sio.priv = modem;
    pd = pppOverSerialOpen(&dongle->sio, _usb3g_dongle_link_status, dongle);
    ppp_set_netifname(pd, "3g");
    return pd;
}

static void _usb3g_stop_ppp(struct usb3gdongle *dongle)
{
    if (dongle->pppfd >= 0)
    {
        pppClose(dongle->pppfd);

        //wait ppp stack release resource
        while (ppp_state(dongle->pppfd))
        {
            osal_task_sleep(100);
        }

        dongle->pppfd = -1;
    }
}


#define AUTO_RE_DIAL 0  //auto re-dial after dialup failed

static void _usb3g_dial_task(int param1, int param2)
{
    int try_dial = 0;
    int try_ppp = 0;
    struct usb3gdongle *dongle = (struct usb3gdongle *) param1;
    usb3gdongle_port *modem = NULL;

    if (dongle == NULL || dongle->valid != 1)
    {
        return;
    }

    while (dongle->state != dongle_state_idle && dongle->valid == 1)
    {
        osal_task_sleep(200);
    }

    _usb3g_dongle_set_state(dongle, dongle_state_connectting);
    USB3G_DEBUG("start dial up!!!\n");
#if 0/*because only support one net interface in same time.*/
    wifi_close();
    ethernet_close();
#endif

    while (dongle->do_connect && dongle->valid == 1)
    {
        switch (dongle->state)
        {
            case dongle_state_connectting:

                if (dongle->pppfd >= 0)
                {
                    _usb3g_stop_ppp(dongle);
                }

                //maybe the state has been changed in _usb3g_dongle_link_status
                //so need ensure the state is dongle_state_connectting
                dongle->state = dongle_state_connectting;

                if (STRLEN(dongle->config.isp.apn) ==  0
                        || STRLEN(dongle->config.isp.phone_num) == 0)
                {
                    dongle->do_connect = 0;
                    break;
                }

                try_dial += 6;
                modem = _usb3g_dongle_dial(dongle);

                if (modem == NULL || !dongle->do_connect || dongle->valid != 1)
                {
                    if (try_dial >= 60)
                    {
                        _usb3g_dongle_set_state(dongle, dongle_state_connect_err);
                    }
                    else
                    {
                        osal_task_sleep(500);
                    }

                    break;
                }

                USB3G_DEBUG("start ppp ...\n");
                try_dial = 0;
                try_ppp += 6;
                dongle->pppfd = _usb3g_start_ppp(dongle, modem);

                if (dongle->pppfd >= 0) //check timeout
                {
                    try_ppp = 0;

                    while (dongle->do_connect && dongle->valid == 1 &&
                            dongle->state == dongle_state_connectting )
                    {
                        osal_task_sleep(500);
                    }
                }
                else if (dongle->pppfd == PPPERR_OPEN)
                {
                    USB3G_DEBUG("other dongles already connnected, "\
                                "only support %d dongle in same time!\n", NUM_PPP);
                    dongle->do_connect = 0;  //stop dailup
                    dongle->pppfd = -1;
                }
                else
                {
                    if (try_ppp >= 60)
                    {
                        _usb3g_dongle_set_state(dongle, dongle_state_connect_err);
                    }
                    else
                    {
                        osal_task_sleep(500);
                    }
                }

                break;
            case dongle_state_connect_err:

                if ((try_dial > 0 && try_dial-- > 0) /*dialup failed*/
                        || (try_ppp > 0 && try_ppp-- > 0) /*ppp connect failed*/
                   )
                {
#if AUTO_RE_DIAL
                    osal_task_sleep(1000);
#else
                    dongle->do_connect = 0; //stop dailup
#endif
                    break;
                }

                osal_task_sleep(1000);
                //connection lost or ppp connection failed, need re-connect
                _usb3g_dongle_set_state(dongle, dongle_state_connectting);
                break;
            case dongle_state_connected:
                osal_task_sleep(1000);
                break;
            default:
                dongle->do_connect = 0;  //stop dailup
                break;
        }
    }

    _usb3g_stop_ppp(dongle);
#if 0 /*because only support one net interface in same time.*/
    ethernet_open();
#endif

    if (dongle->state != dongle_state_none)
    {
        _usb3g_dongle_hangup(dongle);
        _usb3g_dongle_set_state(dongle, dongle_state_idle);
    }

    USB3G_DEBUG("stop dial connect!!!\n");
}



static int _usb3g_dongle_delete(struct ali_3g_modem_dev *dev)
{
    struct usb3gdongle *find = NULL;
    struct usb3gdongle *prev = NULL;
    int i = 0;

    if (dev == NULL)
    {
        return 0;
    }

    USB3G_DEBUG("%s: vendor id:%x, product id:%x\n", __FUNCTION__, dev->usb_vendor_id,
                dev->usb_product_id);
    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex , OSAL_WAIT_FOREVER_TIME);

    for (find = g_usb3g_dongle_mgr->dongle_list; find;
            find = (struct usb3gdongle *)find->next)
    {
        if ((UINT32)find->identify == dev->udev)
        {
            if (prev == NULL)
            {
                g_usb3g_dongle_mgr->dongle_list = find->next;
            }
            else
            {
                prev->next = find->next;
            }

            g_usb3g_dongle_mgr->dongle_num--;
            find->valid = 0;
            USB3G_DEBUG("find dongle, set invalid!!!\n");

            for (i = 0; i < MAX_DONGLE_PORT_CNT; i++)
            {
                find->ports[i].valid = 0;
            }

            break;
        }

        prev = find;
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);

    if (find)
    {
        int i = 0;

        for (i = 0; i < MAX_DONGLE_PORT_CNT; i++)
        {
            find->ports[i].private = NULL;
        }

        usb3g_stop_dail(find);

        if (g_usb3g_dongle_mgr->callback)
        {
            g_usb3g_dongle_mgr->callback(USB3G_DONGLE_HOTPLUG_OUT, find);
        }

        while (find->state != dongle_state_idle && find->state != dongle_state_none)
        {
            osal_task_sleep(100);
        }

        FREE(find);
    }

    USB3G_DEBUG("%s: dongle has been removed!!!\n", __FUNCTION__);
    return 1;
}

static int _usb3g_dongle_add_modem(struct ali_3g_modem_dev *dev)
{
    int newdongle = 0;
    struct usb3gdongle *dongle = NULL;
    usb3gdongle_port *port = NULL;
    USB3G_DEBUG("%s: vendor id:%x, product id:%x\n", __FUNCTION__, dev->usb_vendor_id,
                dev->usb_product_id);

    if (dev == NULL || g_usb3g_dongle_mgr == NULL || dev_get_by_name(dev->name) == NULL)
    {
        return 0;
    }

    dev->notify |= 0x02;
    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex , OSAL_WAIT_FOREVER_TIME);

    //find exist dongle
    for (dongle = g_usb3g_dongle_mgr->dongle_list; dongle;
            dongle = (struct usb3gdongle *)dongle->next)
    {
        if ((UINT32)dongle->identify == dev->udev)
        {
            break;
        }
    }

    // add a new dongle
    if (dongle == NULL)
    {
        dongle = MALLOC(sizeof(struct usb3gdongle));

        if (dongle == NULL) //malloc failed
        {
            osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
            USB3G_DEBUG("malloc failed!!!\n");
            goto _ERR;
        }

        MEMSET(dongle, 0, sizeof(struct usb3gdongle));
        dongle->pppfd = -1;
        dongle->next = g_usb3g_dongle_mgr->dongle_list;
        g_usb3g_dongle_mgr->dongle_list = dongle;
        g_usb3g_dongle_mgr->dongle_num++;
        dongle->valid = 1;
        dongle->state = dongle_state_idle;
        dongle->identify = (int)dev->udev;
        dongle->vendor_id = (short)dev->usb_vendor_id;
        dongle->device_id = (short)dev->usb_product_id;
        dongle->sio.sio_read = _usb_sio_read;
        dongle->sio.sio_write = _usb_sio_write;
        newdongle = 1;
    }

    port = _usb3g_get_free_port(dongle);

    if (port == NULL)
    {
        if (newdongle)
        {
            g_usb3g_dongle_mgr->dongle_list = dongle->next;
        }

        osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
        USB3G_DEBUG("Not support too dongle ports\n");
        goto _ERR;
    }

    port->valid = 1;
    //
    port->private = dev;
    port->hdl = dev->open(dev);

    if (port->hdl == 0)
    {
        USB3G_ERROR("open modem failed!!!!!\n");

        if (newdongle)
        {
            g_usb3g_dongle_mgr->dongle_list = dongle->next;
        }

        osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
        goto _ERR;
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
    //
    _modem_setup(port->hdl);
    //get dongle info
    usb3g_get_dongle_vendor_info(dongle);
    usb3g_get_dongle_product_info(dongle);
    usb3g_dongle_isready(dongle);
    usb3g_dongle_simready(dongle);
    usb3g_get_dongle_sim_imsi(dongle);
    USB3G_DEBUG("usb 3g dongle:\n");
    USB3G_DEBUG("   vendor id: %s\n", dongle->vendor_info);
    USB3G_DEBUG("   device id: %s\n", dongle->product_info);
    USB3G_DEBUG("   sim  card: %s\n", dongle->simready ? "ready" : "error");

    //auto match the ISP information
    if (dongle->config.isp.phone_num[0] == 0 && dongle->imsi[0])
    {
        struct isp_info *isp = usb3g_get_isp_service_by_imsi(dongle->imsi);

        if (isp)
        {
            MEMCPY(&(dongle->config.isp), isp, sizeof(struct isp_info));
            dongle->config.baud_rate = B115200;
        }
    }

    if (newdongle && g_usb3g_dongle_mgr->callback && dongle->valid == 1)
    {
        g_usb3g_dongle_mgr->callback(USB3G_DONGLE_HOTPLUG_IN, dongle);
    }

    return 1;
_ERR:

    if (newdongle)
    {
        FREE(dongle);
    }

    return 0;
}

static int _usb3g_dongle_hotplug_cb(unsigned long event, void *mdm_dev)
{
    switch (event)
    {
        case MODEM_DEVICE_PLUG_IN:
            _usb3g_dongle_add_modem((struct ali_3g_modem_dev *) mdm_dev);
            break;
        case MODEM_DEVICE_PLUG_OUT:
            _usb3g_dongle_delete((struct ali_3g_modem_dev *) mdm_dev);
            break;
    }

    return 1;
}

static void _usb3g_dongle_set_state(struct usb3gdongle *dongle, int newstate)
{
    if (dongle->state != newstate)
    {
        dongle->state = newstate;

        if (g_usb3g_dongle_mgr->callback)
        {
            g_usb3g_dongle_mgr->callback(USB3G_DONGLE_STATE_CHANGED, dongle);
        }
    }
}
#if 0 /*because only support one net interface in same time.*/
int ethernet_open( )
{
    struct net_device *net_dev = dev_get_by_type(NULL, HLD_DEV_TYPE_NET);

    if (netif_find_by_type(HLD_DEV_TYPE_WIFI)) //wifi enabled
    {
        return 0;
    }

    if (net_dev == NULL)
    {
        return -1;
    }

    USB3G_DEBUG("Open ethernet after Stop 3G!!\n");

    if (net_dev->flags & HLD_DEV_STATS_ATTACHED)
    {
        libc_printf("%s already attached!\n", net_dev->name);
        return 0;
    }

    if (sys_add_netif(IP_ADDR_ANY, IP_ADDR_ANY, IP_ADDR_ANY))
    {
        libc_printf("[ERR] %s: add netif fail...\n", __FUNCTION__);
        return -1;
    }

    IP_LOC_CFG ip_cfg;
    get_local_ip_cfg(&ip_cfg);
#ifdef _INVW_JUICE

    if (0)
#else
    if (ip_cfg.dhcp_on == 1)
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

int wifi_close()
{
#ifdef WIFI_SUPPORT

    if (netif_find_by_type(HLD_DEV_TYPE_WIFI)) //wifi enabled
    {
        USB3G_DEBUG("Stop Wifi before 3G dialup!!\n");
        api_wifi_enable_device_function(FALSE);

        if (dev_get_by_type(NULL, HLD_DEV_TYPE_NET)) //ethernet used
        {
            while (netif_find_by_type(HLD_DEV_TYPE_NET) == NULL) //waitting for wifi close complete!
            {
                osal_task_sleep(1000);
            }

            osal_task_sleep(1000);
        }
    }

#endif
}

int ethernet_close()
{
    struct netif *cur_netif = NULL;
    struct net_device *net_dev = dev_get_by_type(NULL, HLD_DEV_TYPE_NET);

    if (net_dev == NULL)
    {
        return -1;
    }

    USB3G_DEBUG("Close ethernet before 3G dialup!!\n");

    while ((cur_netif = netif_find_by_dev(net_dev)) == NULL)
    {
        osal_task_sleep(1000);
    }

    net_close(net_dev);
    net_dev->flags &= ~(HLD_DEV_STATS_ATTACHED);

    if (cur_netif)
    {
        IP_LOC_CFG ip_cfg;
        get_local_ip_cfg(&ip_cfg);
#ifdef _INVW_JUICE

        if (0)
#else
        //if(ip_cfg.dhcp_on == 1)
#endif
        {
            libnet_dhcp_off2(cur_netif);
        }

        netif_remove(cur_netif);
        netif_free(cur_netif);
    }

    return 0;
}
#endif

/******************************************************************************
 * Function: usb3g_dongle_isready
 * Description: -
 *    check the dongle if is ready
 ******************************************************************************/
int usb3g_dongle_isready(struct usb3gdongle *dongle)
{
    int i = 0;

    if (dongle == NULL || dongle->valid != 1)
    {
        return 0;
    }

    if (!dongle->ready)
    {
        for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->valid == 1; i++)
        {
            if (dongle->ports[i].valid)
            {
                if (modem_sendat(&(dongle->ports[i]), "AT\r", "OK", 0))
                {
                    dongle->ready = 1;
                    break;
                }
            }
        }
    }

    return dongle->ready;
}

/******************************************************************************
 * Function: usb3g_dongle_simready
 * Description: -
 *    check SIM card if is ready
 ******************************************************************************/
int usb3g_dongle_simready(struct usb3gdongle *dongle)
{
    int i = 0;

    if (dongle == NULL || dongle->valid != 1)
    {
        return 0;
    }

    if (!dongle->simready)
    {
        for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->valid == 1; i++)
        {
            if (dongle->ports[i].valid)
            {
                int retry = 0;
                char tmpbuf[512];

                while (retry++ < 20)
                {
                    MEMSET(tmpbuf, 0, sizeof(tmpbuf));

                    if (modem_sendat(&(dongle->ports[i]), "AT+CPIN?\r", tmpbuf, sizeof(tmpbuf)) )
                    {
                        if ((_strstr(tmpbuf, sizeof(tmpbuf), "+CPIN: READY")) || (_strstr(tmpbuf, sizeof(tmpbuf), "OK")))
                        {
                            dongle->simready = 1;
                            dongle->ready = 1;
                            break;
                        }
                        else if (_strstr(tmpbuf, sizeof(tmpbuf), "+CME ERROR: SIM busy"))
                        {
                            osal_task_sleep(300);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    return dongle->simready;
}
/******************************************************************************
 * Function: usb3g_get_dongle_vendor_info
 * Description: -
 *    get the vendor id of this dongle.
 *    if the dongle->vendor_id is emptry, this function will read vendor id from dongle.
 *    please check dongle->vendor_id when this function return, maybe can not read
 *    vendor id from dongle.
 ******************************************************************************/
char *usb3g_get_dongle_vendor_info(struct usb3gdongle *dongle)
{
    if (dongle == NULL || dongle->valid != 1)
    {
        return NULL;
    }

    if (dongle->vendor_info[0] == 0)
    {
        int i = 0;

        for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->valid == 1; i++)
        {
            if (dongle->ports[i].valid)
            {
                if (_modem_get_vendor_id(&dongle->ports[i], dongle->vendor_info, 32))
                {
                    break;
                }
            }
        }
    }

    return dongle->vendor_info;
}
/******************************************************************************
 * Function: usb3g_get_dongle_product_info
 * Description: -
 *    get the device id of this dongle.
 *    if the dongle->device_id is emptry, this function will read device id from dongle.
 *    please check dongle->device_id when this function return, maybe can not
 *    read device id from dongle.
 ******************************************************************************/
char *usb3g_get_dongle_product_info(struct usb3gdongle *dongle)
{
    if (dongle == NULL || dongle->valid != 1)
    {
        return NULL;
    }

    if (dongle->product_info[0] == 0)
    {
        int i = 0;

        for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->valid == 1; i++)
        {
            if (dongle->ports[i].valid)
            {
                if (_modem_get_device_id(&dongle->ports[i], dongle->product_info, 32))
                {
                    break;
                }
            }
        }
    }

    return dongle->product_info;
}
/******************************************************************************
 * Function: usb3g_get_dongle_signal_strength
 * Description: -
 *    get signal strength of this dongle
 ******************************************************************************/
int usb3g_get_dongle_signal_strength(struct usb3gdongle *dongle, int *quality)
{
    int i = 0;

    if (dongle == NULL || quality == NULL)
    {
        return 0;
    }

    for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->valid == 1; i++)
    {
        if (dongle->ports[i].valid)
        {
            if (_modem_get_signal_strength(&dongle->ports[i], quality, NULL))
            {
                return 1;
            }
        }
    }

    return 0;
}
/******************************************************************************
 * Function: usb3g_get_dongle_sim_imsi
 * Description: -
 *    get IMSI of SIM card in dongle.
 *    if the dongle->imsi is emptry, this function will read IMSI from dongle.
 *    please check dongle->imsi when this function returned, maybe can not
 *    read IMSI from dongle.
 * Return:
 *    return dongle->imsi.
 ******************************************************************************/
char *usb3g_get_dongle_sim_imsi(struct usb3gdongle *dongle)
{
    int i = 0;

    if (dongle == NULL || dongle->valid != 1)
    {
        return 0;
    }

    if (dongle->imsi[0] == 0)
    {
        for (i = 0; i < MAX_DONGLE_PORT_CNT && dongle->valid == 1; i++)
        {
            if (dongle->ports[i].valid)
            {
                if (_modem_get_imsi(&dongle->ports[i], dongle->imsi))
                {
                    break;
                }
            }
        }
    }

    return dongle->imsi;
}
/******************************************************************************
 * Function: usb3g_start_dail
 * Description: -
 *    start dial connect on the dongle.
 ******************************************************************************/
int usb3g_start_dail(struct usb3gdongle *dongle)
{
    int ret = 1;
    OSAL_T_CTSK ptask;

    if (dongle == NULL || dongle->valid != 1)
    {
        return 0;
    }

    if (!usb3g_dongle_isready(dongle) || !usb3g_dongle_simready(dongle))
    {
        USB3G_ERROR("dongle not ready, can not dialup!!!\n");
        return 0;
    }

    if (dongle->do_connect)
    {
        USB3G_ERROR("dongle already dialup!!\n");
        return 0;
    }

    //maybe dongle hasn't a config, so need try find config again.
    if (STRLEN(dongle->config.isp.phone_num) == 0)
    {
        dongle->imsi[0] = 0;
        usb3g_get_dongle_sim_imsi(dongle);
        struct isp_info *isp = usb3g_get_isp_service_by_imsi(dongle->imsi);

        if (isp)
        {
            MEMCPY(&(dongle->config.isp), isp, sizeof(struct isp_info));
            dongle->config.baud_rate = B115200;
        }
    }

    if (STRLEN(dongle->config.isp.phone_num) == 0)
    {
        USB3G_ERROR("dongle has not set phone number!!!\n");
        return 0;
    }

    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex , OSAL_WAIT_FOREVER_TIME);
    dongle->do_connect = 1;
    ptask.task = (FP)_usb3g_dial_task;
    ptask.itskpri = 20;
    ptask.quantum = 10;
    ptask.stksz = 0x2000;
    ptask.para1 = (int)dongle;
    ptask.para2 =  0;
    ptask.name[0] = 'u';
    ptask.name[1] = '3';
    ptask.name[2] = 'g';

    if (OSAL_INVALID_ID == osal_task_create(&ptask))
    {
        dongle->do_connect = 0 ;
        ret = 0;
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);

    if (ret == 0)
    {
        USB3G_ERROR("create task failed!!\n");
    }

    return ret;
}
/******************************************************************************
 * Function: usb3g_stop_dail
 * Description: -
 *    stop dail connect on the dongle.
 ******************************************************************************/
int usb3g_stop_dail(struct usb3gdongle *dongle)
{
    if (!dongle->do_connect)
    {
        USB3G_DEBUG("dongle has not dialup!!\n");
        return 1;
    }

    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex , OSAL_WAIT_FOREVER_TIME);
    dongle->do_connect = 0;
    _usb3g_dongle_set_state(dongle, dongle_state_disconnectting);
    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
    return 0;
}

/******************************************************************************
 * Function: usb3g_disconnect_all_dongle
 * Description: -
 *    disconnect all dongle
 ******************************************************************************/
int usb3g_disconnect_all_dongle()
{
    struct usb3gdongle *dongle = NULL;
    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex , OSAL_WAIT_FOREVER_TIME);

    for (dongle = g_usb3g_dongle_mgr->dongle_list; dongle;
            dongle = (struct usb3gdongle *)dongle->next)
    {
        if (dongle->do_connect)
        {
            dongle->do_connect = 0;
            _usb3g_dongle_set_state(dongle, dongle_state_disconnectting);

            while (dongle->state != dongle_state_idle && dongle->state != dongle_state_none)
            {
                osal_task_sleep(100);
            }
        }
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
    return 0;
}

/******************************************************************************
 * Function: usb3g_get_dongle_by_dev
 * Description: -
 *    find the dongle by modem dev,
 * Input:
 *    dev: ali_3g_modem_dev device.
 ******************************************************************************/
struct usb3gdongle *usb3g_get_dongle_by_dev(struct ali_3g_modem_dev *dev)
{
    struct usb3gdongle *dongle = NULL;

    if (dev == NULL || g_usb3g_dongle_mgr == NULL)
    {
        return NULL;
    }

    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex, OSAL_WAIT_FOREVER_TIME);

    for (dongle = g_usb3g_dongle_mgr->dongle_list; dongle;
            dongle = (struct usb3gdongle *)dongle->next)
    {
        if ((UINT32)dongle->identify == dev->udev)
        {
            break;
        }
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
    return NULL;
}
/******************************************************************************
 * Function: usb3g_get_dongle_by_index
 * Description: -
 *    get usb 3g dongle by index.
 *    index is start form 1, but can not Exceed the count of dongle.
 ******************************************************************************/
struct usb3gdongle *usb3g_get_dongle_by_index(int index)
{
    int idx = -1;
    struct usb3gdongle *dongle = NULL;

    if (g_usb3g_dongle_mgr == NULL)
    {
        return NULL;
    }

    osal_mutex_lock(g_usb3g_dongle_mgr->dongle_mutex, OSAL_WAIT_FOREVER_TIME);

    if (index >= 0 && index < g_usb3g_dongle_mgr->dongle_num)
    {
        for (dongle = g_usb3g_dongle_mgr->dongle_list; dongle;
                dongle = (struct usb3gdongle *)dongle->next)
        {
            idx++;

            if (idx == index)
            {
                break;
            }
        }
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->dongle_mutex);
    return dongle;
}
/******************************************************************************
 * Function: usb3g_get_dongleport_byid
 * Description: -
 *    get dongle port from dongle by index.
 *    index is start form 0, can not Exceed MAX_DONGLE_PORT_CNT.
 ******************************************************************************/
usb3gdongle_port *usb3g_get_dongleport_byid(struct usb3gdongle *dongle , int index)
{
    int i = 0;
    int idx = -1;
    usb3gdongle_port *port = NULL;

    if (dongle == NULL || index < 0 || index > MAX_DONGLE_PORT_CNT - 1)
    {
        return NULL;
    }

    for (i = 0; i < MAX_DONGLE_PORT_CNT; i++)
    {
        port = &dongle->ports[i];

        if (port->valid)
        {
            idx++;
        }

        if (idx == index)
        {
            return port;
        }
    }

    return NULL;
}


/******************************************************************************
 * Function: usb3g_reg_isp_info
 * Description: -
 *    register a ISP service information to dongle manager
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 *    apn      : access point name, can be NULL.
 *    phone_num: phone number for dialup.
 *    usr      : username, can be NULL.
 *    passwd   : password, can be NULL.
 ******************************************************************************/
int usb3g_reg_isp_info(const char isp_code[MCC_LEN + MNC_LEN], char *phone_num,
                       char *apn, char *usr, char *passwd)
{
    int len = 0 ;

    if (isp_code == NULL || phone_num == NULL || STRLEN(phone_num) == 0)
    {
        USB3G_DEBUG("invalid parameter!!!!\n");
        return 0;
    }

    struct isp_info *newisp = MALLOC(sizeof(struct isp_info));

    if (newisp == NULL)
    {
        USB3G_DEBUG("malloc failed!!!!\n");
        return 0;
    }

    len = STRLEN(isp_code);
    len = len > (MCC_LEN + MNC_LEN) ? (MCC_LEN + MNC_LEN) : len;
    MEMSET(newisp, 0, sizeof(struct isp_info));
    MEMCPY(newisp->isp_code, isp_code, len);
    MEMCPY(newisp->phone_num, phone_num,
           STRLEN(phone_num) > PHONE_LEN ? PHONE_LEN : STRLEN(phone_num));

    if (apn)
    {
        MEMCPY(newisp->apn, apn, STRLEN(apn) > APN_SIZE ? APN_SIZE : STRLEN(apn));
    }

    if (usr)
    {
        MEMCPY(newisp->user, usr, STRLEN(usr) > USR_LEN ? USR_LEN : STRLEN(usr));
    }

    if (passwd)
    {
        MEMCPY(newisp->passwd, passwd, STRLEN(passwd) > PWD_LEN ? PWD_LEN : STRLEN(passwd));
    }

    newisp->next = NULL;
    osal_mutex_lock(g_usb3g_dongle_mgr->isp_mutex, OSAL_WAIT_FOREVER_TIME);
    newisp->next = g_usb3g_dongle_mgr->isplist;
    g_usb3g_dongle_mgr->isplist = newisp;
    osal_mutex_unlock(g_usb3g_dongle_mgr->isp_mutex);
    return 1;
}
/******************************************************************************
 * Function: usb3g_unreg_isp_info
 * Description: -
 *    delete a ISP service information from dongle manager,
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 ******************************************************************************/
int usb3g_unreg_isp_info(char isp_code[MCC_LEN + MNC_LEN])
{
    int len = 0 ;
    struct isp_info *prev = NULL;
    struct isp_info *find = NULL;

    if (g_usb3g_dongle_mgr == NULL)
    {
        return 0;
    }

    len = STRLEN(isp_code);
    len = len > (MCC_LEN + MNC_LEN) ? (MCC_LEN + MNC_LEN) : len;
    osal_mutex_lock(g_usb3g_dongle_mgr->isp_mutex, OSAL_WAIT_FOREVER_TIME);

    for (find = g_usb3g_dongle_mgr->isplist; find != NULL;
            find = (struct isp_info *)find->next)
    {
        if ( MEMCMP(find->isp_code , isp_code , len) == 0)
        {
            if (prev == NULL)
            {
                g_usb3g_dongle_mgr->isplist = find->next;
            }
            else
            {
                prev->next = find->next;
            }

            break;
        }

        prev = find;
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->isp_mutex);

    if (find)
    {
        FREE(find);
    }
    return 1;
}
/******************************************************************************
 * Function: usb3g_get_isp_service
 * Description: -
 *    get ISP service information by ISP code
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 ******************************************************************************/
struct isp_info *usb3g_get_isp_service(char *isp_code)
{
    int len = 0 ;
    struct isp_info *find = NULL;

    if (g_usb3g_dongle_mgr == NULL)
    {
        return 0;
    }

    len = STRLEN(isp_code);
    len = len > (MCC_LEN + MNC_LEN) ? (MCC_LEN + MNC_LEN) : len;

    osal_mutex_lock(g_usb3g_dongle_mgr->isp_mutex, OSAL_WAIT_FOREVER_TIME);

    for (find = g_usb3g_dongle_mgr->isplist; find != NULL;
            find = (struct isp_info *)find->next)
    {
        if ( MEMCMP(find->isp_code , isp_code , len) == 0)
        {
            break;
        }
    }

    osal_mutex_unlock(g_usb3g_dongle_mgr->isp_mutex);
    return find;
}

/******************************************************************************
 * Function: usb3g_get_isp_service
 * Description: -
 *    get ISP service information by ISP code
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 ******************************************************************************/
struct isp_info *usb3g_get_isp_service_by_imsi(char *imsi)
{
    int len = MCC_LEN + MNC_LEN ;
    char isp_code[MCC_LEN + MNC_LEN];
    MEMSET(isp_code, 0, MCC_LEN + MNC_LEN);

    if (STRLEN(imsi) == 15) //European norm
    {
        len = MCC_LEN + MNC_LEN - 1;
    }

    MEMCPY(isp_code, imsi, len);
    return usb3g_get_isp_service(isp_code);
}

/******************************************************************************
 * Function: usb3g_dongle_count
 * Description: -
 *    get the count of usb3g dongles
 ******************************************************************************/
int usb3g_dongle_count()
{
    return g_usb3g_dongle_mgr ? g_usb3g_dongle_mgr->dongle_num : 0;
}
/******************************************************************************
 * Function: usb3g_attach
 * Description: -
 *    struct usb3gdongle module init, needs to be done only once after boot up
 * Input:
 *    callback: usb dongle event callback.
 ******************************************************************************/
void usb3g_attach(usb3g_dongle_cb callback)
{
    g_usb3g_dongle_mgr = MALLOC(sizeof(usb3gdongle_mgr));

    if (g_usb3g_dongle_mgr == NULL)
    {
        libc_printf("usb3g_attach : malloc failed!!!!\n");
        return;
    }

    MEMSET(g_usb3g_dongle_mgr, 0, sizeof(usb3gdongle_mgr));
    g_usb3g_dongle_mgr->dongle_mutex = osal_mutex_create();
    ASSERT(g_usb3g_dongle_mgr->dongle_mutex != INVALID_ID);
    g_usb3g_dongle_mgr->isp_mutex = osal_mutex_create();
    ASSERT(g_usb3g_dongle_mgr->isp_mutex != INVALID_ID);
    g_usb3g_dongle_mgr->callback = callback;
    USB3G_DEBUG("ALi 3G Driver Version: %s\n", ali_3g_modem_get_vesrion());
    ali_3g_modem_init();
    ali_3g_modem_dev_register_notify(_usb3g_dongle_hotplug_cb);
    pppInit();
}

/******************************************************************************
 * Function: usb3g_pcap
 * Description: -
 *    start/stop capture usb 3G data
 * Input:
 *    save_dir: the directory for saving captured data.
 *    start   : 1: start capture, 0: stop capture.
 * Returns:
 *    return file save path if start success, else return NULL.
 * -------------------------------------------------
 ******************************************************************************/
char *usb3g_pcap(char *save_dir, UINT8 start)
{
    struct netif *nf = netif_find("3g");

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
