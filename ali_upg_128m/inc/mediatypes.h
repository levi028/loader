 /**********************************************************************
 *
 *  Copyright (C) 2003 ALi (Shanghai) Corporation.  All Rights Reserved.
 *
 *  File:    mediatypes.h
 *
 *  Contents: definition of general A/V data structure of the system
 *            including
 *
 *            Position (x,y),
 *            Rectangle
 *
 *            Audio properties:   audio stream type,
 *                            bit rate,
 *                            sample frequency,
 *                            quantization word length
 *
 *            Video properties:    video stream type
 *                            TV system,
 *                            frame rate,
 *                            aspect ratio,
 *                            picture format,
 *                            YUV    color,
 *                            YCbCr address,
 *                            highlight information
 *
 *             MPEG-1/2 basic syntax/semantic unit :      SCR & PTS,
 *                                                start code ,
 *                                                stream/sub-stream ID
 *            DVD/VCD PE       A/V settings
 *
 *  History:
 *   Date                  By                Reason
 *   =========    =======    ====================
 *   10/17/2003      Mengdong Lin       create
 *
 **********************************************************************/
#ifndef     _MEDIA_TYPES_H_
#define     _MEDIA_TYPES_H_

//#include <alitypes.h>
#include <sys_config.h>
#include <basic_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define    USE_NEW_VDEC
#define VIRTUAL_DUAL_VIDEO_DECODER
#ifndef _BUILD_OTA_E_
#define DMX_XFER_V_ES_BY_DMA     //added for s3601. transfer video ES by DMA, others by CPU copy
#endif

#if(SYS_PROJECT_FE == PROJECT_FE_ATSC)
#define SC_SEARCH_BY_DMA        // because M3602/M3601c have fixed IC bug, so re-enable it
#endif

//#define    VIDEO_SECOND_B_MONITOR

#ifdef VIDEO_SECOND_B_MONITOR
#define    VIDEO_2B_MONITOR_MAX 50
#define    VIDEO_LINEMEET_1 0x01
#define    VIDEO_LINEMEET_2 0x02
#define    VIDEO_VBLANK     0x03
#define    VIDEO_FINISH     0x04
struct second_b_monitor
{
    UINT8     process_idx;
    UINT32    vbi_index;
    UINT32    de_scan_line;
    UINT32    decoding_head_idx;
    UINT32    mb_y;
    UINT32    mb_x;
};
#endif


#define VIDEO_ADPCM_ON     0x00
#define VIDEO_ADPCM_OFF    0x01
#define VIDEO_ADPCM_OPT_75 0x01
#define VIDEO_ADPCM_OPT_66 0x02
#define VIDEO_ADPCM_OPT_50 0x03

//#define VIDEO_FOR_SDRAM_PRJ
#ifdef VIDEO_FOR_SDRAM_PRJ
#define    IC_REV_0    0x01
#define    IC_REV_1    0x02
#define    IC_REV_2    0x03
#endif
//Position   (x,y)
struct position
{
    UINT16    u_x;    // Horizontal position .
    UINT16    u_y;    // Vertical position.
};

//rectangle size
struct rect_size
{
    UINT16    u_width;     // Horizontal size.
    UINT16    u_height;// Vertical size.
};

//Rectangle
struct rect
{
#if (defined(_MHEG5_SUPPORT_) || defined(_MHEG5_V20_ENABLE_) )
    INT16    u_start_x;    // Horizontal start point.
    INT16    u_start_y;    // Vertical start point.
#else
    UINT16    u_start_x;    // Horizontal start point.
    UINT16    u_start_y;    // Vertical start point.
#endif
    UINT16    u_width;    // Horizontal size.
    UINT16    u_height;    // Vertical size.
};

/********************************************************************
                            Play Mode & Speed

*********************************************************************/
struct play_mode
{
    UINT8    u_mode;
    INT16    n_speed;
};

//for uMode
#define    PLAYMODE_INVALID     0   //For error detection, should not send down
#define    PLAYMODE_FORWARD     1
#define    PLAYMODE_BACKWARD    2
#define    PLAYMODE_PAUSE       3


//for nSpeed-----RATE_2X ~ RATE_64X value should not changed!
#define RATE_1_2X     -2
#define RATE_1_4X     -4
#define RATE_1_8X     -8
#define RATE_1_16X    -16
#define RATE_1_32X    -32
#define RATE_1_64X    -64

#define RATE_DEFAULT  0 //if  not to change rate
#define RATE_1X       1

#define RATE_2X     2
#define RATE_4X     4
#define RATE_8X     8
#define RATE_16X    16
#define RATE_32X    32
#define RATE_64X    64
#define RATE_100X   100

