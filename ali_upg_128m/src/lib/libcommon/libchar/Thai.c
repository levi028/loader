/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: thai.c
 *
 *    Description: Thailand language charset conversion to unicode utility.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libosd/osd_common.h>
#include "char_map.h"
#include "iso_5937.h"

/* Character Type(chtype) */
#define CTRL    0
#define NON     1
#define CONS    2
#define LV      3
#define FV1     4
#define FV2     5
#define FV3     6
#define BV1     7
#define BV2     8
#define BD      9
#define TONE    10
#define AD1     11
#define AD2     12
#define AD3     13
#define AV1     14
#define AV2     15
#define AV3     16

/* Character Level(chlevel) */
#define NONDISP 0
#define BASE    1
#define ABOVE   2
#define BELOW   3
#define TOP     4

/* display rule */
#define DN  0   // display the following character in the next display cell
#define CP  1   // compose the following character with the leading characrer

// and display in the same display cell as the leading charater
#define XC  2   // non-display

#define MAX_THAI_INFO   (sizeof(thai_code_info) / sizeof(thai_code_info[0]))


struct thai_info
{
    UINT16  unicode;
    UINT8   chtype;
    UINT8   chlevel;
};

static const struct thai_info   thai_code_info[] =
{
    { 0x0E01, CONS, BASE },
    { 0x0E02, CONS, BASE },
    { 0x0E03, CONS, BASE },
    { 0x0E04, CONS, BASE },
    { 0x0E05, CONS, BASE },
    { 0x0E06, CONS, BASE },
    { 0x0E07, CONS, BASE },
    { 0x0E08, CONS, BASE },
    { 0x0E09, CONS, BASE },
    { 0x0E0A, CONS, BASE },
    { 0x0E0B, CONS, BASE },
    { 0x0E0C, CONS, BASE },
    { 0x0E0D, CONS, BASE },
    { 0x0E0E, CONS, BASE },
    { 0x0E0F, CONS, BASE },
    { 0x0E10, CONS, BASE },
    { 0x0E11, CONS, BASE },
    { 0x0E12, CONS, BASE },
    { 0x0E13, CONS, BASE },
    { 0x0E14, CONS, BASE },
    { 0x0E15, CONS, BASE },
    { 0x0E16, CONS, BASE },
    { 0x0E17, CONS, BASE },
    { 0x0E18, CONS, BASE },
    { 0x0E19, CONS, BASE },
    { 0x0E1A, CONS, BASE },
    { 0x0E1B, CONS, BASE },
    { 0x0E1C, CONS, BASE },
    { 0x0E1D, CONS, BASE },
    { 0x0E1E, CONS, BASE },
    { 0x0E1F, CONS, BASE },
    { 0x0E20, CONS, BASE },
    { 0x0E21, CONS, BASE },
    { 0x0E22, CONS, BASE },
    { 0x0E23, CONS, BASE },
    { 0x0E24, FV3, BASE },
    { 0x0E25, CONS, BASE },
    { 0x0E26, FV3, BASE },
    { 0x0E27, CONS, BASE },
    { 0x0E28, CONS, BASE },
    { 0x0E29, CONS, BASE },
    { 0x0E2A, CONS, BASE },
    { 0x0E2B, CONS, BASE },
    { 0x0E2C, CONS, BASE },
    { 0x0E2D, CONS, BASE },
    { 0x0E2E, CONS, BASE },
    { 0x0E2F, NON, BASE },
    { 0x0E30, FV1, BASE },
    { 0x0E31, AV2, ABOVE },
    { 0x0E32, FV1, BASE },
    { 0x0E33, FV1, BASE },
    { 0x0E34, AV1, ABOVE },
    { 0x0E35, AV3, ABOVE },
    { 0x0E36, AV2, ABOVE },
    { 0x0E37, AV3, ABOVE },
    { 0x0E38, BV1, BELOW },
    { 0x0E39, BV2, BELOW },
    { 0x0E3A, BD, BELOW },
    { 0x0E3B, NON, NONDISP },
    { 0x0E3C, NON, NONDISP },
    { 0x0E3D, NON, NONDISP },
    { 0x0E3E, NON, NONDISP },
    { 0x0E3F, NON, BASE },
    { 0x0E40, LV, BASE },
    { 0x0E41, LV, BASE },
    { 0x0E42, LV, BASE },
    { 0x0E43, LV, BASE },
    { 0x0E44, LV, BASE },
    { 0x0E45, FV2, BASE },
    { 0x0E46, NON, BASE },
    { 0x0E47, AD2, TOP },
    { 0x0E48, TONE, TOP },
    { 0x0E49, TONE, TOP },
    { 0x0E4A, TONE, TOP },
    { 0x0E4B, TONE, TOP },
    { 0x0E4C, AD1, TOP },
    { 0x0E4D, AD1, TOP },
    { 0x0E4E, AD3, ABOVE },
    { 0x0E4F, NON, BASE },
    { 0x0E50, NON, BASE },
    { 0x0E51, NON, BASE },
    { 0x0E52, NON, BASE },
    { 0x0E53, NON, BASE },
    { 0x0E54, NON, BASE },
    { 0x0E55, NON, BASE },
    { 0x0E56, NON, BASE },
    { 0x0E57, NON, BASE },
    { 0x0E58, NON, BASE },
    { 0x0E59, NON, BASE },
    { 0x0E5A, NON, BASE },
    { 0x0E5B, NON, BASE },
};

