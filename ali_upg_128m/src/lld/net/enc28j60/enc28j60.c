#include <sys_config.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/net/net.h>
#include <hal/hal_gpio.h>
#include <hld/sto/sto_dev.h>

#include <bus/spe/spe.h>
#include "enc28j60.h"

#define ETHERNET_DMA

#define NET_BASE_ADDR  0xAE000000//(priv->base_addr)
#define SF_BASE_ADDR    0xb8000000
UINT32 sflash_ctrl_reg    =  SF_BASE_ADDR;
#define SF_INS            (sflash_ctrl_reg + 0x98)
#define SF_FMT            (sflash_ctrl_reg + 0x99)
#define SF_DUM            (sflash_ctrl_reg + 0x9A)
#define SF_CFG            (sflash_ctrl_reg + 0x9B)

#define SF_HIT_DATA        0x01
#define SF_HIT_DUMM    0x02
#define SF_HIT_ADDR        0x04
#define SF_HIT_CODE        0x08
#define SF_CONT_RD        0x40
#define SF_CONT_WR        0x80
#define SF_LAST_WR        0x20
#define SF_LAST_RD        0x10

#define write_uint8(addr, val)        *((volatile UINT8 *)(addr)) = (val)
#define read_uint8(addr)            *((volatile UINT8 *)(addr))
#define write_uint16(addr, val)    *((volatile UINT16 *)(addr)) = (val)
#define read_uint16(addr)            *((volatile UINT16 *)(addr))
#define write_uint32(addr, val)    *((volatile UINT32 *)(addr)) = (val)
#define read_uint32(addr)            *((volatile UINT32 *)(addr))

#define HALF_DUPLEX
//#define FULL_DUPLEX
//#define LEDB_DUPLEX
// Pseudo Functions
#define LOW(a)                     ((a) & 0xFF)
#define HIGH(a)                 (((a)>>8) & 0xFF)

// ENC28J60 Opcodes (to be ORed with a 5 bit address)
#define    WCR (0x2<<5)            // Write Control Register command
#define     BFS (0x4<<5)            // Bit Field Set command
#define    BFC (0x5<<5)            // Bit Field Clear command
#define    RCR (0x0<<5)            // Read Control Register command
#define     RBM ((0x1<<5) | 0x1A)    // Read Buffer Memory command
#define    WBM ((0x3<<5) | 0x1A) // Write Buffer Memory command
#define    SR  ((0x7<<5) | 0x1F)    // System Reset command does not use an address.
                                //   It requires 0x1F, however.
UINT32 enc_drv_chip_id;
UINT32 enc_drv_chip_rev;

// A header appended at the start of all RX frames by the hardware
typedef struct  __attribute__((aligned(2), packed))
{
    UINT16        next_loc;
    RXSTATUS      status;
} ENC_PREAMBLE;

typedef struct __attribute__((__packed__)) _MAC_ADDR
{
    UINT8 v[6];
} MAC_ADDR;

typedef struct  __attribute__((aligned(2), packed))
{
    MAC_ADDR        dest_macaddr;
    MAC_ADDR        source_macaddr;
    UINT16          type;
} ETHER_HEADER;

static unsigned char default_mac[6]={0x01,0x08,0xDC,0x02,0x03,0x99};
// Prototypes of functions intended for MAC layer use only.
static void bank_sel(net_enc_private *priv, UINT16 reg);
static REG read_eth(net_enc_private *priv, UINT8 addr);
static REG read_mac(net_enc_private *priv, UINT8 addr);
UINT16 get_mac_array(net_enc_private *priv, UINT8 *val, UINT16 len);
PHYREG enc_read_phy(net_enc_private *priv, UINT8 reg);
static void write_reg(net_enc_private *priv, UINT8 addr, UINT8 data);
static void bfcreg(net_enc_private *priv, UINT8 addr, UINT8 data);
static void bfsreg(net_enc_private *priv, UINT8 addr, UINT8 data);
static void enc_reset(net_enc_private *priv);
void enc_hw_init(struct net_device *dev);

__MAYBE_UNUSED__ static UINT8 g_abort = 0;
__MAYBE_UNUSED__ static UINT32 g_abort_seq = 0;
//Continuous read/write can't be interrupted

/*
#define MACON1      (0x00|0x40|SPRD_MASK)
#define MACON2      (0x01|0x40|SPRD_MASK)
#define MACON3      (0x02|0x40|SPRD_MASK)
#define MACON4      (0x03|0x40|SPRD_MASK)
#define MABBIPG     (0x04|0x40|SPRD_MASK)
#define MAIPGL      (0x06|0x40|SPRD_MASK)
#define MAIPGH      (0x07|0x40|SPRD_MASK)
#define MACLCON1    (0x08|0x40|SPRD_MASK)
#define MACLCON2    (0x09|0x40|SPRD_MASK)
#define MAMXFLL     (0x0A|0x40|SPRD_MASK)
#define MAMXFLH     (0x0B|0x40|SPRD_MASK)
#define MAPHSUP     (0x0D|0x40|SPRD_MASK)
#define MICON       (0x11|0x40|SPRD_MASK)
#define MICMD       (0x12|0x40|SPRD_MASK)
#define MIREGADR    (0x14|0x40|SPRD_MASK)
#define MIWRL       (0x16|0x40|SPRD_MASK)
#define MIWRH       (0x17|0x40|SPRD_MASK)
#define MIRDL       (0x18|0x40|SPRD_MASK)
#define MIRDH       (0x19|0x40|SPRD_MASK)
#define MAADR1      (0x00|0x60|SPRD_MASK)
#define MAADR0      (0x01|0x60|SPRD_MASK)
#define MAADR3      (0x02|0x60|SPRD_MASK)
#define MAADR2      (0x03|0x60|SPRD_MASK)
#define MAADR5      (0x04|0x60|SPRD_MASK)
#define MAADR4      (0x05|0x60|SPRD_MASK)
#define MISTAT      (0x0A|0x60|SPRD_MASK)
*/

void enc_enter_op(net_enc_private *priv)
{
    UINT8 tmp;
    sto_mutex_enter();
    if(priv->cs_by_gpio)
        HAL_GPIO_BIT_SET(priv->cs_position, priv->cs_polar);
    priv->flash_bak = read_uint32(SF_INS);
    tmp = read_uint8(SF_CFG);
    tmp &= 0xf0;
    tmp |= priv->enc_clk_div;
    write_uint8(SF_CFG, tmp);
    //NET_PRINTF("Enter ENC OP, 98: %08x\n", *((volatile unsigned long *)0xb8000098));
}

void enc_leave_op(net_enc_private *priv)
{
    write_uint32(SF_INS, priv->flash_bak);
    if(priv->cs_by_gpio)
        HAL_GPIO_BIT_SET(priv->cs_position, !(priv->cs_polar));
    //NET_PRINTF("Leave ENC OP, 98: %08x\n", *((volatile unsigned long *)0xb8000098));
    sto_mutex_exit();
}
UINT16 enc_ntohs(UINT8 *data)
{
    return (data[0] << 8) | data[1];
}

