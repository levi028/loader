/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 specify function for specify hardware
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/


#ifndef __LLD_NIM_DVBS_C3505_PRIVATE_H__
#define __LLD_NIM_DVBS_C3505_PRIVATE_H__

#define CHIP_ID 0x35
#define CHIP_ID_3505 0x350100F0
#define CHIP_ID_3503C 0x350100F1
#define CHIP_ID_3503D 0x350101F2
#define XTAL_CLK 27000



#define NIM_C3505_BASE_ADDR  0xB804C000

  //-End: TAB for New ADPT Add by Hongyu //
//----------------end of defines for CR adaptive----------------
/* 3503 register define for DVBS/DVBS2 */
enum NIM_C3505_REGISTER_ADDRESS
{
	R00_CTRL = 0x00,			// NIM3501 control register 				
	R01_ADC = 0x01,				// ADC Configuration Register
	R02_IERR = 0x02,			// Interrupt Events Register
	R03_IMASK = 0x03,			// Interrupt Mask Register
	R04_STATUS = 0x04,			// Status Register
	R05_TIMEOUT_TRH = 0x05,		// HW Timeout Threshold Register(LSB)
	R07_AGC1_CTRL = 0x07,		// AGC1 reference value register
	R0A_AGC1_LCK_CMD = 0x0a,	// AGC1 lock command register
	R0E_ADPT_CR_CTRL = 0x0e,
	R10_DCC_CFG = 0x10,			// DCC Configure Register
	R11_DCC_OF_I = 0x11,		// DCC Offset I monitor Register
	R12_DCC_OF_Q = 0x12,		// DCC Offset Q monitor Register
	R13_IQ_BAL_CTRL = 0x13,		// IQ Balance Configure Register
	R15_FLT_ROMINDX = 0x15,		// Filter Bank Rom Index Register
	R16_AGC2_REFVAL = 0x16,		// AGC2 Reference Value Register
	R17_AGC2_CFG = 0x17,		// AGC2 configure register
	R18_TR_CTRL = 0x18,			// TR acquisition gain register
	R1B_TR_TIMEOUT_BAND = 0x1b,	// TR Time out band register
	R21_BEQ_CTRL = 0x21,		// BEQ Control REgister
	R22_BEQ_CMA_POW = 0x22,		// BEQ CMA power register
	R24_MATCH_FILTER = 0x24,	// Match Filter Register
	R25_BEQ_MASK = 0x25,		// BEQ Mask Register
	R26_TR_LD_LPF_OPT = 0x26,	// TR LD LPF Output register
	R28_PL_TIMEOUT_BND = 0x28,	// PL Time out Band REgister
	R2A_PL_BND_CTRL = 0x2a,		// PL Time Band Control
	R2E_PL_ANGLE_UPDATE = 0x2e,	// PL Angle Update High/Low limit register
	R30_AGC3_CTRL = 0x30,		// AGC3  Control Register
	R33_CR_CTRL = 0x33,			// CR DVB-S/DVBS-S2  CONTROL register
	R3D_CR_CTRL = 0x3d,			// CR DVBS-S2  Time Band	Acq
	R3E_CR_CTRL = 0x3e,			// CR DVBS-S2  Time Band	Mid
	R43_CR_OFFSET_TRH = 0x43,   // CR Feedback Carrier Threshold register
	R45_CR_LCK_DETECT = 0x45,	// CR lock detecter lpf monitor register
	R47_HBCD_TIMEOUT = 0x47,	// HBCD Time out band register
	R48_VITERBI_CTRL = 0x48,	// Viterbi module control register
	R54_VITERBI_FRAME_SYNC=0x54,
	R57_PKT_CAP = 0x57,		    // DMA packet capture control register
	R59_SWEEP_RANG_REG=0x59,    //Symbol Rate sweep range limitation enable
	R5B_ACQ_WORK_MODE = 0x5b,	// Acquiescent work mode register
	R5C_ACQ_CARRIER = 0x5c,		// Acquiescent carrier control register
	R5F_ACQ_SYM_RATE = 0x5f,	// Acquiescent symbol rate register
	
