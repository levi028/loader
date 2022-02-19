/*****************************************************************************
*Ali Corp. All Rights Reserved. 2002 Copyright (C)
*File: standby_gpio.h
*Description:    This file contains all functions about GPIO operations.
*History:
*                Date                 Athor              Version          Reason
*        ============    =============    =========    =================
*1.    2017/12/14       Chuhua.Tang       Ver 0.1        Create file.
*****************************************************************************/

#ifndef __STANDBY_GPIO_H__
#define __STANDBY_GPIO_H__
#include <osal/osal.h>

#if defined(_M3702_)
#define CHIP_C3702
#endif

#define STANDBY_GPIO_NUM_INVALID   (-100)

/*Add for pinmux setting.*/
#define PMX_IO_BASE (0xB8080000)
#define PMX_GPIO_LA_BASE (0xB808B000)
#define PMX_GPIO_BA_BASE (0xB808B400)
#define PMX_GPIO_BB_BASE (0xB808B480)
#define PMX_GPIO_BC_BASE (0xB808B500)
#define PMX_GPIO_RA_BASE (0xB808B800)
#define PMX_GPIO_TA_BASE (0xB808BC00)
#define PMX_GPIO_TB_BASE (0xB808BC80)

/*Pinmux register low 4 bits.*/
#define PMX_PIN_SEL_GPIO (0x0000)
#define PMX_PIN_SEL_I2C (0x0001)
#define PMX_PIN_SEL_RMII_TXD (0x0001)
#define PMX_PIN_SEL_RMII_TX_EN (0x0001)

#define SETING_ADDR (0x48)
#define DIG0_ADDR (0x68)
#define DIG1_ADDR (0x6A)
#define DIG2_ADDR (0x6C)
#define DIG3_ADDR (0x6E)
#define KEY_ADDR (0x4F)
#define CH455_MODE (0x1)
#define STANDBY_EXTERNAL_PULL_HIGH (1)
#define STANDBY_ERR_I2C_SCL_LOCK (1)
#define STANDBY_ERR_I2C_SDA_LOCK (1)
#define STANDBY_I2C_GPIO_TIMES_OUT (10)
#define STANDBY_ERR_TIME_OUT (-34)/*Waiting time out.*/
#define STANDBY_ERR_FAILURE (0xFF)/*Common error, operation not success.*/   
#define STANDBY_SUCCESS (0)/*Common error, operation not success.*/
#define HAL_GPIO_O_DIR (1)
#define HAL_GPIO_I_DIR (0)
#define HAL_GPIO_SET_HI (1)
#define HAL_GPIO_SER_LOW (0)

/*TODO: UPDATE code that driectly use the following defines.*/
#define STANDBY_GPIO_DIR_REG NULL
#define STANDBY_GPIO1_DIR_REG NULL
#define STANDBY_GPIO_DO_REG NULL
#define STANDBY_GPIO1_DO_REG NULL
#define STANDBY_GPIO_I_DIR (0)
#define STANDBY_GPIO_O_DIR (1)
#define STANDBY_GPIO_INT_EN (0)
#define STANDBY_GPIO_INT_DIS (1)
#define STANDBY_GPIO_EDG_EN (1)
#define STANDBY_GPIO_EDG_DIS (0)

#define OFFSET(x) (ali_gpio->offsets ? ali_gpio->offsets[x] : OFFSETS_DEFAULT[x])
#define OFFSET_TO_REG_INT_EN OFFSET(0)
#define OFFSET_TO_REG_EDGE_RISING OFFSET(1)
#define OFFSET_TO_REG_EDGE_FALLING OFFSET(2)
#define OFFSET_TO_REG_INT_STATUS OFFSET(3)
#define OFFSET_TO_REG_SET_DIR OFFSET(4)
#define OFFSET_TO_REG_SET_OUTPUT OFFSET(5)
#define OFFSET_TO_REG_INPUT_STATUS OFFSET(6)
#define OFFSETS_SIZE (7)

typedef unsigned int u32;

/*offsets of past project such as C3505.*/
static const u32 OFFSETS_DEFAULT[OFFSETS_SIZE] = {0x0, 0x4, 0x8, 0x18, 0x14, 0x10, 0xC};
static const u32 OFFSETS_C3702_C3922[OFFSETS_SIZE] = {0x0, 0x4, 0x8, 0xC, 0x10, 0x14, 0x18};

struct ali_gpio {
	u32 base;
	const u32 *offsets;
};

static struct ali_gpio ali_gpios[] = {
#if (defined CHIP_C3702)
	{
		.base = 0xb808d000,/*la.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d100,/*ba.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d120,/*bb.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d140,/*bc.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d200,/*ra.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d300,/*ta.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d320,/*tb.*/
		.offsets = OFFSETS_C3702_C3922,
	},
#elif (defined CHIP_C3922)
	{
		.base = 0x1808d000,/*a.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0x1808d020,/*lb.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0x1808d040,/*lc.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0x1808d060,/*ld.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0x1808d200,/*ra.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0x1808d220,/*rb.*/
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0x1808d300,/*ta.*/
		.offsets = OFFSETS_C3702_C3922,
	},
#endif
};

