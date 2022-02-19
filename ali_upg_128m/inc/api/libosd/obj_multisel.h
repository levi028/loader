/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_multisel.h
*
*    Description: multisel object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_MULTISEL_H_
#define _OBJ_MULTISEL_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MULTISEL
{
    OBJECT_HEAD head;

    UINT8 b_align;
    //UINT8 bStyle;   // default: Horizontal, No arrow
    UINT8 b_x;       // X offset
    UINT8 b_y;
    UINT8 b_sel_type; // specifies string type of pSelTable

    void* p_sel_table;   // pointer type is determined by bSelType
                        // bSelType = STRING_ID: UINT16[dwTotal]
                        //            STRING_UNICODE: PWCHAR[dwTotal]
                        //            STRING_NUMBER: UINT32[3]
                        //                           pSelTable[0]:
                        //                              the minimum value
                        //                           pSelTable[1]:
                        //                              the maximum value
                        //                           pSelTable[2]: the step
                        //            STRING_NUM_TOTAL:
                        //                          the same as STRING_NUMBER,
                        //but display in format "value/total".
                        //            STRING_PROC: callback get string function
    INT16 n_count;      // ARRAY_SIZE(pSelTable) determined by bSelType

//    UINT16 wLeftIcon;
//    UINT16 wRightIcon;

//    UINT16 wLabelID;
//    PCLABEL_ATTR pLabel;

//    PLIST pList;  // to support drop-down list.

    INT16 n_sel;         // determined by bSelType
                        // bSelType = STRING_ID/STRING_UNICODE:
                        //              current sel to pSelTable
                        //            STRING_NUMBER/STRING_NUM_TOTAL:
                        //              the current value
                        //            STRING_PROC: current sel.
}MULTISEL, *PMULTISEL;

/////////////////////////////////////////////////
// attribute set functions
#define osd_set_multisel_style(s,style) \
    (s)->b_style = (UINT8)(style)

#define osd_set_multisel_xoffset(s,xoffset) \
    (s)->b_x = (UINT8)(xoffset)

#define osd_set_multisel_sel_type(s,sel_type) \
    (s)->b_sel_type = (UINT8)(sel_type)

#define osd_set_multisel_sel_table(s,sel_table) \
    (s)->p_sel_table = (sel_table)

#define osd_set_multisel_count(s,count) \
    (s)->n_count = (INT16)(count)

#define osd_set_multisel_sel(s,sel) \
    (s)->n_sel = (INT16)(sel)
/////////////////////////////////////////////////
// attribute get functions
#define osd_get_multisel_sel(s) \
    (s)->n_sel

#define osd_get_multisel_count(s) \
    (s)->n_count


void    osd_draw_multisel_cell(PMULTISEL p_ctrl, UINT8 b_style_idx,  UINT32 n_cmd_draw);
VACTION osd_multisel_key_map  (POBJECT_HEAD p_obj, UINT32 vkey);
PRESULT osd_multisel_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#ifdef __cplusplus
}
#endif
#endif//_OBJ_MULTISEL_H_
