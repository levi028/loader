/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: power.c
*
*    Description: Routine for power off.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <asm/chip.h>
#include <hld/pan/pan_key.h>
#include <hld/pan/pan_dev.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <hld/dis/vpo.h>
#include <hld/deca/deca_dev.h>
#include <hld/pan/pan.h>
#include <api/libdiseqc/lib_diseqc.h>
#include <hld/sto/sto_dev.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#endif
#include <api/libmp/media_player_api.h>

#ifdef USB_SUPPORT_HUB
#include <bus/usb2/usb.h>
#endif
#include <bus/dog/dog.h>
#ifdef HDTV_SUPPORT
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#endif
#ifdef SUPPORT_FRANCE_HD
#include <api/libsi/si_nit.h>
#endif
#ifdef MULTI_CAS
#include <hld/smc/smc.h>
#endif
#include <bus/rtc/rtc.h>
#include <hld/pmu/pmu.h>
#include <api/libtsi/sie.h>
#include <api/libsi/si_tdt.h>

#include "win_com.h"
#include "key.h"
#include "control.h"
#include "vkey.h"
#ifdef PMU_ENABLE
#ifdef PMU_RETENTION_SUPPORT
#include "ali_pmu_bin_retention.h"
#else
#include "ali_pmu_bin.h"
#endif
#endif
//#include "gpio_cfg.h"
#include "win_com_popup.h"
#ifdef SUPPORT_FRANCE_HD
#include "nit_info.h"
#endif
#ifdef SUPPORT_BC
#include "bc_ap/bc_osm.h"
#endif
#ifdef SUPPORT_BC_STD
#include "bc_ap_std/bc_osm.h"
#endif
#include "platform/board.h"
#include "power.h"
#include "disk_manager.h"
#ifdef FSC_SUPPORT
#include "fsc_ap/fsc_control.h"
#endif

#define POW_PRINTF(...)  do{}while(0)//libc_printf

#define USB_CTL_POWER_DOWN  0
#define USB_CTL_SLEEP       1
#define USB_CTL_WAKEUP      2
extern void get_stc_offset_stream(INT32 *hour_off,INT32* min_off);

#ifndef _BUILD_OTA_E_
static void usb_power_control(int mode)
{
#ifdef USB_SUPPORT_HUB
#if ((!(defined _S3281_||defined _M3503_))||(!defined _BUILD_OTA_E_))
    int i = 0;

    for (i = 0; i < MAX_HANDLE_NUM; i++)
    {
        if ( USB_CTL_POWER_DOWN == mode)
        {
            #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
            usbd_power_down((USBD_NODE_ID)i);
            #endif
        }
    }
#endif
#endif
}
#endif

#ifndef _BUILD_OTA_E_
static void storage_power_down(void)
{
#ifndef DISABLE_USB  //3281 32m maybe don't have the usb hub.
    usb_power_control(USB_CTL_POWER_DOWN);
#endif
}
#endif

static void scart_dev_close(void)
{
    // SE: Mute before scart power off to avoid noise
    api_audio_set_mute(TRUE);
#ifdef _M3503_
    api_scart_tvsat_switch(1);
#else
    api_scart_tvsat_switch(0);
#endif
    api_scart_rgb_on_off(0); //CVBS mode
#ifdef _S3281_
    api_scart_power_on_off(0);
#else
    api_scart_power_on_off(1);
#endif

    return;
}
static void av_dev_close(void)
{
    struct ycb_cr_color bgcolor;

    MEMSET(&bgcolor,0,sizeof(struct ycb_cr_color));
    vpo_win_onoff(g_vpo_dev, FALSE);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    if(NULL != g_sd_vpo_dev)
    {
        vpo_win_onoff(g_sd_vpo_dev,FALSE);
    }
#endif
    bgcolor.u_y = 16;
    bgcolor.u_cb = 128;
    bgcolor.u_cr = 128;
    vpo_ioctl(g_vpo_dev,VPO_IO_SET_BG_COLOR,(UINT32)&bgcolor);
    deca_stop(g_deca_dev,0,ADEC_STOP_IMM);
    //deca_close(g_deca_dev);
    vdec_close(g_decv_dev);
    vpo_close(g_vpo_dev);
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    if(RET_SUCCESS != vpo_close((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1)))
    {
        //ASSERT(0);
    }
#endif
    snd_stop(g_snd_dev);

    return;
}

