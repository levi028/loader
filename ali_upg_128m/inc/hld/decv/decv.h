/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: decv.h
*
*    Description: This file contains definitions of video decoder's api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _DECV_H_
#define  _DECV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <mediatypes.h>
#include <sys_config.h>
#include <hld/dis/vpo.h>
#include <hld/snd/snd.h>

#define VDEC_NAME_LEN                       (16)

#define VDEC27_H_PRECISION_2                (0x02<<16)
#define VDEC27_H_PRECISION_4                (0x04<<16)
#define VDEC27_H_PRECISION_8                (0x08<<16)

#define VDEC27_V_PRECISION_2                0x02
#define VDEC27_V_PRECISION_4                0x04
#define VDEC27_V_PRECISION_8                0x08

#define VDEC27_PREVIEW_VE_SCALE             0x01
#define VDEC27_PREVIEW_DE_SCALE             0x02

#ifndef __ATTRIBUTE_REUSE_
#define __ATTRIBUTE_REUSE_                  __attribute__((section(".reuse")))
#endif

#define VDEC_SPEC_RES_MAX_INDEX             2

#define VDEC27_DROP_THRESHOLD               20

#define VDEC27_DVIEW_PRECISION              (VDEC27_H_PRECISION_4|VDEC27_V_PRECISION_4)

#if ((defined HDTV_SUPPORT) || ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE == CPU_M6303) \
    && (SYS_PROJECT_FE==PROJECT_FE_DVBT || defined(M3105_CHIP))))//for 3105 or M3105D+3501
#define VDEC27_PREVIEW_SOLUTION             VDEC27_PREVIEW_VE_SCALE
#else
#define VDEC27_PREVIEW_SOLUTION             VDEC27_PREVIEW_DE_SCALE
#endif

#define VDEC_DISPLAY                        0x01
#define VDEC_UN_DISPLAY                     0x02
#define VDEC_FRM_REF                        0x03

#define VDEC_SYNC_PTS                       0x00
#define VDEC_SYNC_FREERUN                   0x01

#define VDEC_SYNC_FIRSTFREERUN              0x01
#define VDEC_SYNC_I                         0x02
#define VDEC_SYNC_P                         0x04
#define VDEC_SYNC_B                         0x08
#define VDEC_SYNC_HIGH_LEVEL                0x10

#define SP_ML                               0x58
#define MP_LL                               0x4A
#define MP_ML                               0x48
#define MP_H14                              0x46
#define MP_HL                               0x44
#define SNR_LL                              0x3A
#define SNR_ML                              0x38
#define spatial_h14                         0x26
#define HP_ML                               0x18
#define HP_H14                              0x16
#define HP_HL                               0x14

#define PHY_ADDR(addr)                      ((void *)(((UINT32)(addr))&0x1FFFFFFF))
#define CACHE_ADDR(addr)                    ((void *)((((UINT32)(addr))&0x1FFFFFFF)|0x80000000))
#define UNCACHE_ADDR(addr)                  ((void *)((((UINT32)(addr))&0x1FFFFFFF)|0xa0000000))

#ifndef MIN
#define MIN(a, b)                           (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)                           (((a) > (b)) ? (a) : (b))
#endif

#define NO_WARNING(a)                       (a = a)

#define VDEC_DETACH                         10
#define VDEC_CLOSED                         11
#define VDEC_DECODING                       VDEC27_STARTED
#define VDEC_REVERSING                      12
#define VDEC_PAUSED                         VDEC27_PAUSED
#define VDEC_STOPPED                        VDEC27_STOPPED

#define VDEC_DEV_MAX                        4
#define VDEC_GET_FRM_MAX_NUM                3
#define DTVCC_USER_DATA_LENGTH_MAX          210

#define VDEC_FREEBUF_USER_DB                0x01
#define VDEC_FREEBUF_USER_OSD               0x02