	R62_FC_SEARCH = 0x62,		// FC Search Range Register
	R64_RS_SEARCH = 0x64,		// RS Search Range Register
	R66_TR_SEARCH = 0x66,		// TR Search Step register
	R67_VB_CR_RETRY = 0x67,		// VB&CR Maximum Retry Number Register
	R68_WORK_MODE = 0x68,		// Work Mode Report Register
	R69_RPT_CARRIER = 0x69,		// Report carrier register
	R6C_RPT_SYM_RATE = 0x6c,	// report symbol rate register
	R6F_FSM_STATE = 0x6f,		// FSM State Moniter Register
	R70_CAP_REG = 0x70,
	R71_CAP_RDY = 0x71,			// Capture Param register
	R72_CAP_REG = 0x72,			// Capture Param register
	R74_PKT_STA_NUM = 0x74,		// Packet Statistic Number Register
	R76_BIT_ERR = 0x76,			// Bit Error Register
	R79_PKT_ERR = 0x79,			// Packet Error Register
	R7B_TEST_MUX = 0x7b,		// Test Mux Select REgister
	R7C_DISEQC_CTRL = 0x7c,		// DISEQC Control Register
	R86_DISEQC_RDATA = 0x86,	// Diseqc data for read
	R8E_DISEQC_TIME = 0x8e,		// Diseqc time register
	R90_DISEQC_CLK_RATIO = 0x90,// Diseqc clock ratio register
	R97_S2_FEC_THR = 0x97,		// S2 FEC Threshold register
	R98_S2_FEC_FAIL_THR = 0x98, // S2 FEC FAIL Threshold Register
	R99_H8PSK_THR = 0x99,		// H8PSK CR Lock Detect threshold register
	R9C_DEMAP_BETA = 0x9c,		// Demap Beta register
	R9D_RPT_DEMAP_BETA = 0x9d, // Report demap beta value / CR Table addr
	RA0_RXADC_REG = 0xa0,		// RXADC ANATST/POWER register
	RA3_CHIP_ID = 0xa3,			// Chip ID REgister
	RA5_VER_ID = 0xa5,			// version ID register
	RA6_VER_SUB_ID = 0xa6,			// version sub ID register
	RA7_I2C_ENHANCE = 0xa7,		// I2C Enhance Register
	RA8_M90_CLK_DCHAN = 0xa8,	// M90 clock delay chain register
	RA9_M180_CLK_DCHAN = 0xa9,	// M180 Clock delay chain register
	RAA_S2_FEC_ITER = 0xaa,		// S2 FEC iteration counter register
	RAB_CHIP_SUB_ID = 0xab,		// S2 FEC iteration counter register	
    RAC_S2_FEC_RPT = 0xac,     // S2 FEC RPT BCH Status  
	RAD_TSOUT_SYMB = 0xad,		// ts  out setting SYMB_PRD_FORM_REG
	RAF_TSOUT_PAD = 0xaf,		// TS out setting and pad driving register
	RB0_PLL_CONFIG = 0xb0,		// PLL configure REgister
	RB1_TSOUT_SMT = 0xb1,		// TS output Setting and Pad driving
	RB3_PIN_SHARE_CTRL = 0xb3,	// Pin Share Control register
	RB4_BEQ_CFG = 0xb4,
	RB5_CR_PRS_TRA = 0xb5,		// CR DVB-S/S2 PRS in Tracking State
	RB6_H8PSK_CETA = 0xb6,		// H8PSK COS/SIN Ceta Value Register
	RB8_LOW_RS_CLIP = 0xb8,		// Low RS Clip Value REgister 	
	RBA_AGC1_REPORT = 0xba,		// AGC1 report register
	RBB_SNR_RPT1 = 0xbb,
	RBC_SNR_RPT2 = 0xbc,
	RBD_CAP_PRM = 0xbd,			// Capture Config/Block register
	RBF_S2_FEC_DBG = 0xbf,		// DVB-S2 FEC Debug REgister
	RC0_BIST_LDPC_REG = 0xc0,	// LDPC Average Iteration counter register
	RC1_DVBS2_FEC_LDPC = 0xc1,	// DVBS2 FEC LDPC Register
	RC6_DVBS2_FEC_LDPC = 0xc6,  // LDPC exit control 
	RC8_BIST_TOLERATOR = 0xc8,	// 0xc0	Tolerator MBIST register
	RC9_CR_OUT_IO_RPT = 0xc9,	// Report CR OUT I Q
	// for s3501B
	RCB_I2C_CFG = 0xcb,			// I2C Slave Configure Register
	RCC_STRAP_PIN_CLOCK = 0xcc,	// strap pin and clock enable register
	RCD_I2C_CLK = 0xcd,			// I2C AND CLOCK ENABLE REGISTER
	RCE_TS_FMT_CLK = 0xce,		// TS Format and clock enable register
	RD0_DEMAP_NOISE_RPT = 0xd0,	// demap noise rtp register
	RD3_BER_REG = 0xd3,			// BER register
	RD6_LDPC_REG = 0xd6,		// LDPC register
	RD7_EQ_REG = 0xd7,			// EQ register
	RD8_TS_OUT_SETTING = 0xd8,	// TS output setting register
	RD9_TS_OUT_CFG = 0xd9,		// BYPASS register
	RDA_EQ_DBG = 0xda,			// EQ Debug Register
	RDC_EQ_DBG_TS_CFG = 0xdc,	// EQ debug and ts config register
	RDD_TS_OUT_DVBS = 0xdd,		// TS output dvbs mode setting
	RDF_TS_OUT_DVBS2 = 0xdf,	// TS output dvbs2 mode setting
	RE0_PPLL_CTRL = 0xe0,
	RED_CR_CTRL = 0xed,	
	RF0_HW_TSO_CTRL = 0xf0,
	RF1_DSP_CLK_CTRL = 0xf1,
	RF8_MODCOD_RPT = 0xf8,
	RFA_RESET_CTRL = 0xfa,
	RFC_TSO_CLK_SW_CFG = 0xfc,
	RFF_TSO_CLS = 0xff,
	R100_SWFID = 0x100,        // soft config frame ID
    R104_BBF = 0x104,          // For caputre bb header
    R105_BBF = 0x105,          // For caputre bb header
	R106_BBF_CFG = 0x106,
	R109_BBF_REG = 0x109,
	R10b_ISI_SET = 0x10b,
	R113_NEW_CR_ADPT_CTRL = 0x113,
	R114_DISEQC_TIME_SET = 0x114,
    R11C_MAP_IN_I=0x11c,
    R122_S2_FEC_FSM = 0x122,    
	R124_HEAD_DIFF_NFRAME = 0x124,
    R12d_LDPC_SEL = 0x12d,
	R130_CR_PARA_DIN = 0x130,
	R13b_EST_NOISE = 0x13b,
	R13d_ADPT_CR_PARA_0 = 0x13d,
	R140_ADPT_CR_PARA_1 = 0x140,
	R144_ADPT_CR_PARA_2 = 0x144,
	R159_PLDS_SW_INDEX = 0x159,
	R183_CCI_PLL_R_SEL= 0x183,
	R184_CCI_DIRECT_BYPASS = 0x184,
	R18E_CCI_REPORT_FREQ= 0x18e,	
	R18F_CCI_REPORT_LOCK= 0x18f,	
	R190_CCI_REPORT_LEVEL= 0x190,
	R1C0_DEMAP_FLT_MODU_TYPE=0x1c0,
	R1C1_DEMAP_FLT_FRAME_PILOT=0x1c1,
	R1C2_DEMAP_FLT_CODE_RATE=0x1c2,
	R1C3_DEMAP_FLT_CODE_RATE =0x1c3,
	R15C_DISEQC_CMD_QUE_IN = 0x15c,
	R15D_DISEQC_CMD_QUE_CTL = 0x15d,
	R15E_DISEQC_CMD_QUE_RPT = 0x15e,
	R197_LA_TRG_MASK = 0x197,
	R199_LA_TRG_TAR = 0x199,
	R19B_LA_TRG_PARA = 0x19b,
	R19C_LA_TRG_CMD = 0x19c,
	R19D_LA_TRG_TIM = 0x19d,
	R1A7_SNR_ESTIMATOR = 0x1A7,
	R1BE_PL_SYNC_CFG = 0x1be, 
	R1BF_PL_SYNC_CFG = 0x1bf,
	
