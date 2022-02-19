/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dog.h
*
*    Description:  This file provide common watchdog interface.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LLD_DOG_H__
#define __LLD_DOG_H__

#ifdef __cplusplus
extern "C" {
#endif  /* #ifdef __cplusplus */

#include <sys_config.h>

/* Mode setting */
#define DOG_MODE_WATCHDOG    0x0000
#define DOG_MODE_TIMER        0x0001


#if (SYS_SCI_MODULE == UART16550)

INT32 dog_m3327e_attach(int dev_num);
void dog_m3327e_set_mode(UINT32 id, UINT32 mode, UINT32 duration_us, void (*callback)(UINT32));
UINT32 dog_m3327e_get_time(UINT32 id);
void dog_m3327e_set_time(UINT32 id, UINT32 us);
void dog_m3327e_pause(UINT32 id, int en);
void dog_m3327e_stop(UINT32 id);

/* Duration and time is in uS, callback is interrupt handler if in timer mode */
#define dog_mode_set(id, mode, duration, callback)    dog_m3327e_set_mode(id, mode, duration, callback)
#define dog_get_time(id)                            dog_m3327e_get_time(id)
#define dog_set_time(id, us)                        dog_m3327e_set_time(id, us)
#define dog_pause(id, en)                            dog_m3327e_pause(id, en)
#define dog_stop(id)                                dog_m3327e_stop(id)

#else

#define dog_mode_set(...)                            do{}while(0)
#define dog_get_time(...)                            0
#define dog_set_time(...)                            do{}while(0)
#define dog_pause(...)                                do{}while(0)
#define dog_stop(...)                                do{}while(0)

#endif

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif    /* __LLD_DOG_H__ */
