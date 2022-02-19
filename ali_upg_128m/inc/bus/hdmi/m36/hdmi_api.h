#ifndef __HDMI_API_H__
#define __HDMI_API_H__

#include <mediatypes.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include <basic_types.h>
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _HDMI_IN_E_TREE_
//#define _HDMI_IN_E_TREE_
#endif

#ifndef _HDCP22_
//#define _HDCP22_
#endif

/*******************************************************************
*                   The API interface of HDMI                        *
*******************************************************************/
struct ifm_pkt_header
{
	UINT8 info_fram_type;
	UINT8 info_fram_version;
	UINT8 info_fram_length;
};

struct ifm_packet
{
	struct ifm_pkt_header header;
	UINT8 *payload;
};
//******************* HDMI Resolution definition ************************
enum HDMI_API_RES
{
	HDMI_RES_INVALID = 0,
	HDMI_RES_480I,
	HDMI_RES_480P,
	HDMI_RES_576I ,
	HDMI_RES_576P,
	HDMI_RES_720P_50,
	HDMI_RES_720P_60,
	HDMI_RES_1080I_25,
	HDMI_RES_1080I_30,
	HDMI_RES_1080P_24,
	HDMI_RES_1080P_25,
	HDMI_RES_1080P_30,
	HDMI_RES_1080P_50,
	HDMI_RES_1080P_60,
	HDMI_RES_4096X2160_24,
	HDMI_RES_3840X2160_24,
	HDMI_RES_3840X2160_25,
	HDMI_RES_3840X2160_30,

    /* --- extend for 861-F --- */
	HDMI_RES_480I_60,
	HDMI_RES_480P_120,
	HDMI_RES_480I_120,
	HDMI_RES_480P_240,
	HDMI_RES_576I_50,
	HDMI_RES_576P_100,
	HDMI_RES_576I_100,
	HDMI_RES_576P_200,
	HDMI_RES_720P_24,
	HDMI_RES_720P_25,
	HDMI_RES_720P_30,
	HDMI_RES_720P_100,
	HDMI_RES_720P_120,
	HDMI_RES_1080I_50,
	HDMI_RES_1080I_60,	
	HDMI_RES_1080P_100,
	HDMI_RES_1080P_120,	
	HDMI_RES_3840X2160_50,
	HDMI_RES_3840X2160_60,
	HDMI_RES_4096X2160_25,
	HDMI_RES_4096X2160_30,
	HDMI_RES_4096X2160_50,
	HDMI_RES_4096X2160_60,	
};
//******************The control state of HDMI task***********************
enum HDMI_CTRL_STATE
{
    HDMI_CTRL_STATE_IDLE = 0,		//hotplug has been detached
    HDMI_CTRL_STATE_READY,			//hotplug has been attached
    HDMI_CTRL_STATE_PLAY,			//hdmi has transmitted data
    HDMI_CTRL_STATE_AV_CHANGED		//hdmi A/V has been changed
};
//****************** HDMI deep color definition ***********************
enum HDMI_API_DEEPCOLOR
{
	HDMI_DEEPCOLOR_24 = 0,
	HDMI_DEEPCOLOR_30,
	HDMI_DEEPCOLOR_36,
	HDMI_DEEPCOLOR_48,
	HDMI_DEEPCOLOR_NOT_INDICATED = 0xFF,
};
//****************** HDMI color space definition ***********************
enum HDMI_API_COLOR_SPACE
{
	HDMI_RGB = 0,
	HDMI_YCBCR_422,
	HDMI_YCBCR_444,
	/* extend for 861-F */
	HDMI_YCBCR_420,
	
};
//*************** The Structure of audio information in EDID *******************
enum EDID_AUD_FMT_CODE
{
	EDID_AUDIO_LPCM 				= 0x0001,
	EDID_AUDIO_AC3					= 0x0002,
	EDID_AUDIO_MPEG1				= 0x0004,
	EDID_AUDIO_MP3					= 0x0008,
	EDID_AUDIO_MPEG2				= 0x0010,
	EDID_AUDIO_AAC					= 0x0020,
	EDID_AUDIO_DTS					= 0x0040,
	EDID_AUDIO_ATRAC				= 0x0080,
	EDID_AUDIO_ONEBITAUDIO			= 0x0100,
	EDID_AUDIO_DD_PLUS				= 0x0200,
	EDID_AUDIO_DTS_HD				= 0x0400,
	EDID_AUDIO_MAT_MLP				= 0x0800,
	EDID_AUDIO_DST					= 0x1000,
	EDID_AUDIO_BYE1PRO				= 0x2000,
};
//*************** The Structure of deep color information in EDID *******************
enum EDID_DEEPCOLOR_CODE
{
    EDID_DEEPCOLOR_24 				= 0x01,      //!< DC24 (mandatory)
    /* H14b VSDB */
	EDID_DEEPCOLOR_30				= 0x02,      //!< DC30 (RGB)
	EDID_DEEPCOLOR_36				= 0x04,      //!< DC36 (RGB)
	EDID_DEEPCOLOR_48				= 0x08,      //!< DC48(RGB)
	EDID_DEEPCOLOR_Y444				= 0x10,      //!< above DC_30/DC_36/DC_48 flags apply to YCbCr 444
    /* HF-VSDB */
	EDID_DEEPCOLOR_30_420			= 0x20,      //!< DC30 (YCbCr 420)
	EDID_DEEPCOLOR_36_420			= 0x40,      //!< DC30 (YCbCr 420)
	EDID_DEEPCOLOR_48_420			= 0x80,      //!< DC30 (YCbCr 420)
};

