/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	nim_dvbs_c3505_tds.h
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#ifndef __LLD_NIM_DVBS_C3505_TDS_H__
#define __LLD_NIM_DVBS_C3505_TDS_H__

#define __C3505_TDS__

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
#include "nim_dvbs_c3505_private.h"

#define CHANNEL_CHANGE_ASYNC

// for multistream search tp
#define ERR_SET_FLAG -1;    
#define ERR_CLR_FLAG -2;

// use mutex
#define NIM_MUTEX_CREATE	osal_semaphore_create
#define NIM_MUTEX_DELETE	osal_semaphore_delete
#define NIM_MUTEX_ENTER(priv)  \
	if(priv->ul_status.nim_c3505_sema != OSAL_INVALID_ID) \
	{ \
		do{ \
			priv->ul_status.ret= osal_semaphore_capture(priv->ul_status.nim_c3505_sema, 1000); \
			if(priv->ul_status.ret!= OSAL_E_OK) \
				{NIM_PRINTF("nim_s3501_sema[%d] capture timeout, retry...\n", \
					priv->ul_status.nim_c3505_sema);} \
		}while(priv->ul_status.ret != OSAL_E_OK); \
	}

#define NIM_MUTEX_LEAVE(priv) \
	if(priv->ul_status.nim_c3505_sema != OSAL_INVALID_ID) \
	{ \
		osal_semaphore_release(priv->ul_status.nim_c3505_sema); \
	} 

// use flag
#ifdef CHANNEL_CHANGE_ASYNC
#define NIM_FLAG_CREATE osal_flag_create
#define NIM_FLAG_DEL osal_flag_delete
#define NIM_FLAG_WAIT osal_flag_wait
#define NIM_FLAG_SET osal_flag_set
#define NIM_FLAG_CLEAR osal_flag_clear
#endif

void usb_log_lock();
void usb_log_unlock();

extern void nim_comm_delay(UINT32 us);
extern void nim_comm_sleep(UINT32 ms);

extern INT32 call_tuner_command(struct nim_device *dev, INT32 cmd, INT32 *param);
#define comm_malloc 			MALLOC
#define comm_memset				MEMSET
#define comm_free				FREE
#define comm_delay				nim_comm_delay
#define comm_sleep				nim_comm_sleep
#define comm_memcpy				memcpy

//----------------------------DEFINE for debug---------------------------------//
//#define HW_ADPT_CR_MONITOR 		// print OldAdpt parameters  
//#define HW_ADPT_NEW_CR_MONITOR	// print CR coefficients, no matter OldAdpt or NewAdpt
//#define NEW_CR_ADPT_SNR_EST_RPT 	// print c/n dB value
//#define SW_ADPT_CR					
//#define SW_SNR_RPT_ONLY
//#define DEBUG_SOFT_SEARCH      		// Try someone TP for special case, for DEbug
//#define DEBUG_CHANNEL_CHANGE      	// Try Get current TP spectrum



// For Debug Used
//#define C3505_DEBUG_GET_SPECTRUM  		// Debug autoscan
//#define C3505_DEBUG_FLAG                	// Debug signal issue and so on
//#define DEBUG_REGISTER                  	// Monitor register value



#ifdef C3505_DEBUG_FLAG
	#define NIM_PRINTF_DEBUG
	#define MON_PRINTF_DEBUG
	#define ERR_DEBUG
	#define AUTOSCAN_DEBUG
	#define ACM_PRINTF
	#define DEBUG_IN_TASK
#endif


#ifdef C3505_DEBUG_GET_SPECTRUM
	#define NIM_PRINTF_DEBUG
	#define ERR_DEBUG
	#define FFT_PRINTF_DEBUG
	#define AUTOSCAN_DEBUG
	#define ACM_PRINTF
	#define DEBUG_IN_TASK
	//#define NIM_GET_1024FFT
#endif

//#define PLSN_DEBUG
//#define PRINT_TO_USB

#ifdef PRINT_TO_USB
	static UINT8 tmp_buff[1000];
#endif


#ifdef PLSN_DEBUG
	#ifdef PRINT_TO_USB
		#define PLSN_PRINTF(fmt,args...)  {usb_log_lock(); sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff); usb_log_unlock();}
	#else
		#define PLSN_PRINTF(fmt,args...)  libc_printf("PLSN_PRINTF: " fmt, ##args)  
	#endif  
#else
    #define PLSN_PRINTF(...)
#endif


#ifdef NIM_PRINTF_DEBUG
	#ifdef PRINT_TO_USB
		#define NIM_PRINTF(fmt,args...)  {usb_log_lock(); sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff); usb_log_unlock();}
	#else
		#define NIM_PRINTF(fmt,args...)  libc_printf("NIM_PRINTF: " fmt, ##args)  
	#endif  
