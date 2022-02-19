/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_tuner.h
*
*    Description:  nim header
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _NIM_TUNER_H_
#define _NIM_TUNER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <basic_types.h>
/*
Nim_xxxx_attach
Function: INT32 nim_xxxx_attach (QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
Description: QPSK Driver Attach function
Parameters:  QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner, pointer to structure QPSK_TUNER_CONFIG_API
Return:         INT32, operation status code; configuration successful return with SUCCESS

Nim_Tuner _Init
Function: INT32 nim_Tuner_Init (UINT32* tuner_id,struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
Description: API function for QPSK Tuner Initialization
Parameters:  UINT32* tuner_id, return allocated tuner id value in same tuner type to demod driver.
              struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config, parameter for tuner config
Return:         INT32, operation status code; configuration successful return with SUCCESS

Nim_Tuner_Control
Function: INT32 nim_Tuner _Control (UINT32 tuenr_id, UINT32 freq, UINT32 sym)
Description: API function for QPSK Tuner's Parameter Configuration
Parameters:      UINT32 tuner_id, tuner device id in same tuner type
            UINT32 freq, Channel Frequency
            UINT32 sym, Channel Symbol Rate
Return:         INT32, operation status code; configuration successful return with SUCCESS

Nim_Tuner_Status
Function: INT32 nim_Tuner _Status (UINT32 tuner_id, UINT8 *lock)
Description: API function for QPSK Tuner's Parameter Configuration
Parameters:  UINT32 tuner_id, tuner device id in same tuner type
            UINT8 *lock, pointer to the place to write back the Tuner Current Status
Return:         INT32, operation status code; configuration successful return with SUCCESS
*/
#define MAX_TUNER_SUPPORT_NUM    2
#define FAST_TIMECST_AGC    1
#define SLOW_TIMECST_AGC    0

#define TUNER_CHIP_SANYO        9
#define TUNER_CHIP_CD1616LF_GIH    8
#define TUNER_CHIP_NXP                7
#define TUNER_CHIP_MAXLINEAR    6
#define TUNER_CHIP_MICROTUNE    5
#define TUNER_CHIP_QUANTEK	4
#define TUNER_CHIP_RFMAGIC          3
#define TUNER_CHIP_ALPS             2    //60120-01Angus
#define TUNER_CHIP_PHILIPS          1
#define TUNER_CHIP_INFINEON            0
/****************************************************************************/
//#define tuner_chip_maxlinear 6
#define TUNER_CHIP_EN4020 9//tuner_chip_en4020    9

#define _1ST_I2C_CMD        0
#define _2ND_I2C_CMD        1

/*Front End State*/
#define TUNER_INITIATING    0
#define TUNER_INITIATED        1
#define TUNER_TUNING        2
#define TUNER_TUNED            3

//add by bill for tuner standby function
#define NIM_TUNER_SET_STANDBY_CMD    0xffffffff

#define LNB_CMD_BASE        0xf0
#define LNB_CMD_ALLOC_ID    (LNB_CMD_BASE+1)
#define LNB_CMD_INIT_CHIP    (LNB_CMD_BASE+2)
#define LNB_CMD_SET_POLAR    (LNB_CMD_BASE+3)

#define LNB_CMD_POWER_ONOFF		(LNB_CMD_BASE+4)
#define LNB_CMD_RELEASE_CHIP    (LNB_CMD_BASE+5)
#define LNB_CMD_GET_OCP         (LNB_CMD_BASE+6) //Get A8304 Overcurrent status

#define LNB_CMD_POWER_EN   (LNB_CMD_BASE+7) //(LNB_CMD_BASE+4)
#define LNB_CMD_POWER_OFF	(LNB_CMD_BASE+8)//(LNB_CMD_BASE+5)



typedef INT32  (*INTERFACE_DEM_WRITE_READ_TUNER)(void *nim_dev_priv, UINT8 tuner_address, UINT8 *wdata,\
                                                int wlen, UINT8 *rdata, int rlen);


typedef struct COFDM_TUNER_CONFIG_API *PCOFDM_TUNER_CONFIG_API;


typedef struct
{
    void *nim_dev_priv;  //for support dual demodulator.
                         //The tuner can not be directly accessed through I2C,
                         //tuner driver summit data to dem, dem driver will Write_Read tuner.
    INTERFACE_DEM_WRITE_READ_TUNER  dem_write_read_tuner;
} DEM_WRITE_READ_TUNER;  //Dem control tuner by through mode (it's not by-pass mode).

