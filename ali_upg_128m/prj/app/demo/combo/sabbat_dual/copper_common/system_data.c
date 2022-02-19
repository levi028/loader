/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: system_data.c
*
*    Description: The function of system data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <mediatypes.h>
#include <math.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/p_search.h>
#include <api/libdb/tmp_info.h>
#include <hld/dis/vpo.h>
#include <hld/vbi/vbi.h>
#include <hld/rfm/rfm.h>
#include <hld/rfm/rfm_dev.h>
#include <hld/sto/sto.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/osd/osddrv.h>
#include <hld/deca/deca.h>
#include <hal/hal_gpio.h>
#include <api/libsi/si_tdt.h>
#include <api/libdiseqc/lib_diseqc.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_nim_manage.h>
#else
#include <api/libpub/lib_device_manage.h>
#include "../win_com.h"
#endif
#include <api/libpub/lib_cc.h>
#include <hld/decv/decv.h>

#include <hld/nim/nim_tuner.h>

#ifdef CI_SUPPORT
#include <api/libci/ci_plus.h>
#include <bus/tsi/tsi.h>
#endif

#ifdef CAS9_V6    //20130708#1_URI_DA
#include <api/libcas/conax_mmi.h>
#include "../conax_ap/win_ca_uri.h"
#endif

#include "../osd_rsc.h"
#include "com_api.h"
#include "../ctrl_util.h"
#ifdef HDTV_SUPPORT
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#endif

#ifdef USB3G_DONGLE_SUPPORT
#include <api/libusb3g/lib_usb3g.h>
#endif

#ifdef VPN_ENABLE
#include <api/libvpn/vpn.h>
#endif

#ifdef YOUTUBE_ENABLE
#include <api/libyoutube/youtube.h>
#endif
#ifdef _MHEG5_SUPPORT_
#include "../mheg5_ap/mheg5/glue/mheg_app.h"
#endif

#include "system_data.h"
#include "dev_handle.h"
#include "system_data_inner.h"
#include "../platform/board.h"
#include "../ctrl_pip.h"
#ifdef _FAST_BOOT_UP
//add for fast boot up
UINT8 g_tv_mode_switch=0;
#endif
UINT8 g_second_osd_mode = 0;
static SYSTEM_DATA system_config_bak;
#ifndef OSD_16BIT_SUPPORT
static UINT8 g_pal[COLOR_N*4] = {0};
#endif

#ifdef NETWORK_SUPPORT
void get_remote_ip_dft_cfg(PIP_REMOTE_CFG pcfg);
void get_local_ip_dft_cfg(PIP_LOC_CFG pcfg);
#endif

#if 1//def _INVW_JUICE
// Inview: definition moved from header to here
band_param u_country_param[MAX_BAND_COUNT] ;
#endif
char *stream_iso_639lang_abbr[] =
{
    "eng",
    "fre",
    "deu",
    "ita",
    "spa",
    "por",
    "rus",
    "tur",
    "pol",
    "ara",
    "per",
};

char *iso_639lang_multicode[][2] =
{
    {"fre","fra"},
    {"deu","ger"},
    {"spa","esl"},
    {"cze","ces"},
    {"gre","ell"},
    {"swe","sve"},
};

#define STREAM_ISO_639_NUM  (sizeof(stream_iso_639lang_abbr)/sizeof(char*))
#define STREAM_ISO_639_LANG_MULTICODE_NUM  (sizeof(iso_639lang_multicode)/2/sizeof(char*))
#define AUDIO_CODE_CNT 3

#define MAX_FRONTEND_CNT 2

UINT32 get_stream_lang_cnt(void)
{
    return STREAM_ISO_639_NUM;
}

UINT32 get_stream_langm_cnt(void)
{
    return STREAM_ISO_639_LANG_MULTICODE_NUM;
}

/*******************************************************************************
* system data: load / save / get / init APIs
********************************************************************************/

void sys_data_factroy_init(void)
{
    int i=0;

    MEMSET(&system_config,0,sizeof(system_config));

#ifdef TEMP_INFO_HEALTH
    sys_data_peg_init();
#endif

    sys_data_set_menu_password(0);
    sys_data_set_chan_password(0);

    sys_data_lang_init(&system_config);

    system_config.b_lnb_power = LNB_POWER_ON;
    system_config.antenna_power = ANTANNE_POWER_ON;
    system_config.poweroffmode = STANDBY_MODE_UPGRADE;/* 0 -STANDBY_MODE_LOWPOWER, 1 -  STANDBY_MODE_UPGRADE*/
    system_config.vcrloopmode  = VCRLOOP_STB_MASTER; /* VCRLOOP_VCRMASTER = 0, VCRLOOP_RECEIVERMASTER */
    system_config.wakeup_timer=0;
    system_config.factory_reset = TRUE;

    system_config.local_time.buse_gmt = TRUE;
    system_config.local_time.gmtoffset_count = 23;//the default country is ARG,GMToffset=17;
    system_config.region_id = 16;// add BRA, UK = 16
    system_config.local_time.on_off =0;
#ifdef _INVW_JUICE
  // win_time.c has switched to using country so set country to UK
  system_config.local_time.gmtoffset_count = 23; //UK GMToffset=23
  system_config.country = 16;// add BRA, UK = 16
#endif
    system_config.channel_lock = TRUE;
    system_config.channel_lock_level = CHAN_LOCK_NO;
    system_config.menu_lock = TRUE;
#ifdef PARENTAL_SUPPORT
    system_config.rating_sel= 0;
#endif

    sys_data_avset_init(&system_config);

    system_config.volume = 40;

    sys_data_osd_set_init(&system_config);

    system_config.chan_sw = CHAN_SWITCH_ALL;

    /* Default: Disable/TV/No Channel */
    system_config.startup_switch = 0;
    system_config.startup_mode = TV_CHAN;
    system_config.startup_group_index[0] = 0;
    system_config.startup_group_index[1] = 0;   /* All sate group*/
    MEMSET(system_config.statup_chsid,0xFF,sizeof(system_config.statup_chsid));

    sys_data_timer_set_init(&system_config);

    sys_data_dt_init(&system_config);

    system_config.bstandmode    = 0;
    system_config.poweroffmode = 0;
#if 1//def SUPPORT_C0200A
    system_config.chchgvideo_type = CHAN_CHG_VIDEO_FREEZE;
#else
    system_config.chchgvideo_type = CHAN_CHG_VIDEO_BLACK;
#endif
    system_config.install_beep = 0;
    system_config.motor_lmt = DISABLE_LMT;
    system_config.spdif = DEFAULT_SPDIF;
#ifdef AUDIO_DESCRIPTION_SUPPORT
    system_config.ad_service = 0;        // default to disable
    system_config.ad_mode = 0;            // default to off
    system_config.ad_volume_offset = 0;    // default to 0 offset
    system_config.ad_default_mode = 0;    // default to off
#endif
    system_config.multiviewmode = 0;
    for(i=0; i<MAX_PICTURE_GROUP_NUM; i++)
    {
        system_config.cur_sat_idx[i] = (UINT16)(~0);//default value for first burn
    }

#ifdef     FAV_GROP_RENAME
    for(i=0;i<MAX_FAVGROUP_NUM;i++)
    {
        snprintf(system_config.favgrp_names[i],FAV_GRP_NAME_LEN + 1, "Fav group %d", i+1);
    }
#endif

    sys_data_pvr_set_init(&system_config);

    system_config.normal_tp_switch = FALSE;
    system_config.pip_tp_switch = FALSE;

    system_config.normal_group_idx = 0;
    system_config.pip_group_idx = 1;

#ifdef CI_SUPPORT
    system_config.ci_mode = CI_SERIAL_MODE;
#endif

    system_config.pip_support = 0;

#ifdef NETWORK_SUPPORT
    get_local_ip_dft_cfg(&system_config.ip_cfg.local_cfg);
    get_remote_ip_dft_cfg(&system_config.ip_cfg.rmt_cfg);
    system_config.ip_cfg.use_last_cfg = IP_USE_LAST_CFG_DFT;
#endif

#ifdef AUTO_SYANDBY_DEFAULT_ON
    system_config.auto_standby_en = 1;//default automatic enable
#else
    system_config.auto_standby_en = 0;
#endif

#ifdef RAM_TMS_TEST
    system_config.ram_tms_en = 0;
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
//#ifdef DVBC_COUNTRY_BAND_SUPPORT
    system_config.country = GERMANY;
    system_config.germ_servs_type = 0; //GERMANY Kabel Deutschland
//#endif
    //GERMANY Kabel Deutschland
    system_config.current_ft[0].c_param.sym = 6900;
    system_config.current_ft[0].c_param.constellation = QAM256;
    system_config.current_ft[1].c_param.sym = 6900;
    system_config.current_ft[1].c_param.constellation = QAM64;
    //frank ???
    if(MAX_FRONTEND_PARAM > MAX_FRONTEND_CNT)
    {
        system_config.current_ft[2].c_param.sym = 0xffffffff;
        system_config.current_ft[2].c_param.constellation = 0xff;
    }
    system_config.current_ft_count = 2;
#endif
#if (CC_ON==1)
#ifdef CC_BY_OSD
    system_config.cc_control=0;
    system_config.dtvcc_service=0;
#endif
#endif
    if(( dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) > 0 \
        && dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) < MAX_FRONTEND_CNT) \
        && (NULL != g_nim_dev2))
    {
        system_config.antenna_connect_type = ANTENNA_CONNECT_DUAL_DIFF;
    }
    else if(MAX_TUNER_SUPPORT_NUM == dev_get_device_cnt(HLD_DEV_TYPE_NIM,FRONTEND_TYPE_T))
    {
        system_config.antenna_connect_type = ANTENNA_CONNECT_DUAL_SAME;
    }
    else if(MAX_TUNER_SUPPORT_NUM == dev_get_device_cnt(HLD_DEV_TYPE_NIM,FRONTEND_TYPE_C))
    {
        system_config.antenna_connect_type = ANTENNA_CONNECT_DUAL_SAME;
    }
    else
    {
        #if defined(SUPPORT_TWO_TUNER) && defined(BC_PVR_SUPPORT)
        system_config.antenna_connect_type = ANTENNA_CONNECT_DUAL_SAME;
        #else
        system_config.antenna_connect_type = ANTENNA_CONNECT_SINGLE;//MEMSET have done but code is better to read
        #endif
    }

