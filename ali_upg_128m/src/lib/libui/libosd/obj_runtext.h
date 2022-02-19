/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: obj_runtext.h
*
*    Description: implement runner text object.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OBJ_RUNTEXT_H_
#define _OBJ_RUNTEXT_H_
#ifdef __cplusplus
extern "C"
{
#endif

//#define TXT_RUNNER_SUPPORT
#ifdef TXT_RUNNER_SUPPORT

struct RUNTXTMAP
{
   // struct _TEXT_FIELD *run_head;
  //  struct _TEXT_FIELD org_head;
    POBJECT_HEAD run_head;
    UINT8 run_obj_mode;
    UINT8 offset_step;
    UINT16 offset_value;
    UINT16 total_strlen;
};

void set_osd_txt_runner_obj(struct RUNTXTMAP  p_obj1,struct RUNTXTMAP  p_obj2 , struct RUNTXTMAP p_obj3);
void get_osd_txt_runner_text_field_width(UINT8 index,struct osdrect *p_rect1);

void start_txt_runner(void);

void close_osd_txt_runner_obj(void);

#endif


#ifdef __cplusplus
}
#endif
#endif


