/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OBJ_TEXTFIELD_H_
#define _OBJ_TEXTFIELD_H_

typedef struct _TEXT_FIELD
{
    OBJECT_HEAD head;
    UINT8 b_align;
    UINT8 b_x;
    UINT8 b_y;
    UINT16 w_string_id;  // string id
    UINT16* p_string;   // unicode string pointer

    UINT32 slave;
}TEXT_FIELD, *PTEXT_FIELD;

typedef struct SLAVE_TXTFLD
{
    GUI_RECT  frame;
    COLOR_STYLE style;
    UINT8 b_x;
    UINT8 b_y;

    UINT16* p_string;   // unicode string pointer
}SDTXT, *PSDTXT;

void osd_set_text_field_str_point(PTEXT_FIELD p_ctrl, UINT16*    p_str);
void osd_set_sdtxt_content(PSDTXT p_ctrl, UINT32 string_type, UINT32 value);
void osd_set_text_field_content(PTEXT_FIELD p_ctrl, UINT32 string_type, UINT32 value);
void osd_draw_text_field_cell(PTEXT_FIELD p_ctrl,UINT8 b_style_idx,UINT32 n_cmd_draw);
VACTION osd_text_field_key_map    (POBJECT_HEAD p_obj,    UINT32 vkey);
PRESULT osd_text_field_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#define osd_set_sdtxt_str_point(txt,str)    \
    do{(txt)->p_string = (UINT16 *)str;}while(0)
#define osd_set_text_field_str_id(txt,id)    \
    do{(txt)->w_string_id = (UINT16)id;}while(0)
#define osd_set_text_field_str_point(txt,str)    \
    do{(txt)->p_string = (UINT16 *)str;}while(0)

#define osd_get_text_field_str_id(txt)    \
    (txt)->w_string_id
#define osd_get_text_field_str_point(txt)    \
    (txt)->p_string

#endif//_OBJ_TEXTFIELD_H_
