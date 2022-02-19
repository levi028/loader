/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard_txrx.h
*
*    Description: Head file of smartcard transmission.
*
*    History:
*      Date          Author          Version         Comment
*      ====          ======          =======         =======
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SMARTCARD_TXRX_H__
#define __SMARTCARD_TXRX_H__

#include <types.h>
#include "smartcard.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Smart card transmission operation */
extern void smc_write_tx(struct smartcard_private *tp, UINT32 io_base, UINT16 val);
extern UINT16 smc_read_tx(struct smartcard_private *tp, UINT32 io_base);
extern void smc_write_rx(struct smartcard_private *tp, UINT32 io_base, UINT16 val);
extern UINT16 smc_read_rx(struct smartcard_private *tp, UINT32 io_base);
extern void smc_dev_clear_tx_rx_buf(struct smartcard_private *);
extern INT32 smc_dev_transfer_data(struct smc_device *, UINT8 *, UINT16 , UINT8 *, UINT16 , UINT16 *);
extern INT32 smc_dev_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize);
extern INT32 smc_dev_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize);
extern INT32 smc_dev_iso_transfer(struct smc_device *, UINT8 *, INT16 ,
                        UINT8 *, INT16 , INT16 *);
extern INT32 smc_dev_iso_transfer_t1(struct smc_device *, UINT8 *, INT16 ,
                        UINT8 *, INT16 ,INT32 *);
extern INT32 smc_dev_pps_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize);
extern INT16 smc_dev_pps_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize);
#ifdef __cplusplus
}
#endif
#endif

