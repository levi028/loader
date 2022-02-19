/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt_internal.h
   *
   *    Description: The file is mainly to declare the function and variable of
        the DVB subtitle.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __LIB_SUBT_INTERNAL_H__
#define __LIB_SUBT_INTERNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <hld/osd/osddrv_dev.h>
#include <hld/sdec/sdec.h>
#include <api/libsubt/subt_osd.h>

#define ATSC_SUBT_INVALID_PID 0x1fff
#define ATSC_SUBT_PAST_THRESHOLD 0x80000000
#define ON_COLOR_INDEX ATSC_SUBT_CHAR_COLOR_INDEX // 0x9f
#define OFF_COLOR_INDEX OSD_TRANSPARENT_COLOR //0x22

#define FREE_RUN_THRESHOLD      (9000*90)   // 9000 ms
#define SYNC_OK_THRESHOLD       (10*90)     // 10 ms

//#define SCTE_SDEC_STATE_EMPT        0
#define SCTE_SDEC_STATE_IDLE        1
#define SCTE_SDEC_STATE_PLAY        2
#define SCTE_SDEC_CMD_START             0x00000001
#define SCTE_SDEC_CMD_STOP              0x00000002
#define SCTE_SDEC_MSG_START_OK          0x00000004
#define SCTE_SDEC_MSG_STOP_OK           0x00000008
#define SCTE_SDEC_MSG_DATA_AVAILABLE    0x00000010

#define SUBT_RET_FAILURE 1

#define ATSC_RET_FAILURE 0xffffffff
#if 1//(!defined(DUAL_ENABLE)||(defined(DUAL_ENABLE) && !defined(MAIN_CPU)))
struct subt_config_par g_subt_config_par;
extern BOOL g_subt_show_on;
BOOL g_subt_init;
#endif

//extern void lib_subt_atsc_attach(void *p_config);
extern void lib_subt_atsc_start_filter(UINT16 dmx_id,int pid);
extern void lib_subt_atsc_stop_filter(UINT16 dmx_id);
extern void lib_subt_atsc_set_buf(struct atsc_subt_config_par *p_config);
extern void lib_subt_atsc_clean_up(void);
extern void lib_subt_atsc_delete_timer();
//extern BOOL lib_subt_atsc_check_crc( UINT8 *data, UINT32 data_len, UINT32 crc);
extern UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data);
//extern UINT32 lib_subt_atsc_task_buffer_init(UINT32 param);
extern void lib_subt_atsc_show_onoff(BOOL onoff);
extern BOOL lib_subt_atsc_create_task(void);
extern BOOL lib_subt_atsc_terminate_task(void);
extern BOOL lib_subt_atsc_section_parse_terminate_task();
//extern void osd_subt_enter_2m();
//extern void osd_subt_leave_2m();
//extern void osd_subt_set_clut_2m(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t);
//extern void osd_subt_clear_page_2m();
//extern INT32 osd_subt_create_region_2m(UINT8 region_idx,struct osdrect *rect);
//extern INT32 osd_subt_delete_region_2m(UINT8 region_idx);
//extern INT32 osd_subt_region_show_2m(UINT8 region_idx,UINT8 on);
//extern INT32 osd_subt_draw_pixel_2m(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data);
//extern BOOL osd_region_is_created_2m(UINT8 region_idx);
//extern void osd_subt_draw_pixelmap_2m(struct osdrect rect, UINT8* data);
//extern INT32 osd_subt_get_region_addr_2m(UINT8 region_idx,UINT16 y, UINT32 *addr);
//extern INT32 osd_subt_display_define_2m(struct sdec_display_config *cfg);
extern void tve_cc_onoff(BOOL on);
extern void cc_osd_enter(void);
extern void cc_osd_leave(void);
extern void cc_draw_char(UINT16 x, UINT16 y, UINT8 fg_color, UINT8 bg_color,
    UINT8 double_width,UINT8 double_height, UINT8 character);
extern void cc_fill_char(UINT16 x, UINT16 y,UINT16 w,UINT16 h,UINT8 bg_color);
extern void cc_carriage_return(UINT8 start_row,UINT8 end_row);
extern BOOL osd_region_is_created_no_bl(UINT8 region_idx);
extern INT32 osd_subt_create_region_no_bl(UINT8 region_idx, struct osdrect *p_region);
extern void osd_subt_enter_no_bl(void);
extern void osd_subt_leave_no_bl(void);
extern void osd_subt_set_clut_no_bl(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t);
extern void osd_subt_update_clut_no_bl(void);
extern void osd_subt_clear_page_no_bl(void);
extern INT32 osd_subt_delete_region_no_bl(UINT8 region_idx);
extern INT32 osd_subt_region_show_no_bl(UINT8 region_idx,UINT8 on);
extern INT32 osd_subt_draw_pixel_no_bl(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data);
extern INT32 osd_subt_get_region_addr_no_bl(UINT8 region_idx,UINT16 y, UINT32 *addr);
extern INT32 osd_subt_display_define_no_bl(struct sdec_display_config *cfg);
extern INT32 osd_subt_clear_osd_screen_no_bl(BOOL flag);
extern void lib_subt_callee(UINT8 *msg);
extern void tve_cc_onoff(BOOL on);

extern BOOL lib_subt_atsc_section_parse_create_task(void);

#ifdef _MHEG5_SUPPORT_
extern void tm_mheg5stop_2();
extern void tm_mheg5start_2();
#endif

#ifdef __cplusplus
 }
#endif



#endif /*__LIB_SUBT_INTERNAL_H__  */


