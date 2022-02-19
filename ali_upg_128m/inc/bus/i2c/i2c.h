/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: i2c.h
*
*    Description: This file contains all globe micros and functions declare
*               of I2C driver.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*     1.    Feb.14.2003       Justin Wu      Ver 0.1      Create file.
*     2. Aug.22.2004          Justin Wu      Ver 0.2      Support SCB.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LLD_I2C_H__
#define __LLD_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sys_config.h>

//#define USE_I2C_SDK

#define I2C_TYPE_MASK                     0xffff0000
#define I2C_ID_MASK                       0x0000ffff

#define I2C_TYPE_SCB                      0x00000000
#define I2C_TYPE_GPIO                     0x00010000
#define I2C_TYPE_SCB_RM                   0x00020000

// For TI2C_ISR
#define    ISR_TDI                        0x01   // Transaction Done Interrupt
// For TI2C_ISR
#define ISR1_TRIG                         0x01
#define I2C_TYPE_SCB0                   (I2C_TYPE_SCB|0)
#define I2C_TYPE_SCB1                   (I2C_TYPE_SCB|1)
#define I2C_TYPE_SCB2                   (I2C_TYPE_SCB|2)
#define I2C_TYPE_SCB3			(I2C_TYPE_SCB|3)

#define I2C_TYPE_GPIO0                  (I2C_TYPE_GPIO|0)
#define I2C_TYPE_GPIO1                  (I2C_TYPE_GPIO|1)
#define I2C_TYPE_GPIO2                  (I2C_TYPE_GPIO|2)

#if (!defined(I2C_FOR_TUNER))
        #if(SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE \
            == PROJECT_FE_DVBS2||SYS_PROJECT_FE == PROJECT_FE_ATSC)
                #if (SYS_I2C_MODULE == GPIO_I2C)
                #define I2C_FOR_TUNER                   I2C_TYPE_GPIO
                #else//(SYS_I2C_MODULE == M6303I2C)
                #define I2C_FOR_TUNER                   I2C_TYPE_SCB
                #endif
        #else
                #define I2C_FOR_TUNER                   I2C_TYPE_SCB0
        #endif
#endif

struct i2c_device
{
    INT32   (*mode_set)(UINT32 id, int bps, int en);

    INT32   (*read)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);

    INT32   (*write)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);

    INT32   (*write_read)(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                              int wlen, int rlen);

    INT32   (*write_read_std)(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                                  int wlen, int rlen);

    INT32   (*write_write_read)(UINT32 id, UINT8 segment_ptr, \
                     UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen);
};

struct i2c_device_ext
{
    INT32  (*write_plus_read)(UINT32 id, UINT8 slv_addr, \
                               UINT8*data, int wlen, int rlen);
};

extern struct i2c_device i2c_dev[];
extern struct i2c_device_ext i2c_dev_ext[];

extern UINT32 g_scb_sys_chip_id;
extern UINT32 g_scb_sys_chip_ver;

/* bps: Band rate, bps;
 * en:  1 for enable I2C, 0 for disable I2C;
 * sa:  slave device address;
 * p:   array pointer;
 * wl:    number of write bytes;
 * rl:    number of read bytes;
 */
#ifndef USE_I2C_SDK

#define i2c_mode_set(id, bps, en)         (\
        i2c_dev[(id&I2C_TYPE_MASK)>>16].mode_set((id&I2C_ID_MASK), bps, en))
#define i2c_read(id, sa, p, rl)           (\
        i2c_dev[(id&I2C_TYPE_MASK)>>16].read((id&I2C_ID_MASK), sa, p, rl))
#define i2c_write(id, sa, p, wl)          (\
        i2c_dev[(id&I2C_TYPE_MASK)>>16].write(\
        (id&I2C_ID_MASK), sa, p, wl))
#define i2c_write_read(id, sa, p, wl, rl)   (\
        i2c_dev[(id&I2C_TYPE_MASK)>>16].write_read(\
        (id&I2C_ID_MASK), sa, p, wl, rl))
#define i2c_write_read_std(id, sa, p, wl, rl)   (\
        i2c_dev[(id&I2C_TYPE_MASK)>>16].write_read_std(\
        (id&I2C_ID_MASK), sa, p, wl, rl))
#define i2c_write_write_read(id, ptr,sa1, sa2, p, rl)   (\
        i2c_dev[(id&I2C_TYPE_MASK)>>16].write_write_read(\
        (id&I2C_ID_MASK),ptr, sa1, sa2, p, rl))
#define i2c_write_plus_read(id, sa, p, wl, rl)         (\
        i2c_dev_ext[(id&I2C_TYPE_MASK)>>16].write_plus_read(\
        (id&I2C_ID_MASK), sa, p, wl, rl))

#else

INT32 i2c_scb_attach(int dev_num);
INT32 i2c_gpio_attach(int dev_num, int* gpio_init);
INT32 i2c_mode_set(UINT32 id, int bps, int en);
INT32 i2c_read(UINT32 id, UINT8 sa, UINT8 *p, int rl);
INT32 i2c_write(UINT32 id, UINT8 sa, UINT8 *p, int wl);
INT32 i2c_write_read(UINT32 id, UINT8 sa, UINT8 *p, int wl, int rl);
INT32 i2c_write_read_std(UINT32 id, UINT8 sa, UINT8 *p, int wl, int rl);
INT32 i2c_write_write_read(UINT32 id, UINT8 *ptr, UINT8 sa1,UINT8 sa2, \
                           UINT8 *p, int rl);
INT32 i2c_write_plus_read(UINT32 id, UINT8 sa, UINT8 *p, int wl, int rl);

#endif

extern INT32 i2c_scb_attach(int dev_num);
extern INT32 i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
extern INT32 i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
extern INT32 i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);

INT32 i2c_scb_mode_set(UINT32 id, int bps, int en);
INT32 i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
INT32 i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
INT32 i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                         int wlen, int rlen);
INT32 i2c_scb_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                             int wlen, int rlen);
INT32 i2c_scb_write_write_read(UINT32 id,UINT8 segment_ptr, UINT8 slv1_addr,\
                               UINT8 slv2_addr, UINT8 *data, int rlen);
INT32 i2c_scb_eddc_read(UINT32 id,UINT8 segment_ptr, UINT8 slv1_addr,\
                        UINT8 slv2_addr, UINT8 *data, int rlen);
INT32 i2c_scb_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                              int wlen, int rlen);

// I2c gpio
extern INT32 i2c_gpio_attach(int dev_num);
extern void  i2c_gpio_set(UINT32 id, UINT8 sda, UINT8 scl);
extern INT32 i2c_gpio_write_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
extern INT32 i2c_gpio_mode_set(UINT32 id, int bps, int en);
extern INT32 i2c_gpio_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
extern INT32 i2c_gpio_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
extern INT32 i2c_gpio_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                          int wlen, int rlen);
extern INT32 i2c_gpio_write_write_read(UINT32 id, UINT8 segment_ptr, \
              UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen);
extern INT32 i2c_gpio_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                              int wlen, int rlen);
extern INT32 i2c_gpio_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                               int wlen, int rlen);
extern INT32 i2c_gpio_mbyte_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, \
                                int wlen, int rlen);

#ifdef __cplusplus
 }
#endif

#endif    /* __LLD_I2C_H__ */