#if( !defined(_BUILD_OTA_E_) && defined(AUTO_OTA))
static BOOL auto_ota_power_off_process(BOOL ota_scan)
{
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT) ||defined(DVBC_SUPPORT))
    date_time et;
    BOOL flag_et = FALSE;
    BOOL bootup = FALSE;

    if(ota_scan)
    {
        key_pan_display("srch", 4);
        flag_et = api_ota_get_recently_timer(&et);
#ifdef NEW_MANUAL_OTA
        ota_set_menu = FALSE;
#endif
        bootup = ota_stby_search(&et);
        if(bootup)
        {
            get_local_time(&et);
            flag_et = TRUE;

            return FALSE;
        }
    }
#endif

   return TRUE;
}
#endif
static void lnb_ant_power_off(void)
{
    //api_Scart_OutPut_Switch(0);
    set_lnbshort_detect(0);
    set12vshort_detect(0);
    board_lnb_power(0, 0);
    board_lnb_power(1, 0);
    board_antenna_power(0, 0);
    board_antenna_power(1, 0);
    //api_LNB_power(0);
#if(SYS_12V_SWITCH == SYS_FUNC_ON)
     api_diseqc_set_12v(g_nim_dev, 0);
#endif

   return ;
}

#ifdef MULTI_CAS
static void smc_close_for_power_off(void)
{
    struct smc_device *smc_dev = NULL;
    UINT8 i = 0;

    for(i=0;i<2;i++)
    {
        smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, i);
        if(smc_dev!=NULL)
        {
            smc_close(smc_dev);
        }
    }

   return ;
}
#endif

static void close_eth_mac(void)
{
    //close ethnet
#ifdef NETWORK_SUPPORT
#if (SYS_NETWORK_MODULE == NET_ALIETHMAC)	    
    struct net_device *net_dev=NULL;    
           
    net_dev = dev_get_by_type(NULL, HLD_DEV_TYPE_NET);
    if(NULL != net_dev)
    {
        eth_mac_close(net_dev);
        eth_mac_detach(net_dev);
    }
#endif        
#endif    
}

BOOL power_off_process(BOOL ota_scan __MAYBE_UNUSED__)
{
    UINT8 back_saved = 0;
    BYTE *buffer = NULL;


#ifdef HDTV_SUPPORT
    struct hdmi_device *hdmi_dev = NULL;
#endif
    sys_state_t __MAYBE_UNUSED__ system_state = SYS_STATE_INITIALIZING;
    UINT8 need_free = 0;
    win_popup_choice_t ret = WIN_POP_CHOICE_NO;
    

	if(0 == ret)
	{
		;
	}
    system_state = api_get_system_state();

    close_eth_mac();
      
    struct nim_device* nim_dev=NULL;
    //add by bill 2.12.02.21
    nim_dev = g_nim_dev;
    if((NULL != nim_dev )&& (NULL != nim_dev->do_ioctl))
    {
        nim_dev->do_ioctl(nim_dev, NIM_TURNER_SET_STANDBY, TRUE);
    }
    nim_dev = g_nim_dev2;
    if((NULL != nim_dev )&& (NULL != nim_dev->do_ioctl))
    {
        nim_dev->do_ioctl(nim_dev, NIM_TURNER_SET_STANDBY, TRUE);
    }

#ifdef CI_SUPPORT
    api_ci_unregister(NULL);
#endif

#ifdef _MHEG5_SUPPORT_
    mheg_running_app_kill(0);
#endif

#ifdef DVR_PVR_SUPPORT
    api_pvr_clear_up_all();
#endif

#ifdef USB_MP_SUPPORT
    if(SYS_STATE_USB_MP == system_state)
    {
        ap_udisk_close();
    }
#endif
    if(NULL == osd_get_task_vscr_buffer(osal_task_get_current_id()))
    {
        buffer = (BYTE *)MALLOC(600*1024);
        if(NULL == buffer)
        {
            ali_trace(&buffer);
        }
        osd_task_buffer_init(osal_task_get_current_id(),buffer);
        need_free = 1;
    }
#ifdef _INVW_JUICE
    inview_pause(); /* I think we can live without the "Please wait.." popup */
#else
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg_ext("Please wait...", NULL, 0);
    ret = win_compopup_open_ext(&back_saved);
#endif
#ifndef _BUILD_OTA_E_
    storage_power_down(); // USB power down process
    #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
    usbd_detach_ex();
    #endif
#endif
#ifdef _INVW_JUICE
#else
    win_compopup_smsg_restoreback();
    if (1 == need_free)
    {
        osd_task_buffer_free(osal_task_get_current_id(), buffer);
        free(buffer);
        need_free = 0;
    }
#endif
   scart_dev_close();

#ifdef VDAC_USE_SVIDEO_TYPE
    api_svideo_on_off(0);
#endif

#ifndef NEW_DEMO_FRAME
    if(VIEW_MODE_MULTI != hde_get_mode())
    {
        uich_chg_stop_prog(TRUE);
    }
#endif

#if (TTX_ON == 1)
    ttx_enable(FALSE);
#endif
#if (SUBTITLE_ON == 1)
    subt_enable(FALSE);
#endif
#if (ISDBT_CC==1)
    isdbtcc_enable(FALSE);
#endif

   osd_show_on_off(OSDDRV_OFF);

   av_dev_close();
//**********************************//add for Auto OTA
#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
   if(FALSE == auto_ota_power_off_process(ota_scan))
   {
       return FALSE;
   }
#endif
//***********************************//add end
   lnb_ant_power_off();
   sie_close();

#ifdef NEW_DEMO_FRAME
    sim_close_monitor(0);
#else
    si_monitor_off(0xFFFFFFFF);
#endif
    stop_tdt();
//reduce the space for ota
#ifndef _BUILD_OTA_E_
    epg_off();
#endif

    dm_set_onoff(0);

    //close drivers
    dmx_stop( g_dmx_dev);
    dmx_close( g_dmx_dev);
    if (g_dmx_dev2)
    {
        dmx_stop(g_dmx_dev2);
        dmx_close(g_dmx_dev2);
    }

    snd_close(g_snd_dev);

    api_scart_out_put_switch(0); // put it after snd_close() to avoid noise

// Power HDMI phy at standby mode.
#ifdef HDTV_SUPPORT
    hdmi_dev = (struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);
    if (NULL != hdmi_dev )
    {
        if (SUCCESS != hdmi_dev->close(hdmi_dev))
        {
            PRINTF("hdmi_close failed!!\n");
            ASSERT(0);
        }
    }
#endif

    nim_close(g_nim_dev);
    if(g_nim_dev2)
    {
        nim_close(g_nim_dev2);
    }
    if(g_rfm_dev)
    {
        rfm_close(g_rfm_dev);
    }
    led_display_flag = 0;

#ifdef MULTI_CAS
    smc_close_for_power_off();
#endif
    return TRUE;
}

