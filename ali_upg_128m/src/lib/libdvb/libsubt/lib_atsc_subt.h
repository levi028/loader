/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_atsc_subt.h

   *    Description:define the data, structure uisng by the specification of
        SUBTITLING METHODS FOR BROADCAST CABLE
        History:
        create thie file 2009/8/26 by zhaojian_tang (zhaojian_tang@ali.com.tw)
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    __ATSC_SUBT_H__
#define __ATSC_SUBT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ATSC_SUBT_TABLE_ID 0XC6
#define ATSC_SUBT_TABLE_ID_OFFSET 0X05
#define MAX_DISPLAY_DURATION 2000
#define ATSC_SUBT_HEAD1_LEN    4
#define ATSC_SEGMENT_OVERLAY_LEN 0X05
#define ATSC_SBUT_HEAD2_LEN 12
#define ATSC_SBUT_CRC_LEN 4
#define ATSC_SUBT_FRAME_LEN 8
#define ATSC_SUBT_OUTLINE_LEN 3
#define ATSC_SUBT_DROP_SHADOW_LEN 3
#define ATSC_SUBT_SIMPLY_BMP_HEAD_LEN 9
#define ATSC_SUBT_DIVIDE_SEG 0
#define MODIFY_DISPLAY_DURATION_BY_H_LIMITED    240
#define modify_display_duration_by_v_limited    60
#define OSD_RECT_REDUNDANCY                     1
#define SUBT_DEFAULT_DISPLAY_DURATION           5000
#define SUBT_DISPLAY_DURATION_LIMITED           2500
#define MASK_LOW_FOUR_BIT 0xf
#define EIGHT_BIT 8
#define REASSEMBLY_LENGTH 1024*5
#define bswap_constant_16(x)        \
      (UINT16)((((UINT16)(x) & 0xff00) >> 8) |    \
        (((UINT16)(x) & 0x00ff) << 8))

#define bswap_constant_32(x)            \
      (UINT32)((((UINT32)(x) & 0xff000000u) >> 24) |    \
        (((UINT32)(x) & 0x00ff0000u) >>  8) |    \
        (((UINT32)(x) & 0x0000ff00u) <<  8) |    \
        (((UINT32)(x) & 0x000000ffu) << 24))

typedef enum
{
    no = 0,
    yes =1
}YES_NO;

typedef enum
{
    TYPE_720_480_30 = 0,
    TYPE_720_576_25,
    TYPE_1280_720_60,
    TYPE_1920_1080_60,
    TYPE_DISPLAY_STANDARD_TYPE_SESERVED
}DISPLAY_STANDARD_TYPE;

typedef enum
{
    ATSC_SUBT_TYPE_SESERVED0 = 0,
    ATSC_SIMPLY_BITMAP,
    ATSC_SUBT_TYPE_SESERVED1,
    ATSC_SUBT_TYPE_SESERVED2
}ATSC_SUBTITLE_TYPE;



typedef enum
{
    ATSC_SUBT_CHAR_COLOR_INDEX = 0XF2,
    ATSC_SUBT_FRAME_COLOR_INDEX,
    ATSC_SUBT_OUTLINE_COLOR_INDEX,
    ATSC_SUBT_SHADOW_COLOR_INDEX
}ATSC_SUBTITLE_COLOR_TYPE;

typedef enum
{
    SIMPLY_BITMAP_NO_OPERATION = 0,
    SIMPLY_BITMAP_END_OF_LINE = 1,
    SIMPLY_BITMAP_PIXELS_ON    = 0X10,
    SIMPLY_BITMAP_PIXELS_OFF    = 0X20,
    SIMPLY_BITMAP_PIXELS_ON_OFF    = 0X80,
}ATSC_SUBTITLE_SIMPLY_BITMAP_PATTERN;

typedef enum
{
    SIMPLY_BITMAP_ALL_ON_PIXEL_LEN = 16,
    SIMPLY_BITMAP_ALL_OFF_PIXEL_LEN= 64,
    SIMPLY_BITMAP_PART_ON_PIXEL_LEN= 8,
    SIMPLY_BITMAP_PART_OFF_PIXEL_LEN= 32
}ATSC_SUBTITLE_SIMPLY_BITMAP_PIXEL_LEN;

typedef enum
{
    OUTLINE_NONE = 0, //vicky20110322
    OUTLINE_OUTLINE,
    OUTLINE_DROP_SHADOW,
    OUTLINE_RESERVED
} ATSC_SUBTITLE_SIMPLY_BITMAP_OUTLINE_STYLE;

