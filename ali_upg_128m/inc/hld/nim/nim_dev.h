/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_dev.h
*
*    Description:  nim header
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __HLD_NIM_DEV_H__
#define __HLD_NIM_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>



/*for dvbs front end, define diseqc support*/
#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
#ifndef DISEQC_SUPPORT
#define DISEQC_SUPPORT
#endif
#endif


#define EXT_QPSK_MODE_SPI         0
#define EXT_QPSK_MODE_SSI         1
#define NIM_CHIP_ID_M3501A        0x350100C0
#define NIM_CHIP_ID_M3501B        0x350100D0
#define NIM_CHIP_SUB_ID_S3501D    0x00
#define NIM_CHIP_SUB_ID_M3501B    0xC0
#define NIM_CHIP_SUB_ID_M3501C    0xCC
#define NIM_FREQ_RETURN_REAL    0
#define NIM_FREQ_RETURN_SET        1

#define FS_MAXNUM 15000
#define TP_MAXNUM 2000

#define DEMOD_GPIO_0	0
#define DEMOD_GPIO_1	1
#define DEMOD_GPIO_2	2
#define DEMOD_GPIO_3	3
#define DEMOD_GPIO_4	4

struct DEMOD_GPIO_STRUCT
{
	INT16 position;
	UINT8 value;
};

// For M3501 , 13.5--> 99MHz
// For S3501D, 13.5--> 90MHz
// Here the CRYSTAL_FREQ is very different with M3501
#define CRYSTAL_FREQ             13500    //13.5M

/* NIM Device I/O control command */
enum nim_device_ioctrl_command
{
    NIM_DRIVER_READ_TUNER_STATUS,        /* Read tuner lock status */
    NIM_DRIVER_READ_QPSK_STATUS,          /* Read QPSK  lock status */
    NIM_DRIVER_READ_FEC_STATUS,           /* Read FEC lcok status */

    NIM_DRIVER_READ_QPSK_BER,             /* Read QPSK Bit Error Rate */
    NIM_DRIVER_READ_VIT_BER,              /* Read Viterbi Bit Error Rate */
    NIM_DRIVER_READ_RSUB,                 /* Read Reed Solomon Uncorrected block  or per*/
    NIM_DRIVER_STOP_ATUOSCAN,            /* Stop autoscan */
    NIM_DRIVER_UPDATE_PARAM,            /* Reset current parameters */
    NIM_DRIVER_TS_OUTPUT,                /* Enable NIM output TS*/
    NIM_DRIVER_FFT,
    NIM_DRIVER_GET_CR_NUM,
    NIM_DRIVER_GET_CUR_FREQ,
    NIM_DRIVER_FFT_PARA,
    NIM_DRIVER_GET_SPECTRUM,
    NIM_FFT_JUMP_STEP,                  /* Get AutoScan FFT frequency jump step */ //huang hao add for m3327 qpsk
    NIM_DRIVER_READ_COFFSET,               /* Read Carrier Offset state */ //joey add for stv0297.
    NIM_DRIVER_SEARCH_1ST_CHANNEL,        /* Search channel spot*/ //joey add for stv0297.
    NIM_DRIVER_SET_TS_MODE,                /*Set ts output mode:*/
                                        /*bit0: 1 serial out,     0 parallel out*/
                                        /*bit1: 1 clk rising,     0 clk falling*/
                                        /*bit2: 1 valid gap,        0 valid no gap*/
    NIM_DRIVER_SET_PARAMETERS,            /* Set the parameters of nim,add by Roman at 060321 */
    NIM_DRIVER_SET_RESET_CALLBACK,        /* When nim device need to be reset, call an callback to notice app */
    NIM_DRIVER_ENABLE_DVBS2_HBCD,        /* For DVB-S2, enable/disable HBCD mode */
    NIM_DRIVER_STOP_CHANSCAN,            /* Stop channel change because some low symbol rate TP too long to be locked */
    NIM_DRIVER_RESET_PRE_CHCHG,            /* Reset nim device before channel change */

    NIM_ENABLE_IC_SORTING,                /*Enable IC sorting, set IC sorting param*/
    NIM_DRIVER_GET_OSD_FREQ_OFFSET,     /*get OSD freq offset. */
    NIM_DRIVER_SET_RF_AD_GAIN,            /* Set RF ad table for RSSI display.*/
    NIM_DRIVER_SET_IF_AD_GAIN,            /* Set IF ad table for RSSI display. */
    NIM_DRIVER_GET_RF_IF_AD_VAL,        /* get RF IF ad value. */
    NIM_DRIVER_GET_REC_PERFORMANCE_INFO,/* get receiver performance info. */
    NIM_DRIVER_ENABLE_DEBUG_LOG,        /* enable nim driver debug infor out.*/
    NIM_DRIVER_DISABLE_DEBUG_LOG,        /* disable nim driver debug infor out.*/
    NIM_DRIVER_GET_FRONTEND_STATE,        /* Read front end state.*/
    NIM_DRIVER_SET_FRONTEND_LOCK_CHECK,    /* Set front end lock check flag.*/
    NIM_DRIVER_LOG_IFFT_RESULT,
    NIM_DRIVER_CHANGE_TS_GAP,            /* Change ts gap */
    NIM_DRIVER_SET_SSI_CLK,                /* Set SSI Clock */

