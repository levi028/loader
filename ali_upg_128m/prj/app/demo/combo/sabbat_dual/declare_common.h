/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: control.h
 *
 *    Description: This head file application's control task relate APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef _DECL_COMMON_H_
#define _DECL_COMMON_H_
#include <sys_config.h>
#include <basic_types.h>


#ifdef __cplusplus
extern "C"
{
#endif

///////////////////COMMON INCLUDE /////////////////
#ifdef _MHEG5_SUPPORT_
#include "mheg5_ap/mheg5/glue/mheg_app.h"
#endif

///////////////////COMMON DECLARE /////////////////
#ifdef MULTIVIEW_SUPPORT
BOOL win_multiview_ext_get_state(void);
BOOL win_multiview_get_state(void);
void win_multiview_set_age_lock(BOOL age_lock);
BOOL win_multiview_get_age_lock(void);
BOOL win_multiview_ext_get_lock(void);
void win_multiview_ext_set_lock(BOOL flag);
void win_multiview_ext_set_age_lock(BOOL age_lock);
#endif

#ifdef _MHEG5_SUPPORT_
void mheg_mem_register_callback(void);
#endif

#ifdef __cplusplus
}
#endif
#endif //_DECL_COMMON_H_

