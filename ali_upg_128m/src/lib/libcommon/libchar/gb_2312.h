 /*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: gb_2312.h
 *
 *    Description: define gb2312 declare convert APIs.

 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#ifndef __GB_2312_H__
#define __GB_2312_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef GB2312_SUPPORT

/*
 * name        : convert_gb2312_to_unicode
 * description    : convert the string from gb2312 to unicode.
 * parameter    : 4
 *    IN    UINT8 *gb2312: the gb2312 string.
 *    IN    INT32 length: the gb2312 string length.
 *    OUT    UINT16 *unicode: the generated unicode.
 *    IN    INT32 maxlen: the unicode string maxiam length.
 * return value : INT32
 *    SUCCESS    :    gb2312 string is converted, and stored in unicode.
 *    ERR_FAILUE:
 */
INT32 convert_gb2312_to_unicode(const UINT8 *gb2312, INT32 length, \
                            UINT16 *unicode, INT32 maxlen);

/*
 * name        : convert_unicode_to_gb2312
 * description    : convert the string from unicode to gb2312.
 * parameter    : 4
 *    IN        UINT16 *unicode: the generated unicode.
 *    IN        INT32 u_len: the unicode string  length.
 *    IN        INT32 g_maxlen: the gb2312 string maximum length.
 *    OUT        UINT8 *gb2312: the gb2312 string.
 * return value : INT32
 *    SUCCESS    :    unicode string is converted, and stored in gb2312.
 *    ERR_FAILUE:
 */
INT32 convert_unicode_to_gb2312(UINT16 *unicode,  INT32 u_len, \
         UINT8 *gb2312, INT32 g_maxlen);

#endif

#ifdef __cplusplus
 }
#endif

#endif /* __GB_2312_H__ */