    //use io control command to instead of function interface for combo soluation
    NIM_DRIVER_SET_POLAR,               /*DVB-S NIM Device set LNB polarization */
    NIM_DRIVER_SET_12V,                 /*DVB-S NIM Device set LNB votage 12V enable or not */
    NIM_DRIVER_GET_SYM,                 /* Get Current NIM Device Channel Symbol Rate */
    NIM_DRIVER_GET_BER,                 /* Get Current NIM Device Channel Bit-Error Rate */
    NIM_DRIVER_GET_AGC,                 /* Get Current NIM Device Channel AGC Value */
    NIM_DRIVER_DISABLED,                 /* Disable Current NIM Device */
    NIM_DRIVER_GET_GUARD_INTERVAL,
    NIM_DRIVER_GET_FFT_MODE,
    NIM_DRIVER_GET_MODULATION,
    NIM_DRIVER_GET_SPECTRUM_INV,

    //AD2DMA tool
    NIM_DRIVER_ADC2MEM_START,
    NIM_DRIVER_ADC2MEM_STOP,
    NIM_DRIVER_ADC2MEM_SEEK_SET,
    NIM_DRIVER_ADC2MEM_READ_8K,

	NIM_DRIVER_GET_DEMOD_LOCK_MODE, //TRUE Synchronous, FALSE Asynchronous
	NIM_DRIVER_SET_DEMOD_LOCK_MODE,
	NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY,//only search T2 signal in blindtune
	NIM_DRIVER_T2_LITE_ENABLE,//T2-Lite profile support in blindtune

	NIM_DRIVER_SET_BLSCAN_MODE,    // Para = 0, NIM_SCAN_FAST; Para = 1, NIM_SCAN_ACCURATE(slower)
	
	//!< Use io control command for T2 signal parameter latch.
    NIM_DRIVER_GET_PLP_MODE,
    NIM_DRIVER_GET_PP_MODE,
    NIM_DRIVER_GET_PAPR_MODE,
    NIM_DRIVER_TSO_CONFIG,
    NIM_DRIVER_SYM_LIMIT_RANGE,
	NIM_DRIVER_SEARCH_T_SIGNAL_ONLY,   //!< Only search T signal in blindtune, 20150906
	NIM_DRIVER_CAP_DATA,

	NIM_DRIVER_SET_PLSN,			//set plsn, the value must be gold
	NIM_DRIVER_GET_PLSN,			//get plsn, the autosearch value for plsn
	NIM_DRIVER_PLSN_GOLD_TO_ROOT,	//change the plsn value from gold to root
	NIM_DRIVER_PLSN_ROOT_TO_GOLD,	//change the plsn value from root to gold
	NIM_DRIVER_GENERATE_TABLE,		//generate table for plsn query
	NIM_DRIVER_RELEASE_TABLE,		//release the space
	NIM_DRIVER_GET_PLSN_FINISH_FLAG,

	NIM_DRIVER_SET_TSN,
    NIM_DRIVER_GET_TSN,
    NIM_DRIVER_GET_WIDEBAND_MODE,
    NIM_DRIVER_GET_TSN_FINISH_FLAG,

	NIM_DRIVER_GET_DEMOD_GAIN,
    NIM_DRIVER_GET_DBM_LEVEL,
	NIM_DRIVER_GET_AGC_DBM,
	NIM_DRIVER_GET_VALID_FREQ,
	NIM_DRIVER_GET_SYMBOL,

	// For acm function by paladin.ye 2015.12.24
    NIM_ACM_MODE_GET_TP_INFO,
    NIM_ACM_MODE_SET_ISID,
    NIM_ACM_MODE_GET_ISID,
    NIM_ACM_MODE_GET_BBHEADER_INFO,
    NIM_TS_GS_AUTO_CAP,
    NIM_CAP_STOP, 

	NIM_DRIVER_GET_MER,
	
	NIM_LNB_GET_OCP,
    NIM_LNB_SET_POWER_ONOFF,

	NIM_DRIVER_GPIO_OUTPUT,
	NIM_DRIVER_ENABLE_GPIO_INPUT,
	NIM_DRIVER_GET_GPIO_INPUT,
	
	// Add by paladin for fast lnb detection
	NIM_DRIVER_GET_FAST_LOCK_DETECT,
    //end

	NIM_DRIVER_DVBC_FASTSCAN_ENABLE,
    
    // The following code is for tuner io command
    NIM_TUNER_COMMAND = 0x8000,
    NIM_TUNER_POWER_CONTROL,
    NIM_TUNER_GET_AGC,
    NIM_TUNER_GET_RSSI_CAL_VAL,
    NIM_TUNER_RSSI_CAL_ON, //RSSI calibration on
    NIM_TUNER_RSSI_CAL_OFF,
    NIM_TUNER_RSSI_LNA_CTL,//RSSI set LNA
    NIM_TUNER_SET_GPIO,
    NIM_TUNER_CHECK,
    
    NIM_TURNER_SET_STANDBY,/*set av2012 standby add by bill 2012.02.21*/