/* table for general rules about displaying Thai characters */
/* row: leading char,  column: following char */
static UINT8    thai_dis_rule[17][17] =
{
/*CTRL NON CONS LV FV1 FV2 FV3 BV1 BV2 BD TONE AD1 AD2 AD3 AV1 AV2 AV3 */
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//CTRL
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//NON
  {XC, DN, DN,  DN,DN, DN, DN, CP, CP, CP,CP,  CP, CP, CP, CP, CP, CP},//CONS
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//LV
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//FV1
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//FV2
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//FV3
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,CP,  CP, DN, DN, DN, DN, DN},//BV1
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,CP,  DN, DN, DN, DN, DN, DN},//BV2
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//BD
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//TONE
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//AD1
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//AD2
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,DN,  DN, DN, DN, DN, DN, DN},//AD3
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,CP,  CP, DN, DN, DN, DN, DN},//AV1
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,CP,  DN, DN, DN, DN, DN, DN},//AV2
  {XC, DN, DN,  DN,DN, DN, DN, DN, DN, DN,CP,  DN, CP, DN, DN, DN, DN},//AV3
};
static struct thai_info *thai_seek_info(unsigned short code)
{
    UINT32  i = 0;

    for (i = 0; i < MAX_THAI_INFO; i++)
    {
        if (thai_code_info[i].unicode == code)
        {
            break;
        }
    }

    return i == MAX_THAI_INFO ? NULL : (struct thai_info *) &thai_code_info[i];
}

static UINT8 thai_get_rule(UINT16 leading, UINT16 following)
{
    struct thai_info    *leading_info = NULL;
    struct thai_info    *following_info = NULL;

    leading_info = thai_seek_info(leading);
    following_info = thai_seek_info(following);
    if ((NULL == leading_info) || (NULL == following_info))
    {
        return DN;
    }

    return thai_dis_rule[leading_info->chtype][following_info->chtype];
}

UINT32 thai_get_cell(UINT8 *ptext, struct thai_cell *cell)
{
    if ((NULL == ptext) || (NULL == cell))
    {
        return 0;
    }
    return thai_get_cell_ext(ptext, strlen((char*)ptext), cell);
}
UINT32 thai_get_cell_ext(UINT8 *ptext, UINT32 text_siz, struct thai_cell *cell)
{
    UINT16  wc = 0;
    UINT8   *pstr = NULL;
    UINT32  count = 0;

    if ((NULL == ptext) || (NULL == cell))
    {
        return 0;
    }

    MEMSET(cell, 0, sizeof(struct thai_cell));

    if (text_siz < count)
    {
        return 0;
    }
    pstr = ptext + count;
    wc = com_mb16to_word(pstr);
    if (0 == wc)
    {
        return 0;
    }

    cell->unicode[0] = wc; // unicode[0]
    cell->char_num++;
    count += 2;
    if (text_siz < count)
    {
        return 0;
    }
    wc = com_mb16to_word(pstr + count);
    if (0 == wc)
    {
        return count;
    }

    if (CP == thai_get_rule(cell->unicode[0], wc))
    {
        cell->unicode[1] = wc;
        cell->char_num++;
        count += 2;
        if (text_siz < count)
        {
            return 0;
        }
        wc = com_mb16to_word(pstr + count);
        if (0 == wc)
        {
            return count;
        }

        if (CP == thai_get_rule(cell->unicode[1], wc)) // unicode[1]
        {
            cell->unicode[2] = wc; // unicode[2]
            cell->char_num++;
            count += 2;
        }
    }

    return count;
}

UINT32 thai_get_cell_size(UINT8 *ptext, UINT32 text_siz)
{
    UINT16  lead_wc = 0;
    UINT16  follow_wc = 0;
    UINT8   *pstr = NULL;
    UINT32  count = 0;

    if ((NULL == ptext)|| (0 == text_siz))
    {
        return count;
    }
    do
    {
        pstr = ptext + count;
        lead_wc = com_mb16to_word(pstr);
        follow_wc = com_mb16to_word(pstr + 2);
        count += 2;
    }
    while (CP == thai_get_rule(lead_wc, follow_wc));
    return count;
}

BOOL is_thai_unicode(UINT16 code)
{
    if ((code < 0x0E00) || (code > 0x0E7F))
    {
        return FALSE;
    }

    return TRUE;
}
