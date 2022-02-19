/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: system_data_inner.c
*
*    Description: The internal function of system data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#include <hld/deca/deca.h>
#ifdef PMU_ENABLE
#include <hld/pmu/pmu.h>
#endif
#include <api/libdb/tmp_info.h>
#include <api/libsi/si_tdt.h>

#include "../osd_rsc.h"
#include "com_api.h"

#include "system_data_inner.h"

SYSTEM_DATA system_config;
BOOL system_config_protect = FALSE;
extern int m_boot_vpo_active;

extern BOOL is_bootloader_show_logo_keeping_solution();

void sys_data_lock(void)
{
	system_config_protect = TRUE;
}

void sys_data_unlock(void)
{
	system_config_protect = FALSE;
}

BOOL sys_data_poll(void)
{
	return system_config_protect;
}

void sys_data_lang_init(SYSTEM_DATA *system_config)
{
    if (NULL == system_config)
    {
        return;
    }

     // language set
    system_config->lang.osd_lang = ENGLISH_ENV;
    system_config->lang.audio_lang_1 = ENGLISH_ENV;
    system_config->lang.audio_lang_2 = ENGLISH_ENV;
    system_config->lang.sub_lang = ENGLISH_ENV;
    system_config->lang.ttx_lang = ENGLISH_ENV;
}
#ifdef HDTV_SUPPORT
enum HDMI_API_COLOR_SPACE sys_data_vedio_format_to_hdmi_color_space(DIGITAL_FMT_TYPE vedio_frm)
{
    enum HDMI_API_COLOR_SPACE hdmi_color_spc = HDMI_YCBCR_444;
    switch(vedio_frm)
    {
        case SYS_DIGITAL_FMT_BY_EDID:
            hdmi_color_spc =HDMI_RGB;
            break;
        case SYS_DIGITAL_FMT_RGB:
            hdmi_color_spc =HDMI_RGB;
            break;
        case SYS_DIGITAL_FMT_RGB_EXPD:
            hdmi_color_spc =HDMI_RGB;
            break;
        case SYS_DIGITAL_FMT_YCBCR_444:
            hdmi_color_spc =HDMI_YCBCR_444;
            break;
        case SYS_DIGITAL_FMT_YCBCR_422:
            hdmi_color_spc =HDMI_YCBCR_422;
            break;
        default:
            break;
    }
    return hdmi_color_spc;
}
#endif
void sys_data_avset_init(SYSTEM_DATA *psystem_config)
{
    if (NULL == psystem_config)
    {
        return;
    }

    // AV Set
#if (defined(HDTV_SUPPORT) && defined(SD_PVR))
    psystem_config->avset.tv_mode = TV_MODE_PAL;
#else
    #ifdef SUPPORT_CAS9
        psystem_config->avset.tv_mode = TV_MODE_PAL;
    #else
        #if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
            psystem_config->avset.tv_mode = TV_MODE_1080I_25;
        #else
            psystem_config->avset.tv_mode = TV_MODE_PAL;
        #endif
    #endif
#endif
    psystem_config->avset.tv_ratio = TV_ASPECT_RATIO_AUTO;
    psystem_config->avset.display_mode = DISPLAY_MODE_PANSCAN;
    psystem_config->avset.scart_out = SCART_CVBS;
#ifdef HDTV_SUPPORT
    #ifdef SUPPORT_CAS9
        #ifdef CEC_SUPPORT
            psystem_config->avset.scart_out = SCART_YUV;
        #else
            psystem_config->avset.scart_out = SCART_RGB;
        #endif
    #else
        psystem_config->avset.scart_out = SCART_YUV;
    #endif

    #ifndef SCART_RGB_UNSUPPORT
        #ifdef GPIO_RGB_YUV_SWITCH
            if (1 == g_rgb_yuv_switch)
            {
                psystem_config->avset.scart_out = SCART_RGB;
            }
        #endif
    #endif
#endif
    psystem_config->avset.rf_mode = RF_MODE_PAL_BG;
    psystem_config->avset.rf_channel = 38;

#ifdef HDTV_SUPPORT
#ifdef HDMI_CERTIFICATION_UI
    if(sys_ic_get_chip_id() < ALI_S3821)
    {
        psystem_config->avset.video_format = SYS_DIGITAL_FMT_YCBCR_444;        
    }
    else
    {
        psystem_config->avset.video_format = SYS_DIGITAL_FMT_YCBCR_444; 
        //psystem_config->avset.video_format = HDMI_YCBCR_444;   
    }    
#else
    if(ALI_C3702 == sys_ic_get_chip_id())
    {
        psystem_config->avset.video_format = SYS_DIGITAL_FMT_YCBCR_444;
    }
    else
    {
        psystem_config->avset.video_format = SYS_DIGITAL_FMT_RGB;
    }
#endif
       psystem_config->avset.audio_output = SYS_DIGITAL_AUD_LPCM;
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    psystem_config->avset.dual_output = 1;
#else
    psystem_config->avset.dual_output = 0;
#endif

    psystem_config->avset.brightness = 50;
    psystem_config->avset.contrast = 50;
    psystem_config->avset.saturation = 50;
    psystem_config->avset.sharpness = 5;
    psystem_config->avset.hue = 50;
#endif

#ifdef HDMI_DEEP_COLOR
    system_config.avset.deep_color = HDMI_DEEPCOLOR_24;
#endif

#ifdef SPDIF_DELAY_SUPPORT
    system_config.avset.spdif_delay_tm = 0;
#endif
}