    NIM_TUNER_SET_THROUGH_MODE,
    NIM_TUNER_GET_RF_POWER_LEVEL,    //get tuner power level for SQI/SSI.

	  //begin:added by robin.gan on 2015.4.15
    NIM_TUNER_C3031B_ID,                //!< C3031B ID
    NIM_TUNER_SET_C3031B_FREQ_ERR,   	//!< set c3031b channel_freq_err
    NIM_TUNER_GET_C3031B_FREQ_ERR,   	//!< Get c3031b channel_freq_err
    //end
    
	NIM_TUNER_GET_C3031B_GAIN_FLAG,    
	
	NIM_TUNER_RE_INIT,
};     

/*@brief
Structure for get spectrum parameters 
*/
struct nim_get_symbol
{  
	UINT32 point_length;
	UINT8 *p_cr_out_i;			
	UINT8 *p_cr_out_q;	
};

//autoscan type
enum AUTOSCAN_TYPE
{
    NORMAL_MODE = 0,        // Do 20 times fft, do channel search, program search
    GET_TP_ONLY = 1,        // Do 20 times fft, do channel search
	GET_SPECTRUM_ONLY = 2,  // Do 10 times fft
};

enum nim_device_cmd_scan_tool
{
    NIM_ASCAN_START_PROCESS         = 0x00FA0000,
    NIM_ASCAN_END_PROCESS           = 0x00FA0001,
    NIM_ASCAN_GET_LOOP_CNT          = 0x00FA0002,
    NIM_ASCAN_SET_LOOP_CNT          = 0x00FA0003,
    NIM_ASCAN_GET_TIME              = 0x00FA0004,
    NIM_ASCAN_GET_TPS               = 0x00FA0005,
    NIM_ASCAN_GET_SCAN_TP_ONLY      = 0x00FA0006,
    NIM_ASCAN_SET_SCAN_TP_ONLY      = 0x00FA0007,
    NIM_ASCAN_GET_FREQ_START        = 0x00FA0008,
    NIM_ASCAN_SET_FREQ_START        = 0x00FA0009,
    NIM_ASCAN_GET_FREQ_END          = 0x00FA000A,
    NIM_ASCAN_SET_FREQ_END          = 0x00FA000B,
    NIM_ASCAN_GET_LOG_ONOFF         = 0x00FA000C,
    NIM_ASCAN_SET_LOG_ONOFF         = 0x00FA000D,
    NIM_ASCAN_GET_SATS_IDX          = 0x00FA000E,
    NIM_ASCAN_SET_SATS_IDX          = 0x00FA000F,
    NIM_ASCAN_CLEAR_LOOP_CUR_IDX    = 0x00FA0010,
    NIM_ASCAN_GET_TOP_FLAG          = 0x00FA0011,
    NIM_ASCAN_SET_INFO              = 0x00FA0012,
};

/* NIM Device I/O control command  extension */
enum nim_device_ioctrl_command_ext
{
    NIM_DRIVER_AUTO_SCAN            = 0x00FC0000,    /* Do AutoScan Procedure */
    NIM_DRIVER_CHANNEL_CHANGE        = 0x00FC0001,    /* Do Channel Change */
    NIM_DRIVER_CHANNEL_SEARCH        = 0x00FC0002,    /* Do Channel Search */
    NIM_DRIVER_GET_RF_LEVEL            = 0x00FC0003,    /* Get RF level */
    NIM_DRIVER_GET_CN_VALUE            = 0x00FC0004,    /* Get CN value */
    NIM_DRIVER_GET_BER_VALUE        = 0x00FC0005,    /* Get BER value */
    NIM_DRIVER_QUICK_CHANNEL_CHANGE = 0x00FC0006,   /* Do Quick Channel Change without waiting lock */
    NIM_DRIVER_GET_ID                = 0x00FC0007,   /*Get 3501 type: M3501A/M3501B  or other nim type */
    NIM_DRIVER_SET_PERF_LEVEL        = 0x00FC0008,    /* Set performance level */
    NIM_DRIVER_START_CAPTURE        = 0x00FC0009,    /* Start capture */
    NIM_DRIVER_GET_I2C_INFO         = 0x00FC000A,    /* Start capture */

    NIM_DRIVER_GET_FFT_RESULT       = 0x00FC000B,   /* Get Current NIM Device Channel FFT spectrum result */
    NIM_DRIVER_DISEQC_OPERATION     = 0x00FC000C,   /* NIM DiSEqC Device Opearation */
    NIM_DRIVER_DISEQC2X_OPERATION   = 0x00FC000D,   /* NIM DiSEqC2X Device Opearation */
    NIM_DRIVER_SET_NIM_MODE              = 0x00FC000E, /* NIM J83AC/J83B mode setting */
    NIM_DRIVER_SET_QAM_INFO            = 0x00FC000F,
    NIM_DRIVER_RESET_QAM_FSM        = 0X00FC0010,    //kent,for reset qam finite state machine
    NIM_DRIVER_GET_TUNER_STA        = 0x00FC0011     // For reinit tuner by paladin.ye 03/15/2016 support jiuzhou siti CSTM    
};

