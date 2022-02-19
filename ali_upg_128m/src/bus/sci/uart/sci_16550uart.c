/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: sci_16550uart.c
*
*    Description:    This file contains all globe micros and functions declare
*                    of 16550 UART.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  Sep.19.2004       Justin Wu      Ver 0.1    Create file.
*   2.  Jan.13.2006       Justin Wu      Ver 0.2    Make support multi UART.
*   3.  Feb.16.2006       Justin Wu      Ver 0.3    Support multi task.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
******************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <bus/sci/sci.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <asm/chip.h>

#define SCI_16550_URBR              0
#define SCI_16550_UTBR              0
#define SCI_16550_UIER              1
#define SCI_16550_UIIR              2
#define SCI_16550_UFCR              2
#define SCI_16550_UDLL              0
#define SCI_16550_UDLM              1
#define SCI_16550_ULCR              3
#define SCI_16550_UMCR              4
#define SCI_16550_ULSR              5
#define SCI_16550_UMSR              6
#define SCI_16550_USCR              7
#define SCI_16550_DEVC              8
#define SCI_16550_RCVP              9

#ifdef CAS9_VSC
#define SCI_16550UART_RX_BUF_SIZE   1024
#else
#ifdef _AUI_NESTOR_
#define SCI_16550UART_RX_BUF_SIZE   1024 //in _UART_MODE_ update images over UART,the images is big ,so need bigger buffer,if not, maybe lost data easy
#else
#define SCI_16550UART_RX_BUF_SIZE   256
#endif
#endif

/* uart_write_lock(unlock) is used to remove the mutex protection
 * in sci_write(),
 *  for sci_write() might be used in interrupt handling fuction,
 *  while tds2 does not support synchronization object in interrupt function*/
#define uart_write_lock(...)              do{}while(0)
#define uart_write_unlock(...)            do{}while(0)
//#define uart_write_lock                 osal_mutex_lock
//#define uart_write_unlock               osal_mutex_unlock
#define IIR_PRIOR   0x06

static struct sci_uart_st
{
    UINT8  rx_buf[SCI_16550UART_RX_BUF_SIZE];
    UINT32 rx_buf_head;
    UINT32 rx_buf_tail;
    UINT8  loopback_flag;
    UINT32 rd_mutex_id;
    UINT32 wt_mutex_id;
    UINT32 timeout;
} *sci_16550 = NULL;
//static struct sci_uart_st

/* For M3327/M3327C serial chip */
#define SCI_16550_NUM               2

static int g_dev_num;

static struct
{
    UINT32 reg_base;
    int    irq;
    UINT32 strap_ctrl;
#if (SYS_CHIP_MODULE == ALI_S3601)
} sci_16550_reg[SCI_16550_NUM] = {{0xb8018300, 24, 0}, {0xb8018400, 25, 0}};
#elif (SYS_CHIP_MODULE == ALI_S3602)
#if (defined (SEE_CPU) && !defined (SEE_PRINTF_2_SAME_UART))
} sci_16550_reg[SCI_16550_NUM] = {{0xB8018600, 25, 0}, {0xB8018300, 24, 0}};
#else
    #if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT || defined(M3105_CHIP)))
} sci_16550_reg[SCI_16550_NUM] = {{0xb8001300, 16, 0}, {0xb8001400, 14, 0x00180000}};
    #else
} sci_16550_reg[SCI_16550_NUM] = {{0xB8018300, 24, 0}, {0xB8018600, 25, 0}};
    #endif
#endif
#elif (SYS_CHIP_MODULE==ALI_M3329E)
} sci_16550_reg[SCI_16550_NUM] = {{0xb8001300, 16, 0}, {0xb8001600, 14, 0}};
#else
} sci_16550_reg[SCI_16550_NUM] = {{0xb8001300, 16, 0}, {0xb8001400, 14, 0x00180000}};
#endif

#define SCI_READ8(id, reg)          (*((volatile UINT8 *)(sci_16550_reg[id].reg_base + reg)))
#define SCI_WRITE8(id, reg, data)   (*((volatile UINT8 *)(sci_16550_reg[id].reg_base + reg)) = (data))


static void sci_16550uart_interrupt(UINT32 id);