UINT32 enc_ntohl(UINT8 *data)
{
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

UINT16 enc_htons(UINT16 n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

UINT32 enc_htonl(UINT32 n)
{
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) |
           ((n & 0xff000000) >> 24);
}


void dump_reg(net_enc_private *priv)
{
    unsigned short i, j;
    for(i = 0; i<2; i++)
    {
        bank_sel(priv, i<<8);
        NET_PRINTF("Bank %d: \n", i);
        for(j=0; j<0x20; j++)
        {
            NET_PRINTF("%02x ", j);
        }
        NET_PRINTF("\n");
        for(j=0; j<0x20; j++)
        {
            NET_PRINTF("%02x ", read_eth(priv, (UINT8)j).val);
        }
        NET_PRINTF("\n");
    }
    bank_sel(priv, 0<<8);
}

#if(DUMP_RCV_FRAME||DUMP_TSM_FRAME)
void dump_icmp_frame(UINT8 * buf)
{
#ifdef ENC_DBG_IP
    UINT8 type = buf[0];
    UINT8 code = buf[1];
    NET_PRINTF("        %s: \n", __FUNCTION__);
    NET_PRINTF("        type %d, code %d\n", type, code);
#endif
}

void dump_igmp_frame(UINT8 * buf)
{
#ifdef ENC_DBG_IP
    NET_PRINTF("        %s: \n", __FUNCTION__);
#endif
}

void dump_tcp_frame(UINT8 * buf, UINT8 in, UINT16 pl_len)
{
#ifdef ENC_DBG_TCP
    static UINT32 seq_in = 0;
    static UINT32 seq_out = 0;
    UINT16 src_port = enc_ntohs(buf);
    UINT16 dst_port = enc_ntohs(&buf[2]);
    UINT32 seq = enc_ntohl(&buf[4]);
    UINT32 seq_cfm = enc_ntohl(&buf[8]);
    TCP_FLAG tcp_flag;
    UINT8 hdr_len = ((buf[12]&0xf0)>>4)*4;
    UINT16 win_size = enc_ntohs(&buf[14]);
    UINT16 urg_point = enc_ntohs(&buf[19]);
    static UINT32 in_expt = 0;

    static UINT8 force_abort = 0xff;
    pl_len -= hdr_len;
    tcp_flag.v = buf[13];

    NET_PRINTF("        ");
    if(tcp_flag.bits.syn)
    {
        NET_PRINTF("S");
        if(in)
        {
            seq_in = seq;
            force_abort = 0;
        }
        else
        {
            seq_out = seq;
        }
    }

    if(in)
    {
        seq = seq - seq_in;
        seq_cfm = seq_cfm - seq_out;
    #if 0
        if(seq>30000)
        {
            if(0==force_abort)
            {
                force_abort = 1;
                g_abort = 1;
                g_abort_seq = seq;
                NET_PRINTF(" (Drop at %d) ", g_abort_seq);
            }
            else
            {
                if(g_abort_seq == seq)
                {
                    g_abort = 1;
                    NET_PRINTF(" (Continue Drop at %d) ", g_abort_seq);
                }
            }
        }
    #endif
    }
    else
    {
        seq = seq - seq_out;
        seq_cfm = seq_cfm - seq_in;
    }

    if(tcp_flag.bits.fin)
        NET_PRINTF("F");
    if(tcp_flag.bits.rst)
        NET_PRINTF("R");
    if(tcp_flag.bits.psh)
        NET_PRINTF("P");
    NET_PRINTF(":%d", seq);
    if(tcp_flag.bits.ack)
        NET_PRINTF(" A:%d", seq_cfm);
    if(tcp_flag.bits.urg)
        NET_PRINTF(" URG: %d", urg_point);
    NET_PRINTF(" W:%d ", win_size);
    if(in)
    {
        if((in_expt!= seq)&&(seq>in_expt))
        {
            NET_PRINTF("< %d LOST INPUT ! %d>", in_expt, seq);
        }
        in_expt =(seq+pl_len);
    }
    NET_PRINTF("len %d, next %d\n", pl_len, seq+pl_len);
    NET_PRINTF("\n");
    if(hdr_len>20)
    {
        INT16 opt_len = hdr_len -20;
        buf +=20;
        NET_PRINTF("        opt_len %d: ", opt_len);
        while(buf[0]&&(opt_len>0))
        {
            if(1==buf[0])
            {
                buf++;
                opt_len--;
            }
            else
            {
                UINT8 type = buf[0];
                NET_PRINTF("TYPE %d, LEN %d ", type, buf[1]);
                switch(type)
                {
                    case 2:
                        NET_PRINTF("MSS %d ", enc_ntohs(&buf[2]));
                        break;
                    case 3:
                        NET_PRINTF("WIN EXT %d ", buf[2]);
                        break;
                    case 8:
                        NET_PRINTF("TIME %d ECHO %d ", enc_ntohl(&buf[2]), enc_ntohl(&buf[6]));
                }
                opt_len = opt_len - buf[1];
                buf += buf[1];
            }
        }
        NET_PRINTF("\n");
    }
#endif
}

void dump_udp_frame(UINT8 * buf, UINT8 in)
{
#ifdef ENC_DBG_UDP
    UINT16 src_port = enc_ntohs(buf);
    UINT16 dst_port = enc_ntohs(&buf[2]);
    NET_PRINTF("        %s: \n", __FUNCTION__);
    NET_PRINTF("        src %d ===> dst %d\n", src_port, dst_port);
    /*
    switch(src_port)
    {
        case 69:
            NET_PRINTF("        TFTP application!\n");
            break;
        case 7:
            NET_PRINTF("        ECHO application!\n");
            break;
        case 9:
            NET_PRINTF("        DISCARD application!\n");
            break;
        case 13:
            NET_PRINTF("        DAYTIME application!\n");
            break;
        case 19:
            NET_PRINTF("        CHARGEN application!\n");
            break;
        case 37:
            NET_PRINTF("        TIME application!\n");
            break;
        default:
            NET_PRINTF("        un-known application!\n");
            break;
    }
    */
#endif
}

void dump_ip_frame(UINT8 * buf, UINT8 in)
{
    UINT32 i;
    UINT8 ip_v = (buf[0]&0xf0)>>4;
    UINT8 ip_hdr_len = (buf[0]&0xf)*4;
    UINT16 ip_pld_len = enc_ntohs(&buf[2]);
    UINT16 ip_id = enc_ntohs(&buf[4]);
    UINT16 ip_ofst = enc_ntohs(&buf[6])&0x1fff;
    UINT8 ip_flag = (buf[6]&0xe0)>>5;
    UINT8 ip_ttl = buf[8];
    UINT8 * src_addr = &buf[12];
    UINT8 * dst_addr = &buf[16];
    UINT8 protocol = buf[9];
#ifdef ENC_DBG_IP
    NET_PRINTF("    %s: v %d, hdr %d, pld %d, id %d, ofst %d, flag %d, ttl %d\n    ", __FUNCTION__,
        ip_v, ip_hdr_len, ip_pld_len, ip_id, ip_ofst, ip_flag, ip_ttl);
    for(i = 0; i<4; i++)
        NET_PRINTF("%d ", src_addr[i]);
    NET_PRINTF(" ==> ");
    for(i = 0; i<4; i++)
        NET_PRINTF("%d ", dst_addr[i]);
    NET_PRINTF(" : \n");
#endif
    buf = &buf[ip_hdr_len];
    switch(protocol)
    {
        case 1:
            dump_icmp_frame(buf);
            break;
        case 2:
            dump_igmp_frame(buf);
            break;
        case 6:
            dump_tcp_frame(buf, in, (ip_pld_len- ip_hdr_len));
            break;
        case 17:
            dump_udp_frame(buf, in);
            break;
        default:
            NET_PRINTF("        UN-known protocol!\n");
            break;
    }
}

void dump_arp_frame(UINT8 * buf)
{
#ifdef ENC_DBG_ARP
    UINT16 arp_hw = enc_ntohs(buf);
    UINT16 arp_ptl = enc_ntohs(&buf[2]);
    UINT8 hw_addr_len = buf[4];
    UINT8 ptl_addr_len = buf[5];
    UINT16 arp_op = enc_ntohs(&buf[6]);
    UINT8 i;

    NET_PRINTF("    %s: hw %s, ptl %s, hw addr len %d, ptl addr len %d\n", __FUNCTION__, arp_hw==1?"eth":"un-k",
        arp_ptl==0x800?"ip":"un-k", hw_addr_len, ptl_addr_len);
    NET_PRINTF("    OP:");
    if(arp_op==1)
        NET_PRINTF("ARP REQUEST:\n    ");
    else if(arp_op==2)
        NET_PRINTF("ARP ANSWER:\n    ");
    else if(arp_op==3)
        NET_PRINTF("RARP REQUEST:  WHY?\n    ");
    else if(arp_op==4)
        NET_PRINTF("RARP ANSWER:  WHY?\n    ");
    else
        NET_PRINTF("UN-KNOWN:\n    ");
    buf += 8;
    NET_PRINTF("MAC: ");
    for(i=0; i<6; i++)
        NET_PRINTF("%02x ", buf[i]);
    buf += 6;
    NET_PRINTF("IP: ");
    for(i=0; i<4; i++)
        NET_PRINTF("%d ", buf[i]);
    NET_PRINTF(" ==> ");
    buf += 4;
    NET_PRINTF("MAC: ");
    for(i=0; i<6; i++)
        NET_PRINTF("%02x ", buf[i]);
    buf += 6;
    NET_PRINTF("IP: ");
    for(i=0; i<4; i++)
        NET_PRINTF("%d ", buf[i]);

    NET_PRINTF("\n");

#endif
}

void dump_rarp_frame(UINT8 * buf)
{
#ifdef ENC_DBG_RARP
    UINT16 rarp_hw = enc_ntohs(buf);
    UINT16 rarp_ptl = enc_ntohs(&buf[2]);
    UINT8 hw_addr_len = buf[4];
    UINT8 ptl_addr_len = buf[5];
    UINT16 rarp_op = enc_ntohs(&buf[6]);
    UINT8 i;
    NET_PRINTF("    %s: hw %s, ptl %s, hw addr len %d, ptl addr len %d\n", __FUNCTION__, rarp_hw==1?"eth":"un-k",
        rarp_ptl==0x800?"ip":"un-k", hw_addr_len, ptl_addr_len);
    NET_PRINTF("    OP:");
    if(rarp_op==1)
        NET_PRINTF("ARP REQUEST:   WHY?\n    ");
    else if(rarp_op==2)
        NET_PRINTF("ARP ANSWER:   WHY?\n    ");
    else if(rarp_op==3)
        NET_PRINTF("RARP REQUEST:\n    ");
    else if(rarp_op==4)
        NET_PRINTF("RARP ANSWER:\n    ");
    else
        NET_PRINTF("UN-KNOWN:\n    ");
    buf += 8;
    NET_PRINTF("MAC: ");
    for(i=0; i<6; i++)
        NET_PRINTF("%02x ", buf[i]);
    buf += 6;
    NET_PRINTF("IP: ");
    for(i=0; i<4; i++)
        NET_PRINTF("%d ", buf[i]);
    NET_PRINTF(" ==> ");
    buf += 4;
    NET_PRINTF("MAC: ");
    for(i=0; i<6; i++)
        NET_PRINTF("%02x ", buf[i]);
    buf += 6;
    NET_PRINTF("IP: ");
    for(i=0; i<4; i++)
        NET_PRINTF("%d ", buf[i]);
    NET_PRINTF("\n");
#endif
}
void dump_mac_frame(UINT8 * buf, UINT8 in)
{
    UINT32 i;
    ETHER_HEADER mac_hdr;
    MEMCPY(&mac_hdr, buf, sizeof(ETHER_HEADER));
#ifdef ENC_DBG_MAC
    NET_PRINTF("%s ", __FUNCTION__);
    for(i = 0; i<6; i++)
        NET_PRINTF("%02x ", mac_hdr.source_macaddr.v[i]);
    NET_PRINTF(" => ");
    for(i = 0; i<6; i++)
        NET_PRINTF("%02x ", mac_hdr.dest_macaddr.v[i]);
    NET_PRINTF(" : \n");
#endif
    buf += 14;
    switch(enc_ntohs((UINT8 *)(&(mac_hdr.type))))
    {
        case 0x0800:
            dump_ip_frame(buf, in);
            break;
        case 0x0806:
            dump_arp_frame(buf);
            break;
        case 0x8035:
            dump_rarp_frame(buf);
            break;
        default:
            NET_PRINTF("RFC 1042 Mac Frame, remain len %d\n", enc_ntohs((UINT8 *)(&(mac_hdr.type))));
            buf += 6;
            switch(enc_ntohs(buf))
            {
                case 0x0800:
                    dump_ip_frame(&buf[2], in);
                    break;
                case 0x0806:
                    dump_arp_frame(&buf[2]);
                    break;
                case 0x8035:
                    dump_rarp_frame(&buf[2]);
                    break;
                default:
                    NET_PRINTF("    Un-know type!\n");
                    break;
            }
            break;
    }
}
#endif

//workround for MA/MI register write for M3602 board
static void write_ma_mi_reg(net_enc_private *priv, UINT8 addr, UINT8 data)
{
    unsigned long bak = 0;
    if(priv->cs_by_gpio)
        HAL_GPIO_BIT_SET(priv->cs2_position, priv->cs2_polar);
    else
    {
        if(ALI_S3602==enc_drv_chip_id)
        {
            bak = (*(volatile unsigned long *)(0xb8000088))&0x60;
            *(volatile unsigned long *)(0xb8000088) |= 0x60;
            HAL_GPIO_BIT_SET(51, 0);
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5<=enc_drv_chip_rev)
        {
            *(volatile unsigned long *)(0xb8000010) |= 0x40;
            HAL_GPIO_BIT_SET(7, 0);
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5>enc_drv_chip_rev)
        {
            *(volatile unsigned long *)(0xb8000010) |= 0x10;
            HAL_GPIO_BIT_SET(64, 0);
        }
        else
        {
            NET_PRINTF("Fix Me: %s %d: I don't know the pin mux setting!\n", __FILE__, __LINE__);
            while(1);
        }
    }
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA );
    write_uint8(SF_INS, WCR | addr);
    ((volatile UINT8 *)NET_BASE_ADDR)[0] = data;
    osal_delay(2);
    if(priv->cs_by_gpio)
        HAL_GPIO_BIT_SET(priv->cs2_position, !(priv->cs2_polar));
    else
    {
        if(ALI_S3602==enc_drv_chip_id)
        {
            HAL_GPIO_BIT_SET(51, 1);
            *(volatile unsigned long *)(0xb8000088) &= ~0x60;
            *(volatile unsigned long *)(0xb8000088) |= bak;
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5<=enc_drv_chip_rev)
        {
            *(volatile unsigned long *)(0xb8000010) &= ~0x40;
            HAL_GPIO_BIT_SET(7, 1);
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5>enc_drv_chip_rev)
        {
            *(volatile unsigned long *)(0xb8000010) &= ~0x10;
            HAL_GPIO_BIT_SET(64, 1);
        }
        else
        {
            NET_PRINTF("Fix Me: %s %d: I don't know the pin mux setting!\n", __FILE__, __LINE__);
            while(1);
        }
    }
}