/* NIM performance level setting */
enum nim_perf_level
{
    NIM_PERF_DEFAULT        = 0,
    NIM_PERF_SAFER            = 1,
    NIM_PERF_RISK            = 2,
};

/* FrontEnd Type */
enum fe_type
{
    FE_QPSK    = 0x00,     /* DVB-S */
    FE_QAM     = 0x01,        /* DVB-C */
    FE_OFDM    = 0x02         /* DVB-T */
};

enum nim_cofdm_work_mode
{
    NIM_COFDM_SOC_MODE = 0,
    NIM_COFDM_ONLY_MODE = 1
};

enum nim_cofdm_ts_mode
{
    NIM_COFDM_TS_SPI_8B = 0,
    NIM_COFDM_TS_SPI_4B = 1,
    NIM_COFDM_TS_SPI_2B = 2,
    NIM_COFDM_TS_SSI = 3
};

/* Structure for NIM Device Status */
struct nim_device_stats
{
    UINT16    temp_num;            /* blank */
};

/* Structure for NIM DiSEqC Device Information parameters */
struct t_diseqc_info
{
    UINT8 sat_or_tp;            /* 0:sat, 1:tp*/
    UINT8 diseqc_type;
    UINT8 diseqc_port;
    UINT8 diseqc11_type;
    UINT8 diseqc11_port;
    UINT8 diseqc_k22;
    UINT8 diseqc_polar;            /* 0: auto,1: H,2: V */
    UINT8 diseqc_toneburst;        /* 0: off, 1: A, 2: B */

    UINT8 positioner_type;        /*0-no positioner 1-1.2 positioner support, 2-1.3 USALS*/
    UINT8 position;                /*use for DiSEqC1.2 only*/
    UINT16 wxyz;                /*use for USALS only*/
};
typedef enum
{
    USAGE_TYPE_AUTOSCAN = 0,    //Try to auto detect the signal type (such as DVB-T or DVB-T2).
    USAGE_TYPE_CHANSCAN = 1,    //Try to auto detect the signal type (such as DVB-T or DVB-T2).
    //Tune quickly for play program, don't need to auto detect the signal type (such as DVB-T or DVB-T2).
    USAGE_TYPE_CHANCHG  = 2,
    //Try to auto detect the signal type (such as DVB-T or DVB-T2),
    //but don't spend time to wait DVB-T2 sync locked, just for improve user experience.
    USAGE_TYPE_AERIALTUNE  = 3,
    USAGE_TYPE_NEXT_PIPE_SCAN = 4,  //Try to auto detect the next signal pipe within this channel,
    //such as the next PLP of (MPLP of DVB-T2), or the next priority signal(Hierarchy of DVB-T).
    //Before USAGE_TYPE_NEXT_PIPE_SCAN can be used, you must call USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANSCAN first.
    USAGE_TYPE_SCAN_NEXT_SIGNAL = 5,    //!< search T2_Base and T2_Lite signal
} CHANNEL_CHANGE_USAGE_TYPE;


// For verification ACM mode
//struct nim_c3505_channel_info
struct nim_dvbs_channel_info
{
    UINT32 symbol_rate;
    INT32 freq_offset;
    UINT8 work_mode;    // 1=dvbs2, 0=dvbs
    UINT8 sub_work_mode; 
    UINT8 code_rate;
    UINT8 roll_off;
    UINT8 pilot_mode;
    UINT8 modcod;
    UINT8 iqswap_flag;
    UINT8 map_type;
    UINT8 frame_type; 
    INT8 signal_level;
    INT16 snr;
    UINT32 per;
	UINT32 ber;
    INT32 phase_err;
    UINT32 period_per;  // For per monitor
    UINT8 phase_err_stat_cnt;
    UINT8 fake_lock_dete_thr;
    UINT8 fake_lock_rst_cnt;
};

//struct nim_c3505_bb_header_info
struct nim_dvbs_bb_header_info
{
    // BBHEADER 10Byte
    // [MATYPE(2)]  [UPL(2)]  [DFL(2)]  [SYNC(1)]  [SYNCD(2)]  [CRC-8(1)] 
    UINT8 source_data[10];    // Source bb header data from reg105

    // Decompose of BBHEADER
	UINT8 stream_mode;  // 11=ts/00~01=gs
	UINT8 stream_type;  // 1=signle/0=multi
	UINT8 acm_ccm;      // 0=acm/1=ccm
	UINT8 issyi;        // 1=active/0=no active
	UINT8 npd;          // 1=active/0=no active
	UINT8 roll_off;     //
	UINT8 isi;          // Valid when stream_type == NIM_STREAM_MULTIPLE
	UINT16 upl;         // 0~65535, 0 = continu stream, 188x8d = MPEG ts 
	UINT16 dfl;         // 0 ~ 58112
	UINT8 sync;         //
	UINT16 syncd;       //
	UINT8 crc_8;        //
};


struct nim_tso_clk_cfg
{
    UINT8 is_manual_mode;   //: 0 = auto mode(HW calculate it automatic), 1 = manual mode
    UINT8 tso_clk_src;      //: tso clk source select(Only calid when is_manual_mode = 1)
    UINT8 clk_div_bypass;   //: tso clk div bypass, 0 = don't bypass, 1 = bypass (Only calid when is_manual_mode = 1)
    UINT8 clk_div;          //: tso clk div select(Only calid when is_manual_mode = 1 and clk_div_bypass = 0)
};