#define standby_ioread32(addr) (*((volatile u32 *)(addr)))
#define standby_iowrite32(val, addr) (*((volatile u32 *)(addr)) = (val))
static inline u32 ali_gpio_reg_read(struct ali_gpio *gpio, unsigned int offs)
{
	(void)ali_gpios;/*avoid defined but not used warning.*/
	return standby_ioread32(gpio->base + offs);
}

static inline void ali_gpio_reg_write(struct ali_gpio *gpio, unsigned int offs, u32 val)
{
	standby_iowrite32(val, gpio->base + offs);
}

#define ali_gpio_set_bit_without_lock(pos, offs, flag) do { \
	int index, bit; \
	struct ali_gpio *ali_gpio; \
	u32 val, mask; \
	index = pos / 32; \
	bit = pos % 32; \
	mask = 1 << bit; \
	ali_gpio = &ali_gpios[index]; \
	val = ali_gpio_reg_read(ali_gpio, offs); \
	val = (val & ~mask) | (flag << bit); \
	ali_gpio_reg_write(ali_gpio, offs, val); \
} while (0)

#define ali_gpio_set_bit(pos, offs, flag) do { \
	osal_interrupt_disable(); \
	ali_gpio_set_bit_without_lock(pos, offs, flag); \
	osal_interrupt_enable(); \
} while (0)

#define ali_gpio_get_bit(pos, offs) ({ \
	int index, bit; \
	struct ali_gpio *ali_gpio; \
	index = pos / 32; \
	bit = pos % 32; \
	ali_gpio = &ali_gpios[index]; \
	((ali_gpio_reg_read(ali_gpio, offs)) >> bit) & 1; \
})

#define STANDBY_PMX_PIN_SET(base, number, value) (*(volatile long *)(((long)base) + sizeof (long) * number) = value)
#define STANDBY_PMX_PIN_GET(base, number) (*(volatile long *)(((long)base) + sizeof (long) * number))

#define STANDBY_PMX_PIN_SEL(base, number, sel) do { \
	long value = STANDBY_PMX_PIN_GET(base, number); \
	value = (value & (~((long)0xf))) | sel; \
	STANDBY_PMX_PIN_SET(base, number, value); \
} while (0)

#define STANDBY_GPIO_BIT_GET_SET(pos) ali_gpio_get_bit(pos, OFFSET_TO_REG_SET_OUTPUT)
#define STANDBY_GPIO_BIT_GET(pos) ali_gpio_get_bit(pos, OFFSET_TO_REG_INPUT_STATUS)
#define STANDBY_GPIO_BIT_DIR_GET(pos) ali_gpio_get_bit(pos, OFFSET_TO_REG_SET_DIR)
#define STANDBY_GPIO_BIT_DIR_SET(pos, val) ali_gpio_set_bit(pos, OFFSET_TO_REG_SET_DIR, val)
#define STANDBY_GPIO_BIT_SET(pos, val) ali_gpio_set_bit(pos, OFFSET_TO_REG_SET_OUTPUT, val)
#define STANDBY_GPIO_INT_SET(pos, en) ali_gpio_set_bit(pos, OFFSET_TO_REG_INT_EN, en)
#define STANDBY_GPIO_INT_REDG_SET(pos, rise) ali_gpio_set_bit(pos, OFFSET_TO_REG_EDGE_RISING, rise)
#define STANDBY_GPIO_INT_FEDG_SET(pos, fall) ali_gpio_set_bit(pos, OFFSET_TO_REG_EDGE_FALLING, fall)
#define STANDBY_GPIO_INT_EDG_SET(pos, rise, fall) do { \
		osal_interrupt_disable(); \
		ali_gpio_set_bit_without_lock(pos, OFFSET_TO_REG_EDGE_RISING, rise); \
		ali_gpio_set_bit_without_lock(pos, OFFSET_TO_REG_EDGE_FALLING, fall); \
		osal_interrupt_enable(); \
	} while (0)

#define STANDBY_GPIO_INT_STA_GET(pos) ali_gpio_get_bit(pos, OFFSET_TO_REG_INT_STATUS)
#define STANDBY_GPIO_INT_CLEAR(pos) ali_gpio_set_bit(pos, OFFSET_TO_REG_INT_STATUS, 1)

/*Function Declaration.*/
extern unsigned char stby_i2c_gpio_phase_start(void);
extern unsigned char stby_i2c_gpio_phase_stop(void);
extern void stby_i2c_gpio_phase_set_bit(long val);
extern unsigned char stby_i2c_gpio_phase_get_bit(void);
extern unsigned char stby_i2c_gpio_phase_set_byte(unsigned char dat);
extern unsigned char stby_i2c_gpio_phase_get_byte(long ack);
extern unsigned char stby_i2c_gpio_read_no_stop( unsigned char slv_addr, unsigned char *dat, long len);
extern long stby_i2c_gpio_write_no_stop( unsigned char slv_addr, unsigned char *dat, long len);
extern unsigned char stby_i2c_gpio_read( unsigned char slv_addr, unsigned char *dat, long len);
extern void stdby_i2c_gpio_write( unsigned char slv_addr, unsigned char *dat, long len);
#endif/*__STANDBY_GPIO_H__.*/
