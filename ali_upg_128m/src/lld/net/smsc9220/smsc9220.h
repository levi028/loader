#ifndef __SMSC_9220__
#define __SMSC_9220__

#include <os/tds2/alitypes.h>
#include <sys_config.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>


#include <basic_types.h>
#include <hld/hld_dev.h>
#include <hld/net/net.h>
#include <hld/net/ethtool.h>

#include <bus/spe/spe.h>
#include <hld/net/net_dev.h>


//#define DEBUG_SMSC

#ifdef DEBUG_SMSC
#define SMSC_TRACE        libc_printf
#define SMSC_WARNING    libc_printf
#else
#define SMSC_TRACE(...)        do{}while(0)
#define SMSC_WARNING(...)    do{}while(0)
#endif

#define SMSC_ASSERT        ASSERT
//#define SMSC_ASSERT(...)        do{}while(0)

//#define SMSC_UP_BUS

#define SMSC_TRANSFER_PIO    0x00
#define SMSC_TRANSFER_DMA    0x01

#define SMSC_PKT_BUFFER    1536
#define SMSC_ALIGNMENT 16

#define SMSC_RXPKTS_PERTIME 100

#define IDE_INTR_TXEND_INT 0x02
#define IDE_FLAG_TXEND_INT IDE_INTR_TXEND_INT


#define PLATFORM_DMA_THRESHOLD    200

//Link Status.
#define LINK_OFF                    (0x00UL)
#define LINK_SPEED_10HD        (0x01UL)
#define LINK_SPEED_10FD            (0x02UL)
#define LINK_SPEED_100HD        (0x04UL)
#define LINK_SPEED_100FD        (0x08UL)
#define LINK_SYMMETRIC_PAUSE    (0x10UL)//both-direction Pause Fame
#define LINK_ASYMMETRIC_PAUSE    (0x20UL)//one-direction Pause Fame
#define LINK_AUTO_NEGOTIATE    (0x40UL)

#define LINK_ADVERTISE    (LINK_SPEED_10HD|LINK_SPEED_10FD|LINK_SPEED_100HD|LINK_SPEED_100FD \
                        |LINK_AUTO_NEGOTIATE)

#define HIBYTE(word)  ((UINT8)(((UINT16)(word))>>8))
#define LOBYTE(word)  ((UINT8)(((UINT16)(word))&0x00FFU))
#define HIWORD(d_word) ((UINT16)(((UINT32)(d_word))>>16))
#define LOWORD(d_word) ((UINT16)(((UINT32)(d_word))&0x0000FFFFUL))


#ifndef SMSC_UP_BUS
//Settings
#define _3602_IDE_IO_BASE        0xB8010000
#define _3329_IDE_IO_BASE        0xB8008000

#define ATA_DAR            0x00
#define ATA_AAR            0x02

#define ATA_FSMCR        0x3
#define ATA_CR            0x4

#define ATA_BC4TR        0x08
#define ATA_PIOTCR        0x10

#define ATA_ICR            0x20
#define ATA_ISR            0x21

#define ATA_BFSADR        0x80
#define ATA_BFDADR        0x84

struct smsc_mutex
{
    void (*lock) (void);
    void (*unlock) (void);
};

static /*inline*/UINT32 ata_r32(UINT8 offset)
{
    return *((volatile UINT32 *)(ATA_IO_BASE+offset));
}

static /*inline*/ void ata_w32(UINT8 offset, UINT32 data)
{
    *((volatile UINT32 *)(ATA_IO_BASE+offset)) = data;
}

static /*inline*/UINT16 ata_r16(UINT8 offset)
{
    //asm("nop");
    return *((volatile UINT16 *)(ATA_IO_BASE+offset));
}

static /*inline*/ void ata_w16(UINT8 offset, UINT16 data)
{
    *((volatile UINT16 *)(ATA_IO_BASE+offset)) = data;
    //asm("nop");
}