#ifdef AV_DELAY_SUPPORT
    system_config.avset.avdelay_value = 500;// no delay
#endif

#ifdef _LCN_ENABLE_
    system_config.LCN=0;
#endif
#ifdef MP_SUBTITLE_SETTING_SUPPORT
    system_config.ext_subt_param.font_size = SUBT_FONT_SIZE_NORMAL;
    system_config.ext_subt_param.fg_color = SUBT_COLOR_WHITE;
    system_config.ext_subt_param.bg_color = SUBT_COLOR_TRANSPARENT;
    system_config.ext_subt_param.y_offset = 50;
#endif

#ifdef CAS9_V6 //20130704#2_cache_pin
    UINT8 default_pin[4] = {0};

    default_pin[0]='1';
    default_pin[1]='2';
    default_pin[2]='3';
    default_pin[3]='4';
    sys_data_set_cached_ca_pin(default_pin);
#endif
#ifdef CVBS_HDMI_SWITCH//add
    system_config.tv_output = CVBS_ON;
#endif
    api_parse_defaultset_subblock();
    sys_data_sat2ip_dlna_set_init(&system_config);

#ifdef WIFI_DIRECT_SUPPORT
    system_config.wifi_direct_en = 0;
#endif
    system_config.net_choose_flag = 0;  //default None
#ifdef WIFI_SUPPORT
    memset(system_config.wifi_ap_saved,0x00,sizeof(struct wifi_ap_info)*5);
#endif
#ifdef DB_SUPPORT_HMAC   
    system_config.hmac_valid = 0;
    MEMSET(system_config.hs_hmac,0,HMAC_OUT_LENGTH);
#endif

#ifdef FSC_SUPPORT
    system_config.fsc_en = 0;
#endif

#ifdef FLASH_SOFTWARE_PROTECT_TEST
    system_config.flash_sp_test = 1;
#endif

#ifdef USB_MP_SUPPORT
    system_config.image_slide_setting.u_time_gap = 0xFF;
#endif

#ifdef MULTISTREAM_SUPPORT
    system_config.ms_enable = 1; // Default enable multistream 
#endif
	system_config.satsrch_search_mode = 0;
	system_config.satsrch_prog_type = 0;
	system_config.satsrch_nit_search = 0;
	system_config.satsrch_scan_mode = 0;
	//add by yu use to default audio_channel
	system_config.rev3 = DEFAULT_AUDIO_CHANNEL;
}

SYSTEM_DATA *sys_data_get(void)
{
    return    &system_config;
}