__ATTRIBUTE_REUSE_
INT32 sci_16550uart_attach(int dev_num)
{
    int i = 0;
    UINT32 chip_id = 0;
    UINT32 chip_ver = 0;
    UINT32 is_m3101 = 0;

    if (dev_num < 0)
    {
        return ERR_PARA;
    }
    if(ALI_S3811==sys_ic_get_chip_id())
    {
        if (dev_num > SCI_16550_NUM-1)
        {
            dev_num=1;
        }
    }

    if(ALI_S3281==sys_ic_get_chip_id())
    {
        if (dev_num > SCI_16550_NUM)
        {
            //  return ERR_FAILUE;
            dev_num=2;
        }
        sci_16550_reg[0].reg_base= 0xB8018300;
        sci_16550_reg[0].irq=24;
        sci_16550_reg[0].strap_ctrl=0;
        sci_16550_reg[1].reg_base= 0xB8018600;
        sci_16550_reg[1].irq=25;
        sci_16550_reg[1].strap_ctrl=0;
    }

    if(ALI_C3701==sys_ic_get_chip_id())
    {
        if (dev_num > SCI_16550_NUM)
        {
            //  return ERR_FAILUE;
            dev_num=2;
        }
        sci_16550_reg[0].reg_base= 0xB8018300;
        sci_16550_reg[0].irq=24;
        sci_16550_reg[0].strap_ctrl=0;
        sci_16550_reg[1].reg_base= 0xB8018600;
        sci_16550_reg[1].irq=25;
        sci_16550_reg[1].strap_ctrl=0;
    }
    if(ALI_S3503==sys_ic_get_chip_id())
    {
        if (dev_num > SCI_16550_NUM)
        {
            //  return ERR_FAILUE;
            dev_num=2;
        }
        sci_16550_reg[0].reg_base= 0xB8018300;
        sci_16550_reg[0].irq=24;
        sci_16550_reg[0].strap_ctrl=0;
        sci_16550_reg[1].reg_base= 0xB8018600;
        sci_16550_reg[1].irq=25;
        sci_16550_reg[1].strap_ctrl=0;
    }
    if (dev_num > SCI_16550_NUM)
    {
        return ERR_FAILUE;
    }

    chip_id = sys_ic_get_chip_id();
    chip_ver =sys_ic_get_rev_id();
    if ((ALI_M3329E == chip_id)&& (chip_ver>=IC_REV_5 ))
    {
        sci_16550_reg[SCI_16550_NUM-1].irq = 22;
    }

    is_m3101 = sys_ic_is_m3101();
    if((1==sys_ic_is_m3202()) || is_m3101)
    {
        sci_16550_reg[0].reg_base= 0xb8001300;
        sci_16550_reg[0].irq = 16;
        sci_16550_reg[0].strap_ctrl = 0;

        sci_16550_reg[SCI_16550_NUM-1].reg_base = 0xb8001600;
        sci_16550_reg[SCI_16550_NUM-1].irq = 22;
        sci_16550_reg[SCI_16550_NUM-1].strap_ctrl = 0;
    }
    if((ALI_S3821==sys_ic_get_chip_id())||
       (ALI_C3505==sys_ic_get_chip_id()))
    {
        //if (dev_num > 3)
        //{
            //  return ERR_FAILUE;
            //dev_num=2;
        //}
        sci_16550_reg[0].reg_base= 0xB8018300;
        sci_16550_reg[0].irq=24;
        sci_16550_reg[0].strap_ctrl=0;
        sci_16550_reg[1].reg_base= 0xB8018600;
        sci_16550_reg[1].irq=25;
        sci_16550_reg[1].strap_ctrl=0;
    }
    if (dev_num > 0)
    {
        //dev_num = (dev_num > SCI_16550_NUM) ? SCI_16550_NUM : dev_num;
        sci_16550 = (struct sci_uart_st *)MALLOC(sizeof(struct sci_uart_st) * dev_num);
        if (NULL == sci_16550)
        {
            return ERR_FAILUE;
        }
        MEMSET(sci_16550, 0, sizeof(struct sci_uart_st) * dev_num);
        g_dev_num=dev_num;

        for (i = 0; i < dev_num; i++)
        {
            sci_16550[i].rx_buf_head = 0;
            sci_16550[i].rx_buf_tail = 0;
            sci_16550[i].loopback_flag = 0;
            sci_16550[i].rd_mutex_id = osal_mutex_create();
            sci_16550[i].wt_mutex_id = osal_mutex_create();
#ifndef SEE_CPU
            osal_interrupt_register_lsr(sci_16550_reg[i].irq, sci_16550uart_interrupt, (UINT32)i);
#endif
            if (sci_16550_reg[i].strap_ctrl != 0)
            {
                *(UINT32 *)0xb8000074 = sci_16550_reg[i].strap_ctrl;
            }
        }
    }
    return SUCCESS;
}

