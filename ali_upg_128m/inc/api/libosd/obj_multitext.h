/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_multitext.h
*
*    Description: multitext object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_MULTITEXT_H_
#define _OBJ_MULTITEXT_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TEXT_CONTENT
{
    UINT8 b_text_type; // specifies string type of pText
    union _text
    {
        UINT16 w_string_id;  //STRING_ID
        UINT16* p_string;   //STRING_UNICODE
    }text;
}TEXT_CONTENT, *PTEXT_CONTENT;

typedef struct _MULTI_TEXT
{
    OBJECT_HEAD head;
    UINT8 b_align;
    UINT8 b_count;  // ARRAY_SIZE(pTextTable)
    UINT16 n_line;  // current first line idx for display
    OSD_RECT rc_text;
    SCROLL_BAR      *scroll_bar;
    PTEXT_CONTENT p_text_table;
}MULTI_TEXT, *PMULTI_TEXT;

#define MAX_LINE_NUM    200

typedef struct
{
    UINT8   total_lines;
    UINT8   page_lines;
    UINT8   topline_idx;
    UINT8   topline_tblidx;
    UINT8  *topline_pstr;
    UINT8   line_height[MAX_LINE_NUM];
    UINT16  total_height;
    UINT16  page_height;
}mtxtinfo_t;

/////////////////////////////////////////////////
// attribute set functions
#define osd_set_multi_text_line(mt,line) \
    (mt)->n_line = (UINT16)(line)

void osd_draw_multi_text_cell(PMULTI_TEXT p_ctrl, UINT8 b_style_idx, UINT32 n_cmd_draw);
VACTION osd_multi_text_key_map (POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_multi_text_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1);
UINT16 osd_get_text_total_line(UINT8 *p_text,UINT8 font,UINT16 width,
                            INT16 lineidx,UINT8 **line_str, UINT8 *linesheght);
void osd_get_mtext_infor(PMULTI_TEXT p_ctrl,mtxtinfo_t *p_info);
void osd_set_multi_text_content(PMULTI_TEXT p_ctrl, PTEXT_CONTENT p_text_table);

#ifdef __cplusplus
}
#endif
#endif//_OBJ_MULTITEXT_H_