#define VDEC_IO_FILL_FRM                    0x03
#define VDEC_IO_REG_CALLBACK                0x04
#define VDEC_IO_GET_STATUS                  0x05
#define VDEC_IO_GET_MODE                    0x06
#define VDEC_IO_GET_FRM                     0x07
#define VDEC_IO_WHY_IDLE                    0x08
#define VDEC_IO_GET_FREEBUF                 0x0A
#define VDEC_IO_GET_FRM_ADVANCE             0x0D
#define VDEC_IO_FILL_FRM2                   0x10
#define VDEC_IO_COLORBAR                    0x14
#define VDEC_IO_SET_SCALE_MODE              0x17
#define VDEC_IO_GET_OUTPUT_FORMAT           0x21
#define VDEC_IO_MAF_CMD                     0x22
#define VDEC_SET_DMA_CHANNEL                0x24
#define VDEC_IO_SWITCH                      0x25
#define VDEC_VBV_BUFFER_OVERFLOW_RESET      0x28
#define VDEC_IO_FIRST_I_FREERUN             0x2A
#define VDEC_IO_DONT_RESET_SEQ_HEADER       0x2C
#define VDEC_IO_SET_DROP_FRM                0x2D
#define VDEC_IO_MULTIVIEW_HWPIP             0x2E
#define VDEC_DTVCC_PARSING_ENABLE           0x2f
#define VDEC_IO_PLAYBACK_PS                 0x31
#define VDEC_IO_PLAY_FROM_STORAGE           0x32
#define VDEC_IO_CHANCHG_STILL_PIC           0x33
#define VDEC_IO_SET_SYNC_DELAY              0x34
#define VDEC_IO_SAR_ENABLE                  0x35
#define VDEC_IO_SEAMLESS_SWITCH_ENABLE      0x38
#define VDEC_IO_PAUSE_VIDEO                 0x39
#define VDEC_IO_CONTINUE_ON_ERROR           0x3a
#define VDEC_IO_SET_DECODER_MODE            0x40
#define VDEC_IO_SET_FREERUN_THRESHOLD       0x41
#define VDEC_IO_SET_OUTPUT_RECT             0x42
#define VDEC_IO_SET_AVSYNC_GRADUALLY        0x43
#define VDEC_IO_DBLOCK_ENABLE               0x44
#define VDEC_IO_GET_DECORE_STATUS           0x46
#define VDEC_IO_SET_DEC_FRM_TYPE            0x47
#define VDEC_IO_FILL_BG_VIDEO               0x48
#define VDEC_IO_BG_FILL_BLACK               0x49
#define VDEC_IO_GET_MULTIVIEW_BUF           0x50
#define VDEC_IO_SET_MULTIVIEW_WIN           0x51
#define VDEC_IO_SLOW_PLAY_BEFORE_SYNC       0x52
#define VDEC_IO_DONT_KEEP_PRE_FRAME         0x53
#define VDEC_IO_SET_SEND_GOP_GAP            0x54
#define VDEC_IO_SET_CC_USER_DATA_INFO       0x55
#define VDEC_IO_GET_CC_USER_DATA_INFO       0x56
#define VDEC_IO_GET_FRAME_INFO              0x57
#define VDEC_IO_FLUSH                       0x58
#define VDEC_IO_SET_SLOW_PLAY_TRD           0x59
#define VDEC_IO_GET_AVSYNC_PARAM            0x5a
#define VDEC_IO_SET_AVSYNC_PARAM            0x5b
#define VDEC_IO_SET_SPEC_RES_AVSYNC_RANGE   0x5c
#define VDEC_IO_DROP_FREERUN_PIC            0X5d //only support on mpeg2 driver, drop freerun picture before first picture show.
#define VDEC_IO_SET_PLAY_SCENES             0x5e //only used by dmx
#define VDEC_IO_RESTART_DECODE              0x5f
#define VDEC_IO_GET_ALL_USER_DATA           0x60 //only support on mpeg2 driver, transfer all user data to main cpu.
#define VDEC_IO_SET_SPEC_PLAY_MODE          0x61 //only for dmx used
#define VDEC_IO_RELEASE_VFB                 0x62
#define VDEC_IO_SET_TRICK_MODE              0x63
#define VDEC_IO_GET_CAPTURE_FRAME_INFO      0x64
#define VDEC_IO_DYNAMIC_FB_ALLOC            0x65
#define VDEC_IO_SET_DISPLAY_MODE            0x66
#define VDEC_IO_SET_PIP_PARAM               0x67
#define VDEC_IO_SET_ID                      0x68
#define VDEC_IO_PARSE_AFD                   0x69
#define VDEC_IO_GET_USER_DATA_INFO          0x70 //only support on mpeg2 driver, user data include current frame header info
#define VDEC_IO_SET_SYNC_REPEAT_INTERVAL    0x71
#define VDEC_IO_SET_VIDEOMASTER_MODE        0x7a
#define VDEC_IO_SET_UPDATE_PTS_TO_DMX_CB    0x7b