INT32 sci_16550uart_detach()
{
    if (sci_16550 == NULL)
        return ERR_FAILUE;

    int i;

    for (i=0;  i<g_dev_num;  i++)
    {
        osal_interrupt_unregister_lsr(sci_16550_reg[i].irq, sci_16550uart_interrupt);
    }
    sci_16550=NULL;
    return SUCCESS;
}

#if 0
void sci_16550uart_set_high_speed_mode(UINT32 id, UINT32 mode)
{
    switch(mode)
    {
    case UART_SPEED_562500:
        //high clk mode
        SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) | 0x80);
        SCI_WRITE8(id, SCI_16550_UDLL, 0x06); //div
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) & (~0x80));
        break;
    case UART_SPEED_1687500:
        //high clk mode
        SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) | 0x80);
        SCI_WRITE8(id, SCI_16550_UDLL, 0x02); //div
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) & (~0x80));
        break;
    case UART_SPEED_3375000:
        //high clk mode
        SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) | 0x80);
        SCI_WRITE8(id, SCI_16550_UDLL, 0x01); //div
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) & (~0x80));
        break;
    case UART_SPEED_115200:
    default:
        //normal clk mode
        SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) \
                                                 & (~0x08));
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) \
                                                 | 0x80);
        SCI_WRITE8(id, SCI_16550_UDLL, 0x01); //div
        SCI_WRITE8(id, SCI_16550_ULCR, SCI_READ8(id, SCI_16550_ULCR) \
                                                 & (~0x80));
        break;
    }

    return;
}
#else
#define UART_1_BASE                 0xb8018300

#define REG8(x) (volatile unsigned char *)(UART_1_BASE + x)

// REG OFFSETS
#define REGOFF_URBR            0x0   // Receiver buffer reg
#define REGOFF_UTBR            0x0   // Transmitter buffer reg
#define REGOFF_UIER            0x01  // Interrupt enable reg
#define REGOFF_UIIR            0x02  // Interrupt identification reg
#define REGOFF_UFCR            0x02  // FIFO control reg
#define REGOFF_UDLL            0x0   // Divisor latch LSB
#define REGOFF_UDLM            0x01  // Divisor latch MSB
#define REGOFF_ULCR            0x03  // Line control reg
#define REGOFF_UMCR            0x04  // Modem control reg
#define REGOFF_ULSR            0x05  // Line status reg
#define REGOFF_UMSR            0x06  // Modem status reg
#define REGOFF_USCR            0x07  // Scratch pad reg
#define REGOFF_UDCR            0x08  // Clock select reg
#define REGOFF_URPR            0x09  // Receive parameter register
#define REGOFF_ISR1            0x0E
#define REGOFF_TX_TRIG         0x10
#define REGOFF_RX_TRIG         0x12
#define REGOFF_VERNUM          0xFC
#define REGOFF_IER1            0x0c