enum HDMI_AV_CHG_STE
{
	HDMI_CB_NOTHING      = 0x00,
	HDMI_CB_CLK_RDY2CHG  = 0x01,
	HDMI_CB_AV_INFO_CHG  = 0x02,
	HDMN_CB_RESERVED_03  = 0x03,
	HDMI_CB_CLK_CHG_DONE = 0x04,
	HDMN_CB_RESERVED_05  = 0x05,
	HDMN_CB_RESERVED_06  = 0x06,
	HDMN_CB_RESERVED_07  = 0x07,
	HDMI_CB_CLR_RDY2CHG  = 0x08,
};

//*************** The definition for 3D in H14b VSIF *******************
enum _3D_STRUCTURE
{
    VSI_FRAME_PACKING	    = 0,
    VSI_FIELD_ALTERNATIVE   = 1,
    VSI_LINE_ALTERNATIVE    = 2,
    VSI_SIDE_BY_SIDE_FULL   = 3,
    VSI_L_DEPTH             = 4,
    VSI_L_DEPTH_GRAPHICS    = 5,
    VSI_TOP_AND_BOTTOM      = 6,
    VSI_SIDE_BY_SIDE_HALF   = 8,
};

enum EXT_VIDEO_FORMAT
{
    NO_ADDITIONAL_VIDEO_FORMAT_PRESENTED	= 0,
    EXT_RESOLUTION_FORMAT_PRESENTED         = 1,
    _3D_FORMAT_INDICATION_PRESENTED         = 2,
};

enum EXT_DATA_FIELD
{
	HORIZONTAL_SUB_SAMPLING	= 0,
	ODD_LEFT_ODD_RIGHT		= 4,
	ODD_LEFT_EVEN_RIGHT		= 5,
	EVEN_LEFT_ODD_RIGHT		= 6,
	EVEN_LEFT_EVEN_RIGHT	= 7,
};

