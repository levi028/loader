/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: scart_ak4707.h
*
* Description:
*     scart chip ak4707 driver API implementation for DVBapplication.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SCART_AK4707_H__
#define __SCART_AK4707_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define SCART_AK4707_BASE_ADDR     0x22
#define SCART_AK_PRINTF(...)        do{}while(0)    //libc_printf
#define AK_MAX_REG_LEN        16
//#define AK_DEF_VOL            0x7
struct scart_ak4707_private
{
    void  *priv_addr;
    UINT8 reg_val[AK_MAX_REG_LEN];
    UINT8 reg_dirty_list[AK_MAX_REG_LEN];
};

#ifdef __cplusplus
}
#endif

#endif/*__SCART_AK4707_H__*/
