/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: tsg.h
*
*    Description: This file includes the API of TS Generator.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef    __TSG_H__
#define __TSG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>

/* This value defines the M36XX maximum number of packets to handle in one transfer. */
/* For M3329E, the maximum number is TSG_MAX_XFER_PKTS/2 == 1394; */
#define TSG_MAX_XFER_PKTS 2788

#define TSG_MAX_XFER_WAIT_INTERVAL 2000


RET_CODE tsg_init(UINT8 clk_sel);
UINT32 tsg_transfer(void *addr, UINT16 pkt_cnt, UINT8 sync);
void tsg_wait(UINT32 xfer_id);
void tsg_set_clk(UINT8 clk_sel);
void tsg_set_default_clk(UINT8 clk_sel);
void tsg_send_null_packets_disable(UINT8 flag);

void tsg_set_clk_async(UINT8 clk_sel);
void tsg_start(UINT32 bitrate);
void tsg_stop(void);
RET_CODE tsg_check_buf_busy(UINT32 buf_addr,UINT32 buf_len);
UINT32 tsg_check_remain_buf(void);

#ifdef __cplusplus
}
#endif

#endif    /* __TSG_H__ */

