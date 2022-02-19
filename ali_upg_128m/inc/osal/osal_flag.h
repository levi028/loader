/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_flag.h
*
*    Description:
*    This header file contains the definitions about event notification functions
*    of abstract os layer. Request of notification of event combination is also
*    allowed.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_FLAG_H__
#define __OSAL_FLAG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include "osal.h"

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

enum
{
     /* Release task until all specified bits are set */
    OSAL_TWF_ANDW    = TWF_ANDW,

    /* Release task if one of specified bits are set */
    OSAL_TWF_ORW    = TWF_ORW,

    /* Clear flag after the first waiting task released */
    OSAL_TWF_CLR    = TWF_CLR,
};

#define osal_flag_create                    os_create_flag
#define osal_flag_delete                    os_del_flag
#define osal_flag_wait                        os_wait_flag_time_out
#define osal_flag_set                        os_set_flag
#define osal_flag_clear                        os_clear_flag

#elif (SYS_OS_MODULE == LINUX_2_4_18)

enum
{
    OSAL_TWF_ANDW,        /* Release task until all specified bits are set */
    OSAL_TWF_ORW,        /* Release task if one of specified bits are set */
    OSAL_TWF_CLR,        /* Clear flag after the first waiting task released */
};

#define osal_flag_create(...)                OSAL_INVALID_ID
#define osal_flag_delete(...)                OSAL_E_FAIL
#define osal_flag_wait(...)                    OSAL_E_FAIL
#define osal_flag_set(...)                    OSAL_E_FAIL
#define osal_flag_clear(...)                OSAL_E_FAIL

#else

enum
{
    OSAL_TWF_ANDW,        /* Release task until all specified bits are set */
    OSAL_TWF_ORW,        /* Release task if one of specified bits are set */
    OSAL_TWF_CLR,        /* Clear flag after the first waiting task released */
};

#define osal_flag_create(...)                OSAL_INVALID_ID
#define osal_flag_delete(...)                OSAL_E_FAIL
#define osal_flag_wait(...)                    OSAL_E_FAIL
#define osal_flag_set(...)                    OSAL_E_FAIL
#define osal_flag_clear(...)                OSAL_E_FAIL

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_FLAG_H__ */