#else
    #define NIM_PRINTF(...)
#endif


#ifdef MON_PRINTF_DEBUG 
	#ifdef PRINT_TO_USB
		#define MON_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define MON_PRINTF(fmt,args...)  libc_printf("MON_PRINTF: " fmt, ##args)  
	#endif 	
#else
    #define MON_PRINTF(...)
#endif


#ifdef ERR_DEBUG
	#ifdef PRINT_TO_USB
		#define ERR_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define ERR_PRINTF(fmt,args...)  libc_printf("ERR: " fmt, ##args)  
	#endif 
#else
	#define ERR_PRINTF(...)
#endif

#ifdef FFT_PRINTF_DEBUG
	#ifdef PRINT_TO_USB
		#define FFT_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define FFT_PRINTF(fmt,args...)  libc_printf("FFT: " fmt, ##args)  
	#endif 
#else
	#define FFT_PRINTF(...)
#endif

#ifdef HW_ADPT_CR_MONITOR
	#ifdef PRINT_TO_USB
		#define ADPT_CR_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define ADPT_CR_PRINTF(fmt,args...)  libc_printf("ADPT_CR_PRINTF: " fmt, ##args)  
	#endif 
#else
	#define ADPT_CR_PRINTF(...)
#endif

#ifdef HW_ADPT_NEW_CR_MONITOR
	#ifdef PRINT_TO_USB
		#define ADPT_NEW_CR_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define ADPT_NEW_CR_PRINTF(fmt,args...)  libc_printf("ADPT_NEW_CR_PRINTF: " fmt, ##args)  
	#endif 
#else
	#define ADPT_NEW_CR_PRINTF(...)
#endif

#ifdef AUTOSCAN_DEBUG
	#ifdef PRINT_TO_USB
		#define AUTOSCAN_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define AUTOSCAN_PRINTF(fmt,args...)  libc_printf("AUTOSCAN_PRINTF: " fmt, ##args)  
	#endif 
#else
	#define AUTOSCAN_PRINTF(...)
#endif

#ifdef ACM_PRINTF
	#ifdef PRINT_TO_USB
		#define ACM_DEBUG_PRINTF(fmt,args...)  {usb_log_lock();sprintf(tmp_buff,fmt,##args); nim_usb_log(tmp_buff);usb_log_unlock();}
	#else
		#define ACM_DEBUG_PRINTF(fmt,args...)  libc_printf("ACM_PRINTF: " fmt, ##args)  
	#endif 
#else
	#define ACM_DEBUG_PRINTF(...)	
#endif


#ifdef __LINUX__
typedef struct NIM_AUTO_SCAN         NIM_AUTO_SCAN_T;
typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;
#define OSAL_ID UINT32
#define ID WORD
#define ER long 
#endif


struct nim_c3505_plsn
{
	UINT8  super_scan; 		//set by upper level,  control whether do super scan, 1:super scan, 0:nomal scan
	UINT8  start_search;	//set by driver,  if the TP is config, set start_search to 1, super_scan and start_search control whether start plsn search
	UINT8  auto_scan_start;
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
	UINT8  generate_table_happened_flag; //in order to  control the relese table must happen after generate table happen
};

struct nim_c3505_tsk_status
{
	UINT32 m_lock_flag;
	ID m_task_id;
	UINT32 m_sym_rate;
	UINT8 m_work_mode;
	UINT8 m_map_type;
	UINT8 m_code_rate;
	UINT8 m_info_data;  // lock status 1=lock, 0=unlock
};

struct nim_c3505_t_param
{
	int t_last_snr;		
	int t_last_iter;
	int t_aver_snr;
	int t_snr_state;
	int t_snr_thre1;
	int t_snr_thre2;
	int t_snr_thre3;
	INT32 t_phase_noise_detected;
	INT32 t_dynamic_power_en;
	UINT32 phase_noise_detect_finish;
	UINT32 t_reg_setting_switch;
	UINT8 t_i2c_err_flag;
    UINT8 auto_adaptive_cnt;
    UINT8 auto_adaptive_state;
    UINT8 auto_adaptive_doing;
    UINT32 freq_offset;

};

struct nim_c3505_status
{
	ID nim_c3505_sema;
	ER ret;
	UINT8 c3505_autoscan_stop_flag;
	UINT8 c3505_chanscan_stop_flag;
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
	pfn_nim_reset_callback m_pfn_reset_c3505;
	UINT8 m_enable_dvbs2_hbcd_mode;
	UINT8 m_dvbs2_hbcd_enable_value;
	UINT8 c3505_lock_status;
	UINT8 c3505_lock_adaptive_done;  //In order to fix lowfeq mosaic
	UINT32 phase_err_check_status;
	UINT32 m_c3505_type;
	UINT32 m_c3505_sub_type;
	UINT32 m_setting_freq;
	UINT32 m_err_cnts;
	UINT16 m_hw_timeout_thr;
};

