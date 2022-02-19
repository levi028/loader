/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_mutex.h
*
*    Description:
*    This header file contains the definitions about mutex operation functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_MUTEX_H__
#define __OSAL_MUTEX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include "osal.h"
#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

#define osal_mutex_create            os_create_mutex
#define osal_mutex_delete            os_delete_mutex
#define osal_mutex_lock                os_lock_mutex
#define osal_mutex_unlock            os_unlock_mutex

#else

#define osal_mutex_create(...)        OSAL_INVALID_ID
#define osal_mutex_delete(...)        OSAL_E_FAIL
#define osal_mutex_lock(...)        OSAL_E_FAIL
#define osal_mutex_unlock(...)        OSAL_E_FAIL

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_MUTEX_H__ */