/********************************************************************
                            audio properties
*********************************************************************/
#define MAX_AUDIO_CH_NUM  8 //maximum audio channel number

// Audio stream type
enum audio_stream_type
{
    AUDIO_INVALID,
    AUDIO_MPEG1,        // MPEG I
    AUDIO_MPEG2,        // MPEG II
    AUDIO_MPEG_AAC,
    AUDIO_AC3,            // AC-3
    AUDIO_DTS,        //DTS audio for DVD-Video
    AUDIO_PPCM,        //Packet PCM for DVD-Audio
    AUDIO_LPCM_V,        //Linear PCM audio for DVD-Video
    AUDIO_LPCM_A,        //Linear PCM audio for DVD-Audio
    AUDIO_PCM,            //PCM audio
    AUDIO_BYE1,        //BYE1 audio
    AUDIO_RA8,            //Real audio 8
    AUDIO_MP3,            //MP3 audio
    AUDIO_MPEG_ADTS_AAC,
    AUDIO_OGG,
    AUDIO_EC3,
    AUDIO_MP3_L3,
	AUDIO_MP3_2,
	AUDIO_MP2_2,
	AUDIO_PCM_RAW,//PCM DECODE PARAMS SET BY APP
	AUDIO_STREAM_TYPE_END,//for hld input param check
};    //reserved : A_DV,    A_SACD,    A_BYE1,     A_SVD,      A_RA,     A_HDCD


//audio bit rate, kbps
enum    audio_bit_rate
{
    A_BITRATE_INVALID,
    A_BITRATE_32,    //32k bps    ,
    A_BITRATE_40,
    A_BITRATE_48,
    A_BITRATE_56,
    A_BITRATE_64,
    A_BITRATE_80,
    A_BITRATE_96,
    A_BITRATE_112,    //bit rate 112 kbps
    A_BITRATE_128,
    A_BITRATE_160,
    A_BITRATE_192,
    A_BITRATE_224,
    A_BITRATE_256,
    A_BITRATE_320,
    A_BITRATE_384,
    A_BITRATE_416,
    A_BITRATE_448,
    A_BITRATE_512,
    A_BITRATE_576,
    A_BITRATE_640,
    A_BITRATE_768  //typical for DVD DTS

};

// Audio Sample Rate
enum audio_sample_rate
{
    AUDIO_SAMPLE_RATE_INVALID = 1,    // Invalid sample rate
    AUDIO_SAMPLE_RATE_8,        // 8 KHz
    AUDIO_SAMPLE_RATE_11,        // 11.025 KHz
    AUDIO_SAMPLE_RATE_12,        // 12 KHz
    AUDIO_SAMPLE_RATE_16,        // 16 KHz
    AUDIO_SAMPLE_RATE_22,        // 22.05 KHz
    AUDIO_SAMPLE_RATE_24,        // 24 KHz
    AUDIO_SAMPLE_RATE_32,          // 32 KHz
    AUDIO_SAMPLE_RATE_44,          // 44.1 KHz
    AUDIO_SAMPLE_RATE_48,        // 48 KHz
    AUDIO_SAMPLE_RATE_64,        // 64 KHz
    AUDIO_SAMPLE_RATE_88,        // 88.2 KHz
    AUDIO_SAMPLE_RATE_96,        // 96 KHz
    AUDIO_SAMPLE_RATE_128,        // 128 KHz
    AUDIO_SAMPLE_RATE_176,        // 176.4 KHz
    AUDIO_SAMPLE_RATE_192,        // 192 KHz
    AUDIO_SAMPLE_RATE_END,        //for hld input param check
};


// Audio Quantization
enum audio_quantization
{
    AUDIO_QWLEN_INVALID = 1,
    AUDIO_QWLEN_8,            // 8 Bits
    AUDIO_QWLEN_12,            // 12 Bits
    AUDIO_QWLEN_16,            // 16 Bits
    AUDIO_QWLEN_20,            // 20 Bits
    AUDIO_QWLEN_24,            // 24 Bits
    AUDIO_QWLEN_32,            // 32 Bits
    AUDIO_QWLEN_END,           //for hld input param check
};

//channel assignment
enum adec_channel
{
    ADEC_CHANNEL_NONE,    //not used
    ADEC_CHANNEL_L,        // left, including down-mixed L
    ADEC_CHANNEL_R,        // right, including down-mixed R
    ADEC_CHANNEL_LF,        // front left
    ADEC_CHANNEL_RF,        // front right
    ADEC_CHANNEL_C,        // center
    ADEC_CHANNEL_LFE,        // low frequence
    ADEC_CHANNEL_LS,        // left surround
    ADEC_CHANNEL_RS        // right surround
};


