/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: arabic_convert.h
 *
 *    Description: This header file declares arabic and hebrew charactors
      process API.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#ifndef __ARABIC_CONVERT_H__
#define __ARABIC_CONVERT_H__
#ifdef __cplusplus
extern "C"
{
#endif
void    hebrew_punctuation_process(unsigned short *str, unsigned int size);
#ifdef __cplusplus
}
#endif
#endif /* __ARABIC_CONVERT_H__ */