struct nim_tso_cfg
{
    struct nim_tso_clk_cfg tso_clk_cfg;
    UINT8 tso_switch;
    UINT8 tso_dummy_switch;
    UINT8 tso_clk_phase;
    UINT8 tso_clk_pad_driving;
    UINT8 tso_data_pad_driving;
};

struct nim_dvbs_data_cap
{
    UINT32 dram_base_addr;	// Mem address for demod hw used
    UINT32 cap_len;         // Men len
    UINT8 cap_src;          // Data source which need captured
};

typedef enum PLSN_STATE//added by robin in 20170228
{
	PLSN_RUNNING,	//the task is running
	PLSN_FIND,		//the value has found
	PLSN_OVER,		//all value is tried
	PLSN_STOPED,		//the task is stoped
	PLSN_UNSUPPORT,	//not support super scan
}plsn_state;

typedef enum TSN_OPERATE_TYPE {
	TSN_SET_TP_ONLY,		//set tp only
	TSN_SET_TP_AND_TSN,		//set tp and tsn
	TSN_SET_TP_AND_GET_TSN,	//set tp and get tsn
} tsn_operate_type_t;

typedef struct
{
	UINT8  super_scan; 		//set by upper level,  control whether do super scan, 1:super scan, 0:nomal scan
	UINT8  start_search;	//set by driver,  if the TP is config, set start_search to 1, super_scan and start_search control whether start plsn search
	UINT8  auto_scan_start;
	// For 3501h
	UINT8  plsn_algo_preset;	// from upper
	UINT8  search_algo;		// current search used
	UINT8  search_lock_state;	// 0:haven't lock, 1: pl lock,  2: cr lock
	UINT8  plsn_hw_find;	// New for 3501h
	UINT8  plsn_hw_end;		// New for 3501h
	UINT8  plsn_hw_en;
	UINT8  plsn_same_cnt;
	UINT32 plsn_same_value;
	UINT8  plsn_patch_flag;  // For spec plsn value:262141 in BBH algo 20180106 paladin
	// End
	UINT8  search_plsn_force_stop;
	UINT8  search_plsn_stop;
	UINT8  plsn_find;	
	UINT32 plsn_num;		//how many plsn has found
	UINT32 plsn_val[64];
	//UINT8  plsn_update; // to the upper
	UINT32 plsn_report; // to the upper
	UINT32 plsn_now;  // from the upper
	UINT8  plsn_try;
	UINT32 plsn_try_val;
	
	INT32  plsn_search_algo;//0:pilot on algorithm, 4:bbh algorithm
	OSAL_ID plsn_mutex;
	UINT8  plsn_finish_flag;
	UINT8  plsn_pls_lock; //added for reducing cost time of autoscan when searching plsn is enable
	UINT32 plsn_pls_unlock_cnt; //added for reducing cost time of autoscan when searching plsn is enable
	UINT8  plsn_pls_timeout; //added for reducing cost time of autoscan when searching plsn is enable
}PLS;

struct nim_dvbs_isid
{
	UINT8 get_finish_flag;	//0:not finished, 1:finished	
    UINT8 isid_overflow;    // Unused
    UINT16 isid_num;         // Isid mumber, 0 means it not a multistream TP, max 256
    UINT8 isid_read[256];   // Read  isid from hardware
    UINT16 isid_read_timeout;   // Read  isid timeout, unit is ms

    UINT8 isid_bypass;       // 1 = Bypass isid filter  0= default
    UINT8 isid_mode;         // 1 = use software config, 0 = auto select first id
    UINT8 isid_write;       // Write isid from sw to hw , valid while isi_mode is 1 and isi_bypass is 0
    UINT8 debug_set_isid;	// Unused, for test 
};

struct nim_dvbs_tsn
{
	UINT8 is_wideband;				//0:not wideband, 1:wideband
	UINT8 get_finish_flag;	        // 0:not finished, 1:finished	
    UINT16 tsn_num;                 // TSN mumber, max 256
    UINT8 tsn_read[256];            // TSN read from hardware
    UINT16 tsn_read_timeout;        // TSN read timeout, unit is ms

    UINT8 tsn_enable;               // 1 = Enable tsn filter  0 = disable
    UINT8 tsn_write;                // Write tsn from sw to hw , valid while tsn_en is 1
    tsn_operate_type_t tsn_operate_tpye;
	OSAL_ID tsn_mutex;
};


/*!@struct ali_nim_agc
@brief The structure passes the argument to tuner driver and get tuner's rf_level
*/
struct ali_nim_agc
{
	UINT16 demod_agc;	//the agc of demod, transfer to tuner driver, some tuner mayn't use this param
	INT8 rf_level;    //the rf_level get from tuner driver, unit is dbm
};

struct ali_plsn_address
{
	UINT32 table_address;
	UINT32 capture_address;
};