void power_off_process2(void)
{
    board_cfg *cfg = get_board_cfg();

    if(sys_ic_get_chip_id() != ALI_S3281)
    {
        chip_gpio_set(cfg->sys_power, 0);  // power off
    }

    // SE: Cancel Mute before entering standby to avoid power leak
    api_audio_set_mute(FALSE);

    chip_ci_power(FALSE);
}

void power_on_process(void)
{
    SYSTEM_DATA *sys_data = NULL;
    UINT32 tmp = 0;

	if(0 == tmp)
	{
		;
	}
	sys_data = sys_data_get();

    key_pan_display(" on ", 4);

    pan_close(g_pan_dev);

	if(sys_data->local_time.buse_gmt)
	{
		get_stc_time(&sys_data->sys_dt);
	}
    else
    {
        get_local_time(&sys_data->sys_dt);  //ctchen
    }

    sys_data->bstandmode = 0;
    sys_data_save(1);

    dog_m3327e_attach(1);

   /**
    *  before enter watchdog_reboot,you MUST NOT read/write flash
    *  as soon as possible.make sure the bus is clear.
    */
    osal_interrupt_disable();
    *((volatile UINT32 *)0xb802e098)=0xc2000d03;
    tmp = *((volatile UINT32 *)0xb802e098);
    dog_mode_set(0, DOG_MODE_WATCHDOG, 10, NULL);
    while(1)
    {
         // for cpu loop
        ;
    }
}

void api_standby_led_onoff(BOOL onoff)
{
    board_standby_led_onoff(onoff);
}

#ifdef SUPPORT_FRANCE_HD
static void save_nit_monitor_date(void)
{
    UINT8 i = 0;
    struct nit_section_info *nit_info = NULL;
    struct NIT_VER_INFO *nit_ver_info = NULL;

    nit_info = MALLOC(sizeof(struct nit_section_info));
    if(NULL == nit_info)
    {
        ali_trace(&nit_info);
        return;
    }
    load_nit_monitor_info(nit_info);
    nit_ver_info = MALLOC(sizeof(struct NIT_VER_INFO)+sizeof(struct NIT_SER_INFO)*(nit_info->s_nr-1));
    if(NULL == nit_ver_info)
    {
        FREE(nit_info);
        ali_trace(&nit_info);
        return;
    }
    MEMSET(nit_ver_info, 0, sizeof(struct NIT_VER_INFO)+sizeof(struct NIT_SER_INFO)*(nit_info->s_nr-1));
    nit_ver_info->num = nit_info->s_nr;
    nit_ver_info->verison_number = nit_info->version_num;
    for(i=0; i<nit_info->s_nr; i++)
    {
        nit_ver_info->nit_service[i].sid = nit_info->s_info[i].sid;
        nit_ver_info->nit_service[i].tsid = nit_info->s_info[i].tsid;
        nit_ver_info->nit_service[i].onid = nit_info->s_info[i].onid;
    }
    save_nit_data(nit_ver_info, sizeof(struct NIT_VER_INFO)+sizeof(struct NIT_SER_INFO)*(nit_ver_info->num-1));
    FREE(nit_ver_info);
    FREE(nit_info);

}
#endif