enum ac3bit_stream_mode
{
    AC3_BSMOD_COMPLETE_MAIN = 0,
    AC3_BSMOD_MUSIC_EFFECTS,
    AC3_BSMOD_VISUALLY_IMPAIRED,
    AC3_BSMOD_HEARING_IMPAIRED,
    AC3_BSMOD_DIALOGUE,
    AC3_BSMOD_COMMENTARY,
    AC3_BSMOD_EMERGENCY,
    AC3_BSMOD_VOICE_OVER,
    AC3_BSMOD_KARAOKE,
};

/********************************************************************
                            video & color properties

*********************************************************************/

// Video spec format (mpeg1/2/4),
enum vid_spec_fmt
{
    MPEG1_ES = 0,    // MPEG1 Video ES stream
    MPEG2_ES,     // MPEG2 Video ES stream
    MPEG4_ES     // MPEG4 Video stream
};



// TV system
enum tvsystem
{
    PAL        = 0    , //    PAL4.43(==PAL_BDGHI)        (Fh=15.625,fv=50)
    NTSC        , //    NTSC3.58                    (Fh=15.734,Fv=59.94)
    PAL_M        , //    PAL3.58                    (Fh=15.734,Fv=59.94)
    PAL_N        , //    PAL4.43(changed PAL mode)    (Fh=15.625,fv=50)
    PAL_60        , //                            (Fh=15.734,Fv=59.94)
    NTSC_443    , //    NTSC4.43                    (Fh=15.734,Fv=59.94)
    SECAM        ,
    MAC            ,
    LINE_720_25,    //added for s3601
    LINE_720_30,    //added for s3601
    LINE_1080_25,    //added for s3601
    LINE_1080_30,    //added for s3601

    LINE_1080_50,    //added for s3602f
    LINE_1080_60,   //added for s3602f
    LINE_1080_24,    //added for s3602f
    LINE_1152_ASS,  //added for s3602f
    LINE_1080_ASS,  //added for s3602f
    PAL_NC        , //    PAL3.58(changed PAL mode)    (Fh=15.625,fv=50)
    LINE_576P_50_VESA,
    LINE_720P_60_VESA,
    LINE_1080P_60_VESA,
    LINE_4096X2160_24,
    LINE_3840X2160_24,
    LINE_3840X2160_25,
    LINE_3840X2160_30,

    LINE_480_60,
    LINE_480_120,
    LINE_480_240,
    LINE_576_50,
    LINE_576_100,
    LINE_576_200,
    LINE_720_24,
    LINE_720_25_EXT,    //720p25
    LINE_720_30_EXT,    //720p30
    LINE_720_100,
    LINE_720_120,
    LINE_1080_100_I,
    LINE_1080_100,
    LINE_1080_120_I,
    LINE_1080_120,
    LINE_3840X2160_50,
    LINE_3840X2160_60,
    LINE_4096X2160_25,
    LINE_4096X2160_30,
    LINE_4096X2160_50,
    LINE_4096X2160_60,

    TV_SYS_INVALID = 0xFF,
};


// Frame rate
enum frame_rate
{
    FRAME_RATE_0 = 0,     //    0 f/s, forbidden in MPEG
    FRAME_RATE_23976, //    23.976 f/s
    FRAME_RATE_24 ,    //    24 f/s
    FRAME_RATE_25,    //    25 f/s
    FRAME_RATE_2997,    //    29.97 f/s
    FRAME_RATE_30,    //    30 f/s
    FRAME_RATE_50,    //    50 f/s
    FRAME_RATE_5994,    //    59.94 f/s
    FRAME_RATE_60,    //    60 f/s
    FRAME_RATE_15,    //    15 f/s
    FRAME_RATE_INVALID //    invalid frame rate value
};

//Aspect ratio
enum asp_ratio
{
    DAR_FORBIDDEN = 0, //aspect ratio forbidden
    SAR,                 //sample aspect ratio
    DAR_4_3,            //Display aspect ratio 3/4
    DAR_16_9,            //Display aspect ratio 9/16
    DAR_221_1            //Display aspect ratio 1/2.11
}; //P126, value must NOT change!


//highlight information
struct hlinfo
{
    UINT32    u_color;    // Selection color or action color code definition:reference DVD-VIDEO P VI4-122
    UINT32    u_start_time;    // start pts of hight light.
    struct rect    rect;    // position and size of this hightlight area.
};


//TVMode
enum tvmode
{
    TV_4_3 = 0,
    TV_16_9,
    TV_AUTO    //060517 yuchun for GMI Aspect Auto
};

//DisplayMode
/*
enum DisplayMode
{
    PANSCAN = 0,
    PANSCAN_NOLINEAR,    //non-linear pan&scan
    LETTERBOX
    //BOTH
};
*/

