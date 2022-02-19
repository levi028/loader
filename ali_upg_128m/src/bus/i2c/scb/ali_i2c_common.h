#ifndef __ALI_I2C_COMMON_H__
#define __ALI_I2C_COMMON_H__

#if !defined (__KERNEL__) && !defined(__UBOOT__)
#define ALI_TDS
#define ALI_SEE // distinguish ALISEE from TDS

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#elif defined (__UBOOT__)
#define ALI_UBOOT    
#else
#define ALI_KERNEL
#endif

//#define DEBUG // switch for Linux dev_dbg, should in front #include <linux/platform_device.h>
//#define I2C_DBG_ENABLE

/*
 * Register offsets in bytes from register base.
 */
/* Host Control Register			*/
#define I2C_HCR_REG_OFFSET				(0x00)
/* Host Status Register				*/
#define I2C_HSR_REG_OFFSET				(0x01)
/* Interrupt Enable Register			*/
#define I2C_IER_REG_OFFSET				(0x02)
/* Interrupt Status Register			*/
#define I2C_ISR_REG_OFFSET				(0x03)
/* Slave Address Register			*/
#define I2C_SAR_REG_OFFSET				(0x04)
/* Target serial device address			*/
#define I2C_SSAR_REG_OFFSET				(0x05)
/* High Period Clock Counter Register		*/
#define I2C_HPCC_REG_OFFSET				(0x06)
/* Low Period Clock Counter Register		*/
#define I2C_LPCC_REG_OFFSET				(0x07)
/* Stop Setup Time Register			*/
#define I2C_PSUR_REG_OFFSET				(0x08)
/* Stop Hold Time Register			*/
#define I2C_PHDR_REG_OFFSET				(0x09)
/* Restart Setup Time Register			*/
#define I2C_RSUR_REG_OFFSET				(0x0A)
/* Start Hold Time Register			*/
#define I2C_SHDR_REG_OFFSET				(0x0B)
/* FIFO Control Register			*/
#define I2C_FCR_REG_OFFSET				(0x0C)
/* Device Control Register			*/
#define I2C_DCR_REG_OFFSET				(0x0D)
/* E-DDC Address  Register			*/
#define I2C_DAR_REG_OFFSET				(0x0E)
/* SEG_POINTER Address  Register		*/
#define I2C_SPR_REG_OFFSET				(0x0F)
/* FIFO Data(Tx/Rx) Register			*/
#define I2C_FDR_REG_OFFSET				(0x10)
/* Slave Address(SA) Got Register		*/
#define I2C_AGR_REG_OFFSET				(0x11)
/* Second Slave Address BYTE (SA) Got Register	*/
#define I2C_SGR_REG_OFFSET				(0x12)
/* Slave Device State Register			*/
#define I2C_SSR_REG_OFFSET				(0x13)
/* FIFO Trigger Interrupt Enable Register	*/
#define I2C_IER1_REG_OFFSET				(0x20)
/* FIFO Trigger Interrupt Status Register	*/
#define I2C_ISR1_REG_OFFSET				(0x21)
/* FIFO Trigger Level Register				*/
#define I2C_FTLR_REG_OFFSET				(0x22)
/* Byte Number Expect Transfer One Time Register*/
#define I2C_BCR_REG_OFFSET				(0x23)
/* Enable x Sub-slave Address Byte Register	*/
#define I2C_SSAR_EN_REG_OFFSET				(0x24)
/* The 2nd Sub-slave Address Byte Register	*/
#define I2C_SSAR2_REG_OFFSET				(0x25)
/* The 3rd Sub-slave Address Byte Register	*/
#define I2C_SSAR3_REG_OFFSET				(0x26)
/* The 4th Sub-slave Address Byte Register	*/
#define I2C_SSAR4_REG_OFFSET				(0x27)

/*
 *Control Register masks
 */