#ifdef PMU_ENABLE
#define AP_PMU_WRITE8(i, d)                        (*(volatile UINT8 *)(i)) = (d)

void PMU_Set_Panel_Key(void)
{
#if ((defined _M3821_) || (defined _M3503_) || (defined _M3505_) || (defined _M3503D_) || (defined _M3711C_))
	AP_PMU_WRITE8(STANDBY_PAN_KEY_ADDR, STANDBY_PAN_KEY);
#elif (defined _M3702_)
	pmu_rpc_para->panel_power_key = STANDBY_PAN_KEY;
	hld_pmu_remote_call((UINT32)(g_ali_pmu_bin), sizeof(g_ali_pmu_bin)/sizeof(UINT8),
		SET_PANEL_POWER_KEY, pmu_rpc_para);
#else
    AP_PMU_WRITE8(STANDBY_PAN_KEY_ADDR, STANDBY_PAN_KEY);
#endif
}

static void pmu_standby_set_timer(UINT16 local_time_year, date_time *p_time)
{
	INT32 o_h = 0;
	INT32 o_m = 0;
	INT32 o_s = 0;
	PMU_BIN pmu_bin;
	SYSTEM_DATA *system_data_year = NULL;
	struct rtc_time base_time = {0, 0, 0, 0, 0, 0, 0};
	struct pmu_device *pmu_dev =(struct pmu_device *)dev_get_by_id(HLD_DEV_TYPE_PMU, 0);
	INT32 __MAYBE_UNUSED__ ret = 0;

	MEMSET(&pmu_bin, 0, sizeof(PMU_BIN));
	if(((ALI_S3811 == sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1)) \
		|| ((ALI_C3701 == sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1)) \
		|| (ALI_S3503 == sys_ic_get_chip_id()) \
		|| (ALI_S3821 == sys_ic_get_chip_id()) \
		|| (ALI_C3505 == sys_ic_get_chip_id()) \
		|| (ALI_C3503D == sys_ic_get_chip_id()) \
		|| (ALI_C3711C == sys_ic_get_chip_id()) \
		|| (ALI_C3702 == sys_ic_get_chip_id()))
	{
		get_stc_offset(&o_h,&o_m,&o_s);
		if((ALI_S3503 == sys_ic_get_chip_id()) \
			|| (ALI_C3701 == sys_ic_get_chip_id()) \
			|| (ALI_S3821 == sys_ic_get_chip_id()) \
			|| (ALI_C3505 == sys_ic_get_chip_id()) \
            || (ALI_C3503D == sys_ic_get_chip_id()) \
			|| (ALI_C3711C == sys_ic_get_chip_id()) \
			|| (ALI_C3702 == sys_ic_get_chip_id()))
		{
			base_time.year = p_time->year;
		}
		else
		{
			base_time.year = p_time->year%4;
		}
		base_time.month = p_time->month;
		base_time.date =p_time->day;
		base_time.hour = p_time->hour;
		base_time.min = p_time->min;
		base_time.sec =p_time->sec ;

		if(ALI_S3811 == sys_ic_get_chip_id())
		{
			ret = rtc_s3811_set_value(&base_time);/*set current time.*/
			/*add below code to fix daily record timer issue,
			**because 3811's pmu register has not enough bit to save year.
			*/
			system_data_year = sys_data_get();
			system_data_year->sys_dt.year = local_time_year;
			sys_data_save(1);
		}

		if((ALI_C3701 == sys_ic_get_chip_id()) || (ALI_S3503 == sys_ic_get_chip_id()) \
			|| (ALI_S3821 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()) \
			|| (ALI_C3503D == sys_ic_get_chip_id()) || (ALI_C3711C == sys_ic_get_chip_id()))
		{
			ret = pmu_mcu_enter_stby_timer_set_value(&base_time);/*set current time.*/
		}
		else if(ALI_C3702 == sys_ic_get_chip_id())
		{
			/*Send current time to MCU.*/
			pmu_rpc_para->smc_current_year_h = (base_time.year)/100;
			pmu_rpc_para->smc_current_year_l = (base_time.year)%100;
			pmu_rpc_para->smc_current_month = base_time.month;
			pmu_rpc_para->smc_current_day = base_time.date;
			pmu_rpc_para->smc_current_hour = base_time.hour;
			pmu_rpc_para->smc_current_min = base_time.min;
			pmu_rpc_para->smc_current_sec = base_time.sec;

			libc_printf("\nDump APP current time setting:");
			libc_printf("\nbase_time.year: %d", base_time.year);
			libc_printf("\nbase_time.month: %d", base_time.month);
			libc_printf("\nbase_time.date: %d", base_time.date);
			libc_printf("\nbase_time.hour: %d", base_time.hour);
			libc_printf("\nbase_time.min: %d", base_time.min);
			libc_printf("\nbase_time.sec: %d\n", base_time.sec);

			hld_pmu_remote_call(0, 0, SET_CURRENT_TIME, pmu_rpc_para);
			/*Set PMU standby wakeup time.*/
			hld_pmu_remote_call(0, 0, SET_WAKEUP_TIME, pmu_rpc_para);
		#ifdef PMU_STANDBY_DEBUG_EN
			libc_printf("\nFunction:%s, Line:%d", __FUNCTION__, __LINE__);
			/*Dump all PMU RPC params.*/
			libc_printf("\npmu_rpc_para->smc_current_year_h: 0x%02X", pmu_rpc_para->smc_current_year_h);
			libc_printf("\npmu_rpc_para->smc_current_year_l: 0x%02X", pmu_rpc_para->smc_current_year_l);
			libc_printf("\npmu_rpc_para->smc_current_month: 0x%02X", pmu_rpc_para->smc_current_month);
			libc_printf("\npmu_rpc_para->smc_current_day: 0x%02X", pmu_rpc_para->smc_current_day);
			libc_printf("\npmu_rpc_para->smc_current_hour: 0x%02X", pmu_rpc_para->smc_current_hour);
			libc_printf("\npmu_rpc_para->smc_current_min: 0x%02X", pmu_rpc_para->smc_current_min);
			libc_printf("\npmu_rpc_para->smc_current_sec: 0x%02X", pmu_rpc_para->smc_current_sec);

			libc_printf("\npmu_rpc_para->smc_wakeup_month: 0x%02X", pmu_rpc_para->smc_wakeup_month);
			libc_printf("\npmu_rpc_para->smc_wakeup_day: 0x%02X", pmu_rpc_para->smc_wakeup_day);
			libc_printf("\npmu_rpc_para->smc_wakeup_hour: 0x%02X", pmu_rpc_para->smc_wakeup_hour);
			libc_printf("\npmu_rpc_para->smc_wakeup_min: 0x%02X", pmu_rpc_para->smc_wakeup_min);
			libc_printf("\npmu_rpc_para->smc_wakeup_sec: 0x%02X", pmu_rpc_para->smc_wakeup_sec);

			libc_printf("\npmu_rpc_para->pmu_powerup_type: 0x%02X", pmu_rpc_para->pmu_powerup_type);
			libc_printf("\npmu_rpc_para->panel_power_key: 0x%02X", pmu_rpc_para->panel_power_key);
			libc_printf("\npmu_rpc_para->show_panel_type: 0x%02X", pmu_rpc_para->show_panel_type);
			libc_printf("\npmu_rpc_para->reserved_flag1: 0x%02X", pmu_rpc_para->reserved_flag1);

			libc_printf("\npmu_rpc_para->reserved_flag2: 0x%02X", pmu_rpc_para->reserved_flag2);
			libc_printf("\npmu_rpc_para->reserved_flag3: 0x%02X", pmu_rpc_para->reserved_flag3);
			libc_printf("\npmu_rpc_para->reserved_flag4: 0x%02X", pmu_rpc_para->reserved_flag4);
			libc_printf("\npmu_rpc_para->reserved_flag5: 0x%02X", pmu_rpc_para->reserved_flag5);

			libc_printf("\npmu_rpc_para->reserved_flag6: 0x%02X", pmu_rpc_para->reserved_flag6);
			libc_printf("\npmu_rpc_para->reserved_flag7: 0x%02X", pmu_rpc_para->reserved_flag7);
			libc_printf("\npmu_rpc_para->reserved_flag8: 0x%02X", pmu_rpc_para->reserved_flag8);
			libc_printf("\npmu_rpc_para->reserved_flag9: 0x%02X", pmu_rpc_para->reserved_flag9);

			libc_printf("\npmu_rpc_para->ir_power_key1: 0x%08X", pmu_rpc_para->ir_power_key1);
			libc_printf("\npmu_rpc_para->text_seg_start_addr: 0x%08X", pmu_rpc_para->text_seg_start_addr);
			libc_printf("\npmu_rpc_para->text_seg_end_addr: 0x%08X", pmu_rpc_para->text_seg_end_addr);
			libc_printf("\npmu_rpc_para->mcu_code_start_phys: 0x%08X", pmu_rpc_para->mcu_code_start_phys);

			libc_printf("\npmu_rpc_para->mcu_code_len: 0x%08X", pmu_rpc_para->mcu_code_len);
			libc_printf("\npmu_rpc_para->ir_power_key2: 0x%08X", pmu_rpc_para->ir_power_key2);
			libc_printf("\npmu_rpc_para->wdt_reboot_flag: 0x%08X", pmu_rpc_para->wdt_reboot_flag);
			libc_printf("\npmu_rpc_para->kernel_str_params_addr: 0x%08X", pmu_rpc_para->kernel_str_params_addr);

			libc_printf("\npmu_rpc_para->kernel_str_params_len: 0x%08X", pmu_rpc_para->kernel_str_params_len);
			libc_printf("\npmu_rpc_para->mcu_uart_support: 0x%08X", pmu_rpc_para->mcu_uart_support);
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

			libc_printf("\npmu_rpc_para->mcu_write_data: 0x%08X", pmu_rpc_para->mcu_write_data);
			libc_printf("\npmu_rpc_para->mcu_read_data: 0x%08X", pmu_rpc_para->mcu_read_data);
			libc_printf("\npmu_rpc_para->mcu_rw_offset: 0x%08X", pmu_rpc_para->mcu_rw_offset);
			libc_printf("\nFunction:%s, Line:%d\n", __FUNCTION__, __LINE__);
		#endif
		}

		if((ALI_S3811 == sys_ic_get_chip_id()) && (sys_ic_get_rev_id() >= IC_REV_1))
		{
		#ifdef PMU_POWERSTANDBY_CLOSE
			pmu_m36_powerstandby_close();
		#else
            libc_printf("\nFunction:%s, Line:%d, pmu standby flow start......\n", __FUNCTION__, __LINE__);
			pmu_m36_en();
		#endif
		}
		else if(ALI_C3702 == sys_ic_get_chip_id())
		{
			PMU_Set_Panel_Key();
			hld_pmu_remote_call(0, 0,SET_SHOW_PANEL_TYPE, pmu_rpc_para);
            pmu_rpc_para->mcu_rw_offset = WDT_REBOOT_FLAG_READ_ADDR & 0xFFFF;
    		pmu_rpc_para->mcu_write_data = ALISL_PWR_STANDBY;
    		hld_pmu_remote_call(0, 0, RPC_MCU_WRITE32, pmu_rpc_para);
			/*STB enter standby mode.*/
			libc_printf("\nFunction:%s, Line:%d, pmu standby flow start......\n", __FUNCTION__, __LINE__);
			hld_pmu_remote_call((UINT32)(g_ali_pmu_bin), sizeof(g_ali_pmu_bin)/sizeof(UINT8),
				STB_ENTER_PMU_STANDBY, pmu_rpc_para);
		}
		else
		{
			pmu_bin.buf = (UINT8*)(g_ali_pmu_bin);
			pmu_bin.len = sizeof(g_ali_pmu_bin)/sizeof(UINT8);
			pmu_io_control(pmu_dev, PMU_PANNEL_POWER_STATUS, PANNLE_POWER_ON);
			pmu_io_control(pmu_dev,PMU_SHOW_TYPE_CMD, SHOW_OFF);
			pmu_io_control(pmu_dev, ALI_PMU_SAVE_WDT_REBOOT_FLAG, ALISL_PWR_STANDBY);
			pmu_io_control(pmu_dev, PMU_LOAD_BIN_CMD, (UINT32)(&pmu_bin));
		#ifdef PMU_RETENTION_SUPPORT
			Set_PMU_Retention_Flag();
		#endif
			PMU_Set_Panel_Key();
			libc_printf("\nFunction:%s, Line:%d, pmu standby flow start......\n", __FUNCTION__, __LINE__);
			pmu_m36_en();
		}
	}

	if((ALI_S3811 == sys_ic_get_chip_id()) && (sys_ic_get_rev_id() == IC_REV_0))
	{
	    libc_printf("\nFunction:%s, Line:%d, pmu standby flow start......\n", __FUNCTION__, __LINE__);
		pmu_m36_en();
	}
}
#endif