#define VDEC_IO_ELE_BASE                    0x10000
#define VDEC_IO_PLAYBACK_STR                (VDEC_IO_ELE_BASE + 0x01)
#define VDEC_IO_REG_DROP_FUN                (VDEC_IO_ELE_BASE + 0x02)
#define VDEC_IO_REST_VBV_BUF                (VDEC_IO_ELE_BASE + 0x03)
#define VDEC_IO_KEEP_INPUT_PATH_INFO        (VDEC_IO_ELE_BASE + 0x04)
#define VDEC_IO_PLAY_MEDIA_STR              (VDEC_IO_ELE_BASE + 0x05)
#define VDEC_IO_CAPTURE_DISPLAYING_FRAME    (VDEC_IO_ELE_BASE + 0x09)
#define VDEC_IO_REG_SET_SYNC_FLAG_CB        (VDEC_IO_ELE_BASE + 0x0C)

typedef void (* vdec_cbfunc)(UINT32 u_param1, UINT32 u_param2);
typedef void (* vdec_pull_down_func)(UINT32);
typedef void (* VDEC_BEYOND_LEVEL)();

enum VDEC_OUTPUT_MODE
{
    MP_MODE,        // main picture
    PIP_MODE,       // picture in picture
    PREVIEW_MODE,
    HSCALE_MODE,
    DVIEW_MODE,
    MP_DVIEW_MODE,
    HSCALE_DVIEW_MODE,
    AUTO_MODE,
    DUAL_MODE,
    DUAL_PREVIEW_MODE,
    IND_PIP_MODE,
    SW_PASS_MODE,
    HW_DEC_ONLY_MODE,
    MULTI_VIEW_MODE,
    RESERVE_MODE,
    VDEC_OUTPUT_MODE_END
};

enum VDEC_STATUS
{
    VDEC27_ATTACHED,
    VDEC27_STARTED,
    VDEC27_STOPPED,
    VDEC27_PAUSED,
};

enum vdec_speed
{
    VDEC_SPEED_1_2,
    VDEC_SPEED_1_4,
    VDEC_SPEED_1_8,
    VDEC_SPEED_1_16,
    VDEC_SPEED_1_32,
    VDEC_SPEED_STEP,
    VDEC_SPEED_1,
    VDEC_SPEED_2,
    VDEC_SPEED_4,
    VDEC_SPEED_8,
    VDEC_SPEED_16,
    VDEC_SPEED_32,
};

enum vdec_direction
{
    VDEC_FORWARD = 0,
    VDEC_BACKWARD
};

enum vdecdecoder_mode
{
    VDEC_FULL_MODE,
    VDEC_QUICK_MODE
};

struct vdec_dvr_config_param
{
    BOOL is_scrambled;
};

struct vdec_status_info
{
    enum VDEC_STATUS u_cur_status;
    BOOL   u_first_pic_showed;
    BOOL   b_first_header_got;
    UINT16 pic_width;
    UINT16 pic_height;
    UINT16 status_flag;
    UINT32 read_p_offset;
    UINT32 write_p_offset;
    UINT32 display_idx;
    BOOL   use_sml_buf;
    enum VDEC_OUTPUT_MODE  output_mode;
    UINT32 valid_size;
    UINT32 mpeg_format;
    enum asp_ratio aspect_ratio;
    UINT16 frame_rate;
    UINT32 bit_rate;
    BOOL   hw_dec_error;
    BOOL   display_frm;
    UINT8  top_cnt;
    UINT8  play_direction;
    UINT8  play_speed;
    UINT8  api_play_direction;
    UINT8  api_play_speed;
    BOOL   is_support;
    UINT32 vbv_size;
    UINT8  cur_dma_ch;
    BOOL   progressive;
	INT32  top_field_first;
    INT32  first_pic_decoded;
    UINT32 frames_decoded;
    UINT32 frame_last_pts;
	UINT32 sar_width;
	UINT32 sar_height;
	UINT8  active_format;
    UINT8  layer;
    UINT8  ff_mode;
	UINT16 max_width;
    UINT16 max_height;
    UINT16 max_frame_rate;
};

/*! @struct vdec_capture_frm_info
@brief param of VDEC_IO_GET_CAPTURE_FRAME_INFO.
*/
struct vdec_capture_frm_info
{
    UINT32 pic_height;
    UINT32 pic_width;
    UINT32 pic_stride;
    UINT32 y_buf_addr;
    UINT32 y_buf_size;
    UINT32 c_buf_addr;
    UINT32 c_buf_size;
    UINT8 de_map_mode;
};