#if 0
//*********************The Structure of Video Interface ***********************
struct de2hdmi_video_infor
{
	enum tvsystem			tv_mode;
	UINT16					width;
	UINT16					height;
	enum pic_fmt			format;
	BOOL					scan_mode;
	enum tvmode				output_aspect_ratio;
	enum HDMI_AV_CHG_STE	av_chg_ste;
};
#else
//*********************The Structure of Video Interface ***********************
/* --- extend for 861-F --- */
// 3D_AdditionalInfo from Table1 10-4 of HDMI spec. 2.0b
typedef struct
{
    unsigned char    _3D_Preferred2DView  :2;  //<! [1:0]    No/right_for_2D/left_for_2D/Dont_care
    unsigned char    _3D_ViewDependency   :2;  //<! [3:2]    No/right_independent/left_independent/both_independent
    unsigned char    _3D_DualView         :1;  //<! [4]      normal/DualView 3D
    unsigned char    Reserved             :3;  //<! [7:5]    Reserved
}_3D_ADDITIONAL_INFO;
// 3D_DisparityData from Table1 10-4 of HDMI spec. 2.0b
typedef struct
{
    unsigned char   length   :5;               //<! [4:0]  3D_DisparityData_length
    unsigned char   version  :3;               //<! [7:5]  3D_DisparityData_version
    unsigned char   DisparityData[18];         //<! 3D_DisplayData_1 ~ 3D_DisplayData_J
}_3D_DISPARITY_DATA;
// 3D_MeataData from Table1 10-4 of HDMI spec. 2.0b
typedef struct
{
    unsigned char   length   :5;               //<! [4:0]  3D_MetaaData_length
    unsigned char   type     :3;               //<! [7:5]  3D_MetaData_type
    unsigned char   MetaData[8];               //<! 3D_DisplayData_1 ~ 3D_DisplayData_J
}_3D_META_DATA;
// Dynamic Range and Msstering from Table1 2 of CEA-861.3
typedef struct
{
    unsigned char   EOTF                     :3;  //<! [2:0]  EOTF
    unsigned char   Reserved1                :5;  //<! [7:3]  Reserved
    unsigned char   MetatData_Descriptor_ID  :3;  //<! [2:0]  Sattic MeataData Descriptor ID
    unsigned char   Reserved2                :5;  //<! [7:3]  Reserved
    //unsigned char   MetatData_Descriptor[26];     //<! Static Meatadata Descriptor
    unsigned short display_primaries[3][2];
    unsigned short white_point[2];
    unsigned short max_mastering_luminance;
    unsigned short min_mastering_luminance;
    unsigned short max_content_light_level;
    unsigned short max_pic_average_light_level;
}DRM_DATA;

#if defined(_HDMI_IN_E_TREE_)
struct de2Hdmi_video_infor
#else
struct de2hdmi_video_infor
#endif
{
#if defined(_HDMI_IN_E_TREE_)
	enum TVSystem			tv_mode;
#else
	enum tvsystem			tv_mode;
#endif
	UINT16					width;
	UINT16					height;
#if defined(_HDMI_IN_E_TREE_)
	enum PicFmt				format;
#else
	enum pic_fmt			format;
#endif
	BOOL					scan_mode;
	BOOL					afd_present;
#if defined(_HDMI_IN_E_TREE_)
	enum TVMode				output_aspect_ratio;
#else
	enum tvmode				output_aspect_ratio;
#endif
	UINT8					active_format_aspect_ratio;
	enum HDMI_AV_CHG_STE	av_chg_ste;
    BOOL                    get_done;

    // add 3D
	enum 	EXT_VIDEO_FORMAT	ext_video_format;	//add for 3d function
	enum	_3D_STRUCTURE		_4K_VIC_3D_structure;
	enum	EXT_DATA_FIELD		ext_data;

    /* --- extend for 861-F --- */
    // 3D Video Extension, from sec. 7.4 of HDMI spec. 2.0b
	BOOL					    _3D_Additional_Info_present; //<! _3D_AdditionalInfo present?, 0: no, 1: present
	BOOL					    _3D_Disparity_Data_present;  //<! _3D_DisparityData present?, 0: no, 1: present
	BOOL					    _3D_Meta_present;            //<! _3D_MeataData present?, 0: no, 1: present
    
    _3D_ADDITIONAL_INFO         _3D_AdditionalInfo;
    _3D_DISPARITY_DATA          _3D_DisparityData;
    _3D_META_DATA               _3D_MetaData;
    // HDR, from sec. 8.7 of HDMI spec. 2.0b and CEA-861.3
	BOOL					    DRM_present;  //<! DRMData present?, 0: no DRMData, 1: DRMData present
	DRM_DATA                    DRMData; //<! mastering_display_colour_volume (SEI)

    // bit depth ( based on profile from H.264, HEVC, ...)
	BOOL					   depth_present;  //<! bit_depth present?, 0: no( HDMI use 8 for bit_depth), 1:present
	UINT8                      bit_depth;      //<! bit depth 8~16

    // colour description( from H.264, HEVC, ...)
	BOOL        colour_present;   //<! colour description present?, 0: no(use default colorimetry), 1:present
	UINT8       colour_primaries;           //<! colour primaries (VUI)  
	UINT8       transfer_characteristics;   //<! transfer_characteristics (VUI) 

    // alternative transfer characteristic ( from H.264, HEVC, ...)
	BOOL        alternative_transfer_present;  //<! present?, 0: no , 1: present
	UINT8       preferred_transfer_characteristics;      //<! preferred transfer_characteristics (SEI)
};
struct hdmi2de_edid_infor
{
    /* colorimetry block */
    BOOL            support_bt2020_rgb; //<! BT2020 RGB
    BOOL            support_bt2020_ycc; //<! BT2020 YCbCr
    BOOL            support_bt2020_ycc_c; //<! BT2020 YCbCr_C
    /* hdr block */
    BOOL            support_tradition_sdr;        
    BOOL            support_tradition_hdr;        
    BOOL            support_st2084;        
    BOOL            support_hlg;        
};
#endif
//*********************The Structure of Audio Interface ***********************
enum I2S_FMT_TYPE
{
	I2S_FMT_I2S	=0,
	I2S_FMT_LEFT,
	I2S_FMT_RIGHT
};

