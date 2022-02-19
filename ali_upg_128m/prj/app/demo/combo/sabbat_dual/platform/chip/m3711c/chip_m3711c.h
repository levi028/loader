#ifndef _CHIP_S3503_H_
#define _CHIP_S3503_H_

#define GPIO_REG                               0xb8000430  //gpio0 ~ gpio31
#define GPIOA_REG                             0xb8000434  //gpio32 ~ gpio63
#define GPIOB_REG                             0xb8000438  //gpio64 ~ gpio95
#define GPIOC_REG                             0xb800043c  //gpio96 ~ gpio127
#define GPIOD_REG                             0xb8000440  //gpio128 ~ gpio136

#define PIM_MUX_BASE                      0xb8000000
#define PIN_MUX_REG_MASK               0xfff00000
#define PIN_MUX_REG_SHIFT              20
#define PIN_BIT_HEIGHT_MASK           0x0000ff00
#define PIN_BIT_LOW_MASK               0x000000ff
#define PIN_BIT_HEIGHT_SHIFT          8
#define PIN_BIT_LOW_SHIFT               0

#define PIN_MUX_CTRL                       (0x488 << PIN_MUX_REG_SHIFT)
#define PIN_MUX_CTRL1                     (0x48C << PIN_MUX_REG_SHIFT)
    
// Reg addr | reserved | end bit:bit_num<<8(bit8~15) | start bit:bit_num<<0(bit0~7)
#define XPMU_DOCD_SEL				(PIN_MUX_CTRL | 31)
#define XPMU_UART_SEL				(PIN_MUX_CTRL | 29)
#define XSSI_OUT_SEL2				(PIN_MUX_CTRL | 28)
#define XSC_SEL2				      (PIN_MUX_CTRL | 25)
#define HDMI_DEBUG_SEL			      (PIN_MUX_CTRL | 24)
#define XDISEQC_IN_SEL2			      (PIN_MUX_CTRL | 23)
#define VOUT_SEL					(PIN_MUX_CTRL | 22)
#define XI2C3_SEL				      (PIN_MUX_CTRL | 21)
#define XI2C_SEL				             (PIN_MUX_CTRL | 20)
#define XIF_AGC_S2_SEL				(PIN_MUX_CTRL | 19)
//#define PK256_CA2_SEL2				(PIN_MUX_CTRL | 18)//Reserved
#define XUART_TX_SEL				(PIN_MUX_CTRL | 17)
#define XUART_RX_SEL				(PIN_MUX_CTRL | 16)
#define XUSB_RTD_SEL				(PIN_MUX_CTRL | 15)
#define XSF_SEL			                   (PIN_MUX_CTRL | 14)
#define XDISEQC_HV_SEL			      (PIN_MUX_CTRL | 13)
#define XDISEQC_OUT_SEL			(PIN_MUX_CTRL | 12)
#define XDISEQC_IN_SEL                      (PIN_MUX_CTRL | 11)
#define XI2C3_SEL2				      (PIN_MUX_CTRL | 10)
#define XSSI_OUT_SEL1				(PIN_MUX_CTRL | 9)
#define ASSI_SEL2			             (PIN_MUX_CTRL | 8)
#define XASSI_DATA1_SEL2			(PIN_MUX_CTRL | 7)
#define XASSI_DATA1_SEL1			(PIN_MUX_CTRL | 6)
#define XUART2_TX_SEL				(PIN_MUX_CTRL | 5)
#define XI2C2_SEL					(PIN_MUX_CTRL | 4)
#define XHDMI_HTPG_SEL				(PIN_MUX_CTRL | 3)
#define XSPDIF_SEL				      (PIN_MUX_CTRL | 2)
#define XSC_SEL				             (PIN_MUX_CTRL | 1)
#define ASSI_SEL				             (PIN_MUX_CTRL | 0)

//#define PKBGA_CI_SEL2				(PIN_MUX_CTRL1 | 31)//Reserved
//#define PK144_SQI_SEL				(PIN_MUX_CTRL1 | 30)//Reserved
//#define PK144_NFCEJ1_SEL			(PIN_MUX_CTRL1 | 29)//Reserved
//#define PK256_NFCEJ1_SEL			(PIN_MUX_CTRL1 | 28)//Reserved
//#define PK256_SPI_SEL2				(PIN_MUX_CTRL1 | 27)//Reserved
//#define REVERSE_RMII_SEL			(PIN_MUX_CTRL1 | 26)//Reserved
//#define QAM_TUN_AGC_SEL			(PIN_MUX_CTRL1 | 25)//Reserved
//#define QAM_IF_AGC_SEL				(PIN_MUX_CTRL1 | 24)//Reserved
//#define PKBGA_MMC_SEL				(PIN_MUX_CTRL1 | 23)//Reserved
//#define MCU_DEBUG_SYS_EN			(PIN_MUX_CTRL1 | 22)//Reserved
//#define NANDFLASH_CHIP_ENABLE_SEL	(PIN_MUX_CTRL1 | 21)//Reserved
#define XTUN_AGC_QAM_SEL			(PIN_MUX_CTRL1 | 20)//Reserved
#define XIF_AGC_QAM_SEL			(PIN_MUX_CTRL1 | 19)//Reserved
#define SRGB_PUB_SEL				(PIN_MUX_CTRL1 | 18)//Reserved
#define SRGB_SEL2					(PIN_MUX_CTRL1 | 17)//Reserved
#define SRGB_SEL1		                   (PIN_MUX_CTRL1 | 16)//Reserved
#define XDISEQC_HV_SEL2			(PIN_MUX_CTRL1 | 15)//Reserved
#define XDISEQC_OUT_SEL2			(PIN_MUX_CTRL1 | 14)//Reserved
#define XPRGB_QFN_SEL		             (PIN_MUX_CTRL1 | 13)//Reserved
#define XSC_SEL4				      (PIN_MUX_CTRL1 | 12)//Reserved
#define XSC_SEL5				      (PIN_MUX_CTRL1 | 11)//Reserved
#define XPRGB888_SEL				(PIN_MUX_CTRL1 | 10)//Reserved
#define XSC_SEL3					(PIN_MUX_CTRL1 | 9)//Reserved
#define SSI_OUT_SEL3				(PIN_MUX_CTRL1 | 8)//Reserved
#define SPI_MIERR_SEL				(PIN_MUX_CTRL1 | 7)//Reserved
#define ASSI_MIERR_SEL4				(PIN_MUX_CTRL1 | 6)//Reserved
#define ASSI_DATA1_SEL4			      (PIN_MUX_CTRL1 | 5)//Reserved
#define ASSI_SEL4                               (PIN_MUX_CTRL1 | 4)
#define SPI_SEL			                   (PIN_MUX_CTRL1 | 3)
#define ASSI_MIERR_SEL3			       (PIN_MUX_CTRL1 | 2)
#define ASSI_DATA1_SEL3				(PIN_MUX_CTRL1 | 1)
#define ASSI_SEL3			             (PIN_MUX_CTRL1 | 0)

#endif
