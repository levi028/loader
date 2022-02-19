/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osddrv.h
*
*    Description: This file contains definitions of osd driver's api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _OSDDRV_H_
#define _OSDDRV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <hld/hld_dev.h>
#include "osddrv_dev.h"


#if (SYS_CHIP_MODULE==ALI_M3327C || SYS_PROJECT_FE == PROJECT_FE_ATSC)
#define OSD_BLOCKLINK_SUPPORT
#endif
// ARGB   byte order is: {B, G , R , A}, A in [0, 255]
#define OSDDRV_RGB                  0x00
// AYCbCr byte order is: {Y, Cb, Cr, A}, A in [0, 15 ]
#define OSDDRV_YCBCR            0x01

#define OSD_BYTE_ALIGNED
#define OSD_VSCR_STRIDE         0
#define OSD_CACHE               1

#define OSDDRV_OFF              0x00
#define OSDDRV_ON               0x01

/**************** the position and size of OSD frame buffer********************/
#define OSD_MIN_TV_X        0x20
#define OSD_MIN_TV_Y        0x10
#define OSD_MAX_TV_WIDTH    0x2B0
#define OSD_MAX_TV_HEIGHT   0x1F0 //old value 0x1E0


#define P2N_SCALE_DOWN      0x00
#define N2P_SCALE_UP        0x01
#define OSD_SCALE_MODE      P2N_SCALE_DOWN

//#define P2N_SCALE_IN_NORMAL_PLAY
#define P2N_SCALE_IN_SUBTITLE_PLAY

// scaler
#define OSD_VSCALE_OFF          0x00
#define OSD_VSCALE_TTX_SUBT     0x01
#define OSD_VSCALE_GAME         0x02
#define OSD_VSCALE_DVIEW        0x03
#define OSD_HDUPLICATE_ON       0x04
#define OSD_HDUPLICATE_OFF      0x05
#define OSD_OUTPUT_1080         0x06 // 720x576(720x480)->1920x1080
#define OSD_OUTPUT_720          0x07 // 720x576(720x480)->1280x720
#define OSD_HDVSCALE_OFF        0x08 // 1280x720->720x576(720x480)
#define OSD_HDOUTPUT_1080       0x09 // 1280x720->1920x1080
#define OSD_HDOUTPUT_720        0x0A // 1280x720->1280x720
#define OSD_SET_SCALE_MODE      0x0B // filter mode or duplicate mode
// Suitable for any case. see struct osd_scale_param
#define OSD_SCALE_WITH_PARAM    0x0C
#define OSD_VSCALE_CC_SUBT      0X0D // ATSC CC for HD output scale
// set scale parameter for sd output
//when dual output is enabled. see struct osd_scale_param
#define OSD_SCALE_WITH_PARAM_DEO    0x1000

#define   OSD_SOURCE_PAL        0
#define   OSD_SOURCE_NTSC       1

#define   OSD_SCALE_DUPLICATE   0
#define   OSD_SCALE_FILTER      1

//io_control
#define OSD_IO_UPDATE_PALLETTE  0x00
#define OSD_IO_ADJUST_MEMORY    0x01
#define OSD_IO_SET_VFILTER      0x02
#define OSD_IO_RESPOND_API      0X03
#define OSD_IO_DIS_STATE        0X04
#define OSD_IO_SET_BUF_CACHEABLE 0X05
#define OSD_IO_16M_MODE         0X06
#define OSD_IO_SET_TRANS_COLOR  0x07
#define OSD_IO_SET_ANTI_FLICK_THRE    0x0F
#define OSD_IO_ENABLE_ANTIFLICK       0x10
#define OSD_IO_DISABLE_ANTIFLICK      0x11

#define OSD_IO_SWITCH_DEO_LAYER       0x12
#define OSD_IO_SET_DEO_AUTO_SWITCH    0x13

#define OSD_IO_GET_RESIZE_PARAMATER   0x14
#define OSD_IO_SET_RESIZE_PARAMATER   0x15


#define OSD_IO_ELEPHANT_BASE   0x10000
#define OSD_IO_SWITH_DATA_TRANSFER_MODE (OSD_IO_ELEPHANT_BASE + 0x01)
#define OSD_IO_SET_ANTIFLK_PARA         (OSD_IO_ELEPHANT_BASE + 0x02)
/* dwParam [0x00, 0xff] */
#define OSD_IO_SET_GLOBAL_ALPHA         (OSD_IO_ELEPHANT_BASE + 0x03)
/* OSD layer show or hide(dwParam is UINT32 *) */
#define OSD_IO_GET_ON_OFF               (OSD_IO_ELEPHANT_BASE + 0x04)
/* Enable/Disable filling transparent color in OSDDrv_CreateRegion().
After Open(), default is TRUE. Set it before OSDDrv_CreateRegion().*/
#define OSD_IO_SET_AUTO_CLEAR_REGION    (OSD_IO_ELEPHANT_BASE + 0x05)