	R1C4_DVBS_FEC_CFG = 0x1c4,
};

//------------------------DEFINE for Work Mode--------------------------------//
//#define REPORT_FAIL_TP                  // If open this macro, driver may be report "fail TP" to upper which can not locked. paladin 20160614
//#define HW_ADPT_CR               			// when print, need to open
//#define RPT_CR_SNR_EST         			// print OldAdpt EST_NOISE value
//#define ACM_WORK_MODE


#define CRC_THRESHOLD 5
#define TIMEOUT_THRESHOLD 20

#define NIM_CAPTURE_SUPPORT    				// capture demod internal data 
#define ADC2DRAM_SUPPORT       				// capture ADC data to DRAM in autoscan 
#define DISEQC_DELAY		60				//adjust timing for DiSEqC circuit
#define IMPLUSE_NOISE_IMPROVED              // For impluse noise performance improved

//#define DISEQC_OUT_INVERT      			// DISEQC for Special Case
//#define ANTI_WIMAX_INTF					// Anti-WIMAX patch    
//#define NEW_PLSYNC_ENABLE			    	// New plsync using
//----------------------End of DEFINE for Work Mode-----------------------------//
#define FORCE_WORK_MODE_IN_CHANGE_TP


#ifdef ACM_WORK_MODE
    //#define FIND_ACM_TP
    #define ACM_RECORD                  // For record stream in acm mode
    //#define ACM_RESERVE_FUNCTION        // For channel_change/autoscan in acm mode ,no use now
    #define ACM_CCM_FLAG NIM_OPTR_ACM	
    #define NEW_PLSYNC_ENABLE	
