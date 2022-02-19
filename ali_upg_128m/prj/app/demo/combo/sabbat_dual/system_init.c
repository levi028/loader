/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: system_init.c
*
*    Description: The file is for system HW init when boot up.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <api/libdbc/lib_dbc.h>
#include <bus/rtc/rtc.h>
#include <hld/pmu/pmu.h>
#include <version/ver.h>
#include "head_file.h"
#include "platform/board.h"
#include "platform/board_cfg/board_cfg.h"
#include "win_com.h"
#include "copper_common/com_api.h"
#include "control.h"
#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_block.h>
#endif
#ifdef AUTO_OTA
#include "ota_ctrl.h"
#endif

#ifdef USE_EXT_GPIO_CT1642
#include <bus/ct1642/ct1642.h>
#endif
/******************************************************************************************************/
//new or change add by yuj
#define GOS_RELEASE_TIME		__TIME__" "__DATE__
#define WDT_REBOOT_FLAG_SAVE_ADDR (0xB8053FAC)
#define WDT_REBOOT_FLAG_READ_ADDR (0xB8053FA8)
#define ALI_WDT_REBOOT (0x52454254)/*0x52-'R', 0x45-'E', 0x42-'B', 0x54-'T'.*/
/******************************************************************************************************/

extern int m_boot_vpo_active;
extern void network_attach(void);
extern BOOL is_bootloader_show_logo_keeping_solution();
extern void ap_send_msg(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer);
extern void pmu_gpio_disable(void);
/******************************************************************************************************/

static void system_uart_port_init(void)
{
	UINT32 port_disbale = 0;

#ifdef _CAS9_CA_ENABLE_
#ifndef _BUILD_OTA_E_
	otp_init(NULL);
	otp_read(0x3*4, (UINT8 *)(&port_disbale), 4);
	if(port_disbale&(1<<12))
	{
		port_disbale = 1;
	}
	else
	{
		port_disbale = 0;
	}
#endif
#endif

	if(0 == port_disbale)
	{
		board_sci_init();
	}
}

#ifdef PMU_ENABLE
static void pmu_notify(UINT32 param __MAYBE_UNUSED__)
{
	if((ALI_S3811==sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1))
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_POWER, TRUE);
	#ifdef AUTO_OTA
		if(m_for_fake_standby )
		{
			ota_power_flag=TRUE;
		}
	#endif
	}

	if(((ALI_C3701==sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1))||(ALI_S3503==sys_ic_get_chip_id()))
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_POWER, TRUE);
	#ifdef AUTO_OTA
		if(m_for_fake_standby )
		{
		ota_power_flag=TRUE;
		}
	#endif
	}

	if((ALI_S3811==sys_ic_get_chip_id()) && (sys_ic_get_rev_id() == IC_REV_0))
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY_UI, V_KEY_POWER, TRUE);
	#ifdef AUTO_OTA
		if(m_for_fake_standby )
		{
			ota_power_flag=TRUE;
		}
	#endif
	}
}
#endif

