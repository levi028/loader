/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_cache.h
*
*    Description:
*    This header file contains the definitions about cache operation functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_CACHE_H__
#define __OSAL_CACHE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include "osal.h"

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

#define osal_cache_flush_all            osal_tds2_cache_flush_all
#define osal_cache_invalidate_all(...)    do{}while(0)
#define osal_cache_flush                osal_tds2_cache_flush
#define osal_cache_invalidate            osal_tds2_cache_invalidate

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_cache_flush_all(...)        do{}while(0)
#define osal_cache_invalidate_all(...)    do{}while(0)
#define osal_cache_flush(...)            do{}while(0)
#define osal_cache_invalidate(...)        do{}while(0)

#else

UINT32 osal_nos_cache_flush(void *src, UINT32 len);
UINT32 osal_nos_cache_flush_all(void);
UINT32 osal_nos_cache_invalidate(void *src,UINT32 len);


#define osal_cache_flush_all            osal_nos_cache_flush_all
#define osal_cache_invalidate_all(...)    do{}while(0)
#define osal_cache_flush                osal_nos_cache_flush
#define osal_cache_invalidate            osal_nos_cache_invalidate

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_CACHE_H__ */
