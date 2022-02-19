/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: chip.h
*
*    Description: This file is provide api for chip bonding .
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _ASM_CHIP_H_
#define _ASM_CHIP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>


UINT32 sys_ic_split_enabled();
UINT32 sys_ic_aac_is_enabled_ex();
UINT32 sys_ic_ext_demod_is_enabled();
UINT32 sys_ic_ext_demod_is_enabled();
UINT32 sys_ic_get_hd_enabled_ex();
UINT32 sys_ic_rmvb_is_enabled_ex();
UINT32 sys_ic_mp4_is_enabled_ex();
UINT32 sys_ic_h264_is_enabled_ex();
UINT32 sys_ic_xdplus_is_enabled_ex();
UINT32 sys_ic_xd_is_enabled_ex();
UINT32 sys_ic_mv_is_enabled_ex();
UINT32 sys_ic_ms11_is_enabled_ex();
UINT32 sys_ic_ddplus_is_enabled_ex();
UINT32 sys_ic_ac3_is_enabled_ex();
UINT32 sys_ic_get_product_id_ex();
UINT32 sys_ic_bootrom_is_enabled();
UINT32 sys_ic_dram_scramble_enabled();
UINT32 sys_ic_get_boot_type(void);
UINT32 get_boot_cfg_addr(void);

UINT32 sys_ic_get_cpu_clock(void);
UINT32 sys_ic_get_dram_clock(void);
BOOL sys_ic_get_dram_info(UINT32 *type, UINT32 *size, UINT32 *rate);
BOOL sys_ic_L2_cache_enabled(void);


UINT32 sys_ic_get_chip_id(void);
UINT32 sys_ic_get_rev_id(void);

UINT8 sys_ic_is_m3101(void);
UINT32 sys_ic_m3101_rev_id(void);

UINT32 sys_ic_is_m3202();
INT32 sys_reboot_get_timer(UINT32 *expect, UINT32 *current);
UINT32 sys_ic_is_csa3_enabled(void);
BOOL sys_ic_get_as_status(void);
BOOL sys_ic_adv_hkey_is_enabled(UINT8 *hkey, UINT8 key_len);
UINT32 is_bl_encrypted_enable(void);

/*hardware reboot by watchdog*/
UINT32 hw_watchdog_reboot(void);
UINT32 hw_watchdog_stop(void);

/*ASSERT reboot from sys_ic_reboot*/
UINT32 sys_watchdog_reboot(void);
/*software reboot by enter 0xafc00000*/
UINT32 sys_ic_reboot(void);

UINT32 sys_get_tick(void);
UINT32 sys_get_otp(UINT32 offset);
UINT32 sys_get_otp_from_shadow_asm(UINT32 offset);

UINT32 read_CP0_config2(void);


/*Copy code from load address into running address and then jump into entry*/
UINT32 self_boot(UINT32 run_address, UINT32 load_address, UINT32 length, UINT32 entry);

/*jump to entry and exec code from entry*/
void exec(UINT32 entry);


void pflash_write_soft_protect(UINT32 flash_addr,char src);
void powerdown_test(void);

void sys_ic_enter_standby(UINT32 expect_time, UINT32 current_time);
void sys_enter_cpu_standby(UINT32 expect_time, UINT32 current_time);
UINT32 sys_ic_dsc_access_ce_disable(void);
UINT32 sys_ic_dsc_access_sram_disable(void);

void sflash_write_soft_protect(UINT32, unsigned char *, UINT32, char);
void pflash_soft_protect_init(void);
void powerdown_test_init(void);
void sflash_soft_protect_init(void);
/*get cpu ticks*/
UINT32 read_tsc(void);
void _start(void);

//chip_feature
#if(SYS_CHIP_MODULE == ALI_S3602)
UINT32 sys_ic_get_product_id(void);
UINT32 sys_ic_get_c3603_product(void);
UINT32 sys_otp_get_version(void);
UINT32 sys_otp_get_package(void);
UINT32 sys_otp_get_certification(void);
UINT32 sys_otp_get_security(void);
UINT32 sys_ic_ac3_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_ddplus_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_ms10_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_ms11_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_mg_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_xd_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_xdplus_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_h264_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_mp4_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_rmvb_is_enabled(void);// return  0:  disable, 1:  enable
BOOL sys_ic_vc1_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_avs_is_enabled(void);// return  0:  disable, 1:  enable
BOOL sys_ic_vp8_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_get_hd_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_flv_is_enabled(void);// return  0:  disable, 1:  enable
UINT32 sys_ic_aac_is_enabled(void);
int sys_ic_get_usb_num(void);
BOOL sys_ic_usb_port_enabled(UINT32 port);
BOOL sys_ic_nim_m3501_support(void);
BOOL sys_ic_nim_support(void);
int sys_ic_get_ci_num(void);
int sys_ic_get_mac_num(void);
BOOL sys_ic_hd_is_enabled(void);    // 0: enable SD only, 1: enable SD/HD
BOOL sys_ic_hd_output_is_enabled();
void sys_ic_set_hd_decoder(UINT32 flag);
int sys_ic_sata_enable(void);
int sys_ic_get_tuner_num(void);
BOOL sys_ic_3g_is_enabled(void);
BOOL sys_ic_wifi_is_enabled(void);
BOOL sys_ic_isdb_is_enabled(void);   //call

BOOL sys_ic_cas_is_enabled(void);   //call
BOOL sys_ic_product_is_m3612(void);
INT32 sys_ic_get_kl_key_mode(UINT32 root_index);
INT32 sys_ic_change_boot_type(UINT32 type);
INT32 sys_ic_revert_boot_type(void);
RET_CODE get_boot_total_area_len(UINT32 *len);
INT32 sys_ic_get_secure_flash_map(UINT32 *reserved);
UINT32 sys_ic_vfb_is_enabled(void);
#else
int sys_ic_get_tuner_num(void);
BOOL sys_ic_wifi_is_enabled(void);
BOOL sys_ic_product_is_m3612(void);
#endif

void fw_boot_debug(UINT32 print_char);
UINT32 sys_ic_get_see_clock(void);
UINT32 get_main_see_clk(UINT8 cpu_type);
void copy_performance_optimization(void);
UINT32 sys_ic_seerom_is_enabled(void);
void irq_disable_interrupt(void);
void irq_enable_interrupt(void);

#ifdef __cplusplus
}
#endif


#endif  //_ASM_CHIP_H_