//DisplayMode
enum display_mode
{
    PANSCAN = 0,        // default panscan is 16:9 source on 4:3 TV.
    PANSCAN_NOLINEAR,    //non-linear pan&scan
    LETTERBOX,
    TWOSPEED, //add by t2
    PILLBOX,
    VERTICALCUT,
    NORMAL_SCALE,
    LETTERBOX149,
    AFDZOOM,
    PANSCAN43ON169,        // 4:3 source panscan on 16:9 TV.
    COMBINED_SCALE,
    DONT_CARE,
    VERTICALCUT_149,
    //BOTH
};

/*
enum RGBSubType
{
    RGB_555,    // Each component is 5 bits
    RGB_888        // Each component is 8 bits
};
*/

// picture format
enum pic_fmt
{
    // YCbCr Format
    YCBCR_411,
    YCBCR_420,
    YCBCR_422,
    YCBCR_444,
    //RGB format
    RGB_MODE1,        //rgb (16-235)
	RGB_MODE2,		//rgb (0-255)
	Y_ONLY,
	C_ONLY,
	HW_CFG,
	RGB888,
	ARGB8888,
	RGB4444,
	ARGB4444,
	RGB555,
	ARGB1555,
	RGB565,			
	CLUT2,
	CLUT4,
	CLUT8,
	ACLUT88, 
	TILE_MEM,
	YCbCr888,
	AYCbCr8888,
	YCbCr4444,
	AYCbCr4444,
	YCbCr555,
	AYCbCr1555,
	YCbCr565,	
	PACKET_422
};

//YUV color
struct  ycb_cr_color
{
    UINT8     u_y;
    UINT8    u_cb;
    UINT8    u_cr;
};

//Y, Cb, Cr address
struct ycb_cr_addr
{
    UINT8 *p_y_addr; //address of the  Y valure array of a picture
    UINT8 *p_cb_addr;
    UINT8 *p_cr_addr;
};

// Picture types for MPEG
enum   pic_type
{
    UNKNOWN_PIC = 0,
    I_PIC =    1,
    P_PIC =    2,
    B_PIC =    3,
       D_PIC =    4
};

enum pic_mem_map_mode
{
    MEMMAP_16X16,
    MEMMAP_32X16,
};

enum video_color_primaries
{
    CPRIM_FORBIDDEN = 0,
    CPRIM_BT709     = 1,
    CPRIM_UNKWOWN   = 2,
    CPRIM_RESEVERD3 = 3,
    CPRIM_BT470M    = 4,
    CPRIM_BT470BG   = 5,
    CPRIM_SMPTE170M = 6,
    CPRIM_SMPTE240M = 7,
    CPRIM_GENERIC_FILM = 8,
};

enum video_transfer_character    // Video Transfer Characteristics
{
    TRANSC_FORBIDDEN = 0,
    TRANSC_BT709     = 1,
    TRANSC_UNKWOWN   = 2,
    TRANSC_RESEVERD3 = 3,
    TRANSC_BT470M    = 4,
    TRANSC_BT470BG   = 5,
    TRANSC_SMPTE170M = 6,
    TRANSC_SMPTE240M = 7,
    TRANSC_LINEAR    = 8,
    TRANSC_LOG_100   = 9,
    TRANSC_LOG_316   = 10,
};

enum video_matrix_coeff       // Video Matrix Coefficients
{
    MCOEF_FORBIDDEN = 0,
    MCOEF_BT709     = 1,
    MCOEF_UNKWOWN   = 2,
    MCOEF_RESEVERD3 = 3,
    MCOEF_FCC       = 4,
    MCOEF_BT470BG   = 5,
    MCOEF_SMPTE170M = 6,
    MCOEF_SMPTE240M = 7,
};

enum VP_SOURCE_TYPE
{
    VP_SRC_VIDEO = 0,       // For displaying video pictures from TS, etc.
    VP_SRC_JPEG  = 1,       // For displaying computer pictures, such as JPEG, BMP, etc.
};

enum VIDEO_DECODER_TYPE
{
    VDEC_TYPE_MPEG2     = 0,
    VDEC_TYPE_H264      = 1,
    VDEC_TYPE_VC1       = 2,
    VDEC_TYPE_RMV       = 3,
};
/******************added by rachel ****************************/

enum still_mode
{
    STILLMODE_AUTO = 0,
    STILLMODE_FRAME = 1,
    STILLMODE_FIELD = 2
};



struct vpo_ycb_cr_address
{
    UINT32        u_yaddress;
    UINT32        u_cb_address;
    UINT32        u_cr_address;
};


