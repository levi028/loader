/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osddrv_dev.h
*
*    Description: This file contains definitions of osd device structure
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __OSDDRV_DEV_H__
#define __OSDDRV_DEV_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define OSD_M3602_NOT_ATTACH_DEO    0x80
#define ENABLE_CONTIGUOUS_MEMORY    0x80

#ifdef WIN32
typedef struct osdrect OSD_RECT, *POSD_RECT;
typedef const struct osdrect *PCOSD_RECT;
#endif

struct osdrect
{
    INT16   u_left;
    INT16   u_top;
    INT16   u_width;
    INT16   u_height;
};

enum clipmode
{
    CLIP_INSIDE_RECT = 0,
    CLIP_OUTSIDE_RECT,
    CLIP_OFF
};

enum osdsys
{
    OSD_PAL = 0,
    OSD_NTSC
};

enum osdcolor_mode
{
    OSD_4_COLOR =   0,
    OSD_16_COLOR,
    OSD_256_COLOR,
    OSD_16_COLOR_PIXEL_ALPHA,
    OSD_HD_ACLUT88,//@Alan modify080421
    OSD_HD_RGB565,
    OSD_HD_RGB888,
    OSD_HD_RGB555,
    OSD_HD_RGB444,
    OSD_HD_ARGB565,
    OSD_HD_ARGB8888,
    OSD_HD_ARGB1555,
    OSD_HD_ARGB4444,
    osd_hd_aycb_cr8888,
    OSD_HD_YCBCR888,
    OSD_HD_YCBCR422,
    OSD_HD_YCBCR422MB,
    OSD_HD_YCBCR420MB,
    OSD_COLOR_MODE_MAX
};

typedef struct tag_vscr
{
    struct osdrect  v_r;
    UINT8   *lpb_scr;
    UINT8   b_block_id;
    BOOL    update_pending;
    UINT8   b_color_mode;
    UINT8 b_draw_mode;
}VSCR,*LPVSCR;

struct osdpara
{
    enum osdcolor_mode e_mode; /* Color mode */
    UINT8 u_galpha_enable;
    UINT8 u_galpha;          /* Alpha blending mix ratio */
    UINT8 u_pallette_sel;          /* Pallette index */
};

struct osd_attach_config
{
    /*enable/disable osd vertical filter*/
    BOOL  b_osdvertical_filter_enable;
    /*enable/disable PAL/NTSC scale in normal play mode*/
    BOOL  p2nscale_in_normal_play;
    /*enable/disable PAL/NTSC scale in subtitle play mode*/
    BOOL  p2nscale_in_subtitle_play;
};

struct OSD_AF_PAR
{
    UINT8 id;
    UINT8 vd:1;
    UINT8 af:1;
    UINT8 res:6;
};

typedef struct _OSD_DRIVER_CONFIG
{
    //previous defined as __MM_OSD_START_ADDR, 16 bit align
    UINT32 u_mem_base;
    //previous defined as __MM_OSD_LEN,  usually 720*576/factor
    //and factor = how many pixels that 1 byte maps to
    UINT32 u_mem_size;
    //these three varialbes are only used for M33xxC
    UINT8 b_static_block;
    UINT8 b_direct_draw;
    UINT8 b_cacheable;
    //enable/disable osd vertical filter
    UINT8 b_vfilter_enable;
    //enable/disable PAL/NTSC scale in normal play mode
    UINT8 b_p2nscale_in_normal_play;
    //enable/disable PAL/NTSC scale in subtitle play mode
    UINT8 b_p2nscale_in_subtitle_play;
    //set multi-view scale ratio, only for M33xxC now
    UINT16 u_dview_scale_ratio[2][2];
    struct OSD_AF_PAR af_par[4];
    // memory addr for SD osd when OSD dual-output is enabled,
    // and SD osd source size is not same with HD side
    UINT32 u_sdmem_base;
    // memory size for SD osd when OSD dual-output is enabled
    // and SD osd source size is not same with HD side
    UINT32 u_sdmem_size;

}OSD_DRIVER_CONFIG, *POSD_DRIVER_CONFIG;