void sys_data_osd_set_init(SYSTEM_DATA *psystem_config)
{
    if (NULL == psystem_config)
    {
        return;
    }

    // OSD Set
    psystem_config->osd_set.osd_trans = 0;
    psystem_config->osd_set.pallete = 0;
    psystem_config->osd_set.prog_position = 0;
    psystem_config->osd_set.time_display = 1;
    psystem_config->osd_set.time_out  = 5;
    psystem_config->osd_set.subtitle_display = 1;
#if (CC_ON==1)
    #ifdef CC_BY_OSD
    psystem_config->osd_set.cc_display= 0;
    #endif
#endif
    psystem_config->osd_set.subtitle_lang = SUBTITLE_LANGUAGE_INVALID;
    g_second_osd_mode = OSD_NO_SHOW;
}

void sys_data_timer_set_init(SYSTEM_DATA *psystem_config)
{
    int i=0;
    TIMER_SET_CONTENT *timer = NULL;

    if (NULL == psystem_config)
    {
        return;
    }

    // Timer Set
    psystem_config->timer_set.common_timer_num = 1;
    for(i = 0;i < MAX_COMMON_TIMER_NUM;i++)
    {
        timer = &psystem_config->timer_set.timer_content[i];
        timer->timer_mode       = TIMER_MODE_OFF;
        timer->timer_service    = TIMER_SERVICE_CHANNEL;
        timer->wakeup_year      = DEFAULT_YEAR;
        timer->wakeup_month     = DEFAULT_MONTH;
        timer->wakeup_day       = DEFAULT_DAY;
        timer->wakeup_time = 0;
        timer->wakeup_duration_time = 0;
        timer->wakeup_channel    = 0xFFFFFFFF;
        timer->wakeup_message = TIMER_MSG_BIRTHDAY;
        timer->wakeup_state   = TIMER_STATE_READY;
    }

#ifdef DVR_PVR_SUPPORT
    psystem_config->timer_set.record_timer_num = 1;

#if (MAX_RECORD_TIMER_NUM > 0)
    for(i = MAX_COMMON_TIMER_NUM;i < MAX_TIMER_NUM;i++)
    {
        timer = &psystem_config->timer_set.timer_content[i];
        timer->timer_mode       = TIMER_MODE_OFF;
        timer->timer_service    = TIMER_SERVICE_DVR_RECORD;
        timer->wakeup_year      = DEFAULT_YEAR;
        timer->wakeup_month     = DEFAULT_MONTH;
        timer->wakeup_day       = DEFAULT_DAY;
        timer->wakeup_time = 0;
        timer->wakeup_duration_time = 0;
        timer->wakeup_channel    = 0xFFFFFFFF;
        timer->wakeup_message = TIMER_MSG_BIRTHDAY;
        timer->wakeup_state   = TIMER_STATE_READY;
    }
#endif

#endif
}

void sys_data_dt_init(SYSTEM_DATA *psystem_config)
{
    if (NULL == psystem_config)
    {
        return;
    }

     // System Date Set
    psystem_config->sys_dt.year = DEFAULT_YEAR;
    psystem_config->sys_dt.month = DEFAULT_MONTH;
    psystem_config->sys_dt.day = DEFAULT_DAY;
    psystem_config->sys_dt.hour = 0;
    psystem_config->sys_dt.min = 0;
    psystem_config->sys_dt.sec = 0;
}