#else
    #define ACM_CCM_FLAG NIM_OPTR_CCM 
#endif

#ifdef ANTI_WIMAX_INTF
extern UINT8 force_adpt_disable;
#endif

#define BYPASS_BUF_SIZE_DMA 0x10000  //64K bytes, size of ADC2MEM block in unit BYTE
#define RET_CONTINUE    0xFF

#define	FFT_BITWIDTH			10
#define	STATISTIC_LENGTH		2
#define FS_MAXNUM 					15000
#define TP_MAXNUM 					2000

#define SNR_MERGE 30
#define SNR_ERR_MIN -300
#define PHASE_ERR_MIN -60
#define PHASE_ERR_MAX 60
#define PHASE_ERR_AVG_NUM 10
#define PHASE_ERR_AVG_ERR_MAX 20
#define PHASE_ERR_AVG_ERR_MIN -20


//average length of data to determine threshold
//0:2;1:4;2:8

#define	MAX_CH_NUMBER			32//maximum number of channels that can be stored
#define C3505_LOACL_FREQ   		5150
#define QPSK_TUNER_FREQ_OFFSET	3   // 4
#define C3505_IQ_AD_SWAP   		0x04
#define C3505_QPSK_FREQ_OFFSET 	0x01
#define C3505_NEW_AGC1  		0x20
#define C3505_POLAR_REVERT  	0x10
#define C3505_1BIT_MODE 		0x00 
#define C3505_2BIT_MODE 		0x40
#define C3505_4BIT_MODE 		0x80
#define C3505_8BIT_MODE 		0xc0
#define C3505_AGC_INVERT		0x100
#define C3505_USE_188_MODE		0x400
#define C3505_DVBS_MODE			0x00	
#define C3505_DVBS2_MODE		0x01
#define C3505_SIGNAL_DISPLAY_LIN	0x800
#define C3505_NEW_PLSYNC        0x00
#define C3505_OLD_PLSYNC        0x01
#define NIM_FREQ_RETURN_REAL      0
#define NIM_FREQ_RETURN_SET       1
#define NIM_TUNER_SET_STANDBY_CMD	0xffffffff
#define BYPASS_BUF_SIZE 			0x20000     // 128 KB
#define BYPASS_BUF_MASK 			(BYPASS_BUF_SIZE - 1)

#define S3501_FREQ_OFFSET 1
#define C3505_LOW_SYM_THR 6500  // less than 6.5M is low symble TP
#define LNB_LOACL_FREQ 5150
#define AS_FREQ_MIN 900
#define AS_FREQ_MAX 2200

#define NIM_OPTR_CHL_CHANGE0		0x70
#define NIM_OPTR_CHL_CHANGE			0x00
#define NIM_OPTR_SOFT_SEARCH		0x01
#define NIM_OPTR_FFT_RESULT			0x02
#define NIM_OPTR_DYNAMIC_POW		0x03
#define NIM_OPTR_DYNAMIC_POW0		0x73
#define NIM_OPTR_IOCTL				0x04
#define NIM_OPTR_HW_OPEN			0x05
#define NIM_OPTR_HW_CLOSE			0x06

