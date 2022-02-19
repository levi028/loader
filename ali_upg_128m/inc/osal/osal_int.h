/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_int.h
*
*    Description:
*    This header file contains the definitions about interrupt management functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_INT_H__
#define __OSAL_INT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include <osal/osal.h>

typedef void (*OSAL_T_LSR_PROC_FUNC_PTR)(UINT32);
typedef void (*OSAL_T_HSR_PROC_FUNC_PTR)(UINT32);

#if (SYS_OS_MODULE == ALI_TDS2)
//#include <os/tds2/itron.h>

#define osal_interrupt_register_lsr            os_register_isr
#define osal_interrupt_unregister_lsr        os_delete_isr
#define osal_interrupt_register_hsr            os_register_hsr
#define osal_interrupt_disable                os_disable_interrupt
#define osal_interrupt_enable                os_enable_interrupt

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_interrupt_register_lsr(...)    OSAL_E_FAIL
#define osal_interrupt_unregister_lsr(...)    OSAL_E_FAIL
#define osal_interrupt_register_hsr(...)    OSAL_E_FAIL
#define osal_interrupt_disable(...)            do{}while(0)
#define osal_interrupt_enable(...)            do{}while(0)

#else

OSAL_ER osal_nos_register_interrupt(UINT16 num, void (*isr)(UINT32), UINT32 param);
OSAL_ER osal_nos_remove_interrupt(UINT16 num, void (*isr)(UINT32));
OSAL_ER osal_nos_interrupt_register_hsr(OSAL_T_HSR_PROC_FUNC_PTR hsr_rsp, UINT32 param);
void osal_nos_disable_interrupt(void);
void osal_nos_enable_interrupt(void);
inline void osal_nos_irq_dispatch(int num);

#define osal_interrupt_register_lsr            osal_nos_register_interrupt
#define osal_interrupt_unregister_lsr        osal_nos_remove_interrupt
#define osal_interrupt_register_hsr            osal_nos_interrupt_register_hsr
#define osal_interrupt_disable                osal_nos_disable_interrupt
#define osal_interrupt_enable                osal_nos_enable_interrupt

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_INT_H__ */
