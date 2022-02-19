/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_msgq.h
*
*    Description:
*    This header file contains the definitions about message queue functions
*    of abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_MSGQ_H__
#define __OSAL_MSGQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
//#include "osal.h"

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>

#define osal_msgqueue_create                os_create_msg_buffer
#define osal_msgqueue_delete                os_del_message_buffer
#define osal_msgqueue_send                    os_send_msg_time_out
#define osal_msgqueue_receive                os_get_msg_time_out

#define OSAL_T_CMBF                            T_CMBF

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_msgqueue_create(...)            OSAL_INVALID_ID
#define osal_msgqueue_delete(...)            OSAL_E_FAIL
#define osal_msgqueue_send(...)                OSAL_E_FAIL
#define osal_msgqueue_receive(...)            OSAL_E_FAIL

#else

#define osal_msgqueue_create                osal_nos_msgqueue_create
#define osal_msgqueue_delete                osal_nos_msgqueue_delete
#define osal_msgqueue_send                    osal_nos_msgqueue_send
#define osal_msgqueue_receive                osal_nos_msgqueue_receive

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_MSGQ_H__ */
