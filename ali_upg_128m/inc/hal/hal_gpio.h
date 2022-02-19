/*****************************************************************************
 *    Ali Corp. All Rights Reserved. 2002-2017 Copyright (C)
 *
 *    File:    hal_gpio.h
 *
 *    Description:    This file contains all functions about GPIO operations.
 *    History:
 *           Date            Athor        Version          Reason
 *        ============    =============    =========    =================
 *    1.  Jan.30.2003       Justin Wu       Ver 0.1    Create file.
 *    2.  2003.6.26         Liu Lan         Ver 0.2    Exclusive access
 *    3.  2005.5.12         Justin Wu       Ver 0.3    Support M3327/M3327C only.
 *    4.  2005.12.9         Justin Wu       Ver 0.4    Support interrupt setting.
 *    5.  2017              Billy Zhou      Ver 0.5    Refactor
 *****************************************************************************/

#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#ifdef __cplusplus
//extern "C" {
#endif

#include <types.h>
#include <osal/osal.h>
#include <sys_config.h>

#if defined(_S3281_)
#define CHIP_C3281
#elif defined(_M3821_)
#define CHIP_C3821
#elif defined(_M3503_)
#define CHIP_C3503
#elif defined(_M3505_)
#define CHIP_C3505
#elif defined(_M3702_)
#define CHIP_C3702
#elif defined(_M3922_)
#define CHIP_C3922
#elif defined(_M3503D_)
#define CHIP_C3503D
#elif defined(_M3711C_)
#define CHIP_C3711C
#endif

#define HAL_GPIO_NUM_INVALID   (-100)

// TODO: UPDATE code that driectly use the following defines
#define HAL_GPIO_DIR_REG	NULL
#define HAL_GPIO1_DIR_REG	NULL
#define HAL_GPIO_DO_REG		NULL
#define HAL_GPIO1_DO_REG	NULL

#define HAL_GPIO_I_DIR        	0
#define HAL_GPIO_O_DIR        	1

#define HAL_GPIO_INT_EN        	0
#define HAL_GPIO_INT_DIS    	1
#define HAL_GPIO_EDG_EN        	1
#define HAL_GPIO_EDG_DIS    	0

#define GET_SET_BIT_USE_INLINE
#ifdef GET_SET_BIT_USE_INLINE
#define OFFSET(x) (x)
#else
#define OFFSET(x) (ali_gpio->offsets ? ali_gpio->offsets[x] : OFFSETS_DEFAULT[x])
#endif

#define OFFSET_TO_REG_INT_EN				OFFSET(0)
#define OFFSET_TO_REG_EDGE_RISING			OFFSET(1)
#define OFFSET_TO_REG_EDGE_FALLING			OFFSET(2)
#define OFFSET_TO_REG_INT_STATUS			OFFSET(3)
#define OFFSET_TO_REG_SET_DIR				OFFSET(4)
#define OFFSET_TO_REG_SET_OUTPUT			OFFSET(5)
#define OFFSET_TO_REG_INPUT_STATUS			OFFSET(6)

typedef unsigned int u32;

#define OFFSETS_SIZE 		7
// offsets of past project such as C3505
static const u32 OFFSETS_DEFAULT[OFFSETS_SIZE] 		= {0x0, 0x4, 0x8, 0x18, 0x14, 0x10, 0xc};
static const u32 OFFSETS_C3702_C3922[OFFSETS_SIZE] 	= {0x0, 0x4, 0x8, 0xc, 0x10, 0x14, 0x18};

struct ali_gpio {
	u32 base;
	const u32 *offsets;
};

static struct ali_gpio ali_gpios[] = {
#if defined(CHIP_C3281)

	{
		.base = 0xb8000044,
	},
	{
		.base = 0xb80000a4,
	},
	{
		.base = 0xb80000d4,
	},
	{
		.base = 0xb8000154,//3
	},
     
#elif (defined(CHIP_C3505) || defined(CHIP_C3821) || defined(CHIP_C3503) || defined(CHIP_C3503D) || defined(CHIP_C3711C))
	
	{
		.base = 0xb8000044,
	},
	{
		.base = 0xb80000c4,
	},
	{
		.base = 0xb80000e4,
	},
	{
		.base = 0xb8000344,//3
	},
	
#elif defined(CHIP_C3702)

	{
		.base = 0xb808d000,//la
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d100,//ba
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d120,//bb
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d140,//bc
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d200,//ra
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d300,//ta
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d320,//tb
		.offsets = OFFSETS_C3702_C3922,
	},

#elif defined(CHIP_C3922)

	{
		.base = 0xb808d000,//la
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d020,//lb
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d040,//lc
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d060,//ld
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d200,//ra
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d220,//rb
		.offsets = OFFSETS_C3702_C3922,
	},
	{
		.base = 0xb808d300,//ta
		.offsets = OFFSETS_C3702_C3922,
	},

#else
#error UNKNOWN CHIP FOR HAL_GPIO!!!
#endif
};

#define ioread32(addr)		(*((volatile u32 *)(addr)))
#define iowrite32(val, addr) 	(*((volatile u32 *)(addr)) = (val))

static inline u32 ali_gpio_reg_read(struct ali_gpio *gpio, unsigned int offs)
{
	(void)ali_gpios;// avoid defined but not used warning
	return ioread32(gpio->base + offs);
}