void sys_data_load(void)
{
	BOOL create_view = FALSE;
	BOOL grp_exst = FALSE;
	S_NODE s_node;
	UINT8 group_idx = 0;
	UINT8 group_type = 0;
	UINT8 group_pos = 0;
	UINT16 channel = 0;
	date_time dt;
	__MAYBE_UNUSED__ prog_id_t *prog_id = NULL;
	UINT8 pre_chan_mode = 0;
	UINT8 boot_chan_node = 0;
	UINT16 chan_idx[2] = {0};
	UINT16 boot_group_idx = 0;

	MEMSET(&dt, 0, sizeof(date_time));
	MEMSET(&s_node, 0x0, sizeof (s_node));
	while(sys_data_poll())
	{
		osal_task_sleep(10);
	}
	sys_data_lock();
	sys_data_load_config(&system_config, &dt);
	sys_data_unlock();
	
	if(CUR_CHAN_MODE>TV_CHAN)
	{
		CUR_CHAN_MODE = TV_CHAN;
	}

	if(get_prog_num(VIEW_ALL|CUR_CHAN_MODE, 0) != 0)
	{
		create_view = TRUE;
	}
	else
	{
		create_view = FALSE;
	}

	sys_data_change_normal_tp_view();

	if(system_config.startup_switch && (system_config.startup_mode <= PROG_TVRADIO_MODE))
	{
		UINT16 prog_pos,max_channel;
  		P_NODE p_node;
		
		boot_chan_node = pre_chan_mode = CUR_CHAN_MODE;
		if(system_config.startup_mode==RADIO_CHAN)
		{
			CUR_CHAN_MODE = RADIO_CHAN;
			pre_chan_mode = CUR_CHAN_MODE;
		}
		else if(system_config.startup_mode==TV_CHAN)
		{
			CUR_CHAN_MODE = TV_CHAN;
			pre_chan_mode = CUR_CHAN_MODE;
		}                
        
		if(CUR_CHAN_GROUP_INDEX>=MAX_GROUP_NUM )
	            CUR_CHAN_GROUP_INDEX = 0;
		boot_group_idx = CUR_CHAN_GROUP_INDEX;
		CUR_CHAN_GROUP_INDEX = 0;/* Force to all sate group*/

		if(system_config.startup_mode==RADIO_CHAN || system_config.startup_mode==2)//both
		{
   			chan_idx[0] = P_INVALID_ID;   
            		sys_data_set_cur_chan_mode(RADIO_CHAN);
            		recreate_prog_view(VIEW_ALL|RADIO_CHAN,0);
			prog_id = &system_config.statup_chsid[0];
			max_channel 	= get_prog_num(VIEW_ALL|PROG_RADIO_MODE, 0);		
			if(max_channel > 0)
			{
				prog_pos 	= 0;
				while(prog_pos < max_channel)
				{
					if(get_prog_at(prog_pos, &p_node) != SUCCESS)
					{
						break;
					}
					if(prog_id->sat_id== p_node.sat_id && prog_id->tp_id == p_node.tp_id && prog_id->prog_number ==p_node.prog_number)
					{
						chan_idx[0] = prog_pos;
						break;
					}
					prog_pos++;
				}
			}
			if(chan_idx[0]  != P_INVALID_ID)
				system_config.cur_chan_group[system_config.normal_group_idx][CUR_CHAN_GROUP_INDEX].radio_channel = chan_idx[0] ;
		}

		if(system_config.startup_mode==TV_CHAN || system_config.startup_mode==2)//both
        	{
   			chan_idx[1] = P_INVALID_ID;   
            		sys_data_set_cur_chan_mode(TV_CHAN);
            		recreate_prog_view(VIEW_ALL|TV_CHAN,0);
			prog_id = &system_config.statup_chsid[1];
			max_channel 	= get_prog_num(VIEW_ALL|PROG_TV_MODE, 0);		
			if(max_channel > 0)
			{
				prog_pos 	= 0;
				while(prog_pos < max_channel)
				{
					if(get_prog_at(prog_pos, &p_node) != SUCCESS)
					{
						break;
					}
					if(prog_id->sat_id== p_node.sat_id && prog_id->tp_id == p_node.tp_id && prog_id->prog_number ==p_node.prog_number)
					{
						chan_idx[1] = prog_pos;
						break;
					}
					prog_pos++;
				}
			}
			if(chan_idx[1]  != P_INVALID_ID)
				system_config.cur_chan_group[system_config.normal_group_idx][CUR_CHAN_GROUP_INDEX].tv_channel = chan_idx[1] ;
        	}
        
		if( chan_idx[pre_chan_mode] != P_INVALID_ID)
		{
			sys_data_set_cur_chan_mode(pre_chan_mode);
			create_view = TRUE;
		}
		else
		{
			CUR_CHAN_GROUP_INDEX = boot_group_idx;
			sys_data_set_cur_chan_mode(boot_chan_node);
			system_config.startup_switch = 0;
		}
	}

	sys_data_set_cur_chan_mode(CUR_CHAN_MODE);

	if(CUR_CHAN_GROUP_INDEX>=MAX_GROUP_NUM )
	{
		CUR_CHAN_GROUP_INDEX = 0;
	}

	sys_data_check_channel_groups();

	group_idx = sys_data_get_cur_group_index();
	if(group_idx >= sys_data_get_group_num())
	{
		sys_data_set_cur_group_index(0);
	}

	if(sys_data_get_group_num() > 0)
	{
		create_view = TRUE;
	}

	if(create_view)
	{
		group_idx = sys_data_get_cur_group_index();
		grp_exst = sys_data_get_cur_mode_group_infor(group_idx,&group_type, &group_pos, &channel);

		if(!grp_exst)
		{
			CUR_CHAN_GROUP_INDEX = 0;
			group_type = ALL_SATE_GROUP_TYPE;
		}

		if(ALL_SATE_GROUP_TYPE == group_type)
		{
			recreate_prog_view(VIEW_ALL|CUR_CHAN_MODE,0);
		}
		else if(SATE_GROUP_TYPE == group_type)
		{
			get_sat_at(group_pos,VIEW_ALL,&s_node);
			recreate_prog_view(VIEW_SINGLE_SAT|CUR_CHAN_MODE,s_node.sat_id);
		}
		else
		{
			recreate_prog_view(VIEW_FAV_GROUP|CUR_CHAN_MODE,group_pos);
		}
	}
#ifndef _BUILD_OTA_E_	
	UINT32 len;
	libc_printf("****************SYS**********************\n");
	libc_printf("CUR_CHAN_MODE-------%d\n",CUR_CHAN_MODE);
	libc_printf("CUR_CHAN_GROUP_INDEX-------%d\n",CUR_CHAN_GROUP_INDEX);
	libc_printf("volume-------%d\n",system_config.volume);
	libc_printf("spdif-------%d\n",system_config.spdif);
	libc_printf("****************AV**********************\n");
	libc_printf("tv_mode-------%d\n",system_config.avset.tv_mode);
	libc_printf("tv_ratio-------%d\n",system_config.avset.tv_ratio);
	libc_printf("display_mode-------%d\n",system_config.avset.display_mode);
	libc_printf("scart_out-------%d\n",system_config.avset.scart_out);
	libc_printf("rf_mode-------%d\n",system_config.avset.rf_mode);
	libc_printf("rf_channel-------%d\n",system_config.avset.rf_channel);
	libc_printf("hdmi_en-------%d\n",system_config.avset.hdmi_en);
	libc_printf("video_format-------%d\n",system_config.avset.video_format);
	libc_printf("audio_output-------%d\n",system_config.avset.audio_output);
	libc_printf("ddplus_output-------%d\n",system_config.avset.ddplus_output);
	libc_printf("brightness-------%d\n",system_config.avset.brightness);
	libc_printf("contrast-------%d\n",system_config.avset.contrast);
	libc_printf("saturation-------%d\n",system_config.avset.saturation);
	libc_printf("sharpness-------%d\n",system_config.avset.sharpness);
	libc_printf("hue-------%d\n",system_config.avset.hue);
	libc_printf("dual_output-------%d\n",system_config.avset.dual_output);
	//	libc_printf("avdelay_value-------%d\n",system_config.avset.avdelay_value);
	libc_printf("deep_color-------%d\n",system_config.avset.deep_color);
	//	libc_printf("spdif_delay_tm-------%d\n",system_config.avset.spdif_delay_tm);
	libc_printf("****************OSD**********************\n");
	libc_printf("pallete-------%d\n",system_config.osd_set.pallete);
	libc_printf("time_display-------%d\n",system_config.osd_set.time_display);
	libc_printf("subtitle_display-------%d\n",system_config.osd_set.subtitle_display);
	//	libc_printf("cc_display-------%d",system_config.osd_set.cc_display);
	libc_printf("prog_position-------%d\n",system_config.osd_set.prog_position);
	libc_printf("time_out-------%d\n",system_config.osd_set.time_out);
	libc_printf("osd_trans-------%d\n",system_config.osd_set.osd_trans);
	libc_printf("subtitle_lang-------%d\n",system_config.osd_set.subtitle_lang);
	libc_printf("****************END**********************\n");
	if(sys_data_check_av_set((void*)&system_config,len) != SUCCESS)
	{
		sys_data_avset_init(&system_config);
		system_config.volume = 40;
		sys_data_osd_set_init(&system_config);		
	}
#endif
	MEMCPY(&system_config_bak, &system_config, sizeof(system_config));
	sys_data_load_system_time(&system_config, &dt);
	sys_data_load_av_setting(&system_config);

	/*LNB Setting.*/
	board_lnb_power(0, system_config.b_lnb_power);
	board_lnb_power(1, system_config.b_lnb_power);
	board_antenna_power(0, system_config.antenna_power);
	board_antenna_power(1, system_config.antenna_power);
	set_lnbshort_detect_start(system_config.b_lnb_power);
	osd_set_lang_environment(system_config.lang.osd_lang);
	sys_data_select_audio_language(system_config.lang.audio_lang_1,system_config.lang.audio_lang_2);

#ifdef EPG_MULTI_LANG
	/*epg language setting.*/
	epg_set_lang_code(stream_iso_639lang_abbr[system_config.lang.osd_lang]);
#endif

	set_chchg_local_postion();
	sys_data_set_palette(0);
	if(system_config.rcupos > MAX_RCU_POSNUM)
	{
		system_config.rcupos = 0;
	}

#ifdef CHANCHG_VIDEOTYPE_SUPPORT
	uich_chg_set_video_type(system_config.chchgvideo_type);
	vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_S3601_0"), VDEC_IO_CHANCHG_STILL_PIC, 1);
	vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_AVC_0"), VDEC_IO_CHANCHG_STILL_PIC, 1);
	vdec_io_control((struct vdec_device *)dev_get_by_name("DECV_HEVC_0"), VDEC_IO_CHANCHG_STILL_PIC, 1);
#endif
}