static BOOL enc_is_linked(net_enc_private *priv)
{
    return enc_read_phy(priv, PHSTAT1).phstat1bits.LLSTAT;
}

BOOL enc_is_tx_ready(net_enc_private *priv)
{
    return !read_eth(priv, ECON1).econ1bits.TXRTS;
}

void enc_discard_rx(net_enc_private *priv)
{
    UINT16 new_next_loc;

    // Make sure the current packet was not already discarded

    // Decrement the next packet pointer before writing it into
    // the ERXRDPT registers.  This is a silicon errata workaround.
    // RX buffer wrapping must be taken into account if the
    // NextPacketLocation is precisely RXSTART.

    if(priv->next_loc==RXSTART)
        new_next_loc = RXSTOP;
    else
        new_next_loc = priv->next_loc - 1;
    //NET_PRINTF("{%d}", new_next_loc);
    // Decrement the RX packet counter register, EPKTCNT
    bfsreg(priv, ECON2, ECON2_PKTDEC);

    // Move the receive read pointer to unwrite-protect the memory used by the
    // last packet.  The writing order is important: set the low byte first,
    // high byte last.
    write_reg(priv, ERXRDPTL, ((UINT8 *)&new_next_loc)[0]);
    write_reg(priv, ERXRDPTH, ((UINT8 *)&new_next_loc)[1]);
         //NET_PRINTF("%s: %d\n", __FUNCTION__, new_next_loc);
}

void enc_tx_status(net_enc_private *priv, TXSTATUS * tx_status, UINT8 dump)
{
    TXSTATUS txstatus;
    UINT16 read_ptr_save;
    UINT16 txend;
    __MAYBE_UNUSED__ UINT16 txstart;
    // Save the current read pointer (controlled by application)
    ((UINT8 *)&read_ptr_save)[0] = read_eth(priv, ERDPTL).val;
    ((UINT8 *)&read_ptr_save)[1] = read_eth(priv, ERDPTH).val;

    // Get the location of the transmit status vector
    ((UINT8 *)&txend)[0] = read_eth(priv, ETXNDL).val;
    ((UINT8 *)&txend)[1] = read_eth(priv, ETXNDH).val;
    txend++;

    // Read the transmit status vector
    write_reg(priv, ERDPTL, ((UINT8 *)&txend)[0]);
    write_reg(priv, ERDPTH, ((UINT8 *)&txend)[1]);
    get_mac_array(priv, (UINT8*)&txstatus, sizeof(TXSTATUS));
    // Restore the current read pointer
    write_reg(priv, ERDPTL, ((UINT8 *)&read_ptr_save)[0]);
    write_reg(priv, ERDPTH, ((UINT8 *)&read_ptr_save)[1]);
    MEMCPY(tx_status, &txstatus, sizeof(TXSTATUS));
    if(dump)
    {
    #ifdef ENC_TX_STATUS
        NET_PRINTF("\n%s\n", __FUNCTION__);
        NET_PRINTF("ByteCount: %d\n", txstatus.bits.byte_count);
        NET_PRINTF("CollisionCount: %d\n", txstatus.bits.collision_count);
        NET_PRINTF("CRCError: %d\n", txstatus.bits.crcerror);
        NET_PRINTF("LengthCheckError: %d\n", txstatus.bits.length_check_error);
        NET_PRINTF("LengthOutOfRange: %d\n", txstatus.bits.length_out_of_range);
        NET_PRINTF("Done: %d\n", txstatus.bits.done);
        NET_PRINTF("Multicast: %d\n", txstatus.bits.multicast);
        NET_PRINTF("Broadcast: %d\n", txstatus.bits.broadcast);
        NET_PRINTF("PacketDefer: %d\n", txstatus.bits.packet_defer);
        NET_PRINTF("ExcessiveDefer: %d\n", txstatus.bits.excessive_defer);
        NET_PRINTF("MaximumCollisions: %d\n", txstatus.bits.maximum_collisions);
        NET_PRINTF("LateCollision: %d\n", txstatus.bits.late_collision);
        NET_PRINTF("Giant: %d\n", txstatus.bits.giant);
        NET_PRINTF("Underrun: %d\n", txstatus.bits.underrun);
        NET_PRINTF("BytesTransmittedOnWire: %d\n", txstatus.bits.bytes_transmitted_on_wire);
        NET_PRINTF("ControlFrame: %d\n", txstatus.bits.control_frame);
        NET_PRINTF("PAUSEControlFrame: %d\n", txstatus.bits.pausecontrol_frame);
        NET_PRINTF("BackpressureApplied: %d\n", txstatus.bits.backpressure_applied);
        NET_PRINTF("VLANTaggedFrame: %d\n", txstatus.bits.vlantagged_frame);
        NET_PRINTF("Zeros: %d\n", txstatus.bits.zeros);
    #endif
    //Compare TX buffer data with raw data
    #ifdef ENC_DEBUG_BUF
        // Save the current read pointer (controlled by application)
        ((UINT8 *)&read_ptr_save)[0] = read_eth(priv, ERDPTL).val;
        ((UINT8 *)&read_ptr_save)[1] = read_eth(priv, ERDPTH).val;

        // Get the location of the tx buffer start
        ((UINT8 *)&txstart)[0] = read_eth(priv, ETXSTL).val;
        ((UINT8 *)&txstart)[1] = read_eth(priv, ETXSTH).val;
        txstart++;//skip control byte

        // Set read point to mac frame start
        write_reg(priv, ERDPTL, ((UINT8 *)&txstart)[0]);
        write_reg(priv, ERDPTH, ((UINT8 *)&txstart)[1]);

        // Get the location of the tx buffer end
        ((UINT8 *)&txend)[0] = read_eth(priv, ETXNDL).val;
        ((UINT8 *)&txend)[1] = read_eth(priv, ETXNDH).val;
        txend++;
        priv->tx_len = txend - txstart;
        if(priv->tx_len>ENC_TX_BUF_LEN)
        {
            NET_PRINTF("tx len read back error %d\n", priv->tx_len);
            priv->tx_len = ENC_TX_BUF_LEN;
        }
        get_mac_array(priv, priv->tx_buf, priv->tx_len);
        // Restore the current read pointer
        write_reg(priv, ERDPTL, ((UINT8 *)&read_ptr_save)[0]);
        write_reg(priv, ERDPTH, ((UINT8 *)&read_ptr_save)[1]);
    #endif
    }

}

