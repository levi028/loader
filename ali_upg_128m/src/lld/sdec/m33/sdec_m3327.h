/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m3327.h
*
*    Description: The file is to define the function how to operate subtitle
     decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef __LLD_SDEC_M3327_H__
#define __LLD_SDEC_M3327_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <basic_types.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>

#define SDEC_CMD_START                   0x00000001
#define SDEC_CMD_STOP                    0x00000002
#define SDEC_CMD_PAUSE                   0x00000004
#define SDEC_MSG_START_OK                0x00000100
#define SDEC_MSG_STOP_OK                 0x00000200
#define SDEC_MSG_PAUSE_OK                0x00000400
#define SDEC_MSG_DATA_AVAILABLE          0x00010000
#define SDEC_MSG_BUFF_AVAILABLE          0x00020000
#define SDEC_CMD_SHOWON                  0x00040000
#define SDEC_CMD_SHOWOFF                 0x01000000
#define SDEC_MSG_SHOWON_OK               0x02000000
#define SDEC_MSG_SHOWOFF_OK              0x04000000
#define SDEC_STATE_IDLE        0
#define SDEC_STATE_PLAY        1
#define SDEC_STATE_PAUSE       2
#define SDEC_ASSERT     ASSERT
#define SDEC_NORMAL_10 10
#define SDEC_NORMAL_100 100
#define SDEC_NORMAL_0 0
#define SDEC_NORMAL_1 1
#define SDEC_NORMAL_2 2
#define SDEC_NORMAL_128 128
#define SUBT_PAGE_SEGMENT 0x10
#define SUBT_REGION_SEGMENT 0x11
#define SUBT_CLUT_SEGMENT 0x12
#define SUBT_OBJECT_SEGMENT 0x13
#define SUBT_DISPLAY_SEGMENT 0x14
#define SUBT_SEGMENT_FIND 0x0f

#define SUBT_CODE_VALUE_INVALID 0xff
#define SUBT_CODE_VALUE_C 0x0c
#define SUBT_CODE_VALUE_D 0x0d
#define SUBT_CODE_VALUE_E 0x0e
#define SUBT_CODE_VALUE_F 0x0f

#define SUBT_CODE_VALUE_0 0x00
#define SUBT_CODE_VALUE_8 0x08

#define SBF_UPDATE    0
#define INT_REQUEST 1
#define PALLETTE_OFFSET 0x80
#define STC_OFFSET      0x200
#define SUBT_DRAW_PIXELMAP(x, y)    \
    do                            \
    {                            \
        if(g_sdec_priv->subt_draw_pixelmap)    \
            g_sdec_priv->subt_draw_pixelmap(x, y);\
    }while(0)

#define SUBT_DRAW_PIXEL(a, b, c, d)    \
    do                                \
    {                                \
        if(g_sdec_priv->subt_draw_pixel)    \
            g_sdec_priv->subt_draw_pixel(a, b, c, d);\
    }while(0)

#define SUBT_CREATE_REGION(x, y)    \
    do                                \
    {                                \
        if(g_sdec_priv->subt_create_region)    \
            g_sdec_priv->subt_create_region(x, y);    \
    }while(0)

#define SUBT_REGION_SHOW(x, y)    \
    do                            \
    {                            \
        if(g_sdec_priv->subt_region_show)    \
            g_sdec_priv->subt_region_show(x, y);\
    }while(0)

#define SUBT_DELETE_REGION(x)    \
    do                            \
    {                            \
        if(g_sdec_priv->subt_delete_region)    \
            g_sdec_priv->subt_delete_region(x);\
    }while(0)

#define SUBT_GET_REGION_ADDR(x, y, z)    \
    do                            \
    {                            \
        if(g_sdec_priv->subt_get_region_addr)    \
            g_sdec_priv->subt_get_region_addr(x, y, z);\
    }while(0)

#define SUBT_CLEAR_OSD_SCREEN(x) 		\
	do							\
	{							\
		if(g_sdec_priv->subt_clear_osd_screen) \
			g_sdec_priv->subt_clear_osd_screen(x);\
	}while(0)
//#define SDEC_PRINTF soc_printf
#define SDEC_PRINTF(...) do{}while(0)//soc_printf//libc_printf//PRINTF
//#define SDEC_PRINTF libc_printf
#define OBJ_PRINTF(...) do{}while(0)