void sys_data_pvr_set_init(SYSTEM_DATA *psystem_config)
{
    if (NULL == psystem_config)
    {
        return;
    }

    // PVR Set
#ifdef DVR_PVR_SUPPORT
    //init to default.
    psystem_config->jumpstep = 0;
    psystem_config->record_ttx_subt = TRUE;

    psystem_config->rec_ps_enable = 1;
    psystem_config->tms2rec = 1;

  #if defined(CAS9_PVR_SUPPORT) || defined(BC_PVR_SUPPORT)
    psystem_config->scramble_record_mode = 1; // record as re-encryption for CAS9
  #else
    psystem_config->scramble_record_mode = 0; // record as scramble by default
  #endif
    psystem_config->timeshift = 1;
    psystem_config->record_ts_file_size = 0; // 1G
  #ifdef _INVW_JUICE
    psystem_config->tms2rec = 0;
  #endif

#ifdef NEW_DEMO_FRAME
  #ifdef ONE_RECODER_PVR
    psystem_config->rec_num = 1;
  #else
    psystem_config->rec_num = 2;
  #endif
#else
    psystem_config->rec_num = 1;
#endif

#if defined (SUPPORT_CAS9)||defined (SUPPORT_BC_STD)||defined(SUPPORT_BC)||defined(FTA_ONLY)||defined(SUPPORT_C0200A)
    psystem_config->rec_ps_enable = 0;
    psystem_config->tms2rec = 0;
#endif

#if (defined(SUPPORT_BC) && defined(BC_PVR_SUPPORT))
    psystem_config->rec_num = 2;
#endif

#ifdef SUPPORT_CAS7
    psystem_config->rec_num = 1;
    psystem_config->tms2rec = 0;
    #ifdef CAS7_ORIG_STREAM_RECORDING
        psystem_config->rec_num = 1;
        psystem_config->rec_ps_enable = 0;
        psystem_config->tms2rec = 0;
    #endif
    #ifdef CAS7_PVR_SCRAMBLE//summic
        psystem_config->rec_ps_enable = 0;
        psystem_config->rec_num = 1;
        psystem_config->tms2rec = 0;
    #endif
#endif

    strncpy(psystem_config->rec_disk, "", (16-1));
    strncpy(psystem_config->tms_disk, "", (16-1));
    psystem_config->sd_active = 1;
#endif
}

void sys_data_sat2ip_dlna_set_init(SYSTEM_DATA *psystem_config)
{
    int dlna_rand = 0;

#ifdef SAT2IP_SERVER_SUPPORT
    int i=0;

#endif
	if(0 == dlna_rand)
	{
		;
	}
    if (NULL == psystem_config)
    {
        return;
    }

    // SAT2IP/DLNA Set
#ifdef SAT2IP_SERVER_SUPPORT
    for(i = 0; i < SAT2IP_MAX_SLOT; i++)
    {
        psystem_config->slot_cfg[i] = ~0;
    }
    psystem_config->lnb_cfg = 0;
    psystem_config->sat2ip_en = 1;
#ifdef PVR2IP_SERVER_SUPPORT
	psystem_config->pvr2ip_en = 1;
#endif
#endif
    dlna_rand = RAND(256);

#ifdef DLNA_DMR_SUPPORT
    snprintf(psystem_config->str_dmr_friendly_name, DLNA_FRIENDLYNAME_LEN+1,"ALi Media Render - %d", dlna_rand);
    MEMSET(psystem_config->str_dmr_uuid, 0, 16);
#endif
#ifdef DLNA_DMS_SUPPORT
    snprintf(psystem_config->str_dms_friendly_name, DLNA_FRIENDLYNAME_LEN+1,"ALi Media Server - %d", dlna_rand);
    MEMSET(psystem_config->str_dms_uuid, 0, 16);
#endif
#ifdef SAT2IP_SERVER_SUPPORT
    snprintf(psystem_config->str_sat_ip_friendly_name, DLNA_FRIENDLYNAME_LEN+1,"ALi SAT>IP Server - %d", dlna_rand);
    MEMSET(psystem_config->str_sat_ip_uuid, 0, 16);
    psystem_config->str_sat_ip_boot_id = 0;
    psystem_config->str_sat_ip_device_id = 1;
#endif
}

