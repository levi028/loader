/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_common_str.h
*
* Description:
*     Header file for providing common string for HDMI CEC test.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _CEC_COMMON_STR_
#define _CEC_COMMON_STR_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>

extern UINT8 *cec_opcode_str[];     // cec message code string
extern UINT8 *cdc_opcode_str[];     // cdc message code string
extern UINT8 *cec_key_str[];        // cec_key string
extern UINT8 *cec_short_audio_desc_str[];   // cec short audio descrition string

#ifdef __cplusplus
 }
#endif

#endif
