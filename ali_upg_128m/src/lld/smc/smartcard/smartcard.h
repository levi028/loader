/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard.h
*
*    Description: Head file of UART smart card reader
*
*    History:
*      Date          Author          Version         Comment
*      ====          ======          =======         =======
*    0.              Victor Chen       Ref. code
*    1. 2005.9.8     Gushun Chen       0.1.000         Initial
*    2. 2006.12.5    Victor Chen       0.2.000         for s3602
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  __SMARTCARD_H__
#define  __SMARTCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hld/smc/smc.h>

#include "smartcard_atr.h"
#include "smartcard_t1.h"

//#define SMC_RW_DEBUG
//#define DEBUG_SMC
//#define DEBUG_SMC_T1

#ifdef SMC_RW_DEBUG
#define SMC_RW_PRINTF    libc_printf
#else
#define SMC_RW_PRINTF(...)  do{}while(0)
#endif

#ifdef DEBUG_SMC
#define SMC_PRINTF    libc_printf
//#define SMC_PRINTF    soc_printf
#define SMC_DUMP(data,len) { const int l=(len);\
							 	int i=0;\
                         for(i=0 ; i<l ; i++) SMC_PRINTF(" %02x",*((data)+i)); \
                         SMC_PRINTF("\n"); }
#else
#define SMC_PRINTF(...)     do{}while(0)
#define SMC_DUMP(data,len) ;
#endif

#ifdef DEBUG_SMC_T1
#define T1PRINTF libc_printf
#else
#define T1PRINTF(...)        do{}while(0);
#endif
#define REG_SCR_CTRL                0x00
#define REG_ICCR                    0x01
#define REG_FIFO_CTRL                0x02
#define REG_CLK_SEL                    0x03
#define REG_IER0                    0x04
#define REG_IER1                    0x05
#define REG_ISR0                    0x06
#define REG_ISR1                    0x07
#define REG_ICCSR                    0x08
#define REG_PDBR                    0x09
#define REG_RBR                        0x0A
#define REG_THR                        0x0B
#define REG_ETU0                    0x0C
#define REG_ETU1                    0x0D
#define REG_GTR0                    0x0E
#define REG_GTR1                    0x0F
#define REG_CBWTR0                    0x10
#define REG_CBWTR1                    0x11
#define REG_CBWTR2                    0x12
#define REG_CBWTR3                    0x13
#define REG_RFIFO_CNT                0x14
#define REG_RCVPR                    0x15
#define REG_RCNT_ETU                0x16
#define REG_PIN_VALUE                0x18
#define REG_RXTX_PP                    0x19
#define REG_GT_CNT                    0x1a
#define REG_WT_CNT                    0x1c
#define REG_FSM_STATE                0x20
#define REG_COUNT_DLY                0x22
#define REG_CLKL_SEL                0x24
#define REG_CLKH_SEH                0x26
#define REG_RCNT_3ETU                0x28
#define REG_TX_CNT                    0x2a
#define REG_RX_CNT8                    0x2c
#define REG_CLK_FRAC                0x2d
#define REG_DEV_CTRL                0x2e
#define REG_CLK_VPP                    0x2f
#define REG_VPP_GPIO                0x30

#define REG_PWML                    0x00
#define REG_PWMH                    0x02
#define REG_PWM_CONFIG                0x04
#define REG_PWM_FRAC_DIVISION        0x05

#define SMC_RB_ICCR_PRT_EN            0x30
#define SMC_RB_ICCR_CLK                0x04
#define SMC_RB_ICCR_RST                0x02
#define SMC_RB_ICCR_DIO                0x08
#define SMC_RB_ICCR_VCC                0x01
#define SMC_RB_ICCR_OP                0x0e
#define SMC_RB_ICCR_AUTO_PRT        0x10
#define SMC_RB_CTRL_VPP                0x04

#define SMC_SCR_CTRL_OP                0xe0
#define SMC_SCR_CTRL_INVESE            0x08
#define SMC_SCR_CTRL_TRANS            0x04
#define SMC_SCR_CTRL_RECV            0x02

#define SMC_ISR0_FIFO_EMPTY            0x80
#define SMC_ISR0_FIFO_TRANS            0x40
#define SMC_ISR0_FIFO_RECV            0x20
#define SMC_ISR0_TIMEOUT            0x10
#define SMC_ISR0_BYTE_RECV            0x04
#define SMC_ISR0_PE_RECV            0x01
#define SMC_ISR0_PE_TRANS            0x02

#define SMC_FIFO_CTRL_EN            0x80
#define SMC_FIFO_CTRL_TX_OP            0x40
#define SMC_FIFO_CTRL_RX_OP            0x20
#define SMC_FIFO_CTRL_ENABLE        0x80
#define SMC_FIFO_CTRL_TX_INIT        0x40
#define SMC_FIFO_CTRL_RX_INIT        0x20


