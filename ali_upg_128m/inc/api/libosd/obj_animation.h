/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_animation.h
*
*    Description: the struct of animation object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _OBJ_ANIMATION_H_
#define _OBJ_ANIMATION_H_
#ifdef __cplusplus
extern "C" {
#endif
/////////////////   ANIMATION   ////////////////////////////////
#define OSD_ANM_REFRESH         0

typedef void(*anm_callback_func)(unsigned long type, unsigned long param);

typedef struct _ANIMATION_
{
    OBJECT_HEAD     head;
    UINT8 b_align;
    UINT8 b_x;
    UINT8 b_y;
    UINT8 b_moving_flag; //1:move, 0:still
    UINT16 *p_frame; //pointer of frameID array
    UINT8 b_cur_frame_num; //0~(bAllFrameNum-1)
    UINT8 b_all_frame_num;
    UINT32 dw_interval;  //bFPS;     //frame per second
    ID timer_id;
    anm_callback_func anm_callback;
} ANIMATION,*PANIMATION;


#define osd_animation_set_moving(p_ctrl) \
    (p_ctrl)->b_moving_flag = TRUE

#define osd_animation_clear_moving(p_ctrl) \
    (p_ctrl)->b_moving_flag = FALSE

#define osd_animation_set_cur_frame(p_ctrl, b_cur_frame) \
    (p_ctrl)->b_cur_frame_num = (b_cur_frame)

void osd_animation_start_timer(PANIMATION p_ctrl);
void osd_animation_stop_timer(PANIMATION p_ctrl);
void osd_animation_refresh(PANIMATION p_ctrl);
//draw animation
void osd_draw_animation_cell(PANIMATION p_ctrl,UINT8 b_style_idx,UINT32 n_cmd_draw);

//key map
VACTION osd_animation_key_map(POBJECT_HEAD p_obj,  UINT32 vkey);

//proc
PRESULT osd_animation_proc(POBJECT_HEAD p_obj, UINT32 msg_type, UINT32 msg, UINT32 param1);

#ifdef __cplusplus
}
#endif
#endif//_OBJ_ANIMATIN_H_


