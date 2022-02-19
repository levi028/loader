 /*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: iso_5937.h
 *
 *    Description: declare convert function.

 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/


#include <types.h>
#include <sys_config.h>
#ifndef __ISO_5937_H__
#define __ISO_5937_H__
#ifdef __cplusplus
extern "C"
{
#endif

/*
 * name        : iso6937_convert_ch
 * description    : convert the first character from iso6937 to unicode.
 * parameter    : 3
 *    IN    UINT8 *iso6937: the ISO5937 string.
 *    IN    INT32 length: the ISO5937 string length.
 *    OUT    UINT16 *unicode: the generated unicode.
 * return value : INT32
 *    1    : 1 iso6937 character is detected, and stored in 1 unicode.
 *    2    : 2 iso6937 combined character is detected and stored in 1 unicode.
 *    other    : the iso6937 character is invalid, or should be skipped.
 */
INT32   iso6937_convert_ch(UINT8 *iso6937, INT32 length, UINT16 *unicode, INT32 maxlen);
#ifdef __cplusplus
}
#endif
#endif /* __ISO_5937_H__ */