#define SMC_ISR1_RST_HIGH            0x01
#define SMC_ISR1_RST_LOW            0x02
#define SMC_ISR1_RST_NATR            0x08
#define SMC_ISR1_CARD_INSERT        0x10
#define SMC_ISR1_CARD_REMOVE        0x20
#define SMC_ISR1_COUNT_ST            0x40

#define SMC_IER0_BYTE_RECV_TRIG        0x04
#define SMC_IER0_BYTE_TRANS_TRIG    0x08
#define SMC_IER0_RECV_FIFO_TRIG        0x20
#define SMC_IER0_TRANS_FIFO_TRIG    0x40
#define SMC_IER0_TRANS_FIFO_EMPY    0x80

#define SMC_TX_BUF_SIZE             256
#define SMC_RX_BUF_SIZE             512

#define SMC_RX_FIFO_SIZE_64         64
#define SMC_RESET_L_INTERVAL        60 // t >= 40000/f = 11.2ms, tongfang card need t < 500ms
#define WAITTIMEOUT                    100000

#define FORCE_TX_RX_THLD            2

/* Input / output micro for IO space */
#define INPUT_UINT8(addr)        (*(volatile UINT8  *)(addr))
#define INPUT_UINT16(addr)        (*(volatile UINT16 *)(addr))
#define INPUT_UINT32(addr)        (*(volatile UINT32 *)(addr))
#define OUTPUT_UINT8(addr,val)    ((*(volatile UINT8  *)(addr)) = (val))
#define OUTPUT_UINT16(addr,val)    ((*(volatile UINT16 *)(addr)) = (val))
#define OUTPUT_UINT32(addr,val)    ((*(volatile UINT32 *)(addr)) = (val))

#define WRITE_RX_CNT(ioaddr, val)                    \
    do                                                \
    {                                                \
        OUTPUT_UINT8(ioaddr+REG_RFIFO_CNT, val);    \
        OUTPUT_UINT8(ioaddr+REG_RX_CNT8,             \
        (INPUT_UINT8(ioaddr+REG_RX_CNT8)&0xfe)|(((val)&0x100)>>8));    \
    }                                                \
    while(0)
#define READ_RX_CNT(ioaddr)        (INPUT_UINT8(ioaddr+REG_RFIFO_CNT)|((INPUT_UINT8(ioaddr+REG_RX_CNT8)&0x1)<<8))

#define min( a, b )   ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

/*The default setting is for M3602 serial*/
struct struct_smc_dev
{
    UINT32 io_base;
    int    irq;
    struct smc_device *dev;
    UINT32 pwm_addr;
    UINT8 pwm_sel_ofst;
    UINT8 pwm_seh_ofst;
    UINT8 pwm_gpio_ofst;
    UINT8 pwm_cfg_ofst;
    UINT8 pwm_frac_ofst;
};

extern struct struct_smc_dev smc_dev_set[];


/////////////////////////////////////////////////////////////////
//activation & deactivation timming: refer to ST8024
////////////////////////////////////////////////////////////////
#define BASIC_T            (26<<6)
#define ATV_VCC2IO        ((BASIC_T*5UL)>>1)
#define ATV_IO2CLK        (5)
#define DATV_RST2CLK    (BASIC_T>>1)
#define DATV_CLK2IO        (BASIC_T>>1)
#define DATV_IO2VCC        (BASIC_T>>1)

#define DFT_WORK_CLK    3600000//3579545
#define DFT_WORK_ETU    372

enum
{
    TRANSMIT_MODE = 0,
    RECEIVE_MODE,
};

#ifdef SMALL_FIFO_3602
#define TX_FIFO_SIZE    8
#define RX_FIFO_SIZE    32
#else
#if (SYS_CHIP_MODULE == ALI_S3602)
#define TX_FIFO_SIZE    8
#define RX_FIFO_SIZE    32
#else
#define TX_FIFO_SIZE    64//32//8
#define RX_FIFO_SIZE    64//32//32
#endif
#endif

#define FIRST_CWT_VAL    1000    //ms
#define CWT_VAL            300     //ms

struct smartcard_private
{
    OSAL_ID    smc_sema_id;
    OSAL_ID smc_flag_id;
    volatile UINT8 inserted;    //smart card inserted or not,    1 inserted
    volatile UINT8 reseted;        //smart card has been reset or not,    1 reseted
    UINT8 inverse_convention;   //smart card is using direct or inverse convention, 1 inverse 0 direct
//    UINT8 parity;                // 0 none parity, 1 even parity, 2 odd parity
//    UINT8 reset_on_high;        // 1 reset on high, 0 reset on low
//    UINT8 card_name[32];

    UINT8 atr[ATR_MAX_SIZE];
    UINT16 atr_size;

    atr_t *atr_info;
    enum smc_atr_result atr_rlt;
    UINT32 F;
    UINT32 D;
    UINT32 I;
    UINT32 P;
    UINT8 N;
    UINT8 WI;
    UINT8 T;
    UINT8 CWI;
    UINT8 BWI;
    UINT8 error_check_type;
    UINT8 ta2_spec;        //for TA(2) bit 5, indicate if to use the default F/D

