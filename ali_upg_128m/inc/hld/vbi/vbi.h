/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi.h

   *    Description:define the MACRO, the variable and structure uisng by VBI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    __HLD_VBI_H__
#define __HLD_VBI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <hld/vbi/vbi_dev.h>

#define VBI_LINE_MIN_NUM 16
#define VBI_LINE_MAX_NUM 550

#if 1//(!defined(DUAL_ENABLE)||(defined(DUAL_ENABLE) && !defined(MAIN_CPU)))
extern UINT8 ttx_eng_state;
extern UINT8 transparent_level;
extern UINT8 *subpage_p26_nation;//to store subpage p26 naton map
extern struct ttx_config_par g_ttx_pconfig_par;
extern UINT32 osd_ttx_width;
extern UINT32 osd_ttx_xoffset;
extern UINT32 osd_ttx_yoffset;
//extern UINT8 mosaic_mode;//1:continue mode,  0:separate mode
#endif
//extern UINT8 transparent_value[TTX_TRANSPARENT_LEVEL];

//extern UINT8 seperate_ttxsubtitle_mode;

INT32 vbi_open(struct vbi_device *dev);
INT32 vbi_close(struct vbi_device *dev);
INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param);
INT32 vbi_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,
                        UINT32 *pu_size_got, struct control_block *p_data_cb);
void vbi_update_write(void *pdev, UINT32 u_data_size);
void vbi_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request);
INT32 vbi_start(struct vbi_device *dev,t_ttxdec_cbfunc p_cb_func);
INT32 vbi_stop(struct vbi_device *dev);
RET_CODE vbi_io_control(struct vbi_device *dev, UINT32 cmd, UINT32 param);
INT32 ttx_request_page(struct vbi_device *dev, UINT16 page_id , struct PBF_CB ** cb );
INT32 ttx_request_page_up(struct vbi_device *dev,UINT16 page_id, struct PBF_CB ** cb );
INT32 ttx_request_page_down(struct vbi_device *dev, UINT16 page_id , struct PBF_CB ** cb );
void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set);
INT32 vbi_attach(struct vbi_config_par *cfg_param);
void vbi_hld_enable_ttx_by_osd(struct vbi_device *pdev);
void enable_hld_vbi_transfer(BOOL enable);

void ttx_eng_para_init(void);
void ttx_check_seperate_ttxsubt_mode(UINT8 *mode);
INT32 ttx_alloc_subpage_p26_naton_buffer(void);

/**** just want to clear warning ****/
extern INT32 vbi_m33_attach(struct vbi_config_par *config_par);
extern void vbi_enable_ttx_by_osd(struct vbi_device*dev);
extern void enable_vbi_transfer(BOOL enable);
extern void ttx_eng_init_para_set(void);
extern void check_seperate_ttxsubt_mode(UINT8 *mode);
extern INT32 alloc_subpage_p26_naton_buffer(void);

#ifdef __cplusplus
}
#endif

#endif /*__HLD_VBI_H__*/
