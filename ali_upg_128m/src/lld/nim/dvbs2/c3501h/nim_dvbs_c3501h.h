/*****************************************************************************
*	Copyright (C)2017 Ali Corporation. All Rights Reserved.
*
*	File:	This file contains C3501H DVBS2X system API and hardware 
*           open/closed operate in LLD.
*
*    Description:    
*    History:
*   Date			Athor			Version		Reason
*   ============    =========   	=======   	=================
* 1.  01/03/2017	Paladin.Ye     	Ver 1.0   	Create file for C3501H DVBS2X project
*******************************************************************************/

#ifndef __LLD_DVBS_C3501H_H__
#define __LLD_DVBS_C3501H_H__

#define __TDS__

#ifndef FPGA_PLATFORM

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libdiseqc/lib_diseqc.h>

#else


#include "../nim.h"
#include "../nim_dev.h"
#include "../../tuner/nim_tuner.h"
#include "../../basic/global.h"

#include "nim_dvbs_c3501h_tds.h"
#endif

#ifdef FPGA_PLATFORM
#define osal_get_tick GetTickCount
#define OSAL_INVALID_ID 0xffff
#endif

#define nim_reg_read nim_c3501h_reg_read
#define nim_reg_write nim_c3501h_reg_write

// For CSTM used
//#define FOR_SKEW_LNB_TEST_QPSK
//#define FOR_SKEW_LNB_TEST_8PSK

/////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505.h
////////////////////////////////////////////////////////////////////////////////////////////

// added by gloria 
#define ERR_CRC_HEADER    43
#define XTAL_CLK_27M 27000
#define XTAL_CLK_13_5M 13500

#define COMPEN_MULTISWITCH_DELAY 800    // For some multiswitch,  it need more time to make signal steady. Unit is ms
#define FORCE_WORK_MODE_IN_CHANGE_TP

#ifdef FPGA_PLATFORM
#define _SHIFT_3M_
#endif

//#define FOR_QT_IC_TEST	// Only for qt ic test. 1, Increase PLSN Search timeout 2, No limited for frequency search range
#define NIM_TS_PORT_CAP
//#define ES_TEST
#define I2C_RETRY

//#define SW_TSO_CLK_CFG  // For acm normal/short frame mix stream
//#define I2C_THROUGH_ALWAYS_OPEN
//#define _CR_IMPULSE_IMPROVE_

#ifndef FOR_QT_IC_TEST
	#define FC_SEARCH_RANGE_LIMITED
#endif

#define IMPLUSE_NOISE_IMPROVED
#define SNR_MERGE 30

//#define UNKNOW_FUNCTION
//#define SDK_FPGA_TEST
#define AUTOSCAN_SPEEDUP
#define RS_LIMITED 1000

// When in small RS case,TP can lock but can not search program.The rootcause is it needs much more time to get BB_EN after get LOCK.
// Paladin used [comm_sleep] mechanism to solve the problem early. sleep for  a suitable time to get BB_EN = 1 28/10/2017
// Bentao try to get the BB_EN signal  via  LA  08/01/2018
// this Macro is defined by gloria for different way to get BB_EN=1     08/01/2018
#define GET_BBH_SOLUTION

// bit[7:0]  7 6 5 4 3 2 1 0
// mask:0x31 0 0 1 1 0 0 0 1 , means used method 0, 4, 5
//#define PLSN_SEARCH_ALGO_MASK 0x21
#define PLSN_SEARCH_ALGO_MASK 0x31

#define PLSN_SEARCH_ALGO_NUM 6

#define C3501H_ERR_I2C_NO_ACK	ERR_I2C_NO_ACK
#define T_CTSK					OSAL_T_CTSK


#define NIM_SIGNAL_INPUT_OPEN        0x01
#define NIM_SIGNAL_INPUT_CLOSE    0x02



//extern INT32 snr_initial_en;//Seen, 1 means first snr estimation done, set 1 in channel change
//INT32 snr_initial_en;//Seen, 1 means first snr estimation done, set 1 in channel change


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_tds.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//----------------------------DEFINE for debug---------------------------------//
//#define HW_ADPT_CR_MONITOR 		// print OldAdpt parameters  
//#define HW_ADPT_NEW_CR_MONITOR	// print CR coefficients, no matter OldAdpt or NewAdpt
//#define NEW_CR_ADPT_SNR_EST_RPT 	// print c/n dB value
//#define SW_ADPT_CR					
//#define SW_SNR_RPT_ONLY

#define PLSN_262141_PATCH

//#define C3501H_DEBUG_FLAG                // Debug signal issue and so on
//#define DEBUG_SOFT_SEARCH      		// Try someone TP for special case, for debug
//#define USE_CHANNEL_CHANGE_PARA       // Use channel change config in soft search, for debug
//#define DEBUG_CHANNEL_CHANGE      	// Try Get current TP spectrum
#ifdef C3501H_DEBUG_FLAG
	#define NIM_PRINTF_DEBUG
    #define MON_PRINTF_DEBUG
    #define FFT_PRINTF_DEBUG
    #define NIM_GET_1024FFT
	#define ERR_DEBUG
	#define AUTOSCAN_DEBUG
	#define ACM_PRINTF
    #define PLSN_DEBUG
	#define TSN_DEBUG
    #define DEBUG_REGISTER                        // Monitor register value
    #define DEBUG_IN_TASK
#endif

#ifdef FPGA_PLATFORM
INT32 nim_c3501h_attach(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);  

#ifdef MON_PRINTF_DEBUG
	#define MON_PRINTF  NIM_PRINTF
#else
    #define MON_PRINTF(...)
#endif


#ifdef ERR_DEBUG
    #define ERR_PRINTF  NIM_PRINTF
#else
	#define ERR_PRINTF(...)
#endif

#ifdef FFT_PRINTF_DEBUG
    #define FFT_PRINTF  NIM_PRINTF
#else
	#define FFT_PRINTF(...)
#endif

#ifdef HW_ADPT_CR_MONITOR
    #define ADPT_CR_PRINTF  NIM_PRINTF
#else
	#define ADPT_CR_PRINTF(...)
#endif

#ifdef HW_ADPT_NEW_CR_MONITOR
    #define ADPT_NEW_CR_PRINTF  NIM_PRINTF
#else
	#define ADPT_NEW_CR_PRINTF(...)
#endif

#ifdef AUTOSCAN_DEBUG
    #define AUTOSCAN_PRINTF  NIM_PRINTF
#else
	#define AUTOSCAN_PRINTF(...)
#endif

#ifdef ACM_PRINTF
	#define ACM_DEBUG_PRINTF  NIM_PRINTF
#else
	#define ACM_DEBUG_PRINTF(...)	
#endif

#define SEARCH_PLSN_PRINTF  NIM_PRINTF

#else

#ifdef NIM_PRINTF_DEBUG
	#define NIM_PRINTF(fmt,args...)  libc_printf("NIM_PRINTF: " fmt, ##args)    
#else
    #define NIM_PRINTF(...)
#endif


#ifdef MON_PRINTF_DEBUG
	#define MON_PRINTF(fmt,args...)  libc_printf("NIM_PRINTF: " fmt, ##args)    
#else
    #define MON_PRINTF(...)
#endif


#ifdef ERR_DEBUG
    #define ERR_PRINTF(fmt, args...)  libc_printf("ERR: " fmt, ##args)
#else
	#define ERR_PRINTF(...)
#endif

#ifdef FFT_PRINTF_DEBUG
    #define FFT_PRINTF(fmt, args...)  libc_printf("FFT: " fmt, ##args)
#else
	#define FFT_PRINTF(...)
#endif

#ifdef HW_ADPT_CR_MONITOR
    #define ADPT_CR_PRINTF(fmt, args...)  libc_printf("ADPT_CR_PRINTF: " fmt, ##args)
#else
	#define ADPT_CR_PRINTF(...)
#endif

#ifdef HW_ADPT_NEW_CR_MONITOR
    #define ADPT_NEW_CR_PRINTF(fmt, args...)  libc_printf("ADPT_NEW_CR_PRINTF: " fmt, ##args)
#else
	#define ADPT_NEW_CR_PRINTF(...)
#endif

#ifdef AUTOSCAN_DEBUG
    #define AUTOSCAN_PRINTF(fmt, args...)  libc_printf("AUTOSCAN_PRINTF: " fmt, ##args)
#else
	#define AUTOSCAN_PRINTF(...)
#endif

#ifdef ACM_PRINTF
	#define ACM_DEBUG_PRINTF(fmt, args...)  libc_printf("ACM_PRINTF: " fmt, ##args)
#else
	#define ACM_DEBUG_PRINTF(...)	
#endif

#ifdef PLSN_DEBUG
    #define PLSN_PRINTF(fmt, args...)  libc_printf("PLSN_PRINTF: " fmt, ##args)
#else
    #define PLSN_PRINTF(...)	
#endif

#ifdef TSN_DEBUG
    #define TSN_PRINTF(fmt, args...)  libc_printf("TSN_PRINTF: " fmt, ##args)
#else
    #define TSN_PRINTF(...)	