typedef struct _OSD_MINI_DRIVER_CONFIG
{
    UINT32 u_osd_bk_start_addr;
    UINT32 u_osd_bk_size;
    UINT32 u_osd_dis_hdstart_addr;
    UINT32 u_osd_dis_hdsize;
    UINT32 u_osd_dis_sdstart_addr;
    UINT32 u_osd_dis_sdsize;
    UINT32 u_pallet_buf;
    UINT32 u_ge_temp_buf;
    UINT32 u_ge_temp_size;
    UINT32 u_ge_cmd_buf;
    UINT32 u_ge_cmd_size;
    //enable/disable PAL/NTSC scale in normal play mode
    UINT8 b_p2nscale_in_normal_play;
}OSD_MINI_DRIVER_CONFIG, *POSD_MINI_DRIVER_CONFIG;


struct osd_device
{
    /* Common device structure member */
    struct osd_device *next;            /* link to next device */
    UINT32 type;                        /* Interface hardware type */
    INT8 name[HLD_MAX_NAME_SIZE];       /* Device name */
    UINT32 flags;              /* Interface flags, status and ability */

    /* Hardware privative structure */
    void *priv;              /* pointer to private data */
    UINT32  sema_opert_osd;
    /* Device related functions */
    RET_CODE (*open)(struct osd_device *, struct osdpara *);
    RET_CODE (*close)(struct osd_device *);
    RET_CODE (*ioctl)(struct osd_device *,UINT32,UINT32);
    RET_CODE (*get_para)(struct osd_device *,struct osdpara *);
    RET_CODE (*show_onoff)(struct osd_device *,UINT8);
    RET_CODE (*set_pallette)(struct osd_device *,UINT8 *,UINT16,UINT8);
    RET_CODE (*get_pallette)(struct osd_device *,UINT8 *,UINT16,UINT8);
    RET_CODE (*modify_pallette)(struct osd_device *,UINT8 ,UINT8 ,UINT8 ,UINT8 ,UINT8 );
    RET_CODE (*create_region)(struct osd_device *,UINT8 , struct osdrect *,struct osdpara *);
    RET_CODE (*delete_region)(struct osd_device *,UINT8 );
    RET_CODE (*set_region_pos)(struct osd_device *,UINT8 ,struct osdrect *);
    RET_CODE (*get_region_pos)(struct osd_device *,UINT8 ,struct osdrect *);
    RET_CODE (*region_show)(struct osd_device *,UINT8, BOOL);
    RET_CODE (*region_write)(struct osd_device *,UINT8,VSCR *,struct osdrect *);
    RET_CODE (*region_write_by_surface)(struct osd_device *,UINT8 ,UINT8 *,struct osdrect *,struct osdrect *,UINT32 );
    RET_CODE (*region_read)(struct osd_device *,UINT8,VSCR *,struct osdrect *);
    RET_CODE (*region_fill)(struct osd_device *,UINT8,struct osdrect *, UINT32);
    RET_CODE (*region_write2)(struct osd_device *,UINT8,UINT8 *,UINT16 , UINT16 ,struct osdrect *,struct osdrect *);
    RET_CODE (*draw_hor_line)(struct osd_device *, UINT8, UINT32, UINT32, UINT32, UINT32);
    RET_CODE (*scale)(struct osd_device *, UINT32 ,UINT32);
    RET_CODE (*set_clip)(struct osd_device *,enum clipmode,struct osdrect*);
    RET_CODE (*clear_clip)(struct osd_device *);
    RET_CODE (*get_region_addr)(struct osd_device *,UINT8 ,UINT16, UINT32 *);
    RET_CODE (*region_write_inverse)(struct osd_device *,UINT8, VSCR *,struct osdrect *);
    RET_CODE (*region_read_inverse)(struct osd_device *,UINT8, VSCR *,struct osdrect *);
    RET_CODE (*region_fill_inverse)(struct osd_device *,UINT8, struct osdrect *, UINT32);

};

#ifdef __cplusplus
 }
#endif

#endif