/******************end by rachel ****************************/

// To retrieve struct DisplayInfo.fs, fs is UINT32 *
#define VDEC_FRMINFO_RANGERED_MODE  0   // VC1 range reduction mode: 0 - in entry point, 1 - in pic header
#define VDEC_FRMINFO_SCALE_Y        1
#define VDEC_FRMINFO_SCALE_UV       2



/********************************************************************
            time information of  MPEG 1/2

*********************************************************************/
//system clock reference
struct  SCR_T
{
    UINT8     u_scr0;        // lsb
    UINT32     u_scr;        // msb 32 bits
    UINT16     u_scr_ext;    // 9 bits in MPEG2 (MPEG1 doesn't have this field)
};

//time stamp, for PTS/DTS
struct PTS_T
{
    UINT8     u_ts0;    //lsb
    UINT32     u_ts;    //high 32 bits of time stamp
};


/**************************************************************
    start code name in MPEG 1/2, NOT stream ID!

***************************************************************/
//system start codes
//#define ISO_11172_END_CODE  0x000001B9
#define PACK_START_CODE  0x000001BA
#define SYSTEM_HEADER_START_CODE  0x000001BB

//packet start codes (including those used in PS and TS)
#define PROGRAM_STREAM_MAP     0x000001BC
#define PRIVATE_STREAM_1  0x000001BD
#define PADDING_STREAM        0x000001BE
#define PRIVATE_STREAM_2  0x000001BF

#define AUDIO_STREAM_0         0x000001C0
//...                                        //successive in ascending order
#define AUDIO_STREAM_31     0x000001DF

#define VIDEO_STREAM_0         0x000001E0
//...                                        //successive in ascending order
#define VIDEO_STREAM_15     0x000001EF

#define RESERVED_STREAM_0         0x000001F0
//...                                        //successive in ascending order
#define RESERVED_STREAM_15     0x000001FF


#define V_SEQUENCE_START     0x000001b3
#define V_SEQUENCE_END        0x000001b7
#define PICTURE_START        0x00000100
#define GOP_START            0x000001b8

#define MPEG_AUDIO_SYNCWORD 0xfff

/**************************************************************
    stream ID of MPEG 1/2

***************************************************************/
//system level
#define      ISO_11172_END_CODE     0xB9
#define     PACK_HEAD_CODE        0xBA
#define    SYSTEM_HEAD_CODE      0xBB

#define     STREAM_ID_PROGRAM_MAP     0xBC
#define     STREAM_ID_PRIVATE_1          0xBD
#define     STREAM_ID_PADDING        0xBE
#define     STREAM_ID_PRIVATE_2          0xBF


#define     STREAM_ID_AUDIO_0         0xC0
#define     STREAM_ID_AUDIO_1         0xC1
//...                                        //successive in ascending order
#define     STREAM_ID_AUDIO_31     0xDF

#define     STREAM_ID_VIDEO_0         0xE0
#define     STREAM_ID_VIDEO_1         0xE1    //SVCD still picture
#define     STREAM_ID_VIDEO_2         0xE2    //SVCD high-definition still picture
//...                                        //successive in ascending order
#define     STREAM_ID_VIDEO_15         0xEF

#define     STREAM_ID_RESERVEDM_0         0xF0
//...                                        //successive in ascending order
#define     STREAM_ID_RESERVEDM_15     0xFF
//reserved stream used
#define   STREAM_ID_ECM                0XF0
#define     STREAM_ID_EMM                0XF1
#define   STREAM_ID_DSM_CC            0XF2



/**************************************************************
    sub-stream ID of MPEG 1/2

***************************************************************/
//sub_stream_id for private stream 2, only NV_VR support(not parse now)
#define     SUB_STREAM_ID_RDI    0x50

//NV-video
#define     SUB_STREAM_ID_SUB_PICTURE_0         0x20   //NV_VR also support
#define     SUB_STREAM_ID_SUB_PICTURE_31     0x3f

#define     SUB_STREAM_ID_AC3_0          0x80 //NV_VR also support
#define     SUB_STREAM_ID_AC3_1          0x81//NV_VR also support
#define     SUB_STREAM_ID_AC3_7          0x87

#define     SUB_STREAM_ID_DTS_0          0x88
#define     SUB_STREAM_ID_DTS_7          0x8f

#define     SUB_STREAM_ID_SDDS_0      0x90
#define     SUB_STREAM_ID_SDDS_7      0x97

#define     SUB_STREAM_ID_LPCM_0      0xa0//NV_VR also support
#define     SUB_STREAM_ID_LPCM_1      0xa1//NV_VR also support
#define     SUB_STREAM_ID_LPCM_7      0xa7