void enc_flush_mac(struct net_device *dev)
{
    net_enc_private *priv = (net_enc_private *)dev->priv;
    // Reset transmit logic if a TX Error has previously occured
    // This is a silicon errata workaround
    bfsreg(priv, ECON1, ECON1_TXRST);
    osal_delay(5);
    bfcreg(priv, ECON1, ECON1_TXRST);
    bfcreg(priv, EIR, EIR_TXERIF | EIR_TXIF);

    // Start the transmission
    bfsreg(priv, ECON1, ECON1_TXRTS);

    // Revision B5 and B7 silicon errata workaround
    if(priv->enc_revid == 0x05u || priv->enc_revid == 0x06u)
    {
        UINT16 counter = 0x0000;
        while(!(read_eth(priv, EIR).val & (EIR_TXERIF | EIR_TXIF)) && (++counter < 1000));
                  if(read_eth(priv, EIR).eirbits.TXERIF || (counter >= 1000))
        {
            TXSTATUS txstatus;
            UINT8 i;
            NET_PRINTF("TX Wait: TXERIF %d, counter %d. \n", read_eth(priv, EIR).eirbits.TXERIF, counter);

        #if 0
            enc_hw_init(dev);
        #else
            // Cancel the previous transmission if it has become stuck set
            bfcreg(priv, ECON1, ECON1_TXRTS);
            enc_tx_status(priv, &txstatus, 0);

            // Implement retransmission if a late collision occured (this can
            // happen on B5 when certain link pulses arrive at the same time
            // as the transmission)

            for(i = 0; i < 16u; i++)
            {
                NET_PRINTF("Further : TXERIF %d, LateCol %d\n", read_eth(priv, EIR).eirbits.TXERIF, txstatus.bits.late_collision);

                if(read_eth(priv, EIR).eirbits.TXERIF || txstatus.bits.late_collision)
                {
                    // Reset the TX logic
                    bfsreg(priv, ECON1, ECON1_TXRST);
                    bfcreg(priv, ECON1, ECON1_TXRST);
                    bfcreg(priv, EIR, EIR_TXERIF | EIR_TXIF);

                    // Transmit the packet again
                    bfsreg(priv, ECON1, ECON1_TXRTS);
                    while(!(read_eth(priv, EIR).val & (EIR_TXERIF | EIR_TXIF)));
                    // Cancel the previous transmission if it has become stuck set
                    bfcreg(priv, ECON1, ECON1_TXRTS);
                    enc_tx_status(priv, &txstatus, 0);
                }
                else
                {
                    NET_PRINTF("TX Wait OK: %d. \n", i);
                    break;
                }
            }
            if(i==16)NET_PRINTF("TX tmo: %d. \n", i);
        #endif
        }
    }
    else
    {
        UINT16 counter = 0x0000;
        while(!(read_eth(priv, EIR).val & (EIR_TXIF)) && (++counter < 1000));
    }

#if(defined(ENC_TX_STATUS)||defined(ENC_DEBUG_BUF))
    TXSTATUS tx_state;
    enc_tx_status(priv, &tx_state, 1);
#endif
}

UINT16 get_mac_array(net_enc_private *priv, UINT8 *val, UINT16 len)
{
    INT16 i;
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);//| SF_CONT_RD);
    write_uint8(SF_INS, RBM);
    i = len;// - 1;
    while(i--)
    {
        *val++ = ((volatile UINT8 *)NET_BASE_ADDR)[0];
    }
    /*if(len >= 1)
    {
        write_uint8(SF_DUM, read_uint8(SF_DUM)|SF_LAST_RD);
        *val++ = ((volatile UINT8 *)NET_BASE_ADDR)[0];
    }
    write_uint8(SF_DUM, read_uint8(SF_DUM)&(~SF_LAST_RD));*/
    return 0;

}//end get_macArray

void put_mac(net_enc_private *priv, UINT8 val)
{
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA );
    write_uint8(SF_INS, WBM);
    ((volatile UINT8 *)NET_BASE_ADDR)[0] = val;
}

void put_mac_array(net_enc_private *priv, UINT8 *val, UINT16 len)
{
    INT16 i;
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);// | SF_CONT_WR);
    write_uint8(SF_INS, WBM);
    i = len;// - 1;
    while(i--)
    {
        ((volatile UINT8 *)NET_BASE_ADDR)[0] = *val++;
    }
    /*if(len >= 1)
    {
        write_uint8(SF_DUM, read_uint8(SF_DUM)|SF_LAST_WR);
        ((volatile UINT8 *)NET_BASE_ADDR)[0] = *val++;
    }
    write_uint8(SF_DUM, read_uint8(SF_DUM)&(~SF_LAST_WR));*/
}//end put_macArray

static void enc_reset(net_enc_private *priv)
{
    // Make sure that the device is not in power save
    bfcreg(priv, ECON2, ECON2_PWRSV);

    // delay 1 ms
    osal_delay(1000);

    //Execute the System Reset command
    write_uint8(SF_FMT, SF_HIT_CODE);
    write_uint8(SF_INS, SR);
    ((volatile UINT8 *)NET_BASE_ADDR)[0] = 0;

    // delay 1 ms again after reset
    osal_delay(1000);
}


static REG read_eth(net_enc_private *priv, UINT8 addr)
{
    REG r;
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
    write_uint8(SF_INS, RCR | addr);
    r.val = ((volatile UINT8 *)NET_BASE_ADDR)[0];
    return r;
}

static REG read_mac(net_enc_private *priv, UINT8 addr)
{
    REG r;
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA | SF_HIT_DUMM);
    write_uint8(SF_INS, RCR | addr);
    r.val = ((volatile UINT8 *)NET_BASE_ADDR)[0];
    return r;
}


PHYREG enc_read_phy(net_enc_private *priv, UINT8 reg)
{
    PHYREG result;
    UINT32 tmo = 1000;

    // Set the right address and start the register read operation
    bank_sel(priv, 2<<8);
    write_ma_mi_reg(priv, (UINT8)MIREGADR, reg);
    write_ma_mi_reg(priv, (UINT8)MICMD, MICMD_MIIRD);

    // Loop to wait until the PHY register has been read through the MII
    // This requires 10.24us
    bank_sel(priv, 3<<8);
    while(read_mac(priv, (UINT8)MISTAT).mistatbits.BUSY)
    {
        osal_delay(1);
        tmo --;
        if(0==tmo)
        {
            NET_PRINTF("Wait read phy ready TMO!\n");
            ((UINT8 *)&result.val)[0] = ((UINT8 *)&result.val)[1] = 0;
            bank_sel(priv, 0<<8);
            return result;
        }
    }

    // Stop reading
    bank_sel(priv, 2<<8);
    write_ma_mi_reg(priv, (UINT8)MICMD, 0x00);

    // Obtain results and return
    ((UINT8 *)&result.val)[0] = read_mac(priv, (UINT8)MIRDL).val;
    ((UINT8 *)&result.val)[1] = read_mac(priv, (UINT8)MIRDH).val;

    bank_sel(priv, 0<<8);    // Return to Bank 0
    return result;
}


