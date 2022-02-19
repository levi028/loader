/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: char_map.c
 *
 *    Description: This source file define some utility macros and functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include "char_map.h"

/*
 * name        : u8u16_lookup
 * description    : standard procedure of binary search in u8u16 table.
 * parameter    : 3
 *    IN    UINT8 key : the u8 value.
 *    IN    struct u8u16 *table: the table.
 *    IN    INT32 count : the table count.
 * return value : UINT16
 *    0xFFFF:     no match.
 *    other :     the value that matches the key.
 */
UINT16 u8u16_lookup(UINT16 key, struct u8u16 *table, INT32 count)
{
    UINT16  ret = 0xFFFF;
    INT32   mid = 0;
    INT32   low = -1;
    INT32   high = count;

    if ((NULL == table) || (count < 0))
    {
        return ret;
    }

    do
    {
        mid = (low + high) / 2;
        if (table[mid].key > key)
        {
            high = mid;
        }
        else if (table[mid].key < key)
        {
            low = mid;
        }
        else
        {
            ret = table[mid].value;
            break;
        }
    }
    while (high - low > 1);
    return ret;
}

/*
 * name        : reverse_lookup
 * description    : reverse search in u8u16 table.
 * parameter    : 3
 *    IN    UINT16 value : the u8 value.
 *    IN    struct u8u16 *table: the table.
 *    IN    INT32 count : the table count.
 * return value : UINT16
 *    0xFFFF:     no match.
 *    other :     the key that matches the value.
 */
UINT16 reverse_lookup(UINT16 value, struct u8u16 *table, INT32 count)
{
    INT32   i = 0;
    UINT16  ret = 0xFFFF;

    if (NULL == table)
    {
        return ret;
    }

    for (i = 0; i < count; i++)
    {
        if (value == table[i].value)
        {
            ret = table[i].key;
            break;
        }
    }

    return ret;
}