/*external demodulator config parameter*/
struct EXT_DM_CONFIG
{
    UINT32 i2c_base_addr;
    UINT32 i2c_type_id;
    UINT32 dm_crystal;
    UINT32 dm_clock;
    UINT32 polar_gpio_num;
    UINT32 lock_polar_reverse;
};


/*external lnb controller config parameter*/
struct EXT_LNB_CTRL_CONFIG
{
    UINT32 param_check_sum; //ext_lnb_control+i2c_base_addr+i2c_type_id = param_check_sum
    INT32     (*ext_lnb_control) (UINT32, UINT32, UINT32);
    UINT32 i2c_base_addr;
    UINT32 i2c_type_id;
    UINT8 int_gpio_en;
    UINT8 int_gpio_polar;
    UINT32 int_gpio_num;
	UINT8 int_gpio_select;//0:main chip gpio, 1:demod gpio
};



struct COFDM_TUNER_CONFIG_DATA
{
    UINT8 *pt_mt352;
    UINT8 *pt_mt353;
    UINT8 *pt_st0360;
    UINT8 *pt_st0361;
    UINT8 *pt_st0362;
    UINT8 *pt_af9003;
    UINT8  *pt_nxp10048;
    UINT8  *pt_sh1432;
    UINT16 *pt_sh1409;

    //for ddk and normal design.
    //for I/Q conncetion config. bit2: I/Q swap. bit1: I_Diff swap. bit0: Q_Diff swap.< 0: no, 1: swap>;
    UINT8 connection_config;
            //bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
            //bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
            //bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>
            //bit5: RF-RSSI enable <0: disable, 1: enalbe>;bit6: RF-RSSI slop <0: negtive, 1: positive>
            //bit8: fft_gain function <0: disable, 1: enable>
            //bit9: "blank channel" searching function <0: accuate mode, 1: fast mode>
            //bit10~11: frequency offset searching range <0: +-166, 1: +-(166*2), 2: +-(166*3), 3: +-(166*4)>
            //bit12: RSSI monitor <0: disable, 1: enable>
    UINT16 cofdm_config;

    UINT8 AGC_REF;
    UINT8 RF_AGC_MAX;
    UINT8 RF_AGC_MIN;
    UINT8 IF_AGC_MAX;
    UINT8 IF_AGC_MIN;
    UINT32 i2c_type_sel;
    UINT32 i2c_type_sel_1;//for I2C_SUPPORT_MUTI_DEMOD
    UINT8 demod_chip_addr;
    UINT8 demod_chip_addr1;
    UINT8 demod_chip_ver;
    UINT8 tnuer_id;
    UINT8  c_tuner_tsi_setting_0;
    UINT8  c_tuner_tsi_setting_1;

    UINT32 flag;
    UINT8 *memory;
    UINT32 memory_size;
};