#define NIM_DEMOD_CTRL_0X50			0x50
#define NIM_DEMOD_CTRL_0X51			0x51
#define NIM_DEMOD_CTRL_0X90			0x90
#define NIM_DEMOD_CTRL_0X91			0x91
#define NIM_DEMOD_CTRL_0X02			0x02
#define NIM_DEMOD_CTRL_0X52			0x52

#define NIM_SIGNAL_INPUT_OPEN		0x01
#define NIM_SIGNAL_INPUT_CLOSE		0x02

#define NIM_FRZ_AGC1_OPEN		    0x01
#define NIM_FRZ_AGC1_CLOSE   		0x02

#define NIM_LOCK_STUS_NORMAL		0x00
#define NIM_LOCK_STUS_SETTING		0x01
#define NIM_LOCK_STUS_CLEAR			0x02

#define NIM_FLAG_CHN_CHG_START		(1<<0)
#define NIM_FLAG_CHN_CHANGING		(1<<1)
#define NIM_SWITCH_TR_CR			0x01
#define NIM_SWITCH_RS				0x02
#define NIM_SWITCH_FC				0x04
#define NIM_SWITCH_HBCD				0x08

#define TS_DYM_HEAD0 				0x47
#define TS_DYM_HEAD1 				0x1f
#define TS_DYM_HEAD2 				0xff
#define TS_DYM_HEAD3 				0x10
#define TS_DYM_HEAD4 				0x00

#define M3031_AGC_OFFSET 			0x20

#define NIM_RPT_PLL0  				0x01
#define NIM_RPT_PLL1  				0x02
#define NIM_RPT_PLL2  				0x04

#define NIM_CCI_CANCEL				0x08
#define NIM_CCI_BYPASS				0x0a

 //-------------------defines for ACM/CCM/AUTO-------------------
#define NIM_OPTR_ACM                 0x00
#define NIM_OPTR_CCM                 0x01
#define NIM_OPTR_AUTO                0x03
//--------------------defines for capture operation------------------
#define NIM_CAP_ADC					 0x01
#define NIM_CAP_BIST0                0x02//counter by 1
#define NIM_CAP_BIST1                0x03//PN seqence
#define NIM_CAP_FEC                  0X04
#define NIM_CAP_PAT0                 0x05//	DMA_BB_L3
#define NIM_CAP_PAT1                 0x06// DMA_BB_PURE_L3
#define NIM_CAP_PAT2                 0x07// DMA_BB_ALI
#define NIM_CAP_TS1					 0x08// TS_BB_L3
#define NIM_CAP_TS2					 0x09// TS_BB_ALI

 //------------------------ defines for CR adaptive ---------------------------------
#define SNR_TAB_SIZE	19 // for SI, use macro instead of enum
#define PSK8_TAB_SIZE	14
#define QPSK_TAB_SIZE	3
#define APSK16_TAB_SIZE	11 

