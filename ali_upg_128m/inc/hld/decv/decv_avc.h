/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: decv_avc.h
*
*    Description: This file contains definitions of avc decoder's api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _DECV_AVC_H_
#define  _DECV_AVC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <basic_types.h>
#include <mediatypes.h>
#include <sys_config.h>

struct vdec_avc_memmap
{
    BOOL support_multi_bank;

    UINT32 frame_buffer_base;
    UINT32 frame_buffer_len;

    UINT32 dv_frame_buffer_base;
    UINT32 dv_frame_buffer_len;

    UINT32 mv_buffer_base;
    UINT32 mv_buffer_len;

    UINT32 mb_col_buffer_base;
    UINT32 mb_col_buffer_len;

    UINT32 mb_neighbour_buffer_base;
    UINT32 mb_neighbour_buffer_len;

    UINT32 cmd_queue_buffer_base;
    UINT32 cmd_queue_buffer_len;

    UINT32 vbv_buffer_base;
    UINT32 vbv_buffer_len;

    UINT32 laf_rw_buf;
    UINT32 laf_rw_buffer_len;

    UINT32 laf_flag_buf;
    UINT32 laf_flag_buffer_len;

    BOOL   support_conti_memory;
    UINT32 avc_mem_addr;
    UINT32 avc_mem_len;
    UINT32 auxp_addr;
};

struct vdec_avc_config_par
{
    struct vdec_avc_memmap memmap;
    UINT32 max_additional_fb_num;
    UINT8 dtg_afd_parsing;
};

struct vdec_init_vbv_snd_par
{
    UINT32 vbv_start;
    UINT32 vbv_end;
    UINT32 snd_start;
    UINT32 snd_end;
};


void vdec_avc_attach(struct vdec_avc_config_par *pconfig_par);
void vdec_init_vbv_snd(struct vdec_init_vbv_snd_par *pconfig_par);
#ifdef __cplusplus
}
#endif

#endif  /* _DECV_H_*/