/* Structure for Channel Change parameters */
struct NIM_CHANNEL_CHANGE
{
    UINT32 freq;            /* Channel Center Frequency: in MHz unit */
    UINT32 sym;                /* Channel Symbol Rate: in KHz unit */
    UINT8 fec;                /* Channel FEC rate */
    UINT32 bandwidth;        /* Channel Symbol Rate: same as Channel Symbol Rate ? -- for DVB-T */
    UINT8 guard_interval;    /* Guard Interval -- for DVB-T */
    UINT8 fft_mode;            /* -- for DVB-T */
    UINT8 modulation;        /* -- for DVB-T */
    UINT8 usage_type;        /* -- for DVB-T, CHANNEL_CHANGE_USAGE_TYPE */
    UINT8 inverse;            /* -- for DVB-T */
    UINT8 priority;         /* -- for DVB-T */

    UINT8 t2_signal;        //signal type 0:ISDBT,1:DVB-T signal, 2:DVB-T2 signal.,3:combo signal
    UINT8 plp_num;          //Total number of PLP within this channel.
    UINT8 plp_index;        //Current selected data PLP index.

    UINT8  plp_id;          //plp_id of plp_index.
    UINT16 t2_system_id;    //t2_system_id of this channel.

    //T2_delivery_system_descriptor: transport_stream_id that identified by t2_system_id/plp_id paire.
    UINT16 t_s_id;
    //T2_delivery_system_descriptor: original_network_id that identified by t2_system_id/plp_id paire.
    UINT16 network_id;
	UINT8  t2_profile;		  		//!< 0:SONY_DVBT2_PROFILE_BASE, 1:SONY_DVBT2_PROFILE_LITE, 2:SONY_DVBT2_PROFILE_ANY
	UINT8  t2_signal_mixed_type; 	//!< 01:T2-base, 10:T2-lite, 11:T2-base + T2-lite

    // Only For DVB-S2 multisream
    UINT8 change_type;  			//!< 0 = set TP only, 1 = set TP and ISID, 2 = set ISID only, 3 = set TP and get ISID information
    struct nim_dvbs_isid *isid;
    struct nim_dvbs_tsn *tsn;
	
	UINT8 	super_scan;	 //whether support super scan which will search plsn, 0:not support, 1:support
	UINT8	search_algo_preset;
    UINT32 	plsn_db;	//the plsn value get from database

    UINT8 freq_band;    // For dvb-s/s2, 0 = default, 1 = C band, 2 = Ku band
};

/* Structure for Channel Search parameters */
struct NIM_CHANNEL_SEARCH
{
    UINT32 freq;            /* Channel Center Frequency: in MHz unit */
    UINT32 bandwidth;        /* -- for DVB-T */
    UINT8 guard_interval;    /* -- for DVB-T */
    UINT8 fft_mode;            /* -- for DVB-T */
    UINT8 modulation;        /* -- for DVB-T */
    UINT8 fec;                /* -- for DVB-T */
    UINT8 usage_type;        /* -- for DVB-T, CHANNEL_CHANGE_USAGE_TYPE */
    UINT8 inverse;            /* -- for DVB-T */
    UINT16 freq_offset;        /* -- for DVB-T */
    UINT8 priority;         /* -- for DVB-T */

    UINT8 t2_signal;        //signal type 0:ISDBT,1:DVB-T signal, 2:DVB-T2 signal.,3:combo signal
    UINT8 plp_num;          //Total number of PLP within this channel.
    UINT8 plp_index;        //Current selected data PLP index.

    UINT8  plp_id;          //plp_id of plp_index.
    UINT16 t2_system_id;    //t2_system_id of this channel.

    //T2_delivery_system_descriptor: transport_stream_id that identified by t2_system_id/plp_id paire.
    UINT16 t_s_id;
    //T2_delivery_system_descriptor: original_network_id that identified by t2_system_id/plp_id paire.
    UINT16 network_id;
};



/* Structure for Auto Scan parameters */
struct NIM_AUTO_SCAN
{
    //>>> Unicable begin
    UINT8 unicable;
    //    UINT8 invert;    /* Fub INVERT? */
    UINT16 fub;    /* Unicable: UB slots centre freq (MHz) */
    //<<< Unicable end

    UINT32 sfreq;            /* Start Frequency of the Scan procedure: in MHz unit */
    UINT32 efreq;            /* End Frequency of the Scan procedure: in MHz unit */
    UINT8 isid_num;     // 0 = Not a multiply stream, other = multiply stream
    UINT8 isid;         // Current isid for auto scan
    UINT8 scan_type;    // 0: normal scan(single stream scan); 1: multistream scan
    INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec);    /* Callback Function pointer */
	struct CHANNEL_SPECTRUM
	{		
		INT32 *p_channel_spectrum;
		INT32 spectrum_length;
	}spectrum;
	
	struct VALID_FREQ_SYM
	{
		INT32 *p_frequency;
		INT32 *p_symbol_rate;
		INT32 real_freq_sym;
	}valid_freq_sym;

	UINT8 	super_scan;	 //whether support super scan which will search plsn, 0:not support, 1:support
    UINT32 	plsn_db;	//autoscan will set this value when the tp lock success, the uplevel can get to save
    UINT8 freq_band;    // For dvb-s/s2, 0 = default, 1 = C band, 2 = Ku band
};

