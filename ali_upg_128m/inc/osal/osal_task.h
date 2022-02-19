/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_task.h
*
*    Description:
*    This header file contains the definitions about task management functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_TASK_H__
#define __OSAL_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include <osal/osal.h>

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

/* Type define for task priority */
enum
{
    OSAL_PRI_LOW        = LOW_PRI,            /* Lowest,  for idle task */
    OSAL_PRI_NORMAL        = DEF_PRI,            /* Normal,  for user task */
    OSAL_PRI_HIGH        = 17,                /* High,    for system task */
    OSAL_PRI_CRITICL    = HSR_PRI,            /* Highest, for HSR scheduler */
};

#define    OSAL_TICK_SIZE        10

#define osal_task_get_current_id            os_get_current_thread_id
#define osal_task_create                    os_create_thread
#define osal_task_create_ex                    os_create_thread_ex
#define osal_task_sleep                        os_task_sleep
#define osal_task_exit                        os_exit
#define osal_task_delete                    os_delete_thread
#define osal_task_dispatch_off                os_disable_dispatch
#define osal_task_dispatch_on                os_enable_dispatch

#define OSAL_T_TASK_FUNC_PTR                FP
#define OSAL_T_CTSK                            T_CTSK

#elif (SYS_OS_MODULE == LINUX_2_4_18)

/* Type define for task priority */
enum
{
    OSAL_PRI_LOW,                            /* Lowest,  for idle task */
    OSAL_PRI_NORMAL,                        /* Normal,  for user task */
    OSAL_PRI_HIGH,                            /* High,    for system task */
    OSAL_PRI_CRITICL,                        /* Highest, for HSR scheduler */
};

#define    OSAL_TICK_SIZE        0

#define osal_task_get_current_id(...)        OSAL_INVALID_ID
#define osal_task_create(...)                OSAL_INVALID_ID
#define osal_task_sleep(...)                OSAL_E_FAIL
#define osal_task_exit(...)                    do{}while(0)
#define osal_task_delete(...)                do{}while(0)
#define osal_task_dispatch_off(...)            do{}while(0)
#define osal_task_dispatch_on(...)            do{}while(0)

#else

/* Type define for task priority */
enum
{
    OSAL_PRI_LOW,                            /* Lowest,  for idle task */
    OSAL_PRI_NORMAL,                        /* Normal,  for user task */
    OSAL_PRI_HIGH,                            /* High,    for system task */
    OSAL_PRI_CRITICL,                        /* Highest, for HSR scheduler */
};

#define    OSAL_TICK_SIZE        0

OSAL_ER osal_nos_task_sleep(UINT32 dlytime);

#define osal_task_get_current_id(...)        OSAL_INVALID_ID
#define osal_task_create(...)                OSAL_INVALID_ID
#define osal_task_sleep                        osal_nos_task_sleep
#define osal_task_exit(...)                    do{}while(0)
#define osal_task_delete(...)                do{}while(0)
#define osal_task_dispatch_off(...)            do{}while(0)
#define osal_task_dispatch_on(...)            do{}while(0)

#endif


#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_TASK_H__ */
