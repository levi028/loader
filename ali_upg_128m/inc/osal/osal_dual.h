/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osal_dual.h
*
*    Description:
*    This header file contains the definitions about dual-core communication
*    functions.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __OSAL_DUAL_H__
#define __OSAL_DUAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>

#if (SYS_OS_MODULE == ALI_TDS2)
#include <os/tds2/itron.h>




UINT32 osal_dual_get_see_run_address(void);
UINT32 osal_dual_get_priv_mem_start_address(void);
UINT32 osal_dual_get_priv_mem_end_address(void);
BOOL osal_check_address_range_legal(void *start_addr, UINT32 ui_byte_length);
BOOL osal_address_is_code_section(void *dest);
BOOL osal_dual_is_private_address(void *dest);
    


#elif (SYS_OS_MODULE == LINUX_2_4_18)



#else



#endif


#ifdef __cplusplus
 }
#endif

#endif /* __OSAL_DUAL_H__ */