enum vdec_cbtype
{
    VDEC_CB_SETTING_CHG = 0,
    VDEC_CB_REQ_DATA,
    VDEC_CB_STOP,
    VDEC_CB_FINISH_CUR_PIC,
    VDEC_CB_FINISH_I_FRAME,
    VDEC_CB_FINISH_TARGET_FRAME,
    VDEC_CB_FIRST_SHOWED,
    VDEC_CB_MODE_SWITCH_OK,
    VDEC_CB_BACKWARD_RESTART_GOP,
    VDEC_CB_OUTPUT_MODE_CHECK,
    VDEC_CB_FIRST_HEAD_PARSED,
    VDEC_CB_MONITOR_FRAME_VBV,
    VDEC_CB_MONITOR_VDEC_START,
    VDEC_CB_MONITOR_VDEC_STOP,
    VDEC_CB_FIRST_I_DECODED,
    VDEC_CB_MONITOR_USER_DATA_PARSED,
    VDEC_CB_INFO_CHANGE,
    VDEC_CB_ERROR,
    VDEC_CB_STATE_CHANGED,
    VDEC_CB_DECODER_FINISH,
    VDEC_CB_MALLOC_DONE,
    VDEC_CB_FRAME_DISPLAYED,
	VDEC_CB_MONITOR_GOP,
};

/*! @enum vdec_state_flags
@brief Define flags for vdec state
*/
enum vdec_state_flags
{
    VDEC_STATE_NODATA    = 0x0001,    //!<No data state
    VDEC_STATE_DECODING  = 0x0002,    //!<Decoding state
};

/*! @enum vdec_error_flags
@brief Define flags for vdec error
*/
enum vdec_error_flags
{
    VDEC_ERROR_NODATA    = 0x0001,    //!<No data error
    VDEC_ERROR_HARDWARE  = 0x0002,    //!<Decode error
    VDEC_ERROR_SYNC      = 0x0004,    //!<Sync error
    VDEC_ERROR_FRAMEDROP = 0x0008,    //!<frame drop
    VDEC_ERROR_FRAMEHOLD = 0x0010,    //!<Frame hold
    VDEC_ERROR_GIVEUPSEQ = 0x0020,    //!<Give up sequence
    VDEC_ERROR_INVDATA   = 0x0040,    //!<Invalid data
};

/*! @enum vdec_info_change_flags
@brief Define flags for information changing
*/
enum vdec_info_change_flags
{
    VDEC_CHANGE_DIMENSIONS = 0x0001,    //!<Dimensions change
    VDEC_CHANGE_FRAMERATE  = 0x0002,    //!<Frame rate change
    VDEC_CHANGE_AFD        = 0x0004,    //!<AFD change
    VDEC_CHANGE_SAR        = 0x0008,    //!<SAR change
};

/*! @struct vdec_info_cb_param
@brief Describe video callback parameters for information change
*/
struct vdec_info_cb_param
{
    UINT32 info_change_flags;   //!<Indicate which information change
    UINT32 pic_width;           //!<Picture width
    UINT32 pic_height;          //!<Picture height
    UINT32 frame_rate;          //!<Frame rate
    UINT8  active_format;       //!<Active format
	UINT32 sar_width;           //!<Sample aspect ratio width
    UINT32 sar_height;          //!<Sample aspect ratio height
};

struct vdec_callback
{
    vdec_cbfunc pcb_first_showed;
    vdec_cbfunc pcb_mode_switch_ok;
    vdec_cbfunc pcb_backward_restart_gop;
    vdec_cbfunc pcb_first_head_parsed;
    vdec_cbfunc pcb_new_frame_coming;
    vdec_cbfunc pcb_vdec_start;
    vdec_cbfunc pcb_vdec_stop;
    vdec_cbfunc pcb_vdec_user_data_parsed;
	vdec_cbfunc pcb_vdec_decoder_finish;
    vdec_cbfunc pcb_first_i_decoded;
    vdec_cbfunc pcb_vdec_info_changed;
    vdec_cbfunc pcb_vdec_error;
    vdec_cbfunc pcb_vdec_state_changed;
	vdec_cbfunc pcb_vdec_malloc_done;
	vdec_cbfunc pcb_frame_displayed;
    vdec_cbfunc pcb_vdec_new_frame;
	vdec_cbfunc pcb_vdec_monitor_gop;
};

struct multi_view_setting
{
    UINT32 window_width;
    UINT32 window_height;
    UINT32 multi_view_buf;
    UINT32 multi_view_buf_addr;
    UINT32 multi_view_buf_size;
};

struct adv_setting
{
    UINT8 init_mode;
    enum tvsystem out_sys;
    BOOL bprogressive;
    UINT8 switch_mode;  /* 1: mp<=>preview switch smoothly */
};

struct vdec_pipinfo
{
    struct position pip_sta_pos;
    struct rect_size pip_size;
    struct rect_size mp_size;
    BOOL b_use_bg_color;
    struct ycb_cr_color bg_color;