// REG DEFINES
#define REG_URBR     REG8(REGOFF_URBR)       // Receiver buffer reg
#define REG_UTBR     REG8(REGOFF_UTBR)       // Transmitter buffer reg
#define REG_UIER     REG8(REGOFF_UIER)       // Interrupt enable reg
#define REG_UIIR     REG8(REGOFF_UIIR)       // Interrupt identification reg
#define REG_UFCR     REG8(REGOFF_UFCR)       // FIFO control reg
#define REG_UDLL     REG8(REGOFF_UDLL)       // Divisor latch LSB
#define REG_UDLM     REG8(REGOFF_UDLM)       // Divisor latch MSB
#define REG_ULCR     REG8(REGOFF_ULCR)       // Line control reg
#define REG_UMCR     REG8(REGOFF_UMCR)       // Modem control reg
#define REG_ULSR     REG8(REGOFF_ULSR)       // Line status reg
#define REG_UMSR     REG8(REGOFF_UMSR)       // Modem status reg
#define REG_USCR     REG8(REGOFF_USCR)       // Scratch pad reg
#define REG_UCSR     REG8(REGOFF_UDCR)       // Clock select reg
#define REG_UIER1    REG8(REGOFF_IER1)
#define REG_UISR1    REG8(REGOFF_ISR1)
#define REG_UTX_TRIG REG8(REGOFF_TX_TRIG)
#define REG_URX_TRIG REG8(REGOFF_RX_TRIG)
#define REG_UVERNUM  REG8(REGOFF_VERNUM)


//for high speed mode
void sci_16550uart_set_high_speed_mode(UINT32 id, UINT32 mode)
{
    id = id;
    switch(mode)
    {
    case UART_SPEED_115200:
        //set baudrate 115200 mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDCR) &= (~0X08);
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULCR) = 0X9B;
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDLL) = 0X01;
        break;
    case UART_SPEED_562500:
        //set baudrate 3.375M mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDCR) = 0X08;
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULCR) = 0X9B;
    //set baudrate 562K5 mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDLL) = 0X06;
        break;
    case UART_SPEED_1687500:
        //set baudrate 3.375M mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDCR) = 0X08;
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULCR) = 0X9B;
    //set baudrate 1.6875M mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDLL) = 0X02;
        break;
    case UART_SPEED_3375000:
        //set baudrate 3.375M mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDCR) = 0X08;
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULCR) = 0X9B;
    //set baudrate 3.375M mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDLL) = 0X01;
        break;
    default:
    //set baudrate 115200 mode.
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDCR) &= (~0X08);
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULCR) = 0X9B;
        *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDLL) = 0X01;
        break;
    }

    *(volatile unsigned char*)(UART_1_BASE + REGOFF_UDLM) = 0X0;
    *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULCR) = 0X1B;
    *(volatile unsigned char*)(UART_1_BASE + REGOFF_UIER) = 0X07;
    *(volatile unsigned char*)(UART_1_BASE + REGOFF_ULSR) = 0X0;
    *(volatile unsigned char*)(UART_1_BASE + REGOFF_UFCR) = 0X07; //fifo mode
    //*(volatile unsigned char*)(UART_1_BASE + REGOFF_UFCR) = 0X00;
    *(volatile unsigned char*)(UART_1_BASE + REGOFF_UMCR) = 0X03;
    //*(volatile unsigned char*)(UART_1_BASE + REGOFF_UMCR) = 0X13;
    //*(volatile unsigned char*)(UART_1_BASE + REGOFF_URPR) = 0X4B;

    return;
}

static int check_rxtx_status(unsigned long iobase, unsigned long txorx)
{
    unsigned char status = 0x00;
    unsigned char c_status = 0x00;
    unsigned long count = 0;

    if(txorx)
    {
        c_status = 0x40; //tx
    }
    else
    {
        c_status = 0x04;  //rx
    }

    while(1)
    {
        if (txorx)
        {
         status = *(volatile unsigned char*)(iobase + REGOFF_ULSR) & 0x40;
        }
        else
        {
         status = *(volatile unsigned char*)(iobase + REGOFF_UIIR) & 0x0f;
        }
        count++;
        if(IIR_PRIOR == status)
        {
        //soc_printf("LSR: %x",*(volatile unsigned char*)(iobase + REGOFF_ULSR));
                SDBBP();
        }
        else if(status == c_status)
        {
            return 0;
        }
        // delay_uS(5); // 100
        // delay_x100nS(10);
    }

    return 1;
}

void sci_16550uart_hight_speed_mode_write(UINT32 id, UINT8 ch)
{
    id = id;
    *(volatile unsigned char*)(UART_1_BASE + REGOFF_UTBR) = ch;
    if(check_rxtx_status(UART_1_BASE, 1))
    {
        //soc_printf("transfer data error\n");
        SDBBP();
    }

    return;
}

