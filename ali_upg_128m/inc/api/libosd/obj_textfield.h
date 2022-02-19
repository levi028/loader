/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_textfield.h
*
*    Description: implement textfield object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_TEXTFIELD_H_
#define _OBJ_TEXTFIELD_H_

#include <api/libosd/osd_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

// String Type Definition


typedef struct _TEXT_FIELD
{
    OBJECT_HEAD head;
    UINT8 b_align;
//    UINT8 bStyle;
    INT32 b_x;
    INT32 b_y;
    UINT16 w_string_id;  // string id
//    UINT16 wIconID;
//    OSD_RECT rcIcon;
    UINT16* p_string;   // unicode string pointer
//    UINT16 nSize;   // pString Buffer size in WCHAR
}TEXT_FIELD, *PTEXT_FIELD;

void osd_set_text_field_str_point(PTEXT_FIELD p_ctrl, UINT16*    p_str);
void osd_set_text_field_content(PTEXT_FIELD p_ctrl, UINT32 string_type, UINT32 value);

void osd_draw_text_field_cell(PTEXT_FIELD p_ctrl,UINT8 b_style_idx, UINT32 n_cmd_draw);

VACTION osd_text_field_key_map (POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_text_field_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1);


#ifdef __cplusplus
}
#endif
#endif//_OBJ_TEXTFIELD_H_