    BOOL buse_sml_buf;

    struct rect src_rect;
    struct rect dst_rect;

    struct adv_setting adv_setting;
    struct multi_view_setting para;
};

struct vdec_frm_output_format
{
    // VE config
    BOOL   h_scale_enable;
    UINT32 h_scale_factor;    //0:reserved, 1: Y h_scale only, 2: Y,C h_scale
    BOOL   dview_enable;
    UINT32 dv_h_scale_factor; //0:no dview, 1: 1/2 dview, 2: 1/4 dview, 3: 1/8 dview
    UINT32 dv_v_scale_factor; //0:no dview, 1: 1/2 dview, 2: 1/4 dview, 3: 1/8 dview
    UINT32 dv_mode;

    //DE config
    UINT32 field_src;  //0: both fields, 1:top only field
    UINT32 scaler_src; //0: frame base, 1: field base
    UINT32 vpp_effort; //0:high, 1: middle, 2: low, 3:very low
};

struct vdec_frm_info_api
{
    UINT32 u_y_addr;
    UINT32 u_c_addr;
    UINT16 u_width;
    UINT16 u_height;
    UINT32 u_y_size;
    UINT32 u_c_size;
};

struct vdec_mem_map
{
    UINT32 frm0_y_size;
    UINT32 frm0_c_size;
    UINT32 frm1_y_size;
    UINT32 frm1_c_size;
    UINT32 frm2_y_size;
    UINT32 frm2_c_size;

    UINT32 frm0_y_start_addr;
    UINT32 frm0_c_start_addr;
    UINT32 frm1_y_start_addr;
    UINT32 frm1_c_start_addr;
    UINT32 frm2_y_start_addr;
    UINT32 frm2_c_start_addr;

    UINT32 dvw_frm_size;
    UINT32 dvw_frm_start_addr;

    UINT32 maf_size;
    UINT32 maf_start_addr;

    UINT32 vbv_size;
    UINT32 vbv_start_addr;
    UINT32 vbv_end_addr;

    UINT32 frm3_y_size;
    UINT32 frm3_c_size;
    UINT32 frm3_y_start_addr;
    UINT32 frm3_c_start_addr;

    UINT32 frm_num;//3
    UINT32 res_bits;
    UINT32 *res_pointer;

    UINT32 ext_maf_buf1;
    UINT32 ext_maf_buf2;
    UINT32 auxp_buf;
};

struct vdec_adpcm
{
    UINT8 adpcm_mode;
    UINT8 adpcm_ratio;
};

struct vdec_sml_frm
{
    UINT8 sml_frm_mode;
    UINT32 sml_frm_size;
};

struct vdec_config_par
{
    //feature entrys
    UINT8 user_data_parsing;
    UINT8 dtg_afd_parsing;
    UINT8 drop_freerun_pic_before_firstpic_show;
    UINT8 reset_hw_directly_when_stop;
    UINT8 show_hd_service;
    UINT8 still_frm_in_cc;
    UINT8 extra_dview_window;
    UINT8 not_show_mosaic;
    UINT8 return_multi_freebuf;
    UINT8 advance_play;
    struct vdec_adpcm adpcm;
    struct vdec_sml_frm sml_frm;
    UINT8 lm_shiftthreshold;
    UINT8 vp_init_phase;
    UINT8 preview_solution;
    struct vdec_mem_map mem_map;
    UINT32 res_bits;
    UINT32 *res_pointer;
};

struct vdec_device
{
    struct vdec_device *next;
    UINT32 type;
    INT8   name[32];
    UINT8  flags;
    UINT8  index;
    void   *top_info;
    void   *priv;

    RET_CODE (*open)(struct vdec_device *);
    RET_CODE (*close)(struct vdec_device *);
    RET_CODE (*start)(struct vdec_device *);
    RET_CODE (*stop)(struct vdec_device *,BOOL,BOOL);
    RET_CODE (*vbv_request)(struct vdec_device *, UINT32, void **, UINT32 *, struct control_block *);
    void     (*vbv_update)(struct vdec_device *, UINT32);
    RET_CODE (*set_output)(struct vdec_device *, enum VDEC_OUTPUT_MODE, struct vdec_pipinfo *, \
                           struct mpsource_call_back *, struct pipsource_call_back *);
    RET_CODE (*sync_mode)(struct vdec_device *,  UINT8,UINT8);
    RET_CODE (*ioctl)(struct vdec_device *, UINT32 , UINT32);
    /* for advanced play */
    RET_CODE (*playmode)(struct vdec_device *, enum vdec_direction , enum vdec_speed);
    RET_CODE (*step)(struct vdec_device *);
    RET_CODE (*dvr_pause)(struct vdec_device *);
    RET_CODE (*dvr_resume)(struct vdec_device *);
    RET_CODE (*profile_level)(struct vdec_device *,  UINT8,VDEC_BEYOND_LEVEL);
    RET_CODE (*dvr_set_param)(struct vdec_device *, struct vdec_dvr_config_param);
    /* end */
    RET_CODE (*internal_set_output)(struct vdec_device *, enum VDEC_OUTPUT_MODE, struct vdec_pipinfo *, \
                                    struct mpsource_call_back *, struct pipsource_call_back *);
    RET_CODE (*internal_set_frm_output_format)(struct vdec_frm_output_format *);
    void     (*de_hw_using)(struct vdec_device *, UINT8, UINT8, UINT8);
};