static void system_pmu_init(void)
{
 #ifdef PMU_ENABLE
 	/*
 	**According to IC team's new design, C3702/C3922 MAIN cpu
 	**have not authority to access PMU module.
 	*/
	if((ALI_C3702 != sys_ic_get_chip_id()) && (ALI_C3922 != sys_ic_get_chip_id()))
 	{
		/*pmu sw/hw init.*/
		unsigned int __MAYBE_UNUSED__ nec_ir_key = STANDBY_IR_KEY;
		unsigned char ir_type = 0;
		unsigned char pmu_pan_type = 0x0;
		UINT64 nec_ir_key_first = 0x60DF708F;
		UINT64 nec_ir_key_second = 0;

		/*
		**PAN_TYPE_CH455 = 0.
		**PAN_TYPE_FD650K =1.
		*/
		pmu_m36_attach(pmu_notify, (UINT32)NULL);
		pmu_panel_type(pmu_pan_type);
		pmu_m36_ir_init();
		pmu_key_transfrom_extend(nec_ir_key_first, ir_type, nec_ir_key_second, ir_type, NULL);
		rtc_s3811_set_tick_msk();

	#ifdef PMU_RETENTION_SUPPORT
		DDR_Parameter_Backup_Before_Standby();
	#endif
	}
	else if(ALI_C3702 == sys_ic_get_chip_id())
	{
		/*Malloc buffer for struct pmu_rpc_para saving PMU RPC params.*/
		pmu_rpc_para = (struct pmu_smc_call_para *)malloc(sizeof(struct pmu_smc_call_para));

		/*Init PMU RPC params.*/
		pmu_rpc_para->reserved_flag1 = 1;
		pmu_rpc_para->reserved_flag2 = 2;
		pmu_rpc_para->reserved_flag3 = 3;
		pmu_rpc_para->reserved_flag4 = 4;
		pmu_rpc_para->reserved_flag5 = 5;
		pmu_rpc_para->reserved_flag6 = 6;
		pmu_rpc_para->reserved_flag7 = 7;
		pmu_rpc_para->reserved_flag8 = 8;
		pmu_rpc_para->reserved_flag9 = 9;
		pmu_rpc_para->reserved_flag10 = 10;
		pmu_rpc_para->reserved_flag11 = 11;
		pmu_rpc_para->reserved_flag12 = 12;
		pmu_rpc_para->reserved_flag13 = 13;
		pmu_rpc_para->reserved_flag14 = 14;
		pmu_rpc_para->reserved_flag15 = 15;
		pmu_rpc_para->reserved_flag16 = 16;
		pmu_rpc_para->reserved_flag17 = 17;
		pmu_rpc_para->reserved_flag18 = 18;
		pmu_rpc_para->reserved_flag19 = 19;

		/*IR power key is default inited as ALi IR power key.*/
		pmu_rpc_para->ir_power_key1 = STANDBY_IR_KEY;
		pmu_rpc_para->show_panel_type = 0;//Show OFF default

	#ifdef PMU_STANDBY_DEBUG_EN
		/*Dump all PMU RPC params.*/
		libc_printf("\npmu_rpc_para->reserved_flag1: 0x%08X", pmu_rpc_para->reserved_flag1);
		libc_printf("\npmu_rpc_para->reserved_flag2: 0x%08X", pmu_rpc_para->reserved_flag2);
		libc_printf("\npmu_rpc_para->reserved_flag3: 0x%08X", pmu_rpc_para->reserved_flag3);
		libc_printf("\npmu_rpc_para->reserved_flag4: 0x%08X", pmu_rpc_para->reserved_flag4);
		libc_printf("\npmu_rpc_para->reserved_flag5: 0x%08X", pmu_rpc_para->reserved_flag5);
		libc_printf("\npmu_rpc_para->reserved_flag6: 0x%08X", pmu_rpc_para->reserved_flag6);
		libc_printf("\npmu_rpc_para->reserved_flag7: 0x%08X", pmu_rpc_para->reserved_flag7);
		libc_printf("\npmu_rpc_para->reserved_flag8: 0x%08X", pmu_rpc_para->reserved_flag8);
		libc_printf("\npmu_rpc_para->reserved_flag9: 0x%08X", pmu_rpc_para->reserved_flag9);
		libc_printf("\npmu_rpc_para->reserved_flag10: 0x%08X", pmu_rpc_para->reserved_flag10);
		libc_printf("\npmu_rpc_para->reserved_flag11: 0x%08X", pmu_rpc_para->reserved_flag11);
		libc_printf("\npmu_rpc_para->reserved_flag12: 0x%08X", pmu_rpc_para->reserved_flag12);
		libc_printf("\npmu_rpc_para->reserved_flag13: 0x%08X", pmu_rpc_para->reserved_flag13);
		libc_printf("\npmu_rpc_para->reserved_flag14: 0x%08X", pmu_rpc_para->reserved_flag14);
		libc_printf("\npmu_rpc_para->reserved_flag15: 0x%08X", pmu_rpc_para->reserved_flag15);
		libc_printf("\npmu_rpc_para->reserved_flag16: 0x%08X", pmu_rpc_para->reserved_flag16);
		libc_printf("\npmu_rpc_para->reserved_flag17: 0x%08X", pmu_rpc_para->reserved_flag17);
		libc_printf("\npmu_rpc_para->reserved_flag18: 0x%08X", pmu_rpc_para->reserved_flag18);
		libc_printf("\npmu_rpc_para->reserved_flag19: 0x%08X", pmu_rpc_para->reserved_flag19);
	#endif

		pmu_rpc_para->mcu_rw_offset = WDT_REBOOT_FLAG_SAVE_ADDR;
		hld_pmu_remote_call(0, 0, RPC_MCU_READ32, pmu_rpc_para);

		pmu_rpc_para->mcu_rw_offset = WDT_REBOOT_FLAG_READ_ADDR;
		pmu_rpc_para->mcu_write_data = pmu_rpc_para->mcu_read_data;
		hld_pmu_remote_call(0, 0, RPC_MCU_WRITE32, pmu_rpc_para);

		pmu_rpc_para->mcu_rw_offset = WDT_REBOOT_FLAG_SAVE_ADDR;
		pmu_rpc_para->mcu_write_data = ALI_WDT_REBOOT;
		hld_pmu_remote_call(0, 0, RPC_MCU_WRITE32, pmu_rpc_para);

		hld_pmu_remote_call(0, 0, GET_PMU_MCU_TIME, pmu_rpc_para);
		main_cpu_read_time_init(pmu_rpc_para);
		hld_pmu_remote_call(0, 0, SUSPEND_MCU, pmu_rpc_para);
	}
#endif

#ifdef RTC_TIME_SUPPORT
	if((ALI_C3701 == sys_ic_get_chip_id()) || (ALI_S3503 == sys_ic_get_chip_id()) \
		|| (ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()) \
		|| (ALI_C3702 == sys_ic_get_chip_id()) || (ALI_C3503D == sys_ic_get_chip_id()) \
		|| (ALI_C3711C == sys_ic_get_chip_id()))
	{
		south_rtc_init(0);
	}
#endif

#ifndef PMU_ENABLE
	if((ALI_S3811==sys_ic_get_chip_id()) \
		&& (sys_ic_get_rev_id() >= IC_REV_1))
	{
		rtc_s3811_set_tick_msk();
	}
#endif

	/*Suspend MCU.*/
	if((ALI_C3702 == sys_ic_get_chip_id()) || (ALI_C3922 == sys_ic_get_chip_id()))
	{
		hld_pmu_remote_call(0, 0, SUSPEND_MCU, NULL);
	}

	/*Disable pmu gpio.*/
	pmu_gpio_disable();
}
static void system_advance_init(board_cfg *cfg)
{
	g_nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
	g_nim_dev2 = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);

	if(NULL == cfg)
	{
		libc_printf("%s NULL input!\n",__FUNCTION__);
		ASSERT(0);
	}

	if((NULL == g_nim_dev) && (NULL == g_nim_dev2))
	{
		ASSERT(0);
	}
	else if(NULL == g_nim_dev2)
	{
		cfg->adv_cfg_para.front_end_num = 1;
	}
	else
	{
		cfg->adv_cfg_para.front_end_num = 2;
	}