/* Enable/Disable GE ouput YCBCR format to DE when source is CLUT8,
clut8->ycbcr, not do color reduction only used when output is 576i/p or 480i/p*/
#define OSD_IO_SET_YCBCR_OUTPUT    (OSD_IO_ELEPHANT_BASE + 0x06)

#define OSD_IO_SET_DISPLAY_ADDR    (OSD_IO_ELEPHANT_BASE + 0x07)
#define OSD_IO_SET_MAX_PIXEL_PITCH     (OSD_IO_ELEPHANT_BASE + 0x08)
#define OSD_IO_WRITE2_SUPPORT_HD_OSD   (OSD_IO_ELEPHANT_BASE + 0x09)
#define OSD_IO_SUBT_RESOLUTION     (OSD_IO_ELEPHANT_BASE + 0x0A)
/* Create region with external buffer, dwParam is pcosd_region_param */
#define OSD_IO_CREATE_REGION            (OSD_IO_ELEPHANT_BASE + 0x0b)
/* Move the region created by OSD_IO_CREATE_REGION,
 dwParam is pcosd_region_param */
#define OSD_IO_MOVE_REGION              (OSD_IO_ELEPHANT_BASE + 0x0c)
/* Get the region information , dwParam is posd_region_param */
#define OSD_IO_GET_REGION_INFO          (OSD_IO_ELEPHANT_BASE + 0x0d)
/* struct OSDRect * */
#define OSD_IO_GET_DISPLAY_RECT         (OSD_IO_ELEPHANT_BASE + 0x0e)
/* struct OSDRect * */
#define OSD_IO_SET_DISPLAY_RECT         (OSD_IO_ELEPHANT_BASE + 0x0f)
/* c3811 enhance par */
#define OSD_IO_SET_ENHANCE_PAR          (OSD_IO_ELEPHANT_BASE + 0x10)


enum OSD_SUBT_RESOLUTION
{
    OSD_SUBT_RESO_720X576 = 1,
    OSD_SUBT_RESO_720X480,
    OSD_SUBT_RESO_1280X720,
    OSD_SUBT_RESO_1920X1080
};
#if 1
#define OSD_PRF(...)					do{}while(0)
#define OSD_ERR_PRF(...)				do{}while(0)
#else
#define OSD_PRF						libc_printf
#define OSD_ERR_PRF					libc_printf
#endif

#define MAX_REGION_NUM 	10

#define OSD_GMA_MAX_WIDTH	1920
#define OSD_GMA_MAX_HEIGHT	1080

struct osd_region_par
{
	int valid;
	struct osdrect reg_rect;
	int bpp;

	struct osdpara para;

	void *mem_start;
	int mem_size;
	int pitch;
};

struct osd_private_gma
{
	int index;
	int open;
	int show_on;

	struct osdpara default_para;
	struct gma_device *gma_dev;
	UINT32 trans_color;

	struct osd_region_par region[MAX_REGION_NUM];

	void *pallette_start;
	int pallette_size;
	int pallette_type;

	BOOL auto_clear_region;             //!<Whether to clear region after creation
};

typedef struct _osd_scale_param
{
    UINT16 tv_sys;       // enum TVSystem
    UINT16 h_div;
    UINT16 v_div;
    UINT16 h_mul;
    UINT16 v_mul;
} osd_scale_param, *posd_scale_param;


typedef struct _osd_resize_param
{
    INT32 h_mode;
    INT32 v_mode;
} osd_resize_param, *posd_resize_param;

typedef struct
{
    UINT8 enable;
    UINT8 layer;
    UINT8 no_temp_buf; // not use temp buffer
    UINT8 reserved;
}osd_clut_ycbcr_out;  /*output ycbcr to DE, source is clut format*/

typedef struct
{
    UINT8 region_id;
    UINT8 reserved[3];
    UINT32 disp_addr; // buffer address to be displayed
}osd_disp_addr_cfg;