static void write_reg(net_enc_private *priv, UINT8 addr, UINT8 data)
{
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA );
    write_uint8(SF_INS, WCR | addr);

    ((volatile UINT8 *)NET_BASE_ADDR)[0] = data;
}

static void bfcreg(net_enc_private *priv, UINT8 addr, UINT8 data)
{
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA );
    write_uint8(SF_INS, BFC | addr);
    ((volatile UINT8 *)NET_BASE_ADDR)[0] = data;
}

static void bfsreg(net_enc_private *priv, UINT8 addr, UINT8 data)
{
    write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA );
    write_uint8(SF_INS, BFS | addr);
    ((volatile UINT8 *)NET_BASE_ADDR)[0] = data;
}

void enc_write_phy(net_enc_private *priv, UINT8 reg, UINT16 data)
{
    UINT32 tmo = 1000;
    // Write the register address
    bank_sel(priv, 2<<8);
    write_ma_mi_reg(priv, (UINT8)MIREGADR, reg);

    // Write the data low byte first
    write_ma_mi_reg(priv, (UINT8)MIWRL, ((UINT8 *)&data)[0]);
    write_ma_mi_reg(priv, (UINT8)MIWRH, ((UINT8 *)&data)[1]);

    // Wait until the PHY register has been written
    bank_sel(priv, 3<<8);
    while(read_mac(priv, (UINT8)MISTAT).mistatbits.BUSY)
    {
        osal_delay(1);
        tmo --;
        if(0==tmo)
        {
            NET_PRINTF("Wait write phy ready TMO!\n");
            break;
        }
    }

    bank_sel(priv, 0<<8);    // Return to Bank 0
}


static void bank_sel(net_enc_private *priv, UINT16 reg)
{
   // osal_delay(0xff00);
    bfcreg(priv, ECON1, ECON1_BSEL1 | ECON1_BSEL0);
   // osal_delay(0xff00);
    bfsreg(priv, ECON1, ((UINT8 *)&reg)[1]);
   // osal_delay(0xff00);
}

/*
void MACPowerDown(net_enc_private *priv)
{
    // Disable packet reception
    BFCReg(priv, ECON1, ECON1_RXEN);

    // Make sure any last packet is completed
    while(read_eth(priv, ESTAT).ESTATbits.RXBUSY);

    // Wait for transmitting to finish
    while(read_eth(priv, ECON1).ECON1bits.TXRTS);

    // Enter sleep mode
    BFSReg(priv, ECON2, ECON2_PWRSV);
}

void MACPowerUp(net_enc_private *priv)
{
    // Leave power down mode
    BFCReg(priv, ECON2, ECON2_PWRSV);

    // Wait for the 300us Oscillator Startup Timer (OST) to time out.
    while(!read_eth(priv, ESTAT).ESTATbits.CLKRDY);

    // Enable packet reception
    BFSReg(priv, ECON1, ECON1_RXEN);
}

void SetCLKOUT(net_enc_private *priv, UINT8 NewConfig)
{
    BankSel(priv, ECOCON);
    WriteReg(priv, (UINT8)ECOCON, NewConfig);
    BankSel(priv, ERDPTL);
}

UINT8 GetCLKOUT(net_enc_private *priv)
{
    UINT8 i;
    BankSel(priv, ECOCON);
    i = read_eth(priv, (UINT8)ECOCON).Val;
    BankSel(priv, ERDPTL);
    return i;
}
*/
void net_enc_task(UINT32 param1, UINT32 param2)
{
    struct net_device *dev = (struct net_device *)param1;
    net_enc_private *priv = (net_enc_private *)dev->priv;
    ENC_PREAMBLE header;
    UINT8 pkt_num = 0;
    UINT16 pkt_len=0; //data_len=0;
    //UINT16 ptr=0;
    //UINT32 cur_rx;
    OSAL_ER  ret;
    //UINT32 i;
    UINT8 abort = 0;
    MEMSET(&header, 0x0, sizeof (header));
    while(1)
    {
        if(0==pkt_num)
        {
            if(priv->int_by_gpio)
            {
                osal_interrupt_disable();
                if(!priv->int_enable)
                {
                    //NET_PRINTF("GPIO INT re-open!\n");
                    HAL_GPIO_INT_SET(priv->int_position, 1);
                    priv->int_enable = 1;
                }
                osal_interrupt_enable();
            }
            ret = osal_mutex_lock(priv->tx_rx_mutex, priv->tsk_sleep);

            if(OSAL_E_OK==ret)
            {
                //NET_PRINTF("<\n");
                priv->tsk_sleep = 10;
            }
            else
            {
                if(priv->int_by_gpio&&priv->tsk_sleep<100)
                {
                    priv->tsk_sleep += 10;
                    //NET_PRINTF("> %d\n", priv->tsk_sleep);
                }
            }

        }
        enc_enter_op(priv);
        bank_sel(priv, 1<<8);
        pkt_num =  (UINT8)(read_eth(priv, (UINT8)EPKTCNT).val);

        if(!pkt_num)
        {
            bank_sel(priv, 0<<8);
            enc_leave_op(priv);
            continue;
         }
        //Flow control
        /*
         BankSel(priv, 3<<8);
        if(pkt_num>RXTHLD)
            WriteReg(priv, (UINT8)EFLOCON, EFLOCON_FCEN1);
        else
            WriteReg(priv, (UINT8)EFLOCON, 0);
            */
        bank_sel(priv, 0<<8);
        priv->tsk_sleep = 10;
        NET_SIM_PRINTF("<:");
            NET_PRINTF("\nRCV PKT: %d\n", pkt_num);

            //Set the SPI read pointer to the next unprocessed packet
            write_reg(priv, ERDPTL, ((UINT8 *)&priv->next_loc)[0]);
            write_reg(priv, ERDPTH, ((UINT8 *)&priv->next_loc)[1]);

            // Obtain the MAC header from the Ethernet buffer
            // ethernet header is copied into lwip pbuf
            get_mac_array(priv, (UINT8 *)&header, 6);
            pkt_len = (header.next_loc > priv->next_loc)?(header.next_loc - priv->next_loc):((UINT16)RXSIZE - priv->next_loc + header.next_loc);

              //NET_PRINTF("cur rd %d, next rd %d, len %d\n", priv->next_loc, header.next_loc, pkt_len);

        pkt_len -= 6; //Remove RX
        abort  = 0;
        if(pkt_len>ENC_RX_BUF_LEN)
            pkt_len = ENC_RX_BUF_LEN;
        if(priv->dma_cpy)
        {
            UINT32 flag = SPE_BYTE_TRANS|SPE_DMA_READ|SPE_DMA_SYNC|SPE_ADDR_NOINC;
            write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA/*|SF_CONT_RD*/);
            write_uint8(SF_INS, RBM);
            priv->dma_cpy(priv->spe_id, priv->map_addr+0, (UINT32)priv->rx_buf, pkt_len, flag);
            osal_cache_invalidate((void *)priv->rx_buf, pkt_len);
        }
        else
            get_mac_array(priv, priv->rx_buf, pkt_len);

        pkt_len -= 4;//Remove CRC
        if(pkt_len>MAX_MAC_FRM)
        {
            abort = 1;
            NET_PRINTF("len BIG THAN mac mtu : %d\n", pkt_len);
        }
        // Save the next packet location
            priv->next_loc = header.next_loc;
                enc_discard_rx(priv);
        pkt_num --;
    #if(DUMP_RCV_FRAME)
        NET_PRINTF("\n<<%d", pkt_len);
        g_abort = 0;
        dump_mac_frame(priv->rx_buf, 1);
        if(g_abort)
            abort = 1;
    #endif
        enc_leave_op(priv);

        if ((dev->callback)&&(!abort))
        {
            struct net_pkt_t cur_rx_pkt;
            cur_rx_pkt.buffer = priv->rx_buf;
            cur_rx_pkt.length = pkt_len;
            dev->callback(dev, (UINT32)NET_RECV_PKT, (UINT32)&cur_rx_pkt);
            NET_SIM_PRINTF(" %d\n", pkt_len);
        }
        NET_PRINTF("\n");
        //priv->last_proc = header.next_loc;


        // The EtherType field, like most items transmitted on the Ethernet medium
        // are in big endian.
        //header.Type.Val = swaps(header.Type.Val);

        // Validate the data returned from the ENC28J60.  Random data corruption,
        // such as if a single SPI bit error occurs while communicating or a
        // momentary power glitch could cause this to occur in rare circumstances.
        /*if(header.NextPacketPointer > RXSTOP || ((BYTE_VAL*)(&header.NextPacketPointer))->bits.b0 ||
           header.StatusVector.bits.Zero ||
           header.StatusVector.bits.CRCError ||
           header.StatusVector.bits.ByteCount > 1518u ||
           !header.StatusVector.bits.ReceiveOk)
        {
            Reset();
        }*/


    }

}