#endif

#endif

#ifndef FPGA_PLATFORM
#define CHANNEL_CHANGE_ASYNC
#endif

// for multistream search tp
#define ERR_SET_FLAG -1;    
#define ERR_CLR_FLAG -2;

// use mutex
#define NIM_MUTEX_CREATE	osal_semaphore_create
#define NIM_MUTEX_DELETE	osal_semaphore_delete
#define NIM_MUTEX_ENTER(priv)  \
	if(priv->ul_status.nim_c3501h_sema != OSAL_INVALID_ID) \
	{ \
		do{ \
			priv->ul_status.ret= osal_semaphore_capture(priv->ul_status.nim_c3501h_sema, 1000); \
			if(priv->ul_status.ret!= OSAL_E_OK) \
				{NIM_PRINTF("nim_c3501h_sema[%d] capture timeout, retry...\n", \
					priv->ul_status.nim_c3501h_sema);} \
		}while(priv->ul_status.ret != OSAL_E_OK); \
	}

#define NIM_MUTEX_LEAVE(priv) \
	if(priv->ul_status.nim_c3501h_sema != OSAL_INVALID_ID) \
	{ \
		osal_semaphore_release(priv->ul_status.nim_c3501h_sema); \
	} 

// use flag
#ifdef CHANNEL_CHANGE_ASYNC
#define NIM_FLAG_CREATE osal_flag_create
#define NIM_FLAG_DEL osal_flag_delete
#define NIM_FLAG_WAIT osal_flag_wait
#define NIM_FLAG_SET osal_flag_set
#define NIM_FLAG_CLEAR osal_flag_clear
#endif

//move to basic_type.h
#ifndef FPGA_PLATFORM
extern void nim_comm_delay(UINT32 us);
extern void nim_comm_sleep(UINT32 ms);

extern INT32 call_tuner_command(struct nim_device *dev, INT32 cmd, INT32 *param);
#define comm_malloc 			MALLOC
#define comm_memset				MEMSET
#define comm_free				FREE
#define comm_delay				nim_comm_delay
#define comm_sleep				nim_comm_sleep
#define comm_memcpy				memcpy
#endif

#ifdef __LINUX__
typedef struct NIM_AUTO_SCAN         NIM_AUTO_SCAN_T;
typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;
#define OSAL_ID UINT32
#define ID WORD
#define ER long 
#endif

#ifdef FPGA_PLATFORM
typedef WORD ID;
typedef long               ER;
typedef INT32  OSAL_ER;
typedef UINT32 OSAL_ID;
#endif

struct nim_c3501h_tsk_status
{
	UINT32 m_lock_flag;
	ID m_task_id;
	UINT32 m_sym_rate;
	UINT8 m_work_mode;
	UINT8 m_map_type;
	UINT8 m_code_rate;
	UINT8 m_info_data;  // lock status 1=lock, 0=unlock
};

struct nim_c3501h_t_param
{
	UINT32 t_reg_setting_switch;
	UINT8 t_i2c_err_flag;
    UINT8 auto_adaptive_cnt;
    UINT8 auto_adaptive_state;
    UINT8 auto_adaptive_doing;
    INT32 freq_offse_rpt;  
    INT32 freq_search_range;
};


struct nim_c3501h_status
{
	ID nim_c3501h_sema;
	ER ret;
	UINT8 c3501h_autoscan_stop_flag;
	UINT8 c3501h_chanscan_stop_flag;
	UINT32 old_ber;
	UINT32 old_per;
	UINT32 old_ldpc_ite_num;
	UINT8 *adc_data;// = (unsigned char *)__MM_DMX_FFT_START_BUFFER;//[2048];
	UINT8 *adc_data_malloc_addr;
	UINT8 *adc_data_raw_addr;
	INT32 m_freq[256];
	UINT32 m_rs[256];
	INT32 fft_i_1024[1024];
	INT32 fft_q_1024[1024];
	UINT8 m_cr_num;
	UINT32 m_cur_freq;
	UINT8 c_rs ;
	UINT32 m_step_freq;
	pfn_nim_reset_callback m_pfn_reset_c3501h;
	UINT8 m_enable_dvbs2_hbcd_mode;
	UINT8 m_dvbs2_hbcd_enable_value;
	UINT32 check_lock_time; // Used to indicate how long the get_lock_check_limited function has not been called, this function need call every 500ms least
	UINT8 lock_status;   // 0:softsearch, 1: channel change tp lock and freq offset in rangeopen tso, 2: channel change tp lock but freq offset over range close tso, 3: channel change tp unlock
	UINT32 phase_err_check_status;
	UINT32 m_c3501h_type;
    UINT32 m_c3501h_strap_bond_info;
	UINT32 m_c3501h_sub_type;
	UINT32 m_setting_freq;
    UINT32 m_real_tun_freq;
    UINT32 m_real_freq;
	UINT32 m_err_cnts;
	UINT16 m_hw_timeout_thr;
};



#ifdef __TDS__
#define NIM_FLAG_LOCK UINT32
#endif


struct nim_c3501h_private
{
	INT32	 (*nim_tuner_init) (UINT32 *, struct QPSK_TUNER_CONFIG_EXT *);	 // Tuner Initialization Function
	INT32	 (*nim_tuner_control) (UINT32, UINT32, UINT32);  // Tuner Parameter Configuration Function
	INT32	 (*nim_tuner_status) (UINT32, UINT8 *);  
	/*C3031B Tuner special api function,add by dennis on 20140905*/
	//INT32	 (*nim_tuner_command)(UINT32 , INT32 , INT32 );
	INT32	 (*nim_tuner_command)(UINT32 , INT32 , INT32 *);//clean warning
	INT32	 (*nim_tuner_gain)(UINT32,UINT32);
	int (*nim_tuner_close) ();
	struct   QPSK_TUNER_CONFIG_DATA tuner_config_data;
	UINT32 tuner_index;
	UINT32 tuner_id;	    //current Tuner type. 
	UINT32 tuner_type;  	// 3031 or not 3031  
	UINT32 i2c_type_id;
	UINT32 polar_gpio_num;
	UINT32 sys_crystal;
	UINT32 sys_clock;
	UINT16 pre_freq;
	UINT16 pre_sym;
	INT8 autoscan_stop_flag;
	UINT8 search_type;  // 1=AutoScan; 0=ChannelChange
	UINT8 debug_flag;  // DEBUG_FLAG
	UINT8 autoscan_control;	//0:normal autoscan, 1:get valid freq, 2:get 64K spectrum,just execute half autoscan
							//attention: you should set the variable base on the function before call nim_s3503_autoscan
	
#ifdef __TDS__
	struct nim_device_stats stats;
#endif
	
	UINT8 chip_id;
	struct EXT_DM_CONFIG ext_dm_config;
	struct nim_c3501h_status ul_status;
	INT32 ext_lnb_id;
	INT32 (*ext_lnb_control) (UINT32, UINT32, UINT32);
	UINT8 lnb_gpio_select;	//0:main chip gpio, 1:demod gpio
	UINT32 lnb_gpio_num;
	struct nim_c3501h_tsk_status tsk_status;
	struct nim_c3501h_t_param t_param;
	UINT32 cur_freq;
	UINT32 cur_sym; 
    UINT8 change_type; // 0:set tp only  1: set tp and ISID   2: set ISID only     3 :set tp and get ISID
	NIM_FLAG_LOCK flag_id;
	enum NIM_BLSCAN_MODE blscan_mode;
	OSAL_ID c3501h_mutex;
	
	// For ACM used
	struct nim_dvbs_channel_info *channel_info;
	struct nim_dvbs_bb_header_info *bb_header_info;
	struct nim_dvbs_isid *isid;
    struct nim_dvbs_tsn *tsn;
    OSAL_ID scan_mutex;	
	//end
	
	// For PLSN Search used
	ID plsn_task_id;//add by robin,20170216
	PLS plsn;
	// end

#ifdef __LINUX__
	struct mutex 					i2c_mutex;
	struct mutex 					multi_process_mutex;
	struct mutex 					tuner_open_mutex;
	NIM_AUTO_SCAN_T 				as_info;
	struct t_diseqc_info 			diseqc_info;
	struct QPSK_TUNER_CONFIG_EXT 	tuner_config;
	AUTO_SCAN_PARA 					blind_msg;
	NIM_FLAG_LOCK					flag_lock;
	UINT8 							dev_idx;
	UINT8 							nim_used;
	BOOL 							yet_return;
	BOOL							nim_init;
	unsigned char 					as_status;
	UINT8 							work_alive;
	UINT8							tuner_opened;
	wait_queue_head_t       		as_sync_wait_queue;
#endif
	//1:function waiting_channel_lock finished;0:function waiting_channel_lock is working now
	UINT8							wait_chanlock_finish_flag;

}; 