#ifdef NEW_DEMO_FRAME
#ifdef FSC_SUPPORT
	if(g_nim_dev!=NULL)
	{
		dev_register_device((void *)g_nim_dev, HLD_DEV_TYPE_NIM, 0, FRONTEND_TYPE_C);
	}

	if(g_nim_dev2!=NULL)
	{
		dev_register_device((void *)g_nim_dev2, HLD_DEV_TYPE_NIM, 1, FRONTEND_TYPE_C);
	}

	if(dev_get_by_id(HLD_DEV_TYPE_NIM,2) != NULL)
	{
		dev_register_device((void *)dev_get_by_id(HLD_DEV_TYPE_NIM,2), HLD_DEV_TYPE_NIM, 2, FRONTEND_TYPE_C);
	}

	if(dev_get_by_id(HLD_DEV_TYPE_NIM,3) != NULL)
	{
		dev_register_device((void *)dev_get_by_id(HLD_DEV_TYPE_NIM,3), HLD_DEV_TYPE_NIM, 3, FRONTEND_TYPE_C);
	}
#else
	if(g_nim_dev!=NULL)
	{
		dev_register_device((void *)g_nim_dev, HLD_DEV_TYPE_NIM, 0, board_get_frontend_type(0));
	}
	if(g_nim_dev2!=NULL)
	{
		dev_register_device((void *)g_nim_dev2, HLD_DEV_TYPE_NIM, 1, board_get_frontend_type(1));
	}