void power_switch(UINT32 mode)
{
    unsigned long keycode = 0;
    SYSTEM_DATA *sys_data = NULL;
    UINT32 vkey = 0;
    UINT32 times = 0;
    UINT32 display_type=0;
    UINT32  cur_time = 0;
    date_time dt;
    date_time dt_bak;
    UINT32 hh = 0;
    UINT32 mm = 0;
    char time_str[10] = {0};
    struct pan_key key_struct;
    UINT32 timer = 0;
#ifdef _C0200A_CA_ENABLE_
    UINT32 timer_emmwakeup = 0;
#endif
    struct sto_device *dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    sys_state_t __MAYBE_UNUSED__ system_state = SYS_STATE_INITIALIZING;

#if (defined(USB_SUPPORT_HUB)&&(!defined(_BUILD_OTA_E_)))
    //make sure USB command is complete
    int i=0;
    #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
    usb_inhibit_rw(1);
    #endif
    for(i=0;i<1000;i+=10)
    {
        osal_task_sleep(10);
	#if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
        if(TRUE == usb_rw_completed())
        {
            break;
        }
	#endif
    }
    #if((!defined(_M3503D_)) && (!defined(_M3711C_)))//temp close for fix compiler error.
    force_usb_suspend();
    #endif
#endif
	MEMSET(&dt, 0, sizeof(date_time));
	MEMSET(&dt_bak, 0, sizeof(date_time));
    MEMSET(&key_struct, 0, sizeof(struct pan_key));
#if defined(SUPPORT_BC_STD) || defined(SUPPORT_BC)
    api_check_osm_triggers(OSM_POWER_DOWN);
#endif
#ifdef SUPPORT_FRANCE_HD
    save_nit_monitor_date(void);
#endif

#ifdef  SUPPORT_CAS9
    clean_mmi_msg(CA_MMI_PRI_05_MSG,1);
#endif
    sys_data = sys_data_get();
    sys_data->bstandmode = 1;

    #ifdef BG_TIMER_RECORDING
     //save the right time//derek201406  //by derek.yang 2014 Jun related to timer rec (stand by)
    if(sys_data->local_time.buse_gmt)
    {
        get_stc_time(&sys_data->sys_dt);
    }
    else
    {
        get_local_time(&sys_data->sys_dt);
    }
    #endif

	//save time offset get from stream before enter standby
	if(system_config.local_time.buse_gmt) 
	{
		INT32 h = 0,m = 0;
		get_stc_offset_stream(&h,&m);
		sys_data->local_time.gmtoffset_h = h;
		sys_data->local_time.gmtoffset_m = m;
		sys_data->local_time.on_off = 1;
	}

    sys_data_save(1);
    system_state = api_get_system_state();

#ifndef _BUILD_OTA_E_
#ifdef MEDIA_PLAYER_VIDEO_SUPPORT
    if(SYS_STATE_USB_MP == system_state)
    {
        mpg_cmd_stop_proc(0);
        mpg_cmd_stop_proc(1);
    }
#endif
#endif
   api_set_system_state(SYS_STATE_POWER_OFF);

//Power off USB
    board_usb_power(FALSE);

#ifdef FSC_SUPPORT
    fsc_control_play_stop_all(1);
#endif

    #ifdef AUTO_OTA
    api_stop_play(0);
    if(!power_off_process(1))
    {
        power_on_process();
        return;
    }
    #else
    power_off_process(0);
    #endif

    power_off_process2();
//in "power_off_process()" call api_stop_record(), the function may change database
//so save data again here
    sys_data_save(1);
	if(mode != 1)
	{
 #ifdef DDR_POWER_CONTROL_ENABLE
	// enable DDR power control gpio
	//GPIO_enable_pos(DDR_POWER_CTL_GPIO_POS, TRUE);
	key_pan_display("    ", 4);
#else
	key_pan_display("oFF ", 4);
#endif
	}

	key_pan_display_standby(1);
	key_pan_display_lock(0);
#ifdef SHOW_TWO_TUNER_LOCK_STATUS
	key_pan_display_lock2(0);
#endif

	api_standby_led_onoff(TRUE);
	board_power(FALSE);
	//GPIO_power_onoff(FALSE);

	get_local_time(&dt);
	get_stc_time(&dt_bak);
	pan_close(g_pan_dev);

	/*Get timer info for standby.*/
	timer = api_get_recently_timer();

#if defined(_C0200A_CA_ENABLE_) && !defined(_BUILD_UPG_LOADER_)
	if(sys_data->emmwakeup_en)
	{
		nvcak_ird_waking_timer_stop();   
		timer_emmwakeup = nvcak_ird_get_sleeping_timer();
		if((timer == 0) || (timer_emmwakeup <= timer))
		{
			timer = timer_emmwakeup;
		}
	}
#endif

	/*disable interrupt.*/
	osal_interrupt_disable();
	cur_time = (dt.sec & 0x3F ) | ((dt.min & 0x3F )<<6)  | ((dt.hour & 0x1F )<<12) | ((dt.day & 0x1F)<<17)
				| ((dt.month & 0xF) << 22) | (((dt.year % 100) & 0x3F)<<26);
 #ifdef PMU_ENABLE
        libc_printf("\n[%s ::: %d],Enter PMU Standby!\n", __FUNCTION__, __LINE__);
        pmu_standby_set_timer(dt.year, &dt);
 #endif

#ifdef WATCH_DOG_SUPPORT
        dog_stop(0);
#endif

	if(sto_io_control(dev, STO_DRIVER_SUSPEND,0) != SUCCESS)
	{
		POW_PRINTF("sto_io_control() failed!\n");
	}

#ifdef NEW_CPU_STANDBY_ENABLE/*Enter new cpu standby flow----merged with firmware.*/
	if((ALI_S3503 == sys_ic_get_chip_id()) || (ALI_C3505 == sys_ic_get_chip_id()) \
		|| (ALI_C3702==sys_ic_get_chip_id()) || (ALI_C3503D==sys_ic_get_chip_id()) \
		|| (ALI_C3711C == sys_ic_get_chip_id()))
	{
        extern void IP_Power_Down_Process(void);
		IP_Power_Down_Process();
		libc_printf("\nFunction:%s, Line:%d, Enter CPU Standby, wakeup time: 0x%08X, cur_time: 0x%08X\n",
			__FUNCTION__, __LINE__, timer, cur_time);
		sys_enter_cpu_standby(timer, cur_time);
	}
	else
#endif
	{
		sys_ic_enter_standby(timer, cur_time);/*Enter old cpu standby flow----merged with bootloader.*/
	}

	/*enable interrupt.*/
	osal_interrupt_enable();

	while (1)
	{
		times++;
		times = times % 100; 
		osal_delay(5000);

		if( 0 == times)
		{
			//get_cur_time(&hh,&mm,&ss);
			get_local_time(&dt);
			hh = dt.hour;
			mm = dt.min;

			if(0 == display_type)
			{
				snprintf(time_str,10,"%02lu%02lu ",hh,mm);
			}
			else
			{
				snprintf(time_str,10,"%02lu.%02lu",hh,mm);
			}

			key_pan_display(time_str, 5);
			display_type++;
			display_type %= 2;
		}

		vkey = V_KEY_NULL;
		if(key_get_key(&key_struct,0))
		{
			keycode = scan_code_to_msg_code(&key_struct);
			ap_hk_to_vk(0, keycode, &vkey);
		}
		else
		{
			keycode = PAN_KEY_INVALID;
		}

		if(V_KEY_POWER == vkey)
		{
			power_on_process();
		}
	}
}

void power_key_proc_in_popup(void)
{
#ifndef _BUILD_OTA_E_
#ifdef BG_TIMER_RECORDING
	/*check schedule recording, if schedule recording
	**from standby,enable av first.
	*/
	if(TRUE == g_silent_schedule_recording)
	{
		//libc_printf("Turn on display and sound, enbale av\n");
		g_silent_schedule_recording = FALSE;
		enable_av_display();
	}
	else
    #endif
    {
        if( api_pvr_is_rec_only_or_play_rec())
        {
            UINT8 back_saved;
          
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
            win_compopup_set_msg("Please stop record before shutdown,are you sure to stop?", NULL, 0);
            win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
            win_popup_choice_t choice = win_compopup_open_ext(&back_saved);

            if(choice == WIN_POP_CHOICE_YES)
            {
                //stop all record
                api_stop_play_record(FALSE);
                api_pvr_tms_proc(FALSE);
                api_stop_record(0,1);
                api_stop_record(0,1);
            }
        }

        if( !api_pvr_is_rec_only_or_play_rec() )
            power_switch(0);
   }
#endif
}
void restart_stb(void)
{
	sys_data_save(TRUE);
	power_off_process(0);
	power_on_process();												
}