void enc_hw_init(struct net_device *dev)
{
    net_enc_private *priv = (net_enc_private *)dev->priv;
    UINT8 i;
    UINT32 tmo = 1000;

    // Disable packet reception
    bfcreg(priv, ECON1, ECON1_RXEN);
    bank_sel(priv, 1<<8);
    i = (UINT8)(read_eth(priv, (UINT8)EPKTCNT).val);
    while(i)
    {
        // Decrement the RX packet counter register, EPKTCNT
        while(i)
        {
            bfsreg(priv, ECON2, ECON2_PKTDEC);
            i--;
        }
        i = (UINT8)(read_eth(priv, (UINT8)EPKTCNT).val);
    }
    bank_sel(priv, 0<<8);
    do
    {
        enc_reset(priv);
        i = (UINT8)(read_eth(priv, ESTAT).val);
        tmo --;
        if(0==tmo)
        {
            NET_PRINTF("%s:Wait HW ready TMO!\n", __FUNCTION__);
            return;
        }
    } while((i & 0x08) || (~i & ESTAT_CLKRDY));


    // Receive buffer management init
    //priv->last_proc = 0;
    //priv->discarded = TRUE;
    priv->next_loc = RXSTART;
    bank_sel(priv,     0<<8);
    bfsreg(priv, ECON1, ECON1_RXRST);
    osal_delay(5);
    bfcreg(priv, ECON1, ECON1_RXRST);

    write_reg(priv, ERXSTL, LOW(RXSTART));
    write_reg(priv, ERXSTH, HIGH(RXSTART));
    write_reg(priv, ERXRDPTL, LOW(RXSTOP));    // Write low byte first
    write_reg(priv, ERXRDPTH, HIGH(RXSTOP));    // Write high byte last
    write_reg(priv, ERXNDL, LOW(RXSTOP));
    write_reg(priv, ERXNDH, HIGH(RXSTOP));
    write_reg(priv, ETXSTL, LOW(TXSTART));
    write_reg(priv, ETXSTH, HIGH(TXSTART));
    // Write a permanant per packet control byte of 0x00
    write_reg(priv, EWRPTL, LOW(TXSTART));
    write_reg(priv, EWRPTH, HIGH(TXSTART));
    put_mac(priv, 0x00);


    // Enter Bank 1 and configure Receive Filters
    //Currently, use OR logic and enable unicast/crc/broadcast, that means multi cast not support.
    bank_sel(priv,     1<<8);
    write_reg(priv, (UINT8)ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN/*|ERXFCON_MCEN*/);

    // Write ERXFCON_CRCEN only to ERXFCON to enter promiscuous mode

    // Promiscious mode example:
    //BankSel(priv, ERXFCON);
    //WriteReg(priv, (UINT8)ERXFCON, ERXFCON_CRCEN);
    // soc_printf("ee%x\n",read_mac(priv, (UINT8)ERXFCON).Val);

    // Enter Bank 2 and configure the MAC
    bank_sel(priv, 2<<8);

    // Enable the receive portion of the MAC
    write_reg(priv, (UINT8)MACON1, MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);

    // Pad packets to 60 bytes, add CRC, and check Type/Length field.
    if(NET_LINK_FULL==priv->link_mode)
    {
        write_ma_mi_reg(priv, (UINT8)MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);
        // Set non-back-to-back inter-packet gap to 9.6us.
        write_ma_mi_reg(priv, (UINT8)MABBIPG, 0x15);
    }
    else
    {
        write_ma_mi_reg(priv, (UINT8)MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
        // Set non-back-to-back inter-packet gap to 9.6us.
        write_ma_mi_reg(priv, (UINT8)MABBIPG, 0x12);
    }

    // Allow infinite deferals if the medium is continuously busy
    // (do not time out a transmission if the half duplex medium is
    // completely saturated with other people's data)
    write_ma_mi_reg(priv, (UINT8)MACON4, MACON4_DEFER);

    // Late collisions occur beyond 63+8 bytes (8 bytes for preamble/start of frame delimiter)
    // 55 is all that is needed for IEEE 802.3, but ENC28J60 B5 errata for improper link pulse
    // collisions will occur less often with a larger number.
    write_ma_mi_reg(priv, (UINT8)MACLCON2, 63);


    write_ma_mi_reg(priv, (UINT8)MAIPGL, 0x12);
    write_ma_mi_reg(priv, (UINT8)MAIPGH, 0x0C);

    // Set the maximum packet size which the controller will accept
    write_ma_mi_reg(priv, (UINT8)MAMXFLL, LOW(6+6+2+1500+4));     // 1518 is the IEEE 802.3 specified limit
    write_ma_mi_reg(priv, (UINT8)MAMXFLH, HIGH(6+6+2+1500+4)); // 1518 is the IEEE 802.3 specified limit

    // Enter Bank 3 and initialize physical MAC address registers

    bank_sel(priv, 3<<8);
    osal_delay(3);
    write_ma_mi_reg(priv, (UINT8)MAADR1, priv->mac_addr[0]);
    write_ma_mi_reg(priv, (UINT8)MAADR2, priv->mac_addr[1]);
    write_ma_mi_reg(priv, (UINT8)MAADR3, priv->mac_addr[2]);
    write_ma_mi_reg(priv, (UINT8)MAADR4, priv->mac_addr[3]);
    write_ma_mi_reg(priv, (UINT8)MAADR5, priv->mac_addr[4]);
    write_ma_mi_reg(priv, (UINT8)MAADR6, priv->mac_addr[5]);
    NET_PRINTF("mac = %02x %02x %02x %02x %02x %02x\n", read_mac(priv, (UINT8)MAADR1).val, read_mac(priv, (UINT8)MAADR2).val, read_mac(priv, (UINT8)MAADR3).val
    , read_mac(priv, (UINT8)MAADR4).val, read_mac(priv, (UINT8)MAADR5).val, read_mac(priv, (UINT8)MAADR6).val);

    //Disable the CLKOUT output to reduce EMI generation
    write_reg(priv, (UINT8)ECOCON, 0x00);    // Output off (0V)
    //WriteReg(priv, (UINT8)EFLOCON, 0x00);    // Flow control
    //WriteReg(priv, (UINT8)ECOCON, 0x01);    // 25.000MHz
    //WriteReg(priv, (UINT8)ECOCON, 0x03);    // 8.3333MHz (*4 with PLL is 33.3333MHz)

    // Get the Rev ID so that we can implement the correct errata workarounds
    priv->enc_revid = read_eth(priv, (UINT8)EREVID).val;
    NET_PRINTF("ENC28J60 REVID %d\n", priv->enc_revid);
    // Disable half duplex loopback in PHY.  Bank bits changed to Bank 2 as a
    // side effect.
    enc_write_phy(priv, PHCON2, PHCON2_HDLDIS);

    // Configure LEDA to display LINK status, LEDB to display TX/RX activity
    enc_write_phy(priv, PHLCON, 0x3472);

    // Set the MAC and PHY into the proper duplex state

    if(NET_LINK_FULL==priv->link_mode)
        enc_write_phy(priv, PHCON1, PHCON1_PDPXMD);
    else
        enc_write_phy(priv, PHCON1, 0x0000);


    bank_sel(priv, 0<<8);        // Return to default Bank 0
    if(priv->int_by_gpio)
    {
        bfsreg(priv, EIE, EIE_INTIE|EIE_PKTIE);
    }
    // Enable packet reception
    bfsreg(priv, ECON1, ECON1_RXEN);

}

static RET_CODE net_enc_start_xmit(struct net_device *dev, void *buf, UINT16 len)
{
    UINT16 data_len = len;
    UINT8 ctrl_byte = 0x0e;
    UINT16 tmo = 2000;
    net_enc_private * priv = (net_enc_private *)dev->priv;

    enc_enter_op(priv);

    NET_PRINTF("\n>>%d", len);
    NET_SIM_PRINTF(">:");
#if(DUMP_TSM_FRAME)
    dump_mac_frame(buf, 0);
#endif
    while(!enc_is_tx_ready(priv))
    {
        NET_PRINTF("^");
        tmo --;
        if(1500==tmo||1000==tmo||500==tmo)
        {
            bfsreg(priv, ECON1, ECON1_TXRST);
                bfcreg(priv, ECON1, ECON1_TXRST);
        }
        enc_leave_op(priv);
            osal_task_sleep(1);
        if(0==tmo)
        {
            NET_PRINTF("Wait TX ready TMO!\n");
            return ERR_FAILUE;
        }
           enc_enter_op(priv);
    }


    write_reg(priv, EWRPTL, LOW(TXSTART));
        write_reg(priv, EWRPTH, HIGH(TXSTART));
    put_mac_array(priv, &ctrl_byte, 1);
     // Calculate where to put the TXND pointer
        data_len += (TXSTART);

    // Write the TXND pointer
    write_reg(priv, ETXNDL, ((UINT8 *)&data_len)[0]);
    write_reg(priv, ETXNDH, ((UINT8 *)&data_len)[1]);

    if(priv->dma_cpy)
    {
        UINT32 flag = SPE_BYTE_TRANS|SPE_DMA_WRITE|SPE_DMA_SYNC|SPE_ADDR_NOINC;
        write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA/*|SF_CONT_WR*/);
        write_uint8(SF_INS, WBM);
        osal_cache_flush((void *)buf, len);
        priv->dma_cpy(priv->spe_id, priv->map_addr+0, (UINT32)buf, len, flag);
    }
    else
        put_mac_array(priv, (UINT8 *)buf, len);

    enc_flush_mac((struct net_device *)dev);
    NET_SIM_PRINTF(" %d\n", len);
#ifdef ENC_DEBUG_BUF
    if(1)
    {
        UINT32 i, j;
        UINT8 * buf_rd = priv->tx_buf;
        UINT8 * buf_wr = (UINT8 *)buf;

        if(len!=priv->tx_len)
            NET_PRINTF("TX LEN %d != %d\n", len, priv->tx_len);

        for(i=0; i<len; i++)
        {
            if(buf_rd[i]!=buf_wr[i])
            {
                NET_PRINTF("tx diff from %d\n", i);
                NET_PRINTF("source: \n");
                for(j=0; j<32; j++)
                    NET_PRINTF("%02x ", buf_wr[i+j]);
                NET_PRINTF("\ntarget: \n");
                for(j=0; j<32; j++)
                    NET_PRINTF("%02x ", buf_rd[i+j]);
                NET_PRINTF("\n\n");
                break;
            }
        }
        if(i==len)
            NET_PRINTF("tx data OK\n");
    }

#endif
        enc_leave_op(priv);

        return SUCCESS;
}

