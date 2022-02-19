/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: sci.h
*
*  Description: This file provide common Serial Communication Interface.
*
*  History:
*      Date        Author      Version        Comment
*      ====        ======      =======        =======
*  1.  2003.2.13   Liu Lan     0.1.000        Initial
*  2.  2003.3.12   Justin Wu   0.1.001        Add M6303GSI support
*  3.  2006.1.13   Justin Wu   0.2.000        Make support multi UART; remove MDM.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
******************************************************************************/
#ifndef __LLD_SCI_H__
#define __LLD_SCI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <types.h>

/* sci_mode_set(): parity bit alocate:
 *    byte1 half low      byte0 half high     byte0 half low
 *  (stop bit setting) (word length setting) (parity setting)
 */
/* Parity setting */
#define SCI_PARITY_NONE     0x0000
#define SCI_PARITY_EVEN     0x0001
#define SCI_PARITY_ODD      0x0002

/* Word length setting */
#define SCI_WORDLEN_8       0x0000
#define SCI_WORDLEN_7       0x0010
#define SCI_WORDLEN_6       0x0020
#define SCI_WORDLEN_5       0x0030

/* Stop bit setting */
#define SCI_STOPBIT_1       0x0000
#define SCI_STOPBIT_2       0x0100

/* Device ID */
#define SCI_FOR_RS232       0
#define SCI_FOR_MDM         1
#define SCI_MAX_NUM  	    2

#define UART_SPEED_115200  0
#define UART_SPEED_562500  1
#define UART_SPEED_1687500  2
#define UART_SPEED_3375000  3

#define UART_BPS_115200  115200
#define UART_BPS_MAX     2000000


#if (SYS_SCI_MODULE == UART16550)

#define sci_mode_set(id, bps, parity, ...)  sci_16550uart_set_mode(id, \
                                                         bps, parity)
#define sci_clear_buff(id)                  sci_16550uart_clear_buff(id)
#define sci_loopback(id, mode)              sci_16550uart_loopback(id, mode)
#define sci_read(id)                        sci_16550uart_read(id)
#define sci_read_tm(id, ch, tm)             sci_16550uart_read_tm(id, ch, tm)
#define sci_write(id, ch)                   sci_16550uart_write(id, ch)
#define sci_fifowrite(id, buf,len)          sci_16550uart_fifowrite(id, \
                                                                 buf, len)

#else

#define sci_mode_set(...)                   do{}while(0)
#define sci_clear_buff(...)                 do{}while(0)
#define sci_loopback(...)                   do{}while(0)
#define sci_read(...)                       0
#define sci_read_tm(...)                    ERR_FAILUE
#define sci_write(...)                      do{}while(0)

#endif



/* Telelogic Logiscope  */
void sci_16550uart_set_mode(UINT32 id, UINT32 bps, int parity);
void sci_16550uart_hight_speed_mode_write(UINT32 id, UINT8 ch);
UINT8 sci_16550uart_hight_speed_mode_read(UINT32 id);
void sci_16550uart_set_high_speed_mode(UINT32 id, UINT32 mode);
INT32 sci_16550uart_attach(int dev_num);
UINT8 sci_16550uart_read(UINT32 id);
INT32 sci_16550uart_read_tm(UINT32 id, UINT8 *data, UINT32 timeout);
void sci_16550uart_loopback(UINT32 id, UINT8 mode);
void sci_16550uart_write(UINT32 id, UINT8 ch);
INT32 sci_16550uart_fifowrite(UINT32 id, UINT8 *buf,int len);
void sci_16550uart_clear_buff(UINT32 id);
void sci_16550uart_set_write_delay(int mode);
typedef void (*ComIntrruptCB)(unsigned char byChar);
extern ComIntrruptCB g_ComIntrruptCB;

#ifdef __cplusplus
 }
#endif


#endif  /* __LLD_SCI_H__ */
