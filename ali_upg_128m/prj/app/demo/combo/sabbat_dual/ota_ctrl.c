/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: ota_ctrl.c
*
*    Description: ota parameter init & ota information management.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/tsi/tsi.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_search.h>
#include <api/libosd/osd_lib.h>
#include <api/libsi/sie_monitor.h>
#include "./copper_common/system_data.h"
#include "./copper_common/menu_api.h"
#include "control.h"
#include "key.h"
#include "ota_ctrl.h"

#ifdef AUTO_OTA
static BOOL m_ota_inform = TRUE;
static volatile INT8 ota_search_over = 0;

BOOL m_for_fake_standby = FALSE;
BOOL ota_power_flag = FALSE;

extern band_param u_country_param[MAX_BAND_COUNT];
#endif

#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)
void set_ota_inform(BOOL enable_prompt)
{
    m_ota_inform = enable_prompt;
}

BOOL get_ota_inform(void)
{
    return m_ota_inform;
}

#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
void init_ota_dvbt_param(void)
{
    UINT32 freq = 0;
    UINT32 band = 0;
    UINT16 pid = 0;
    UINT8 ota_flag = 0;

    ota_flag = sys_data_get_auto_ota_flag();
    freq = sys_data_get_ota_freq();
    band = sys_data_get_ota_band();
    pid = sys_data_get_ota_pid();

    set_ota_upgrade_found(FALSE);

    if ((ota_flag != 0) && (freq != 0) && (band != 0) && (pid != 0))
    {
        si_set_ota_freq_t(freq);
        si_set_ota_bandwidth(band);
        si_set_ota_ssupid_t(pid);
        set_ota_upgrade_found(TRUE);

        //clean up OTA parameter found by auto scan.
        sys_data_set_ota_freq(0);
        sys_data_set_ota_band(0);
        sys_data_set_ota_pid(0);
        sys_data_set_auto_ota_flag(0);
    }
}
#endif
#ifdef DVBC_SUPPORT
void init_ota_dvbc_param(void)
{
    UINT32 freq = 0;
    UINT32 sym = 0;
    UINT16 pid = 0;
    UINT8  qam = 0;

    freq = sys_data_get_ota_freq();
    sym = sys_data_get_ota_symbol();
    qam = sys_data_get_ota_modulation();
    pid = sys_data_get_ota_pid();

    set_ota_upgrade_found(FALSE);

    if ((freq != 0) && (sym != 0) && (qam != 0)&& (pid != 0))
    {
        si_set_ota_freq_c(freq);
        si_set_ota_symbol_c(sym);
        si_set_ota_modulation_c(qam);
        si_set_ota_ssupid_c(pid);
        set_ota_upgrade_found(TRUE);

        //clean up OTA parameter found by auto scan.
        sys_data_set_ota_freq(0);
        sys_data_set_ota_modulation(0);
        sys_data_set_ota_pid(0);
        sys_data_set_ota_symbol(0);
    }
}
#endif

void ui_check_ota(void)
{
    POBJECT_HEAD menu = NULL;
    SYSTEM_DATA* sys_data = sys_data_get();
    UINT32 prog_num = get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0);

#ifdef ACTIVE_STANDBY
    if(TRUE == get_active_stby_flag())
    {
        return;
    }
#endif

    if(!(get_ota_upgrade_found()) ||( !get_ota_inform()))
    {
        return;
    }

    /* check menu */
    /* only pass on NULL or Chanbar or ParentLck */

    menu = menu_stack_get_top();
#ifndef NEW_MANUAL_OTA
    if((prog_num!=0)&&((menu!=NULL)&&(menu!=(POBJECT_HEAD)&g_win2_progname)&&(menu!=(POBJECT_HEAD)&g_win_sigstatus)))
#else
    if(menu == (POBJECT_HEAD)&g_win_welcom) //show "ota upgrade pop" in menu interface
#endif
    {
        return;
    }
#ifdef PARENTAL_SUPPORT
    /* close parental lock */
    if (get_current_stream_rating() != 0)
    {
        set_current_stream_rating(0);
    }
#endif
    tsi_select(TSI_DMX_0, ts_route_get_nim_tsiid(0));
    tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);

    ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OTA_INFORM, 0, FALSE);
}