typedef struct _osd_region_param
{
    // The region id to create/move/get info
    UINT8   region_id;
    // enum OSDColorMode
    UINT8   color_format;
    // 0 - use color by color alpha; 1 - enable global alpha for this region
    UINT8   galpha_enable;
    // If global alpha enable, please set global_alpha [0x00, 0xff]
    UINT8   global_alpha;
    // pallette index for CLUT4
    UINT8   pallette_sel;
    // x offset of the region, from screen top_left pixel
    UINT16  region_x;
    // y offset of the region, from screen top_left pixel
    UINT16  region_y;
    UINT16  region_w;
    UINT16  region_h;
    /* 0 - use uMemBase(internal memory) which is set by
    ge_attach(ge_layer_config_t *);bitmap_addr not 0 - use
    external memory address as region bitmap addr*/
    UINT32  bitmap_addr;
    /* pixel pitch(not byte pitch) for internal memory or bitmap_addr
    ge_gma_create_region(): bitmap_addr and pixel_pitch determines
    the region bitmap address, total 4 cases:
    Case 1: if bitmap_addr is 0, and pixel_pitch is 0,
          it will use region_w as pixel_pitch,
          and region bitmap addr will be allocated from uMemBase dynamically.
    Case 2: if bitmap_addr is 0, and pixel_pitch is not 0,
          the region bitmap addr will be fixed:
          uMemBase + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel
    Case 3: if bitmap_addr is not 0, and pixel_pitch is 0,
          the region bitmap addr will be:
          bitmap_addr + (bitmap_w * bitmap_y + bitmap_x) * byte_per_pixel
    Case 4: if bitmap_addr is not 0, and pixel_pitch is not 0,
          the region bitmap addr will be:
          bitmap_addr + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel
    ge_gma_move_region(): region using internal memory can only
    change region_x, region_y, pal_sel;
    ge_gma_move_region(): region using external memory can
    change everyting in ge_gma_region_t;*/
    UINT32  pixel_pitch;
    // x offset from the top_left pixel in bitmap_addr or internal memory
    UINT32  bitmap_x;
    // y offset from the top_left pixel in bitmap_addr or internal memory
    UINT32  bitmap_y;
    /* bitmap_w must >= bitmap_x + region_w,
    both for internal memory or external memory*/
    UINT32  bitmap_w;
    /* bitmap_h must >= bitmap_y + region_h,
    both for internal memory or external memory*/
    UINT32  bitmap_h;
} osd_region_param, *posd_region_param;


void osd_m36f_attach(char *name, OSD_DRIVER_CONFIG *attach_config);

RET_CODE osddrv_open(HANDLE hdev,struct osdpara *open_para);
RET_CODE osddrv_close(HANDLE hdev);
RET_CODE osddrv_io_ctl(HANDLE hdev,UINT32 cmd,UINT32 param);
RET_CODE osddrv_get_para(HANDLE hdev,struct osdpara* para);
RET_CODE osddrv_show_on_off(HANDLE hdev,UINT8 on_off);
RET_CODE osddrv_set_pallette(HANDLE hdev,UINT8 *pallette, UINT16 color_n, UINT8 type);
RET_CODE osddrv_get_pallette(HANDLE hdev,UINT8 *pallette, UINT16 color_n, UINT8 type);
RET_CODE osddrv_modify_pallette(HANDLE hdev,UINT8 index,UINT8 y, UINT8 cb,UINT8 cr,UINT8 k);

RET_CODE osddrv_create_region(HANDLE hdev,UINT8 region_id, struct osdrect *rect,struct osdpara *open_para);
RET_CODE osddrv_delete_region(HANDLE hdev,UINT8 region_id);

RET_CODE osddrv_set_region_pos(HANDLE hdev,UINT8 region_id,struct osdrect *rect);
RET_CODE osddrv_get_region_pos(HANDLE hdev,UINT8 region_id,struct osdrect *rect);

RET_CODE osddrv_region_show(HANDLE hdev,UINT8 region_id,BOOL on);
RET_CODE osddrv_region_write_by_surface(HANDLE hdev,UINT8 region_id, UINT8 *src_data,
                              struct osdrect *dest_rect, struct osdrect *src_rect, UINT32 pitch);
RET_CODE osddrv_region_write(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);

RET_CODE osddrv_region_read(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);
RET_CODE osddrv_region_fill(HANDLE hdev,UINT8 region_id, struct osdrect *rect, UINT32 color_data);

RET_CODE osddrv_region_write_inverse(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);
RET_CODE osddrv_region_read_inverse(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);
RET_CODE osddrv_region_fill_inverse(HANDLE hdev,UINT8 region_id, struct osdrect *rect, UINT32 color_data);
RET_CODE osddrv_region_write2(HANDLE hdev,UINT8 region_id, UINT8 *src_data,UINT16 src_width,
                           UINT16 src_height,struct osdrect *src_rect, struct osdrect *dest_rect);
