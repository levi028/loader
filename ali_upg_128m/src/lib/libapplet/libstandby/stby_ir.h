#ifndef __STBY_IR_H
#define __STBY_IR_H
#include <sys_config.h>
//==============================================================================================//

#define READ_BYTE(addr) (*(volatile unsigned char *)addr)
#define WRITE_BYTE(addr, data) (*(volatile unsigned char *)addr = data)
#define WRITE_DW(addr, data) (*(volatile unsigned long *)addr = data)

#define IR_IOBASE (0xB8018100)
#define IR_REG_CFG (IR_IOBASE+0x00)
#define IR_REG_FIFOCTRL (IR_IOBASE+0x01)
#define IR_REG_TIMETHR (IR_IOBASE+0x02)
#define IR_REG_NOISETHR (IR_IOBASE+0x03)
#define IR_REG_IER (IR_IOBASE+0x06)
#define IR_REG_ISR (IR_IOBASE+0x07)
#define IR_REG_RLCBYTE (IR_IOBASE+0x08)
#define SYS_NB_BASE (0xB8000000)
#ifdef _M3702_
#define NB_IB_ENABLE_REG1 (0xB8081040)
#define NB_IB_ENABLE_REG2 (0xB8081044)
#define NB_IB_ENABLE_REG3 (0xB8081048)
#define NB_IB_ENABLE_REG4 (0xB808104C)
#define NB_IB_ENABLE_REG5 (0xB8081050)
#define NB_IB_ENABLE_REG6 (0xB8081054)
#else
#define NB_IB_ENABLE_REG (SYS_NB_BASE+0x38)
#endif
#define NB_IR_ENABLE_BIT (1<<19)
#define DEFAULT_IRC_VALUE (0x84)
#define IR_FIFO_SIZE (0x10)
#define IR_TIMETHR_VALUE (0x5)
#define IR_TIMEOUT_FIFO_INT (0x3)
#define NOISETHR_CLK (0x1)
#define IR_BUF_LEN (256)
#ifdef _M3702_
#define CLK_CYC_US (19)/*Work clock cycle, in uS.*/
#else
#define CLK_CYC_US (75)/*Work clock cycle, in uS.*/
#endif
#define TIMEOUT_US (24000)/*Timeout threshold, in uS.*/
#define NOISETHR_US (80)/*Noise threshold, in uS.*/
#define PAN_KEY_INVALID (0xFFFFFFFF)
#define PULSE_LOW (((unsigned short)0x0 << 15))
#define PULSE_HIGH ((unsigned short)0x1 << 15)
#define PULSE_POL(x) (x & ((unsigned short)0x1 << 15))
#define PULSE_VALUE(x) (x & (~((unsigned short)0x1 << 15)))
#define INRANGE(x, value, tol) ((x > (value-tol)) && (x < (value+tol)))
#define IRP_CNT (8)//(sizeof(irc_decoders) / sizeof(struct irc_decoder))
//==============================================================================================//

extern void Exit_Standby_Process(void);
extern void stdby_uart_debug(unsigned long data);
//==============================================================================================//
#endif