struct COFDM_TUNER_CONFIG_EXT
{
    UINT16  c_tuner_crystal;
    UINT8  c_tuner_base_addr;        /* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
    UINT8  c_chip;
    UINT8  c_tuner_ref_div_ratio;
    UINT16 w_tuner_if_freq;
    UINT8  c_tuner_agc_top;
    UINT16 c_tuner_step_freq;
    INT32  (*tuner_write)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);        /* Write Tuner Program Register */
    INT32  (*tuner_read)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);        /* Read Tuner Status Register */
    INT32  (*tuner_write_read)(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int len);
         /*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/
    UINT32 i2c_type_id;


         // copy from COFDM_TUNER_CONFIG_DATA struct in order to  let tuner knows whether the RF/IF AGC is
         // enable or not.
         // esp for max3580, which uses this info to turn on/off internal power detection circuit.
         // See max3580 user manual for detail.

         //bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
         //bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
         //bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>
         //bit5: RF-RSSI enable <0: disable, 1: enalbe>;bit6: RF-RSSI slop <0: negtive, 1: positive>
    UINT16 cofdm_config;

    INT32  if_signal_target_intensity;
};

#if((SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ISDBT) || defined COMBOUI)
struct COFDM_TUNER_CONFIG_API
{
    struct COFDM_TUNER_CONFIG_DATA config_data;
    INT32 (*nim_tuner_init) (UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
    INT32 (*nim_tuner_control) (UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                UINT8 *data, UINT8 cmd_type);
    INT32 (*nim_tuner_status) (UINT32 tuner_id, UINT8 *lock);
    union
    {
        INT32 (*nim_tuner_cal_agc) (UINT32 tuner_id, UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data);
        INT32 (*nim_tuner_command)(UINT32 tuner_id, INT32 cmd, UINT32 param);
    };
    void (*nim_lock_cb) (UINT8 lock);
    struct COFDM_TUNER_CONFIG_EXT tuner_config;
    struct EXT_DM_CONFIG ext_dm_config;

    UINT32 tuner_type;
    UINT32 rev_id           : 8;
    UINT32 config_mode      : 1;
    UINT32 work_mode        : 1;    // NIM_COFDM_SOC_MODE or NIM_COFDM_ONLY_MODE
    UINT32 ts_mode          : 2;    // enum nim_cofdm_ts_mode, only for NIM_COFDM_ONLY_MODE
    UINT32 reserved         : 20;
};
// >=0: successful; 1: need to config pin mux
// <0: failed


#else
struct COFDM_TUNER_CONFIG_API
{
    struct COFDM_TUNER_CONFIG_DATA config_data;
    INT32 (*nim_tuner_init) (struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
    INT32 (*nim_tuner_control) (UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 cmd_type);
    INT32 (*nim_tuner_status) (UINT8 *lock);
    INT32 (*nim_tuner_cal_agc) (UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data);

    void (*nim_lock_cb) (UINT8 lock);
    struct COFDM_TUNER_CONFIG_EXT tuner_config;
    struct EXT_DM_CONFIG ext_dm_config;
};
#endif


typedef enum//For  TSI  select
{
    NIM_0_SPI_0      = 0,
    NIM_0_SPI_1     ,
    NIM_1_SPI_0     ,
    NIM_1_SPI_1     ,
    NIM_0_SSI_0     ,
    NIM_0_SSI_1     ,
    NIM_1_SSI_0     ,
    NIM_1_SSI_1
} nim_tsi_setting ;

typedef enum//For  qam mode  select, add by robin for R858 tuner use
{
    QAM_MODE_J83AC = 0,
    QAM_MODE_J83B
} QAM_MODE_STR ;

struct QPSK_TUNER_CONFIG_DATA
{
    UINT16 recv_freq_low;
    UINT16 recv_freq_high;
    UINT16 ana_filter_bw;
    UINT8 connection_config;
    UINT8 reserved_byte;
    UINT8 agc_threshold_1;
    UINT8 agc_threshold_2;
    UINT16 qpsk_config;
    /*bit0:QPSK_FREQ_OFFSET,bit1:EXT_ADC,bit2:IQ_AD_SWAP,bit3:I2C_THROUGH,
      bit4:polar revert bit5:NEW_AGC1,
      bit6bit7:QPSK bitmode:00:1bit,01:2bit,10:4bit,11:8bit*/
    INT16 disqec_polar_position;/*control disqec_polar by gpio*/
};

struct QPSK_TUNER_CONFIG_EXT
{
    UINT16 w_tuner_crystal;            /* Tuner Used Crystal: in KHz unit */
    UINT8  c_tuner_base_addr;        /* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
    UINT8  c_tuner_out_s_d_sel;        /* Tuner Output mode Select: 1 --> Single end, 0 --> Differential */
     /*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/
    UINT32 i2c_type_id;
};

struct QPSK_TUNER_CONFIG_API
{
    /* struct for QPSK Configuration */
    struct   QPSK_TUNER_CONFIG_DATA config_data;

    /* Tuner Initialization Function */
    INT32 (*nim_tuner_init) (UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);

    /* Tuner Parameter Configuration Function */
    INT32 (*nim_tuner_control) (UINT32 tuner_id, UINT32 freq, UINT32 sym);

	//begin:added by robin.gan on 2014.4.15
	//!<Tuner special api function
	INT32 (*nim_tuner_command)(UINT32 tuner_id, INT32 cmd, INT32 *param);
	INT32 (*nim_tuner_gain)(UINT32 tuner_id, UINT32 agc_level);		
	//end
	
    /* Get Tuner Status Function */
    INT32 (*nim_tuner_status) (UINT32 tuner_id, UINT8 *lock);