/*M3328F external QPSK configuration*/
struct ext_qpsk_config
{
    UINT8 gpio_28f_reset;    /* External M3328F reset GPIO*/
    UINT8 ext_qpsk_mod;     /* EXT_QPSK_MODE_SSI / EXT_QPSK_MODE_SPI */
    UINT8 host_uart_id;        /* 1: UART1; 2: UART2; 3: UART3; other value: UART2(default)*/
    UINT8 tsi_mode;            /* Supposed TSI mode. Output parameter from NIM driver.*/
};

/* for NIM_DRIVER_SET_RESET_CALLBACK */
typedef INT32 (*pfn_nim_reset_callback)(UINT32 param);

typedef struct
{
    UINT8 lock;
    BOOL valid;
    UINT32 ber;
    UINT32 per;
} nim_rec_performance_t;

/* Structure for IC sorting parameters */
struct nim_ic_soring
{
    BOOL    b_enable;            /* Enable/disable ic sorting*/
    UINT8   *p_error_indicator;  /* pointer to global error indicator */
};

/* structure for RSSI display.*/
#if 1//def NEW_AD_GAIN_T
typedef UINT16 nim_ad_gain_t;
#else
typedef struct
{
    UINT16 ad;
    INT8 gain;
} nim_ad_gain_t;
#endif

typedef struct
{
    nim_ad_gain_t *p_table;
    int count;
} nim_ad_gain_table_t;


typedef struct
{
    UINT16 rf_ad_val;
    UINT16 if_ad_val;
} nim_rf_if_ad_val_t;

struct NIM_I2C_INFO
{
    UINT32 i2c_type;
    UINT32 i2c_addr;
};

typedef struct
{
    UINT32 freq;
    UINT32 *start_addr;
} nim_get_fft_result_t;  //for NIM_DRIVER_GET_FFT_RESULT

typedef struct
{
    UINT32 mode;
    UINT8 *cmd;
    UINT8 *rt_value;
    UINT8 *rt_cnt;
    UINT8 cnt;
} nim_diseqc_operate_para_t; //for  NIM_DRIVER_DISEQC_OPERATION

typedef enum DVBT_modulation_type {

    /**
    Value to specify the <b> auto mode </b> for the modulation type for
    DVB-T system

    @note   This mode can be set when connecting NIM Module
    */
    MODULATION_AUTO,

    /**
    Value to specify the @b DQPSK modulation
    */
    MODULATION_DQPSK,

    /**
    Value to specify the @b QPSK modulation
    */
    MODULATION_QPSK,

    /**
    Value to specify the @b 16QAM modulation
    */
    MODULATION_16QAM,

    /**
    Value to specify the @b 64QAM modulation
    */
    MODULATION_64QAM,
	/**
	   Value to specify the @b 256QAM modulation
	   */

	MODULATION_256QAM

} modulation_type_t;


/* Structure nim_device, the basic structure between HLD and LLD of demomulator device */
struct nim_device
{
    struct nim_device   *next;                        /* Next nim device structure */
    UINT32                type;                        /* Interface hardware type */
    INT8                name[HLD_MAX_NAME_SIZE];    /* Device name */
    UINT32                base_addr;                    /* Demodulator address */
    UINT32              tuner_select;                /* I2C TYPE for  TUNER select  */
    UINT16                flags;                        /* Interface flags, status and ability */

    /* Hardware privative structure */
    void        *priv;                                /* pointer to private data */
    /* Functions of this dem device */
                /* NIM Device Initialization */
    INT32    (*init)();
    INT32    (*open)(struct nim_device *dev);            /* NIM Device Open */
    INT32    (*stop)(struct nim_device *dev);            /* NIM Device Stop */
                /* NIM Device I/O Control */
    INT32    (*do_ioctl)(struct nim_device *dev, INT32 cmd, UINT32 param);
                /* NIM Device I/O Control Extension */
    INT32     (*do_ioctl_ext)(struct nim_device *dev, INT32 cmd, void *param_list);
                /* Get Current NIM Device Channel Lock Status */
    INT32    (*get_lock)(struct nim_device *dev, UINT8 *lock);
                 /* Get Current NIM Device Channel Frequency */
    INT32    (*get_freq)(struct nim_device *dev, UINT32 *freq);                 
                /* Get Current NIM Device Channel FEC Rate */
    INT32    (*get_fec)(struct nim_device *dev, UINT8 *fec);
                /* Get Current NIM Device Channel SNR Value */
    INT32    (*get_snr)(struct nim_device *dev, UINT8 *snr);
                 /* Get Current NIM Device Channel Bit-Error Rate */
    INT32    (*get_ber)(struct nim_device *dev, UINT32 *ber);
                /* Get Current NIM Device Channel AGC Value */
    INT32    (*get_agc)(struct nim_device *dev, UINT8 *agc);
                /*Complement channel Search function.*/
    INT32    (*channel_search)(struct nim_device *dev, ...);
	INT32	 (*get_mode)(struct nim_device *dev, UINT8 *work_mode);

