/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_timer.h
*
*    Description:
*    This header file contains the definitions about timer management functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_TIMER_H__
#define __OSAL_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include <osal/osal.h>

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

enum
{
    OSAL_TIMER_ALARM    = TIMER_ALARM,        /* Alarm, run only once */
    OSAL_TIMER_CYCLE    = TIMER_CYCLIC,        /* Cycle */
};

#define osal_timer_create                    os_create_timer
#define osal_timer_delete                    os_delete_timer
#define osal_timer_set                        os_set_timer
#define osal_timer_activate                    os_activate_timer
#define osal_get_tick                        os_get_tick_count
#define osal_get_time                        os_get_time
#define osal_delay                            os_delay

#define OSAL_T_TIMER_FUNC_PTR                TMR_PROC
#define OSAL_T_CTIM                            T_TIMER

#elif (SYS_OS_MODULE == LINUX_2_4_18)

enum
{
    OSAL_TIMER_ALARM,                        /* Alarm, run only once */
    OSAL_TIMER_CYCLE,                        /* Cycle */
};

#define osal_timer_create(...)                OSAL_INVALID_ID
#define osal_timer_delete(...)                OSAL_E_FAIL
#define osal_timer_set(...)                    OSAL_E_FAIL
#define osal_timer_activate(...)            OSAL_E_FAIL
#define osal_get_tick(...)                    0
#define osal_get_time(...)                    0
#define osal_delay(...)                        do{}while(0)

#else

enum
{
    OSAL_TIMER_ALARM,                        /* Alarm, run only once */
    OSAL_TIMER_CYCLE,                        /* Cycle */
};

extern UINT32 g_us_ticks;
void osal_nos_delay(UINT16 n);

#define osal_timer_create(...)                OSAL_INVALID_ID
#define osal_timer_delete(...)                OSAL_E_FAIL
#define osal_timer_set(...)                    OSAL_E_FAIL
#define osal_timer_activate(...)            OSAL_E_FAIL
#define osal_get_tick(...)                    0
#define osal_get_time(...)                    0
#define osal_delay                            osal_nos_delay

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_TIMER_H__ */