static inline void ali_gpio_reg_write(struct ali_gpio *gpio, unsigned int offs, u32 val)
{
	iowrite32(val, gpio->base + offs);
}

#ifdef GET_SET_BIT_USE_INLINE
static inline void ali_gpio_set_bit_without_lock(int pos, int idx, int flag)
{
        struct ali_gpio *ali_gpio;
        int bit, offs;
        u32 val, mask;

        if (-1 == pos) {
                return;
        }

        flag = (0 == flag) ? 0 : 1;
        bit = pos % 32;
        mask = 1 << bit;
        ali_gpio = &ali_gpios[pos / 32];
        offs = ali_gpio->offsets ? ali_gpio->offsets[idx] : OFFSETS_DEFAULT[idx];
        val = ali_gpio_reg_read(ali_gpio, offs);
        val = (val & ~mask) | (flag << bit);
        ali_gpio_reg_write(ali_gpio, offs, val);
}

static inline void ali_gpio_set_bit(int pos, int idx, int flag)
{
        osal_interrupt_disable();
        ali_gpio_set_bit_without_lock(pos, idx, flag);
        osal_interrupt_enable();
}

static inline int ali_gpio_get_bit(int pos, int idx)
{
        struct ali_gpio *ali_gpio;
        int bit, offs;

        if (-1 == pos) {
                return -1;
        }

        bit = pos % 32;
        ali_gpio = &ali_gpios[pos / 32];
        offs = ali_gpio->offsets ? ali_gpio->offsets[idx] : OFFSETS_DEFAULT[idx];
        return ((ali_gpio_reg_read(ali_gpio, offs)) >> bit) & 1;
}
#else
#define ali_gpio_set_bit_without_lock(pos, offs, flag) do {             \
                int index, bit;                                         \
                struct ali_gpio *ali_gpio;                              \
                if (-1 != pos) {                                        \
                        u32 val, mask;                                  \
                        index = pos / 32;                               \
                        bit = pos % 32;                                 \
                        mask = 1 << bit;                                \
                        ali_gpio = &ali_gpios[index];                   \
                        val = ali_gpio_reg_read(ali_gpio, offs);        \
                        val = (val & ~mask) | (flag << bit);            \
                        ali_gpio_reg_write(ali_gpio, offs, val);        \
                }                                                       \
        } while (0)

#define ali_gpio_set_bit(pos, offs, flag) do {                  \
                osal_interrupt_disable();                       \
                ali_gpio_set_bit_without_lock(pos, offs, flag); \
                osal_interrupt_enable();                        \
        } while (0)

#define ali_gpio_get_bit(pos, offs) ({                                  \
        int index, bit, res = -1;                                       \
        if (-1 != pos) {                                                \
                struct ali_gpio *ali_gpio;                              \
                index = pos / 32;                                       \
                bit = pos % 32;                                         \
                ali_gpio = &ali_gpios[index];                           \
                res = ((ali_gpio_reg_read(ali_gpio, offs)) >> bit) & 1; \
        }                                                               \
        res;																														\
})
#endif

#define HAL_GPIO_BIT_GET_SET(pos) 	ali_gpio_get_bit(pos, OFFSET_TO_REG_SET_OUTPUT)
#define HAL_GPIO_BIT_GET(pos)		ali_gpio_get_bit(pos, OFFSET_TO_REG_INPUT_STATUS)
#define HAL_GPIO_BIT_DIR_GET(pos)	ali_gpio_get_bit(pos, OFFSET_TO_REG_SET_DIR)
#define HAL_GPIO_BIT_DIR_SET(pos, val)	ali_gpio_set_bit(pos, OFFSET_TO_REG_SET_DIR, val)
#define HAL_GPIO_BIT_SET(pos, val) 	ali_gpio_set_bit(pos, OFFSET_TO_REG_SET_OUTPUT, val)
#define HAL_GPIO_INT_SET(pos, en)	ali_gpio_set_bit(pos, OFFSET_TO_REG_INT_EN, en)
#define HAL_GPIO_INT_REDG_SET(pos, rise) ali_gpio_set_bit(pos, OFFSET_TO_REG_EDGE_RISING, rise)
#define HAL_GPIO_INT_FEDG_SET(pos, fall)  ali_gpio_set_bit(pos, OFFSET_TO_REG_EDGE_FALLING, fall)
#define HAL_GPIO_INT_EDG_SET(pos, rise, fall)	do {			\
		osal_interrupt_disable();				\
		ali_gpio_set_bit_without_lock(pos, OFFSET_TO_REG_EDGE_RISING, rise); \
		ali_gpio_set_bit_without_lock(pos, OFFSET_TO_REG_EDGE_FALLING, fall); \
		osal_interrupt_enable();				\
	} while (0)

#define HAL_GPIO_INT_STA_GET(pos) ali_gpio_get_bit(pos, OFFSET_TO_REG_INT_STATUS)
#define HAL_GPIO_INT_CLEAR(pos) ali_gpio_set_bit(pos, OFFSET_TO_REG_INT_STATUS, 1)

#ifdef __cplusplus
//}
#endif

#endif /* __HAL_GPIO_H__ */