void sys_data_load_config(SYSTEM_DATA *psystem_config, date_time *pdt)
{
    INT32 ret = 0;

    sys_data_factroy_init();
    //modify for adding welcome page when only open dvbt 2011 10 19
#ifdef  SUMMER_TIME_AUTO
    sys_data_set_country(psystem_config->country);
#endif
    //modify end
    CUR_CHAN_MODE = TV_CHAN;
    PIP_CHAN_MODE = TV_CHAN;
#ifdef FACTROY_SET_1ST_SAT
    sys_data_check_channel_groups();
    if(sys_data_get_group_num() > 0)
    {
        sys_data_set_cur_group_index(1);
    }
#endif

    /* Load data here */
    if((ret = load_tmp_data((UINT8*)psystem_config,sizeof(SYSTEM_DATA))) != SUCCESS)
    {
        osal_task_sleep(50);
        if((ret = load_tmp_data((UINT8*)psystem_config,sizeof(SYSTEM_DATA))) != SUCCESS)
        {
            osal_task_sleep(50);
            erase_tmp_sector();
            sys_data_factroy_init();
            save_tmp_data((UINT8*)psystem_config,sizeof(SYSTEM_DATA));
        }
    }
#ifdef TEMP_INFO_HEALTH
    if(ret == SUCCESS)
    {
        if(sys_data_verify_data((UINT8*)psystem_config,sizeof(SYSTEM_DATA)) != SUCCESS)
        {
            sys_data_factroy_init();
            save_tmp_data((UINT8*)psystem_config,sizeof(SYSTEM_DATA));
        }
    }
#endif
    if ((SUCCESS == ret) && (1 == psystem_config->factory_reset))
    {
        MEMCPY(pdt, &psystem_config->sys_dt, sizeof(date_time)); // backup the data_time
        sys_data_factroy_init(); // reset system_config using new firmware default values
        sys_data_set_cur_chan_mode(TV_CHAN);
        sys_data_set_factory_reset(TRUE);
        MEMCPY(&psystem_config->sys_dt, pdt, sizeof(date_time)); // restore the data_time
    }
}

#ifdef PMU_ENABLE
static void sys_data_pmu_resume_time_offset(date_time *dt)
{
    SYSTEM_DATA* sys_data=NULL;
    PMU_RESUME_FLAG boot_flag=0;
    RET_CODE ret_pmu = 0;
    UINT8 pmu_resume=0;
    date_time current_time;
    INT32 hour_offset=0,min_offset=0,sec_offset=0;

	if(0 == sec_offset)
	{
		;
	}
    MEMSET(&current_time,0,sizeof(date_time));
    
    ret_pmu = pmu_resume_status(&boot_flag);
    if(ret_pmu != 0)
    {
        //libc_printf("%s pmu_resume_status return error.\n",__FUNCTION__);
        pmu_resume=0;
    }
    else
    {
        if(PMU_RESUME == boot_flag)
            pmu_resume=1;
        else
            pmu_resume=0;
    }

    if(1 == pmu_resume)
    {
        sys_data = sys_data_get();
        get_stc_time(&current_time);
        //libc_printf("%s month=%d day=%d hour =%d min=%d\n",__FUNCTION__,current_time.month,current_time.day,current_time.hour,current_time.min);

        if(sys_data->local_time.buse_gmt)
        {
            if(sys_data->local_time.on_off) 
            {
                hour_offset=sys_data->local_time.gmtoffset_h;
                min_offset=sys_data->local_time.gmtoffset_m;
            }
            else
            {
                sys_data_gmtoffset_2_hmoffset(&hour_offset,&min_offset);
            }
        }
        //libc_printf("%s, Line:%d, hour_offset=%d, min_offset=%d\n", __FUNCTION__, __LINE__, hour_offset, min_offset);
        if((0 != hour_offset) || (0 != min_offset))/*resume  time offset after pmu standby wakeup.*/
        {
            
            convert_time_by_offset(dt, &current_time, -hour_offset, -min_offset);
            //libc_printf("%s after convert_time_by_offset,  month=%d day=%d hour=%d min=%d sec=%d\n",__FUNCTION__,dt->month,dt->day,dt->hour,dt->min,dt->sec);
            set_stc_time(dt);
        }
    }
}
#endif