//----------------------------DEFINE for variable---------------------------------//
//extern UINT32  va_ber;
//extern UINT32  va_ber_window;
//extern UINT32  va_mer;
//extern UINT32  va_mer_window;
//extern UINT8   va_22k;
extern const UINT8 c3505_map_beta_active_buf_normal[];
extern const UINT8 c3505_map_beta_active_buf_short[];
extern const UINT8 c3505_map_beta_buf_normal[];
extern const UINT8 c3505_map_beta_buf_short[];
extern const UINT16 c3505_demap_noise_normal[];
extern const UINT16 c3505_demap_noise_short[];
extern const UINT8 c3505_map_beta_normal_adaptive_en[];
extern const UINT8 c3505_map_beta_short_adaptive_en[];
extern const UINT8 c3505_map_beta_buf_normal_extra[32][3];
extern const UINT16 c3505_map_beta_normal_snr_thres[32][2];
extern const UINT8 c3505_snr_tab[];
extern const UINT8 c3505_snr_dbx10_tab[];
extern const UINT16 c3505_snr_thr[];
extern const UINT32 c3505_cr_para_8psk_3f5[];
extern const UINT32 c3505_cr_para_8psk_others[];
extern const UINT16 c3505_s2_awgn_noise[];
extern const UINT16 c3505_s2_awgn_noise_extra[];
extern const UINT8 c3505_s2_awgn_coef_set[];
extern const UINT8 c3505_s2_awgn_coef_set_extra[];
extern const UINT8 c3505_s2_pon_irs_delta[];
extern const UINT8 c3505_s2_pon_irs_delta_extra[];
extern const UINT8 c3505_s2_pon_prs_delta[];
extern const UINT8 c3505_s2_pon_prs_delta_extra[];
extern const UINT8 c3505_s2_loop_coef_set[];
extern const UINT8 c3505_s2_loop_coef_extra[];
extern const UINT8 c3505_s2_clip_ped_set[];
extern const UINT8 c3505_s2_clip_ped_set_extra[];
extern const UINT8 c3505_s2_avg_ped_set[];
extern const UINT8 c3505_s2_avg_ped_set_extra[];
extern const UINT8 c3505_s2_force_old_cr[];
extern const UINT8 c3505_s2_llr_shift[];
extern const UINT8 c3505_symbol_before_head[];
extern const UINT8 c3505_head_ped_gain[];
extern const UINT8 c3505_prs_fraction[];
extern const UINT8 c3505_prs[];
extern const UINT8 c3505_irs_fraction[];
extern const UINT8 c3505_irs[];
extern const UINT8 c3505_symbol_before_head_extra[];
extern const UINT8 c3505_head_ped_gain_extra[];
extern const UINT8 c3505_prs_fraction_extra[];
extern const UINT8 c3505_prs_extra[];
extern const UINT8 c3505_irs_fraction_extra[];
extern const UINT8 c3505_irs_extra[];
extern const UINT8 c3505_wider_prs_delta[];
extern const UINT8 c3505_wider_irs_delta[];
extern const UINT8 c3505_wider_snr_delta[];
extern const UINT8 c3505_prs_step[];
extern const UINT8 c3505_irs_step[];
extern const UINT8 c3505_max_snr_delta[];
extern const UINT8 c3505_head_gain_snr_delta[];
extern const UINT8 c3505_clip_ped2[];
extern const UINT8 c3505_clip_ped2_en[];
extern const UINT8 c3505_clip_ped1[];
extern const UINT8 c3505_clip_ped1_en[];
extern const UINT8 c3505_clip_mult_step[];
extern const UINT8 c3505_avg_ped2[];
extern const UINT8 c3505_avg_ped2_en[];
extern const UINT8 c3505_avg_ped1[];
extern const UINT8 c3505_avg_ped1_en[];
extern const UINT8 c3505_avg_mult_step[];
extern const UINT8 c3505_avg_snr_delta[];
extern const UINT16 c3505_s_awgn_noise[];
extern const UINT8 c3505_s_awgn_coef_set[];
extern const UINT8 c3505_s_pon_irs_delta[];
extern const UINT8 c3505_s_pon_prs_delta[];
extern const UINT8 c3505_s_loop_coef_set[];
extern const UINT8 c3505_s_clip_ped_set[];
extern const UINT8 c3505_s_avg_ped_set[];
extern const UINT8 c3505_s_force_old_cr[];
extern const UINT8 c3505_s_llr_shift[];
extern INT32 call_tuner_command(struct nim_device *dev, INT32 cmd, INT32 *param);

extern const UINT8 ADC_SAMPLE_FREQ[];

INT32 nim_c3505_set_32apsk_target(struct nim_device *dev);
INT32 nim_c3505_set_dynamic_power(struct nim_device *dev, UINT8 snr);
INT32 nim_c3505_set_ldpc_iter_para(struct nim_device *dev, UINT16 most_cnt, UINT8 unchg_thld0, UINT8 unchg_thld1, UINT8 pass_thld);
INT32 nim_c3505_set_ldpc_iter_cnt_record_last(struct nim_device *dev);
INT32 nim_c3505_set_ldpc_iter_cnt_record_max(struct nim_device *dev);
INT32 nim_c3505_ldpc_keep_working_enable(struct nim_device *dev);
INT32 nim_c3505_ldpc_keep_working_disable(struct nim_device *dev);
INT32 nim_c3505_tso_soft_cbr_on (struct nim_device *dev, UINT32 bitrate);
INT32 nim_c3505_tso_soft_cbr_off (struct nim_device *dev);
INT32 nim_c3505_pl_sync_cfg(struct nim_device *dev, UINT8 pl_sync_type);


#endif// __LLD_NIM_DVBS_C3505_PRIVATE_H__ */