#endif
#endif

#ifdef MULTI_CAS
	cfg->adv_cfg_para.class_select = board_class_select;
#endif
#ifdef FPGA_TEST
	cfg->adv_cfg_para.scart_aspect_change = NULL;
#else
	cfg->adv_cfg_para.scart_aspect_change = api_scart_aspect_callback;
#endif

	if(TRUE == is_bootloader_show_logo_keeping_solution())
	{
		cfg->adv_cfg_para.vpo_active = TRUE;
	}
	else
	{
		cfg->adv_cfg_para.vpo_active = FALSE;
	}

	board_advance_init(&(cfg->adv_cfg_para));
}

static void ddr_256m_supprot(void)
{
	*(volatile UINT32 *)(0xb8000220) |= (0x1<<24);
#ifdef _M3505_
	*(volatile UINT32 *)(0xb8070000) |= 0x1;
#else
	*(volatile UINT32 *)(0xb8000110) |= 0x1;/*bit0 set 1, Flash memory for spi flash, default for BootRom.*/
#endif
}

static void ali_dog_enable(void)
{
#ifdef WATCH_DOG_SUPPORT
	*(volatile unsigned char *)(0xb8018504) = 0x67;
	*(volatile unsigned int *)(0xb8018500) = 0xFF000000;
#endif
}

static void ali_dog_disable(void)
{
#ifdef WATCH_DOG_SUPPORT
	*(volatile unsigned char *)(0xb8018504) = 0x0;
	*(volatile unsigned int *)(0xb8018500) = 0x0;
#endif
}

#if (defined(NETWORK_SUPPORT) && defined(GPIO_CONTROL_NET_LED))
static void network_led_gpio_init(void)
{
	int __MAYBE_UNUSED__ gpio_pin = 0, gpio_val = 1;

	/*for M3823 Network LED GPIO15/2 init.*/
	if((SYS_MAIN_BOARD == BOARD_DB_M3823_02V01) \
		|| (SYS_MAIN_BOARD == BOARD_DB_M3823_01V01))
	{
		gpio_pin = 15;
		HAL_GPIO_BIT_SET(gpio_pin,gpio_val);
		gpio_pin = 2;
		HAL_GPIO_BIT_SET(gpio_pin,gpio_val);
	}

	/*for M3257/M3727 Network LED GPIO 0/1 init.*/
	else if((SYS_MAIN_BOARD == BOARD_DB_M3527_01V01) \
		|| (SYS_MAIN_BOARD == BOARD_DB_M3727_01V01))
	{
		gpio_pin = 0;
		HAL_GPIO_BIT_SET(gpio_pin,gpio_val);
		gpio_pin = 1;
		HAL_GPIO_BIT_SET(gpio_pin,gpio_val);
	}
}
#endif

static void system_gpio_check(board_cfg *cfg)/*check all gpio setting in gpio_array.*/
{
	gpio_info_t* gpio_list = NULL;
	UINT16 i = 0, j = 0;

	if((cfg == NULL) || (cfg->all_gpio_array == NULL))
	{
		return;
	}

	gpio_list = cfg->all_gpio_array;
	for(i=0; i<cfg->all_gpio_count; i++)
	{
		for(j=i+1; j<cfg->all_gpio_count; j++)
		{
			if(gpio_list[i].position == gpio_list[j].position)
			{
				libc_printf("Warning!, GPIO %d used more than one times in gpio_array!\n", gpio_list[i].position);
			}
		}
	}
}