static /*inline*/ UINT8 ata_r8(UINT8 offset)
{
    //asm("nop");
    return *((volatile UINT8 *)(ATA_IO_BASE+offset));
}

static /*inline*/ void ata_w8(UINT8 offset, UINT8 data)
{
    *((volatile UINT8 *)(ATA_IO_BASE+offset)) = (data);
    //asm("nop");
}

#endif //SMSC_UP_BUS


typedef struct _PRIVATE_DATA {
    struct net_device *dev;

    struct net_device_stats stats;

    UINT32 chip_id;            //ALi Chip Id.
    UINT32 irq;                //GPIO Irq.

    //Chip information.
    UINT32 id_rev;            // =>0x92200000.

    //Config Options.
    UINT32 transfer_mode;        //PIO/DMA.
    UINT32 tx_fifo_size;        //from 0x00020000UL(2K) to 0x000E0000UL(14K).
    UINT32 interrupt_config;    //
    UINT32 debug_mode;        //debug mode. if yes=>0x7UL, if no=>0x0UL.
    UINT32 afc_config;            //
    UINT32 rx_max_data_fifo_size;
    UINT32 dma_threshold;    //minimum packet size for DMA to be used.
    BOOL use_tx_csum;        //en tx checksum.
    BOOL use_rx_csum;        //en rx checksum.


    //Phy.
    UINT32 phy_addr;
    UINT32 link_mode;
    UINT32 link_speed;        //Link Speed & Duplex mode.(must be one of Link Settings).FM-20090105.
    UINT32 link_settings;        //Link Settings.
    UINT16 auto_mdix;            //0=>Use Straight Cable, 1=>Use CrossOver, 2=>En AutoMdix.FM-20090105.
    OSAL_ID task_id;
    BOOL link_status;

    //Mac
    UINT32 mac_addr_hi16;
    UINT32 mac_addr_lo32;
    UINT8 dev_addr[ETH_ALEN];
    UINT16 w_last_adv;
    UINT16 last_advat_restart;

    //Status mark.
    BOOL lan_initialized;
    BOOL software_interrupt_signal;
    BOOL register_isr;

    //Internal Resources.
    OSAL_ID  mac_phy_lock;        //MAC CSR mutex.
    OSAL_ID  trans_mutex;        //ATA Transfer mutex.
    OSAL_ID  reg_mutex;        //SMSC Register mutex.
    OSAL_ID  int_sem;            //ATA Transfer mutex.

    //Rx.
    UINT32 *rx_buffer;                //DWORD Rx Buffer.FM-20090103
    UINT32 rx_interrupts;
    UINT32 rx_off_count;                //?

    BOOL rx_overrun;                    //SMSC9220 is not Generation 0, RxOverrun is always False.FM-20090103

    //Tx.
    UINT16 tx_tag;

    //Interrupt Status.
    UINT32 last_int_status1;    //(New)
    UINT32 last_int_status2;
    UINT32 last_int_status3;    //(Old)

} PRIVATE_DATA, *PPRIVATE_DATA;



__ATTRIBUTE_REUSE_
RET_CODE smsc9220_attach(SMSC_CONFIG *cfg);

RET_CODE
smsc9220_open(struct net_device*dev, void (*callback)(struct net_device *,UINT32, UINT32));

RET_CODE
smsc9220_send_packet(struct net_device *dev, void *buf, UINT16 len);

RET_CODE
smsc9220_ioctl(struct net_device *dev, UINT32 cmd, UINT32 param);

RET_CODE
smsc9220_get_info(struct net_device *dev, UINT32 info_type, void *info_buf);

RET_CODE
smsc9220_set_info(struct net_device *dev, UINT32 info_type, void *info_buf);

RET_CODE
smsc9220_close(struct net_device *dev);

RET_CODE
smsc9220_detach(struct net_device *dev);

#endif //__SMSC_9220__
