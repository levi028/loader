/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pmu.h
*
*    Description: This file is provide api for pmu modues .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _PMU_H_
#define _PMU_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <bus/rtc/rtc.h>
//=====================================================================================================================//

//#define PMU_STANDBY_DEBUG_EN
#define PMU_MAX_NAME_SIZE (16)/*Max device name length.*/
// These values identify some different wakeup reasons about WDT.
// Set-top box started as a result of a reboot from the watch dog, 0x52454254 'REBT'
#define ALISL_PWR_WDT (0x52454254)
// Set-top box started as a result of a reboot requested, 0x424F4F54 'BOOT'
#define ALISL_PWR_REBOOT (0x424F4F54)
// Set-top box started as a result of wake up form standby mode, 0x57414B45 'WAKE'
#define ALISL_PWR_STANDBY (0x57414B45)

#define WDT_REBOOT_FLAG_SAVE_ADDR (0xB8053FAC)
#define WDT_REBOOT_FLAG_READ_ADDR (0xB8053FA8)


/*io cmd.*/
enum _PMU_IO_CMD
{
	PMU_SHOW_TYPE_CMD = 0,
	PMU_LOAD_BIN_CMD,
	PMU_RESUME_STATUS,
	PMU_PANNEL_POWER_STATUS,
	PMU_POWER_ON_STATUS,
	ALI_PMU_SAVE_WDT_REBOOT_FLAG,
	ALI_PMU_GET_WDT_REBOOT_FLAG,
};

//=====================================================================================================================//

typedef enum
{
	IR_EXIT = 0,
	KEY_EXIT,
	CEC_EXIT,
	RTC_EXIT,
	UNKNOWN_TYPE = 0xFF,
}PMU_EXIT_FLAG, *p_pmu_exit_flag;

typedef enum
{
	COLD_BOOT=0,
	PMU_RESUME,
}PMU_RESUME_FLAG,*p_pmu_resume_flag;

typedef enum
{
	SHOW_OFF = 0,
	SHOW_TIME,
	SHOW_BANK,
	SHOW_DEFAULT,
	SHOW_NO_CHANGE,/*just keep panel on, do not changed the content.*/
}PANNEL_SHOW_TYPE;

typedef enum
{
	PANNLE_POWER_ON =0,
	PANNEL_POWER_OFF,
}PANNEL_POWER,*p_pannel_power;

enum  PANEL_TYPE
{
	PAN_TYPE_CH455 = 0,
	PAN_TYPE_FD650K,
	PAN_TYPE_FD650K_ISP,
};

typedef struct
{
	UINT8 *buf;
	UINT32 len;
}PMU_BIN,*p_pmu_bin;

struct pmu_device
{
	struct pmu_device *next;
	UINT32 type;
	UINT8 name[PMU_MAX_NAME_SIZE];
	UINT32 flags;
	UINT32 hardware;
	UINT32 busy;
	UINT32 minor;

	void *priv;
	UINT32 base_addr;
};

/*! @struct pmu_smc_call_para
@brief It defines paras when cpu send it from Main to See.
*/
struct pmu_smc_call_para
{
	unsigned char smc_current_year_h;
	unsigned char smc_current_year_l;
	unsigned char smc_current_month;
	unsigned char smc_current_day;
	unsigned char smc_current_hour;
	unsigned char smc_current_min;
	unsigned char smc_current_sec;
    
	unsigned char smc_wakeup_month;
	unsigned char smc_wakeup_day;
	unsigned char smc_wakeup_hour;
	unsigned char smc_wakeup_min;
	unsigned char smc_wakeup_sec;

	unsigned char pmu_powerup_type;
	unsigned char panel_power_key;
	unsigned char show_panel_type;
	unsigned char reserved_flag1;

	unsigned char reserved_flag2;
	unsigned char reserved_flag3;
	unsigned char reserved_flag4;
	unsigned char reserved_flag5;

	unsigned char reserved_flag6;
	unsigned char reserved_flag7;
	unsigned char reserved_flag8;
	unsigned char reserved_flag9;

	unsigned int ir_power_key1;
	unsigned int text_seg_start_addr;
	unsigned int text_seg_end_addr;
	unsigned int mcu_code_start_phys;

	unsigned int mcu_code_len;
	unsigned int ir_power_key2;
	unsigned int wdt_reboot_flag;
	unsigned int kernel_str_params_addr;

	unsigned int kernel_str_params_len;
	unsigned int mcu_uart_support;
	unsigned int reserved_flag10;
	unsigned int reserved_flag11;

	unsigned int reserved_flag12;
	unsigned int reserved_flag13;
	unsigned int reserved_flag14;
	unsigned int reserved_flag15;

	unsigned int reserved_flag16;
	unsigned int reserved_flag17;
	unsigned int reserved_flag18;
	unsigned int reserved_flag19;

	unsigned int mcu_write_data;
	unsigned int mcu_read_data;
	unsigned int mcu_rw_offset;
};

/*! @enum PMU_RPC_NO
@brief It defines pmu rpc type.
*/
enum PMU_RPC_NO {
	STB_ENTER_PMU_STANDBY = 0x52504301,/*'R'-0x52, 'P'-0x50, 'C'-0x43.*/
	ENABLE_MCU_UART,
	GET_PMU_MCU_TIME,
	SUSPEND_MCU,
	SET_SHOW_PANEL_TYPE,
	GET_POWERUP_TYPE,
	SET_WAKEUP_TIME,
	SET_CURRENT_TIME,
	SET_PANEL_POWER_KEY,
	SET_IR_POWER_KEY,
	SAVE_WDT_REBOOT_FLAG,
	GET_WDT_REBOOT_FLAG,
	GET_WDT_REBOOT_FLAG_FIRST,
	RPC_MCU_WRITE8,
	RPC_MCU_WRITE16,
	RPC_MCU_WRITE32,
	RPC_MCU_READ8,
	RPC_MCU_READ16,
	RPC_MCU_READ32,
	KERNEL_STR_PARAM_BACKUP,
	SEE_CLK_PRECISION_TEST,
	RETENTION_PARAMS_DUMP,
	SEE_ENTER_STR_SUSPEND,
};

extern struct pmu_smc_call_para *pmu_rpc_para;
//=====================================================================================================================//

RET_CODE pmu_m36_attach(void (*notify)(UINT32 ), UINT32 param);
RET_CODE pmu_io_control(struct pmu_device *dev, UINT32 cmd, UINT32 param);
RET_CODE pmu_m36_en(void);
RET_CODE pmu_m36_ir_init(void);
void pmu_panel_type(UINT8 pmu_pan_type);
void pmu_key_transfrom_extend(UINT64 power_key_fst, UINT8 ir_type,
	UINT64 power_key_snd, UINT8 ir_type_sec, UINT32 *ext_reserved);
void rtc_s3811_set_tick_msk(void);
int pmu_mcu_enter_stby_timer_set_value(struct rtc_time *base_time);
INT32 rtc_s3811_set_value(struct rtc_time *base_time);
void south_rtc_init(UINT32 init);
struct rtc_time *rtc_time_read_init_value(void);
int pmu_mcu_wakeup_timer_set_min_alarm(struct min_alarm *alarm);
RET_CODE  pmu_resume_status(p_pmu_resume_flag status);
void hld_pmu_remote_call(UINT32 mcu_src_addr, UINT32 mcu_code_len,
	UINT32 pmu_rpc_no, struct pmu_smc_call_para *pmu_params);
void pmu_disable_all_interrupt(void);
void main_cpu_read_time_init(struct pmu_smc_call_para *pmu_params);
#endif

