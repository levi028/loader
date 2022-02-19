/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_matrixbox.h
*
*    Description: matrixbox object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _OBJ_MATRIXBOX_H_
#define _OBJ_MATRIXBOX_H_
#ifdef __cplusplus
extern "C" {
#endif

//#include <basic_types.h>
//#include <api/libosd/osd_lib.h>

#define DT_LEFT     0x00    // H left, default
#define DT_VCENTER  0x00    // V center, default
#define DT_CENTER   0x01    // H center
#define DT_RIGHT    0x02    // H right
#define DT_TOP      0x04    // V top
#define DT_BOTTOM   0x08    // V bottom
#define DT_BORDER   0x20    // draw picture and fill rect with background color
#define DT_MIXER    0x40    // if DT_MIXER specified, not draw transparent color
                            // in picture;
                            // otherwise, draw transparent color with background
                            // color.

#define ITEM_SELECTED           0x80
#define ITEM_UNSELECTED         0x7f
typedef struct _MATRIX_BOX_NODE
{
    UINT32      str; // strid or *str
    UINT16      num; // num or iconid
    UINT8       b_attr;                  // bit 0 -> 1 Active, 0->inactive.
                                        //Definition is same as
//  bAction     7       6-1     0
//      0       unselected          inactive
//      1       selected                active
}MATRIX_BOX_NODE, *PMATRIX_BOX_NODE;

#define MATRIX_TYPE_NULL            0x00
#define MATRIX_TYPE_WCHAR       0x01
#define MATRIX_TYPE_STRID       0x02
#define MATRIX_TYPE_NUMBER      0x04
#define MATRIX_TYPE_BITMAP      0x08

// MatrixBox style: bit(0)=padding mode, bit(1-2)=row mode, bit(3)=col mode
#define MATRIX_DEFAULT          0x00    // padding, row loop, col loop
#define MATRIX_NO_PADDING       0x01    // don't draw padding cell
#define MATRIX_ROW_WRAP         0x02    // wrap to next row
#define MATRIX_ROW_STAY         0x04    // don't move when out of row range
#define MATRIX_COL_STAY         0x08    // don't move when out of col range
#define MATRIX_STYLE_NMN        0x00
#define MATRIX_STYLE_4A1        0x10
#define MATRIX_STYLE_5A1        0x20
#define MATRIX_STYLE_12A1       0x40
#define MATRIX_STYLE_MODE(style)    ((UINT8)(style) & 0x60)
#define MATRIX_PADDING_MODE(style)  ((UINT8)(style) & 0x01)
#define MATRIX_ROW_MODE(style)      ((UINT8)(style) & 0x06)
#define MATRIX_COL_MODE(style)      ((UINT8)(style) & 0x08)
//#define MATRIX_STYLE_DEFAULT            0x00    // default: padding, line wrap
//#define MATRIX_STYLE_LINEWRAP           0x01    // loop to next line
//#define MATRIX_STYLE_LOOP_HOR_DISABLE   0x02    // hor loop disable
//#define MATRIX_STYLE_DRAW_AFTER_CHANGE  0x04    // draw item after calling
                                                  // change proc

typedef struct _MATRIX_BOX
{
    OBJECT_HEAD head;
    UINT8 b_style;
    UINT8 b_cell_type;
    UINT16 n_count;
    PMATRIX_BOX_NODE p_cell_table;

    UINT16 n_row;
    UINT16 n_col;

    UINT8 b_assign_type;
    INT16 b_interval_x;
    INT16 b_interval_y;
    INT8 b_interval_tt;
    INT8 b_interval_tl;
    INT8 b_interval_it;
    INT8 b_interval_il;
//  INT8 bBackColor;
    UINT16 n_pos;            // the item index of current position
}MATRIX_BOX, *PMATRIX_BOX;

#define osd_set_matrix_box_tshift(mb,x, y)   \
    do{ \
        ((PMATRIX_BOX)(mb))->b_interval_tl = (INT8)(x); \
        ((PMATRIX_BOX)(mb))->b_interval_tt = (INT8)(y); \
    }while(0)

#define osd_set_matrix_box_ishift(mb,x, y)   \
    do{ \
        ((PMATRIX_BOX)(mb))->b_interval_il = (INT8)(x); \
        ((PMATRIX_BOX)(mb))->b_interval_it = (INT8)(y); \
    }while(0)

VACTION osd_matrixbox_key_map(POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_matrixbox_proc(POBJECT_HEAD p_obj_head, UINT32 msg_type, UINT32 msg,UINT32 param1);
void osd_draw_matrix_box_cell(PMATRIX_BOX mb, UINT8 b_style_idx, UINT32 n_cmd_draw);
void osd_get_mtrx_box_cell_location(PMATRIX_BOX m, UINT16 n_pos, UINT16 *p_col, UINT16 *p_row);
UINT16 osd_get_mtrxbox_cur_pos(PMATRIX_BOX m);
void osd_set_mtrxbox_cur_pos(PMATRIX_BOX m, UINT16 n_pos);
void osd_set_mtrxbox_count(PMATRIX_BOX m, UINT16 count, UINT16 row, UINT16 col);
PRESULT osd_shift_matrix_box_sel_hor(PMATRIX_BOX m, short s_shift_x);
PRESULT osd_shift_matrix_box_sel_ver(PMATRIX_BOX m, short s_shift_x);
UINT16 osd_get_word_len(UINT8 *p_text, UINT8 font,UINT16 *wordwidth, UINT16 *wordheight ,UINT16 width);

#ifdef __cplusplus
}
#endif
#endif