    /* Extension struct for Tuner Configuration */
    struct QPSK_TUNER_CONFIG_EXT tuner_config;
    struct EXT_DM_CONFIG ext_dm_config;
    struct EXT_LNB_CTRL_CONFIG ext_lnb_config;
    UINT32 device_type;    //current chip type. only used for M3501A
};

struct QAM_TUNER_CONFIG_DATA
{
    UINT8 RF_AGC_MAX;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
    UINT8 RF_AGC_MIN;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
    UINT8 IF_AGC_MAX;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
    UINT8 IF_AGC_MIN;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
    UINT8 AGC_REF; //the average amplitude to full scale of A/D. % percentage rate.
    UINT8 c_tuner_tsi_setting;
};

struct QAM_TUNER_CONFIG_EXT
{
    UINT8  c_tuner_crystal;
    UINT8  c_tuner_base_addr;        /* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
    UINT8  c_chip;
    UINT8  c_tuner_special_config;        /*0x01, RF AGC is disabled*/
    UINT8  c_tuner_ref_div_ratio;
    UINT16 w_tuner_if_freq;
    UINT8  c_tuner_agc_top;
    UINT8  c_tuner_step_freq;

    /*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/
    UINT32 i2c_type_id;
    UINT8 c_tuner_freq_param;/* RT810_Standard_Type */
    UINT16 c_tuner_reopen;
    UINT16 w_tuner_if_freq_j83a;
    UINT16 w_tuner_if_freq_j83b;
    UINT16 w_tuner_if_freq_j83c;
    UINT8  w_tuner_if_j83ac_type; //0x00 j83a , 0x01 j83c
    QAM_MODE_STR  qam_mode;//add by robin for R858 tuner use
};

struct DEMOD_CONFIG_ADVANCED
{
    UINT32  qam_config_advanced; //bit0: demode_mode 0:j83b 1:j83ac;
    //bit1: ad sample clock 0: 27m, 1:54m;
    UINT32 qam_buffer_len;
    UINT32 qam_buffer_addr;
};
struct QAM_TUNER_CONFIG_API
{
    /* struct for QAM Configuration */
    struct QAM_TUNER_CONFIG_DATA tuner_config_data;

    /* Tuner Initialization Function */
    INT32 (*nim_tuner_init)(UINT32 *ptr_tun_id, struct QAM_TUNER_CONFIG_EXT *ptr_tuner_config);

    // Tuner Parameter Configuration Function,since there will no bandwidth demand, so pass "sym" for later use.
    INT32 (*nim_tuner_control)(UINT32 tun_id, UINT32 freq, UINT32 sym, UINT8 agc_time_const, UINT8 _i2c_cmd);

    /* Get Tuner Status Function */
    INT32 (*nim_tuner_status)(UINT32 tun_id, UINT8 *lock);

	INT32 (*nim_tuner_command)(UINT32 tuner_id, INT32 cmd, INT32 *param);	
    /* Extension struct for Tuner Configuration */
    struct QAM_TUNER_CONFIG_EXT tuner_config_ext;

    struct EXT_DM_CONFIG ext_dem_config;
    struct DEMOD_CONFIG_ADVANCED dem_config_advanced;

};

#if (SYS_TUN_MODULE == ANY_TUNER)
//extern UINT32 SYS_TUN_TYPE;
//extern UINT32 SYS_TUN_BASE_ADDR;

extern INT32 nim_mt2060_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_mt2060_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 nim_mt2060_status(UINT8 *lock);

extern INT32 tun_sch201a_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_sch201a_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_sch201a_status(UINT8 *lock);


extern INT32 tun_td1336_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_td1336_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_td1336_status(UINT8 *lock);

extern INT32 tun_dtt76806_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dtt76806_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dtt76806_status(UINT8 *lock);

extern INT32 tun_dtt76801_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dtt76801_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dtt76801_status(UINT8 *lock);

extern INT32 tun_dtt76809_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dtt76809_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dtt76809_status(UINT8 *lock);

extern INT32 tun_uba00ap_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_uba00ap_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_uba00ap_status(UINT8 *lock);

extern INT32 tun_dph261d_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dph261d_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dph261d_status(UINT8 *lock);

extern INT32 nim_mt2131_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_mt2131_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 nim_mt2131_status(UINT8 *lock);

extern INT32 tun_dtt75300_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_dtt75300_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_dtt75300_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_dtt7596_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  nim_dtt7596_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32  nim_dtt7596_status(UINT32 tuner_id, UINT8 *lock);