extern void dump_heap(char *title);
extern void gos_gpio_set_tuner_reset(void);//new or change add by yuj
extern front_end_cfg* front_end_c_cfg(UINT8 tuner_id,UINT32 type);//new or change add by yuj
void system_hw_init(board_cfg *cfg)
{
	UINT32 boot_total_area_len = 0;

	set_board_cfg(cfg);

	/*init chip.*/
	global_chip_set(chip_create());
	chip_init(&(cfg->chip_param));

	/*config gpio.*/
	chip_gpio_pin_init_ext(cfg->all_gpio_array, cfg->all_gpio_count);

	/*for 256M DDR.*/
#ifdef _SUPPORT_256M_MEM
	ddr_256m_supprot();
#endif

	if(((NULL != cfg->front_end_cfg[0]) && (FRONTEND_TYPE_S == cfg->front_end_cfg[0]->front_end_type))
		|| ((NULL != cfg->front_end_cfg[1] )&& (FRONTEND_TYPE_S == cfg->front_end_cfg[1]->front_end_type)))
	{
		/*power off 1000ms.*/
		chip_gpio_set(cfg->sys_power, 0);
		osal_task_sleep(1000);
	}

	/*config pinmux.*/
	chip_pin_mux_set_ext(cfg->pin_mux_array, cfg->pin_mux_count);

#if (defined(NETWORK_SUPPORT) && defined(GPIO_CONTROL_NET_LED))
	/*init the network LED light gpio value.*/
	network_led_gpio_init();
#endif

	/*power on.*/
	board_power(TRUE);

#ifdef DUAL_ENABLE
	ali_dog_enable();
	hld_dev_see_init((void *)(__MM_SEE_DBG_MEM_ADDR));
	ali_dog_disable();
#endif

	/*uart init.*/
	system_uart_port_init();

#ifndef _BUILD_OTA_E_
	// warning, please don't remove version information
	// it's very helpfull for us to debug/trace issue.
	#ifdef SFU_TEST_SUPPORT
	//SciEcho("MC: APP init ok\r\n");
	//SciEcho(get_sdk_ver());
	//SciEcho("\r\n");
	//SciEcho(get_core_ver());
	//SciEcho("\r\n");
	//SciEcho(get_app_ver());
	//SciEcho("\r\n");
	#else
	FIXED_PRINTF("MC: APP init ok\r\n");
	FIXED_PRINTF("%s\r\n", get_sdk_ver());
	FIXED_PRINTF("%s\r\n", get_core_ver());
	FIXED_PRINTF("%s\r\n", get_app_ver());
	#ifdef SUPPORT_CAS_A
	FIXED_PRINTF("%s\r\n", get_c1700a_ver());
	#endif

	#ifdef CAS9_V6
	FIXED_PRINTF("%s\r\n", get_plus_ver());
	FIXED_PRINTF("%s\r\n", get_ca_lib_ver());
	#endif
	#endif
#endif

	/*PMU init.*/
	system_pmu_init();

	/*i2c.*/
	board_i2c_init();
#if 1//new or change add by yuj
	
	INT32 result836 = ERR_FAILUE;
//	INT32 result820 = ERR_FAILUE;
//	INT32 result850 = ERR_FAILUE;
	INT32 result2040 = ERR_FAILUE;
	INT32 result253 = ERR_FAILUE;
//	INT32 result5880 = ERR_FAILUE;
//	INT32 result608 = ERR_FAILUE;
	UINT8 data = 0;

	gos_gpio_set_tuner_reset();

	result836 	= i2c_scb_read(I2C_TYPE_SCB1, 0x74, &data, 1);
//	result820 	= i2c_scb_read(I2C_TYPE_SCB1, 0x34, &data, 1);
//	result850 	= i2c_scb_read(I2C_TYPE_SCB1, 0xf8, &data, 1);
	result253 	= i2c_scb_read(I2C_TYPE_SCB1, 0xF8, &data, 1);
	result2040 	= i2c_scb_read(I2C_TYPE_SCB1, 0xC0, &data, 1);
//	result5880 	= i2c_scb_read(I2C_TYPE_SCB1, 0xC2, &data, 1);
//	result608 	= i2c_scb_read(I2C_TYPE_SCB1, 0xC6, &data, 1); 

	libc_printf("App Release Time: %s\n",GOS_RELEASE_TIME);
	libc_printf("--------i2c_read-------result836=%d\n",result836);
//	libc_printf("--------i2c_read-------result820=%d\n",result820);
//	libc_printf("--------i2c_read-------result850=%d\n",result850);
	libc_printf("--------i2c_read-------result253=%d\n",result253);
	libc_printf("--------i2c_read-------result2040=%d\n",result2040);
//	libc_printf("--------i2c_read-------result5880=%d\n",result5880);
//	libc_printf("--------i2c_read-------result608=%d\n",result608);


	if( result836 == SUCCESS
	|| result2040 == SUCCESS
	|| result253 == SUCCESS)
	{
		if(result2040 == SUCCESS) 
		{
			cfg->front_end_cfg[0] = front_end_c_cfg(0,ATBM2040);	
			libc_printf("--------front_end_c_cfg-------ATBM2040\n");
		}
		else if (result253 == SUCCESS)
		{
			cfg->front_end_cfg[0] = front_end_c_cfg(0,ATBM253);
			libc_printf("--------front_end_c_cfg-------ATBM253\n");
		}
		else if (result836 == SUCCESS)
		{
			cfg->front_end_cfg[0] = front_end_c_cfg(0,RT836);
			libc_printf("--------front_end_c_cfg-------RT836\n");
		}		
	}
    	set_board_cfg(cfg);
#endif

	/*scart.*/
	if((NULL != cfg->scart_attach) && (NULL != cfg->scart_param))
	{
		cfg->scart_attach(cfg->scart_param);
		g_scart_dev = (struct scart_device *)dev_get_by_id(HLD_DEV_TYPE_SCART, 0);
	}

	/*front panel.*/
	if((NULL != cfg->pan_attach) && (NULL != cfg->pan_config))
	{
		cfg->pan_attach(cfg->pan_config);
		g_pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
		if(NULL != cfg->pan_i2c_id_set)
		{
			cfg->pan_i2c_id_set(g_pan_dev, cfg->pan_i2c_id);
		}

		if(pan_open(g_pan_dev) != SUCCESS)
		{
			PRINTF("%s(): failed!\n", __FUNCTION__);
		}
		pan_display(g_pan_dev, " on ", 4);
	}

	system_gpio_check(cfg);
	if(ALI_C3505 == sys_ic_get_chip_id())
	{
		*(volatile unsigned long *)(0xb8070000) &= ~(1<<25);
	}

	/*flash.*/
	if(1 == cfg->flash_type)
	{
		sto_local_flash_attach(NULL);
	}
	else
	{
		sto_local_sflash_attach(NULL);
	}

	g_sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	//sto_open(g_sto_dev);
	if(g_sto_dev)
	{
		if(sto_open(g_sto_dev) != SUCCESS)
		{
			PRINTF("sto_open() failed!\n");
		}
	}
	else
	{
		PRINTF("Flash attach failed\n");
	}

#ifdef FLASH_SOFTWARE_PROTECT
	ap_set_flash_lock_len(DEFAULT_PROTECT_ADDR);
#endif
	get_boot_total_area_len(&boot_total_area_len);
	if((INT32)boot_total_area_len > g_sto_dev->totol_size)
	{
		PRINTF("%s() %d err:bootloader size is larger than the flash size!!!\n",\
			__FUNCTION__,__LINE__);
	}

	if(g_sto_dev)
	{
		sto_chunk_init(boot_total_area_len, g_sto_dev->totol_size - boot_total_area_len);
	}

#ifdef _NV_PROJECT_SUPPORT_
	if(g_sto_dev)
	{
		sto_block_init(0, g_sto_dev->totol_size);
	}
#endif

	/*rfm.*/
	if(NULL != cfg->rfm_attach)
	{
		cfg->rfm_attach();
		g_rfm_dev = (struct rfm_device*)dev_get_by_id(HLD_DEV_TYPE_RFM, 0);
		if(NULL != g_rfm_dev)
		{
			g_rfm_dev->i2c_type_id_cfg = 1;
			g_rfm_dev->i2c_type_id = cfg->rfm_i2c_id;
		}
	}

	/*frontend.*/
	board_frontend_init();

	/*advance init.*/
	system_advance_init(cfg);

	/*GE.*/
	g_ge_dev = (struct ge_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_GE);

#ifdef USB_MP_SUPPORT
	#ifdef ALI_SDK_API_ENABLE
	//deca_ext_dec_enable((struct deca_device*)dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
	#endif

#ifdef PVR_FS_ZH
	init_file_system();
#endif
#endif

#if ((defined(NETWORK_SUPPORT)) && (!defined(MAC_TEST)))
	network_attach();
#endif
}