/* Load system time*/
void sys_data_load_system_time(SYSTEM_DATA *psystem_config, date_time *pdt)
{
    const UINT32 year_least = 1900;
    const UINT32 year_most = 2100;
    const UINT8 mon_least = 1;
    const UINT8 mon_most = 12;
    const UINT8 day_least = 1;
    const UINT8 day_most = 31;
    const UINT8 hour_most = 23;
    const UINT8 min_most = 59;
    const UINT8 sec_most = 59;
    INT32 hoffset = 0;
    INT32 moffset = 0;

    if ((NULL == psystem_config) || (NULL == pdt))
    {
        return;
    }

    pdt = &psystem_config->sys_dt;
    if((pdt->year<year_least) || (pdt->year>year_most) || (pdt->month<mon_least) || (pdt->month>mon_most) || \
        (pdt->day<day_least)||(pdt->day>day_most)||(pdt->hour>hour_most) || (pdt->min>min_most) || (pdt->sec>sec_most))
    {
        /* If system is invalid,set system to initial time*/
        pdt->year   = DEFAULT_YEAR;
        pdt->month  = DEFAULT_MONTH;
        pdt->day    = DEFAULT_DAY;
        pdt->hour = 0;
        pdt->min = 0;
        pdt->sec = 0;
    }

    set_stc_time(pdt);

#ifdef PMU_ENABLE
    if((ALI_C3701 == sys_ic_get_chip_id()) \
        || (ALI_S3503 == sys_ic_get_chip_id()) \
        || (ALI_S3821 == sys_ic_get_chip_id()) \
        || (ALI_C3505 == sys_ic_get_chip_id()) \
        || (ALI_C3702 == sys_ic_get_chip_id()) \
        || (ALI_C3503D == sys_ic_get_chip_id()) \
        || (ALI_C3711C == sys_ic_get_chip_id()))
    {
        SYSTEM_DATA *system_data_temp, *data_sys;

        //reset the time offset if boot from pmu.because set local time to pmu, need to convert time to STC time.
        sys_data_pmu_resume_time_offset(pdt);
        
        system_data_temp=sys_data_get();
        //year = base_time_init->year;
        system_data_temp->sys_dt.year=pdt->year;
        system_data_temp->sys_dt.month=pdt->month;
        system_data_temp->sys_dt.day=pdt->day;
        system_data_temp->sys_dt.hour=pdt->hour;
        system_data_temp->sys_dt.min=pdt->min;
        system_data_temp->sys_dt.sec=pdt->sec;

        sys_data_save(1);
        data_sys=sys_data_get();
    }
#endif

    init_utc(pdt);

    if(psystem_config->local_time.buse_gmt)
    {
   		if(system_config.local_time.on_off) 
		{
			hoffset=system_config.local_time.gmtoffset_h;
			moffset=system_config.local_time.gmtoffset_m;
    	}
		else
		{
        	sys_data_gmtoffset_2_hmoffset(&hoffset,&moffset);
		}
		
        set_stc_offset(hoffset, moffset, 0);
        enable_summer_time(psystem_config->local_time.summer_time);
    }
    else
    {
        disable_time_parse();
    }
}