void sys_data_save(INT32 block)
{
    if(    MEMCMP(&system_config_bak, &system_config, sizeof(system_config)))
    {
#ifdef TEMP_INFO_HEALTH
        if(sys_data_verify_data(&system_config,sizeof(system_config)) == ERR_FAILURE )
        {
        	//add by yu 数据异常，立即恢复数据
        	MEMCPY(&system_config, &system_config_bak, sizeof(system_config));
            return;
        }
#endif
        save_tmp_data((UINT8*)&system_config,sizeof(system_config));
        MEMCPY(&system_config_bak, &system_config, sizeof(system_config));
    }
}

BOOL sys_data_get_normal_tp_switch(void)
{
    return system_config.normal_tp_switch;
}

void sys_data_set_normal_tp_switch(BOOL onoff)
{
    system_config.normal_tp_switch = onoff;
}

void sys_data_set_pip_tp_switch(BOOL onoff)
{
    system_config.pip_tp_switch = onoff;
}

INT32 sys_data_change_normal_tp_view(void)
{
    P_NODE p_node;
    S_NODE s_node;
    UINT8 group_idx = 0;
    UINT8 group_type = 0;
    UINT8 group_pos = 0;
    UINT16 channel = 0;
    BOOL grp_exst = FALSE;
    P_NODE playing_pnode;

    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&playing_pnode, 0x0, sizeof(P_NODE));

    if(!sys_data_get_normal_tp_switch())
    {
        return !SUCCESS;
    }

    sys_data_set_normal_tp_switch(FALSE);
    ap_get_playing_pnode(&playing_pnode);
    MEMCPY(&p_node, &playing_pnode, sizeof(P_NODE));

    group_idx = sys_data_get_cur_group_index();

    grp_exst = sys_data_get_cur_mode_group_infor(group_idx,&group_type, &group_pos, &channel);

    if(!grp_exst)
    {
        CUR_CHAN_GROUP_INDEX = 0;
        group_type = ALL_SATE_GROUP_TYPE;
    }

    if(ALL_SATE_GROUP_TYPE == group_type)
    {
        recreate_prog_view(VIEW_ALL|CUR_CHAN_MODE,0);
    }
    else if(SATE_GROUP_TYPE == group_type)
    {
        get_sat_at(group_pos,VIEW_ALL,&s_node);
        recreate_prog_view(VIEW_SINGLE_SAT|CUR_CHAN_MODE,s_node.sat_id);
    }
    else
    {
        recreate_prog_view(VIEW_FAV_GROUP|CUR_CHAN_MODE,group_pos);
    }

    channel = get_prog_pos(p_node.prog_id);

    if(P_INVALID_ID == channel)
    {
        channel = 0;
    }
    sys_data_set_cur_group_channel(channel);

    return SUCCESS;
}

INT32 sys_data_change_normal_tp(P_NODE *p_node)
{
    INT32 ret = 0;
    BOOL  need_recreate_view = TRUE;

#ifdef SAT2IP_CLIENT_SUPPORT
    S_NODE s_node;

#endif
    UINT16 prog_pos = 0;

#ifdef SAT2IP_CLIENT_SUPPORT
    get_sat_by_id(p_node->sat_id, &s_node);
    if (1 == s_node.sat2ip_flag)
    {
        return ret;
    }
#endif

    sys_data_set_cur_intgroup_index(0);
    sys_data_set_normal_tp_id(p_node->tp_id);
#ifdef NEW_DEMO_FRAME
#ifndef _BUILD_OTA_E_
#ifndef SUPPORT_CAS7
    if(sys_data_get_pip_support() || (MAX_TUNER_SUPPORT_NUM == g_tuner_num))
    {
        need_recreate_view = FALSE;
    }
#else
    if(sys_data_get_pip_support())
    {
        need_recreate_view  = FALSE;
    }
#endif

#endif
#endif
    if (need_recreate_view)
    {
        ret = recreate_prog_view(VIEW_SINGLE_TP |CUR_CHAN_MODE, p_node->tp_id);
    }
#if 1//def ENHANCE_PIP_VIEW
    if (FALSE == need_recreate_view)
    {
        ret = api_pip_create_view(0, VIEW_SINGLE_TP |CUR_CHAN_MODE);
    }
#endif
    prog_pos = get_prog_pos(p_node->prog_id);
    if(INVALID_POS_NUM == prog_pos)
    {
        PRINTF("wrong prog pos!\n");
    }
    sys_data_set_normal_group_channel(prog_pos);

    return ret;
}

INT32 sys_data_change_normal_tp_sat2ip(T_NODE *t_node)
{
	INT32 ret = 0;

	//sys_data_set_cur_intgroup_index(0);
	//sys_data_set_normal_tp_id(t_node->tp_id);
#ifdef NEW_DEMO_FRAME
#ifndef _BUILD_OTA_E_
#ifndef SUPPORT_CAS7
	if(sys_data_get_pip_support() || (g_tuner_num == 2))
#else
	if(sys_data_get_pip_support())
#endif
	ret = api_pip_create_view(0, VIEW_SINGLE_TP |CUR_CHAN_MODE);
	else
#endif
#endif
		ret = recreate_prog_view(VIEW_SINGLE_TP |CUR_CHAN_MODE, t_node->tp_id);

	return ret;
}

