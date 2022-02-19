/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: root.h
*
*    Description: This file contains application's root function - appinit().
                  It inits all modules used in the application and creates all the tasks
                  used. It also send a command to play the current tv program.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _ROOT_H_
#define _ROOT_H_
#ifdef __cplusplus
extern "C"{
#endif

extern struct vdec_device *g_decv_dev;
extern struct vdec_device *g_decv_avc_dev;
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
extern struct vbi_device *g_vbi_dev2; //vicky20101229 dbg
extern struct ge_device *g_ge_dev;
extern struct osd_device *g_osd_dev;
extern struct osd_device *g_osd_dev2;
extern struct sto_device *g_sto_dev;
extern struct vpo_device *g_vpo_dev;
extern struct vpo_device *g_sd_vpo_dev;
extern struct pan_device *g_pan_dev;
extern struct rfm_device *g_rfm_dev;
extern struct scart_device *g_scart_dev;
#if (defined(SUPPORT_BC)||defined(SUPPORT_BC_STD))
    extern struct smc_device *g_smc_dev;
#endif
#if (ISDBT_CC ==1)
    extern struct sdec_device *g_isdbtcc_dec_dev;
#endif


#ifdef  SUBT_FUNC_POINTER_ENABLE
extern osd_subt_create_region_t osd_subt_create_region;
extern osd_subt_delete_region_t osd_subt_delete_region;
extern osd_subt_region_show_t   osd_subt_region_show;
extern osd_subt_draw_pixel_t    osd_subt_draw_pixel;
extern osd_region_is_created_t  osd_region_is_created;
extern osd_subt_draw_pixelmap_t osd_subt_draw_pixelmap;
#endif
//for nim_m3101_init.c  so bad!!! just make it pass compile, no useful.

extern UINT32 app_uart_id;
extern int m_boot_vpo_active;

#ifdef TRUE_COLOR_HD_OSD
extern UINT32 ap_get_osd_scale_param(enum tvsystem e_tvmode, INT32 n_screen_width);
#endif
extern void system_hw_init(board_cfg *cfg);

extern enum output_frame_ret_code vdec_output_frame(struct display_info *p_display_info);
extern BOOL vdec_release_fr_buf(UINT8 top_index, UINT8 bottom_index);

// This function is defined in upgrade_serial.c --->
extern RET_CODE generate_stbinfo_hmac(UINT8 *input, const UINT32 len, UINT8 *output);
// < --- end

extern INT32 hdmi_tx_open(struct hdmi_device *dev);
extern void deca_m36_init_tone_voice(struct deca_device  *dev);
extern void snd_m36g_init_tone_voice(struct snd_device  *dev);
extern void snd_m36g_init_spectrum(struct snd_device  *dev);
#ifdef PSI_MONITOR_SUPPORT
extern void ap_pid_change(BOOL need_chgch);
#endif
extern RET_CODE verify_hdrchunk_hmac(UINT8 type);;
extern RET_CODE verify_allhdr_hmac(UINT8 type);
extern BOOL ap_task_init(void);
extern void tds_do_global_ctors(void);

#ifdef __cplusplus
 }
#endif
#endif//_ROOT_H_