#define     INVALID_MPEG_STREAM_ID      0
/**************************************************************
    DVD/HDD/VCD PE   stream ID

***************************************************************/

//default A/V settings of PE
//#define      PE_INVALID_STREAM_ID        INVALID_MPEG_STREAM_ID    //when there is no audio/sub-picture stream exists
                                        //if A/SP stream not exist, don't use API to give settings

//#define      PE_DEFALT_ID_VIDEO          STREAM_ID_VIDEO_0

#define     DVD_SECTOR_SIZE             2048

//Rachel From M3357 DRV_Common.h
enum deinterlacing_alg
{
    NORMAL_WEAVE = 0,
    HALF_PHASE_WEAVE,
    NORMAL_BOB,
    SINGLE_FIELD_BOB,
    NORMAL_MAF,
    SINGLE_FIELD_MAF,
	NORMAL_BOB_CONV,
	DI_WEAVE,
	PURE_BOB,
	EPEC_PMF,
	EPEC_BOB,
	EPEC_MAF,
	EPEC_PMF_BOB,
	EPEC_PMF_WEAVE	
};

enum dit_alg_single_option
{
    DIT_SINGLE_TOP = 0,
    DIT_SINGLE_BOT
};
/*
enum AspRatio
{
    DAR_FORBIDDEN,
    DAR_4_3,
    DAR_16_9,
};
*/
enum repeat_field
{
    repeat_top_field,
    repeat_bottom_field,
    repeat_none,

    repeat_top,
    repeat_bot,
    repeat_top_bot,
    repeat_bot_top,
    repeat_top_bot_top,
    repeat_bot_top_bot,
};

enum output_frame_ret_code
{
    ret_output_success,
//    Ret_Decision_Not_Ready,    //VPO Decision process not finish
//    Ret_Time_Not_Reach,        //decided by A/V sync, the display time is not reached
    ret_no_decoded_frame    //there is no decoded frame in display queue
};

struct pan_scan_info
{
    UINT32 u_number_of_offset;
    short horizontal_offsets[3];
};

struct video_info
{
    int width;        //picture size in horizontal
    int height;        //picture size in vertical
    int frame_rate;    //video source frame rate
    //enum AspRatio eAspectRatio; //aspect ratio for display
    UINT32 e_aspect_ratio;
    UINT8  active_format;
    int sar_width; //Added for H264, this information is from decoder
    int sar_height; //Added for H264, this information is from decoder
    enum tvsystem src_tv_sys; //Added for H264, this information is from decoder
    UINT32 dv_h_precision;
    UINT32 dv_v_precision;
};

struct time_code_t
{
    UINT8 hour;
    UINT8 minute;
    UINT8 second;
    UINT8 frame;
};

enum top_to_bottom_phase_diff
{
    vertical_phase_diff_1_2=0,
    vertical_phase_diff_1_4,
    vertical_phase_diff_1_8,
    vertical_phase_diff_1_16,
    vertical_phase_diff_1_32
};

enum field_sourcce
{
    top_field_source_only=0,
    bottom_field_source_only,
    both_fields_source
};

enum scaler_source
{
    scaler_field_based=0,
    scaler_frame_based
};

enum deinterlace_effort
{
   dit_effort_very_low = 0,
   dit_effort_low,
   dit_effort_medium,
   dit_effort_high
};


typedef enum _MAPMPING_MODE
{
    MAPPING_MODE_MPEG2,
    MAPPING_MODE_H264,
    MAPPING_MODE_RASTER,
    MAPPING_MODE_H265_INTERLACE,
    MAPPING_MODE_H265_PROGRESSIVE,
    MAPPING_MODE_SEMI_420,
    MAPPING_MODE_SEMI_422,
    MAPPING_MODE_RGB,
    MAPPING_MODE_ARGB
}t_e_mapping_mode;

#define    VIDEO_OPEN_WIN_FLAG         0x01
#define    VIDEO_RESET_SRC_DST_FLAG    0x02
#define VIDEO_INTO_SML_MODE         0x04
#define VIDEO_LEAVE_SML_MODE        0x08
#define    VIDEO_FIRST_PIC_SYNC_FLAG   0x10
#define VIDEO_OPEN_PIP_WIN_FLAG     0X80

struct display_info
{
//#ifdef H264_SUPPORT
    struct frame_store *fs;
//#endif
    BOOL b_new_video_info;    //the flag indicates the structure pVideoInfo has changed or not

    BOOL b_progressive_source;// Indicating the source is progressive or not
    BOOL b_top_field_first;    // VPO should only check this flag when bProgressiveSource is FALSE;
    enum repeat_field e_repeat_field;   // Only check it when frame rate is 24 FPS