INT32 sys_data_change_pip_tp(P_NODE *p_node)
{
    INT32 ret = 0;
    UINT16 prog_pos = 0;
    BOOL    need_recreate_view = TRUE;

    sys_data_set_cur_intgroup_index(0);
    sys_data_set_pip_tp_id(p_node->tp_id);
#ifdef NEW_DEMO_FRAME
#ifndef _BUILD_OTA_E_
    #ifndef SUPPORT_CAS7
    if(sys_data_get_pip_support() || (MAX_TUNER_SUPPORT_NUM == g_tuner_num))
    {
        need_recreate_view = FALSE;
    }
    #else
    if(sys_data_get_pip_support())
    {
        need_recreate_view = FALSE;
    }
    #endif
#endif
#endif
    if (need_recreate_view)
    {
        ret = recreate_prog_view(VIEW_SINGLE_TP |CUR_CHAN_MODE, p_node->tp_id);
    }
#ifdef ENHANCE_PIP_VIEW
    if (FALSE == need_recreate_view)
    {
        ret = api_pip_create_view(1, VIEW_SINGLE_TP |CUR_CHAN_MODE);
    }
#endif
    prog_pos = get_prog_pos(p_node->prog_id);
    if(INVALID_POS_NUM == prog_pos)
    {
        PRINTF("wrong prog pos!\n");
    }
    sys_data_set_pip_group_channel(prog_pos);

    return ret;
}


/*******************************************************************************
* system data: Extend APIs
********************************************************************************/
/* Get & Set Menu/Chan password & lock*/
BOOL sys_data_get_menu_lock(void)
{
    return system_config.menu_lock;
}

BOOL sys_data_get_channel_lock(void)
{
    return system_config.channel_lock;
}

UINT32 sys_data_get_menu_password(void)
{
    return system_config.menu_password;
}

UINT32 sys_data_get_chan_password(void)
{
    return system_config.chan_password;
}

void   sys_data_set_menu_lock(BOOL lock)
{
    system_config.menu_lock = lock;
}

void   sys_data_set_channel_lock(BOOL lock)
{
    system_config.channel_lock = lock;
}

void sys_data_set_menu_password(UINT32 password)
{
    system_config.menu_password = password;
}

void sys_data_set_chan_password(UINT32 password)
{
    system_config.chan_password = password;
}

/*Get & set Local longitute & latitute*/

#ifndef NEW_DEMO_FRAME
void set_chchg_local_postion(void)
{
    usals_local local;
    double local_longtitue = 0;
    double local_latitude = 0;
    SYSTEM_DATA *sys_data = NULL;
    struct nim_lnb_info lnb_info;
    UINT32 i = 0;

    sys_data = sys_data_get();
    sys_data_get_local_position(&local);

    local_longtitue = api_usals_local_word2double(local.local_longitude);
    local_latitude  = api_usals_local_word2double(local.local_latitude);
    set_local_position(local_longtitue,local_latitude,TRUE);
#ifndef SUPPORT_TWO_TUNER
    sys_data->antenna_connect_type = ANTENNA_CONNECT_SINGLE;
#endif
    if(sys_data->antenna_connect_type != ANTENNA_CONNECT_DUAL_DIFF)
    {
        sys_data->tuner_lnb_type[1] = sys_data->tuner_lnb_type[0];
        sys_data->tuner_lnb_antenna[1] = sys_data->tuner_lnb_antenna[0];
        if(sys_data->antenna_connect_type == ANTENNA_CONNECT_SINGLE)
        {
            sys_data->tuner_lnb_type[1] = LNB_FIXED;
        }
    }

    lib_nimng_init(sys_data->antenna_connect_type);

    for(i=0;i<2;i++)
    {
        MEMSET(&lnb_info,0,sizeof(struct nim_lnb_info));
        lnb_info.tuner_type = sys_data->tuner_lnb_type[i] + 1;
        if( sys_data->tuner_lnb_type[i] == LNB_FIXED)
        {
            lnb_info.positioner_type = 0;
        }
        else
        {
            lnb_info.positioner_type = (sys_data->tuner_lnb_antenna[i].motor_type == LNB_MOTOR_DISEQC12)? 1: 2;
        }
        lnb_info.lnb_type = sys_data->tuner_lnb_antenna[i].lnb_type;
        lnb_info.lnb_low  = sys_data->tuner_lnb_antenna[i].lnb_low;
        lnb_info.lnb_high = sys_data->tuner_lnb_antenna[i].lnb_high;
        lnb_info.unicable_ub = sys_data->tuner_lnb_antenna[i].unicable_ub;
        lnb_info.unicable_freq = sys_data->tuner_lnb_antenna[i].unicable_freq;
        lib_nimg_set_lnb_info(i+1,&lnb_info);
    }
}
#else
void set_chchg_local_postion(void)
{
    usals_local local;
    double local_longtitue = 0;
    double local_latitude = 0;
    SYSTEM_DATA *sys_data = NULL;
    struct nim_config lnb_info;
    UINT32 i = 0;
    struct nim_device *nim = NULL;
    UINT32 sub_type = 0;

    MEMSET(&lnb_info, 0, sizeof(struct nim_config));
    sys_data = sys_data_get();
    sys_data_get_local_position(&local);

    local_longtitue = api_usals_local_word2double(local.local_longitude);
    local_latitude  = api_usals_local_word2double(local.local_latitude);
    chchg_set_local_position(local_longtitue,local_latitude);

    if(sys_data->antenna_connect_type != ANTENNA_CONNECT_DUAL_DIFF)
    {
        sys_data->tuner_lnb_type[1] = sys_data->tuner_lnb_type[0];
        sys_data->tuner_lnb_antenna[1] = sys_data->tuner_lnb_antenna[0];
        if(ANTENNA_CONNECT_SINGLE == sys_data->antenna_connect_type)
        {
            sys_data->tuner_lnb_type[1] = LNB_FIXED;
        }
    }

    for(i=0;i<2;i++)
    {
        MEMSET(&lnb_info,0,sizeof(struct nim_config));
        lnb_info.antenna.lnb_type = sys_data->tuner_lnb_type[i] + 1;
        if(LNB_FIXED == sys_data->tuner_lnb_type[i])
        {
            lnb_info.antenna.positioner_type = 0;
        }
        else
        {
            lnb_info.antenna.positioner_type = (LNB_MOTOR_DISEQC12 == sys_data->tuner_lnb_antenna[i].motor_type)? 1: 2;
        }
        lnb_info.antenna.lnb_type = sys_data->tuner_lnb_antenna[i].lnb_type;
        lnb_info.antenna.lnb_low  = sys_data->tuner_lnb_antenna[i].lnb_low;
        lnb_info.antenna.lnb_high = sys_data->tuner_lnb_antenna[i].lnb_high;
        lnb_info.antenna.antenna_enable = 1;
        lnb_info.antenna.unicable_ub = sys_data->tuner_lnb_antenna[i].unicable_ub;
        lnb_info.antenna.unicable_freq = sys_data->tuner_lnb_antenna[i].unicable_freq;
        nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, i);
        sub_type = dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM);
        if(FRONTEND_TYPE_S == sub_type)
        {
            dev_set_nim_config(nim, sub_type, &lnb_info);
        }
    }
}
#endif