enum AUDIO_CODING_TYPE
{
	AUD_TYPE_PCM				= 0x01,//0X0,
	AUD_TYPE_AC3				= 0x02,//0X200,
	AUD_TYPE_MPEG1_L12			= 0x03,//0X101,
	AUD_TYPE_MPEG1_L3			= 0x04,//0X102,
	AUD_TYPE_MEPG2				= 0x05,//0X110,
	AUD_TYPE_AAC				= 0x06,//0X130,
	AUD_TYPE_DTS				= 0x07,
	AUD_TYPE_ATRAC				= 0x08,
	AUD_TYPE_ONEBITAUDIO		= 0x09,
	AUD_TYPE_DD_PLUS			= 0x0A,
	AUD_TYPE_DTS_HD				= 0x0B,
	AUD_TYPE_MAT_MLP			= 0x0C,
	AUD_TYPE_DST				= 0x0D,
	AUD_TYPE_BYE1PRO			= 0x0E,
};

/*Channel Position*/
#define CH_FL	0X0
#define CH_FC	0X1
#define CH_FR	0X2
#define CH_FLC	0X3
#define CH_FRC	0X4
#define CH_RL	0X5
#define CH_RC	0X6
#define CH_RR	0X7
#define CH_RLC	0X8
#define CH_RRC	0X9
#define CH_LFE	0Xa

#if defined(_HDMI_IN_E_TREE_)
struct snd2Hdmi_audio_infor
#else
struct snd2hdmi_audio_infor
#endif
{
    /* INFO by user setting */
    UINT32 user_def_ch_num;     /* 2 or 8                           */
    UINT32 pcm_out;                 /* 1: PCM 0: BIT STREAM 2: FORCE DD                 */

    /* AUDIO stream status */
    enum AUDIO_CODING_TYPE coding_type;
    UINT32 max_bit_rate;                /* maximum bit rate                     */
    UINT32 ch_count;                    /* 2, 6(5.1), 8                         */
    UINT32 sample_rate;             /* 48000, 44100, 32000 etc              */
    UINT32 level_shift;             /* level shift after down-mixing            */

    /* S/PDIF config dynamic setting */
    UINT32 spdif_edge_clk;          /*0: rising edge latch data, 1: falling edge latch data */

    /* I2S config dynamic setting */
    /*  31:30   Reserved
        29:28   Clock Accuracy
        27:24   Sample rate
        23:20   Channel Number
        19:16   Source Number
        15:8    L & Category
        7:6     Mode
        5:3     Emphasis
        2       Copyright
        1       Audio content flag
        0       Professional flag   */
    UINT32 ch_status;

    /* ch_position[i]
    bit0 ~ bit3: speaker(CH_FL, CH_FC etc)
    bit4 ~ bit6: channel position(0~7, the position in I2S dma buffer)
    bit7: speaker enable(1:enable, 0:disable)*/
    UINT8   ch_position[8];

    /* I2S config fixed setting */
    UINT32 bclk_lrck;                   /* 32, 64                                       */
    UINT32 word_length;             /* 16bits, 24bits                               */
    UINT32 i2s_edge_clk;                /* 0:rising edge latch data, 1:falling edge latch data  */
    enum I2S_FMT_TYPE i2s_format;   /* I2S, Left Justify and Right Justify              */
    UINT32 lrck_hi_left;                /* 1: lrck high for left channel, 0: reverse            */
    enum HDMI_AV_CHG_STE    av_chg_ste;
};

typedef enum
{
	CEA_AUD_TYPE,
	CEA_VIDEO_TYPE,
	CEA_VSD_TYPE,
	CEA_SPK_TYPE
}CEA_DB_TYPE;