static void net_enc_hsr(UINT32 param)
{
    NET_PRINTF(".");
    osal_mutex_unlock((OSAL_ID)param);

}

static void net_enc_interrupt(UINT32 param)
{
    struct net_device *dev = (struct net_device *)param;
    net_enc_private *priv = (net_enc_private *)dev->priv;
    //NET_PRINTF(".\n");
    if(HAL_GPIO_INT_STA_GET(priv->int_position)==0)
    {
        //NET_PRINTF("^\n");
        return;
    }
    if(HAL_GPIO_BIT_GET(priv->int_position)!=priv->int_polar)
    {
        //NET_PRINTF("$\n");
        return;
    }
    HAL_GPIO_INT_SET(priv->int_position, 0);
    priv->int_enable = 0;
    HAL_GPIO_INT_CLEAR(priv->int_position);
    NET_PRINTF("*");
    //NET_PRINTF("%s \n", __FUNCTION__);

    osal_interrupt_register_hsr(net_enc_hsr, (UINT32)(priv->tx_rx_mutex));
}

static RET_CODE net_enc_open(struct net_device *dev, void (*callback)(struct net_device *,UINT32, UINT32))
{
    net_enc_private *priv = (net_enc_private *)dev->priv;
    OSAL_T_CTSK t_ctsk;
    UINT8 *rx_buf;

    /* Allocate memory for rx buffer */
    rx_buf = (UINT8 *)MALLOC(ENC_RX_BUF_LEN);
    if (rx_buf == NULL)
    {
        NET_PRINTF("net_enc_open: Alloc net device receive buffer error!\n");
        return ERR_NO_MEM;
    }
    MEMSET(rx_buf, 0, sizeof(ENC_RX_BUF_LEN));
    priv->rx_buf = rx_buf;
#ifdef ENC_DEBUG_BUF
    /* Allocate memory for tx debug buffer */
    rx_buf = (UINT8 *)MALLOC(ENC_TX_BUF_LEN);
    if (rx_buf == NULL)
    {
        NET_PRINTF("net_enc_open: Alloc net device rx dbg buffer error!\n");
        return ERR_NO_MEM;
    }
    MEMSET(rx_buf, 0, sizeof(ENC_TX_BUF_LEN));
    priv->tx_buf= rx_buf;
#endif
    //priv->last_proc = 0;
    //priv->discarded = TRUE;
        priv->next_loc = RXSTART;
    dev->callback = callback;

    priv->tx_rx_mutex = osal_mutex_create();
    ASSERT(priv->tx_rx_mutex!=OSAL_INVALID_ID);
    osal_mutex_lock(priv->tx_rx_mutex, OSAL_WAIT_FOREVER_TIME);
    t_ctsk.stksz = 0x4000;
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.para1=(UINT32)dev ;
    t_ctsk.para2=0 ;
    t_ctsk.name[0] = 'E';
    t_ctsk.name[1] = 'N';
    t_ctsk.name[2] = 'C';
    t_ctsk.task = (FP)net_enc_task;
    priv->tsk_id = osal_task_create(&t_ctsk);
    if (priv->tsk_id== OSAL_INVALID_ID)
    {
        NET_PRINTF("net_enc_open: Create net_enc_task failed!\n");
        return ERR_FAILUE;
    }
    if(priv->int_by_gpio)
    {
        HAL_GPIO_INT_SET(priv->int_position, 1);
        HAL_GPIO_INT_REDG_SET(priv->int_position, 0);
        HAL_GPIO_INT_FEDG_SET(priv->int_position, 1);
        HAL_GPIO_INT_CLEAR(priv->int_position);
        priv->int_enable = 1;
        if(ALI_S3602==enc_drv_chip_id)
        {
            priv->int_number = 8;
            //NET_PRINTF("Fix Me: %s %d: I don't know the gpio interrupt number setting!\n", __FILE__, __LINE__);
            //while(1);
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5<=enc_drv_chip_rev)
        {
            if(priv->int_position<32)
                priv->int_number = 9;
            else if(priv->int_position<64)
                priv->int_number = 24;
            else
                priv->int_number = 25;
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5>enc_drv_chip_rev)
        {
            if(priv->int_position<32)
                priv->int_number = 9;
            else if(priv->int_position<64)
                priv->int_number = 24;
            else
                priv->int_number = 31;
        }
        else
        {
            NET_PRINTF("Fix Me: %s %d: I don't know the gpio interrupt number setting!\n", __FILE__, __LINE__);
            while(1);
            //if(ALI_M3327C==enc_drv_chip_id)
            //if(ALI_M3329E==enc_drv_chip_id)
            //if(sys_ic_is_M3202())
        }
        osal_interrupt_register_lsr(priv->int_number, net_enc_interrupt, (UINT32)dev);
    }
    enc_enter_op(priv);
#ifdef ENC_DEBUG
    dump_reg(priv);
#endif
    enc_hw_init(dev);
#ifdef ENC_DEBUG
    dump_reg(priv);
#endif
    enc_leave_op(priv);
    NET_PRINTF("<%d ====RX: %d ==== %d><%d====TX: %d==== %d>\n", RXSTART, \
        RXSIZE, RXSTOP, TXSTART, TXSIZE, (HW_RAMSIZE-1));
    return SUCCESS;
}

//Close net device
static RET_CODE net_enc_close(struct net_device *dev)
{
    net_enc_private *priv = (net_enc_private *)(dev->priv);

    enc_enter_op(priv);
    if(priv->int_by_gpio)
        osal_interrupt_unregister_lsr(priv->int_number, net_enc_interrupt);
    osal_task_delete(priv->tsk_id);
    osal_mutex_delete(priv->tx_rx_mutex);
#ifdef ENC_DEBUG_BUF
    FREE(priv->tx_buf);
#endif
    FREE(priv->rx_buf);
    enc_leave_op(priv);
    NET_PRINTF("%s.\n", __FUNCTION__);
    return SUCCESS;
}

//Net device io control
static RET_CODE net_enc_ioctl(struct net_device *dev, UINT32 cmd, UINT32 param)
{

    NET_PRINTF("net_enc_ioctl: No content in net device ioctl!\n");
    return SUCCESS;
}


static RET_CODE net_enc_get_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
    net_enc_private *priv = (net_enc_private *)dev->priv;
    UINT32 ret = SUCCESS;
    UINT8 ret_info[32];
    UINT32 ret_len = 0;

    if (info_buf == NULL)
        return ERR_PARA;
    NET_PRINTF("%s: info_type %d\n", __FUNCTION__, info_type);
    switch(info_type)
    {
        case NET_GET_CURRENT_MAC :
            MEMCPY(ret_info, priv->mac_addr, ETH_ALEN);
            ret_len = ETH_ALEN;
                break;
            case NET_GET_PERMANENT_MAC :
            enc_enter_op(priv);
                MEMCPY(ret_info, priv->mac_addr, ETH_ALEN);
            enc_leave_op(priv);
                ret_len = ETH_ALEN;
                break;
            case NET_GET_LINK_STATUS :
            enc_enter_op(priv);
                *((enum net_link_status *)ret_info) = enc_is_linked(priv)?NET_LINK_CONNECTED:NET_LINK_DISCONNECTED;
            enc_leave_op(priv);
                ret_len = sizeof(enum net_link_status);
                break;
            case NET_GET_LINK_SPEED :
                  *((enum net_link_speed *)ret_info) = NET_LINK_10MBPS;
            ret_len = sizeof(enum net_link_speed);
                break;
            case NET_GET_LINK_MODE :
              *((enum net_link_mode *)ret_info) = priv->link_mode;
            ret_len = sizeof(enum net_link_mode);
            break;
            default :
            ret = ERR_PARA;
    }
    if (ret == SUCCESS)
    {
        MEMCPY(info_buf, ret_info, ret_len);
    }

    return ret;
}