#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))
static BOOL get_ota_parameter(POTA_PARAM param)
{
	union ssu_info ssu;

    MEMSET(&ssu,0,sizeof(ssu));
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
	si_get_ota_ssuinfo_t(&ssu);
	param->freq = ssu.t_ssuinfo.frequency;
	param->band = ssu.t_ssuinfo.bandwidth;
	param->pid = ssu.t_ssuinfo.ssu_pid;

	if ( param->freq == 0 )   //ota not found.
	{
		param->freq = 0;
		param->band = 0;
		param->pid = 0;
		return FALSE;
	}
#endif

#if(defined(DVBC_SUPPORT))
	si_get_ota_ssuinfo_c(&ssu);
	param->freq = ssu.c_ssuinfo.frequency;
	param->symbol = ssu.c_ssuinfo.symbol;
	param->qam = ssu.c_ssuinfo.qam;
	param->pid = ssu.c_ssuinfo.ssu_pid;
	
	if ( param->freq == 0 )   //ota not found.
	{
		param->freq = 0;
		param->symbol= 0;
		param->qam =0;
		param->pid = 0;
		return FALSE;
	}
#endif

    return TRUE;
}

static void ota_detect_event(UINT8 aue_type, UINT32 aue_value)
{
	INT8 *p_is_stop = NULL;
	switch(aue_type)
	{
		case SI_OTA_VERSION_UPDATE:
			ota_search_over = 1;
			break;
		case SI_OTA_SEARCH_OVER:
			ota_search_over = 1;
			break;
		case SI_OTA_CHECK_STOPSEARCH_SIGNAL:
			p_is_stop= (INT8*)aue_value;
			*p_is_stop = 0; //ota_srch_stop
			break;
		default:
			break;
    }

}

static void start_ota_detect()
{
	UINT32 i = 0;
	struct as_service_param as_param;
	UINT8 cur_cntry_idx = 0;
	SYSTEM_DATA *sys_data = NULL;

	ota_search_over = 0;

	MEMSET(&as_param, 0, sizeof(as_param));
	as_param.as_method = AS_METHOD_SSU_SEARCH;
	as_param.as_handler = ota_detect_event;
	sys_data = sys_data_get();
	
#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
	as_param.as_frontend_type = FRONTEND_TYPE_T;
	//cur_cntry_idx = sys_data->region_id;
	cur_cntry_idx = sys_data->country;
#if(defined(ISDBT_SUPPORT))
	get_isdbt_bandparam(cur_cntry_idx, (band_param *)&u_country_param);
#elif(defined(DVBT_SUPPORT))
	get_dvbt_bandparam(cur_cntry_idx, (band_param *)&u_country_param);
#endif
#endif

#if(defined(DVBC_SUPPORT))
	as_param.as_frontend_type = FRONTEND_TYPE_C;
	if(sys_data->current_ft_count > 0)
    {
	    as_param.ft[0].c_param.sym= sys_data->current_ft[0].c_param.sym;
	    as_param.ft[0].c_param.constellation = sys_data->current_ft[0].c_param.constellation;
    }
	get_default_bandparam(sys_data->country, (band_param *)&u_country_param);
#endif

	as_param.band_group_param = u_country_param;
#ifdef COMBOUI
	psi_set_search_ftype(as_param.as_frontend_type);
#endif
	for(i=0; i<MAX_BAND_COUNT; i++)
	{
		if(0 == u_country_param[i].start_freq)
			break;
		else
			as_param.scan_cnt++;
	}
	as_service_start(&as_param);
}


int ota_auto_search(POTA_PARAM param,  date_time *wakeup_time)
{
	UINT32 hkey __MAYBE_UNUSED__= 0;
    UINT32 vkey = 0xFF;
	unsigned long keycode = 0;
	struct pan_key key_struct;
	int result = OTA_SEARCH_NOT_FOUND;
	UINT32 addr =0 ;
    UINT32 len = 0;

    MEMSET(param, 0, sizeof(OTA_PARAM));
    MEMSET(&key_struct, 0, sizeof(key_struct));

	addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
	addr &= 0x8fffffff;
	len = __MM_AUTOSCAN_DB_BUFFER_LEN;
	db_search_init((UINT8 *)addr, len);

	start_ota_detect();
	while(!ota_search_over)
	{
		if(key_get_key(&key_struct,0))
		{
			keycode = scan_code_to_msg_code(&key_struct);
			ap_hk_to_vk(0, keycode, &vkey);
		}
		else
		{
			if(ota_power_flag)
			{
			    result = OTA_SEARCH_USER_CANCEL;
			    break;
		    }
		}

		if((vkey == V_KEY_POWER) ||
            //(m_for_fake_standby && (CTRL_MSG_SUBTYPE_KEY_FP == hkey)) ||
       		(!m_for_fake_standby && (V_KEY_MENU == vkey || V_KEY_EXIT== vkey)))
		{
			result = OTA_SEARCH_USER_CANCEL;
			break;
		}

		if(wakeup_time != NULL) //for check wake up timer
		{
			if(wakeup_time->year)
			{
				date_time dt;
				get_local_time(&dt);
				if(api_compare_day_time_ext(&dt,wakeup_time) >= 0) //auto wake up from standby.
				{
					result = OTA_SEARCH_TIMER_WAKEUP;
					break;
				}
			}
//			UpdatePanelAsStandby();
		}
		else
		{
//			UpdateSearchingProgress();
		}
		osal_task_sleep(10);
//		ui_check_tuner();
//		ui_check_vcr();
	}
	if(ota_search_over) //lwj for ota debug
	{
		as_service_stop();
		db_search_exit();
	}

	if(!ota_search_over)
		as_otasearch_service_stop(!m_for_fake_standby);
	else
		get_ota_parameter(param);

	if(param->freq)
	{
		result = OTA_SEARCH_OK;
	}

	return result;
}

