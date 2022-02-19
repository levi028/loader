/*-----------------------------------------------------------------------------
*    Copyright (C) 2010 ALI Corp. All rights reserved.
*    GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OBJ_CONTAINER_H_
#define _OBJ_CONTAINER_H_

typedef struct _CONTAINER_{
    OBJECT_HEAD        head;
    POBJECT_HEAD    p_next_in_cntn;        //point to objects included in this container; container<-obj1<-obj2
    UINT8            focus_object_id;        //which object in container which is focused
    UINT8            b_hilite_as_whole;        //when hl is on container,wheather to highlight all objects included in this container
    UINT32            slave;
} CONTAINER,*PCONTAINER;

typedef struct SLAVE_CONTAINER_{
    GUI_RECT  frame;
    COLOR_STYLE style;
} SDCON,*PSDCON;

void osd_draw_container_cell(PCONTAINER p_ctrl,UINT8 b_style_idx,UINT32 n_cmd_draw);

//key map
VACTION osd_container_key_map(POBJECT_HEAD p_obj,    UINT32 vkey);

//proc
PRESULT osd_container_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg,UINT32 param1);

PRESULT osd_container_chg_focus(CONTAINER    *c, UINT8 b_new_focus_id,UINT32 parm);

//set contianer foucus
#define osd_set_container_focus(c, id)    \
    (c)->focus_object_id = (id)
//get focus id
#define osd_get_container_focus(c)    \
    ((c)->focus_object_id)

#define osd_get_container_next_obj(c)    \
    (c)->p_next_in_cntn

#define osd_set_container_next_obj(c,p_next_in_cont)    \
    (c)->p_next_in_cntn = (POBJECT_HEAD)p_next_in_cont;

#endif//_OBJ_CONTAINER_H_

