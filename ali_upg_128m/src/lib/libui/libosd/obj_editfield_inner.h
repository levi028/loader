/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_editfield_inner.h
*
*    Description: editfield object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _OBJ_EDITFIELD_INNER_H_
#define _OBJ_EDITFIELD_INNER_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define C_MAX_EDIT_LENGTH   32
#define C_TIME_FMT_OFFWSET  5// 4
#define C_TIME_SEP          3


typedef struct _EDIT_PATTERN
{
    UINT8 b_tag;
    UINT8 b_len;         // length without seperator
    UINT8 b_max_len;      // length with seperator
    UINT16 w_cursor_map;
    union
    {
        struct
        {
            INT8 b_sub;
        } f;

        struct
        {
            UINT8 b_init;
            UINT8 b_mask;
        } p;

        struct
        {
            UINT8 b_format;
            UINT8 b_sep;
        } dt;

        struct
        {
            INT32 n_min;
            INT32 n_max;
        } r;

        struct
        {
            UINT16 *p_string;
        } s;

        struct
        {
            UINT8 b_format;
            UINT8 b_sep;
        } ip;

        struct
        {
            UINT8 b_init;
            UINT8 b_mask;
        } m;
    };
} EDIT_PATTERN, *PEDIT_PATTERN;




#ifdef __cplusplus
}
#endif
#endif