struct sdec_m3327_private
{
    void *priv_addr;
    void (*stream_parse_cb)(void);
    void (*display_entry_cb)(UINT8);
    void (*subt_draw_pixelmap)(struct osdrect, UINT8 *);
    INT32 (*subt_draw_pixel)(UINT8, UINT16, UINT16, UINT8);
    BOOL (*region_is_created)(UINT8);
    INT32 (*subt_create_region)(UINT8, struct osdrect *);
    INT32 (*subt_region_show)(UINT8, UINT8);
    INT32 (*subt_delete_region)(UINT8);
    INT32 (*subt_get_region_addr)(UINT8 ,UINT16, UINT32 *);
    void (*draw_obj_hardware)(UINT8 *, UINT16, UINT8, UINT8,UINT16, UINT8, UINT16, UINT16);
    INT32 (*subt_display_define)(struct sdec_display_config *);
    UINT8 support_hw_decode;//1,support;0,no
    UINT8 status;
	INT32 (*subt_clear_osd_screen)(BOOL);
};


struct sdec_data_hdr
{
    BOOL b_info_valid;
    UINT8 u_pes_start;
    // Bit map to specify which parameter in the structure is valid:
    //"1" valid, "0" not invlid .
    // Bit0: STC_ID section valid or not;
    // bit1: Data discontinue or not
    // Bit2: PTS section valid or not;
    // Bit3: uVobuNum and uVobuBaseTime section valid or not.
    // Bit4: Video trick mode or not

    UINT8    u_ctrl_byte;
    UINT8    u_stc_id;    // System timer counter identify:0: STC 0;1: STC 1;
    UINT32    u_pts;    // Corresponding PTS value for a data block.

    UINT32 u_data_size;     // data size
    UINT8 *pu_start_point;
};



struct page_cb
{
    struct page_cb *next;
    struct page_cb *prev;
    UINT32 u_pts;
    UINT8 u_stcid;
    UINT8 page_time_out;
    UINT8 page_version_number;
    UINT8 page_state;
    UINT8 region_cnt;
    UINT8 end_of_display;
    UINT8 clut_cnt;

    UINT8 region_id[MAX_REGION_IN_PAGE];
    UINT16 region_x[MAX_REGION_IN_PAGE];
    UINT16 region_y[MAX_REGION_IN_PAGE];

    UINT16 display_width;
    UINT16 display_height;
};

struct region_cb
{
    UINT8 region_id;
    UINT8 region_version_number;
    UINT8 region_fill_flag;
    UINT16 region_width;
    UINT16 region_height;
    UINT8 region_level;
    UINT8 region_depth;
    UINT8 clut_id;
    UINT8 region_2b_pixel_code;
    UINT8 region_4b_pixel_code;
    UINT8 region_8b_pixel_code;

    UINT8 object_cnt;
    UINT16 object_id[MAX_OBJECT_IN_REGION];
    UINT8 object_type[MAX_OBJECT_IN_REGION];
    UINT8 object_provider_flag[MAX_OBJECT_IN_REGION];
    UINT16 object_x[MAX_OBJECT_IN_REGION];
    UINT16 object_y[MAX_OBJECT_IN_REGION];

    UINT8 foregroud_pixel_code[MAX_OBJECT_IN_REGION];
    UINT8 backgroud_pixel_code[MAX_OBJECT_IN_REGION];


    //UINT8* top_field_rle_data;//UINT8 top_field_rle_data[2500];
    //UINT32 top_field_data_len;

    //UINT8* bot_field_rle_data;//UINT8 bot_field_rle_data[2500];
    //UINT32 bot_field_data_len;

    //UINT8 *data;

};

struct sdec_msg
{
    UINT8 type;
};


//extern UINT32 stc_delay;


INT32  sdec_m3327_open(struct sdec_device *dev);
INT32  sdec_m3327_close(struct sdec_device *dev);
INT32  sdec_m3327_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id);
INT32  sdec_m3327_stop(struct sdec_device *dev);
INT32  sdec_m3327_pause(struct sdec_device *dev);
INT32  sdec_m3327_showonoff(struct sdec_device *dev,BOOL b_on);
INT32 sdec_m3327_request_write(struct sdec_device *dev,UINT32 u_size_requested,
    struct control_block* p_tdata_ctrl_blk,UINT8** ppu_data,UINT32* pu_size_got);
void sdec_m3327_update_write(struct sdec_device *dev,UINT32 data_size);
BOOL get_sdec_status();

extern void set_sdec_clear_page(BOOL clear);
extern void sdec_add_to_temp(UINT8 *p_data,UINT32 size);
extern void sdec_copy_to_temp(UINT8 *p_data,UINT32 size);
extern UINT8 getnext2bit(void);
extern UINT8 getnext4bit(void);
extern UINT16 getnext8bit(void);
extern INT32 sdec_m3327_init(void);
extern void lld_sdec_sw_callee(UINT8 *msg);
extern INT32 sdec_m33_attach(struct sdec_feature_config *cfg_param);
extern void subt_disply_bl_init(struct sdec_device *dev);

#ifdef __cplusplus
}
#endif

#endif /*__LLD_SDEC_M3327_H__*/