void sys_data_get_local_position(usals_local *local)
{
    local->local_pos        = system_config.local.local_pos;
    local->local_longitude  = system_config.local.local_longitude;
    local->local_latitude   = system_config.local.local_latitude;
}

void sys_data_set_local_position(usals_local *local)
{
    system_config.local.local_pos       = local->local_pos;
    system_config.local.local_longitude = local->local_longitude;
    system_config.local.local_latitude  = local->local_latitude;

    set_chchg_local_postion();
}

UINT8 sys_data_get_utcoffset(void)
{
    return system_config.local_time.gmtoffset_count;
}

UINT8 sys_data_set_utcoffset(UINT8 offset)
{
    const UINT8 l_off = 47;

    if(offset<=l_off)
    {
        system_config.local_time.gmtoffset_count = offset;
    }

    return RET_SUCCESS;
}

void sys_data_gmtoffset_2_hmoffset(INT32 *hoffset,INT32 *moffset)
{
    UINT8 gmt_offset = 0;

    gmt_offset = system_config.local_time.gmtoffset_count;
    *hoffset=(INT32)(gmt_offset-23)/2;
    *moffset=(INT32)((gmt_offset-23)%2*30);
}

/* Set OSD language */
void sys_data_select_language(UINT8 langid)
{
    osd_set_lang_environment(langid);
    system_config.lang.osd_lang = langid;
}

void sys_data_select_audio_language(UINT8 langid1,UINT8 langid2)
{
    UINT8 langgroup[AUDIO_CODE_CNT][4] = {{0,},};/*default is 2,for support multicode*/
    UINT8 i = 0;
    UINT8 multi_audio_cnt = 0;
    UINT8 strm_lang_num = 0;
    UINT8 strm_multicode_num = 0;
    UINT8 lang_code[4] = {0};
    UINT8 lang_codem[4] = {0};
    const UINT8 aud_cnt_2 = 2;
    const UINT8 aud_cnt_3 = 3;

	if(0 == strm_lang_num)
	{
		;
	}
    if(langid1 >= STREAM_ISO_639_NUM)
    {
        langid1 = 0;
    }
    if(langid2 >= STREAM_ISO_639_NUM)
    {
        langid2 = langid1;
    }

    multi_audio_cnt = aud_cnt_2;
    strm_lang_num = get_stream_lang_cnt();
    strm_multicode_num = get_stream_langm_cnt();
    MEMSET(lang_code,0,4);    // add lang_code init
    MEMSET(lang_codem,0,4);
    MEMCPY(lang_code,stream_iso_639lang_abbr[langid1],3);
    MEMCPY(lang_codem,lang_code,3);
    for(i=0;i<strm_multicode_num;i++)
    {
        if(0 == MEMCMP(iso_639lang_multicode[i][0],lang_code,3))
        {
            MEMCPY(lang_codem,iso_639lang_multicode[i][1],3);
            multi_audio_cnt = 3;
            break;
        }
    }

    if(multi_audio_cnt == aud_cnt_2)
    {
        strncpy((char *)langgroup[0],(const char *)lang_code, (4-1));/*copy Country abrev.*/
        strncpy((char *)langgroup[1],(const char *)stream_iso_639lang_abbr[langid2], (4-1));
    }
    else if(multi_audio_cnt == aud_cnt_3)
    {
        strncpy((char *)langgroup[0],(const char *)lang_code, (4-1));
        strncpy((char *)langgroup[1],(const char *)lang_codem, (4-1));
        strncpy((char *)langgroup[2],stream_iso_639lang_abbr[langid2], (4-1));
    }

    system_config.lang.audio_lang_1 = langid1;
    system_config.lang.audio_lang_2 = langid2;
#ifdef NEW_DEMO_FRAME
    api_set_audio_language((BYTE *)langgroup, multi_audio_cnt );
#else
    uich_chg_set_aud_language((BYTE *)langgroup, multi_audio_cnt);
#endif
}

/*Get & set factory reset */
BOOL sys_data_get_factory_reset(void)
{
    if(system_config.factory_reset)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void sys_data_set_factory_reset(BOOL bset)
{
    if(bset == TRUE)
    {
        system_config.factory_reset = 1;
    }
    else
    {
        system_config.factory_reset = 0;
    }
}

void sys_data_set_multiviewmode(UINT8 flag)
{
    system_config.multiviewmode = flag;
}

UINT8 sys_data_get_multiviewmode(void)
{
    return system_config.multiviewmode;
}

UINT8 sys_data_get_dms_mode(void)
{
    return system_config.dlna_dms_on;
}

void sys_data_set_dms_mode(UINT8 bonoff)
{
    system_config.dlna_dms_on = bonoff;
}
//////////////////////////////////////////////
void sys_data_set_transparent(UINT8 dev_id)
{
    UINT8  val = 0;
    UINT8  transval = 0;
    struct osd_device *osd_dev = NULL;

     val = system_config.osd_set.osd_trans;
    if(val > MAX_OSD_TRANSPARENCY)
    {
         val = 0;
    }
    transval = (UINT8) ((0xFF * ( 10 - val)) / 10);
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,dev_id);
    osddrv_io_ctl((HANDLE)osd_dev,OSD_IO_SET_GLOBAL_ALPHA,(UINT32)transval);
}

void sys_data_set_palette(UINT8 dev_id)
{
#ifdef OSD_16BIT_SUPPORT
    // use global alpha
    sys_data_set_transparent(dev_id);
#else
    UINT8 *pal_data = NULL;
    INT32 i = 0;
    UINT8 val = 0;
    struct osd_device *osd_dev = NULL;

    pal_data = (UINT8*)osd_get_rsc_pallette(0x4080 | system_config.osd_set.pallete);
    switch(system_config.osd_set.osd_trans)
    {
        case 1:
            val = 1;
            break;
        case 2:
            val = 2;
            break;
        case 3:
            val = 3;
            break;
        case 4:
            val = 4;
            break;
        default:
            val = 0;
            break;
    }
    MEMCPY(g_pal,pal_data,COLOR_N*4);
#if 1
    for(i=0;i<COLOR_N;i++)
    {
        g_pal[4*i+3] = g_pal[4*i+3]*(10-val)/10;
    }
#endif
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,dev_id);
    osddrv_set_pallette((HANDLE)osd_dev, g_pal, COLOR_N, OSDDRV_YCBCR);
#endif
}

void sys_data_set_display_mode(av_set    *av_set)
{
    enum tvmode tvaspect = TV_AUTO;
    enum display_mode e169displaymode = PANSCAN;

    if(TV_ASPECT_RATIO_169 == av_set->tv_ratio)
    {
        tvaspect = TV_16_9;
    }
    else
    {
        tvaspect = TV_4_3;
    }

    if(TV_ASPECT_RATIO_AUTO == av_set->tv_ratio)
    {
        e169displaymode = NORMAL_SCALE;
    }
    else if(DISPLAY_MODE_LETTERBOX == av_set->display_mode)
    {
        e169displaymode = LETTERBOX;
    }
    else if(DISPLAY_MODE_PANSCAN == av_set->display_mode)
    {
        e169displaymode = PANSCAN;
    }
    else
    {
        e169displaymode = NORMAL_SCALE;
    }

    if(TV_ASPECT_RATIO_169 == av_set->tv_ratio)
    {
        e169displaymode = PILLBOX;
    }
#if defined(_MHEG5_V20_ENABLE_)
    mheg5api_notify_video_pref_changed();
#endif
#ifdef _MHEG5_SUPPORT_
    mheg_notify_video_pref_changed();
#endif
    vpo_aspect_mode( (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), tvaspect, e169displaymode);

#ifdef DUAL_VIDEO_OUTPUT
    vpo_aspect_mode((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), tvaspect, e169displaymode);
#endif

    if(TV_ASPECT_RATIO_169 == av_set->tv_ratio)
    {
        api_scart_aspect_switch(1);
    }
    else if(TV_ASPECT_RATIO_43 == av_set->tv_ratio)
    {
        api_scart_aspect_switch(0);
    }
}

