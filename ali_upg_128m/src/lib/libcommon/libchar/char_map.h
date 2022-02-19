/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: char_map.h
 *
 *    Description: This header file declares some utility macros and functions.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#ifndef __CHAR_MAP_H__
#define __CHAR_MAP_H__
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(ISO8859_SUPPORT)
#define ISO_8859_2_SUPPORT
#define ISO_8859_3_SUPPORT
#define ISO_8859_4_SUPPORT
#define ISO_8859_5_SUPPORT
#define ISO_8859_6_SUPPORT
#define ISO_8859_7_SUPPORT
#define ISO_8859_8_SUPPORT
#define ISO_8859_9_SUPPORT
#define ISO_8859_10_SUPPORT
#define ISO_8859_11_SUPPORT
#define ISO_8859_13_SUPPORT
#define ISO_8859_14_SUPPORT
#define ISO_8859_15_SUPPORT
#define ISO_8859_16_SUPPORT
#endif

#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
#define cpu_to_be16u(x) ((x >> 8) | ((x & 0xFF) << 8))
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define cpu_to_be16u(x) (x)
#else
#error "please check SYS_CPU_ENDIAN in <sys_config.h>"
#endif
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
#define cpu_to_le16u(x) (x)
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define cpu_to_le16u(x) ((x >> 8) | ((x & 0xFF) << 8))
#endif
#define le16_to_cpu(x)  cpu_to_le16u(x)
#define be16_to_cpu(x)  cpu_to_be16u(x)

/*
 * redirection Marco
 */
#if ((SYS_CHIP_MODULE == ALI_M3327C) && (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_SDRAM_SIZE == 2))
#define U16_TO_UNICODE(x)   cpu_to_le16u(x)
#else
#define U16_TO_UNICODE(x)   cpu_to_be16u(x)
#endif
struct u8u16
{
    UINT16  key;
    UINT16  value;
};
struct lookup_table
{
    struct u8u16    *table;
    UINT8           count;
};

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
UINT16  u8u16_lookup(UINT16 key, struct u8u16 *table, INT32 count);

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
UINT16  reverse_lookup(UINT16 value, struct u8u16 *table, INT32 count);
#ifdef __cplusplus
}
#endif
#endif /* __CHAR_MAP_H__ */