struct nim_c3501h_tp_scan_para
{
    UINT32 est_rs;      // Softsearch mode = Estimate rs from search arithmetic;  Channel change mode = The rs from upper software
	UINT32 correct_rs;
	INT32 est_freq;    // Softsearch mode = Estimate freq from search arithmetic;  Channel change mode = The freq from upper software
	UINT32 tuner_freq;
	UINT32 est_fec;     // Unused 
    UINT32 low_sym;     // 1 = low sym(less than LOW_SYM_THR); 0 = normal
    INT32  freq_err;    // 1/256M only valid for M3031
    INT32  delfreq;     // Delta freq set to demod to correct deomdsearch freq point
    UINT32 pl_lock_thr; // Used for softsearch
    UINT32 tr_lock_thr; // Used for softsearch
    UINT32 cr_lock_thr; // Used for softsearch
    UINT32 fs_lock_thr; // Used for softsearch
    UINT32 search_status; // 0 = initial, 1 = has tr lock
    UINT8 change_work_mode_flag; // 0: Try s2, 1: Try s
};

enum PLSN_SUB_REG
{
	PLSN_PILOT_DIFF_THR       = 1 ,  
	PLSN_BBH_DIFF_THR         = 2 ,  
	PLSN_DATA_DIFF_THR        = 3 ,  
	PLSN_DATA_WINDOW          = 4 ,  
	PLSN_BBH_UPL              = 5 ,  
	PLSN_BBH_SYNC             = 6 ,  
	PLSN_CAP_PD_SKIP_TIME_THR = 7 ,  
	PLSN_CAP_PD_TIME_THR      = 8 ,  
	PLSN_CAP_BBH_TIME_THR     = 9 ,  
	PLSN_CAP_NP_TIME_THR      = 10,  
};

enum PLSN_SUB_REG_RW
{
	PLSN_SUB_WRITE			= 0 ,  
	PLSN_SUB_READ			= 1 ,  
};

enum PLSN_SEARCH_STATUS
{
	PLSN_FAIL						= 0,  
	PLSN_METHOD_1_SEARCH			= 1,  
	PLSN_METHOD_2_SEARCH			= 2,
	PLSN_METHOD_3_SEARCH			= 3,
};

enum PLSN_TRY_STATUS
{
	PLSN_DEFAULT_TRY			= 0,  
	PLSN_METHOD_1_TRY			= 1,  
	PLSN_METHOD_2_TRY			= 2,
	PLSN_METHOD_3_TRY			= 3,
};

#ifdef __LINUX__
#define LNB_CMD_BASE			0xf0
#define LNB_CMD_ALLOC_ID		(LNB_CMD_BASE+1)
#define LNB_CMD_INIT_CHIP		(LNB_CMD_BASE+2)
#define LNB_CMD_SET_POLAR		(LNB_CMD_BASE+3)
#define LNB_CMD_POWER_EN		(LNB_CMD_BASE+4)

#define NIM_TUNER_C3031B_ID			NIM_TUNER_M3031_ID
#define	NIM_TUNER_SET_C3031B_FREQ_ERR   NIM_TUNER_SET_M3031_FREQ_ERR
#define	NIM_TUNER_GET_C3031B_FREQ_ERR   NIM_TUNER_GET_M3031_FREQ_ERR
#define	NIM_TUNER_GET_C3031B_GAIN_FLAG  NIM_TUNER_GET_M3031_GAIN_FLAG

#define HAL_GPIO_BIT_SET ali_gpio_set_value

extern RET_CODE 	nim_send_as_msg(int id, unsigned char lck, unsigned char polar, unsigned short freq, 
                                unsigned int sym, unsigned char fec, unsigned char as_stat);
extern UINT32 		nim_c3501h_dvbs_as_cb2_ui(void *p_priv, unsigned char lck, unsigned char polar, unsigned short freq, 
                               unsigned int sym, unsigned char fec, unsigned char as_stat);
                               
extern INT32 		nim_callback(NIM_AUTO_SCAN_T *pst_auto_scan, void *pfun, UINT8 status, UINT8 polar, 
                             UINT16 freq, UINT32 sym, UINT8 fec, UINT8 stop); 
#endif

extern UINT8  *dram_base_t;
                              
//---------------System Essensial Function --------------//
DWORD nim_c3501h_multu64div(UINT32 v1, UINT32 v2, UINT32 v3);
INT32 nim_c3501h_crc8_check(void *input, INT32 len,INT32 polynomial);
UINT32 nim_c3501h_Log10Times100_L( UINT32 x);
#ifndef FPGA_PLATFORM
INT32 nim_c3501h_attach(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
#endif
INT32 nim_c3501h_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
INT32 nim_c3501h_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list);
#ifdef __TDS__
INT32 nim_c3501h_task_init(struct nim_device *dev);
#endif
void nim_c3501h_task(UINT32 param1, UINT32 param2);
INT32 nim_c3501h_open(struct nim_device *dev);
INT32 nim_c3501h_close(struct nim_device *dev);

/////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_data_cap.h
////////////////////////////////////////////////////////////////////////////////////////////
//---------------- capture Function --------------//
// added by Gloria.Zhou for TS GS auto cap 2015.12.1
INT32 nim_c3501h_tso_bbfrm_cap_mod(struct nim_device *dev);
INT32 nim_c3501h_rec_stop(struct nim_device *dev);
INT32 nim_c3501h_ts_gs_auto_cap_mod(struct nim_device *dev);
INT32 nim_c3501h_tsbb_cap_set(struct nim_device *dev, UINT8 s_case,UINT32 swpidf,UINT32 swpidd);
INT32 nim_c3501h_dma_cap_set(struct nim_device *dev, UINT8 s_case);
INT32 nim_c3501h_cap_cfg(struct nim_device *dev,UINT32 dram_len);
INT32 nim_c3501h_la_cfg(struct nim_device *dev);
INT32 nim_c3501h_normal_tso_set(struct nim_device *dev);
#ifdef FPGA_PLATFORM
INT32 nim_c3501h_adc2mem_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src);
#endif
INT32 nim_c3501h_ts_cap_data_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src);
INT32 nim_c3501h_cap_data_entity(struct nim_device *dev,  UINT32 *cap_buffer, UINT32 dram_len, UINT8 cap_src);
INT32 nim_c3501h_cap_calculate_energy(struct nim_device *dev);
INT32 nim_c3501h_adc2mem_calculate_energy(struct nim_device *dev,UINT8 *cap_buffer,UINT32 dram_len);
INT32 nim_c3501h_autoscan_signal_input(struct nim_device *dev, UINT8 s_case);
void nim_c3501h_cap_fft_result_read(struct nim_device *dev);
INT32 nim_c3501h_cap_iq_enerage(struct nim_device *dev);
INT32 nim_c3501h_cap(struct nim_device *dev, UINT32 startFreq, INT32 *cap_buffer, UINT32 sym);
INT32 nim_c3501h_wide_band_scan_open(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq,UINT32 step_freq);
INT32 nim_c3501h_wide_band_scan_close();
INT32 nim_c3501h_wide_band_scan(struct nim_device *dev,UINT32 start_freq, UINT32 end_freq);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_debug.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void nim_c3501h_cr_adaptive_monitor(struct nim_device *dev);
void nim_c3501h_cr_new_adaptive_monitor(struct nim_device *dev);

INT32 nim_c3501h_cr_sw_adaptive(struct nim_device *dev);
INT32 nim_c3501h_debug_intask(struct nim_device *dev);
INT32 nim_c3501h_mon_signal(struct nim_device *dev);
INT32 nim_c3501h_mon_reg(struct nim_device *dev);
INT32 nim_c3501h_mon_lock_status(struct nim_device *dev);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_define.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
#define qpsk_config				QPSK_Config
#define get_fec 				get_FEC
#define get_snr 				get_SNR
#define di_seq_c_operate 		DiSEqC_operate
#define di_seq_c2x_operate 		DiSEqC2X_operate
#define get_agc 				get_AGC
#define get_ber 				get_BER
#define NIM_CHANNEL_CHANGE 		NIM_Channel_Change
#define NIM_AUTO_SCAN 			NIM_Auto_Scan
#define fub 					Fub
#define fft_energy_1024 		FFT_energy_1024
#define	fft_energy_1024_tmp		FFT_energy_1024_tmp
#define last_tuner_if 			Last_Tuner_IF 
#define chlspec_num 			chlspec_Num 
#define tp_number 				TP_number 
#define os_get_tick_count 		OS_GetTickCount
#define final_est_freq 			Final_Est_Freq
#define final_est_sym_rate 		Final_Est_SymRate 
#define max_fft_energy 			Max_FFT_energy 
#define frequency_est			Frequency_Est
#define symbol_rate_est			SymbolRate_Est

#define nim_tuner_init 						nim_Tuner_Init
#define nim_tuner_control 					nim_Tuner_Control
#define nim_tuner_status 					nim_Tuner_Status
#define nim_tuner_gain 						nim_Tuner_Gain
#define nim_tuner_cal_agc 					nim_Tuner_Cal_Agc
#define nim_tuner_command 					nim_Tuner_Command
#define nim_s3501_fft_wideband_scan			nim_s3501_FFT_WidebandScan
//#define nim_s3501_fft_wideband_scan_m3031	nim_s3501_FFT_WidebandScan_C3031B 	
#define nim_s3501_search_tp					nim_s3501_SearchTP
#define nim_s3501_median_filter				nim_s3501_MedianFilter
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_hw_cfg.h
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef FPGA_PLATFORM
    //INT32 nim_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
    //INT32 nim_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
