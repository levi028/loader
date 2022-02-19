#ifndef _LOADER_DB_H_
#define _LOADER_DB_H_

#ifdef __cplusplus
extern "C" {
#endif
#define GACAS_BOOT_VER 			"Ali3711c_128M_BOOT_V1.1"
#define GACAS_LOADER_VER		"Ali3711c_128M_LOADER_V1.1"

#define GACAS_BOOT_CHUNK_ID         	0x30820122
#define GACAS_BOOT_CHUNK_OFFSET     	0x80000


#define GACAS_OTP_KEY_INDEX				2
#define GACAS_FLASH_BLOCK_SIZE			0x10000
#define GACAS_FLASH_TOTAL_SIZE			0x800000//0x800000
#define GACAS_MAX_CHUNK_NUM        		32
#define GACAS_DEFAULT_CHUNK_NUM        	17
#define GACAS_LOADER_FLASH_ADDR			0xC0000
#define GACAS_LOADER_DB_FLASH_ADDR		0x510000	// 0x400000 + 0x100000 + 0xFF80 + 0x80
#define GACAS_LOADER_DB_STORE_NUM		32
#define GACAS_LOADER_DB_FLAG			0x123456AB

#define CHUNK_ID_LOADER        			0x00FF0100
#define CHUNK_ID_LOADER_SEE        		0x07F80000
#define CHUNK_ID_LOADER_DB        		0x0AF50100

#define CHUNK_ID_MAIN_CODE        		0x01FE0101
#define CHUNK_ID_MAIN_CODE_SEE		0x06F90101
#define CHUNK_ID_LOGO_BOOT        		0x02FD0100
#define CHUNK_ID_LOGO_MAIN        		0x02FD0200
#define CHUNK_ID_LOGO_UPG        		0x02FD0300

#define CHUNK_ID_USER_DATA        		0x05FA0100
#define CHUNK_ID_DB        				0x04FB0100


/* HDCP protect key OTP 地址 */
#define OTP_ADDR_HDCP_PROTECT_KEY   0x59
#define OTP_HDCP_PROTECT_KEY_LEN    	16


#define DEFAULTE_FLASH_BURN_ADDR     	0x200000
#define DEFAULTE_FLASH_BURN_SIZE    	0x300000

#define DEFAULTE_USER_DATA_ADDR     	0x540000
#define DEFAULTE_USER_DATA_SIZE    	0x180000

typedef enum 
{
	GACAS_LOADER_TYPE_OTA_MANNAL = 0, 	
	GACAS_LOADER_TYPE_OTA_FORCE, 	
	GACAS_LOADER_TYPE_OTA_BOOT, 	
	GACAS_LOADER_TYPE_USB_MANNAL, 		
	GACAS_LOADER_TYPE_USB_BOOT, 		
	GACAS_LOADER_TYPE_BUTT, 		
} GACAS_LOADER_TYPE_E;
/**TUNER signal type*/
typedef enum 
{
	GACAS_TUNER_SIG_TYPE_CAB = 1,     	/**<Cable signal*//**<CNcomment: 有线信号 */
	GACAS_TUNER_SIG_TYPE_SAT = 2,     	/**<Satellite signal*//**<CNcomment: 卫星信号 */
	GACAS_TUNER_SIG_TYPE_TER = 4,	/**<Terrestrial signal*//**<CNcomment: 地面信号 */
	GACAS_TUNER_SIG_TYPE_BUTT          	/**invalid type*//**<CNcomment: 无效信号类型 */
}GACAS_TUNER_SIG_TYPE_E;
/**Parameter for DiSEqC 1.0/2.0 switch
   Some DiSEqC device need set polarization(13/18V) and 22K, you can set them here. */

/**CNcomment:DiSEqC 1.0/2.0 开关参数有些DiSEqC设备需要设置极化方式和22K的，如果用了这种设备，需要在这里设置 */
typedef struct hiTunerSwitch4Port_S
{
	UINT32 u32_level;  			/**<DiSEqC device level*//**<CNcomment: 级别 */
	UINT32 u32_port;   			/**<DiSEqC switch port*//**<CNcomment:  开关端口*/
	UINT32 u32_polar;  		/**<Polarization type *//**<CNcomment:  极化方式*/
	UINT32 u32_LNB_22K; 		/**<22K status*//**<CNcomment: 22K开关状态 */
} GACAS_TUNER_SWITCH4PORT_S;

/**Parameter for DiSEqC 1.1/2.1 switch */
/**CNcomment:DiSEqC 1.1/2.1 开关参数 */
typedef struct hiTunerSwitch16PORT_S
{
	UINT32 u32_level;          	/**<DiSEqC device level*//**<CNcomment: 级别 */
	UINT32 u32_port;           	/**<DiSEqC switch port*//**<CNcomment: 开关端口 */
} GACAS_TUNER_SWITCH16PORT_S;

/*Define Cable tuner parameter */
typedef struct tagLoaderCabPara
{
	UINT32 u32_freq_kHz;         	/**< frequency pointer in kHz, for example, 403000. *//**<CNcomment: 频点，单位 kHz */
	UINT32 u32_symb_kbps;     	/**< symbol rate in kbps, for example, 6875. *//**<CNcomment: 符号率，单位 kbps */
	UINT32 u32_modulation;   	/**< constellation count.0:16QAM 1:32QAM 2:64QAM 3:128QAM 4:256QAM  *//**<CNcomment: QAM方式，0:16QAM 1:32QAM 2:64QAM 3:128QAM 4:256QAM  */
} GACAS_TUNER_CAB_PARA_T;

/*Define Terrestrial tuner parameter */
typedef struct tagLoaderTerPara
{
	UINT32 u32_freq_kHz;         	/**< frequency pointer in kHz, for example, 177500. *//**<CNcomment: 频点，单位 kHz */
	UINT32 u32_band_width_kHz; 	/**< BandWidth in kHz, for example, 7000. *//**<CNcomment: 带宽，单位 kHz */
	UINT32 u32_modulation;   	/**< constellation count.0:16QAM 1:32QAM 2:64QAM 3:128QAM 4:256QAM  *//**<CNcomment: QAM方式，0:16QAM 1:32QAM 2:64QAM 3:128QAM 4:256QAM  */
} GACAS_TUNER_TER_PARA_T;
#ifdef DEF_SAT_BY_GOS
/*Define Sat tuner parameter */
typedef struct tagLoaderSatPara
{
	UINT32 u32_freq_kHz;         	/**< frequency pointer in kHz, for example, 403000. *//**<CNcomment: 频点，单位 kHz */
	UINT32 u32_symb_kbps;     	/**< symbol rate in kbps, for example, 27500. *//**<CNcomment: 符号率，单位 kbps */
	UINT32 u32_polar;          	/**<Polarization type:
	                                             0:Horizontal Polarization
	                                             1:Vertical Polarization
	                                             2:Left-hand circular polarization
	                                             3:Right-hand circular polarization>*/
	
	UINT32 u32_low_LO;           	/**< Low Local Oscillator Frequency, MHz *//**<CNcomment: 低本振 */
	UINT32 u32_high_LO;          	/**< High Local Oscillator Frequency, MHz*//**<CNcomment: 高本振 */

	/*support kinds of switch, support class-relation*/
	/**CNcomment:支持各种开关，支持级联 */
	GACAS_TUNER_SWITCH16PORT_S st_16port;
	GACAS_TUNER_SWITCH4PORT_S  st_4port;
	UINT32 	u32_switch_22K; /** 22KHz switch *//**<CNcomment: 22K开关 */
	UINT32	u32_LNB_power;/*LNB Power*//**<CNcomment: LNB供电控制 */
} GACAS_TUNER_SAT_PARA_T;
#else
typedef struct tagLoaderSatPara
{
	/**********************SAT INFO***********************/
	UINT16 sat_id;
	UINT16 reserve_id;              //4

	UINT16 sat_orbit;               //2
	UINT16 selected_flag   : 1;
	UINT16 tuner1_valid    : 1;
	UINT16 tuner2_valid    : 1;
	UINT16 sat2ip_flag      : 1;     //2
	UINT16 reserve_1        :12;     //2

	//tuner1 antenna setting*****
	UINT8 lnb_power         : 1;
	UINT8 lnb_type          : 7;

	UINT8 pol               : 2;
	UINT8 k22              	: 1;
	UINT8 v12               	: 1;
	UINT8 toneburst         : 2;
	UINT8 unicable_pos 	: 1;
	UINT8 reserve_2    	: 1;       //2
	UINT16 reserve	;     //2

	UINT16 lnb_low;
	UINT16 lnb_high;                    //4

	UINT8 di_seq_c_type       : 4;
	UINT8 di_seq_c_port       : 4;

	UINT8 di_seq_c11_type     : 4;
	UINT8 di_seq_c11_port     : 4;

	UINT16 positioner_type  : 3;
	UINT16 position         : 8;
	UINT16 reserve_3        	: 3;        //4
	UINT16 pol_tp                	: 2;
	//*************************
	UINT16 reference_chan1;
	UINT16 reference_chan2;
	/*************TP info***************/
	UINT32 frq;
	UINT32 sym;
#if 0
	UINT16 pol_tp                  : 2;
	UINT16 fec_inner            : 4;  //T: 0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:7/8  //T2: 5:3/5, 6:4/5 //0xF:unknown
	UINT16 universal_22k_option : 1;
	UINT16 big5_indicator       : 1;
	UINT16 ft_type              : 2;  //front end type,check lib_frontend.h
	UINT16 inverse              : 1;
	UINT16 band_type            : 1;
	UINT16 priority             : 2;  //for DVB-T HIERARCHY mode
	UINT16 hier                 : 2;

	UINT16 preset_flag          : 2;
	UINT16 usage_status         : 1;
	UINT16 nit_pid              :13;

	UINT32 t_s_id               :16;
	UINT32 network_id           :16;  //original_network_id

	UINT32 net_id               :16;  //network_id
	UINT32 t2_signal            : 1;  //0:DVB-T signal, 1:DVB-T2 signal. (when ft_type==FRONTEND_TYPE_T)
	UINT32 plp_index            : 8;
	UINT32 t2_profile			: 2;
	UINT32 reserved             : 5;

	UINT32 sdt_version          : 8;
	UINT32 t2_system_id         :16;
	UINT32 plp_id               : 8;

	//T2: 128:1/128, (19+128):19/128, 19:19/256, //0xFF:unknown
	UINT32 guard_interval       : 8;    //4: 1/4, 8: 1/8, 16: 1/16, 32:1/32
	UINT32 FFT                  : 8;    //2:2k, 8:8k //T2: 1:1k, 4:4k, 16:16k, 32:32k, //0xFF:unknown

	//T2: (64+1):256 QAM, //0xFF:unknown
	UINT32 modulation           : 8;    //2:DQPSK 4:QPSK, 16:16 QAM, 64:64 QAM
	UINT32 bandwidth            : 8;
#endif
}GACAS_TUNER_SAT_PARA_T;
#endif
typedef struct
{
	UINT32 id;
	UINT32 addr;
	UINT32 ver;
	UINT32 offset;
	UINT32 len;
	UINT32 sign_len;
	UINT32 crc32;
	BOOL   b_update;
}LOADER_CHUNK;

typedef struct
{
	UINT32 	u32_flag_start;
	UINT32 	u32_loader_index;		/**是否使用备份loader*/
	UINT8 	u8_type;			//GACAS_LOADER_TYPE_E
	UINT8 	u8_times;
    UINT16 	u16_pid;         		/**< TS PID. The parameter is vital and set it with caution..*//**<CNcomment: PID */

	//但未指定升级chunk_id时，则按地址升级
	UINT32 	u32_flash_addr;		/**<flash_addr for loader*//**<CNcomment: 升级文件flash开始地址*/	
	UINT32 	u32_flash_len;		/**<flash_len for loader*//**<CNcomment: 升级文件长度*/	

	UINT32 	reserved_0;	
	UINT32 	u32_smc_id_bak;	
	UINT32 	reserved_2	:31;	
	UINT32 	b_first_time	:1;	
	UINT32 	reserved_len;
    #if 0
	UINT8 	reserved[16];	
    #else
	UINT8 	reserved[32];	
    #endif
	UINT32 	u32_key_up;	
	UINT32 	u32_key_down;	
	UINT32 	u32_key_left;	
	UINT32 	u32_key_right;	
	UINT32 	u32_smc_id;	
	UINT32 	u32_run_area;	
	UINT32 	u32_crc32_upg;	
	UINT8 	u8_nit_ver;
	UINT8 	u8_usb_log_out;
	UINT8    	tv_mode           : 5; // enum TV_SYS_TYPE
	UINT8   	tv_ratio            : 3;
	UINT8   	display_mode   : 3;
	UINT8    	scart_out         : 3;
	UINT8    	default_mode	: 2;//0 user set,1 sd mode ,2 hd mode
	UINT32 	u32_sig_type;					/**<Signal type*//**<CNcomment: 信号类型 */
	GACAS_TUNER_CAB_PARA_T 	st_cab;       	/**<Cable signal parameter *//**<CNcomment: 有线信号参数 */
	GACAS_TUNER_SAT_PARA_T 	st_sat;        	/**<Satellite signal parameter*//**<CNcomment: 卫星信号参数 */
	GACAS_TUNER_TER_PARA_T 	st_ter;        	/**<Terrestrial signal parameter*//**<CNcomment: 地面信号参数 */

	GACAS_TUNER_CAB_PARA_T 	st_centrt_cab[2];       	/**<Cable signal parameter *//**<CNcomment: 有线信号参数 */
	GACAS_TUNER_SAT_PARA_T 	st_centrt_sat[2];        	/**<Satellite signal parameter*//**<CNcomment: 卫星信号参数 */
	GACAS_TUNER_TER_PARA_T 	st_centrt_ter[2];        	/**<Terrestrial signal parameter*//**<CNcomment: 地面信号参数 */

	UINT32 	u32_file_id_new;			
	UINT32 	u32_file_id;			
	UINT16 	u16_chunk_use;			
	UINT16 	u16_chunk_num;			
	LOADER_CHUNK	st_chunk[GACAS_MAX_CHUNK_NUM];
	UINT32 	u32_crc32;
	UINT32 	u32_flag_end;
}GACAS_LOADER_DB_T;

typedef struct
{
	UINT16 	water_mark_x;
	UINT16 	water_mark_y;
	UINT8   	scan_mode;
	UINT8   	order_mode;
	UINT8   	time_area;
	UINT8   	water_apha;
	GACAS_TUNER_CAB_PARA_T center_tp[2];
	UINT32   reserve32[7];
	UINT32 	crc32;
}GOS_DEFAULT_INFO_T;//add by yuj for default config tool
UINT32 	gacas_loader_str2uint32(UINT8 *str, UINT8 len);
INT32 	gacas_loader_db_save(void);
BOOL 	gacas_loader_db_is_chunk_use(void);
BOOL 	gacas_loader_db_is_usb(void);
BOOL 	gacas_loader_db_is_ota_force(void);
BOOL 	gacas_loader_db_is_upg(void);
BOOL 	gacas_loader_db_is_back_up(void);
void 		gacas_loader_db_deal_one_time(UINT8 type);
void 		gacas_loader_db_ota_success(void);
BOOL 	gacas_loader_db_is_ota_by_addr(UINT32 *p_addr,UINT32 *p_size);
INT32 	gacas_loader_db_clear_chunk_update(void);
INT32 	gacas_loader_db_set_chunk(LOADER_CHUNK *p_chuck,UINT16 num);
INT32 	gacas_loader_db_get_chunk(LOADER_CHUNK *p_chuck,UINT16 *p_num);
INT32 	gacas_loader_db_get_ota_cab(UINT16 *p_pid,UINT32 *p_freq,UINT32 *p_symbol,UINT32 *p_mod);
INT32 	gacas_loader_db_set_ota_cab(UINT16 pid,UINT32 freq,UINT32 symbol,UINT32 mod);
UINT32 	gacas_loader_db_get_chip_id(void);
UINT32 	gacas_loader_db_get_file_id_new(void);
UINT32 	gacas_loader_db_get_file_id(void);
INT32 	gacas_loader_db_set_file_id_new(UINT32 file_id);
INT32 	gacas_loader_db_get_centrt_tp(UINT8 tp_index,UINT32 	u32_sig_type,void*p_info);
INT32 	gacas_loader_db_set_centrt_tp(UINT8 tp_index,UINT32 	u32_sig_type,void*p_info);
INT32	gacas_loader_db_get_reserved(UINT8 *p_data,UINT32 *p_len);
INT32 	gacas_loader_db_set_reserved(UINT8 *p_data,UINT32 len);
INT32 gacas_loader_db_clear_reserved(void);
INT32 gacas_loader_db_is_first_time(void);
void 		gacas_loader_db_set_key(UINT32 u32_key_up,UINT32 u32_key_down,UINT32 u32_key_left,UINT32 u32_key_right);
INT32 	gacas_loader_db_get_key(UINT32 *p_key_up,UINT32 *p_key_down,UINT32 *p_key_left,UINT32 *p_key_righ);
UINT32 	gacas_loader_db_get_run_area(void);
void 		gacas_loader_db_set_run_area(UINT32 run_area);
UINT32 	gacas_loader_db_get_nit_ver(void);
void 		gacas_loader_db_set_nit_ver(UINT8 nit_ver);
UINT8 	gacas_loader_db_get_usb_log(void);
void 		gacas_loader_db_set_usb_log_out(UINT8 usb_log_out);
UINT32 	gacas_loader_db_get_crc32_upg(void);
void 		gacas_loader_db_set_crc32_upg(UINT32 crc32_upg);
UINT32 	gacas_loader_db_get_smc_id(void);
void 		gacas_loader_db_set_smc_id(UINT32 smc_id);
INT32 	gacas_loader_db_set_av_default_mode(UINT8 default_mode);
UINT8 	gacas_loader_db_get_av_default_mode(void);
INT32 	gacas_loader_db_set_av(UINT8 tv_mode, UINT8 tv_ratio, UINT8 display_mode, UINT8 scart_out);
INT32 	gacas_loader_db_get_av(UINT8 *p_tv_mode, UINT8 *p_tv_ratio, UINT8 *p_display_mode, UINT8 *p_scart_out);
INT32 	gacas_loader_db_set_chunk_update(UINT32 id);
INT32 	gacas_loader_db_set_type(UINT8 type,UINT32 start_addr,UINT32 size,UINT8 times);
INT32   	gacas_loader_db_get_chunk_len(UINT32 id, UINT32 *addr, UINT32 *len,UINT32 *offset);
INT32 	gacas_loader_check_center_tp_cab(UINT8 tp_index);
INT32 	gacas_loader_check_center_tp_is_same(void);//add by yuj for default config tool
INT32 	gacas_loader_get_centrt_tp_cab(UINT8 tp_index, UINT32 *p_freq,UINT32 *p_symbol,UINT32 *p_mod);
INT32 	gacas_loader_set_centrt_tp_cab(UINT8 tp_index, UINT32 freq,UINT32 symbol,UINT32 mod);
void 		gacas_loader_db_deal_one_time_all(void);
void 		gacas_loader_db_set_index(UINT32 index);
INT32 	gacas_loader_db_init(void);
void 		gacas_loader_get_boot_len(UINT32 *p_boot_len);
INT32 	gacas_loader_flash_lock(UINT32 addr,UINT32 len);
INT32 	gacas_loader_flash_unlock(UINT32 addr,UINT32 len);
//add by yuj for default config tool -s
INT32 gacas_loader_check_center_tp_is_home_freq(void);
void gacas_loader_reset_home_freq(void);
INT32 gacas_loader_get_default_home_tp_cab(UINT8 tp_index, UINT32 *p_freq,UINT32 *p_symbol,UINT32 *p_mod);
UINT8 gacas_loader_get_default_scan_mode(void);
UINT8 gacas_loader_get_default_order_mode(void);
UINT8 gacas_loader_get_default_time_area(void);
INT32 gacas_loader_get_default_water_mark_xy(UINT16 *p_x,UINT16 *p_y);
UINT8 gacas_loader_get_default_water_apha(void);
void gacas_loader_default_info_init(UINT8 *p_data,UINT16 len);
//add by yuj for default config tool -e
#ifdef __cplusplus
 }
#endif

#endif