typedef struct SHORT_AUDIO_DESCRIPTOR
{
	UINT16 audio_format_code;
	UINT8 max_num_audio_channels;
	UINT8 audio_sampling_rate;
	UINT16 max_audio_bit_rate;
	struct SHORT_AUDIO_DESCRIPTOR *next;
}short_audio_descriptor_t;

typedef struct SHORT_VIDEO_DESCRIPTOR
{
	UINT8 native_indicator;
	UINT8 video_id_code;
	struct SHORT_VIDEO_DESCRIPTOR * next;
    BOOL ycbcr420_only;
    BOOL ycbcr420_capabilty;
    BOOL ycbcr422_capabilty;
    BOOL ycbcr444_capabilty;
}short_video_descriptor_t;

typedef struct SHORT_CEA_DESC
{
	UINT8 cea_data;
	struct SHORT_CEA_DESC *next;
}short_cea_desc;

typedef struct
{
	UINT8 hdmi_3d_multi_present;
	UINT8 hdmi_vic_len;
	UINT8 hdmi_3d_len;
	short_cea_desc	*short_hdim_vic_desc;
	short_cea_desc	*short_3d_desc;
}HDMI_3D_DESCRIPTOR;

typedef struct STAND_TIMING_DESC
{
	UINT16 horiz_add_pixel;
	UINT8 field_fresh_rate : 6;
	UINT8 image_aspect_ratio :2;
}STAND_TIMING_DESC;

typedef enum
{
	AR_16_to_10,
	AR_4_to_3,
	AR_5_to_4,
	AR_16_to_9
}HDMI_ASPECT_RATIO;
#define STAND_TIMING_NUM	8
//*********************End - The Structure of Audio Interface ***********************

INT32 set_audio_info_to_hdmi(UINT32 param);
INT32 set_video_info_to_hdmi(UINT32 param);
UINT16 api_get_physical_address(void);
BOOL api_get_hdmi_cec_onoff(void);
void api_set_hdmi_cec_onoff(BOOL b_on_off);
INT32 api_set_logical_address(UINT8 logical_address);
UINT8 api_get_logical_address(void);
INT32 api_hdmi_cec_transmit(UINT8* message, UINT8 message_length);
INT32 api_get_edid_block_data(UINT8  block, UINT8 * data);
#if defined(_HDMI_IN_E_TREE_)
INT32 api_get_edid_video_format(enum PicFmt *format);
#else
INT32 api_get_edid_video_format(enum pic_fmt *format);
#endif
INT32 api_get_edid_video_resolution(enum HDMI_API_RES *res);
INT32 api_get_edid_all_video_resolution(UINT32 *native_res_index, enum HDMI_API_RES *video_res);
INT32 api_get_edid_audio_out(enum EDID_AUD_FMT_CODE *aud_fmt);
INT32 api_get_edid_all_audio_out(enum EDID_AUD_FMT_CODE *aud_fmt);
INT32 api_get_edid_manufacturer_name(UINT16 *m_name);
INT32 api_get_edid_product_id(UINT16 *p_id);
INT32 api_get_edid_serial_number(UINT32 *s_number);
INT32 api_get_edid_week_manufacturer(UINT8 *w_manufacture);
INT32 api_get_edid_year_manufacturer(UINT16 *y_manufacture);
INT32 api_get_edid_monitor_name(UINT8 *m_name);
INT32 api_get_edid_deep_color(enum EDID_DEEPCOLOR_CODE *dc_fmt);
void api_set_hdmi_audio_onoff(BOOL b_on_off);
BOOL api_get_hdmi_audio_onoff(void);
BOOL api_get_hdcp_result(void);
BOOL api_get_hdcp_repeater(void);
void api_hdmi_switch(BOOL b_on_off);
BOOL api_hdmi_get_onoff(void);
BOOL api_get_hdmi_hdcp_onoff(void);
void api_set_hdmi_hdcp_onoff(BOOL b_on_off);
void api_set_hdmi_only_support_hdcp14(BOOL is_enable);
void api_set_hdmi_only_support_hdcp22(BOOL is_enable);
INT32 api_get_hdmi_sink_hdcp_cap(UINT8 *hdcp_cap);
INT32 api_get_hdmi_sink_hdcp_version(void);
void api_set_hdmi_res(enum HDMI_API_RES res);
UINT32 api_get_hdmi_res(void);
UINT32 api_set_hdmi_deep_color(enum HDMI_API_DEEPCOLOR deep_color);
UINT32 api_get_hdmi_deep_color(void);
UINT32 api_set_hdmi_color_space(enum HDMI_API_COLOR_SPACE color);
UINT32 api_get_hdmi_color_space(void);
void api_set_hdmi_phy_clk_onoff(BOOL b_on_off);
UINT32 api_get_hdmi_state(void);
char *get_hdmi_sw_version(void);
INT32 api_hdmi_register(void);
UINT8 *get_hdmi_cm_mfc_code(void);
UINT8 *get_hdmi_cm_key_id(void);
UINT8 *get_hdmi_cm_status(void);
void hdmi_log_edid_start(void);
void hdmi_log_edid_stop(void);
void hdmi_log_debug_start(void);
void hdmi_log_debug_stop(void);
INT16 api_get_edid_audio_delay(void);
UINT32 api_set_format_depth_mode(BOOL en_auto);
BOOL api_get_format_depth_mode(void);
UINT32 api_set_edid_mode(BOOL en_user_edid);
BOOL api_get_edid_mode(void);
UINT32 api_set_lte_340_scramble_mode(BOOL en_lte_340_scramble);
BOOL api_get_lte_340_scramble_mode(void);
//20140801 KP begin
////////////////////////////////////////////////////
/////////////		DEFINITION		////////////////
////////////////////////////////////////////////////
typedef struct HDMI_AUD_SETTING
{
	UINT8 AUD_TYPE;
	UINT8 sample_freq;
	UINT8 sample_size;
	UINT8 ch_cnt;
	UINT8 ch_alloc;
	UINT8 LVS_val;
	UINT8 down_mix;
	UINT8 LFE_playback_level;
	UINT8 reserved;
}HDMI_AUD_SETTING;

