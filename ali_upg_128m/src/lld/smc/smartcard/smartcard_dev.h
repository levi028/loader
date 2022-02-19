/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard_dev.h
*
*    Description: Head file of smartcard device operation.
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
#ifndef __SMARTCARD_DEV_H__
#define __SMARTCARD_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <types.h>


/* Smart card device operation */
extern INT32 smc_dev_deactive(struct smc_device *);
extern INT32 smc_dev_reset(struct smc_device *, UINT8 *, UINT16 *);
extern INT32 smc_dev_deactive(struct smc_device *);
extern INT32 smc_warm_reset(struct smc_device *);
extern INT32 smc_dev_set_pps(struct smc_device *, UINT8 , UINT8 , UINT8 );
extern INT32 smc_dev_card_exist(struct smc_device *);
extern INT32 smc_dev_multi_class_reset(struct smc_device *, UINT8 *, UINT16 *);
extern void invert(UINT8 *data, INT32 n);

#ifdef __cplusplus
}
#endif
#endif