    //For DVBS, DVBS2, DVBC.
    /* Get Current NIM Device Channel Symbol Rate */
    INT32    (*get_sym)(struct nim_device *dev, UINT32 *sym);
    /* Get Current NIM Device Channel FFT spectrum result */
    INT32    (*get_fft_result)(struct nim_device *dev, UINT32 freq, UINT32 *start_addr);

    //For DVBS, DVBS2.
    // ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)
    UINT8        diseqc_typex;                /* NIM DiSEqC Device Type */
    UINT8        diseqc_portx;                /* NIM DiSEqC Device Port */
    struct t_diseqc_info diseqc_info;        /* NIM DiSEqC Device Information Structure */
    /*DVB-S NIM Device set LNB polarization */
    INT32    (*set_polar)(struct nim_device *dev, UINT8 polar);
    /*DVB-S NIM Device set LNB votage 12V enable or not */
    INT32    (*set_12v)(struct nim_device *dev, UINT8 flag);
    /* NIM DiSEqC Device Opearation */
    INT32   (*di_seq_c_operate)(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt);
    /* NIM DiSEqC2X Device Opearation */
    INT32   (*di_seq_c2x_operate)(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, \
                 UINT8 *rt_value, UINT8 *rt_cnt);
    /* Get Current NIM Device Version Number */
    const char *(*get_ver_infor)(struct nim_device *dev);

    INT32   (*reg_read)(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
    INT32   (*reg_write)(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
    //For DVBT, ISDBT, ATSC.
    INT32    (*channel_change)(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 guard_interval, \
            UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);
    /* Disable Current NIM Device */
    INT32    (*disable)(struct nim_device *dev);
    INT32    (*get_guard_interval)(struct nim_device *dev, UINT8 *gi);
    INT32    (*get_fftmode)(struct nim_device *dev, UINT8 *fftmode);
    INT32    (*get_modulation)(struct nim_device *dev, UINT8 *modulation);
    INT32    (*get_spectrum_inv)(struct nim_device *dev, UINT8 *inv);
#ifdef SMART_ANT_SUPPORT
    INT32     (*set_smartenna)(struct nim_device *dev, UINT8 position, UINT8 gain, UINT8 pol, UINT8 channel);
    INT32     (*get_smartenna_setting)(struct nim_device *dev, UINT8 *p_position, UINT8 *p_gain,\
                                        UINT8 *p_pol, UINT8 *p_channel);
    INT32     (*get_smartenna_metric)(struct nim_device *dev, UINT8 metric, UINT16 *p_metric);
#endif
    INT32    (*get_vsb_agc)(struct nim_device *dev, UINT16 *agc);
    INT32    (*get_vsb_snr)(struct nim_device *dev, UINT16 *snr);
    INT32    (*get_vsb_per)(struct nim_device *dev, UINT32 *per);

    INT32    (*get_hier)(struct nim_device *dev, UINT8 *hier);
    INT8    (*get_priority)(struct nim_device *dev, UINT8 *priority);
    INT32    (*get_freq_offset)(struct nim_device *dev, INT32 *freq_offset);//051222 yuchun
};

// new add from 3503/3501h.h file for type redefinition
//#define __TDS__
//#define FPGA_PLATFORM

enum NIM_BLSCAN_MODE
{
    NIM_SCAN_FAST = 0,
	NIM_SCAN_SLOW = 1,
};
enum NIM_CHANNEL_CHANGE_TYPE
{
	NIM_CHANGE_SET_TP_ONLY = 0,
	NIM_CHANGE_SET_TP_AND_ISID = 1,
	NIM_CHANGE_SET_ISID_ONLY = 2,
	NIM_CHANGE_SET_TP_AND_GET_ISID = 3,
};
enum NIM_TS_GS_MODE
{
    NIM_GS_PACKET = 0,
	NIM_GS_CONTINUE = 1,
    NIM_RESERVED = 2,
	NIM_TS = 3,	
};

enum NIM_STREAM_TYPE
{
    NIM_STREAM_MULTIPLE = 0,
	NIM_STREAM_SINGLE = 1,
};

enum NIM_CODE_MODE
{
    NIM_ACM = 0,    // VCM is signalled as ACM
	NIM_CCM = 1,
};

enum NIM_ISSYI
{
    NIM_ISSYI_NOT_ACTIVE = 0,    
	NIM_ISSYI_ACTIVE = 1,
};

enum NIM_NPD
{
    NIM_NPD_NOT_ACTIVE = 0,    
	NIM_NPD_ACTIVE = 1,
};

enum NIM_ROLL_OFF
{
    NIM_RO_035 = 0,    
	NIM_RO_025 = 1,
    NIM_RO_020 = 2,    
	NIM_RO_RESERVED = 3,	
};

enum NIM_TUNER_TYPE
{
	IS_M3031 = 0,
	NOT_M3031 = 1,
};

enum NIM_RET
{
    NIM_SUCCESS = 0,
    NIM_FAILED = 1,
};

enum DEV_SWITCH
{
    DEV_ENABLE = 0,
    DEV_DISABLE = 1,
};

enum SIGNAL_TYPE
{
	ISDBT_TYPE = 0,
	DVBT_TYPE,
	DVBT2_TYPE,
	DVBT2_COMBO
};

#ifdef __cplusplus
}
#endif

#endif  /* __HLD_NIM_DEV_H__ */