    UINT8 top_index;    //frame buffer index containing top field
    UINT8 *top_y;    //the address of frame buffer, which contains top field luminance
    UINT8 *top_c;    //the address of frame buffer, which contains top field chrominance
    UINT8 bottom_index;    //frame buffer index containing bottom field
    UINT8 *bottom_y;    //the address of frame buffer, which contains bottom field luminance
    UINT8 *bottom_c;    //the address of frame buffer, which contains bottom field chrominance
    UINT8 *pb_mafbuffer; //the address of flag buffer, which contains motion adaptive deinterlacing flags.
    BOOL bh264_mapping;
    int stride;        //marco block numbers in a marco block line

    struct video_info *p_video_info;    //a pointer to structure stored video information
    struct pan_scan_info *p_pan_scan_info; //
    BOOL zoomflag;    //the flag indicates the accomplishment of zoom coefficients calculations

    BOOL    b_dit_alg_valid;
    enum deinterlacing_alg e_deinterlacing_alg; //selection of deinterlacing algorithm
    enum dit_alg_single_option e_dit_single_opt;
    BOOL    bbase_single_field;
    BOOL    bbase_single_top;

    UINT8    src_output_mode;
    BOOL    src_adpcm_onoff;
    UINT8    src_adpcm_ratio;
    BOOL    src_top_fieldstructure;
    BOOL    src_top_sml;
    UINT8    src_top_sml_height;
    BOOL    src_bot_fieldstructure;
    BOOL    src_bot_sml;
    UINT8    src_bot_sml_height;
    BOOL    bshow_one_field_time;
    struct rect        src_rect;
    struct rect        dst_rect;
    UINT8     init_phase;

    UINT8    bvp_act_flag;
    BOOL    bhalf_mode;//addedf for s3601

    //vpo->vdec
    UINT8    u_cur_top_bot;
    //vdec->vpo accordint to the uCurTopBot and size changed
    BOOL    bwaitnextfield;

    UINT8   type;       // enum PicType
    UINT32  display_idx;
    UINT32  head_idx;
    UINT32 *pvld_err;

    UINT8 colour_primaries;    // 1:BT709, 0:BT601, see enum VideoColorPrimaries
    BOOL    b_release_in_lm;//if true,release frame in linemeet,else release frame in vblanking.
    BOOL     b_h264decoder;//temporarily use
    BOOL   b_dview;


    enum top_to_bottom_phase_diff   e_top_to_bottom_phase_diff_luma;
    enum top_to_bottom_phase_diff   e_top_to_bottom_phase_diff_chroma;
    enum field_sourcce   e_field_source;
    enum scaler_source   e_scaler_source;
    enum deinterlace_effort   e_deinterlace_effort;

    BOOL     b_maf_enable;
    UINT8   transfer_characteristics;   // enum VideoTransferCharacter
    UINT8    matrix_cofficients;         // enum VideoMatrixCoeff
    UINT8   sample_format;              // enum PicFmt
    UINT8   source_type;                // enum VP_SOURCE_TYPE
    t_e_mapping_mode e_mapping_mode;

    UINT32 vc1_mixer_control;
    BOOL b_contrast_enable;
    UINT32 b_contrast_buf_start;
    UINT32 b_contrast_buf_size;
    UINT32 b_contrast_pixel_num;
    UINT32 b_contrast_avg;

    BOOL   smooth_switch;
    BOOL    bEnableVC1;
    UINT8   VC1Mode;
    UINT8   VC1RangeMap_Y;
    UINT8   VC1RangeMap_UV;
    UINT32     mbdi_addr;
    UINT32     extra_info_addr;
	
	struct time_code_t time_code;
    UINT32  sample_bits;
    BOOL  b_preview;
    UINT32  sample_bits_c;
    BOOL fbc_enable;
    UINT32  fbc_hdr_addr_top;
    UINT32  fbc_hdr_addr_bot;
    UINT32  sample_order;
};

//wl add
struct pip_dislay_info
{

    BOOL b_new_video_info;    //the flag indicates the structure pVideoInfo has changed or not

    BOOL b_progressive_source;// Indicating the source is progressive or not
    BOOL b_top_field_first;    // VPO should only check this flag when bProgressiveSource is FALSE;
    enum repeat_field e_repeat_field;   // Only check it when frame rate is 24 FPS


    UINT8 buf_index;
    UINT8 *pu_yaddr;
    UINT8 *pu_caddr;

    int stride;        //marco block numbers in a marco block line

    struct video_info *p_video_info;    //a pointer to structure stored video information
    enum deinterlacing_alg e_deinterlacing_alg; //selection of deinterlacing algorithm
    BOOL b_single_field_is_top_field;