    volatile UINT8 isr0_interrupt_status;
    volatile UINT8 isr1_interrupt_status;
    volatile UINT8 trans_trigger;
    UINT32 smc_clock;
    UINT32 smc_etu;
	UINT8 reset_etu_flag;             /*this flag use for reset etu if the default etu is not support*/
//    UINT32 uart_baudrate;
//    UINT32 guard_time;    //extra guard time
    UINT32 first_cwt;
    UINT32 cwt;
    volatile UINT8 *smc_tx_buf;
    volatile UINT32 smc_tx_rd;//real transfer data length
    volatile UINT32 smc_tx_wr;//need transfer data length
    volatile UINT8 *smc_rx_buf;//
    volatile UINT8 *smc_rx_tmp_buf;//malloc in open
    volatile UINT32 smc_rx_head;//have received data lenth
    volatile UINT32 smc_rx_tail;//need received data length
    UINT32 got_first_byte :1;
    UINT32 use_default_etu:1;
    UINT32 use_gpio_cd:1;
    UINT32 parity_disable: 1;
    UINT32 parity_odd: 1;
    UINT32 apd_disable: 1;
    UINT32 warm_reset_enable: 1;
    UINT32 use_gpio_vpp:1;
    UINT32 disable_pps:1;
    UINT32 ts_auto_detect:1;
    UINT32 internal_ctrl_vpp:1;
    UINT32 invert_power: 1;
    UINT32 invert_detect: 1;
    UINT32 class_selection_supported: 1;
    UINT32 board_supported_class: 6;
    UINT32 smc_supported_class: 6;
    UINT32 reserved : 6;
    UINT32 *init_clk_array;
    UINT32 init_clk_number;
    UINT32 init_clk_idx;
    UINT32 default_etu;
    void (*hsr)(UINT32);
    INT16 gpio_cd_pol:1;        /* Polarity of GPIO, 0 or 1 active */
    INT16 gpio_cd_io:1;        /* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
    INT16 gpio_cd_pos: 14;
    INT16 gpio_vpp_pol:1;        /* Polarity of GPIO, 0 or 1 active */
    INT16 gpio_vpp_io:1;        /* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
    INT16 gpio_vpp_pos: 14;
    UINT8  force_tx_rx_triger;
    UINT8  auto_tx_rx_triger;
    UINT8  force_tx_rx_thld;
    UINT8  force_tx_rx_state;
    UINT32 char_frm_dura;        /*In unit of 1/1000000 second*/
    UINT32 warm_reset;
    UINT32 the_last_send;        /*for 3602*/
    UINT8   ext_atr_delay;	/* the ext delay time in ms to get atr, default is 40000 clock */
    void     (*class_select)(enum class_selection );
    enum class_selection smc_current_select;
    t1_state_t          T1;

    /* Those parameters are bound IC */
    UINT32 scr_sys_clk;
    UINT32 pwm_sys_clk;
    UINT32 smc_chip_id;
    UINT32 smc_chip_version;
    UINT16 smc_tx_fifo_size;
    UINT16 smc_rx_fifo_size;
};

enum smc_error_e
{
    SMART_NO_ERROR,
    SMART_WRONG_ATR,
    SMART_TOO_MANY_RETRIES,
    SMART_OVERRUN_ERROR,
    SMART_FRAME_ERROR,
    SMART_PTS_NOT_SUPPORTED,
    SMART_INVALID_STATUS_BYTES_SEQUENCE,
    SMART_INVALID_CLASS,
    SMART_INVALID_CODE,
    SMART_INCORRECT_REFERENCE,
    SMART_INCORRECT_LENGTH,
    SMART_UNKNOWN_ERROR,
    SMART_NOT_INSERTED,
    SMART_NOT_RESETED,
    SMART_INVALID_PROTOCOL,
    SMART_USER_ABORT,
    SMART_BAD_COMMAND,
    SMART_WRITE_ERROR ,
    SMART_READ_ERROR,
    SMART_NO_ANSWER,
    SMART_PARITY_ERROR,
};
typedef enum smc_error_e smc_error_t;

enum    SMC_STATE_REPORT
{
    SMC_INSERTED = 0x1,
    SMC_REMOVED = 0x2,
    SMC_TX_FINISHED = 0x4,
    SMC_RX_FINISHED = 0x8,
    SMC_RX_BYTE_RCV = 0x10
};

extern void smc_init_hw(struct smc_device *);
extern void smc_set_wclk(struct smartcard_private *, UINT32 , UINT32 );

//20151021
extern void smc_set_protocol_config_T_0(struct smartcard_private *tp);
extern void smc_set_protocol_config_T_1(struct smartcard_private *tp);
//20151021

#ifdef __cplusplus
}
#endif
#endif /*__SMC_UART_H__*/