#define MAX_NUM_AUDIO_INFO 9
typedef struct HDMI_AUDIO_INFO
{
	UINT8 audio_type; 
	UINT8 max_channels;
	UINT8 sample_frequency_mask;
	UINT8 info;
	UINT8 reserved;
}HDMI_AUDIO_INFO;

typedef struct HDMI_AUDIO_CAPABLE
{
	HDMI_AUDIO_INFO audio_info[MAX_NUM_AUDIO_INFO];
	UINT8 audio_type_count;
	UINT8 reserved;
	UINT8 speaker_allocation_mask;
}HDMI_AUDIO_CAPABLE;

enum HDMI_RPC_CB_CMD
{
	HDMI_RPC_CB_CMD_EDID_READY = 0,
	HDMI_RPC_CB_CMD_PLUG_IN = 1,
	HDMI_RPC_CB_CMD_PLUG_OUT = 2,
	HDMI_RPC_CB_CMD_CEC_MSG = 3,
	HDMI_RPC_CB_CMD_HDCP_MSG = 4,
	HDMI_RPC_CB_CMD_LOG_MSG = 5,
	HDMI_RPC_CB_CMD_HDCP_RECEIVER_ID_NOTIFY_MSG =6,
	HDMI_RPC_CB_CMD_NUM
};

typedef struct 
{
	enum HDMI_RPC_CB_CMD  cmd;
	unsigned char param[128];
	UINT32 param_len;
} HDMI_CALLBAK_INFO;

enum HDMI_LINK_STATUS {
	HDMI_LINK_STATUS_UNKNOWN = 0x00000000,
	HDMI_LINK_STATUS_UNLINK = 0x00000001,
	HDMI_LINK_STATUS_HDCP_SUCCESSED = 0x00000002,
	HDMI_LINK_STATUS_HDCP_FAILED = 0x00000004,
	HDMI_LINK_STATUS_HDCP_IGNORED = 0x00000008,
	HDMI_LINK_STATUS_HDCP_NOT_IGNORED = 0x00000010,
	HDMI_LINK_STATUS_LINK = 0x00000020,
	HDMI_LINK_STATUS_CEC_IGNORED = 0x00000040,
	HDMI_LINK_STATUS_CEC_NOT_IGNORED = 0x00000080,
	HDMI_LINK_STATUS_HDMI_IGNORED = 0x00000100,
	HDMI_LINK_STATUS_HDMI_NOT_IGNORED = 0x00000200,
};