/* Master Start Transaction Bit			*/
#define I2C_HCR_START_TRANSACTION_MASK			BIT(0)
/* Command Protocol Bit				*/
#define I2C_HCR_COMMAND_PROTOCOL_MASK			(BIT(1)|BIT(2)|BIT(3))
/* Repeated start				*/
#define I2C_HCR_REPEATED_START_MASK			BIT(4)
/* EDDC_OPERATION				*/
#define I2C_HCR_EDDC_OPERATION_MASK			BIT(5)
/* Device Not Exist Enable/Disable		*/
#define I2C_HCR_DEVICE_NOT_EXIST_MASK			BIT(6)
/* Host Controller Enable/Disable		*/
#define I2C_HCR_HOST_CONCTOLLER_MASK			BIT(7)
/* 1.843M/12M clock for I2C interface Bit*/
#define I2C_DCR_CLOCK_MASK				BIT(3)
/* Read Request Detect Enable/Disable Mask	*/
#define I2C_DCR_READ_REQ_DETECT_MASK			BIT(5)
/* FIFO Flush Bit				*/
#define I2C_FCR_FIFO_MASK				BIT(7)
/* Byte Count (BC) Bit				*/
#define I2C_FCR_BC_MASK					0x1F
/* FIFO Bit when the data are read from the FIFO*/
#define I2C_HSR_RFIFO_MASK				BIT(0)
/* FIFO Bit when the data are written from the FIFO*/
#define I2C_HSR_WFIFO_MASK				BIT(1)
/* Host Busy Bit				*/
#define I2C_HSR_HB_MASK					BIT(5)
/* Data for Cacluating Period Counter When Using 12M Clock */
#define I2C_12M_CLOCK_CONSTANT				6000000
/* Data for SCL Rising Time for 1K of Pull High */
#define I2C_SCL_RT_CONSTANT				2

/* FIFO_EMPTY Interrupt Enable/Disable Bit	*/
#define I2C_IER_FEIE_MASK				BIT(7)
/* FIFO_FULL Interrupt Enable/Disable Bit	*/
#define I2C_IER_FFIE_MASK				BIT(6)
/* S_P Interrupt Enable/Disable Bit		*/
#define I2C_IER_SPIE_MASK				BIT(5)
/* SLAVE_SELECTED Interrupt Enable/Disable Bit	*/
#define I2C_IER_SSIE_MASK				BIT(4)
/* Arbiter Lost Interrupt Enable/Disable Bit*/
#define I2C_IER_ARBLIE_MASK				BIT(3)
/* Device Busy Interrupt Enable/Disable Bit	*/
#define I2C_IER_DBIE_MASK				BIT(2)
/* Device Not Exist Interrupt Enable/Disable Bit*/
#define I2C_IER_DNEE_MASK				BIT(1)
/* Transaction Done Interrupt Enable/Disable Bit*/
#define I2C_IER_TDIE_MASK				BIT(0)
/* Read Request Enable/Disable Bit	      	*/
#define I2C_IER1_READ_REQ_MASK				BIT(1)
/* FIFO Trigger Interrupt Enable/Disable Bit	*/
#define I2C_IER1_TRIGGER_MASK				BIT(0)
/* Enable x sub-slave address byte Mask		*/
#define I2C_SSAR_EN_REG_MASK				(BIT(0)|BIT(1))

/* Write Operation				*/
#define I2C_HCR_WT					0x00
/* Current Address Read Operation		*/
#define I2C_HCR_CAR					0x04
/* Sequential Read Operation			*/
#define I2C_HCR_SER					0x08
/* Standard Read Operation			*/
#define I2C_HCR_STR					0x0c

/* The following constants specify the depth of the FIFOs */
#define I2C_FIFO_DEPTH					16
/* Rx fifo trigger level			*/
#define I2C_FIFO_RX_TRIG_LVL				(I2C_FIFO_DEPTH / 2)
/* Tx fifo trigger level			*/
#define I2C_FIFO_TX_TRIG_LVL				(I2C_FIFO_DEPTH / 2)