struct vdec_control
{
    struct vdec_device *dev[VDEC_DEV_MAX];
    UINT8 cur_idx;
};

struct back_pip_info
{
    UINT32 left_x;
    UINT32 left_y;
    UINT32 window_width;
    UINT32 window_height;
    UINT32 full_screen_width;
    UINT32 full_screen_height;
};

struct multiview_info
{
    UINT32 multiview_buf;
    UINT32 full_screen_width;
    UINT32 full_screen_height;
    UINT32 multiview_buf_length;
};

struct vdec_io_reg_callback_para
{
    enum vdec_cbtype e_cbtype;
    vdec_cbfunc p_cb;
    UINT32 monitor_rate;
};

struct vdec_io_set_slow_play_para
{
    UINT32 slow_play_hold_trd;
    UINT32 slow_play_show_trd;
};

struct vdec_io_get_frm_para
{
    UINT8 ufrm_mode;
    struct vdec_frm_info_api t_frm_info;
};

struct vdec_io_get_frm_para_advance
{
    UINT8 ufrm_mode;
    UINT8 request_frm_number;
    struct vdec_frm_info_api t_frm_info[VDEC_GET_FRM_MAX_NUM];
    UINT8 return_frm_number;
};

struct user_data_pram
{
    UINT32 user_data_size;
    UINT8 user_data[DTVCC_USER_DATA_LENGTH_MAX];
};

struct use_data_header_info
{
    UINT32 top_field_first     :1;
    UINT32 repeat_first_field  :1;
    UINT32 reserve_0           :30;
};
struct vdec_get_frm_info
{
    UINT32 pic_height;
    UINT32 pic_width;
    UINT32 y_buf_addr;
    UINT32 y_buf_size;
    UINT32 c_buf_addr;
    UINT32 c_buf_size;
    UINT8 de_map_mode;
    UINT32 pic_width_dbk;
};

struct vdec_io_get_freebuf_para
{
    UINT8  request_user_id;
    UINT32 request_size;
    UINT32 got_free_addr;
    UINT32 got_free_size;
};

struct vdec_adpcm_cmd
{
    BOOL onoff;
    UINT8 ratio;
};

struct vdec_maf_cmd
{
    BOOL onoff;
};

struct vdec_err_conceal_cmd
{
    BOOL onoff;
    UINT8 method;
    UINT8 threshold;
    UINT8 next_mb_x;
    UINT8 next_mb_y;
};

struct vdec_abs_cmd
{
    BOOL onoff;
    BOOL adaptive_threshold;
    UINT8 threshold;
};

enum vdec_out_pic_type
{
    VDEC_PIC_IMC1,
    VDEC_PIC_IMC2,
    VDEC_PIC_IMC3,
    VDEC_PIC_IMC4,
    VDEC_PIC_YV12
};

struct vdec_picture
{
    enum vdec_out_pic_type type;
    UINT8  *out_data_buf;
    UINT32 out_data_buf_size;
    UINT32 out_data_valid_size;
    UINT32 pic_width;
    UINT32 pic_height;
};

enum video_decoder_type
{
    MPEG2_DECODER = 0,
    H264_DECODER,
    AVS_DECODER,
	H265_DECODER,
    VC1_DECODER,
    MPEG4_DECODER,
    VP8_DECODER,
    RV_DECODER,
    MJPG_DECODER,

    INVALID_DECODER = 0xFF,
};

struct vdec_pip_param
{
    UINT32 layer;
};