UINT8 sys_data_get_aspect_mode(void)
{
    return system_config.avset.tv_ratio;
}
void sys_data_set_rf(UINT8 rf_mode,UINT8 rf_channel)
{
    struct rfm_device    *rfm_dev = NULL;
    UINT32 set_rf_mode = 0;

    rfm_dev =(struct rfm_device *) dev_get_by_type(NULL, HLD_DEV_TYPE_RFM);
    if(NULL == rfm_dev)
    {
        return;
    }
    switch(rf_mode)
    {
    case RF_MODE_NTSC_M:
        set_rf_mode = RFM_SYSTEM_NTSC_M;
        break;
    case RF_MODE_PAL_I:
        set_rf_mode = RFM_SYSTEM_PAL_I;
        break;
    case RF_MODE_PAL_DK:
        set_rf_mode = RFM_SYSTEM_PAL_DK;
        break;
    case RF_MODE_PAL_BG:
    default:
        set_rf_mode = RFM_SYSTEM_PAL_BG;
        break;
    }

    rfm_system_set(rfm_dev, set_rf_mode);
    rfm_channel_set(rfm_dev, rf_channel);
}

UINT16 sys_data_get_cur_satidx(void)
{
    return (system_config.cur_sat_idx[system_config.normal_group_idx]);
}

void sys_data_set_cur_satidx(UINT16 sat_idx)
{
    system_config.cur_sat_idx[system_config.normal_group_idx] = sat_idx;
}

#if    (TTX_ON == 1)
UINT8 sys_data_get_ttxpatch(void)
{
    UINT8 g0_set = 0;

    switch(system_config.lang.ttx_lang)
        {
            case ENGLISH_ENV:
            case FRANCE_ENV:
            case ITALY_ENV:
            case PORTUGUESE_ENV:
            case SPANISH_ENV:
            case GERMANY_ENV:
                g0_set = LATIN_0;
                break;
            case TURKISH_ENV:
                g0_set = LATIN_2;
                break;
            case POLISH_ENV:
                g0_set = LATIN_1;
                break;
            case RUSSIAN_ENV:
                g0_set = LATIN_CYRILLIC;
                break;             
            case ARABIC_ENV:
                g0_set = LATIN_ARABIC;
                break;
            case GREEK_ENV:
                g0_set = LATIN_GREEK;
                break;
            default:
                g0_set = LATIN_0;
                break;
        }
    return g0_set;
}

void ttxeng_set_g0_set(void)
{
    UINT8 lang_font=0;

    lang_font = sys_data_get_ttxpatch();
    ttxeng_default_g0_set(lang_font);
}
#endif

BOOL sys_data_get_pip_support(void)
{
    return system_config.pip_support;
}

#ifdef HDTV_SUPPORT
enum HDMI_API_COLOR_SPACE sys_data_vfmt_to_hdmi_color_space(DIGITAL_FMT_TYPE vedio_frm)
{
    return sys_data_vedio_format_to_hdmi_color_space(vedio_frm);
}
#endif

//////////////////////////////////

enum TV_SYS_TYPE sys_data_get_tv_mode(void)
{
    return system_config.avset.tv_mode;
}

BOOL sys_data_is_progressive(enum TV_SYS_TYPE tvmode)
{
    BOOL bprogressive =FALSE;

#ifdef HDTV_SUPPORT
    switch (tvmode)
    {
    case TV_MODE_576P:
    case TV_MODE_480P:
    case TV_MODE_720P_50:
    case TV_MODE_720P_60:
    case TV_MODE_1080P_25:
    case TV_MODE_1080P_30:
    case TV_MODE_1080P_24:
    case TV_MODE_1080P_50:
    case TV_MODE_1080P_60:
        bprogressive = TRUE;
        break;
    case TV_MODE_1080I_25:
    case TV_MODE_1080I_30:
        bprogressive = FALSE;
        break;
    default:
        break;
    }
#endif
    return bprogressive;
}

enum TV_SYS_TYPE tv_mode_to_sys_data(enum tvsystem tvsys)
{
    switch(tvsys)
    {
    case PAL:
     return TV_MODE_PAL;
    case PAL_N:
     return TV_MODE_PAL_N;
    case PAL_60:
     return TV_MODE_PAL;
    case NTSC:
     return TV_MODE_NTSC358;
    case PAL_M:
     return TV_MODE_PAL_M;
    case NTSC_443:
     return TV_MODE_NTSC443;
    case SECAM:
     return TV_MODE_SECAM;
#ifdef HDTV_SUPPORT
    case LINE_720_25:
     return TV_MODE_720P_50;
    case LINE_720_30:
     return TV_MODE_720P_60;
    case LINE_1080_25:
     return TV_MODE_1080I_25;
    case LINE_1080_30:
     return TV_MODE_1080I_30;
    case LINE_1080_24:
        return TV_MODE_1080P_24;
    case LINE_1080_50:
        return TV_MODE_1080P_50;
    case LINE_1080_60:
        return TV_MODE_1080P_60;
#endif
    default:
        return TV_MODE_AUTO;
    }
}

enum TV_SYS_TYPE tv_mode_to_sys_data_ext(enum tvsystem tvsys, BOOL bprogressive)
{
    switch(tvsys)
    {
    case PAL:
     return (bprogressive ? TV_MODE_576P : TV_MODE_PAL);
    case PAL_N:
     return TV_MODE_PAL_N;
    case PAL_60:       
     return TV_MODE_PAL;
    case NTSC:
     return (bprogressive ? TV_MODE_480P : TV_MODE_NTSC358);
    case PAL_M:
     return TV_MODE_PAL_M;
    case NTSC_443:
     return TV_MODE_NTSC443;
    case SECAM:
     return TV_MODE_SECAM;
#ifdef HDTV_SUPPORT
    case LINE_720_25:
     return TV_MODE_720P_50;
    case LINE_720_30:
     return TV_MODE_720P_60;
    case LINE_1080_25:
     return (bprogressive ? TV_MODE_1080P_25 : TV_MODE_1080I_25);
    case LINE_1080_30:
     return (bprogressive ? TV_MODE_1080P_30 : TV_MODE_1080I_30);
    case LINE_1080_24:
        return TV_MODE_1080P_24;
    case LINE_1080_50:
        return TV_MODE_1080P_50;
    case LINE_1080_60:
        return TV_MODE_1080P_60;
#endif
    default:
        return TV_MODE_AUTO;
    }
}

