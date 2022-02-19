/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_lib_internel.h
*
*    Description: OSD object management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OSD_LIB_INTERNAL_H_
#define _OSD_LIB_INTERNAL_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "osd_common_draw_base.h"

#ifndef OSD_VSRC_MEM_MAX_SIZE
#define OSD_VSRC_MEM_MAX_SIZE   (30*1024)
#endif

#ifndef OSD_TRANSPARENT_COLOR_BYTE
#define OSD_TRANSPARENT_COLOR_BYTE 0
#endif

#ifndef OSD_TRANSPARENT_COLOR
#define OSD_TRANSPARENT_COLOR   0
#endif

#ifndef FACTOR
#define FACTOR 0
#endif

#ifndef COLOR_N
#define COLOR_N 256
#endif

#ifndef BIT_PER_PIXEL
#define BIT_PER_PIXEL   8
#endif

#define CIRCLE_PIX 5

#define CHECK_LEAVE_RETURN(result,p_obj) \
    if((result) == PROC_LEAVE)      \
    {   \
        if(NULL == (p_obj)->p_root)   \
            goto CLOSE_OBJECT;      \
        else                        \
            goto EXIT;              \
    }

BOOL    osd_event_pass_to_child(UINT32 msg,UINT32* submsg);
//common object proc function
PRESULT osd_obj_common_proc(POBJECT_HEAD p_obj, UINT32 hkey, UINT32 param,
                          UINT32* pvkey,VACTION* pvact,BOOL* b_continue);

#ifdef __cplusplus
}
#endif
#endif//_OSD_LIB_INTERNAL_H_