extern INT32    nim_dtf8570_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   nim_dtf8570_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32   nim_dtf8570_status(UINT32 tuner_id, UINT8 *lock);

extern INT32    nim_dtt75411_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   nim_dtt75411_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                     UINT8 *data, UINT8 _i2c_cmd);
extern INT32   nim_dtt75411_status(UINT32 tuner_id, UINT8 *lock);

extern INT32   tun_edt1022_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_edt1022_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_edt1022_status(UINT32 tuner_id, UINT8 *lock);

extern INT32   tun_td1611alf_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_td1611alf_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                     UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_td1611alf_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 tun_ed5065_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_ed5065_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_ed5065_status(UINT8 tuner_id, UINT8 *lock);


extern INT32    nim_tda18211_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   nim_tda18211_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   nim_tda18211_status(UINT8 tuner_id, UINT8 *lock);

extern INT32    tun_tda18218_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_tda18218_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_tda18218_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_tda18218_command(UINT32 tuner_id, INT32 cmd, UINT32 param);


extern INT32    tun_tda18212_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_tda18212_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_tda18212_status(UINT32 tuner_id, UINT8 *lock);

extern INT32    tun_bf6009_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_bf6009_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_bf6009_status(UINT32 tuner_id, UINT8 *lock);


extern INT32 tun_rd3432_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_rd3432_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_rd3432_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                 UINT8 *data, UINT8 _i2c_cmd);

extern INT32 tun_max3580_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_max3580_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_max3580_status(UINT32  tuner_id, UINT8 *lock);
extern INT32 tun_max3580_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

extern INT32 tun_mxl136_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl136_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_mxl136_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl136_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

extern INT32 tun_sharp6401_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_sharp6401_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_sharp6401_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 tun_tda18250_init(UINT32 *ptr_tun_id, struct QAM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_tda18250_control(UINT32 tun_id, UINT32 freq, UINT32 sym, UINT8 agc_time_const, UINT8 _i2c_cmd);
extern INT32 tun_tda18250_status(UINT32 tun_id, UINT8 *lock);

extern INT32 nim_av2000_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_av2000_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_av2000_status(UINT32 tuner_id, UINT8 *lock);

#if ((SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) || defined COMBOUI)

extern INT32 tun_mxl603_init_cdt_mn88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init_cxd2834(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init_isdbt(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl603_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

extern INT32    tun_mxl301_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_mxl301_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_mxl301_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl301_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

extern INT32    tun_mxl5005_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_mxl5005_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT32 tuner_id, UINT8 *lock);

extern INT32    tun_mxl5007_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_mxl5007_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl5007_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

extern INT32    tun_db5515_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_db5515_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_db5515_status(UINT32 tuner_id, UINT8 *lock);
#else

extern INT32 tun_mxl603_init_cdt_mn88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init_cxd2834(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init_isdbt(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl603_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

extern INT32    tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_mxl5005_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT8 *lock);

extern INT32  tun_mxl5007_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_mxl5007_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT8 *lock);

#endif


extern INT32 tun_ed6092b_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_ed6092b_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                 UINT8 *data, UINT8 _i2c_cmd)    ;
extern INT32 tun_ed6092b_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 tun_mxl136_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl136_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl136_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                UINT8 *data, UINT8 _i2c_cmd);

extern INT32 tun_cdt_9vm80_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_cdt_9vm80_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd)    ;
extern INT32 tun_cdt_9vm80_status(UINT32 tuner_id, UINT8 *lock);

#else