enum tvsystem sys_data_to_tv_mode(enum TV_SYS_TYPE tvmode)
{
    enum tvsystem tvs = PAL;

    switch(tvmode)
    {
    case TV_MODE_PAL:  
     return PAL;
    case TV_MODE_PAL_N:
     return PAL_N;
    case TV_MODE_NTSC358:
     return NTSC;
    case TV_MODE_PAL_M:
     return PAL_M;
    case TV_MODE_NTSC443:
     return NTSC_443;
    case TV_MODE_SECAM:
     return SECAM;
#ifdef HDTV_SUPPORT
    case TV_MODE_480P:
     return NTSC;
    case TV_MODE_576P:
     return PAL;
    case TV_MODE_720P_50:
     return LINE_720_25;
    case TV_MODE_720P_60:
     return LINE_720_30;
    case TV_MODE_1080I_25:
     return LINE_1080_25;
    case TV_MODE_1080I_30:
     return LINE_1080_30;
    case TV_MODE_1080P_25:
     return LINE_1080_25;
    case TV_MODE_1080P_30:
     return LINE_1080_30;
    case TV_MODE_1080P_24:
     return LINE_1080_24;
    case TV_MODE_1080P_50:
     return LINE_1080_50;
    case TV_MODE_1080P_60:
     return LINE_1080_60;
    default:
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_OUT_MODE, (UINT32)(&tvs));
        break;

#endif
    }
    return tvs;
}

void sys_data_set_storage(UINT8 type, UINT8 sub_type, UINT8 partition)
{
     system_config.storage_type = type;
     system_config.storage_sub_device = sub_type;
     system_config.storage_sub_dev_partition = partition;
}

UINT8 sys_data_get_sd_ide_statue(void)
{
    return system_config.sd_active;
}

void sys_data_set_sd_ide_statue(UINT8 sd_active)
{
    if(sd_active)
    {
        system_config.sd_active = 1;
    }
     else
    {
         system_config.sd_active = 0;
    }
}

UINT8 sys_data_get_storage_type(void)
{
    return system_config.storage_type;
}

UINT8 sys_data_get_storage_sub_dev(void)
{
     return system_config.storage_sub_device;
}

UINT8 sys_data_get_storage_partition(void)
{
    return system_config.storage_sub_dev_partition;
}

UINT32 sys_data_get_volume(void)
{
    return system_config.volume;
}
UINT32 sys_data_get_default_audio_channel(void)
{
	if(system_config.rev3 <=  AUDIO_DEF_CH_MONO)
	     	return system_config.rev3;
	else
	     	return DEFAULT_AUDIO_CHANNEL;
}
//modify for adding welcome page when only open dvbt 2011 10 19
void sys_data_set_country(UINT8 country)
{
#ifdef SUMMER_TIME_AUTO
    UINT32 cntry_code = 0xFFFFFF;

#endif

    system_config.country = country;
#ifdef _INVW_JUICE
    // Applications still using region to select web sites
    system_config.region_id = country;
#endif

    #ifdef SUMMER_TIME_AUTO
            SYSTEM_DATA* p_sys_data=sys_data_get();
            UINT8 *cntry = win_time_get_country_code( p_sys_data->country );
            if( cntry != NULL )
            {
                cntry_code = cntry[0]<<16 | cntry[1]<<8 | cntry[2];
            }
            tdt_set_country_code( cntry_code );
    #endif
}

UINT8 sys_data_get_country(void)
{
    return system_config.country;
}
//modify end

#if (CC_ON==1)
#ifdef CC_BY_OSD
void sys_data_set_cc_control(UINT8 control)
{
    system_config.cc_control= control;
}
UINT8 sys_data_get_cc_control(void)
{
    return system_config.cc_control;
}
void sys_data_set_dtvcc_service(UINT8 service)
{
    system_config.dtvcc_service= service;
}
UINT8 sys_data_get_dtvcc_service(void)
{
    return system_config.dtvcc_service;
}
#endif
#endif
#ifdef CEC_SUPPORT
UINT32 sys_data_get_mute_state(void)
{
    return system_config.b_mute_sate;
}
void sys_data_set_mute_state(BOOL b_mute_state)
{
    system_config.b_mute_sate=b_mute_state;
}
#endif

#ifdef _LCN_ENABLE_
void sys_data_set_lcn(UINT8 flag)
{
    system_config.LCN = flag;
}

UINT8 sys_data_get_lcn(void)
{
    return system_config.LCN;
}
#endif

void sys_data_set_osd_pallete(UINT8 pallete_val)
{
    system_config.osd_set.pallete = pallete_val;
}

#ifdef WIFI_DIRECT_SUPPORT
UINT32 sys_data_get_wifi_direct_onoff(void)
{
    return system_config.wifi_direct_en;
}

void sys_data_set_wifi_direct_onoff(UINT32 mode)
{
    return system_config.wifi_direct_en= mode;
}
#endif
UINT8 sys_data_get_osd_pallete(void)
{
    return system_config.osd_set.pallete;
}
UINT32 sys_data_get_net_choose_flag(void)
{
    return system_config.net_choose_flag;
}

void sys_date_set_net_choose_flag(UINT32 net_choose_flag)
{
    system_config.net_choose_flag = net_choose_flag;
}

#ifdef WIFI_SUPPORT
void sys_data_get_wifi_ap_list(struct wifi_ap_info *wifi_ap)
{
    memcpy(wifi_ap,system_config.wifi_ap_saved,sizeof(struct wifi_ap_info)*5);
}

void sys_data_save_wifi_ap(struct wifi_ap_info *wifi_ap)
{
    UINT8 i = 0;

    for(i = 3; i > 0; i--);
    {
        memcpy(&(system_config.wifi_ap_saved[i+1]), &(system_config.wifi_ap_saved[i]), sizeof(struct wifi_ap_info));
    }
    memcpy(system_config.wifi_ap_saved, wifi_ap, sizeof(struct wifi_ap_info));
}

void sys_date_delete_wifi_ap(UINT8 index)
{
    UINT8 i = 0;

    if(index > 4)
    {
        return;
    }

    for(i = index; i < 4; i++)
    {
        memcpy(&(system_config.wifi_ap_saved[i]), &(system_config.wifi_ap_saved[i+1]), sizeof(struct wifi_ap_info));
    }
    memset(&(system_config.wifi_ap_saved[4]),0x00,sizeof(struct wifi_ap_info));
}
#endif


UINT32 sys_data_get_fsc_onoff(void)
{
    #ifdef FSC_SUPPORT
    return system_config.fsc_en;
    #else
    return 0;
    #endif 
    
}
#ifdef FSC_SUPPORT
void sys_data_set_fsc_onoff(UINT32 mode)
{  
#ifndef NEW_FCC_DMX_IO    
    fsc_switch_dmx_state(mode);
#endif
    return system_config.fsc_en = mode & 0x00000001;
}
#endif

#ifdef YOUTUBE_ENABLE
void sys_data_get_u2b_setting(youtube_config *u2bcfg)
{
    MEMCPY(u2bcfg, &system_config.u2bcfg, sizeof(youtube_config));
}

void sys_data_set_u2b_setting(youtube_config *u2bcfg)
{
    MEMCPY(&system_config.u2bcfg, u2bcfg, sizeof(youtube_config));
}
#endif

