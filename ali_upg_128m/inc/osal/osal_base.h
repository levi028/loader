/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_base.h
*
*    Description:
*    This header file contains the definitions about basic types of
*    abstract os layer.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_BASE_H__
#define __OSAL_BASE_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef INT32                               OSAL_ER;
typedef UINT32                              OSAL_ID;


#if (SYS_OS_MODULE == ALI_TDS2)

    #include <os/tds2/itron.h>

    #define OSAL_E_OK                       E_OK
    #define OSAL_E_FAIL                     E_FAILURE
    #define OSAL_E_TIMEOUT                  E_TIMEOUT

    #define OSAL_INVALID_ID                 INVALID_ID
    #define OSAL_WAIT_FOREVER_TIME          0xFFFFFFFF

#elif (SYS_OS_MODULE == LINUX_2_4_18)

    #define OSAL_E_OK                       SUCCESS
    #define OSAL_E_FAIL                     ERR_FAILUE
    #define OSAL_E_TIMEOUT                  ERR_TIME_OUT

    #define OSAL_INVALID_ID                 0
    #define OSAL_WAIT_FOREVER_TIME          0xFFFFFFFF

#else

    #define OSAL_E_OK                       SUCCESS
    #define OSAL_E_FAIL                     ERR_FAILUE
    #define OSAL_E_TIMEOUT                  ERR_TIME_OUT

    #define OSAL_INVALID_ID                 0xFFFFFFFF
    #define OSAL_WAIT_FOREVER_TIME          0xFFFFFFFF

    #define INLINE                          inline

    typedef long                            ER;
    typedef long                            INT;
    typedef unsigned char                   BYTE;
    typedef unsigned short                  WORD;
    typedef unsigned long                   DWORD;
    typedef WORD                            ID;

    /* System call Error Code */
    #define E_OK                            0
    #define E_FAILURE                       (-1)
    #define E_TIMEOUT                       (-2)

struct os_config
{

    UINT16 task_num;
    UINT16 sema_num;
    UINT16 flg_num;
    UINT16 mbf_num;
    UINT16 mutex_num;
    UINT16 parity_value;
};
#endif


#ifdef __cplusplus
}
#endif

#endif /* __OSAL_BASE_H__ */