typedef enum
{
    BG_TRANSPARENT = 0,
    BG_FRAME
} ATSC_SUBTITLE_SIMPLY_BITMAP_BG_STYLE;


typedef struct
{
    UINT8 table_id:8;

    UINT8 section_len4:4;
    UINT8 iso_reserve:2;
    UINT8 zero1:2;

    UINT8 section_len8;

    UINT32 protocol_version:6;
    UINT32 segmentation_overlay_included:1;
    UINT8 zero2:1;
} atsc_subt_fixed_header1;

typedef struct
{
    UINT8 table_extension_hight;
    UINT8 table_extension_low;

    UINT8    last_segment_number_hight;

    UINT8    segment_number_hight:4;
    UINT8    last_segment_number_low:4;

    UINT8   segment_number_low;
} atsc_subt_segment_overlay;


typedef struct
{
    UINT8 iso_639_language_code[3];

    UINT8 display_standard:5;
    UINT8 reserved1:1;
    UINT8 immediate:1;
    UINT8 pre_clear_display:1;

    UINT32 display_in_pts;

    UINT8 display_duration1:3;
    UINT8 reserved2:1;
    UINT8 subtitle_type:4;

    UINT8 display_duration2;

    UINT16 block_length;

} atsc_subt_fixed_header2;

typedef struct
{
    UINT8 stuffing_descriptor_tag;
    UINT8 stuffing_descriptor_len;
    UINT8 *stuffing_descriptor_string;
}atsc_subt_stuffing_descriptor;

typedef struct
{
    UINT8 y_component;
    UINT8 cr_component;
    UINT8 cb_component;
    UINT8 opaque_enable;
}atsc_subt_char_color;

typedef struct
{
    UINT16 frame_t_h_coordinate;
    UINT16 frame_t_v_coordinate;
    UINT16 frame_b_h_coordinate;
    UINT16 frame_b_v_coordinate;
    atsc_subt_char_color frame_color;
}atsc_subt_frame;


typedef struct
{
    UINT8 outline_thickness;
    atsc_subt_char_color outline_color;
}atsc_subt_outline;

typedef struct
{
    UINT8 shadow_right;
    UINT8 shadow_bottom;
    atsc_subt_char_color shadow_color;
}atsc_subt_drop_shadow;


typedef struct
{
    UINT8 outline_style:2;
    UINT8 backgroud_style:1;
    UINT8 resversed1:5;
    atsc_subt_char_color char_color;
    UINT16 bitmap_t_h_coordinate;
    UINT16 bitmap_t_v_coordinate;
    UINT16 bitmap_b_h_coordinate;
    UINT16 bitmap_b_v_coordinate;

    atsc_subt_frame bmp_frame;
    atsc_subt_outline bmp_outline;
    atsc_subt_drop_shadow bmp_drop_shadow;
    UINT16 bmp_length;

} atsc_subt_bmp_header;

struct scte_subt_msg    // total 36B
{
    BOOL b_info_valid;                    // 4B
    atsc_subt_fixed_header1 header1;    // 4B
    atsc_subt_segment_overlay overlay;    // 5B, Align-use3B
    atsc_subt_fixed_header2 header2;    // 12B
    UINT32 u_data_size;                 // 4B
    UINT8 *buff;                         // 4B
};

extern UINT16 globe_width_real;
extern UINT16 globe_height_real;
extern UINT8 osd_status;
extern struct osdrect g_subt_atsc_rect;

UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data);
void lib_subt_atsc_delete_timer();
void lib_subt_atsc_clean_up(void);
RET_CODE lib_subt_atsc_create_region(struct osd_device *subt_atsc_osd_dev,
    struct osdrect *rect, DISPLAY_STANDARD_TYPE type);
#ifdef DUAL_ENABLE
INT32 atsc_subt_sec_parse_create(void);
INT32 atsc_subt_sec_parse_wr_req(UINT32 * pSize, UINT8 ** ppData);
void atsc_subt_sec_parse_wr_update(UINT32 Size);
INT32 atsc_subt_sec_parse_rd_req(UINT32 * pSize, UINT8 ** ppData);
void atsc_subt_sec_parse_rd_update(UINT32 Size);
INT32 atsc_sdec_m3327_request_write(void* pdev,UINT32 uSizeRequested,UINT8** ppuData,UINT32* puSizeGot);
void atsc_sdec_m3327_update_write(void* pdev,UINT32 uSize);
INT32 lib_subt_atsc_sec_parse_task_start(INT32 param);	
INT32 lib_subt_atsc_sec_parse_task_stop(INT32 param) ;
#endif
#ifdef __cplusplus
 }
#endif



#endif
