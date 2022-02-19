/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal.h
*
*    Description:
*    This header file contains the definitions and declaration about
*    abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef __OSAL_H__
#define __OSAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <retcode.h>
#include <types.h>

#include "osal_base.h"
#include "osal_task.h"
#include "osal_int.h"
#include "osal_timer.h"
#include "osal_msgq.h"
#include "osal_sema.h"
#include "osal_flag.h"
#include "osal_mm.h"
#include "osal_cache.h"
#include "osal_mutex.h"
#include "osal_dual.h"

#ifdef __cplusplus
}
#endif

#endif /* __OSAL_H__ */
