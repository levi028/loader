/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: erom.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _EROM_H_
#define _EROM_H_

#ifdef __cplusplus
extern "C" {
#endif
enum erom_sync_mode
{
    DUPLEX_MODE ,
    SIMPLEX_MODE
};


void uart_reset(void);
int erom_sync(unsigned long sync_tmo, unsigned int simplex_mode);
int erom_sync_ext(unsigned long sync_tmo, unsigned int simplex_mode);//for mploader
int erom_rw(unsigned long mem_addr, void *buf, \
     unsigned long len, unsigned int block_mode, unsigned int is_read);
int erom_read_tmo( unsigned long mem_addr, void *buf,unsigned long len,\
         unsigned int block_mode, unsigned long tmo_us);
int erom_setpc(unsigned long new_pc);

void erom_enter(unsigned int waitforever);

#define erom_rm(a,b,c,d)  erom_rw(a,b,c,d,1)
#define erom_wm(a,b,c,d)  erom_rw(a,b,c,d,0)


#ifdef __cplusplus
 }
#endif

#endif    /*_EROM_H_*/
