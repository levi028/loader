/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_editfield.h
*
*    Description: editfield object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_EDITFIELD_H_
#define _OBJ_EDITFIELD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <api/libsi/si_tdt.h>
//for bStyle
#define NORMAL_EDIT_MODE 0x00
#define SELECT_EDIT_MODE  0x10
#define SELECT_STATUS 0x0
#define EDIT_STATUS 0x1
#define FORCE_TO_VALBAK 0x2

#define EDITFIELD_SUPPORT_TIME  1

typedef enum
{
    CURSOR_NO = 0,      /* No cursor, can change immediately: eg PID*/
    CURSOR_NORMAL,     /* Cursor under the char, length must be fixed */
    CURSOR_SPECIAL,    /* Only EDIT mode and not reach to maxlen has cursor '_'*/
}CUSOR_MODE;

typedef struct _EDIT_FIELD
{
    OBJECT_HEAD head;
    UINT8 b_align;
    UINT8 b_x;
    UINT8 b_y;
    UINT8 b_max_len;

    char* pcs_pattern; // fraction: "fXY" - X: integer width, Y: fraction width
                      // password: "pIML" - I: the initial char for password
                      // M: the masked char for password
                      // L: the password length
                      // range: "rNNNNN~MMMMM" - NNNNN: the minimum value
                      //                         MMMMM: the maximum value
                      // date: "dFS" - F: format index, S: seperator index
                      //       F = 0: yyyy-mm-dd
                      //           1: yy-mm-dd
                      //           2: mm-dd-yyyy
                      //           3: mm-dd-yy
                      //           4: dd-mm-yyyy
                      //       S = 0: '.'
                      //           1: '-'
                      //           2: '/'
                      // time: "t0" - hh:mm:ss
                      //       "t1" - hh:mm
                      //       "t2" - mm:ss
                      //        "T0" - hh:mm:ss AM/PM
                      //        "T1" - hh:mm AM/PM
                      //        "T2" - mm:ss AM/PM
                      // string: "sLLLLL" - LLLLL: the string maximum length
                      //(advanced feature)
                      // ip address: "iS"  S = 0: '.', 1: '-', 2: '/'
                      // mask data, "mIMLL"
                      //    I: the initial char for mask
                      //    M: the masked char for password
                      //    LL: the mask max length
//    UINT16 wSuffixID; //??
//    UINT16 wLabelID;  //??
//    PCLABEL_ATTR pLabel;  //??

    UINT8 b_style;  // SELECT_STATUS / EDIT_STATUS | NORMAL_EDIT_MODE /
                   // SELECT_EDIT_MODE
    UINT8 b_cursor_mode;      // cursor mode
    UINT8 b_cursor;
    UINT16 *p_string;
    UINT16 *p_prefix;   //eg. E 168
    UINT16 *p_suffix;    //eg. xxx MHz
    UINT32  valbak;
}EDIT_FIELD, *PEDIT_FIELD;

/////////////////////////////////////////////////
// attribute set functions
#define osd_set_edit_field_pattern(e,pattern) \
    (e)->pcs_pattern = (pattern)

#define osd_set_edit_field_prefix_point(e,prefix) \
    (e)->p_prefix = (prefix)

#define osd_set_edit_field_suffix_point(e,suffix) \
    (e)->p_suffix = (suffix)

#define osd_set_edit_field_style(e,style) \
    (e)->b_style = (UINT8)(style)

#define osd_set_edit_field_cursor_mode(e,cursormode) \
    (e)->b_cursor_mode = (UINT8)(cursormode)

#define osd_set_edit_field_cursor(e,cursor) \
    (e)->b_cursor = (UINT8)(cursor)

#define osd_set_edit_field_str_point(e,str) \
    (e)->p_string = (UINT16 *)(str)

#define osd_set_edit_field_valbak(e,val_bak) \
    (e)->valbak = (UINT32)(val_bak)

#define osd_get_edit_field_mode(e) \
    ( (e)->b_style & SELECT_EDIT_MODE)

#define osd_get_edit_field_status(e)   \
    ( (e)->b_style & EDIT_STATUS)

#define osd_get_edit_field_default_val(e)   \
    ((e)->b_style & FORCE_TO_VALBAK)

void    osd_draw_edit_field_cell(PEDIT_FIELD p_ctrl, UINT8 b_style_idx,
    UINT32 n_cmd_draw);
VACTION osd_edit_field_key_map (POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_edit_field_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,
    UINT32 param1);


BOOL osd_set_edit_field_content(PEDIT_FIELD p_ctrl, UINT32 string_type,
    UINT32 dw_value);
BOOL osd_set_edit_field_prefix (PEDIT_FIELD p_ctrl, UINT32 string_type,
    UINT32 dw_value);
BOOL osd_set_edit_field_suffix (PEDIT_FIELD p_ctrl, UINT32 string_type,
    UINT32 dw_value);

BOOL osd_get_edit_field_int_value(PEDIT_FIELD p_ctrl,UINT32 *p_value);
BOOL osd_get_edit_field_time_date(PEDIT_FIELD p_ctrl,date_time* dt);
UINT32 osd_get_edit_field_content(PEDIT_FIELD p_ctrl);

#ifdef __cplusplus
}
#endif
#endif//_OBJ_EDITFIELD_H_