#ifdef __C3505_TDS__
#define NIM_FLAG_LOCK UINT32
#endif
struct nim_c3505_private
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
	UINT8 autoscan_debug_flag;  // AUTOSCAN_DEBUG_FLAG
	UINT8 autoscan_control;	//0:normal autoscan, 1:get valid freq, 2:get 64K spectrum,just execute half autoscan
							//attention: you should set the variable base on the function before call nim_s3503_autoscan
	
#ifdef __C3505_TDS__
	struct nim_device_stats stats;
#endif
	
	UINT8 chip_id;
	struct EXT_DM_CONFIG ext_dm_config;
	struct nim_c3505_status ul_status;
	INT32 ext_lnb_id;
	INT32 (*ext_lnb_control) (UINT32, UINT32, UINT32);
	struct nim_c3505_tsk_status tsk_status;
	struct nim_c3505_t_param t_param;
	UINT32 cur_freq;
	UINT32 cur_sym; 
    UINT8 change_type; // For ACM multi stream, 0 = Set TP and get ISID, 1 = Set TP and set ISID, 2 = Onyl set ISID
	NIM_FLAG_LOCK flag_id;
	enum NIM_BLSCAN_MODE blscan_mode;
	OSAL_ID c3505_mutex;
	
	// For ACM used
	struct nim_dvbs_channel_info *channel_info;
	struct nim_dvbs_bb_header_info *bb_header_info;
	struct nim_dvbs_isid *isid;

	struct nim_c3505_plsn plsn;
    OSAL_ID scan_mutex;
	ID plsn_task_id;//add by robin,20170216
	ID generate_table_task_id;//add by Ziv ,20170319
	
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
    struct nim_tso_cfg nim_tso_cfg;
	//1:channel change or autoscan is working now;0:channel change or autoscan finished
	UINT8 							chanscan_autoscan_flag;
	//1:function waiting_channel_lock finished;0:function waiting_channel_lock is working now
	UINT8							wait_chanlock_finish_flag;
}; 

struct nim_c3505_tp_scan_para
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
extern UINT32 		nim_c3505_dvbs_as_cb2_ui(void *p_priv, unsigned char lck, unsigned char polar, unsigned short freq, 
                               unsigned int sym, unsigned char fec, unsigned char as_stat);
                               
extern INT32 		nim_callback(NIM_AUTO_SCAN_T *pst_auto_scan, void *pfun, UINT8 status, UINT8 polar, 
                             UINT16 freq, UINT32 sym, UINT8 fec, UINT8 stop); 
#endif

extern UINT8  *dram_base_t;
                              
//---------------System Essensial Function --------------//
DWORD nim_c3505_multu64div(UINT32 v1, UINT32 v2, UINT32 v3);
INT32 nim_c3505_crc8_check(void *input, INT32 len,INT32 polynomial);
UINT32 nim_c3505_Log10Times100_L( UINT32 x);
INT32 nim_c3505_attach(struct QPSK_TUNER_CONFIG_API *ptr_qpsk_tuner);
INT32 nim_c3505_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
INT32 nim_c3505_ioctl_ext(struct nim_device *dev, INT32 cmd, void *param_list);
#ifdef __C3505_TDS__
INT32 nim_c3505_task_init(struct nim_device *dev);
#endif
void nim_c3505_task(UINT32 param1, UINT32 param2);
void nim_c3505_plsn_task(UINT32 param1, UINT32 param2);//add by robin,20170216
void nim_c3505_generate_table_task(UINT32 param1, UINT32 param2);//add by ziv,20170319

INT32 nim_c3505_open(struct nim_device *dev);
INT32 nim_c3505_close(struct nim_device *dev);

extern INT32 nim_c3505_generate_table(struct nim_device *dev);
extern INT32 nim_c3505_start_generate(struct nim_device *dev, struct ali_plsn_address *p_addr);
extern INT32 nim_c3505_release_table(struct nim_device *dev);
extern INT32 nim_c3505_search_plsn_top(struct nim_device *dev);
extern INT32 nim_c3505_plsn_gold_to_root(INT32 plsn_gold);
extern INT32 nim_c3505_plsn_root_to_gold(INT32 plsn_root);

extern UINT8 g_table_finish_flag;

#endif	// __LLD_NIM_DVBS_C3505_TDS_H__ */

