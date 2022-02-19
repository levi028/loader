/*****************************************************************************
*
*  File Name: decv_hevc.h
*
*  Description:
*  Hurricane high efficient video buffer  memory mapping configuration head file
*  Define different buffer  memory base addr/size for Hurricane hevc engine driver
*
* Author: CHEN <CHEN@Alitech.com>
*
*  Create date : Apri. 03, 2015
*
**********************************************************************************
*
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* ALi Corporation. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by ALi Corporation.                  *
*                                                                               *
* ALi Corporation reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright ALi Corporation 2015.                                                     *
* Last Revision: Jan. 2015                                                      *
*********************************************************************************
*
*  Revision history:
*  --------------------------------------------------------------------------------
*  Date                                 Modification                Revision                Name
*  --------------------------------------------------------------------------------
*  Apri. 03, 2015                   Created                     V0.1                      CHEN

 ****************************************************************************/

#ifndef  _DECV_HEVC_H_
#define  _DECV_HEVC_H_

#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>

/*!
@struct  vdec_hevc_memmap
@brief  Define the memory mapping configuration  for High Efficiency Video decoder
*/
typedef struct
{
    BOOL support_multi_bank;
    BOOL support_conti_memory;          //!< if system support continual memory allocation mode

    UINT32 mp_frame_buffer_base;            //!< Frame buffer base address for main picture video
    UINT32 mp_frame_buffer_len;               //!< Frame buffer length for main picture video

    UINT32 dv_frame_buffer_base;             //!< Frame buffer base address for decimated picture video
    UINT32 dv_frame_buffer_len;                //!< Frame buffer length for decimated picture video

    UINT32 collocated_mv_buffer_base;     //!< Collocated MV buffer base address
    UINT32 collocated_mv_buffer_len;        //!< Collocated MV buffer length

    UINT32 ph_buffer_base;                         //!< Picture Header Syntax buffer base address
    UINT32 ph_buffer_len;                            //!< Picture Header Syntax buffer length

    UINT32 inner_es_buffer_base;              //!< INNER Elementary Stream buffer base address
    UINT32 inner_es_buffer_len;                 //!< INNER Elementary Stream buffer length

    UINT32 inner_aux_buffer_base;            //!< INNER Auxilary buffe buffer base address
    UINT32 inner_aux_buffer_len;               //!< INNER Auxilary buffe buffer length

    UINT32 neighbour_buffer_base;            //!< Neighbour buffer base address
    UINT32 neighbour_buffer_len;               //!< Neighbour buffer length

    UINT32 ep_cmd_queue_buffer_base;    //!< EP command queue buffer base address
    UINT32 ep_cmd_queue_buffer_len;       //!< EP command queue buffer length

    UINT32 md_cmd_queue_buffer_base;   //!< MD command queue buffer base address
    UINT32 md_cmd_queue_buffer_len;      //!< MD command queue buffer length

    UINT32 vbv_buffer_base;                        //!< VBV buffer base address
    UINT32 vbv_buffer_len;                           //!< VBV buffer length

    UINT32 hevc_mem_addr;                        //!<  memory base address for High Efficiency Video decoder based on system support continual memory allocation mode
    UINT32 hevc_mem_len;                          //!< memory base address length

	UINT32 laf_buffer_base;
	UINT32 laf_buffer_len;
}vdec_hevc_memmap;


struct vdec_hevc_config_par
{
    char *decv_hevc_inst_name;        //!< H265 decoder instance name, support multi-instance based on different name
    vdec_hevc_memmap memmap;          //!< H265 memory mapping of the buffers
    UINT32 max_additional_fb_num;
	UINT8 dtg_afd_parsing;
    UINT8 dev_num;
};

void hevc_config(struct vdec_hevc_config_par *par);

void vdec_hevc_attach(struct vdec_hevc_config_par *pconfig_par);

#endif  /* _DECV_HEVC_H_*/