    // wl add
    UINT16 u_xsta_pos;
    UINT16 u_ysta_pos;
// ~wl add
    BOOL    b_first_pip;

////only for  VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
    BOOL    bpip_layer_on;
    UINT32    pip_y_addr;
    UINT32    pip_c_addr;
    struct rect    pip_rect;
    UINT16    pip_buffer_stride;
//#endif
};
//End Rachel

/***Add some definition for STB solution***/
#define     VOB_START_MASK              0x01    // used with control_block.vob_start
#define     VOB_END_MASK                0x02    // used with control_block.vob_start
typedef struct control_block
{
       UINT8 stc_id_valid; // 1: valid, 0: invalid
       UINT8 pts_valid; // 1:valid, 0:invalid
       UINT8 data_continue; // 1:not continue, 0: continue
       UINT8 ctrlblk_valid; // 1:valid, 0: invalid
       UINT8 instant_update; //provided by dec, 1:need update instantly, 0: NOT
       UINT8 vob_start;
       UINT8 pts_start; // 1:Identify one new pts reach
	UINT8 reserve; //
    UINT8 stc_id;
    UINT32 pts;
	UINT8 bstream_run_back;
	UINT8 stc_offset_idx;
}st_control_block;
typedef enum output_frame_ret_code(* t_mprequest)(struct display_info *);
typedef BOOL (* t_mprelease)(UINT8, UINT8);
typedef enum output_frame_ret_code(* t_piprequest)(struct pip_dislay_info *);
typedef BOOL (* t_piprelease)(UINT8);

typedef enum output_frame_ret_code(* t_request_ext)(void *,void *);
typedef BOOL (* t_release_ext)(void*,UINT8, UINT8);
//added for s3601
struct release_info
{
    UINT8    src_path;
    UINT8     src_frm_type_index;
    UINT8    src_frm_index;
    struct frame_store *fs;
};

struct request_info
{
    UINT8    src_path;
    UINT8    case_index;
    UINT8    drop_frame_count;   // if drop_frame_count > 0, vdec should drops some frame follows the current frame
    UINT8    reserved;           // reserved for the future
    //60frames/s to 50frames/s(or 50 fields/s) case,need to adjust frame rate convert in vdec module
    BOOL     b_ve_adjust_frame_rate;
	UINT32      display_dly;
};

struct vblanking_info
{
    UINT8    src_path;
    UINT8     src_frm_type_index;
    UINT8    src_frm_index;
};

typedef enum output_frame_ret_code(* t_request)(void *,struct display_info *,struct request_info *);
typedef RET_CODE (* t_release)(void *, struct release_info *);
typedef RET_CODE (* t_vblanking)(void *, struct vblanking_info *);
typedef RET_CODE (* t_geresize)(UINT32);
//add-end
struct mpsource_call_back
{
    struct vdec_device *handler;
    t_mprequest    request_callback;
    t_mprelease    release_callback;
    t_vblanking vblanking_callback;
};

struct pipsource_call_back
{
    t_request_ext    request_callback;
    t_release_ext    release_callback;
    t_vblanking vblanking_callback;
    struct vdec_device *handler;
};

struct source_call_back_pip
{
    t_request_ext    request_callback;
    t_release_ext    release_callback;
    t_vblanking vblanking_callback;
    struct vdec_device *handler;
};
struct source_call_back_ext
{
    struct vdec_device *handler;
    t_request_ext    request_callback;
    t_release_ext    release_callback;
    t_vblanking vblanking_callback;
};
struct source_callback
{
    struct vdec_device *handler;
    t_request request_callback;
    t_release release_callback;
    t_vblanking vblanking_callback;
};

//end
//Steve add for connection between VBI & VPO
typedef void (* t_vbirequest)(UINT8 field_polar);
//end

#define TTX_VBI    0
#define TTX_OSD    1
//added for s3601. information between DE and HDMI
struct s3601to_hdmi_video_infor
{
    enum tvsystem        tv_mode;
    UINT16            width;
    UINT16            height;
    enum pic_fmt        format;
    BOOL            scan_mode;
    enum tvmode    output_aspect_ratio;

};

enum mheg5scene_ar
{
    /* Set display aspect ratio to 16:9 - wide screen */
    scene_aspect_ratio_16x9,

    /* Set display aspect ratio to 4:3 - normal TV mode  with CCO*/
    scene_aspect_ratio_4x3_cco,

    /* Set display aspect ratio to 4:3 - normal TV mode  with LB*/
    scene_aspect_ratio_4x3_lb,

    /* Scene aspect ratio is not specified */
    SCENE_ASPECT_RATIO_UNSEPCIFIED
};

#ifdef __cplusplus
}
#endif


#endif





