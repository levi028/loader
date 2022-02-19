/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: sdec.h

   *    Description:define the MACRO, the variable and structure uisng by subtitle
        decoder.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    __HLD_SDEC_H__
#define __HLD_SDEC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <hld/osd/osddrv.h>
#include <hld/sdec/sdec_dev.h>

#define MAX_REGION_IN_PAGE 8
#define MAX_OBJECT_IN_REGION 8
#define MAX_ENTRY_IN_CLUT 16

#define SEGMENT_HEADER_LENGTH 6
#define SDEC_NORMAL_0 0
#define SDEC_NORMAL_1 1
#define SDEC_NORMAL_2 2
#define SDEC_NORMAL_3 3

struct sdec_display_config
{
    UINT8 dds_version_number;
    UINT8 display_window_flag;
    UINT16 display_width;
    UINT16 display_height;
    UINT16 display_window_hor_min;
    UINT16 display_window_hor_max;
    UINT16 display_window_ver_min;
    UINT16 display_window_ver_max;
};
struct subtitle_cb_info_s
{
    UINT8 func_type;
    UINT32 param1;
    UINT32 param2;
    UINT32 param3;
    UINT32 param4;
};



    /** Just add for new modification **/

struct ts_see2main_subt_pkt_s
{
    struct control_block ctrl_blk;
    UINT8 subt_pkt_size;    /** Actual subt pkt data size. **/
    UINT8 subt_pkt_buff[188];
};

#define TS_SEE2MAIN_MAX_PKT_NUM 4
struct ts_see2main_data_cb_new_s
{
    UINT8 subt_pkt_num; /** Actual subt pkt transfering. **/
    struct ts_see2main_subt_pkt_s subt_pkt[TS_SEE2MAIN_MAX_PKT_NUM];
};

enum subtitle_cb_info_e
{
    SDEC_START_T,
    SDEC_STOP_T,
    SDEC_OSD_ENTER_T,
    SDEC_OSD_LEAVE_T,
};

struct osdrect;
struct sdec_feature_config
{
    UINT8 *temp_buf;//buffer for temp info storage, user can alloc
                    //memory for this buffer or assign 0 to this buffer
                   // address and sdec driver will alloc memory for it.
    UINT32 temp_buf_len;//recommended value is 0x2000 in byte unit
    UINT8 *bs_buf;//bitstream buffer for subtitle data, user can alloc
                   // memory for this buffer or assign 0 to this buffer
                   // address and sdec driver will alloc memory for it.
    UINT32 bs_buf_len;//recommended value is 0X5000 in byte unit
    UINT32 bs_hdr_buf;//MUST be 0
    UINT32 bs_hdr_buf_len;//MUST be100
    UINT8 *pixel_buf;//pixel buffer for display, user can alloc
                    //memory for this buffer or assign 0 to this buffer
                    //address and sdec driver will alloc memory for it.
    UINT32 pixel_buf_len;//recommended value is 0X19000 in byte unit
    UINT8 tsk_qtm;//task quantum for sdec decoder, recommended value is 10
    UINT8 tsk_pri;
       //task priority for sdec decoder, recommended value is OSAL_PRI_NORMAL
    UINT8 transparent_color;//depends on solution requirements.
    UINT8 support_hw_decode;//1,support;0,no
    UINT8 *sdec_hw_buf;
    UINT16 sdec_hw_buf_len;
    void (*subt_draw_pixelmap)(struct osdrect, UINT8 *);
    INT32 (*subt_draw_pixel)(UINT8, UINT16, UINT16, UINT8);
    BOOL (*region_is_created)(UINT8);
    INT32 (*subt_create_region)(UINT8, struct osdrect *);
    INT32 (*subt_region_show)(UINT8, UINT8);
    INT32 (*subt_delete_region)(UINT8);
    INT32 (*subt_get_region_addr)(UINT8 ,UINT16 , UINT32 *);
    void (*draw_obj_hardware)
         (UINT8 *, UINT16, UINT8, UINT8,UINT16, UINT8, UINT16, UINT16);
    INT32 (*subt_display_define)(struct sdec_display_config *);
	INT32 (*subt_clear_osd_screen)(BOOL);
};

INT32 sdec_open(struct sdec_device *dev);
INT32 sdec_close(struct sdec_device *dev);
INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id);
INT32  sdec_stop(struct sdec_device *dev);
INT32  sdec_pause(struct sdec_device *dev);
INT32 sdec_ioctl(struct sdec_device *dev, UINT32 cmd, UINT32 param);
INT32 sdec_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,UINT32 *pu_size_got,
    struct control_block *ps_data_cb);
void sdec_update_write(void *pdev,UINT32 u_data_size);
INT32  sdec_showonoff(void *pdev,BOOL b_on);


void draw_object_hardware(UINT8 * data, UINT16 len, UINT8 clut_id,
     UINT8 region_idx,UINT16 stride, UINT8 field_polar, UINT16 x, UINT16 y);
INT32 sdec_attach(struct sdec_feature_config *cfg_param);
void subt_hld_disply_bl_init(struct sdec_device *dev);

extern INT32 sdec_m33_attach(struct sdec_feature_config *cfg_param);
extern void subt_disply_bl_init(struct sdec_device *dev);
INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param);
void isdbtcc_disply_init(struct sdec_device *dev);

#ifdef __cplusplus
}
#endif

#endif /*__HLD_SDEC_H__*/






