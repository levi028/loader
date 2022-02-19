/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_hde.h
*
* Description:
*     This file implement the hardware display engine on ALi chip
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_PUB27_HDE_H__
#define __LIB_PUB27_HDE_H__

#include <basic_types.h>

#define VIEW_MODE_FULL      0
#define VIEW_MODE_PREVIEW   1
#define VIEW_MODE_MULTI     2
#define VIEW_MODE_SML 3

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


void hde_back_color(UINT8, UINT8, UINT8);
void hde_set_mode(UINT32);
void hde_config_preview_window(UINT16 x, UINT16 y, UINT16 w, UINT16 h, BOOL is_pal);

UINT32 hde_get_mode(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __LIB_PUB27_HDE_H__ */