RET_CODE osddrv_draw_hor_line(HANDLE hdev, UINT8 region_id, UINT32 x, UINT32 y, UINT32 width, UINT32 color);
RET_CODE osddrv_scale(HANDLE hdev, UINT32 scale_cmd,UINT32 scale_param);
RET_CODE osddrv_set_clip(HANDLE hdev,enum clipmode clip_mode, struct osdrect *rect);
RET_CODE osddrv_clear_clip(HANDLE hdev);
RET_CODE osddrv_get_region_addr(HANDLE hdev,UINT8 region_idx, UINT16 y, UINT32 *addr);

UINT8 osd_get_byte_per_pixel(UINT8 color_mode);
UINT16 osd_get_pitch(UINT8 color_mode, UINT16 width);
void osd_m33c_attach(char *name, OSD_DRIVER_CONFIG *config);
void osd_init_regionfill_2m(struct osd_device *dev);

#ifdef DUAL_ENABLE
void osd_m33c_see_attach(char *name, OSD_DRIVER_CONFIG *pconfig);
void osd_see_init_regionfill_2m(struct osd_device *dev);
#endif

RET_CODE gmadrv_open(HANDLE hdev,struct osdpara *open_para);
RET_CODE gmadrv_close(HANDLE hdev);
RET_CODE gmadrv_io_ctl(HANDLE hdev,UINT32 cmd,UINT32 param);
RET_CODE gmadrv_get_para(HANDLE hdev,struct osdpara* para);
RET_CODE gmadrv_show_on_off(HANDLE hdev,UINT8 on_off);
RET_CODE gmadrv_set_pallette(HANDLE hdev,UINT8 *pallette, UINT16 color_n, UINT8 type);
RET_CODE gmadrv_get_pallette(HANDLE hdev,UINT8 *pallette, UINT16 color_n, UINT8 type);
RET_CODE gmadrv_modify_pallette(HANDLE hdev,UINT8 index,UINT8 y, UINT8 cb,UINT8 cr,UINT8 k);

RET_CODE gmadrv_create_region(HANDLE hdev,UINT8 region_id, struct osdrect *rect,struct osdpara *open_para);
RET_CODE gmadrv_delete_region(HANDLE hdev,UINT8 region_id);

RET_CODE gmadrv_set_region_pos(HANDLE hdev,UINT8 region_id,struct osdrect *rect);
RET_CODE gmadrv_get_region_pos(HANDLE hdev,UINT8 region_id,struct osdrect *rect);

RET_CODE gmadrv_region_show(HANDLE hdev,UINT8 region_id,BOOL on);
RET_CODE gmadrv_region_write_by_surface(HANDLE hdev,UINT8 region_id, UINT8 *src_data,
                              struct osdrect *dest_rect, struct osdrect *src_rect, UINT32 pitch);
RET_CODE gmadrv_region_write(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);

RET_CODE gmadrv_region_read(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);
RET_CODE gmadrv_region_fill(HANDLE hdev,UINT8 region_id, struct osdrect *rect, UINT32 color_data);

RET_CODE gmadrv_region_write_inverse(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);
RET_CODE gmadrv_region_read_inverse(HANDLE hdev,UINT8 region_id, VSCR *vscr,struct osdrect *rect);
RET_CODE gmadrv_region_fill_inverse(HANDLE hdev,UINT8 region_id, struct osdrect *rect, UINT32 color_data);
RET_CODE gmadrv_region_write2(HANDLE hdev,UINT8 region_id, UINT8 *src_data,UINT16 src_width,
                           UINT16 src_height,struct osdrect *src_rect, struct osdrect *dest_rect);
RET_CODE gmadrv_draw_hor_line(HANDLE hdev, UINT8 region_id, UINT32 x, UINT32 y, UINT32 width, UINT32 color);
RET_CODE gmadrv_scale(HANDLE hdev, UINT32 scale_cmd,UINT32 scale_param);
RET_CODE gmadrv_set_clip(HANDLE hdev,enum clipmode clip_mode, struct osdrect *rect);
RET_CODE gmadrv_clear_clip(HANDLE hdev);
RET_CODE gmadrv_get_region_addr(HANDLE hdev,UINT8 region_idx, UINT16 y, UINT32 *addr);

#define osddrv_region_fill32 osddrv_region_fill
#define osddrv_region_fill32inverse osddrv_region_fill_inverse
#define osd_get_pitch_by_color_mode osd_get_pitch
#define osd_resize_param    osd_resize_param
#define posd_resize_param   posd_resize_param
typedef const osd_scale_param *pcosd_scale_param;
typedef const osd_region_param *pcosd_region_param;



#ifdef __cplusplus
 }
#endif

#endif