struct vdec_info
{
    enum video_decoder_type vdec_type;
    struct vdec_device *vdec_dev;
    void *vdec_priv;
    void *vdec_mgr;
    UINT8 vdec_id;
    UINT8 vdec_mode; /* 0: live 1: media player */
    UINT8 preview;
    UINT8 layer;
    UINT32 hw_request[2]; /* 0: MD 1: EP A */
    BOOL hw_using[2];
    void (*vdec_interrupt)(void *);
    INT32 (*vdec_decode_function)(void *phandle, int cmd, void *pparam1, void *pparam2);
    struct source_callback display_callback;
    INT32 (*md_wake_up)(void *pparam);
    INT32 (*ep_wake_up)(void *pparam);
};

struct vdec_manager_priv
{
    struct vdec_info *vdec_obj[VDEC_DEV_MAX];
    UINT8 vdec_num;
    BOOL register_isr;
};


struct vdec_vfb_cfg
{
    UINT32 enable;
    UINT32 start_addr;
    UINT32 end_addr;
    UINT8 vdec_id;
};

struct vdec_cc_user_data_info
{
    UINT8 *array;
    INT32 array_cnt;
};

struct vdec_fill_frm_info
{
    UINT32 top_y;
    UINT32 top_c;
    UINT32 bot_y;
    UINT32 bot_c;
    UINT32 y_buf_size;
    UINT32 c_buf_size;
	UINT32 sample_bits_y;
	UINT32 sample_bits_c;
	BOOL is_compressed;
	t_e_mapping_mode mapping_type;
};


/*! @enum decv_sync_mode
@brief Set vdec sync mode.
*/
enum decv_sync_mode
{
    VDEC_SYNC_MODE_PCR = 0,
    VDEC_SYNC_MODE_AUDIO,
    VDEC_SYNC_MODE_FREERUN,
};

/*! @enum decv_play_scens
@brief Set play scens, only for drvier used.
*/
enum decv_play_scens
{
    MP_SCENES = 0,      //!<MP play
    DMX_SCENES,         //!<DMX play, include live play, timeshift, playback
    DMX_VE_QUALITY, //!<just for video signal quality test stream
};

/*! @struct video_avsync_param
@brief Define parameter of video av sync
*/
struct video_avsync_param
{
    enum av_sync_accuracy_level av_sync_level;
	enum decv_sync_mode vde_sync_mode;	//Read/Write.
	INT32 vde_avsync_adjust_value;
	INT32 vde_pts_stc_diff; 				//Only Read. APP can not modify the value
	INT32 vde_stc_offset;				    //Read/Write. 1. stc_offset>0, Audio will output early; 2.stc_offset<0, Audio will delay output.
	INT32 aud_pts_stc_diff; 				//It be assigned by SND module.
	UINT32 de_output_delay;			        //Unit: ms. DE engine delay, it be assigned by DE module.
	UINT32 cur_drop_threshold;
	UINT32 cur_hold_threshold;
    INT32 sync_high_threshold;
    INT32 sync_middle_threshold;
    INT32 sync_low_threshold;
	struct vpo_io_get_info vpo_info;
	UINT32 frame_rate;
    enum decv_play_scens play_scens;    	//!<Live play/Timeshift/Playback used DMX_SCENES, Other used MP_SCENES.
};

enum specified_resolution
{
    RES_I25_P50 = 0,
    RES_I30_P60 = 1,
};

struct spec_resolution_avsync_range
{
    enum specified_resolution spec_res;	/* for example av sync -20~+30ms*/
    UINT32 drop_threshold;		/* -15, */
    UINT32 hold_threshold;		/* +25, */
};

/*! @enum vdec_output_mode
@brief Video output mode
*/
enum vdec_output_mode
{
    VDEC_FULL_VIEW,    //!<Full screen display
    VDEC_PREVIEW,      //!<Preview display
    VDEC_SW_PASS,      //!<Do not decode, just consume data
};

/*! @struct vdec_display_rect
@brief Display rectangle
*/
struct vdec_display_rect
{
    INT32 src_x;    //!<Horizontal start point of source
    INT32 src_y;    //!<Vertical start point of source
    INT32 src_w;    //!<Horizontal size of source
    INT32 src_h;    //!<Vertical size of source
    INT32 dst_x;    //!<Horizontal start point of destination
    INT32 dst_y;    //!<Vertical start point of destination
    INT32 dst_w;    //!<Horizontal size of destination
    INT32 dst_h;    //!<Vertical size of destination
};

/*! @struct vdec_display_mode
@brief Display mode
*/
struct vdec_display_param
{
    enum vdec_output_mode mode;
    struct vdec_display_rect rect;
};

enum vdec_playback_rate
{
    VDEC_RATE_1_2,
    VDEC_RATE_1_4,
    VDEC_RATE_1_8,
    VDEC_RATE_1_16,
    VDEC_RATE_1_32,
    VDEC_RATE_STEP,
    VDEC_RATE_1,
    VDEC_RATE_2,
    VDEC_RATE_4,
    VDEC_RATE_8,
    VDEC_RATE_16,
    VDEC_RATE_32,
};

