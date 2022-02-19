/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dev_handle
*
*    Description: the define of dev handle
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _DEV_HANDLE_H_
#define _DEV_HANDLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern struct vdec_device *g_decv_dev;
extern struct vdec_device *g_decv_dev2;
extern struct deca_device *g_deca_dev;
extern struct dmx_device *g_dmx_dev;
extern struct dmx_device *g_dmx_dev2;
extern struct dmx_device *g_dmx_dev3;
extern struct nim_device *g_nim_dev;
extern struct nim_device *g_nim_dev2;
extern struct snd_device *g_snd_dev;
extern struct sdec_device *g_sdec_dev;
extern struct vbi_device *g_vbi_dev;
extern struct ge_device *g_ge_dev;
extern struct osd_device *g_osd_dev;
extern struct osd_device *g_osd_dev2;
extern struct sto_device *g_sto_dev;
extern struct vpo_device *g_vpo_dev;
extern struct rfm_device *g_rfm_dev;
extern struct pan_device *g_pan_dev;
extern struct scart_device *g_scart_dev;
extern struct vpo_device *g_sd_vpo_dev;
extern struct vdec_device *g_decv_avc_dev;
extern struct vdec_device *g_decv_hevc_dev;
#ifdef __cplusplus
}
#endif

#endif//_DEV_HANDLE_H_

