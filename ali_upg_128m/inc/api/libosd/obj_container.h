/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_container.h
*
*    Description: define the struct of container.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _OBJ_CONTAINER_H_
#define _OBJ_CONTAINER_H_

#include <api/libosd/osd_lib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    DRAW_STYLE_ONE = 0,
    DRAW_STYLE_FADE,
    DRAW_STYLE_SPHERE,
    DRAW_STYLE_ROTATE,
    DRAW_STYLE_SLIDE,
    DRAW_STYLE_ZOOM,
    DRAW_STYLE_FLIP,
    DRAW_STYLE_MOVE
}DRAW_STYLE;
/////////////////   CONTAINER   ////////////////////////////////
typedef struct _CONTAINER_
{
    OBJECT_HEAD     head;
    POBJECT_HEAD    p_next_in_cntn;      //point to objects included in this
                                      //container; container<-obj1<-obj2
    UINT8           focus_object_id;    //which object in container which is
                                      //focused
    UINT8           b_hilite_as_whole;   //when hl is on container,wheather to
                                      //highlight all objects included in this
                                      //container
                                      //Alan extend bHiliteAsWhole high 6bit
                                      //using 3D OSD effect

} CONTAINER,*PCONTAINER;


//draw container
void osd_draw_container_cell(PCONTAINER p_ctrl,UINT8 b_style_idx,UINT32 n_cmd_draw);

//key map
VACTION osd_container_key_map(POBJECT_HEAD p_obj,  UINT32 vkey);

//proc
PRESULT osd_container_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1);

PRESULT osd_container_chg_focus(CONTAINER *c, UINT8 b_new_focus_id,UINT32 parm);

//set contianer foucus
#define osd_set_container_focus(c, id)    \
    (c)->focus_object_id = (id)
//get focus id
#define osd_get_container_focus(c)    \
    ((c)->focus_object_id)

#define osd_get_container_next_obj(c)  \
    (c)->p_next_in_cntn

#define osd_set_container_next_obj(c,p_next_in_cont)  \
    (c)->p_next_in_cntn = (POBJECT_HEAD)p_next_in_cont;

#ifdef __cplusplus
}
#endif

#endif//_OBJ_CONTAINER_H_