#else
	// Use fpga platform spec io same with 3503/3505 fpga
	#define NIM_C3501H_BASE_ADDR  0xb8002000
#endif

enum C3501H_SUB_WORK_MODE
{
	ACM_SUB_WORK_MODE = 0,
	CCM_SUB_WORK_MODE = 1,
	CCM_DUMMY_SUB_WORK_MODE = 2,
	AUTO_SUB_WORK_MODE = 3,
};

enum C3501H_WORK_MODE
{
	DVBS_WORK_MODE = 0,
	DVBS2_WORK_MODE = 1,
	DSS_WORK_MODE = 2,
	AUTO_WORK_MODE = 3,
};


INT32 nim_c3501h_reg_read(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_c3501h_reg_write(struct nim_device *dev, UINT16 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_c3501h_set_err(struct nim_device *dev);
INT32 nim_c3501h_get_err(struct nim_device *dev);
INT32 nim_c3501h_clear_err(struct nim_device *dev);
INT32 nim_c3501h_sub_read_fsm(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_fsm(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_new_iqb(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_new_iqb(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_cci(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_cci(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_tr(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_tr(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_beq(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_beq(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_pl(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_pl(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_demap_agc3(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_demap_agc3(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_cr(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_cr(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_eq_dd(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_eq_dd(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_demap(struct nim_device *dev, INT32 ADDR, INT32 CMD, INT32 *VALUE);
INT32 nim_c3501h_sub_write_demap(struct nim_device *dev, INT32 ADDR, INT32 CMD, INT32 VALUE);
INT32 nim_c3501h_sub_read_demap_flt(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_demap_flt(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_demap_mon_tsn(struct nim_device *dev, UINT32 ADDR, UINT8 *VALUE);
INT32 nim_c3501h_sub_read_demap_flt_tsn(struct nim_device *dev, UINT32 ADDR, UINT8 *VALUE);
INT32 nim_c3501h_sub_write_demap_flt_tsn(struct nim_device *dev, UINT32 ADDR, UINT8 VALUE);
INT32 nim_c3501h_sub_read_plsn(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_plsn(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_vb(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_vb(struct nim_device *dev, INT32 ADDR, INT32 VALUE);
INT32 nim_c3501h_sub_read_packet(struct nim_device *dev, INT32 ADDR, INT32 *VALUE);
INT32 nim_c3501h_sub_write_packet(struct nim_device *dev, INT32 ADDR, INT32 VALUE);

INT32 get_isid_debug(struct nim_device *dev);

//---------------Demodulator configuration Function --------------//
//INT32 nim_c3501h_sym_config(struct nim_device *dev, UINT32 sym); // unused
INT32 nim_c3501h_set_interrupt_mask(struct nim_device *dev, UINT8 int_mask);
INT32 nim_c3501h_clear_interrupt(struct nim_device *dev);
INT32 nim_c3501h_reset_fsm(struct nim_device *dev);
INT32 nim_c3501h_start_fsm(struct nim_device *dev);
INT32 nim_c3501h_clean_fsm_st(struct nim_device *dev);
INT32 nim_c3501h_reg_get_fsm_state(struct nim_device *dev, UINT8 *fsm_state);
INT32 nim_c3501h_set_cap_mode(struct nim_device *dev, UINT8 cap_mode);
INT32 nim_c3501h_set_hw_timeout(struct nim_device *dev, UINT16 time_thr);
static INT32 nim_c3501h_i2c_through_open(struct nim_device *dev);
static INT32 nim_c3501h_i2c_through_close(struct nim_device *dev);
INT32 nim_c3501h_set_ext_lnb(struct nim_device *dev, struct QPSK_TUNER_CONFIG_API *ptrQPSK_Tuner);
INT32 nim_c3501h_after_reset_set_param(struct nim_device *dev);	// unfinished

INT32 nim_c3501h_set_adc(struct nim_device *dev, UINT8 vpp_sel);
INT32 nim_c3501h_set_dsp_clk (struct nim_device *dev, UINT8 clk_sel);
INT32 nim_c3501h_get_dsp_clk(struct nim_device *dev, UINT32 *sample_rate);
INT32 nim_c3501h_set_agc1(struct nim_device *dev, UINT8 low_sym, UINT8 s_Case, UINT8 frz_agc1);
INT32 nim_c3501h_set_rs(struct nim_device *dev, UINT32 rs);
INT32 nim_c3501h_set_polar(struct nim_device *dev, UINT8 polar);
INT32 nim_c3501h_set_fc_search_range(struct nim_device *dev, UINT8 s_Case, UINT32 rs);
INT32 nim_c3501h_set_rs_search_range(struct nim_device *dev, UINT8 s_Case, UINT32 rs);
INT32 nim_c3501h_set_search_range_limited(struct nim_device *dev, UINT8 cfg);
INT32 nim_c3501h_cfg_tuner_get_freq_err(struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param);
INT32 nim_c3501h_set_freq_offset(struct nim_device *dev, INT32 delfreq);
//INT32 nim_c3501h_freq_offset_set(struct nim_device *dev, UINT8 low_sym, UINT32 *s_Freq);
//INT32 nim_c3501h_freq_offset_reset(struct nim_device *dev, UINT8 low_sym);
//INT32 nim_c3501h_freq_offset_reset1(struct nim_device *dev, UINT8 low_sym, INT32 delfreq);
INT32 nim_c3501h_tr_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym);
INT32 nim_c3501h_cr_setting(struct nim_device *dev, UINT8 s_Case, UINT32 sym);
INT32 nim_c3501h_tr_cr_setting(struct nim_device *dev, UINT8 s_Case);
//INT32 nim_c3501h_cr_adaptive_initial (struct nim_device *dev);
//INT32 nim_c3501h_cr_adaptive_configure (struct nim_device *dev, UINT32 sym);
INT32 nim_c3501h_cr_tab_init(struct nim_device *dev);
//INT32 nim_c3501h_cr_new_tab_init(struct nim_device *dev);
INT32 nim_c3501h_cr_new_adaptive_unlock_monitor(struct nim_device *dev);    // reserved
//INT32 nim_c3501h_cr_new_modcod_table_init(struct nim_device *dev,UINT32 sym);
//INT32 nim_c3501h_cr_adaptive_method_choice(struct nim_device *dev, UINT8 choice_type);
INT32 nim_c3501h_nframe_step_tso_setting(struct nim_device *dev,UINT32 sym_rate,UINT8 s_Case);  // reserved
//INT32 nim_c3501h_fec_set_ldpc(struct nim_device *dev, UINT8 s_Case, UINT8 c_ldpc, UINT8 c_fec);
INT32 nim_c3501h_tso_initial (struct nim_device *dev, UINT8 insert_dummy, UINT8 tso_mode);
INT32 nim_c3501h_tso_off (struct nim_device *dev);
INT32 nim_c3501h_tso_on (struct nim_device *dev);
INT32 nim_c3501h_tso_dummy_off (struct nim_device *dev);
INT32 nim_c3501h_tso_dummy_on (struct nim_device *dev);
INT32 nim_c3501h_invert_moerrj (struct nim_device *dev);
INT32 nim_c3501h_recover_moerrj (struct nim_device *dev);
INT32 nim_c3501h_diseqc_initial(struct nim_device *dev);
INT32 nim_c3501h_diseqc_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt);
INT32 nim_c3501h_diseqc2x_operate(struct nim_device *dev, UINT32 mode, UINT8 *cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt);
INT32 nim_c3501h_fec_set_demap_noise(struct nim_device *dev);
INT32 nim_c3501h_open_ci_plus(struct nim_device *dev, UINT8 *ci_plus_flag);
INT32 nim_c3501h_module_demap_flt_ini(struct nim_device *dev);
INT32 nim_c3501h_set_12v(struct nim_device *dev, UINT8 flag);
INT32 nim_c3501h_fft_set_para(struct nim_device *dev);
INT32 nim_c3501h_get_bypass_buffer(struct nim_device *dev);
INT32 nim_c3501h_module_demap_flt_set(struct nim_device *dev, UINT8 flt_en, UINT8 flt_modu_type, UINT8 flt_frame_type, UINT8 flt_pilot, UINT16 flt_code_rate);
INT32 nim_c3501h_module_cci_ini(struct nim_device *dev);	
INT32 nim_c3501h_module_cci_set(struct nim_device *dev, UINT8 s_case);
INT32 nim_c3501h_module_diseqc_cmd_queue_ini(struct nim_device *dev);
INT32 nim_c3501h_module_diseqc_cmd_queue_set(struct nim_device *dev, UINT8 diseqc_que_cmd);
INT32 nim_c3501h_set_map_beta(struct nim_device *dev, UINT8 index, UINT8 frame_mode);
INT32 nim_c3501h_set_demap_noise(struct nim_device *dev,UINT8 index, UINT8 frame_mode);
INT32 nim_c3501h_set_work_mode(struct nim_device *dev, UINT8 work_mode);
INT32 nim_c3501h_set_sub_work_mode(struct nim_device *dev, UINT8 sub_work_mode);
INT32 nim_c3501h_set_retry(struct nim_device *dev, UINT8 retry);
INT32 nim_c3501h_set_iq_swap(struct nim_device *dev, UINT8 default_iqswap, UINT8 search_en);
//INT32 nim_c3501h_acm_set(struct nim_device *dev, UINT8 s_case);
INT32 nim_c3501h_get_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid);
INT32 nim_c3501h_set_isid(struct nim_device *dev, struct nim_dvbs_isid *p_isid);
INT32 nim_c3501h_get_isid_after_lock(struct nim_device *dev, UINT32 change_type, struct nim_dvbs_isid *p_isid);
INT32 nim_c3501h_task_tso_setting(struct nim_device * dev,UINT32 sym_rate,UINT8 s_case);
INT32 nim_c3501h_s2_ldpc_est_ber_enable(struct nim_device *dev);
INT32 nim_c3501h_s2_ldpc_est_ber_disable(struct nim_device *dev);
INT32 nim_c3501h_module_demap_flt_rpt(struct nim_device *dev, UINT16 *demap_flt_status);
INT32 nim_c3501h_module_cci_rpt(struct nim_device *dev, UINT16 *cci_rpt_freq, UINT8 *cci_rpt_level, UINT8 *cci_rpt_lock, UINT8 s_Case);
INT32 nim_c3501h_module_diseqc_cmd_queue_rpt(struct nim_device *dev);
INT32 nim_c3501h_hw_open(struct nim_device *dev);
INT32 nim_c3501h_hw_close(struct nim_device *dev);
INT32 nim_c3501h_hw_check(struct nim_device *dev);
INT32 nim_c3501h_hw_init(struct nim_device *dev);
//INT32 nim_c3501h_trans_pls(struct nim_device *dev, PLS *pls_data);
//INT32 nim_c3501h_set_pls(struct nim_device *dev, PLS *pls_data);
INT32 nim_c3501h_set_acq_workmode(struct nim_device *dev, UINT8 s_Case);
void nim_c3501h_cr_write_adpt_tab(struct nim_device *dev, int TAB_ADDR, int TAB_ITEM);  // hyman cr adaptive init
void nim_c3501h_cr_write_adpt_s2_coderatetab(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_s2_coderatetablow(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_s_coderatetab(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_s_coderatetab_low(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_awgncoeftab(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_loopcoeftab(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_clippedtab(struct nim_device *dev, int TAB_CELL);
void nim_c3501h_cr_write_adpt_avgpedtab(struct nim_device *dev, int TAB_CELL);
INT32 nim_c3501h_cr_adpt_tab_init(struct nim_device *dev);
INT32 nim_c3501h_freeze_fsm(struct nim_device *dev, UINT8 enable, UINT8 state);
INT32 nim_c3501h_preset_fsm(struct nim_device *dev, UINT8 enable, UINT8 state);
INT32 nim_c3501h_set_set_cr_dis_pilot(struct nim_device *dev);
INT32 nim_c3501h_set_set_cr_en_pilot(struct nim_device *dev);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_monitor.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------Demodulator monitor Function --------------//
INT32 nim_c3501h_get_interrupt(struct nim_device *dev, UINT8 *intdata);
INT32 nim_c3501h_reg_get_chip_type(struct nim_device *dev);
INT32 nim_c3501h_reg_get_strap_bond_info(struct nim_device *dev);
INT32 nim_c3501h_get_lock(struct nim_device *dev, UINT8 *lock);
#ifdef FC_SEARCH_RANGE_LIMITED
INT32 nim_c3501h_get_lock_check_limited(struct nim_device *dev, UINT8 *lock_state);
#endif
INT32 nim_c3501h_get_lock_reg(struct nim_device *dev, UINT16 *lock_state);
INT32 nim_c3501h_get_tuner_lock(struct nim_device *dev, UINT8 *tun_lock);
INT32 nim_c3501h_reg_get_iqswap_flag(struct nim_device *dev, UINT8 *iqswap_flag);
INT32 nim_c3501h_get_demod_gain(struct nim_device *dev, UINT8 *agc);
INT32 nim_c3501h_get_tuner_gain(struct nim_device *dev,UINT8 agc, INT32 *agc_tuner);
INT32 nim_c3501h_get_dbm_level(struct nim_device *dev,UINT8 agc_demod, INT32 agc_tuner,INT8 *level_db);
INT32 nim_c3501h_get_agc_filter(struct nim_device *dev, INT8 *agc);
INT32 nim_c3501h_get_agc(struct nim_device *dev, UINT8 *agc);
INT32 nim_c3501h_get_agc_dbm(struct nim_device *dev, INT8 *agc_dbm);
INT32 nim_c3501h_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
INT32 nim_c3501h_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_c3501h_get_tune_freq(struct nim_device *dev, INT32 *freq);
INT32 nim_c3501h_reg_get_work_mode(struct nim_device *dev, UINT8 *work_mode);
INT32 nim_c3501h_reg_get_sub_work_mode(struct nim_device *dev, UINT8 *sub_work_mode);
INT32 nim_c3501h_reg_get_code_rate(struct nim_device *dev, UINT8 *code_rate);
INT32 nim_c3501h_reg_get_map_type(struct nim_device *dev, UINT8 *map_type);
INT32 nim_c3501h_reg_get_pilot(struct nim_device *dev, UINT8 *pilot);
INT32 nim_c3501h_reg_get_frame_type(struct nim_device *dev, UINT8 *frame);
INT32 nim_c3501h_reg_get_roll_off(struct nim_device *dev, UINT8 *roll_off);
INT32 nim_c3501h_reg_iq_swap(struct nim_device *dev, UINT8 *iq_swap);
INT32 nim_c3501h_get_bit_rate(struct nim_device *dev, UINT8 work_mode, UINT8 map_type, UINT8 code_rate, UINT32 rs, UINT32 *bit_rate);
INT32 nim_c3501h_get_bitmode(struct nim_device *dev, UINT8 *bitMode);
INT32 nim_c3501h_get_ber(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_c3501h_get_new_ber(struct nim_device *dev, UINT32 *ber);
INT32 nim_c3501h_get_per(struct nim_device *dev, UINT32 *rs_ubc);
INT32 nim_c3501h_get_fer(struct nim_device *dev, UINT32 *fer);

// added by gloria 2017.2.23
INT32 nim_c3501h_get_modcod(struct nim_device *dev,  UINT8 *rs_ubc);
//INT32 nim_c3501h_get_work_mode(struct nim_device *dev, UINT8 *rs_ubc);
INT32 nim_c3501h_get_bchover(struct nim_device *dev, UINT8 *rs_ub);
INT32 nim_c3501h_get_ldpc_iter_cnt(struct nim_device *dev, UINT8 *rs_ub);

INT32 nim_c3501h_get_new_per(struct nim_device *dev, UINT32 *per);
INT32 nim_c3501h_get_phase_error(struct nim_device *dev, INT32 *phase_error);
INT32 nim_c3501h_get_mer(struct nim_device *dev, UINT32 *mer);
INT32 nim_c3501h_get_snr(struct nim_device *dev, UINT8 *snr);
INT32 nim_c3501h_get_snr_db(struct nim_device *dev, INT16 *snr_db);
INT32 nim_c3501h_check_ber(struct nim_device *dev, UINT32 *rs_ubc);
UINT8 nim_c3501h_get_snr_index(struct nim_device *dev);
INT32 nim_c3501h_get_cur_freq(struct nim_device *dev, UINT32 *freq);
UINT8 nim_c3501h_get_cr_num(struct nim_device *dev);
//INT32 nim_c3501h_get_ldpc(struct nim_device *dev, UINT32 *rs_ubc); // == nim_c3501h_get_ldpc_iter_cnt
INT32 nim_c3501h_cr_sw_snr_rpt(struct nim_device *dev);
UINT8 nim_c3501h_map_beta_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT8 *demap_beta_active, UINT8 *demap_beta);
UINT16 nim_c3501h_demap_noise_report(struct nim_device *dev, UINT8 index, UINT8 frame_mode, UINT16 *demap_noise);
INT32 nim_c3501h_reg_get_freqoffset(struct nim_device *dev, INT32 *freq_offset);
INT32 nim_c3501h_get_symbol(struct nim_device *dev, struct nim_get_symbol *p_symbol);
INT32 nim_c3501h_get_bb_header_info(struct nim_device *dev);
INT32 nim_c3501h_get_channel_info(struct nim_device *dev);
INT32 nim_c3501h_get_tp_info(struct nim_device *dev);
INT32 nim_c3501h_printf_tp_info(struct nim_device *dev);
INT32 nim_c3501h_show_acm_ccm_info(struct nim_device *dev);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_private.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CHIP_ID 0x35
#define CHIP_ID_3501H 0x35012017

#define DSP_REG_OFFSET 0
#define FEC_REG_OFFSET 0x100
#define PHY_REG_OFFSET 0x140
#define SYS_REG_OFFSET 0x1C0

enum NIM_C3501H_REGISTER_ADDRESS
{
	// DSP field
	C3501H_R00_TOP_FSM_CFG_1 = DSP_REG_OFFSET + 0x00,                                  
	C3501H_R04_TOP_FSM_CFG_2 = DSP_REG_OFFSET + 0x04,                                  
	C3501H_R08_TOP_FSM_CFG_3 = DSP_REG_OFFSET + 0x08,                                  
	C3501H_R0C_TOP_FSM_CFG_4 = DSP_REG_OFFSET + 0x0c,                                  
	C3501H_R10_TOP_FSM_CFG_5 = DSP_REG_OFFSET + 0x10,                                  
	C3501H_R14_TOP_FSM_RPT_1 = DSP_REG_OFFSET + 0x14,                                  
	C3501H_R18_TOP_FSM_RPT_2 = DSP_REG_OFFSET + 0x18,                                  
	C3501H_R1C_MODU_INFO_RPT_1 = DSP_REG_OFFSET + 0x1c,                                
	C3501H_R20_MODU_INFO_RPT_2 = DSP_REG_OFFSET + 0x20,                                
	C3501H_R24_MODU_INFO_RPT_3 = DSP_REG_OFFSET + 0x24,                                
	C3501H_R28_MODU_INFO_RPT_4 = DSP_REG_OFFSET + 0x28,                                
	C3501H_R2C_ANA_AGC1_CFG = DSP_REG_OFFSET + 0x2c,                                   
	C3501H_R30_AGC1_CFG = DSP_REG_OFFSET + 0x30,                                       
	C3501H_R34_AGC1_DCC_RPT = DSP_REG_OFFSET + 0x34,                                   
	C3501H_R38_DCC_IQB_CFG = DSP_REG_OFFSET + 0x38,                                    
	C3501H_R3C_NEW_IQB_CFG = DSP_REG_OFFSET + 0x3c,                                    
	C3501H_R40_NEW_IQB_RPT = DSP_REG_OFFSET + 0x40,                                    
	C3501H_R44_CCI_CANCEL_CFG_0 = DSP_REG_OFFSET + 0x44,                               
	C3501H_R48_CCI_CANCEL_CFG_1 = DSP_REG_OFFSET + 0x48,                               
	C3501H_R4C_CCI_CANCEL_RPT = DSP_REG_OFFSET + 0x4c,                                 
	C3501H_R50_FLT_SNIR = DSP_REG_OFFSET + 0x50,                                       
	C3501H_R54_AGC2_CFG = DSP_REG_OFFSET + 0x54,                                       
	C3501H_R58_TR_CFG_0 = DSP_REG_OFFSET + 0x58,                                       
	C3501H_R5C_TR_CFG_1 = DSP_REG_OFFSET + 0x5c,                                       
	C3501H_R60_TR_CFG_2 = DSP_REG_OFFSET + 0x60,                                       
	C3501H_R68_BEQ_WORK_MODE_CFG = DSP_REG_OFFSET + 0x68,                              
	C3501H_R6C_PL_RPT = DSP_REG_OFFSET + 0x6c,                                         
	C3501H_R70_PL_RPT_1 = DSP_REG_OFFSET + 0x70,                                       
	C3501H_R74_PL_RPT_2 = DSP_REG_OFFSET + 0x74,                                       
	C3501H_R78_PL_RPT_4 = DSP_REG_OFFSET + 0x78,                                       
	C3501H_R7C_PL_CFG_0 = DSP_REG_OFFSET + 0x7c,                                       
	C3501H_R80_PL_CFG_1 = DSP_REG_OFFSET + 0x80,                                       
	C3501H_R84_PL_CFG_2 = DSP_REG_OFFSET + 0x84,                                       
	C3501H_R88_PL_CFG_3 = DSP_REG_OFFSET + 0x88,                                       
	C3501H_R8C_PL_CFG_4 = DSP_REG_OFFSET + 0x8c,                                       
	C3501H_R90_PL_CFG_5 = DSP_REG_OFFSET + 0x90,                                       
	C3501H_R94_PL_CFG_6 = DSP_REG_OFFSET + 0x94,                                       
	C3501H_R98_AGC3_CFG = DSP_REG_OFFSET + 0x98,                                       
	C3501H_R9C_DEMAP_AGC3_CFG = DSP_REG_OFFSET + 0x9c,                                 
	C3501H_RA0_CR_CFG_0 = DSP_REG_OFFSET + 0xa0,                                       
	C3501H_RA4_CR_CFG_1 = DSP_REG_OFFSET + 0xa4,                                       
	C3501H_RA8_CR_CFG_2 = DSP_REG_OFFSET + 0xa8,                                       
	C3501H_RAC_CR_CFG_3 = DSP_REG_OFFSET + 0xac,                                       
	C3501H_RB0_CR_CFG_4 = DSP_REG_OFFSET + 0xb0,                                       
	C3501H_RB4_CR_CFG_5 = DSP_REG_OFFSET + 0xb4,                                       
	C3501H_RB8_CR_RPT_0 = DSP_REG_OFFSET + 0xb8,                                       
	C3501H_RBC_CR_RPT_1 = DSP_REG_OFFSET + 0xbc,                                       
	C3501H_RC0_CR_ADPT_CF = DSP_REG_OFFSET + 0xc0,                                     
	C3501H_RC4_CR_ADPT_CF_RPT = DSP_REG_OFFSET + 0xc4,                                 
	C3501H_RC8_CR_ADPT_RPT_0 = DSP_REG_OFFSET + 0xc8,                                  
	C3501H_RCC_CR_ADPT_RPT_1 = DSP_REG_OFFSET + 0xcc,                                  
	C3501H_RD0_CR_SUB_REG_CFG = DSP_REG_OFFSET + 0xd0,                                 
	C3501H_RD8_EQ_DD_CFG_0 = DSP_REG_OFFSET + 0xd8,                                    
	C3501H_RDC_EQ_DD_CFG_1 = DSP_REG_OFFSET + 0xdc,                                    
	C3501H_RE0_EQ_DD_CFG_2 = DSP_REG_OFFSET + 0xe0,                                    
	C3501H_RE4_DEMAP_CFG = DSP_REG_OFFSET + 0xe4,                                      
	C3501H_RE8_DEMAP_RPT = DSP_REG_OFFSET + 0xe8,                                      
	C3501H_REC_DEMAP_FLT_CFG = DSP_REG_OFFSET + 0xec,                                  
	C3501H_RF0_DEMAP_FLT_CFG = DSP_REG_OFFSET + 0xf0,                                  
	C3501H_RF4_PLSN_CFG = DSP_REG_OFFSET + 0xf4,                                  
	C3501H_RF8_TEST_MUX_OUT = DSP_REG_OFFSET + 0xf8,                                   
	C3501H_RFC_PLSN_RPT = DSP_REG_OFFSET + 0xfc,                                         

	// FEC field
	C3501H_R00_VB_CFG_RPT = FEC_REG_OFFSET + 0x00,
	C3501H_R04_RS_CFG_RPT = FEC_REG_OFFSET + 0x04,
	C3501H_R08_S1_ITER_FEC_CFG = FEC_REG_OFFSET + 0x08,
	C3501H_R0C_S2_FEC_CFG_0 = FEC_REG_OFFSET + 0x0C,
	C3501H_R10_S2_FEC_CFG_1 = FEC_REG_OFFSET + 0x10,
	C3501H_R14_S2_FEC_RPT = FEC_REG_OFFSET + 0x14,
	C3501H_R18_BB_CMD_CFG = FEC_REG_OFFSET + 0x18,
	C3501H_R1C_BB_DEC_CFG = FEC_REG_OFFSET + 0x1c,
	C3501H_R20_BB_ROLL_OFF_CFG = FEC_REG_OFFSET + 0x20,
	C3501H_R24_PACKET_CFG = FEC_REG_OFFSET + 0x24,
	C3501H_R28_MON_PER_CFG = FEC_REG_OFFSET + 0x28,
	C3501H_R2C_MON_PER_RPT = FEC_REG_OFFSET + 0x2c,
	C3501H_R30_MON_FER_CFG = FEC_REG_OFFSET + 0x30,
	C3501H_R34_MON_FER_RPT = FEC_REG_OFFSET + 0x34,
	C3501H_R38_MON_BER_CFG = FEC_REG_OFFSET + 0x38,
	C3501H_R3C_MON_BER_RPT = FEC_REG_OFFSET + 0x3c,
	C3501H_R40_FEC_TEST_MUX = FEC_REG_OFFSET + 0x40,

	// PHY field
	C3501H_R00_CHIP_ID = PHY_REG_OFFSET + 0x00,
	C3501H_R04_SYS_CFG = PHY_REG_OFFSET + 0x04,
	C3501H_R0C_SYS_REG = PHY_REG_OFFSET + 0x0C,
	C3501H_R10_TSO_CLKG_CFG_1 = PHY_REG_OFFSET + 0x10,
	C3501H_R14_TSO_CLKG_CFG_2 = PHY_REG_OFFSET + 0x14,
	C3501H_R18_TSO_CFG = PHY_REG_OFFSET + 0x18,
	C3501H_R1C_CAPTURE_CFG = PHY_REG_OFFSET + 0x1c,
	C3501H_R20_CAPTURE_DISEQC_RPT = PHY_REG_OFFSET + 0x20,
	C3501H_R24_DISEQC_MODE_REG = PHY_REG_OFFSET + 0x24,
	C3501H_R28_DISEQC_TX_TIME_CFG = PHY_REG_OFFSET + 0x28,
	C3501H_R2C_DISEQC_RX_TIME_CFG = PHY_REG_OFFSET + 0x2c,
	C3501H_R30_DISEQC_CLK_DUTY_CFG = PHY_REG_OFFSET + 0x30,
	C3501H_R34_DISEQC_WRITE_DATA_L = PHY_REG_OFFSET + 0x34,
	C3501H_R38_DISEQC_WRITE_DATA_H = PHY_REG_OFFSET + 0x38,
	C3501H_R3C_DISEQC_READ_DATA_L = PHY_REG_OFFSET + 0x3c,
	C3501H_R40_DISEQC_READ_DATA_H = PHY_REG_OFFSET + 0x40,
	C3501H_R44_LA_PROBE_1_CFG = PHY_REG_OFFSET + 0x44,
	C3501H_R48_LA_PROBE_1_TAR = PHY_REG_OFFSET + 0x48,
	C3501H_R4C_LA_PROBE_2_CFG = PHY_REG_OFFSET + 0x4c,
	C3501H_R50_LA_PROBE_2_TAR = PHY_REG_OFFSET + 0x50,
	C3501H_R54_LA_TIMER = PHY_REG_OFFSET + 0x54,
	C3501H_R58_LA_FIFO_CFG = PHY_REG_OFFSET + 0x58,
	C3501H_R5C_LA_FIFO_RPT = PHY_REG_OFFSET + 0x5c,
	C3501H_R60_CAP2DRAM_CFG_1 = PHY_REG_OFFSET + 0x60,
	C3501H_R64_CAP2DRAM_RPT_1 = PHY_REG_OFFSET + 0x64,
	C3501H_R68_CAP2DRAM_CFG_2 = PHY_REG_OFFSET + 0x68,
	C3501H_R6C_CAP2DRAM_CFG_3 = PHY_REG_OFFSET + 0x6c,
	C3501H_R70_PAT_DMA_CFG_1 = PHY_REG_OFFSET + 0x70,
	C3501H_R74_PAT_DMA_RPT_1 = PHY_REG_OFFSET + 0x74,
	C3501H_R78_PAT_DMA_CFG_2 = PHY_REG_OFFSET + 0x78,
	C3501H_R7C_PAT_DMA_CFG_3 = PHY_REG_OFFSET + 0x7c,

	// SYS field
    C3501H_R00_STRAP_BOND_INFO = SYS_REG_OFFSET + 0x00,
    C3501H_R04_MPLL_CFG = SYS_REG_OFFSET + 0x04,
    C3501H_R08_I2C_SLAVE_CFG_1 = SYS_REG_OFFSET + 0x08,
    C3501H_R0C_I2C_SLAVE_CFG_2 = SYS_REG_OFFSET + 0x0c,
    C3501H_R10_PINMUX_CFG_1 = SYS_REG_OFFSET + 0x10,
    C3501H_R14_PINMUX_CFG_ERR = SYS_REG_OFFSET + 0x14,
    C3501H_R18_PINMUX_SEL_ERR = SYS_REG_OFFSET + 0x18,
    C3501H_R20_DIGPLL1_CTRL_REG1 = SYS_REG_OFFSET + 0x20,
    C3501H_R24_DIGPLL1_CTRL_REG2 = SYS_REG_OFFSET + 0x24,
    C3501H_R28_PROCESS_MONITOR = SYS_REG_OFFSET + 0x28,
    C3501H_R2C_GPIO_CFG = SYS_REG_OFFSET + 0x2c,
    C3501H_R30_XCTRL_CFG = SYS_REG_OFFSET + 0x30,
    C3501H_R34_PINMUX_DRV_1 = SYS_REG_OFFSET + 0x34,
    C3501H_R38_PINMUX_DRV_2 = SYS_REG_OFFSET + 0x38,
    C3501H_R3C_PINMUX_DRV_3 = SYS_REG_OFFSET + 0x3c,   
};

//------------------------DEFINE for Work Mode--------------------------------//
//#define REPORT_FAIL_TP                  // If open this macro, driver may be report "fail TP" to upper which can not locked. paladin 20160614
//#define HW_ADPT_CR               			// when print, need to open
//#define RPT_CR_SNR_EST         			// print OldAdpt EST_NOISE value

// For acm used


#define ACM_RECORD                  // For record stream in acm mode
#define ACM_CCM_FLAG NIM_OPTR_ACM

#define CRC_THRESHOLD 5

#define NIM_CAPTURE_SUPPORT    				// capture demod internal data 
#define DISEQC_DELAY		60				//adjust timing for DiSEqC circuit
//#define ANTI_WIMAX_INTF					// Anti-WIMAX patch, invalid    
//----------------------End of DEFINE for Work Mode-----------------------------//

#ifdef ANTI_WIMAX_INTF
UINT8 force_adpt_disable = 1;
#endif

#define BYPASS_BUF_SIZE_DMA 0x10000  //64K bytes, size of ADC2MEM block in unit BYTE
#define RET_CONTINUE    0xFF

#define	FFT_BITWIDTH			10
#define	STATISTIC_LENGTH		2
#define FS_MAXNUM 					15000
#define TP_MAXNUM 					2000

//average length of data to determine threshold
//0:2;1:4;2:8

#define	MAX_CH_NUMBER			32//maximum number of channels that can be stored
#define C3501H_LOACL_FREQ   		5150
#define C3501H_QPSK_TUNER_FREQ_OFFSET	3
#define C3501H_I2C_THROUGH   	0x08
#define C3501H_IQ_AD_SWAP   		0x04
#define C3501H_QPSK_FREQ_OFFSET 	0x01
#define C3501H_NEW_AGC1  		0x20
#define C3501H_POLAR_REVERT  	0x10
#define C3501H_1BIT_MODE 		0x00 
#define C3501H_2BIT_MODE 		0x40
#define C3501H_4BIT_MODE 		0x80
#define C3501H_8BIT_MODE 		0xc0
#define C3501H_AGC_INVERT		0x000
#define C3501H_USE_188_MODE		0x400
#define C3501H_DVBS_MODE			0x00	
#define C3501H_DVBS2_MODE		0x01
#define C3501H_DVBS2X_MODE		0x02
#define C3501H_SIGNAL_DISPLAY_LIN	0x800
#define C3501H_NEW_PLSYNC        0x00
#define C3501H_OLD_PLSYNC        0x01
#define NIM_FREQ_RETURN_REAL      0
#define NIM_FREQ_RETURN_SET       1
#define NIM_TUNER_SET_STANDBY_CMD	0xffffffff
#define BYPASS_BUF_SIZE 			0x20000     // 128 KB
#define BYPASS_BUF_MASK 			(BYPASS_BUF_SIZE - 1)


#define C3501H_LOW_SYM_THR 6500  // less than 6.5M is low symble TP
#define QPSK_TUNER_FREQ_OFFSET 3
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

extern INT32 call_tuner_command(struct nim_device *dev, INT32 cmd, INT32 *param);

extern const UINT8 ADC_SAMPLE_FREQ[];

INT32 nim_c3501h_set_32apsk_target(struct nim_device *dev);
INT32 nim_c3501h_set_dynamic_power(struct nim_device *dev, UINT8 snr);
INT32 nim_c3501h_set_ldpc_iter_para(struct nim_device *dev, UINT16 most_cnt, UINT8 unchg_thld0, UINT8 unchg_thld1, UINT8 pass_thld);
INT32 nim_c3501h_set_ldpc_iter_cnt_record_last(struct nim_device *dev);
INT32 nim_c3501h_set_ldpc_iter_cnt_record_max(struct nim_device *dev);
INT32 nim_c3501h_ldpc_keep_working_enable(struct nim_device *dev);
INT32 nim_c3501h_ldpc_keep_working_disable(struct nim_device *dev);
INT32 nim_c3501h_tso_soft_cbr_on (struct nim_device *dev, UINT32 bitrate);
INT32 nim_c3501h_tso_soft_cbr_off (struct nim_device *dev);
INT32 nim_c3501h_pl_sync_cfg(struct nim_device *dev);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COPY FROM nim_dvbs_c3505_tp_scan.h
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern INT32 fft_energy_1024[1024];
extern INT32 fft_energy_1024_tmp[1024];
extern INT32 frequency_est[TP_MAXNUM];
extern INT32 symbol_rate_est[TP_MAXNUM];
extern INT32 tp_number;
extern INT32 *channel_spectrum;
extern INT32 *channel_spectrum_tmp;
extern INT32 last_tuner_if;
extern INT32 chlspec_num;
extern INT32 called_num;
extern INT32 final_est_freq;
extern INT32 final_est_sym_rate;
extern INT32 max_fft_energy;

#define LOOP_PERIOD 25
#define USER_STOP -44

struct nim_c3501h_frequency_seg_params
{
    UINT32 seg_spectrum_start;    // segment spectrum start freq
    UINT32 seg_spectrum_end;      // segment spectrum end freq

    UINT32 fft_energy_seg_sum;    // sum of segment spectrum fft energy 
    UINT32 fft_energy_seg_avg;    // average of segment spectrum fft energy 
    UINT32 fft_energy_seg_max;    // max of segment spectrum fft energy 

    UINT32 no_tp_flag;            // 0 = there is no tp in tihs frequency rang 
};

struct nim_c3501h_wideband_params
{
    UINT32 fft_energy_sum;    // sum of segment spectrum fft energy 
    UINT32 fft_energy_avg;    // average of segment spectrum fft energy 
    UINT32 fft_energy_max;    // max of segment spectrum fft energy 
};

struct nim_c3501h_autoscan_params
{
    struct nim_c3501h_private *priv;
    struct NIM_AUTO_SCAN *pst_auto_scan;
    UINT32 start_t;
    UINT32 temp_t;
    UINT32 end_t;
    INT32 adc_sample_freq;
    INT32 success;
    UINT32 fft_freq;
    UINT8 reg0a_data_bak;                                   // used to restore register 0a value 
    struct nim_c3501h_wideband_params wideband_para;         // used to filter false TP for reduce time of autoscan
    struct nim_c3501h_frequency_seg_params head_seg_para;    // used to filter false TP for reduce time of autoscan
    struct nim_c3501h_frequency_seg_params tail_seg_para;    // used to filter false TP for reduce time of autoscan
    UINT8 freq_band;    // For dvb-s/s2, 0 = default, 1 = C band, 2 = Ku band
};

typedef enum WIDEBAND_MODE {
	NOT_WIDEBAND,		//not wideband mode
	WIDEBAND,			//wideband mode
} dvbs2x_wideband_mode_type_t;

INT32 nim_c3501h_tsn_enable(struct nim_device *dev, BOOL tsn_enable);
INT32 nim_c3501h_get_tsn(struct nim_device *dev, struct nim_dvbs_tsn *p_tsn);
INT32 nim_c3501h_set_tsn(struct nim_device *dev, struct nim_dvbs_tsn *p_tsn);
dvbs2x_wideband_mode_type_t nim_c3501h_get_wideband_mode(struct nim_device *dev);

//------------------Scan Function -----------------------//
INT32 nim_c3501h_channel_change(struct nim_device *dev, struct NIM_CHANNEL_CHANGE * pst_channel_change);
INT32 nim_c3501h_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 change_type, struct nim_dvbs_isid *p_isid);
INT32 nim_c3501h_autoscan_find_acm_tp(struct nim_device *dev, UINT32 lock_tp_idx, UINT32 cur_freq, UINT32 cur_sym);
INT32 nim_c3501h_autoscan_init(struct nim_device *dev, struct nim_c3501h_autoscan_params *params);
INT32 nim_c3501h_autoscan_get_spectrum(struct nim_device *dev, struct nim_c3501h_autoscan_params *params);
INT32 nim_c3501h_autoscan_estimate_tp(struct nim_device *dev, struct nim_c3501h_autoscan_params *params);
INT32 nim_c3501h_autoscan_try_tp(struct nim_device *dev, struct nim_c3501h_autoscan_params *params);
INT32 nim_c3501h_autoscan(struct nim_device *dev, struct NIM_AUTO_SCAN *pst_auto_scan);
INT32 nim_c3501h_adc2mem_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, UINT32 dram_len);
INT32 nim_c3501h_cap_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, INT32 *cap_buffer);
INT32 nim_c3501h_channel_search(struct nim_device *dev, UINT32 cr_num);
INT32 nim_c3501h_soft_search_init( struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param);
INT32 nim_c3501h_soft_search(struct nim_device *dev, struct nim_c3501h_tp_scan_para *tp_scan_param);
INT32 nim_c3501h_fft(struct nim_device *dev, UINT32 startFreq);
INT32 nim_c3501h_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq);
INT32 nim_c3501h_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr);

//Nim Autosearch,R2FFT 
extern INT32 nim_s3501_autosearch(INT32 *success, INT32 *delta_fc_est, INT32 *SymbolRate_est, INT32 *m_IF_freq, INT32 *m_ch_number);
extern void nim_s3501_smoothfilter(void);
extern void nim_s3501_median_filter(INT32 FLength, INT32 *Fdata, INT32 Scan_mode);
extern INT32 nim_s3501_search_tp(INT32 chlspec_num, INT32 *channel_spectrum,UINT32 sfreq,UINT32 adc_sample_freq,INT32 loop);

extern INT32 nim_s3501_fft_wideband_scan(struct nim_device *dev,UINT32 tuner_if, UINT32 adc_sample_freq);
//extern INT32 nim_s3501_fft_wideband_scan_m3031(struct nim_device *dev, UINT32 tuner_if, UINT32 adc_sample_freq);

extern void R2FFT(INT32 *FFT_I_1024, INT32 *FFT_Q_1024);
//demux signal pass
extern INT32 nim_s3501_autoscan_signal_input(struct nim_device *dev, UINT8 s_Case);

#ifdef FPGA_PLATFORM
INT32 nim_c3501h_channel_change_task(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
#endif

// For PLSN Search used
INT32 nim_c3501h_start_plsn_search(struct nim_device *dev);
INT32 nim_c3501h_close_plsn_search(struct nim_device *dev);
INT32 nim_c3501h_sel_plsn_search_algo(struct nim_device *dev, UINT8 algo);
INT32 nim_c3501h_rw_plsn_search_sub_reg(struct nim_device *dev, UINT8 rw, UINT8 cmd, UINT8 *value);
INT32 nim_c3501h_get_plsn_search_ret(struct nim_device *dev);
INT32 nim_c3501h_plsn_gold_to_root(INT32 plsn_gold);
INT32 nim_c3501h_plsn_root_to_gold(INT32 plsn_root);
INT32 nim_c3501h_set_plsn(struct nim_device *dev);  
INT32 nim_c3501h_clear_plsn(struct nim_device *dev);   
INT32 nim_c3501h_get_plsn(struct nim_device *dev); 
plsn_state nim_c3501h_try_plsn(struct nim_device *dev, UINT8 *index);
INT32 nim_c3501h_search_plsn_exit(struct nim_device *dev);
INT32 nim_c3501h_search_plsn_top(struct nim_device *dev);
void nim_c3501h_plsn_task(UINT32 param1, UINT32 param2);
INT32 nim_c3501h_get_search_timeout(struct nim_device *dev, UINT32 *single_timeout, UINT32 *total_timeout);
INT32 nim_c3501h_search_plsn_sw_top(struct nim_device *dev);
INT32 nim_c3501h_search_plsn_hw_top(struct nim_device *dev);
INT32 nim_c3501h_search_plsn_hw(struct nim_device *dev);
INT32 nim_c3501h_check_hw_reg(struct nim_device *dev);
INT32 nim_c3501h_force_cur_work_mode(struct nim_device *dev);
#ifdef IMPLUSE_NOISE_IMPROVED
const UINT8 c3501h_map_beta_normal_adaptive_en[32];
const UINT8 c3501h_map_beta_short_adaptive_en[32];
const UINT16 c3501h_map_beta_normal_snr_thres[32][2];
INT32 nim_c3501h_set_eq_dd_gain(struct nim_device *dev, UINT8 enable, UINT16 eq_dd_gain);
INT32 nim_c3501h_set_demap_llr_shift(struct nim_device *dev, UINT8 enable);
INT32 nim_c3501h_auto_adaptive(struct nim_device *dev);      
#endif

#ifdef C3501H_DEBUG_FLAG
INT32 nim_c3501h_set_debug_flag(struct nim_device *dev, UINT32 freq, UINT32 sym);
#endif
INT32 nim_c3501h_tuner_control(struct nim_device *dev, UINT32 freq, UINT32 sym, INT32 *freq_err);
void nim_c3501h_cr_adpt_debug(struct nim_device *dev, int debug_sel);
INT32 nim_c3501h_set_beq(struct nim_device *dev);
INT32 nim_c3501h_set_roll_off(struct nim_device *dev, UINT8 ro_en, UINT8 ro_val);

INT32 nim_c3501h_set_pl_cfe_dis_pilot(struct nim_device *dev);
INT32 nim_c3501h_pl_setting(struct nim_device *dev, UINT8 s_case, UINT32 sym);

void nim_c3501h_gpio_output(struct nim_device *dev, UINT8 pin_num, UINT8 value);
void nim_c3501h_enable_gpio_input(struct nim_device *dev, UINT8 pin_num);
void nim_c3501h_get_gpio_input(struct nim_device *dev, UINT8 pin_num, UINT8 *value);

#endif /* __LLD_DVBS_C3501H_H__ */