#if (SYS_TUN_MODULE==SH201A)
extern INT32 tun_sch201a_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_sch201a_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_sch201a_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==MXL603)
extern INT32 tun_mxl603_init_cdt_mn88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init_cxd2834(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init_isdbt(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl603_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#elif (SYS_TUN_MODULE==MXL301)
extern INT32    tun_mxl301_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_mxl301_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_mxl301_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl301_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#elif (SYS_TUN_MODULE==SHARP6401)
extern INT32 tun_sharp6401_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_sharp6401_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_sharp6401_status(UINT32 tuner_id, UINT8 *lock);

#elif (SYS_TUN_MODULE==TD1336)
extern INT32 tun_td1336_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_td1336_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_td1336_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==DTT76806)
extern INT32 tun_dtt76806_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dtt76806_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dtt76806_status(UINT8 *lock);

#elif ((SYS_TUN_MODULE==DTT76801)||(SYS_TUN_MODULE==DTT76852))
extern INT32 tun_dtt76801_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dtt76801_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dtt76801_status(UINT8 *lock);
#elif (SYS_TUN_MODULE==DTT76809)
extern INT32 tun_dtt76809_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dtt76809_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dtt76809_status(UINT8 *lock);

#elif( (SYS_TUN_MODULE==UBA00AP)||(SYS_TUN_MODULE==UBD00AL))
extern INT32 tun_uba00ap_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_uba00ap_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_uba00ap_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==SAMSUNG_DPH261D)
extern INT32 tun_dph261d_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dph261d_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_dph261d_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==MXL5005)
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
extern INT32    tun_mxl5005_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_mxl5005_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT32 tuner_id, UINT8 *lock);
#else
extern INT32    tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_mxl5005_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT8 *lock);
#endif
#elif (SYS_TUN_MODULE==MXL5007)
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
extern INT32    tun_mxl5007_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_mxl5007_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl5007_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#else
extern INT32  tun_mxl5007_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_mxl5007_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT8 *lock);
#endif
#elif (SYS_TUN_MODULE==MT2060)
extern INT32 nim_mt2060_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_mt2060_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 nim_mt2060_status(UINT8 *lock);
#elif (SYS_TUN_MODULE==MT2131)
extern INT32 nim_mt2131_init(struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_mt2131_control(UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
extern INT32 nim_mt2131_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==DTT75300)
extern INT32 tun_dtt75300_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tun_dtt75300_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tun_dtt75300_status(UINT32 tuner_id, UINT8 *lock);

#elif (SYS_TUN_MODULE==DTT7596)
extern INT32 nim_dtt7596_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  nim_dtt7596_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32  nim_dtt7596_status(UINT32 tuner_id, UINT8 *lock);

#elif (SYS_TUN_MODULE==DTF8570)
extern INT32    nim_dtf8570_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  nim_dtf8570_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32  nim_dtf8570_status(UINT32 tuner_id, UINT8 *lock);


#elif (SYS_TUN_MODULE==DTT75411)
extern INT32    nim_dtt75411_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   nim_dtt75411_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   nim_dtt75411_status(UINT32 tuner_id, UINT8 *lock);

#elif (SYS_TUN_MODULE==TDA18211)
extern INT32    nim_tda18211_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   nim_tda18211_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   nim_tda18211_status(UINT8 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==EDT1022B)
extern INT32    tun_edt1022_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_edt1022_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_edt1022_status(UINT32 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==TD1611ALF)
extern INT32 tun_td1611alf_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_td1611alf_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_td1611alf_status(UINT32 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==ED5065)
extern INT32 tun_ed5065_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_ed5065_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_ed5065_status(UINT8 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==QT3010)
extern INT32 tuner_3010_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32  tuner_3010_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32  tuner_3010_status(UINT8 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==TDA18218)
extern INT32    tun_tda18218_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_tda18218_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                    UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_tda18218_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_tda18218_command(UINT32 tuner_id, INT32 cmd, UINT32 param);
#elif (SYS_TUN_MODULE==RADIO3432)
extern INT32 tun_rd3432_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_rd3432_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_rd3432_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                 UINT8 *data, UINT8 _i2c_cmd);
#elif (SYS_TUN_MODULE==MAX3580)
extern INT32 tun_max3580_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_max3580_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                 UINT8 *data, UINT8 _i2c_cmd);
extern INT32 tun_max3580_status(UINT32  tuner_id, UINT8 *lock);
extern INT32 tun_max3580_command(UINT32 tuner_id, INT32 cmd, UINT32 param);
#elif (SYS_TUN_MODULE==ED6092B)
extern INT32 tun_ed6092b_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_ed6092b_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const, \
                                 UINT8 *data, UINT8 _i2c_cmd)    ;
extern INT32 tun_ed6092b_status(UINT32 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==TDA18212)
extern INT32    tun_tda18212_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_tda18212_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                   UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_tda18212_status(UINT32 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==BF6009)
extern INT32    tun_bf6009_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32   tun_bf6009_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                  UINT8 *data, UINT8 _i2c_cmd);
extern INT32   tun_bf6009_status(UINT32 tuner_id, UINT8 *lock);
#elif(SYS_TUN_MODULE==MXL136)
extern INT32 tun_mxl136_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_mxl136_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl136_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                                UINT8 *data, UINT8 _i2c_cmd);
#elif(SYS_TUN_MODULE==NM120)
extern INT32 tun_nm120_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_nm120_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_nm120_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 agc_time_const,\
                               UINT8 *data, UINT8 _i2c_cmd);