UINT8 sci_16550uart_hight_speed_mode_read(UINT32 id)
{
    UINT8 ch = 0;
    id = id;
    if(check_rxtx_status(UART_1_BASE, 0))
    {
        //soc_printf("receive data error\n");
        SDBBP();
    }

    ch = *(volatile unsigned char*)(UART_1_BASE + REGOFF_UTBR);

    return ch;
}
#endif

void sci_16550uart_set_mode(UINT32 id, UINT32 bps, int parity)
{
    unsigned int div = 0;
    UINT32 chip_id = 0;
    UINT32 rev_id = 0;

    if (NULL == sci_16550)
    {
        return;
    }
    osal_mutex_lock(sci_16550[id].wt_mutex_id, OSAL_WAIT_FOREVER_TIME);
    //10000000 / bps; /* Tout = 10*10^6/Rband */
    /*while write first char after mode set, takes 5ms*/
    sci_16550[id].timeout = 20000;
    /* Disable all interrupt */
    SCI_WRITE8(id, SCI_16550_UIER, 0);

    /* Set baud rate and transmit format */
    if (bps > UART_BPS_MAX)
    {
        SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
        div = (bps / 90000) * 1843;
    }
    else if (bps > UART_BPS_115200)
    {
        SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
        div = (bps / 32000) * 1843;
    }
    else
    {
        if ((ALI_M3327C == sys_ic_get_chip_id())&& (sys_ic_get_rev_id() >= IC_REV_3))
        {
            SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) & 0xf7);
        }
    }
    div = 115200 / bps;
    SCI_WRITE8(id, SCI_16550_ULCR, 0x9b);   /* Enable setup baud rate */
    SCI_WRITE8(id, SCI_16550_UDLL, (div & 0xff));
    SCI_WRITE8(id, SCI_16550_UDLM, ((div >> 8) & 0xff));

    div = (((((unsigned int)parity) >> 6) & 0x04) | (~(((unsigned int)parity) >> 4) & 0x03));
    switch (parity & 0x03)
    {
    case SCI_PARITY_EVEN:
        SCI_WRITE8(id, SCI_16550_ULCR, 0x18 | div); /* even parity */ /*? 1b */
        //SCI_WRITE8(id, SCI_16550_ULCR, 0x1b | div);
        /* even parity */ /*? 1b */
        break;
    case SCI_PARITY_ODD:
        SCI_WRITE8(id, SCI_16550_ULCR, 0x08 | div); /* odd parity */
        break;
    default :
        SCI_WRITE8(id, SCI_16550_ULCR, 0x00 | div); /* none parity */
        break;
    };

    /* Enable FIFO, threshold is 4 bytes */
    chip_id = sys_ic_get_chip_id();
    rev_id = sys_ic_get_rev_id();
    if (((ALI_M3327 == chip_id) && (rev_id < IC_REV_4)) || ((ALI_M3327C == chip_id) && (rev_id < IC_REV_1)))
    {
      /* Don't reset FIFO for fix IC bug */
        SCI_WRITE8(id, SCI_16550_UFCR, 0x41);
    }
    else
    {
        SCI_WRITE8(id, SCI_16550_UFCR, 0x47);   /* Reset FIFO */
    }

    SCI_WRITE8(id, SCI_16550_ULSR, 0x00);       /* Reset line status */
    SCI_WRITE8(id, SCI_16550_UMCR, 0x03);       /* Set modem control */

    /* Enable receiver interrupt */
    SCI_WRITE8(id, SCI_16550_UIER, 0x05);  /* Enable RX & timeout interrupt */

    /* signon message or measure TIMEOUT */
    osal_mutex_unlock(sci_16550[id].wt_mutex_id);
}

ComIntrruptCB g_ComIntrruptCB=NULL;
static CHAR s_byComRcvTmp=0;
CHAR drvGetChar4Nocs()
{
	return s_byComRcvTmp;
}
void drvSetChar4Nocs(UINT8 byCharInput)
{
	s_byComRcvTmp=byCharInput;
}

