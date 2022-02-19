/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: subt_osd.h

   *    Description:define the MACRO, structure uisng by subtitle showing on OSD.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    _SUBT_OSD_H_
#define  _SUBT_OSD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <hld/osd/osddrv.h>
#include <hld/sdec/sdec.h>

#define SUBT_WIDTH    640//640//608//608//720
#define SUBT_HEIGHT 520//520//480//416//576
#define SUBT_HOR_OFFSET ((720-SUBT_WIDTH)/2)
#define SUBT_VER_OFFSET ((576-SUBT_HEIGHT)/2)

#define DVB_SD_SUBT_OSD_WIDTH 720
#define DVB_HD_SUBT_OSD_WIDTH 1280

#define DVB_SD_SUBT_OSD_HEIGHT 576
#define DVB_HD_SUBT_OSD_HEIGHT 720

#define SUBT_DISPLAY_HEIGHT_480 480
#define SUBT_DISPLAY_HEIGHT_576 576
#define SUBT_DISPLAY_HEIGHT_720 720
#define SUBT_DISPLAY_HEIGHT_1080 1080

#define ATSC_SUBT_OSD_WIDTH 720
#define ATSC_SUBT_OSD_HEIGHT 576
#define DIVIDE_HALF 2

#define SUBT_TRANSPARENT_COLOR    OSD_TRANSPARENT_COLOR//0//0xff

struct subt_config_par
{
   UINT8 cc_by_vbi;
   UINT8 cc_by_osd;
   UINT8 osd_blocklink_enable;
   UINT8 *g_ps_buf_addr;
   UINT32 g_ps_buf_len;
   UINT8 sys_sdram_size;//2:2M,8:8M.....
   UINT16 max_subt_height;
   UINT8 speed_up_subt_enable;
   UINT8 osd_layer_id;
   UINT8 hd_subtitle_support;
   UINT16 subt_width;
   UINT16 subt_height;
   UINT16 subt_hor_offset;
   UINT16 subt_ver_offset;
    UINT16 user_force_set_hor_offset;// Added for configuration by user
    UINT16 user_force_set_ver_offset;
    BOOL user_force_pos;

   void (*osd_subt_enter)();
   void (*osd_subt_leave)();
#ifndef  SUBT_FUNC_POINTER_ENABLE
   void (*osd_subt_set_clut)(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t);
   void (*osd_subt_clear_page)();
   INT32 (*osd_subt_create_region)(UINT8 region_idx,struct osdrect *rect);
   INT32 (*osd_subt_delete_region)(UINT8 region_idx);
   INT32 (*osd_subt_region_show)(UINT8 region_idx,UINT8 on);
   INT32 (*osd_subt_draw_pixel)(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data);
   BOOL (*osd_region_is_created)(UINT8 region_idx);
   void (*osd_subt_draw_pixelmap)(struct osdrect rect, UINT8 *data);
   INT32 (*osd_subt_get_region_addr)(UINT8 region_idx,UINT16 y, UINT32 *addr);
   void (*osd_subt_update_clut)();
   INT32 (*osd_subt_display_define)(struct sdec_display_config *cfg);
#endif
	UINT32     (*osd_get_scale_para)(enum tvsystem tvsys,INT32 scr_width);

	UINT8 *osd_buf;
	UINT32 osd_buf_length;
	INT32 (*osd_subt_clear_osd_screen)(BOOL flag);
};


//#if (SYS_CHIP_MODULE!=ALI_M3327C)
//void osd_subt_init_screen();
//void osd_subt_enter();
//void osd_subt_leave();
//void osd_subt_set_clut(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t);
//void osd_subt_clear_page();
void osd_subt_enter();
void osd_subt_leave();
#ifndef  SUBT_FUNC_POINTER_ENABLE
void osd_subt_draw_pixelmap(struct osdrect rect, UINT8* data);
//#else
void osd_subt_set_clut(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t);
void osd_subt_clear_page();
INT32 osd_subt_create_region(UINT8 region_idx,struct osdrect *rect);
INT32 osd_subt_delete_region(UINT8 region_idx);
INT32 osd_subt_region_show(UINT8 region_idx,UINT8 on);
INT32 osd_subt_draw_pixel(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data);
BOOL osd_region_is_created(UINT8 region_idx);
INT32 osd_subt_get_region_addr(UINT8 region_idx,UINT16 y, UINT32 *addr);
void osd_subt_update_clut();
INT32 osd_subt_display_define(struct sdec_display_config *cfg);
INT32 osd_subt_clear_osd_screen(BOOL flag);

#endif
//#endif
void lib_subt_init();
void lib_subt_attach(struct subt_config_par *psubt_config_par);
void lib_subt_attach_2m(struct subt_config_par *psubt_config_par);
void lib_subt_attach_8m(struct subt_config_par *psubt_config_par);

//for subt function pointer
//typedef void (*osd_subt_enter_t)();
//typedef void (*osd_subt_leave_t)();
typedef void (*osd_subt_set_clut_t)(UINT8 entry_id,UINT8 y,UINT8 cb,UINT8 cr,UINT8 t);
typedef void (*osd_subt_clear_page_t)();
typedef INT32 (*osd_subt_create_region_t)(UINT8 region_idx,struct osdrect *rect);
typedef INT32 (*osd_subt_delete_region_t)(UINT8 region_idx);
typedef INT32 (*osd_subt_region_show_t)(UINT8 region_idx,UINT8 on);
typedef INT32 (*osd_subt_draw_pixel_t)(UINT8 region_idx,UINT16 x,UINT16 y,UINT8 data);
typedef BOOL (*osd_region_is_created_t)(UINT8 region_idx);
typedef void (*osd_subt_draw_pixelmap_t)(struct osdrect rect, UINT8* data);
void lib_subt_atsc_hcenter_on_off(BOOL b_on_off);
//void lib_subt_atsc_shift_y_set(INT32 nShiftY);
void lib_subt_atsc_force_shift_y_set(BOOL b_user_force,INT32 n_shift_y);
void lib_subt_atsc_color_exchange_on_off(BOOL b_on_off);
void lib_subt_atsc_color_exchange__color_char_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k);
void lib_subt_atsc_color_exchange__color_outline_set(UINT8 u_y,UINT8 u_cb,UINT8 u_cr,UINT8 u_k);
void lib_subt_dvb_auto_top_pos_onoff(BOOL b_on_off,UINT16 us_top_max,UINT16 us_top_save);
void lib_subt_dvb_shift_y_set(INT32 n_shift_y);
void lib_subt_shift_time_set(INT32 n_shift_t,BOOL b_on_off);
void lib_subt_dvb_bg_color_set(BOOL b_background,UINT32 b_color);
void lib_subt_dvb_char_color_set(BOOL b_char,UINT32 b_color);

void lib_subt_copy_data(UINT32 data, UINT32 length);

#ifdef __cplusplus
 }
#endif


#endif