// TRUE: bootup right now; FALSE: enter passive standby
BOOL ota_stby_search(date_time *wakeup_time)
{
	BOOL flag_boot = FALSE;
	OTA_PARAM ota_param;
	date_time dt;
    int result = OTA_SEARCH_NOT_FOUND;

#ifdef ACTIVE_STANDBY
	m_for_fake_standby = FALSE;
#else
	#ifdef NEW_MANUAL_OTA
   	extern BOOL ota_set_menu;
   	if(ota_set_menu)
		m_for_fake_standby = FALSE;
   	else
	#endif
	m_for_fake_standby = TRUE;
#endif

    MEMSET(&ota_param, 0, sizeof(OTA_PARAM));
	MEMCPY(&dt, wakeup_time, sizeof(date_time));

	result = ota_auto_search(&ota_param, &dt);
	#ifdef NEW_MANUAL_OTA
   	if(ota_set_menu)
	{	
		UINT8 back_saved;
      	UINT32 msg_code;
		win_popup_choice_t choice;
		if(OTA_SEARCH_OK == result)
        {
      	 
	      	win_compopup_close();		
		  	win_compopup_smsg_restoreback(); 
		  	sys_data_set_ota_freq(ota_param.freq);
		  	sys_data_set_ota_band(ota_param.band);
		  	sys_data_set_ota_pid(ota_param.pid);
		  	sys_data_set_auto_ota_flag(1);
		  	sys_data_save(TRUE);
		  	osal_delay(40);
		  	win_compopup_init(WIN_POPUP_TYPE_OKNO);
		  	win_compopup_set_msg("Are you sure to upgrade the firmware?", NULL, 0);
		  	win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
		  	choice = win_compopup_open_ext(&back_saved);
		  	if(choice == WIN_POP_CHOICE_YES)
		    {
			  	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_OTA_INFORM, 0, FALSE);
			}
		  	else
		  	{
			   	ap_vk_to_hk(0, V_KEY_EXIT, &msg_code);
			   	ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
		    }
        }
	  	else if((OTA_SEARCH_NOT_FOUND== result)||(OTA_SEARCH_USER_CANCEL== result))
		{		
	     	win_compopup_close();
			win_compopup_smsg_restoreback(); 
			win_compopup_init(WIN_POPUP_TYPE_OK);
		    win_compopup_set_msg("Can't find new firmware!", NULL, 0);
		  	choice = win_compopup_open_ext(&back_saved);
			//if(choice == WIN_POP_CHOICE_YES)
		    {
				//UINT32 msg_code; // construct virtual operation
				ap_vk_to_hk(0, V_KEY_EXIT, &msg_code);
				ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
			}
		}
	}
	else
	#endif
	if(OTA_SEARCH_OK == result)
	{
#if (defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT))
		sys_data_set_ota_freq(ota_param.freq);
		sys_data_set_ota_band(ota_param.band);
		sys_data_set_ota_pid(ota_param.pid);
		sys_data_set_auto_ota_flag(1);
#endif	
#ifdef DVBC_SUPPORT
		sys_data_set_ota_freq(ota_param.freq);
		sys_data_set_ota_symbol(ota_param.symbol);
		sys_data_set_ota_pid(ota_param.pid);
		sys_data_set_ota_modulation(ota_param.qam);
#endif	

		sys_data_save(TRUE);
		osal_delay(40);
	}
	else if(OTA_SEARCH_USER_CANCEL == result)
		flag_boot = TRUE;

    return flag_boot;
}
#endif //end <#if(defined(DVBT_SUPPORT)||defined(ISDBT_SUPPORT)||defined(DVBC_SUPPORT))>
#endif //end <#if !defined(_BUILD_OTA_E_) && defined(AUTO_OTA)>