static void sci_16550uart_interrupt(UINT32 id)
{
    UINT8 istatus = 0;
    int count = 0;
    CHAR byChar=0;

    ASSERT(sci_16550 != NULL);
    /* It seems not need read UIIR but LSR for interrupt processing, but need
     * to read UIIR for clear interrupt. If ULSR error ocured, read ULSR and
     * clear it. */
    while (0 == ((istatus = (SCI_READ8(id, SCI_16550_UIIR) & 0x0f)) & 1))
    {
        if(IIR_PRIOR == istatus)
        {
            if (SCI_READ8(id, SCI_16550_ULSR) & 0x9e)
            {
                PRINTF("sci_16550uart_interrupt: lstatus error!\n");
            }
            count = 0;
            while (SCI_READ8(id, SCI_16550_ULSR) & 1)
            {
                /* Patch for M3327 AXX/BXA */
                if ((sys_ic_get_rev_id() < IC_REV_4) && (++count >= SCI_16550UART_RX_BUF_SIZE))
                {
                    PRINTF("sci_16550uart_interrupt: chip error!\n");
                    osal_interrupt_unregister_lsr(sci_16550_reg[id].irq, sci_16550uart_interrupt);
                    return;
                }
                sci_16550[id].rx_buf[sci_16550[id].rx_buf_head++] = SCI_READ8(id, SCI_16550_URBR);
                sci_16550[id].rx_buf_head %= SCI_16550UART_RX_BUF_SIZE;
                /* Patch for M3327 AXA */
                if (IC_REV_0 == sys_ic_get_rev_id())
                {
                    osal_delay(1);
                }
            }
        }

        switch (istatus)
        {
            #if 0
            case 0x06:  /* LSR error: OE, PE, FE, or BI */
                if (SCI_READ8(id, SCI_16550_ULSR) & 0x9e)
                 {
                    PRINTF("sci_16550uart_interrupt: lstatus error!\n");
                 }
            #endif
                /* We continue receive data at this condition */
            case 0x0c:  /* Character Timer-outIndication */
            case 0x04:  /* Received Data Available */
                count = 0;
                while (SCI_READ8(id, SCI_16550_ULSR) & 1)
                {
                    /* Patch for M3327 AXX/BXA */
                    if ((sys_ic_get_rev_id() < IC_REV_4) && (++count >= SCI_16550UART_RX_BUF_SIZE))
                    {
                        PRINTF("sci_16550uart_interrupt: chip error!\n");
                        osal_interrupt_unregister_lsr(sci_16550_reg[id].irq, sci_16550uart_interrupt);
                        return;
                    }
                    
                    byChar=SCI_READ8(id, SCI_16550_URBR);
                    sci_16550[id].rx_buf[sci_16550[id].rx_buf_head++] = byChar;// SCI_READ8(id, SCI_16550_URBR);
                    sci_16550[id].rx_buf_head %= SCI_16550UART_RX_BUF_SIZE;
                    /* Patch for M3327 AXA */
                    if (IC_REV_0 == sys_ic_get_rev_id())
                    {
                        osal_delay(1);
                    }
                    
										#if((defined AUI_TEST) || (defined CNX_1X_CSD_CERT))
										if(NULL!=g_ComIntrruptCB)
										{
											g_ComIntrruptCB(byChar);
										}
										#endif
                }
                break;
            case 0x02:  /* TransmitterHoldingRegister Empty */
            case 0x00:  /* Modem Status */
            default:
                break;
        }
    }

    return;
}

UINT8 sci_16550uart_read(UINT32 id)
{
    UINT8 data = 0;
    UINT8 received = 0;

    if ((NULL == sci_16550) || (id >= (UINT32)g_dev_num))
    {
        return 0;
    }
    ASSERT(sci_16550 != NULL);
    osal_mutex_lock(sci_16550[id].rd_mutex_id, OSAL_WAIT_FOREVER_TIME);

    received = !(sci_16550[id].rx_buf_head == sci_16550[id].rx_buf_tail);
    while (!received)
    {
        osal_mutex_unlock(sci_16550[id].rd_mutex_id);
        osal_task_sleep(1);
        osal_mutex_lock(sci_16550[id].rd_mutex_id, OSAL_WAIT_FOREVER_TIME);
        received = !(sci_16550[id].rx_buf_head == sci_16550[id].rx_buf_tail);
    }

    data = sci_16550[id].rx_buf[sci_16550[id].rx_buf_tail++];
    sci_16550[id].rx_buf_tail %= SCI_16550UART_RX_BUF_SIZE;
    osal_mutex_unlock(sci_16550[id].rd_mutex_id);

    return data;
}