enum vdec_playback_dir
{
    VDEC_PLAY_FORWARD = 0,
    VDEC_PLAY_BACKWARD,
};

struct vdec_playback_param
{
    enum vdec_playback_dir direction;
    enum vdec_playback_rate rate;
	UINT32 mode;
};

/*! @enum VDecRotationAngle
@brief
The rotation angle of DECV module.
*/
enum vdec_rotation_angle
{
    VDEC_ANGLE_0,
    VDEC_ANGLE_90,
    VDEC_ANGLE_180,
    VDEC_ANGLE_270,

    VDEC_ANGLE_MAX,
};

enum VDEC_DBG_FLAG
{
	VDEC_DBG_NONE,
	VDEC_DBG_DEFAULT,
};

typedef enum
{
	FILL_COLOR_WHITE = 0x0,
	FILL_COLOR_YELLOW,
	FILL_COLOR_CYAN,
	FILL_COLOR_GREEN,
	FILL_COLOR_MAGENTA,
	FILL_COLOR_RED,
	FILL_COLOR_BLUE,
	FILL_COLOR_BLACK,
}SINGLE_COLOR;

typedef struct
{
    UINT32  top_y;
    UINT32  top_c;
    UINT32  bot_y;
    UINT32  bot_c;
    UINT32 y_buf_size;
    UINT32 c_buf_size;
	UINT32 sample_bits_y;
	UINT32 sample_bits_c;
	BOOL is_compressed;
	t_e_mapping_mode mapping_type;
}T_FILL_FRAME_BUFFER_INFO;

RET_CODE vdec_open(struct vdec_device *dev);
RET_CODE vdec_close(struct vdec_device *dev);
RET_CODE vdec_start(struct vdec_device *dev);
RET_CODE vdec_stop(struct vdec_device *dev,BOOL close_vp,BOOL fill_black);
RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 sync_mode,UINT8 sync_level);
RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct vdec_dvr_config_param param);
RET_CODE vdec_set_output(struct vdec_device *dev, enum VDEC_OUTPUT_MODE mode,struct vdec_pipinfo *init_info, \
                           struct mpsource_call_back *mp_call_back, struct pipsource_call_back *pip_call_back);
RET_CODE vdec_vbv_request(void *dev, UINT32 size_requested, void **v_data, \
                             UINT32 *size_got, struct control_block * ctrl_blk);
void     vdec_vbv_update(void *dev, UINT32 data_size);
RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param);
RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 profile_level, VDEC_BEYOND_LEVEL cb_beyond_level);
struct vdec_device * get_selected_decoder(void);
void     h264_decoder_select(int select, BOOL in_preivew);
BOOL     is_cur_decoder_avc(void);
enum video_decoder_type get_current_decoder(void);
void     video_decoder_select(enum video_decoder_type select, BOOL in_preview);
void     set_avc_output_mode_check_cb(vdec_cbfunc cb);

/* for advanced play */
RET_CODE vdec_dvr_pause(struct vdec_device *dev);
RET_CODE vdec_dvr_resume(struct vdec_device *dev);
RET_CODE vdec_playmode(struct vdec_device *dev, enum vdec_direction direction, enum vdec_speed speed);
RET_CODE vdec_step(struct vdec_device *dev);
RET_CODE vdec_enable_advance_play(struct vdec_device *dev);
/* end */

void     hld_decv_callee(UINT8 *msg);
void     vdec_m36_attach(struct vdec_config_par *pconfig_par);

enum output_frame_ret_code vdec_s3601_de_request(void *dev,struct display_info *p_display_info,\
                                                      struct request_info *p_request_info);
INT32    vdec_s3601_de_release(void *dev,struct release_info *p_release_info);

void vdec_copy_data(UINT32 dev, UINT32 src_addr, UINT32 req_data, UINT32 *got_size);

#ifdef AVC_FIRST_HEAD_PARSE_SUPPORT
extern UINT32 callback_avc_first_head_parse(UINT32 u_param1, UINT32 u_param2);
#endif

RET_CODE vdec_capture_display_frame(struct vdec_device *dev, struct vdec_picture *ppic);
void vdec_fill_single_color(struct vdec_fill_frm_info *src_info, struct ycb_cr_color *color);
UINT8* vdec_get_dev_name(enum video_decoder_type type);

#ifdef __cplusplus
 }
#endif

#endif  /* _DECV_H_*/

