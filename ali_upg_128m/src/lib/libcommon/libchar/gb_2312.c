 /*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: gb_2312.c
 *
 *    Description: gb2312 charset mapping file, define convert functions.

 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libchar/lib_char.h>
#include "char_map.h"
#include "gb_2312.h"
#include "gb_2312_data.h"
#ifdef GB2312_SUPPORT

//#define gb2312_debug    libc_printf
#define gb2312_debug(...)    do{}while(0)

#define EUC_FORM_VAL    0x8080  // GB2312 <--> EUC form translate value

#define GB2312_MAP_LENGTH   (sizeof(gb2312_map) / sizeof(struct u8u16))
#define UNICODE_MAP_LENGTH  (sizeof(u_g_map) / sizeof(struct u8u16))

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
INT32 convert_gb2312_to_unicode(const UINT8 *gb2312, INT32 length, UINT16 *unicode, INT32 maxlen)
{
    UINT16  gb = 0;
    INT32   si = 0;
    INT32   di = 0;

    if ((NULL == unicode) || (NULL == gb2312))
    {
        return 0;
    }

    if ((0 == length) || (0 == maxlen))
    {
        return 0;
    }
    while ((si < length) && (di < maxlen))
    {
        if (gb2312[si] < ASCII_EXT_BEGIN_CHAR)
        {
            unicode[di] = cpu_to_be16u((UINT16) gb2312[si]);
            di++;
            si++;
        }
        else if ((gb2312[si] != 0xE0)
             && (gb2312[si] >= ASCII_EXT_BEGIN_CHAR)
             && (gb2312[si] <= 0x9F))
        {
            si += 1;    /* control code, ignore it now. */
        }
        else if ((0xE0 == gb2312[si])
             && (si < length - 1) &&
             (gb2312[si + 1] >= ASCII_EXT_BEGIN_CHAR) &&
             (gb2312[si + 1] <= 0x9F))
        {
            si += 2;    /* control code, ignore them now. */
        }
        else
        {
            if (si + 1 < length)
            {
                gb = (UINT16) ((gb2312[si] << 8) | gb2312[si + 1]);
                unicode[di] = u8u16_lookup(gb, gb2312_map, GB2312_MAP_LENGTH);
                if (0xFFFF == unicode[di])
                {
                    // if not found, blank
                    unicode[di] = 0x3000;
                }

                unicode[di] = cpu_to_be16u(unicode[di]);
                si += 2;
                di++;
            }
            else
            {           // skip a single byte
                si++;
            }
        }
    }

    unicode[di] = 0x0000;
    return di;
}
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
INT32 convert_unicode_to_gb2312(UINT16 *unicode, INT32 u_len, UINT8 *gb2312, INT32 g_maxlen)
{
    UINT16  gb = 0;
    INT32   gi = 0;
    INT32   ui = 0;

    if ((NULL == unicode) || (NULL == gb2312))
    {
        return 0;
    }

    if ((0 == g_maxlen) || (0 == u_len))
    {
        return 0;
    }

    while ((gi + 2 < g_maxlen) && (ui < u_len))
    {
        unicode[ui] = U16_TO_UNICODE(unicode[ui]);
        if (0 == (0xFF80 & unicode[ui]))
        {
            //gb2312[gi] = 0x0;
            gb2312[gi] = (UINT8) (unicode[ui] & 0x00FF);
            ui++;
            gi += 1;
        }
        else
        {
            gb = u8u16_lookup(unicode[ui], u_g_map, UNICODE_MAP_LENGTH);
            if (0xFFFF == gb)
            {
                gb2312[gi] = 0xFF;
                gb2312[gi + 1] = (UINT8) (unicode[ui] & 0x00FF);
                gb2312[gi + 2] = (UINT8) ((unicode[ui] & 0xFF00) >> 8);
                ui++;
                gi += 3;
            }
            else
            {
                if (gb < EUC_FORM_VAL)
                {
                    gb += EUC_FORM_VAL;
                }

                gb = cpu_to_be16u(gb);
                if (gi + 1 < g_maxlen)
                {
                    gb2312[gi] = (UINT8) (gb & 0x00FF);
                    gb2312[gi + 1] = (UINT8) (gb >> 8);
                }

                ui++;
                gi += 2;
            }
        }
    }

    gb2312[gi] = 0x0000;
    return gi;
}
#endif /* GB2312_SUPPORT*/