INT32 sci_16550uart_read_tm(UINT32 id, UINT8 *data, UINT32 timeout)
{
    if (NULL == data)
    {
        return ERR_PARA;
    }
    if ((NULL == sci_16550) || (id >= (UINT32)g_dev_num))
    {
        return RET_FAILURE;
    }
    ASSERT(sci_16550 != NULL);
    osal_mutex_lock(sci_16550[id].rd_mutex_id, OSAL_WAIT_FOREVER_TIME);
    while(1)
    {
        if (sci_16550[id].rx_buf_head == sci_16550[id].rx_buf_tail)
        {
            if (timeout <= 0)
            {
                osal_mutex_unlock(sci_16550[id].rd_mutex_id);
                return ERR_FAILUE;
            }
            osal_delay(1);
            timeout--;
        }
        else
        {
            break;
        }
    }

    *data = sci_16550[id].rx_buf[sci_16550[id].rx_buf_tail++];
    sci_16550[id].rx_buf_tail %= SCI_16550UART_RX_BUF_SIZE;
    osal_mutex_unlock(sci_16550[id].rd_mutex_id);

    return SUCCESS;
}

void sci_16550uart_loopback(UINT32 id, UINT8 mode)
{
    if (NULL == sci_16550)
    {
        return;
    }
    ASSERT(sci_16550 != NULL);
    sci_16550[id].loopback_flag = mode;
}

void sci_16550uart_write(UINT32 id, UINT8 ch)
{
    int i = 0;
    int retry = 3;

    if ((NULL == sci_16550) || (id >= (UINT32)g_dev_num))
    {
        return;
    }

    ASSERT(sci_16550 != NULL);
    if (sci_16550[id].loopback_flag)
    {
        osal_interrupt_disable();
        sci_16550[id].rx_buf[sci_16550[id].rx_buf_head++] = ch;
        sci_16550[id].rx_buf_head %= SCI_16550UART_RX_BUF_SIZE;
        osal_interrupt_enable();
        return;
    }

    //osal_mutex_lock(sci_16550[id].wt_mutex_id, OSAL_WAIT_FOREVER_TIME);
    uart_write_lock(sci_16550[id].wt_mutex_id, OSAL_WAIT_FOREVER_TIME);
    while (retry)
    {
        /* Send character. */
        SCI_WRITE8(id, SCI_16550_UTBR, ch);
        /* wait for transmission finished */
        i = sci_16550[id].timeout;
        while (--i)
        {
            osal_delay(1);
            if (SCI_READ8(id, SCI_16550_ULSR) & 0x20)
            {
                #if (SYS_CHIP_MODULE == ALI_S3602)
                    // When CPU is too fast, UART output will
            // have repeated character,
                    // Add a delay is a temporary solution,
            // wait for IC team check it.
                    osal_delay(10);
                #endif
                break;
            }
        }
        if (0 != i)
        {
            break;
        }

        /* Timeout, reset XMIT FIFO */
        SCI_WRITE8(id, SCI_16550_UFCR, SCI_READ8(id, SCI_16550_UFCR) | 0x04);
        osal_delay(10);
        retry--;
    }

    //osal_mutex_unlock(sci_16550[id].wt_mutex_id);
    uart_write_unlock(sci_16550[id].wt_mutex_id);
    return;
}

INT32 sci_16550uart_fifowrite(UINT32 id, UINT8 *buf,int len)
{
    //null to avoid compile error in shuttle time
    id = id;
    buf = buf;
    len = len;
    return 0;
}
void sci_16550uart_clear_buff(UINT32 id)
{
    if (NULL == sci_16550)
    {
        return;
    }
    ASSERT(sci_16550 != NULL);
    sci_16550[id].rx_buf_head = 0;
    sci_16550[id].rx_buf_tail = 0;
}