////////////////////////////////////////////////////
/////////////		New	API			  ///////////////
////////////////////////////////////////////////////
INT32 api_get_audio_infoframe(struct ifm_packet *pAudioIfm);
INT32 api_set_audio_infoframe(struct ifm_packet *pAudioIfm);

INT32 api_get_audio_settings(UINT8* pHdmi_Aud_setting);

INT32 api_get_avi_infoframe(struct ifm_packet *pAviIfm);
INT32 api_set_avi_infoframe(struct ifm_packet *pAviIfm);


INT32 api_get_CEC_data(UINT8 *pCEC_Data);

INT32 api_get_audio_capable(UINT8 *pAudioCapable);
INT32 api_get_edid_serial_number_string(char *strSN);
INT32 api_get_edid_feature_support(UINT8 *pFS);
INT32 api_get_edid_color_encoding(UINT8 *pCE);
//Please refer api_get_edid_CEA_item.
//INT32 api_get_edid_svd(UINT32 *pSVD);

INT32 api_get_raw_edid(UINT16 nLength,UINT8 *pDataBuf);
INT32 api_get_raw_edid_length(UINT16 *nLength);

INT32 api_get_edid_version(UINT16 *version); 
INT32 api_get_edid_max_screen_size(UINT16 *screen_size); 

INT32 api_get_vsi_infoframe(struct ifm_packet *pVsiIfm);
INT32 api_set_vsi_infoframe(struct ifm_packet *pVsiIfm);

BOOL api_get_hdmi_MDI_onoff(void);
void api_set_hdmi_MDI_onoff(BOOL bOnOff);

/* unable to implement
INT32 api_get_connect_map(void); //need cec supported devices
INT32 api_hdmi_get_driver_notify(); //should implement by middleware
INT32 api_hdmi_issue_command(); //need cec supported devices
INT32 api_set_audio_settings(); //need to implement by audio device driver
INT32 api_set_color_encoding(); //need to implement by DE device driver
INT32 api_set_output_resolution(); //need to implement by DE device driver
*/
//20140801 KP end

UINT16 api_get_edid_CEA_num(CEA_DB_TYPE datablock_type);
UINT32* api_get_edid_CEA_item(UINT16 idx,CEA_DB_TYPE datablock_type);
HDMI_3D_DESCRIPTOR* api_get_CEA_3D_descriptor(void);
STAND_TIMING_DESC* api_get_HDMI_stand_timings(UINT8 idx);
UINT32 api_set_av_mute(BOOL IsMute);
UINT32 api_get_av_mute(void);

INT32 api_set_hdcp_key(UINT8 *pKey, UINT16 nKeySize);
INT32 api_get_raw_edid_block(UINT8 nBlkIdx, UINT8 *pDataBuf,UINT16 nBufLen);
UINT32 api_set_av_blank(BOOL IsBlank);
UINT32 api_get_av_blank(void);

BOOL api_get_hdmi_device_support(void);
void api_set_audio_transcoding_manual(BOOL b_on_off);
BOOL api_get_audio_transcoding_manual(void);
void api_set_hdcp_ce_key(unsigned char *key, UINT32 key_len, BOOL is_set_to_ce);
int apt_get_hdcp_ce_key(unsigned char *key, UINT32 key_len);
void api_set_hdmi_mem_sel(BOOL mem_sel);
BOOL api_get_edid_ready(void);
INT32 api_hdmi_stop_transmit(void);

enum HDMI_LINK_STATUS api_get_hdmi_link_status(void);
INT32 api_set_hdmi_link_status(enum HDMI_LINK_STATUS status);

INT32 api_set_hdmi_audio_pcm_ch_out(UINT32 ch_count);
INT32 api_get_hdmi_audio_pcm_ch_out(void);
UINT32 api_program_hdmi_hdcp(UINT8* hdcp_key, UINT32 length);
UINT32 api_hdmi_cm_protection_enable(void);
UINT32 api_hdmi_audio_interface_set(enum HDMI_AUD_INTERFACE type);
void api_set_hdmi_bist_mode(UINT32 bist_on_off);
void api_set_hdmi_scramble_mode(UINT32 scramble_mode);
HDMI_PM_STATUS api_get_hdmi_pm_status(void);
INT32 hdmi_tx_attach(struct config_hdmi_parm *hdmi_param);

#ifdef __cplusplus
}
#endif

#endif

