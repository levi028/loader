/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard_irq.h
*
*    Description: Head file of smartcard interrupt.
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
#ifndef __SMARTCARD_IRQ_H__
#define __SMARTCARD_IRQ_H__

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Smart card interrupt operation */

extern void smc_dev_interrupt(UINT32 );
extern void smc_dev_hsr_1(UINT32 );
extern void smc_dev_hsr_0(UINT32 );

#ifdef __cplusplus
}
#endif
#endif