#define I2C_READ_OPERATION				1
#define I2C_WRITE_OPERATION				0

#define DRIVER_NAME					"ali-i2c"

#ifdef ALI_TDS
#include <api/libc/printf.h>

#ifdef I2C_DBG_ENABLE
#define I2C_DBG(format, args...) libc_printf("[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)
#else
#define I2C_DBG(format, args...)
#endif
#define I2C_INFO(format, args...) libc_printf("[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)
#define I2C_ERR(format, args...) libc_printf("[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)

#define __iomem
typedef UINT32 MUTEX;
typedef	UINT32 COMPLETION;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct i2c_msg {
	u16 addr;	/* slave address			*/
	u16 flags;
#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
#define I2C_M_RD		0x0001	/* read data, from slave to master */
#define I2C_M_STOP		0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
	u16 len;		/* msg length				*/
	u8 *buf;		/* pointer to msg data			*/
};

#elif defined (ALI_UBOOT)
#include <linux/kernel.h>
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/types.h>
#include <asm/io.h>// ioread/iowrite
#include <i2c.h>
#include <linux/compat.h>

#ifdef I2C_DBG_ENABLE
#define I2C_DBG(format, args...) printf("[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)
#else
#define I2C_DBG(format, args...)
#endif
#define I2C_INFO(format, args...) printf("[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)
#define I2C_ERR(format, args...) printf("[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)

typedef int MUTEX;
typedef	int COMPLETION;

#else

#include <linux/kernel.h>
#include <linux/i2c.h>

#ifdef I2C_DBG_ENABLE
#define I2C_DBG(format, args...) dev_dbg(i2c->dev, "[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)
#else
#define I2C_DBG(format, args...)
#endif
#define I2C_INFO(format, args...) dev_info(i2c->dev, "[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)
#define I2C_ERR(format, args...) dev_err(i2c->dev, "[%s:%d]\t"format, __FUNCTION__, __LINE__, ##args)

typedef struct mutex MUTEX;
typedef	struct completion COMPLETION;

#endif

struct ali_i2c_xfer_result {
	int err;
	COMPLETION completion;
};

struct ali_i2c {
#ifdef ALI_KERNEL
	struct device *dev;
	struct i2c_adapter adap;
#endif
	MUTEX mutex;

#ifdef ALI_TDS
	UINT32 base;
#else
	void __iomem *base;
#endif
	int irq;
	u32 bus_clk_rate;
	u32 one_byte_time_us; // in microseconds
	int enable_int;

	struct ali_i2c_xfer_result xfer_result;
	struct i2c_msg *msgs; // current msgs to xfer.
	unsigned int nmsgs;
	u16 xfer_len;

	u8 ddc_addr;
	u8 ddc_seg_ptr;
	int is_ddc_read;
	int is_std_read; // use STD read command protocol instead of SER(default)
	int is_standalone; // indicate that each `i2c_msg` is standalone, include the case of `nmsgs` == 1

	void (*read_req_cb)(void *read_req_cb_data);
	void *read_req_cb_data;

#ifdef CONFIG_ALI_STANDBY_TO_RAM
	void *ram_cache_regs;// registers content cache in RAM
#endif
};

void __ali_i2c_init(struct ali_i2c *i2c);
void __ali_i2c_deinit(struct ali_i2c *i2c);
int  __ali_i2c_start_xfer(struct ali_i2c *i2c);
void __ali_i2c_isr(struct ali_i2c *i2c);

void __ali_i2c_lock(struct ali_i2c *i2c);
void __ali_i2c_unlock(struct ali_i2c *i2c);

int  __ali_i2c_read_req_register_cb(struct ali_i2c *i2c, void (*read_req_cb)(void *read_req_cb_data), void *read_req_cb_data);
int  __ali_i2c_read_req_unregister_cb(struct ali_i2c *i2c);

void ali_board_i2c_init(const void *blob);

#endif//__ALI_I2C_COMMON_H__
