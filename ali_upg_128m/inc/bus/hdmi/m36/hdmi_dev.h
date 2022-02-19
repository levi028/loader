#ifndef __HDMI_DEV_H__
#define __HDMI_DEV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define HDMI_ASSERT           ASSERT
//#define HDMI_DEGUB_PRINT    1
//#define S3602A_IC_PATCH        1

#define    PRI_HDCP_DEV_ID    0x74
#define    SND_HDCP_DEV_ID    0x76
#define    EDID_DEV_ID        0xA0

#define HLD_MAX_NAME_SIZE 16

//*********************InfoFrame/Packet header *******************/

struct data_island_pkt
{
    struct ifm_packet    *avi_info_frame;
    struct ifm_packet    *audio_info_frame;
    struct ifm_packet    *vsi_info_frame;
    struct ifm_packet    *mpeg_info_frame;
    struct ifm_packet    *spd_info_frame;
    struct ifm_packet    *general_packet;
    struct ifm_packet    *acp_packet;
    struct ifm_packet    *isrc1_packet;
    struct ifm_packet    *isrc2_packet;
    struct ifm_packet    *drm_info_frame;
};

//*********************** The structure of NCTS **************************
struct HDMI_NCTS
{
    UINT8 n_data[3];
    UINT8 cts_data[3];
};

/********************************************************************
*                    app config HDMI parameters                            *
********************************************************************/
struct config_hdmi_parm
{
	UINT8 *spd_vendor_name;
	UINT8 *spd_product_desc;
	UINT8 cm_scl;
	UINT8 cm_sda;
	UINT8 *cm_password;
	UINT8 *cm_secretseed;
	BOOL   hdcp_disable;
	UINT8  *hdcp_cstm_key;
	BOOL  otp_ce_load_hdcp_key; // OTP_CE_LOAD_HDCP_KEY
	UINT32 i2c_for_eddc;
	BOOL boot_media;
	BOOL boot_logo;
};

//********************* The Structure of Audio Interface ***********************
struct HDMI_AUD_CONFIG
{
	BOOL aud_interface;		//TRUE is I2S interface, otherwise is SPDIF interface
	UINT8 frq_rate;
	UINT8 word_length;
	UINT8 edge_i2s;
	UINT8 lr_clk;
	UINT8 i2s_mode;
	UINT8 ch_status[5];
	UINT8 ch_number;
	UINT8 UV;
	UINT8 u_spdif_clk_edge;
};

/********************************************************************
*                    command type with hdmi I/O control                    *
********************************************************************/
enum HDMI_IO_CMD_TYPE
{
	HDMI_CMD_AV_MUTE = 1,
	HDMI_CMD_GET_MUTE_STA,
	HDMI_CMD_RST_AUD_CH,
	HDMI_CMD_INTERFACE_MODE,
	HDMI_CMD_SWITCH_MDI_INT,
	HDMI_CMD_SET_HDCP_EN,
	HDMI_CMD_GET_EDID_RES,
	HDMI_CMD_REG_CALLBACK,
	HDMI_CMD_AUD_INTERFACE,
	HDMI_CMD_ADD_SRM_LIST,
	HDMI_CMD_SET_TMDS_DRV_CURRENT,
};

enum HDMI_TMDS_DRV_CURRENT   //TMDS driver current 9ma,9.5ma,10ma,10.5ma,11ma,11.5ma.
{
    HDMI_DRV_CUR_8MA = 0,
	HDMI_DRV_CUR_85MA,
	HDMI_DRV_CUR_9MA,
    HDMI_DRV_CUR_95MA,
    HDMI_DRV_CUR_10MA,
    HDMI_DRV_CUR_105MA,
    HDMI_DRV_CUR_11MA,
    HDMI_DRV_CUR_115MA,
    HDMI_DRV_CUR_12MA,
	HDMI_DRV_CUR_125MA,
    HDMI_DRV_CUR_13MA,
    HDMI_DRV_CUR_135MA,
    HDMI_DRV_CUR_14MA,
    HDMI_DRV_CUR_145MA,
    HDMI_DRV_CUR_15MA,
    HDMI_DRV_CUR_155MA,
    HDMI_DRV_CUR_16MA,
};

enum HDMI_AUD_INTERFACE
{
	HDMI_AUD_AUTO = 0, 	//default value, by EDID, if HDMI Sink support decoder, switch to SPDIF out otherwise switch to LPCM out.
	HDMI_AUD_LPCM,		// Fix LPCM output
	HDMI_AUD_SPDIF,		// Fix output the same with SPDIF interface.
	HDMI_AUD_SNDDRV,
};

enum hdmicbtype
{
	HDMI_CB_EDID_READY = 0,
	HDMI_CB_HOT_PLUG_OUT,
	HDMI_CB_DBG_MSG,
	HDMI_CB_HOT_PLUG_IN,
	HDMI_CB_CEC_MESSAGE,
	HDMI_CB_HDCP_FAIL,
};

typedef enum
{
    HDMI_PM_SUSPEND = 0,
    HDMI_PM_RESUME = 1,
    HDMI_PM_UNKOWN
}  HDMI_PM_STATUS;

typedef enum
{
    HDMI_PM_NO_EVENT= 0x00,
    HDMI_PM_SAVE_STATUS_DONE = 0x01,
    HDMI_PM_RESTORE_STATUS_DONE = 0x02,
} HDMI_PM_EVENT;

/********************************************************************
*                        HDMI device structure                                 *
********************************************************************/
struct hdmi_device
{
	struct hdmi_device   *next;				/* Next hdmi device structure */
	UINT32	type;							/* Interface hardware type */
	INT8	name[HLD_MAX_NAME_SIZE];		/* Device name */
	UINT32	hdmi_base_addr;					/* HDMI address */
	UINT32	hdcp_pri_base_addr;				/* HDCP Primary address */
	UINT32	hdcp_snd_base_addr;				/* HDCP Secondly address */
	UINT32	edid_base_addr;					/* EDID address */
    
    int pm_event_id;
    UINT32 pm_event_condition_bit_mask;
    HDMI_PM_STATUS pm_status;	

	UINT32	flags;							/* Interface flags, status and ability */
/* Hardware privative structure */
	void	*priv;							/* pointer to private data */
/* HDMI Device Initialization */
    INT32    (*init)(struct config_hdmi_parm *);
/* HDMI Device Open */
    INT32    (*open)(struct hdmi_device *);
/* HDMI Device Close */
    INT32    (*close)(struct hdmi_device *);
/* HDMI Device Start Transmit */
    INT32    (*start_tx)(struct hdmi_device *, struct data_island_pkt *, UINT16, struct HDMI_NCTS *);
/* HDMI Device StopTransmit */
    INT32    (*stop_tx)(struct hdmi_device *);
/* HDMI I/O control */
    INT32    (*io_control)(struct hdmi_device *, enum HDMI_IO_CMD_TYPE, UINT32, UINT32);
/* HDCP authentication process */
    INT32    (*hdcp_authen)(struct hdmi_device *, UINT32, UINT32 *);
/* HDMI Read EDID from TV */
    INT32    (*read_edid)(UINT8, UINT8 *, UINT16);
};

#ifdef __cplusplus
}
#endif

#endif

