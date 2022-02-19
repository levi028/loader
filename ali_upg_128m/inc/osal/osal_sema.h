/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_mutex.h
*
*    Description:
*    This header file contains the definitions about semaphore operation functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_SEMA_H__
#define __OSAL_SEMA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include "osal.h"

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

#define osal_semaphore_create                os_create_semaphore
#define osal_semaphore_delete                os_del_semaphore
#define osal_semaphore_capture                os_acquire_semaphore_time_out
#define osal_semaphore_release                os_free_semaphore
#define osal_semaphore_release2                os_free_semaphore2

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_semaphore_create(...)            OSAL_INVALID_ID
#define osal_semaphore_delete(...)            OSAL_E_FAIL
#define osal_semaphore_capture(...)            OSAL_E_FAIL
#define osal_semaphore_release(...)            OSAL_E_FAIL

#else

#define osal_semaphore_create(...)            OSAL_INVALID_ID
#define osal_semaphore_delete(...)            OSAL_E_FAIL
#define osal_semaphore_capture(...)            OSAL_E_FAIL
#define osal_semaphore_release(...)            OSAL_E_FAIL

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_SEMA_H__ */