#elif(SYS_TUN_MODULE==RT820T)
extern INT32 tun_rt820_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_rt820_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_rt820_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, \
                               UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd);
#elif (SYS_TUN_MODULE==CDT_9VM80)
extern INT32 tun_cdt_9vm80_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_cdt_9vm80_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, \
                                   UINT8 agc_time_const, UINT8 *data, UINT8 _i2c_cmd)    ;
extern INT32 tun_cdt_9vm80_status(UINT32 tuner_id, UINT8 *lock);
#endif
#endif
extern INT32 tun_cxd_ascot3_Init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_cxd_ascot3_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_cxd_ascot3_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_cxd_ascot3_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

/*--- Tuner MAX2118 Callback Functions --- */
extern INT32 nim_max2118_init (UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_max2118_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_max2118_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner IX2410 Callback Functions --- */
extern INT32 nim_ix2410_init(UINT32 *tuner_id,  struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_ix2410_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_ix2410_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner SL1935 Callback Functions --- */
extern INT32 nim_sl1935_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_sl1935_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_sl1935_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner ZL10036 Callback Functions --- */
extern INT32 nim_zl10036_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_zl10036_control(UINT32 tuner_id, UINT32 freq, UINT32 bs);
extern INT32 nim_zl10036_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner ZL10039 Callback Functions --- */
extern INT32 nim_zl10039_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_zl10039_control(UINT32 tuner_id, UINT32 freq , UINT32 sym );
extern INT32 nim_zl10039_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner STB6000 Callback Functions --- */
extern INT32 nim_st6000_init(UINT32 *tuner_id,  struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_st6000_control(UINT32 tuner_id, UINT32 freq, UINT16 sym );
extern INT32 nim_st6000_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner IX2476 Callback Functions --- */
extern INT32 nim_ix2476_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_ix2476_control(UINT32 tuner_id, UINT32 freq, UINT16 bs);
extern INT32 nim_ix2476_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner Tda8262 Callback Functions --- */
extern INT32 tun_tda8262_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_tda8262_control(UINT32 tuner_id, UINT32 freq , UINT32 sym );
extern INT32 tun_tda8262_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_hz6306_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_hz6306_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_hz6306_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_sp7006_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_sp7006_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_sp7006_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7306_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_vz7306_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7306_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7803_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_vz7803_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7803_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_stv6110_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_stv6110_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_stv6110_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_stv6111_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_stv6111_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_stv6111_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_rda5812_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_rda5812_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_rda5812_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_av2011_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_av2011_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_av2011_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_zl10037_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_zl10037_control(UINT32 tuner_id, UINT32 freq , UINT32 sym );
extern INT32 nim_zl10037_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7851_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_vz7851_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7851_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7903_init(UINT32 *tuner_id, struct QPSK_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 nim_vz7903_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7903_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 tun_dct2a_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptr_tuner_config);
extern INT32 tun_dct2a_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 agc_time_const, UINT8 _i2c_cmd);
extern INT32 tun_dct2a_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_cxd2834_attach(char *name, PCOFDM_TUNER_CONFIG_API p_config);
extern INT32 nim_s3281_dvbc_attach(struct QAM_TUNER_CONFIG_API *ptrqam_tuner);

//begin:added by robin.gan on 2015.4.15
extern INT32 nim_m3031_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptr_tuner_config);
extern INT32 nim_m3031_control(UINT32 tuner_id, UINT32 freq, UINT32 bb_sym);
extern INT32 nim_m3031_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 nim_m3031_gain(UINT32 tuner_id, UINT32 demod_agc);
extern INT32 nim_m3031_lt_gain(UINT32 tuner_id, INT32 lt_gain);
extern INT32 nim_m3031_standby(UINT32 tuner_id);
extern INT32 nim_m3031b_command(UINT32 tuner_id, INT32 cmd, INT32 *param);
//end

#ifdef __cplusplus
}
#endif


#endif // _NIM_TUNER_H_