void sys_data_load_av_setting(SYSTEM_DATA *psystem_config)
{
    av_set *av_set = NULL;
    enum HDMI_API_COLOR_SPACE color = HDMI_YCBCR_444;
 
#if 1
    /* Audio Setting */
    api_audio_set_volume(psystem_config->volume);
    sys_data_set_spdif(psystem_config->spdif);
#endif

    /* Video Setting */
    av_set = &psystem_config->avset;
  #ifdef GPIO_RGB_YUV_SWITCH
    if (1 == g_rgb_yuv_switch)
    {
        av_set->scart_out = SCART_RGB;
    }
    else
    {
        av_set->scart_out = SCART_YUV;
    }
  #endif

#ifdef HDMI_DEEP_COLOR
    if(FALSE == is_bootloader_show_logo_keeping_solution())
    {
        sys_data_set_deep_color(system_config.avset.deep_color);
    }
#endif

#ifdef AV_DELAY_SUPPORT
    api_set_avdelay_value(av_set->avdelay_value,AVDELAY_LIVE_PLAY_MODE);
#endif
    if(FALSE == is_bootloader_show_logo_keeping_solution())
    {
        api_video_set_tvout(av_set->tv_mode);
    }

#ifdef _FAST_BOOT_UP
    //add for fast boot up
    g_tv_mode_switch=1;
#endif
    api_scart_tvsat_switch(0); /* modify for TV SlowBlanking */

    /* At system bootup, check scart output and vcr loop through. */
#ifdef POS_SCART_TV_FB
    if((av_set->scart_out == SCART_RGB) &&
        psystem_config->vcrloopmode == VCRLOOP_VCR_MASTER)
    {
        av_set->scart_out = SCART_CVBS;
        psystem_config->vcrloopmode = VCRLOOP_STB_MASTER;
    }
#endif
#ifdef POS_SCART_VCR_DETECT
    if(api_scart_vcr_detect())
    {
        api_scart_rgb_on_off(0);/*Force to CVBS*/
        /*VCR sense connector pin will automatic pull MUX to high to switch*/
        api_scart_out_put_switch(0);
    }
    else
    {
        api_scart_rgb_on_off((av_set->scart_out == SCART_RGB) ? 1 : 0);
      #ifdef VDAC_USE_SVIDEO_TYPE
        api_svideo_on_off((av_set->scart_out == SCART_SVIDEO) ? 1 : 0);
      #endif
        api_scart_out_put_switch(1);
    }
#else
    api_scart_rgb_on_off((SCART_RGB == av_set->scart_out) ? 1 : 0);
#ifdef VDAC_USE_SVIDEO_TYPE
    api_svideo_on_off((SCART_SVIDEO == av_set->scart_out) ? 1 : 0);
#endif
    api_scart_out_put_switch(psystem_config->vcrloopmode);
#endif

    #ifdef HDMI_CERTIFICATION_UI
    extern BOOL check_tv_display_in_hd_mode();
    extern void win_av_set_ratio(SYSTEM_DATA *p_sys_data,UINT16 result);
    if(check_tv_display_in_hd_mode())
    {
        win_av_set_ratio(psystem_config,3); // 16:9
    }
    #endif

    sys_data_set_display_mode(av_set);
    sys_data_set_rf(av_set->rf_mode,av_set->rf_channel);

#ifdef HDTV_SUPPORT
    sys_data_set_brightness(psystem_config->avset.brightness);
    sys_data_set_contrast(psystem_config->avset.contrast);
    sys_data_set_saturation(psystem_config->avset.saturation);
    sys_data_set_sharpness(psystem_config->avset.sharpness);
    sys_data_set_hue(psystem_config->avset.hue);
#ifdef HDMI_CERTIFICATION_UI
    if((sys_ic_get_chip_id() < ALI_S3821)  ||
       (sys_ic_get_chip_id() == ALI_C3503C)||
       (sys_ic_get_chip_id() == ALI_C3503D)|| 
       (sys_ic_get_chip_id() == ALI_C3711C))
    {
        sys_data_set_video_format(psystem_config->avset.video_format);
    }
    else
    {
        color = sys_data_vedio_format_to_hdmi_color_space(psystem_config->avset.video_format);
        api_set_hdmi_color_space(color);
    }
#else
	if(ALI_C3702 == sys_ic_get_chip_id())
	{
        if(psystem_config->avset.video_format == SYS_DIGITAL_FMT_BY_EDID)
        {
            sys_data_set_video_format(psystem_config->avset.video_format);
        }
        else
        {
            color = sys_data_vedio_format_to_hdmi_color_space(psystem_config->avset.video_format);
    		api_set_hdmi_color_space(color);
        }
	}
	else
	{
		sys_data_set_video_format(psystem_config->avset.video_format);
	}

#endif
    sys_data_set_audio_output(psystem_config->avset.audio_output);

#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    psystem_config->avset.dual_output = 1;
#else
    psystem_config->avset.dual_output = 0;
#endif
#endif

#ifdef SPDIF_DELAY_SUPPORT
    sys_data_set_spdif_delay();
#endif
    if(TRUE == is_bootloader_show_logo_keeping_solution())
    {
        m_boot_vpo_active = 0;
    }
}

void sys_data_set_normal_tp_id(UINT32 tp_id)
{
    system_config.normal_tp_id = tp_id;
}

BOOL sys_data_get_pip_tp_switch(void)
{
    return system_config.pip_tp_switch;
}

UINT32 sys_data_get_pip_tp_id(void)
{
    return system_config.pip_tp_id;
}

void sys_data_set_pip_tp_id(UINT32 tp_id)
{
    system_config.pip_tp_id = tp_id;
}

void sys_data_set_spdif(UINT32 mode)
{
    struct deca_device *deca_dev = NULL;

    deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    deca_io_control(deca_dev, DECA_SET_DOLBY_ONOFF, mode?TRUE:FALSE);
    system_config.spdif = mode;
}
#ifdef PVR2IP_SERVER_SUPPORT
UINT32 sys_data_get_pvrip_onoff_config(void)
{
    return system_config.pvr2ip_en;
}
void sys_data_set_pvrip_onoff_config(UINT32 mode)
{
    system_config.pvr2ip_en = mode;
}
#endif