static RET_CODE net_enc_set_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
    net_enc_private *priv = (net_enc_private *)dev->priv;
    UINT32 ret = SUCCESS;

    if (info_buf == NULL)
        return ERR_PARA;


    switch(info_type)
    {
        case NET_SET_MAC :
            enc_enter_op(priv);
            MEMCPY(priv->mac_addr, info_buf, ETH_ALEN);
            bank_sel(priv, 3<<8);
            write_ma_mi_reg(priv, (UINT8)MAADR1, priv->mac_addr[0]);
            write_ma_mi_reg(priv, (UINT8)MAADR2, priv->mac_addr[1]);
            write_ma_mi_reg(priv, (UINT8)MAADR3, priv->mac_addr[2]);
            write_ma_mi_reg(priv, (UINT8)MAADR4, priv->mac_addr[3]);
            write_ma_mi_reg(priv, (UINT8)MAADR5, priv->mac_addr[4]);
            write_ma_mi_reg(priv, (UINT8)MAADR6, priv->mac_addr[5]);
            NET_PRINTF("mac = %02x %02x %02x %02x %02x %02x\n", read_mac(priv, (UINT8)MAADR1).val, read_mac(priv, (UINT8)MAADR2).val, read_mac(priv, (UINT8)MAADR3).val
            , read_mac(priv, (UINT8)MAADR4).val, read_mac(priv, (UINT8)MAADR5).val, read_mac(priv, (UINT8)MAADR6).val);
            bank_sel(priv, 0<<8);
            enc_leave_op(priv);
            break;
        case NET_SET_LINK_MODE :
        case NET_SET_LINK_SPEED :
        case NET_ADD_MULTICAST_ADDR :
        case NET_DEL_MULTICAST_ADDR :
        default :
            ret = ERR_PARA;
            break;
    }


    return ret;
}

static RET_CODE net_enc_detach(struct net_device *dev)
{
    net_enc_private *priv = (net_enc_private *)(dev->priv);

    NET_PRINTF("net_enc_detach: Detach net device driver.\n");

    FREE(priv);
    dev_free(dev);
    return SUCCESS;
}

static char net_enc_name[HLD_MAX_NAME_SIZE] = "NET_ENC28J60_0";
RET_CODE net_enc28j60_attach(ENC_CONFIG * cfg)
{
    struct net_device *dev;
    net_enc_private *priv;
    UINT32 mem_clk, enc_clk;
    UINT32 div;
    UINT8 revid;
    if(NULL==cfg)
        return ERR_FAILUE;
    enc_drv_chip_id = sys_ic_get_chip_id();
    enc_drv_chip_rev = sys_ic_get_rev_id();
    if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5<=enc_drv_chip_rev)
        sflash_ctrl_reg |= 0x8000;
    /* Alloc structure space for device */
    dev = (struct net_device *)dev_alloc(net_enc_name, HLD_DEV_TYPE_NET, sizeof(struct net_device));
    if (dev == NULL)
    {
        NET_PRINTF("net_enc_attach: Alloc net device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space for private */
    priv = (void *)MALLOC(sizeof(net_enc_private));
    if (priv == NULL)
    {
        dev_free(dev);
        NET_PRINTF("net_enc_attach: Alloc net device private memory error!\n");
        return ERR_NO_MEM;
    }
    MEMSET(priv, 0, sizeof(net_enc_private));
    dev->priv =  (void *)priv;
    /*Init interface setting*/
    if(cfg->cs_by_gpio)
    {
        if(!cfg->cs2_by_gpio)
        {
            NET_PRINTF("Fix Me: %s %d: we must have second GPIO for PHY setting!\n", __FILE__, __LINE__);
            while(1);
        }
            dev->base_addr = 0xafc00000;

        priv->cs_by_gpio = 1;
        priv->cs_io = HAL_GPIO_O_DIR;
        priv->cs_polar = cfg->cs_polar;
        priv->cs_position = cfg->cs_position;

        priv->cs2_by_gpio = 1;
        priv->cs2_io = HAL_GPIO_O_DIR;
        priv->cs2_polar = cfg->cs2_polar;
        priv->cs2_position = cfg->cs2_position;

        HAL_GPIO_BIT_SET(priv->cs_position, !(priv->cs_polar));
        HAL_GPIO_BIT_DIR_SET(priv->cs_position, priv->cs_io);
        HAL_GPIO_BIT_SET(priv->cs2_position, !(priv->cs2_polar));
        HAL_GPIO_BIT_DIR_SET(priv->cs2_position, priv->cs2_io);
    }
    else
    {
        if(ALI_S3602==enc_drv_chip_id)
        {
            dev->base_addr = priv->base_addr = 0xAE000000;
            /*CS GPIO share pin*/
            HAL_GPIO_BIT_SET(51, 1);
              HAL_GPIO_BIT_DIR_SET(51, HAL_GPIO_O_DIR);
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5<=enc_drv_chip_rev)
        {
            dev->base_addr = priv->base_addr = 0xAE000000;
            /*CS GPIO share pin*/
            HAL_GPIO_BIT_SET(7, 1);
              HAL_GPIO_BIT_DIR_SET(7, HAL_GPIO_O_DIR);
        }
        else if(ALI_M3329E==enc_drv_chip_id&&IC_REV_5>enc_drv_chip_rev)
        {
            dev->base_addr = priv->base_addr = 0xAE000000;
            /*CS GPIO share pin*/
            HAL_GPIO_BIT_SET(64, 1);
              HAL_GPIO_BIT_DIR_SET(64, HAL_GPIO_O_DIR);
        }
        else
        {
            NET_PRINTF("Fix Me: %s %d: I don't know the flash space and pin mux setting!\n", __FILE__, __LINE__);
            while(1);
            //if(ALI_M3327C==enc_drv_chip_id)
            //if(ALI_M3329E==enc_drv_chip_id)
            //if(sys_ic_is_M3202())
        }
    }

    /*reset setting*/
    if(cfg->rst_by_gpio)
    {
        priv->rst_by_gpio = 1;
        priv->rst_io = HAL_GPIO_O_DIR;
        priv->rst_polar = cfg->rst_polar;
        priv->rst_position = cfg->rst_position;
        HAL_GPIO_BIT_SET(priv->rst_position, !(priv->rst_polar));
        HAL_GPIO_BIT_DIR_SET(priv->rst_position, priv->rst_io);
    }

    /*int setting*/
    priv->tsk_sleep = 10;
    if(cfg->int_by_gpio)
    {
        priv->int_by_gpio = 1;
        priv->int_io = HAL_GPIO_I_DIR;
        priv->int_polar = cfg->int_polar;
        priv->int_position = cfg->int_position;
        //priv->tsk_sleep = 1000;
        HAL_GPIO_BIT_DIR_SET(priv->int_position, priv->int_io);
    }
    if(cfg->spe_cpy_en)
    {
        priv->spe_id = cfg->spe_id;
        priv->map_addr = cfg->map_addr;
        priv->dma_cpy = cfg->dma_cpy;
    }
    /*timming setting*/
    /*20M > mem_clk/2X(div+1) ==> div > (mem_clk - 40M)/40M */

    mem_clk = sys_ic_get_dram_clock()*1000;

    div = (mem_clk - 40000)/40000;
    enc_clk = mem_clk/(2*(div+1));
    if(enc_clk>20000)
        div++;
    priv->enc_clk_div = (UINT8)div;
    priv->enc_clk_div &= 0xf;
    priv->link_mode = cfg->link_mode;
    if(cfg->init_mac_addr)
        MEMCPY(priv->mac_addr, cfg->mac_addr, 6);
    else
        MEMCPY(priv->mac_addr, default_mac, 6);
    /* Init function point */
    dev->attach = NULL;//net_enc28j60_attach;
    dev->detach = net_enc_detach;
    dev->open = net_enc_open;
    dev->close = net_enc_close;
    dev->ioctl = net_enc_ioctl;
    dev->send_packet = net_enc_start_xmit;
    dev->callback = NULL;
    dev->get_info = net_enc_get_info;
    dev->set_info = net_enc_set_info;

    enc_enter_op(priv);
    bank_sel(priv, 3<<8);
    revid = read_eth(priv, (UINT8)EREVID).val;
    bank_sel(priv, 0<<8);
    enc_leave_op(priv);
    NET_PRINTF("check enc dev id : %d\n", revid);
    if((revid<4)||(revid>7))
    {
        FREE(priv);
        dev_free(dev);
        NET_PRINTF("Incorrect enc28j60 chip id %d\n", revid);
        return ERR_FAILUE;
    }
    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NET_PRINTF("net_enc_attach: Register net device error!\n");
        FREE(priv);
        dev_free(dev);
        return ERR_FAILUE;
    }
    return SUCCESS;
}

