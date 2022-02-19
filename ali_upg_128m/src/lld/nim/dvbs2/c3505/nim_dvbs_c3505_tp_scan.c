/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 tp scan function such
*		as auto_scan channel_change
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#include "nim_dvbs_c3505.h"


#ifdef ASKEY_PATCH1
	UINT8 g_askey_patch_1_enable = 0;
#endif


static UINT32 last_tp_freq = 0;
static INT32 fft_energy_store[1024];

#ifdef AUTOSCAN_DEBUG
static UINT32 search_times = 0;
#endif
static UINT32 re_search_num = 0;

INT32 c3505_real_freq[TP_MAXNUM];//the real frequency and sysbolrate used by NIM_DRIVER_GET_VALID_FREQ
INT32 c3505_real_sym_rate[TP_MAXNUM];
INT32 c3505_real_tp_number;

UINT16 config_data;
UINT32 tp_lock_time = 0;

#ifdef ANTI_WIMAX_INTF
UINT8 force_adpt_disable = 1;
#endif

extern plsn_state nim_c3505_try_plsn(struct nim_device *dev, UINT8 *index);

/*****************************************************************************
*  INT32 nim_C3505_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
* Description: Set tp information to default
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*  Parameter3: UINT32 sym
*  Parameter4: UINT8 fec
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_channel_change(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *pst_channel_change)
{
	UINT8 data = 0x00;
    UINT32 i = 0;
    UINT32 sym = 0, freq = 0;
    UINT8 fec = 0;
    struct nim_c3505_private *priv = NULL;
    struct nim_c3505_tp_scan_para *tp_scan_param;
	//plsn_state state = PLSN_UNSUPPORT;
	//UINT8 temp_index = 0;
	UINT32 stop_start = 0;//used for exit while loop

    // Check 
    if((NULL == dev->priv) || (NULL == pst_channel_change))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

	priv = (struct nim_c3505_private *)dev->priv;

    tp_lock_time = osal_get_tick();

    freq = pst_channel_change->freq;
    sym = pst_channel_change->sym;
    fec = pst_channel_change->fec; 

    NIM_PRINTF("Enter Fuction %s : Freq = %d, Sym = %d, Fec = %d, change_type = %d\n ", __FUNCTION__, freq, sym, fec, pst_channel_change->change_type);
    
	priv->plsn.super_scan = pst_channel_change->super_scan;
	priv->plsn.plsn_now = pst_channel_change->plsn_db;	
	priv->plsn.plsn_report = pst_channel_change->plsn_db;
	priv->plsn.plsn_finish_flag = 0;

	PLSN_PRINTF("[%s %d]priv->plsn.super_scan=%d, priv->plsn.plsn_now=%d, priv->plsn.plsn_finish_flag=%d\n", __FUNCTION__, __LINE__, priv->plsn.super_scan, priv->plsn.plsn_now, priv->plsn.plsn_finish_flag);	
		
	priv->plsn.plsn_try = 0;
    nim_c3505_set_plsn(dev);
	
    if(pst_channel_change->change_type != NIM_CHANGE_SET_ISID_ONLY)
    	priv->isid->get_finish_flag = 0;
	NIM_PRINTF("[%s %d]priv->isid->get_finish_flag=%d\n", __FUNCTION__, __LINE__, priv->isid->get_finish_flag);
    //------------------------------------------------------------------------------//
    // Set isid
    if((NIM_CHANGE_SET_ISID_ONLY == pst_channel_change->change_type) || (NIM_CHANGE_SET_TP_AND_ISID == pst_channel_change->change_type))
    {
        NIM_PRINTF("Enter Fuction %s : Set isid Only, isid = %d\n ", __FUNCTION__, pst_channel_change->isid->isid_write);
        if(SUCCESS != nim_c3505_set_isid(dev, pst_channel_change->isid))
        {
        	ERR_PRINTF("[%s %d]set isid error!\n", __FUNCTION__, __LINE__);
			return ERR_FAILED;
        }

        // Set isid only
        if (NIM_CHANGE_SET_ISID_ONLY == pst_channel_change->change_type)
        {   
			priv->chanscan_autoscan_flag = 0;
            //osal_mutex_unlock(priv->c3505_mutex);
            return SUCCESS;
        }
    }
    //------------------------------------------------------------------------------//

    // Param  check
	if((0 == freq) || (0 == sym))
    {   
	    ERR_PRINTF("Exit channel change param ERROR in %s \n",__FUNCTION__);
	    return ERR_FAILED;
    }

    tp_scan_param = (struct nim_c3505_tp_scan_para *) comm_malloc(sizeof(struct nim_c3505_tp_scan_para));
    
	if (NULL == tp_scan_param)
	{
		ERR_PRINTF("Alloc tp_scan_param memory error!/n");
		return ERR_NO_MEM;
	}
    
    comm_memset((int *)tp_scan_param, 0, sizeof(struct nim_c3505_tp_scan_para));

    // initial param
    tp_scan_param->est_freq = freq * 1024;
    tp_scan_param->est_rs = sym;
    tp_scan_param->est_fec= fec;
    
	priv->search_type = NIM_OPTR_CHL_CHANGE;

	priv->t_param.t_phase_noise_detected = 0;
	priv->t_param.t_dynamic_power_en = 0;
	priv->t_param.t_last_snr = -1;
	priv->t_param.t_last_iter = -1;
	priv->t_param.t_aver_snr = -1;
	priv->t_param.t_snr_state = 0;
	priv->t_param.t_snr_thre1 = 256;
	priv->t_param.t_snr_thre2 = 256;
	priv->t_param.t_snr_thre3 = 256;
	priv->t_param.phase_noise_detect_finish = 0x00;

    // For fake lock monitor 20180124 paladin
    priv->channel_info->phase_err_stat_cnt = 0;
    priv->channel_info->fake_lock_rst_cnt = 0;

    if (sym > C3505_LOW_SYM_THR)
        priv->channel_info->fake_lock_dete_thr = 2;
    else if (sym > 2500)
        priv->channel_info->fake_lock_dete_thr = 6;
    else
        priv->channel_info->fake_lock_dete_thr = 10;
    // For auto adaptive
#ifdef IMPLUSE_NOISE_IMPROVED
    while(priv->t_param.auto_adaptive_doing)
    {
        comm_sleep(10);
        i++;
        if (i > 10)
            break;
    }
    NIM_PRINTF("Wait auto adaptive timeout = %d\n", i);
    priv->t_param.auto_adaptive_cnt = 0;
    priv->t_param.auto_adaptive_state = 0;

    for (i = 0; i < 32; i++) // For normal, set map beta to default
    {
        if (c3505_map_beta_normal_adaptive_en[i])
        {
    		nim_c3505_set_map_beta_extra(dev, i,  0, priv->t_param.auto_adaptive_state);     
        }
    }
    
    for (i = 0; i < 32; i++) // For short, set map beta to default
    {
        if (c3505_map_beta_short_adaptive_en[i])
        {
		    nim_c3505_set_map_beta_extra(dev, i,  1, priv->t_param.auto_adaptive_state);     
        }
    }
#endif	
    // end

    
	// capture freq
#ifdef DEBUG_CHANNEL_CHANGE
	nim_c3505_cap(dev, freq,  fft_energy_store,  sym);
	nim_c3505_cap(dev, freq,  fft_energy_store,  0);
#endif	

#ifdef CHANNEL_CHANGE_ASYNC
	UINT32 flag_ptn = 0;
	#ifdef __C3505_TDS__
	if (NIM_FLAG_WAIT(&flag_ptn, priv->flag_id, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0) == OSAL_E_OK)
	#endif
	
	#ifdef __C3505_LINUX__
	flag_ptn = nim_flag_read(&priv->flag_lock, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
	if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
	#endif
	{
		// channel chaning, stop the old changing first.
		stop_start = osal_get_tick();
		priv->ul_status.c3505_chanscan_stop_flag = 1;
		NIM_PRINTF("channel changing already, stop it first\n");//autoscan force cancel and start TP scan 
		while (0 == priv->wait_chanlock_finish_flag)
		{
			comm_sleep(1);
			if (osal_get_tick() - stop_start > 15)
			{
				NIM_PRINTF("[%s %d]force stop timeout = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				break;
			}
		}	
		priv->isid->get_finish_flag = 0;
		priv->plsn.plsn_finish_flag = 0;
	}
	NIM_PRINTF("CHANNEL_CHANGE_ASYNC Enable\n");
#endif
	priv->ul_status.phase_err_check_status = 0;
	priv->ul_status.c3505_lock_status = NIM_LOCK_STUS_NORMAL;
	priv->ul_status.m_setting_freq = freq;
	priv->ul_status.c3505_lock_adaptive_done = 0;

	//reset first
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    if(tp_scan_param->est_rs < 1500)   
    {
        nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[3]);     
    }
    else if(tp_scan_param->est_rs < 44000)   
    {
        nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[1]);     
    }
    else      
    {
        if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
        {
            nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[2]); 
        }
        else
        {
            nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[0]); 
        } 
    }

#if (NIM_OPTR_ACM == ACM_CCM_FLAG)  
    // Set for ACM mode autoscan by paladin 2015-12-30
    //data = 0x1f;
    //nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 1
#endif

	nim_c3505_set_work_mode(dev, 0x03);
    nim_c3505_open_ppll(dev);
	nim_c3505_sym_config(dev, sym);
	// time for channel change and soft search.
	nim_c3505_tr_cr_setting(dev, NIM_OPTR_CHL_CHANGE);
    // set tuner and get freq err to correct delta freq
    nim_c3505_cfg_tuner_get_freq_err(dev, tp_scan_param);
	nim_c3505_set_adc(dev);

	nim_c3505_interrupt_mask_clean(dev);

	// hardware timeout setting
	nim_c3505_set_hw_timeout(dev, 0xff);

    // FSM state monitor for PL Lock find
    data = 0x87;
    nim_reg_write(dev, R6F_FSM_STATE, &data, 1);	

	// AGC1 setting
	//	nim_C3505_set_agc1(dev, low_sym, NIM_OPTR_CHL_CHANGE,NIM_FRZ_AGC1_OPEN);

#ifdef ANTI_WIMAX_INTF
    if ((freq > 1680) && (freq < 1700) && (sym > 28000))    
    {
		nim_c3505_set_agc1(dev, 1, NIM_OPTR_CHL_CHANGE,NIM_FRZ_AGC1_OPEN);
    }
	else
	{
		nim_c3505_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE,NIM_FRZ_AGC1_OPEN);
	}
#else
	nim_c3505_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE,NIM_FRZ_AGC1_OPEN);
#endif

#ifdef ASKEY_PATCH1
	if ((((freq > 1447) && (freq < 1453)) || ((freq > 2047) && (freq < 2053))) &&\
		((sym > 14950) && (sym < 15050)))
	{
		g_askey_patch_1_enable = 1;
		nim_c3505_askey_patch_in_set_tp(dev, 1, tp_scan_param);
	}
	else
	{
		g_askey_patch_1_enable = 0;
		nim_c3505_askey_patch_in_set_tp(dev, 0, tp_scan_param);
	}
#endif

	// Set symbol rate
	nim_c3505_set_rs(dev, sym);

	// Set carry offset
	priv->t_param.freq_offset = tp_scan_param->delfreq;
    nim_c3505_set_freq_offset(dev, tp_scan_param->delfreq);

	nim_c3505_cr_setting(dev, NIM_OPTR_CHL_CHANGE);
  	//nim_C3505_set_acq_workmode(dev, NIM_OPTR_CHL_CHANGE0); //Bentao remove it 20150707
	// set sweep range
	nim_c3505_set_fc_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
	nim_c3505_set_rs_search_range(dev, NIM_OPTR_CHL_CHANGE, sym);
	// LDPC parameter
	nim_c3505_fec_set_ldpc(dev, NIM_OPTR_CHL_CHANGE, 0x00, 0x01);

	// paladin.ye 2015-11-12	
	// In ACM mode do not insertion of dummy TS packet  edit for vcr mosaic	
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)		
	nim_c3505_tso_dummy_on(dev);
#endif

	nim_c3505_tso_soft_cbr_off(dev);
	nim_c3505_cr_adaptive_configure (dev, sym);
	
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

	//force stop
	if (priv->plsn.super_scan)
	{
		stop_start = osal_get_tick();
		priv->plsn.search_plsn_force_stop = 1;
		PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
		while (1==priv->plsn.start_search)
		{			
			comm_sleep(2);
			if (osal_get_tick() - stop_start > 6000)
			{
				PLSN_PRINTF("[%s %d]force stop timeout = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				break;
			}
		}		
		priv->plsn.search_plsn_force_stop = 0;
		PLSN_PRINTF("[%s %d]stop cost=%d ms\n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
		PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
		priv->plsn.start_search = 1;		
	}
	
	// re-initial NEW CR TAB
    nim_c3505_cr_new_modcod_table_init(dev,sym);
#ifdef ASKEY_PATCH1
		//nim_c3505_set_cr_prs_table(dev, 15, 0, 0);
#endif

    nim_c3505_cr_new_adaptive_unlock_monitor(dev);
    nim_c3505_nframe_step_tso_setting(dev, sym, 0x01);

	//  CR Feedback Carrier Threshold setting	
    // For low symbol rate tp lock speedup, 07/14/2017 hyman
	//data = 0x70;
	//nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1);
	
#if defined CHANNEL_CHANGE_ASYNC && !defined NIM_S3501_ASCAN_TOOLS
	//priv->cur_freq = freq * 1024;
	priv->cur_freq = tp_scan_param->est_freq;   // Real tuner freq
	priv->cur_sym = sym;
    priv->change_type = pst_channel_change->change_type;
	if ((pst_channel_change->isid != NULL) && (priv->change_type != 0))
	{
		comm_memcpy(priv->isid, pst_channel_change->isid, sizeof(struct nim_dvbs_isid));
	}
	NIM_PRINTF("set flag NIM_FLAG_CHN_CHG_START\n");
#ifdef __C3505_TDS__
	NIM_FLAG_SET(priv->flag_id, NIM_FLAG_CHN_CHG_START);
#endif

#ifdef __C3505_LINUX__
	nim_flag_set(&priv->flag_lock, NIM_FLAG_CHN_CHG_START);
	#endif
#else
	if (NULL == pst_channel_change->isid)
	{
		pst_channel_change->isid = priv->isid;
	}
	nim_c3505_waiting_channel_lock(dev, tp_scan_param->est_freq, sym, pst_channel_change->change_type, pst_channel_change->isid);
    nim_c3505_set_work_mode(dev, 0x03);
#endif

	NIM_PRINTF("    Leave Fuction nim_C3505_channel_change \n");
	//priv->isid->get_finish_flag = 0;
	//priv->ul_status.c3505_chanscan_stop_flag = 0;
#ifdef ANTI_WIMAX_INTF
	if(force_adpt_disable == 1)
	{
		nim_c3505_cr_new_adaptive_unlock_monitor(dev);
		nim_c3505_nframe_step_tso_setting(dev,100,0x00);
	}
	else
	{
		nim_c3505_task_tso_setting(dev,100,0x00);
	}
#else		 
	nim_c3505_cr_new_adaptive_unlock_monitor(dev);
	nim_c3505_nframe_step_tso_setting(dev,100,0x00);
#endif
	comm_free(tp_scan_param);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym)
* 
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*  Parameter3: UINT32 sym
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_waiting_channel_lock(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 change_type, struct nim_dvbs_isid *p_isid)
 {
	UINT32 timeout = 0, locktimes = 200;
	UINT32 tempFreq = 0;
	UINT32 rs = 0;
	UINT8 work_mode, map_type, iqswap_flag, roll_off, modcod;
	UINT8 intdata = 0;
	UINT8 code_rate = 0;
	//UINT8 data = 0;
    UINT8 lock = 0;
    int lock_cnt = 0;
	struct nim_c3505_private *priv = NULL;
	INT32 result;
	UINT32 lock_time_s = 0;
	UINT8 ierr = 0;
	UINT8 try_index = 0;
	plsn_state state = PLSN_UNSUPPORT;
	//UINT8 count_run = 0;
	UINT32 search_start_single = 0; //record timeout for one time
	UINT32 search_single_timeout = 0;
	UINT32 search_start_total = 0; //record timeout for total
	UINT32 search_total_timeout = 0;
	UINT32 stop_start = 0;//used for exit while loop	
	UINT8 err_flag = 0; 
    UINT8 fsm_state = 0;

#ifdef FIND_VALUE_FIRST
	UINT8 use_default_flag = 1;
#endif

#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
	UINT32 pl_find_thr = 0;
    UINT32 time_tr_lock = 0;
    UINT8 pl_find_flag = 0;
    UINT8 tr_lock_flag = 0;
    UINT8 change_work_mode_flag = 0;
#endif

    // Check 
    if(NULL == dev)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
	priv = (struct nim_c3505_private *) dev->priv;   
	if(NULL == priv)
    {
        ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
        return ERR_NO_DEV;
    }	
    
    NIM_PRINTF("Enter Fuction %s : Freq = %d, Sym = %d, change_type = %d \n ", __FUNCTION__, freq, sym, change_type);

    // Set default
	p_isid->get_finish_flag = 0;
	priv->ul_status.c3505_chanscan_stop_flag = 0;
	p_isid->isid_num = 0;
    priv->bb_header_info->stream_mode = 3;
    priv->bb_header_info->stream_type = 1;
    priv->bb_header_info->acm_ccm = 1;

    // Set timeout
	if (sym > 40000)
		locktimes = 204;
	else if (sym < 2000)
		locktimes = 2000;
	else if (sym < 4000)
		locktimes = 1604 - sym / 40;
	else if (sym < 6500)
		locktimes = 1004 - sym / 60;
	else
		locktimes = 604 - sym / 100;

    // Unit is ms
	locktimes *= 2;	
	locktimes += COMPEN_MULTISWITCH_DELAY;   // For some multiswitch,  it need more time to make signal steady.

#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
    pl_find_thr = (900*1024)/sym;   // More margn for channel change
    if(pl_find_thr < 300)
	{
        pl_find_thr = 300;
	}
#endif
	search_start_single = osal_get_tick();
	search_start_total = osal_get_tick();
	lock_time_s = osal_get_tick();

	while (priv->ul_status.c3505_chanscan_stop_flag == 0)
	{
		timeout = osal_get_tick() - lock_time_s;

		if (locktimes < timeout)// hardware timeout, the first timeout will used the upper level's plsn  
		{		
			err_flag = 1;
			goto ERR_HANDLE;
		}		

	#ifdef FIND_VALUE_FIRST//when use default value can't lock, and have find new plsn, use this value
		if (use_default_flag)
		{
			state = nim_c3505_try_plsn(dev, &try_index);
			if (PLSN_FIND == state)
			{
				PLSN_PRINTF("[%s %d]reinit the start time, try_index=%d!\n", __FUNCTION__, __LINE__, try_index);
				use_default_flag = 0;
				lock_time_s = osal_get_tick();//reinit the start time
				err_flag = 0;
			}
		}
	#endif

    nim_reg_read(dev, R04_STATUS, &intdata, 1);

#if 0
    nim_reg_read(dev, R6F_FSM_STATE, &work_mode, 1);
         
    if(0x40 == (work_mode&0x40))
    {
        libc_printf("\n\n\n %%%%%%%% PL Found cost %d ms %%%%%%%%\n\n\n", osal_get_tick() - lock_time_s);
    }  
#endif  
   
#ifdef FORCE_WORK_MODE_IN_CHANGE_TP
		// Tr lock
        if ((0x04 == (intdata&0x04)) && (0 == tr_lock_flag))
        {
            tr_lock_flag = 1;
            time_tr_lock = osal_get_tick();
         }

		// PL find
		if (0 == change_work_mode_flag)
		{      
            nim_reg_read(dev, R6F_FSM_STATE, &work_mode, 1);

            if((0x40 == (work_mode&0x40)) && (pl_find_flag == 0) && (tr_lock_flag))     
            {
                pl_find_flag = 1;
                NIM_PRINTF("PL Find, timeout %d\n", timeout);
            }

			// Can't find pl
			//if ((0 == pl_find_flag) && ((timeout - time_tr_lock) > pl_find_thr))
			if ((0 == pl_find_flag) && ((osal_get_tick() - time_tr_lock) > pl_find_thr) && tr_lock_flag)
			{         
				// Set force DVB-S mode timeout
				change_work_mode_flag = 1;
                nim_reg_read(dev, R6F_FSM_STATE, &fsm_state, 1);
                // Force DVB-S mode
                nim_c3505_set_work_mode(dev, 0x00);
                NIM_PRINTF("Now change to DVB-S mode in waiting channel lock\n");

                if ((fsm_state&0x3f) < 8) 
                {
    		 	    // Set HW
    		  	    nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
                    AUTOSCAN_PRINTF("Reset FSM \n");
                    // Start search
                    nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);   
                } 
			}
		}

#endif
		nim_c3505_get_lock(dev, &lock);
		
        if (1 == lock)	// If lock, get and print TP information
        {
        	lock_cnt++;
        }
        else
        {
        	lock_cnt--;
        	if (lock_cnt < 0)
        	{
        		lock_cnt = 0;
        	}
        }

        if (3 == lock_cnt)
		{			
			NIM_PRINTF("        lock chanel \n");
			NIM_PRINTF("Lock tp cost %d ms time\n", osal_get_tick() - tp_lock_time);
			nim_c3505_lock_unlock_adaptive(dev);	// Speedup the program when change tp, c3505_lock_adaptive_done
			
			priv->plsn.search_plsn_force_stop = 1;	
			PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
			if (priv->plsn.plsn_try)
			{
				priv->plsn.plsn_report = priv->plsn.plsn_try_val;
			}
			else
			{
				//priv->plsn.plsn_report = priv->plsn.plsn_now;
				priv->plsn.plsn_report = nim_c3505_get_plsn(dev);
			}

			priv->plsn.plsn_finish_flag = 1;
			PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);
			
			PLSN_PRINTF("[%s %d]priv->plsn.plsn_report=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_report);
			
            nim_c3505_reg_get_work_mode(dev, &work_mode);
			NIM_PRINTF("            work_mode is %d\n", work_mode);
			switch(work_mode)
			{
				case 0 : {NIM_PRINTF("            work_mode is DVB-S \n"); break;}
				case 1 : {NIM_PRINTF("            work_mode is DVB-S2 \n"); break;}
				case 2 : {NIM_PRINTF("            work_mode is DVB-H8PSK \n"); break;}
				case 3 :
				{
					nim_reg_read(dev, R02_IERR, &ierr, 1);
					NIM_PRINTF("            R02_IERR is %d \n", ierr);
					break;
				}
			}

            // In DVBS2 & Mult stream mode, we need set isid to hardware by software in auto mode
            if(1 == work_mode)
            {   
            	p_isid->get_finish_flag = 0;
                result = nim_c3505_get_isid_after_lock(dev, change_type, p_isid);
				if ((result != SUCCESS) && (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag))
				{
					p_isid->get_finish_flag = 0;
					
					NIM_PRINTF("[%s %d]result=%d, p_isid->get_finish_flag=%d\n", 
					__FUNCTION__, __LINE__, result, p_isid->get_finish_flag);
					
					return result;
				}	
            }    
			
			// Fix a bug which will cause multi stream search TP slow in DVBS mode
			// Whether DVB-S or DVB-S2 mode soft will set get_finish_flag.
			//p_isid->get_finish_flag = 1;
			
			if (priv->isid != p_isid)
			{
				comm_memcpy(priv->isid, p_isid, sizeof(struct nim_dvbs_isid));
			}
			
			priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_CLEAR;
			priv->ul_status.m_cur_freq = tempFreq;
			priv->ul_status.c3505_chanscan_stop_flag = 0;

			// ------ Get information start
			tempFreq = freq;
			nim_c3505_get_freq(dev, &tempFreq);
			NIM_PRINTF("            Freq is %d\n", tempFreq);

			nim_c3505_get_symbol_rate(dev, &rs);
			NIM_PRINTF("            Rs is %d\n", rs);

			nim_c3505_reg_get_code_rate(dev, &code_rate);
			if(0 == work_mode)
			{
				switch(code_rate)
				{
					case 0 : {NIM_PRINTF("            code_rate is 1/2\n"); break;}
					case 1 : {NIM_PRINTF("            code_rate is 2/3\n"); break;}
					case 2 : {NIM_PRINTF("            code_rate is 3/4\n"); break;}
					case 3 : {NIM_PRINTF("            code_rate is 5/6\n"); break;}
					case 4 : {NIM_PRINTF("            code_rate wrong\n"); break;}
					case 5 : {NIM_PRINTF("            code_rate is 7/8\n"); break;}
                    default : break;
				}

			}
			else if(1 == work_mode)
			{
				switch(code_rate)
				{
					case 0 : {NIM_PRINTF("            code_rate is 1/4\n"); break;}
					case 1 : {NIM_PRINTF("            code_rate is 1/3\n"); break;}
					case 2 : {NIM_PRINTF("            code_rate is 2/5\n"); break;}
					case 3 : {NIM_PRINTF("            code_rate is 1/2\n"); break;}
					case 4 : {NIM_PRINTF("            code_rate is 3/5\n"); break;}
					case 5 : {NIM_PRINTF("            code_rate is 2/3\n"); break;}
					case 6 : {NIM_PRINTF("            code_rate is 3/4\n"); break;}
					case 7 : {NIM_PRINTF("            code_rate is 4/5\n"); break;}
					case 8 : {NIM_PRINTF("            code_rate is 5/6\n"); break;}
					case 9 : {NIM_PRINTF("            code_rate is 8/9\n"); break;}
					case 10 : {NIM_PRINTF("            code_rate is 9/10\n"); break;}
                    default : break;
				}
			}

			if(1 == work_mode)
			{
				nim_c3505_reg_get_roll_off(dev, &roll_off);
				switch(roll_off)
				{
					case 0 : {NIM_PRINTF("            roll_off is 0.35\n");break;};
					case 1 : {NIM_PRINTF("            roll_off is 0.25\n");break;};
					case 2 : {NIM_PRINTF("            roll_off is 0.20\n");break;};
					case 3 : {NIM_PRINTF("            roll_off is wrong\n");break;};
                    default : break;
				}
			}
			
			if(1 == work_mode)
			{
				nim_c3505_reg_get_modcod(dev, &modcod);
				if(modcod & 0x01)
				{
					NIM_PRINTF("            Pilot on \n");
				}
				else
				{
					NIM_PRINTF("            Pilot off \n");
				}
				modcod = modcod>>2;
				NIM_PRINTF("            Modcod is %d\n", modcod);
			}

			nim_c3505_reg_get_iqswap_flag(dev, &iqswap_flag);
			NIM_PRINTF("            iqswap_flag is %d\n", iqswap_flag);

			nim_c3505_reg_get_map_type(dev, &map_type);
			//NIM_PRINTF("            map_type is %d\n", map_type);
			if(1 == work_mode)
			{
				switch(map_type)
				{
					case 0 : {NIM_PRINTF("            map_type is HBCD\n"); break;}
					case 1 : {NIM_PRINTF("            map_type is BPSK\n"); break;}
					case 2 : {NIM_PRINTF("            map_type is QPSK\n"); break;}
					case 3 : {NIM_PRINTF("            map_type is 8PSK\n"); break;}
					case 4 : {NIM_PRINTF("            map_type is 16APSK\n"); break;}
					case 5 : {NIM_PRINTF("            map_type is 32APSK\n"); break;}
                    default : break;
				}
			}
			else
			{
				NIM_PRINTF("            map_type is QPSK\n");
			}

			// ------ Get information end

		#ifdef NIM_S3501_ASCAN_TOOLS
			tmp_lock = 0x01;
		#endif			

			priv->t_param.phase_noise_detect_finish = 1;
			if ((work_mode == C3505_DVBS2_MODE) && (map_type == 3) && (priv->t_param.t_phase_noise_detected == 0))
			{
				// S2, 8PSK
				if (code_rate == 4)
				{
					// coderate3/5
					priv->t_param.t_snr_thre1 = 30;
					priv->t_param.t_snr_thre2 = 45;
					priv->t_param.t_snr_thre3 = 85;
				}
				else if ((code_rate == 5) || (code_rate == 6))
				{
					// coderate2/3,3/4
					priv->t_param.t_snr_thre1 = 35;
					priv->t_param.t_snr_thre2 = 55;
				}
				else if (code_rate == 7)
				{
					// coderate5/6
					priv->t_param.t_snr_thre1 = 55;
					priv->t_param.t_snr_thre2 = 65;
				}
				else if (code_rate == 8)
				{
					// coderate8/9
					priv->t_param.t_snr_thre1 = 75;
				}
				else
				{
					// coderate9/10
					priv->t_param.t_snr_thre1 = 80;
				}
			}

			if ((work_mode == C3505_DVBS2_MODE) && (map_type <= 3)) 	//only s2 need dynamic power
				priv->t_param.t_dynamic_power_en = 1;

			//  CR Feedback Carrier Threshold setting, for tai5 3441/v/1555 loss lock
			#if 0	// Fix TP loss lock(pdpd 3430/h/1250) issue
			if(rs < 3000)
			{
				data = 0x08; //using high threshold after lock 
				nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1);
			}
			#endif
			/* Keep current frequency.*/
			priv->ul_status.m_cur_freq = tempFreq;
			nim_c3505_interrupt_clear(dev);
			priv->ul_status.c3505_chanscan_stop_flag = 0;			

			p_isid->get_finish_flag = 1;
			if (priv->ul_status.c3505_chanscan_stop_flag || priv->ul_status.c3505_autoscan_stop_flag)
			{
				ACM_DEBUG_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
					priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
					__FUNCTION__, __LINE__, priv->ul_status.c3505_chanscan_stop_flag,
					priv->ul_status.c3505_autoscan_stop_flag);
				p_isid->get_finish_flag = 0;

				priv->plsn.plsn_finish_flag = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);
				return ERR_FAILED;
			}

			PLSN_PRINTF("[%s %d]lock success, search cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
			NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", 
					__FUNCTION__, __LINE__, p_isid->get_finish_flag);
			return SUCCESS;
		}
		else
		{			
			if (priv->ul_status.c3505_chanscan_stop_flag)
			{								
				priv->ul_status.c3505_chanscan_stop_flag = 0;
				priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
				p_isid->get_finish_flag = 1;
				NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", 
					__FUNCTION__, __LINE__, p_isid->get_finish_flag);
				return ERR_FAILED;
			}
			comm_sleep(1);
			
			// For monitor lock status
            if (0 == (timeout%100))
            {
			    NIM_PRINTF("wait lock... loop = %d, cr04 = 0x%x\n", timeout, intdata);
            }
		}

ERR_HANDLE:
		if (err_flag)
		{
			nim_c3505_get_search_timeout(dev, &search_single_timeout, &search_total_timeout);
			
			state = nim_c3505_try_plsn(dev, &try_index);
				
			if (PLSN_FIND == state) 
			{
				PLSN_PRINTF("[%s %d]reinit the start time!\n", __FUNCTION__, __LINE__);//robin
				lock_time_s = osal_get_tick();//reinit the start time
				err_flag = 0;
			}
			else if ((PLSN_OVER == state) || (PLSN_RUNNING == state))
			{
				comm_sleep(2);//sleep, so the system can dispatch search task
				if ((PLSN_OVER == state) || (osal_get_tick() - search_start_single > search_single_timeout))
				{
					if (osal_get_tick() - search_start_total < search_total_timeout)
					{
						PLSN_PRINTF("[%s %d]start next search plsn, state=%d!, search_single_timeout=%d ms, search_total_timeout=%d ms, timeout=%d ms\n", 
							__FUNCTION__, __LINE__, state, search_single_timeout, search_total_timeout, osal_get_tick() - search_start_single);				
									
						stop_start = osal_get_tick();
						priv->plsn.search_plsn_force_stop = 1;//force stop
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						while (1==priv->plsn.start_search)
						{			
							comm_sleep(2);
							if (osal_get_tick() - stop_start > 6000)
							{
								PLSN_PRINTF("[%s %d]force stop timeout = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
								break;
							}
						}	
						PLSN_PRINTF("[%s %d]stop cost=%d ms\n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
						priv->plsn.search_plsn_force_stop = 0;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

						PLSN_PRINTF("[%s %d]reset demod before next search!\n", __FUNCTION__, __LINE__);
						//reset demod
						nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);	
						priv->plsn.plsn_try = 0;
						nim_c3505_set_plsn(dev);
						comm_sleep(2);
						//start demod
						nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);
						
						priv->plsn.start_search = 1;
						priv->plsn.search_plsn_stop = 0;//to avoid get wrong state when plsn task not start immediate
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_stop);
						try_index = 0;//reinit the try index
						PLSN_PRINTF("[%s %d]reinit try index!\n", __FUNCTION__, __LINE__);
						comm_sleep(5);//sleep, so the system can dispatch search task
						lock_time_s = osal_get_tick();//reinit the start time
						search_start_single = osal_get_tick();
					}
					else
					{
						PLSN_PRINTF("[%s %d]state=%d!\n", __FUNCTION__, __LINE__, state);//robin
						priv->t_param.phase_noise_detect_finish = 1;
						nim_c3505_interrupt_clear(dev);
						NIM_PRINTF("[%s %d]timeout= %d locktimes =%d\n", __FUNCTION__, __LINE__, timeout, locktimes);
						NIM_PRINTF("[%s %d]Lock tp fail cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - tp_lock_time);
						PLSN_PRINTF("[%s %d]lock fail, search cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
						priv->ul_status.c3505_chanscan_stop_flag = 0;
						priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
						p_isid->get_finish_flag = 1;
						NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", __FUNCTION__, __LINE__, p_isid->get_finish_flag);

						priv->plsn.plsn_finish_flag = 1;
						PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);
						priv->plsn.search_plsn_force_stop = 1;
						PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d!\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
						//return ERR_FAILED;
						return ERR_TIME_OUT;
					}
				}
			}
			else
			{
				PLSN_PRINTF("[%s %d]state=%d!\n", __FUNCTION__, __LINE__, state);//robin
				priv->t_param.phase_noise_detect_finish = 1;
				nim_c3505_interrupt_clear(dev);
				NIM_PRINTF("[%s %d]timeout= %d locktimes =%d\n", __FUNCTION__, __LINE__, timeout, locktimes);
				NIM_PRINTF("[%s %d]Lock tp fail cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - tp_lock_time);
				PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
				priv->ul_status.c3505_chanscan_stop_flag = 0;
				priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_SETTING;
				p_isid->get_finish_flag = 1;
				NIM_PRINTF("[%s %d]p_isid->get_finish_flag=%d\n", __FUNCTION__, __LINE__, p_isid->get_finish_flag);

				priv->plsn.plsn_finish_flag = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.plsn_finish_flag=%d!\n", __FUNCTION__, __LINE__, priv->plsn.plsn_finish_flag);
				priv->plsn.search_plsn_force_stop = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d!\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				//return ERR_FAILED;
				return ERR_TIME_OUT;
			}
		}
	}

	priv->ul_status.c3505_chanscan_stop_flag = 0;
	p_isid->get_finish_flag = 1;
	NIM_PRINTF("[%s %d]Leave!\n", __FUNCTION__, __LINE__);
	//return SUCCESS;
	return ERR_FAILED;
}

#ifdef FIND_ACM_TP
/*****************************************************************************
*  INT32 nim_c3505_autoscan_process_acm_tp(struct nim_device *dev, struct NIM_Auto_Scan *pstAuto_Scan)
* 	autoscan function
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct NIM_Auto_Scan *pstAuto_Scan
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_autoscan_find_acm_tp(struct nim_device *dev, UINT32 lock_tp_idx, UINT32 cur_freq, UINT32 cur_sym)
{
    UINT8 work_mode = 0;
    UINT8 modcod = 0;
    UINT8 modcod_last = 0;
    INT32 unlock_cnt = 0;
    INT32 lock_cnt = 0;
    UINT8 error_type  = 0;   // 0 for CRC  1 for unlock
    UINT8 lock = 0;
    INT32 ret_get_bb_header = ERR_CRC_HEADER;
    INT32 crc_correct_cnt = 0;
    INT32 time_out = 0;

    struct nim_dvbs_isid *p_isid = NULL;
    struct nim_dvbs_bb_header_info *p_bb_header_info = NULL;
    struct nim_dvbs_channel_info *p_channel_info = NULL;

    // Device point check
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
    p_isid = (struct nim_dvbs_isid*)(priv->isid);
	p_bb_header_info = (struct nim_dvbs_bb_header_info*)(priv->bb_header_info);
	p_channel_info   = (struct nim_dvbs_channel_info*)(priv->channel_info);

    nim_c3505_reg_get_work_mode(dev, &work_mode);
    p_channel_info->work_mode = work_mode;     
    
    // For debug review
    if (lock_tp_idx == 1)
    {
        ACM_DEBUG_PRINTF("\n\n\n");
        ACM_DEBUG_PRINTF("\n===================================================\n");
        ACM_DEBUG_PRINTF("\n          New Carrier or Polar Direction           \n");
        ACM_DEBUG_PRINTF("\n===================================================\n");
    }
    
    ACM_DEBUG_PRINTF("\n");
    
    ACM_DEBUG_PRINTF("\n------------------------------------\n");
    ACM_DEBUG_PRINTF("          TP  %d  %s              \n", lock_tp_idx, \
        ((1 == work_mode)? "DVB-S2" : "DVB-S"));
    ACM_DEBUG_PRINTF("------------------------------------\n");
    ACM_DEBUG_PRINTF("IF Freq = %d Mhz, Symbol Rate = %d Khz \n", cur_freq, cur_sym);
 
    if(1 == work_mode)
    {   
        while(1)
        {   
            ret_get_bb_header = nim_c3505_get_bb_header_info(dev);
            time_out ++;

            if (TIMEOUT_THRESHOLD <= time_out)
            {
                error_type = 0;
                break;
            }
            
            if (SUCCESS == ret_get_bb_header)
            {
                crc_correct_cnt ++;
                
                if (crc_correct_cnt == CRC_THRESHOLD)
                {
                    ACM_DEBUG_PRINTF("[Way 1]Get BBHeader Success!\n");
                    nim_c3505_show_acm_ccm_info(dev);

                    // when all bbheader CRC are right we can use ISID module
                    if((NIM_STREAM_MULTIPLE == p_bb_header_info->stream_type) && (CRC_THRESHOLD == time_out))
                    {
                         ACM_DEBUG_PRINTF("BBHeader CRC Check Pass!\n");    // For debug paladin
                    }
                    ACM_DEBUG_PRINTF("\n");
                    return SUCCESS;
                }
            }
            else if(ERR_CRC_HEADER == ret_get_bb_header)
            {
                crc_correct_cnt = 0;
                comm_sleep(5);
            }
            else
            {                                           
                crc_correct_cnt = 0;
                unlock_cnt ++;
                if(unlock_cnt >= 25)
                {   
                    error_type = 1;
                    break;
                }
                comm_sleep(100);  
            }
        }

        ACM_DEBUG_PRINTF("[Way 1]Find BBHeader fail[error_type = %d], Change to [Way 2]!\n", error_type);

        // Check Lock stable
        time_out = 0;
        while(1)
        {
            nim_c3505_get_lock(dev, &lock);
            if(1 == lock)
                lock_cnt ++;
            else
                lock_cnt = 0;

            if(200 <= lock_cnt)
                break;

            time_out ++;
            comm_sleep(2);

            if (time_out >= 1500)
            {
                ACM_DEBUG_PRINTF("[Way 2]TP unlock, can't found ACM TP, Exit!\n");
                ACM_DEBUG_PRINTF("\n");
                return ERR_FAILED;
            }
        }

        // Check Mode code
        nim_c3505_reg_get_modcod(dev, &modcod);
        modcod_last = modcod;

        time_out = 0;
        while(1)
        {
            nim_c3505_get_lock(dev, &lock);
            if(1 == lock)
            {
                nim_c3505_reg_get_modcod(dev, &modcod);
            }
            time_out ++;

            if (time_out >= 1500)
            { 
                ACM_DEBUG_PRINTF("[Way 2]Time out! can't found ACM TP, Exit!\n");
                ACM_DEBUG_PRINTF("\n");
                return ERR_FAILED;
            }   

            if (modcod_last != modcod)
            {
                p_bb_header_info->acm_ccm = NIM_ACM;
                ACM_DEBUG_PRINTF("[Way 2]Find ACM/CCM TP success, The current TP is ACM!\n");
                ACM_DEBUG_PRINTF("\n");
                return SUCCESS;
            }
            comm_sleep(2);  
        }
        return ERR_FAILED;  
    }//  if(1 == work_mode)
    
    return SUCCESS;
}
#endif


/*****************************************************************************
*  INT32 nim_c3505_autoscan_init(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
* 	autoscan initial function
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3505_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_autoscan_init(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
{
    UINT8 data = 0;
	
#ifdef CHANNEL_CHANGE_ASYNC
	UINT32 flag_ptn = 0;
#endif

#ifdef IMPLUSE_NOISE_IMPROVED
	UINT32 i = 0;
#endif

    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);

	// For some multiswitch,  it need more time to make signal steady
    comm_sleep(COMPEN_MULTISWITCH_DELAY);
	
    last_tp_freq = 0;

	params->priv->search_type = NIM_OPTR_SOFT_SEARCH;

#ifdef AUTOSCAN_DEBUG
	config_data = params->priv->tuner_config_data.qpsk_config;
	params->priv->tuner_config_data.qpsk_config |= C3505_SIGNAL_DISPLAY_LIN;
#endif

#ifdef CHANNEL_CHANGE_ASYNC
	//UINT32 flag_ptn = 0;
#ifdef __C3505_TDS__
	if(NIM_FLAG_WAIT(&flag_ptn, params->priv->flag_id, NIM_FLAG_CHN_CHG_START|NIM_FLAG_CHN_CHANGING,OSAL_TWF_ORW, 0) == OSAL_E_OK)
#endif

#ifdef __C3505_LINUX__
	flag_ptn = nim_flag_read(&params->priv->flag_lock, NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING, OSAL_TWF_ORW, 0);
	if((flag_ptn & (NIM_FLAG_CHN_CHG_START | NIM_FLAG_CHN_CHANGING)) && (flag_ptn != OSAL_INVALID_ID))
#endif
	{
		params->priv->ul_status.c3505_chanscan_stop_flag = 1;
		NIM_PRINTF("[%s %d]channel changing already, stop it first\n",__FUNCTION__,__LINE__);//autoscan force cancel and start TP scan 
		comm_sleep(10);
	}
#endif
    params->pst_auto_scan->isid_num = 0;
    params->pst_auto_scan->isid = 0;

	params->priv->tsk_status.m_lock_flag = NIM_LOCK_STUS_NORMAL;

    // For auto adaptive
#ifdef IMPLUSE_NOISE_IMPROVED	
    while(params->priv->t_param.auto_adaptive_doing)
    {
        comm_sleep(10);
        i++;
        if (i > 10)
            break;
    }
    NIM_PRINTF("Wait auto adaptive timeout = %d\n", i);
    params->priv->t_param.auto_adaptive_cnt = 0;
    params->priv->t_param.auto_adaptive_state = 0;

    for (i = 0; i < 32; i++) // For normal, set map beta to default
    {
        if (c3505_map_beta_normal_adaptive_en[i])
        {
    		nim_c3505_set_map_beta_extra(dev, i,  0, params->priv->t_param.auto_adaptive_state);     
        }
    }
    
    for (i = 0; i < 32; i++) // For short, set map beta to default
    {
        if (c3505_map_beta_short_adaptive_en[i])
        {
		    nim_c3505_set_map_beta_extra(dev, i,  1, params->priv->t_param.auto_adaptive_state);     
        }
    }
#endif	
    // end


	// reset HW FSM
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);
	//nim_c3505_ioctl(dev, NIM_DRIVER_STOP_ATUOSCAN, 0);
#ifdef __C3505_TDS__
	params->priv->ul_status.c3505_autoscan_stop_flag = 0;
#endif

#ifdef __C3505_LINUX__
	if (params->priv->ul_status.c3505_autoscan_stop_flag)
    {
        nim_send_as_msg(params->priv->nim_kumsgq , 0, 0, 0, 0, 0, 1);
        params->priv->ul_status.c3505_autoscan_stop_flag = 0;
		NIM_PRINTF("[%s %d]priv->ul_status.c3505_chanscan_stop_flag=%d,\n\
				priv->ul_status.c3505_autoscan_stop_flag=%d\n", 
				__FUNCTION__, __LINE__, params->priv->ul_status.c3505_chanscan_stop_flag,
				params->priv->ul_status.c3505_autoscan_stop_flag);
        return SUCCESS;
    }
#endif
	
	data = 0x1f; // setting for soft search function
	nim_reg_write(dev, R1B_TR_TIMEOUT_BAND, &data, 1);
    
#if (NIM_OPTR_ACM == ACM_CCM_FLAG)	
    // Set for ACM mode autoscan by paladin 2015-12-30
    //data = 0x1f;
    //nim_reg_write(dev, RB4_BEQ_CFG, &data, 1);   //BEQ CENTRAL UPDATE STEP = 1
#endif
    
	//nim_c3505_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
	nim_c3505_cr_setting(dev, NIM_OPTR_CHL_CHANGE);
	nim_c3505_tr_cr_setting(dev, NIM_OPTR_SOFT_SEARCH);
	nim_c3505_fec_set_ldpc(dev, NIM_OPTR_SOFT_SEARCH, 0x00, 0x01);
	nim_c3505_tso_on(dev);
	
#ifdef ADC2DRAM_SUPPORT   
	nim_c3505_set_adc(dev);
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
	nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[1]);
	nim_c3505_interrupt_mask_clean(dev);
	// a difference btw adc2dram and capture, in adc2dram agc1 must be set to unlock to make sure FSM at AGC1 state
	nim_reg_read(dev, R0A_AGC1_LCK_CMD, &(params->reg0a_data_bak), 1);//backup
	data = 0x55; ////0x2B; let AGC can not lock, try to modify tuner's gain
	nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);	
	nim_c3505_set_agc1(dev, 0x00, NIM_OPTR_FFT_RESULT,NIM_FRZ_AGC1_OPEN);	
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X52);	
#endif	

	last_tuner_if = 0;
	chlspec_num = 0;
	called_num = 0;	
	max_fft_energy = 0;

	channel_spectrum = (INT32 *) comm_malloc(FS_MAXNUM * 4);
	if(channel_spectrum == NULL)
	{
		AUTOSCAN_PRINTF("\n channel_spectrum--> no enough memory!\n");
		return ERR_NO_MEM;
	}

	channel_spectrum_tmp = (INT32 *)comm_malloc(FS_MAXNUM * 4);
	if(channel_spectrum_tmp == NULL)
	{
		AUTOSCAN_PRINTF("\n channel_spectrum_tmp--> no enough memory!\n");
		comm_free(channel_spectrum);
		return ERR_NO_MEM;
	}
    
    return SUCCESS;

}

/*****************************************************************************
*  INT32 nim_c3505_autoscan_get_spectrum(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
* 	autoscan get wideband spectrum by fft
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3505_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_autoscan_get_spectrum(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
{
    INT32 i = 0;
    UINT32 step_freq = 0;
    INT32 channel_freq_err = 0;
    INT32 fft_shift_num = 0;
    INT32 scan_mode = 0;
    INT32 energy_step = 0;
	 
	struct nim_c3505_private *priv = NULL;
    
    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }

	priv = (struct nim_c3505_private *)dev->priv;
	
    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);

	params->adc_sample_freq = ADC_SAMPLE_FREQ[1];

	if(IS_M3031 == params->priv->tuner_type)	
	{
		step_freq = params->adc_sample_freq/4;
	}
	else
	{
		step_freq = params->adc_sample_freq/2;
	}

    AUTOSCAN_PRINTF("sfreq = %d, efreq = %d, step_freq = %d \n", params->pst_auto_scan->sfreq, params->pst_auto_scan->efreq, step_freq);
    
	// start cap fft data and calculate the tp 
    for (params->fft_freq = params->pst_auto_scan->sfreq; params->fft_freq < params->pst_auto_scan->efreq; \
           params->fft_freq += step_freq)
    {
		comm_memset(fft_energy_1024, 0, sizeof(fft_energy_1024));
		while(1)
		{				
        #ifdef ADC2DRAM_SUPPORT
			if (GET_SPECTRUM_ONLY == priv->autoscan_control)	//get spectrum or get frequency/sysbolrate 
			{
            	nim_c3505_adc2mem_start(dev, params->fft_freq, 0, 0x8000);	// 32K byte for get spectrum
			}
			else	// Normal autoscan or get tip only
			{
				nim_c3505_adc2mem_start(dev, params->fft_freq, 0, BYPASS_BUF_SIZE_DMA); // 64K byte for wideband scan
			}
						
			call_tuner_command(dev, NIM_TUNER_GET_C3031B_FREQ_ERR, &channel_freq_err);
        #else
			nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);        
			nim_c3505_cap_start(dev, params->fft_freq, 0, fft_energy_1024);
			call_tuner_command(dev, NIM_TUNER_GET_C3031B_FREQ_ERR, &channel_freq_err);
        #endif

            // make sure fft_energy is ok
            for(i = 0; i < 1024; i++)
            {
                if(fft_energy_1024[i] > 0)
                    break;
            }
            if(i < 1024)    
                break;

			if(params->priv->ul_status.c3505_autoscan_stop_flag)
			{
		#ifdef ADC2DRAM_SUPPORT
			    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &(params->reg0a_data_bak), 1);//restore	
		#endif
				comm_free(channel_spectrum);
				comm_free(channel_spectrum_tmp);
				comm_free((int *)params->priv->ul_status.adc_data_malloc_addr);
				AUTOSCAN_PRINTF("\tleave fuction nim_C3505_autoscan\n");
				return USER_STOP;
			}
		}
		//remove value<=0 point
		for(i=0;i<1024;i++)
    	{
        	if(fft_energy_1024[i]<=0)
           		 fft_energy_1024[i]=1;
    	}
        //remove DC component parts, need before correct freq err, paladin add 20160621
        energy_step = fft_energy_1024[511+2] - fft_energy_1024[511-2];
        energy_step /=4;
        for(i=(511-2); i<(511+2); i++)
            fft_energy_1024[i+1] = fft_energy_1024[i] + energy_step;

        // correct the fft point with the freq err 
		if(IS_M3031 == params->priv->tuner_type)
		{
			fft_shift_num = (4*channel_freq_err + params->adc_sample_freq/2) / params->adc_sample_freq;
			AUTOSCAN_PRINTF("channel_freq_err = %d, FFT_shift_num = %d \n", channel_freq_err, fft_shift_num);

			if(0 == fft_shift_num) // nothing to do
				;
			else if(fft_shift_num > 0) // cut end
			{
				for (i = 1023; i >= 0; i--)
				{
					if(i<fft_shift_num)
					fft_energy_1024[i] = 0;
					else
					fft_energy_1024[i] = fft_energy_1024[i - fft_shift_num];
				}
			}
			else //cut start
			{
				fft_shift_num = -fft_shift_num;
				for (i = 0; i < 1024; i++)
				{
				    if((i + fft_shift_num) > 1023)
					    fft_energy_1024[i] = 0;
				    else
					    fft_energy_1024[i] = fft_energy_1024[i + fft_shift_num];
				}
			}
		}
		called_num++;

    #if (defined(FFT_PRINTF_DEBUG)&& defined(NIM_GET_1024FFT))
        //if(params->priv->autoscan_debug_flag & 0x02)
        
        {
            FFT_PRINTF("call_num = %d, Tuner_IF = %d \n", called_num, params->fft_freq);
            FFT_PRINTF("FFT1024 point without median filter\n");
            for(i = 0; i < 1024; i++)
                FFT_PRINTF("FFT1024-> %4d :  %d \n", i, fft_energy_1024[i]);
        }
    #endif
        
        if(NIM_SCAN_SLOW == params->priv->blscan_mode)
        {
            scan_mode = 1;
        }
        else
        {
            scan_mode = 0;
        }
        
        //nim_s3501_smoothfilter3(1024,fft_energy_1024);
        nim_s3501_median_filter(1024, fft_energy_1024, scan_mode); // Median filter for restrain single tone noise
        //nim_s3501_smoothfilter3(1024,fft_energy_1024);

#if 0
        if(IS_M3031 == params->priv->tuner_type)
        {
            nim_s3501_fft_wideband_scan_m3031(dev, params->fft_freq, params->adc_sample_freq);
		}
		else
		{
			nim_s3501_fft_wideband_scan(dev, params->fft_freq, params->adc_sample_freq);
		}
#else
        nim_s3501_fft_wideband_scan(dev, params->fft_freq, params->adc_sample_freq);
#endif

		if(params->priv->ul_status.c3505_autoscan_stop_flag)
		{
	#ifdef ADC2DRAM_SUPPORT
		    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &(params->reg0a_data_bak), 1);//restore	
	#endif
			comm_free(channel_spectrum);
			comm_free(channel_spectrum_tmp);
			comm_free((int *)params->priv->ul_status.adc_data_malloc_addr);
			AUTOSCAN_PRINTF("\tleave fuction nim_C3505_autoscan\n");
			return USER_STOP;
		}
	}

#ifdef ADC2DRAM_SUPPORT
	nim_reg_write(dev, R0A_AGC1_LCK_CMD, &(params->reg0a_data_bak), 1);//restore	
#endif
    comm_free((int *)params->priv->ul_status.adc_data_malloc_addr);

#ifdef FFT_PRINTF_DEBUG
    //if(params->priv->autoscan_debug_flag & 0x04)    
    {
        for(i = 0; i < chlspec_num; i++)
            FFT_PRINTF("FFT_WBS-->%d :      %d\n", i, channel_spectrum[i]);
    }
#endif
    
#ifdef AUTOSCAN_DEBUG
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("\tGet wideband spectrum by FFT cost time      %dms\n", params->end_t - params->start_t);
    params->temp_t = params->end_t;
#endif

    return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_c3505_autoscan_estimate_tp(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
* 	autoscan estimate tp from wideband spectrum
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3505_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_autoscan_estimate_tp(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
{
    INT32 loop = 1;
    INT32 i = 0;
    INT32 loop_index = 0;
#ifdef DEBUG_SOFT_SEARCH    
    INT32 bug_freq[6]={0};
    INT32 bug_sym[6]={0};
#endif


    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);
	AUTOSCAN_PRINTF("max_fft_energy = %d\n",max_fft_energy);
	loop = 1;
	if(max_fft_energy > 8388607)
	{
		loop = 2;
	}
	AUTOSCAN_PRINTF("************loop=%d****************\n",loop);
	if(loop == 2)
	{
		for(i = 0;i < chlspec_num;i++)
		{
			channel_spectrum_tmp[i]= channel_spectrum[i]/(8388607>>5);

			if(channel_spectrum_tmp[i] <= 0)
				channel_spectrum_tmp[i] = 1;
			else if(channel_spectrum_tmp[i] > 8388607)
				channel_spectrum_tmp[i] = 8388607;

			if (channel_spectrum[i] > 8388607)    // max 23bit = 0x7fffff
				channel_spectrum[i] = 8388607;
			else if(channel_spectrum[i] == 0) 
				channel_spectrum[i] = 1;
		}
	}

	////******find TP***
	tp_number = 0;
	comm_memset(frequency_est, 0, sizeof(frequency_est));
	comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));

	for(loop_index = 0; loop_index < loop; loop_index++)
	{
		if(loop_index == 0)
		{
			nim_s3501_search_tp(chlspec_num, channel_spectrum, params->pst_auto_scan->sfreq, \
                params->adc_sample_freq,loop_index);
			AUTOSCAN_PRINTF("Time %d : Find TP number is %d\n",loop_index,tp_number);
		}
		else if(loop_index == 1)
		{
			nim_s3501_search_tp(chlspec_num, channel_spectrum_tmp, params->pst_auto_scan->sfreq, \
                params->adc_sample_freq,loop_index);
			AUTOSCAN_PRINTF("Time %d : Find TP number is %d\n",loop_index,tp_number);
		}
		}
		comm_free(channel_spectrum);
		comm_free(channel_spectrum_tmp);

#ifdef DEBUG_SOFT_SEARCH
    if(IS_M3031 == params->priv->tuner_type)
    {
	    params->success=0;
		tp_number=200;
		bug_freq[0] = 1134752; ////TP1
		bug_sym[0] = 1875;
		bug_freq[1] = 1186544; ////TP2
		bug_sym[1] = 4218;
		bug_freq[2] = 1414544; ////TP3
		bug_sym[2] = 13031;
		bug_freq[3] = 1179824; ////TP4
		bug_sym[3] = 3093;
		bug_freq[4] = 1186400; ////TP5
		bug_sym[4] = 4312;
		bug_freq[5] = 0; ////TP6
		bug_sym[5] = 0;
	}
	else
	{
		params->success=0;
		tp_number=1000;
		bug_freq[0]=1889824;///12446/V/1537
		bug_sym[0]=2062;
		bug_freq[1]=1762000;////12321/V/1500
		bug_sym[1]=1968;
	}
	comm_memset(frequency_est, 0, sizeof(frequency_est));
	comm_memset(symbol_rate_est, 0, sizeof(symbol_rate_est));
	for(i=0; i < tp_number; i++)
	{
		if(IS_M3031 == params->priv->tuner_type)
		{
			frequency_est[i] = bug_freq[i%5];  // max_number=10
			symbol_rate_est[i] = bug_sym[i%5]; // max_number=10
		}
		else
		{
			frequency_est[i] = bug_freq[i%2];
			symbol_rate_est[i] = bug_sym[i%2];
		}
	}
#endif

	if(tp_number > 0)
    {
        params->success = NIM_SUCCESS;
		AUTOSCAN_PRINTF("\n\n---------------------------------------------- \n");
        AUTOSCAN_PRINTF("Search TP success , TP_number = %d  \n", tp_number);
        for(i = 0; i < tp_number; i++)
            AUTOSCAN_PRINTF("TP_number =   %d  Freq =  %d  Sym =   %d \n", i, frequency_est[i], symbol_rate_est[i]);            
		AUTOSCAN_PRINTF("\n");
		c3505_real_tp_number = 0;
    	comm_memset(c3505_real_freq, 0, sizeof(c3505_real_freq));
    	comm_memset(c3505_real_sym_rate, 0, sizeof(c3505_real_sym_rate));
    }
    else
    {
        params->success = NIM_FAILED;
        AUTOSCAN_PRINTF("Search TP failed\n");
    }

#ifdef AUTOSCAN_DEBUG
    params->end_t = os_get_tick_count();
    AUTOSCAN_PRINTF("Search TP cost time      %dms\n", params->end_t - params->temp_t);
    params->temp_t = params->end_t;
#endif

    return SUCCESS;
}


/*****************************************************************************
*  INT32 nim_c3505_autoscan_try_tp(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
* 	autoscan try to lock estimate tp
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3505_autoscan_params *params
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_autoscan_try_tp(struct nim_device *dev, struct nim_c3505_autoscan_params *params)
{
    UINT32 last_lock_rs = 0;
    INT32 last_lock_freq = 0;
    INT32 i = 0, j = 0;
    INT32 temp_freq = 0;
    UINT8 cur_fec = 0;
    UINT32 cur_freq = 0;
    UINT32 cur_sym = 0;
    UINT32 success_num = 0;
#ifdef AUTOSCAN_DEBUG	
    UINT32 fail_num = 0;
#endif
    INT32  ret_value = SUCCESS;
    INT32 ret = SUCCESS;
    UINT8 lock = 0;
    UINT32 unlock_cnt = 0;
    UINT8 do_next_tp = 0;
    INT32 lock_tp_idx = 0;
 	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;
	
    if((NULL == dev) || (NULL == params))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
    AUTOSCAN_PRINTF("Enter %s \n", __FUNCTION__);
    
	if (NIM_SUCCESS == params->success)
	{
		last_lock_freq = 0;
		last_lock_rs = 0;
		last_tp_freq = 0;
		final_est_freq = 0;
		final_est_sym_rate = 0;
        
		for (i = 0; i < tp_number; i++)
		{
            #ifdef AUTOSCAN_DEBUG
                params->temp_t = os_get_tick_count();
            #endif 

			AUTOSCAN_PRINTF("\n\n----> Try %d-th TP [freq,Sym]=[%d, %d]\n",i,frequency_est[i],symbol_rate_est[i]);
			if (params->priv->ul_status.c3505_autoscan_stop_flag)
			{
				AUTOSCAN_PRINTF("\tleave fuction nim_C3505_autoscan\n");
				return USER_STOP;
			}
			
    		#ifdef DEBUG_SOFT_SEARCH
    			params->priv->ul_status.m_cr_num = 1;
    			params->priv->ul_status.m_freq[0] = frequency_est[i];
    			params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
    		#else
            
    			//if(frequency_est[i]/1024 < (params->pst_auto_scan->sfreq - 5))
    			if(frequency_est[i]/1024 < 945)
                {         
    				AUTOSCAN_PRINTF("Exit_1 search TP[%d][freq,Sym]=[%d, %d] with freq too closed sfreq %d\n", i, frequency_est[i]/1024, symbol_rate_est[i], params->pst_auto_scan->sfreq);
    				continue;
                }

    			
    			//if(frequency_est[i]/1024 > (params->pst_auto_scan->efreq + 5))
    			if(frequency_est[i]/1024 > 2155)
                {         
    				AUTOSCAN_PRINTF("Exit_2 search TP[%d][freq,Sym]=[%d, %d] with freq too closed efreq %d\n", i, frequency_est[i]/1024, symbol_rate_est[i], params->pst_auto_scan->efreq);
    				continue;
                }
                
    			temp_freq = frequency_est[i] - last_lock_freq;

                if (temp_freq < 0)
                  temp_freq = -temp_freq;

                AUTOSCAN_PRINTF("Temp_freq = %d \n", temp_freq);

                if(temp_freq <= 1024)
                {         
    				AUTOSCAN_PRINTF("Exit_3 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i], temp_freq);
    				continue;
                }
                
                if ((last_lock_rs + symbol_rate_est[i]*8/10)/2 > (UINT32)temp_freq)
                {    
                  AUTOSCAN_PRINTF("Exit_6 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i],temp_freq);
                  continue;
                } 

                if (symbol_rate_est[i]/2 > temp_freq)
                {
    				AUTOSCAN_PRINTF("Exit_4 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i],temp_freq);
    				continue;
                }
                
                if (last_lock_rs/2 > (UINT32)temp_freq)
                {    
    				AUTOSCAN_PRINTF("Exit_5 search TP[%d][freq,Sym]=[%d, %d] with freq too closed %dKhz \n", i, frequency_est[i]/1024, symbol_rate_est[i],temp_freq);
    				continue;
                }                       

    			params->priv->ul_status.m_cr_num = 1;
    			params->priv->ul_status.m_freq[0] = frequency_est[i];
    			params->priv->ul_status.m_rs[0] = symbol_rate_est[i];
                
		    #endif
		
			if (SUCCESS == nim_c3505_channel_search(dev, 0))
			{
			    success_num++;
            #ifdef AUTOSCAN_DEBUG
                params->end_t = os_get_tick_count();
			    AUTOSCAN_PRINTF("Lock SUCCESS %d-th TP[freq %d, sym %d] ,Cost time     %d\n", \
                    i, final_est_freq, final_est_sym_rate, params->end_t - params->temp_t);
            #endif 

                // For low symbol rate tp lock speedup, 07/14/2017 hyman
                //data = 0x00;
                //nim_reg_write(dev, R43_CR_OFFSET_TRH+1, &data, 1); 
                // Move ppll adaptive to here for autoscan.
                nim_c3505_adapt_ppll_mode(dev);
                
				nim_c3505_reg_get_code_rate(dev, &cur_fec);
                cur_sym = final_est_sym_rate;
                last_lock_freq = final_est_freq;
                last_lock_rs = cur_sym;
                final_est_freq = (final_est_freq + 512) / 1024;
				cur_freq = final_est_freq;                

				priv->plsn.search_plsn_force_stop = 1;//stop plsn search
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				if (priv->plsn.plsn_try)
				{
					priv->plsn.plsn_report = priv->plsn.plsn_try_val;
					params->pst_auto_scan->plsn_db = priv->plsn.plsn_try_val;
				}
				else
				{
					priv->plsn.plsn_report = nim_c3505_get_plsn(dev);
					params->pst_auto_scan->plsn_db = priv->plsn.plsn_report;
				}
				AUTOSCAN_PRINTF("[%s %d]params->pst_auto_scan->plsn_db=%d!\n", __FUNCTION__, __LINE__, params->pst_auto_scan->plsn_db);//robin
				if (GET_TP_ONLY == priv->autoscan_control)//get tp only
				{
					continue;
				}
				
				if ((cur_freq >= params->pst_auto_scan->sfreq) && (cur_freq <= params->pst_auto_scan->efreq))
				{
					if (params->pst_auto_scan->unicable) //cur_freq -> [950, 2150]
					{
						AUTOSCAN_PRINTF("\tUnicable FFT_freq: %d, cur_freq: %d, Fub: %d\n", params->fft_freq, cur_freq, params->pst_auto_scan->fub);
					#ifdef __C3505_TDS__
						ret_value = params->pst_auto_scan->callback(1, 0, params->fft_freq + cur_freq - params->pst_auto_scan->fub, cur_sym, cur_fec);
					#endif

					#ifdef __C3505_LINUX__
						ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, params->fft_freq + cur_freq - params->pst_auto_scan->fub, cur_sym, cur_fec, 0);
					#endif
					}
					else
					{
						if((cur_freq > (last_tp_freq - 2)) && (cur_freq < (last_tp_freq + 2)))
						{
						#ifdef __C3505_TDS__
							ret_value = params->pst_auto_scan->callback(0, 0, frequency_est[i]/1024, 0, 0);						
						#endif

						#ifdef __C3505_LINUX__
							ret_value = nim_callback(params->pst_auto_scan, priv, 0, 0, frequency_est[i]/1024, 0, 0, last_tp_freq);
						#endif
							AUTOSCAN_PRINTF("\tRescan a tp: FFT_freq: %d, last_tp_freq: %d\n", frequency_est[i]/1024, last_tp_freq);
						}
						else
						{	
                            unlock_cnt = 0;
                            do_next_tp = 0;

                            while(1)
                            {    
                                // If open this macro, driver may be report "fail TP" to upper which can not locked. paladin 20160614
                                //#ifdef REPORT_FAIL_TP
                               	#if 1
                                    lock = 1;
                                #else
                                    nim_c3505_get_lock(dev, &lock);	// Double check lock with bch
                                #endif
                                
                                if(1 == lock)
                                {
                                    lock_tp_idx ++;
                                    // Find the ACM/CCM TP for debug
                                    #ifdef FIND_ACM_TP
                                        nim_c3505_autoscan_find_acm_tp(dev, lock_tp_idx, cur_freq, cur_sym);
                                        nim_c3505_printf_tp_info(dev);
                                    #endif

                                    params->priv->bb_header_info->stream_mode = 3;
                                    params->priv->bb_header_info->stream_type = 1;
                                    params->priv->bb_header_info->acm_ccm = 1;

                                    // Get work mode and config hardware, then check if the TP is multi stream or not, don't get isid yet here
                                    ret = nim_c3505_get_isid_after_lock(dev, 0, params->priv->isid);

                                    if(1 == params->pst_auto_scan->scan_type)
                                    {
                                        // Now begin get isid for multi stream autoscan, DVB-S2 && multi stream && get bbheader success
                                        if(SUCCESS == ret)
                                        {
                                            // While get isid error,  jump to next tp
                                            if(SUCCESS != nim_c3505_get_isid(dev, params->priv->isid))
                                            {
                                                ACM_DEBUG_PRINTF("[%s %d]get isid error!\n", __FUNCTION__, __LINE__);
                                                do_next_tp = 1;
                                                break ;   
                                            }
                                            
                                            params->priv->isid->isid_bypass = 0;
                                            params->priv->isid->isid_mode = 1;

                                            for(j = 0; j < params->priv->isid->isid_num; j++)
                                            {
                                                params->priv->isid->isid_write = params->priv->isid->isid_read[j];

                                                if(SUCCESS == nim_c3505_set_isid(dev, params->priv->isid))
                                                {
                                                    #ifdef __C3505_TDS__
                                                        ret_value = params->pst_auto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec);  
                                                    #endif
													
													#ifdef __C3505_LINUX__
                                                        ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
                                                    #endif
                                                }
                                                // While set isid error,  break current "for loop" and  jump to next tp
                                                else
                                                {
                                                    ACM_DEBUG_PRINTF("[%s %d]set isid error!\n", __FUNCTION__, __LINE__);
                                                    do_next_tp = 1;
                                                    j = params->priv->isid->isid_num;
                                                    break ;  
                                                }
                                            }
                                        }
                                        // DVB-S || single stream || get bbheader fail
                                        else    
                                        {
                                            if (ERR_CRC_HEADER == ret)
                                            {
                                                ACM_DEBUG_PRINTF("[%s %d]BBHeader check fail in autosacn try tp, do next tp!\n", __FUNCTION__, __LINE__);
                                                do_next_tp = 1;
                                                break ;   
                                            }
                                            else
                                            {
                                                #ifdef __C3505_TDS__
                                                    ret_value = params->pst_auto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec);  
                                                #endif
												#ifdef __C3505_LINUX__
                                                    ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
                                                #endif
                                            }
                                        }    
                                    }
                                    else
                                    {
                                        #ifdef __C3505_TDS__
                                            ret_value = params->pst_auto_scan->callback(1, 0, cur_freq, cur_sym, cur_fec);  
                                        #endif
										#ifdef __C3505_LINUX__
                                            ret_value = nim_callback(params->pst_auto_scan, priv, 1, 0, cur_freq, cur_sym, cur_fec, 0);
                                        #endif
                                    }
								
                                    break;
                                }
                                else
                                {
                                    if(unlock_cnt >= 10)//18-2-2017 By Paladin
                                    {
                                        do_next_tp = 1;
                                        //ret = pst_auto_scan->callback(0, 0, frequency_est[i]/1024, 0, 0);             
                                        AUTOSCAN_PRINTF(" Can not lock TP para fre: %d, rs: %d\n", cur_freq, cur_sym);
                                        break;  
                                    }
                                    else
                                    {
                                        unlock_cnt ++;
                                        comm_sleep(5);
                                    }
                                }   
                            }

                            if(do_next_tp)
                                continue ;   

							AUTOSCAN_PRINTF(" Add TP[TS, single stream] para fre: %d, rs: %d\n", cur_freq, cur_sym);
                            AUTOSCAN_PRINTF("Find Program %d-th TP[freq%d, sym %d] ---> SUCCESS! ,Cost time        %d\n", i, final_est_freq, final_est_sym_rate, os_get_tick_count() - params->end_t);
                            
							last_tp_freq = cur_freq;
						}
					}
				}
				else
				{
					AUTOSCAN_PRINTF("tleave out of the frequence range %d-th TP[freq%d, sym %d] ,Cost time     %d\n",i,final_est_freq,final_est_sym_rate,os_get_tick_count() - params->end_t);
                    
					if (params->priv->ul_status.c3505_autoscan_stop_flag)
					{
						AUTOSCAN_PRINTF("\tleave fuction nim_C3505_autoscan\n");
						return USER_STOP;
					}
					else
					{
						continue ;
					}
				}
			}
			else
			{ 
			    #ifdef AUTOSCAN_DEBUG
					fail_num++;
                    params->end_t = os_get_tick_count();
                    AUTOSCAN_PRINTF("Lock Faild %d-th TP[freq %d, sym %d] ,Costtime        %d\n",i,(frequency_est[i] + 512)/1024,symbol_rate_est[i],params->end_t - params->temp_t);
                #endif 
				
				#ifdef __C3505_TDS__
					ret_value = params->pst_auto_scan->callback(0, 0, frequency_est[i]/1024, 0, 0);				
                #endif
				#ifdef __C3505_LINUX__
					ret_value = nim_callback(params->pst_auto_scan, priv, 0, 0, frequency_est[i]/1024, 0, 0, 0);
				#endif
				AUTOSCAN_PRINTF("ret2 value is: %d \n", ret_value);

                #ifdef AUTO_SCAN_SAVE_TIME
                if ((0 != final_est_freq) && ((0 != final_est_sym_rate)))
                {
                    last_lock_freq = final_est_freq;
                    last_lock_rs = final_est_sym_rate;
                }
                #endif
                
			}
			
			if (ret_value == 2)
			{
				return ERR_FAILED;
			}
			else if (ret_value == 1)
			{
				goto nim_as_break;
			}
			

		}
	}

#ifdef AUTOSCAN_DEBUG
	params->priv->tuner_config_data.qpsk_config = config_data;
#endif

#ifdef AUTOSCAN_DEBUG
    params->end_t=os_get_tick_count();
    AUTOSCAN_PRINTF("\t autoscan cost time   %d:%d  ,est_tp_num = %d, soft_search_times = %d, success_num = %d, fail_num = %d, re_search_num = %d \n", \
        (((params->end_t - params->start_t)+500)/1000/60), \
        (((params->end_t - params->start_t)+500)/1000 -(((params->end_t - params->start_t)+500)/1000/60)*60), \
        tp_number, search_times, success_num, fail_num, re_search_num);        
#endif    
    
    NIM_PRINTF("\tFinish autoscan\n");
    return SUCCESS;
	
nim_as_break:
	#ifdef __C3505_TDS__
		params->pst_auto_scan->callback(2, 0, 0, 0, 0);  /* Tell callback search finished */
    #endif
    #ifdef __C3505_LINUX__
		nim_callback(params->pst_auto_scan, priv, 2, 0, 0, 0, 0, 0);  /* Tell callback search finished */
	#endif
	
    return ret_value;
}


/*****************************************************************************
*  INT32 nim_C3505_autoscan(struct nim_device *dev, struct NIM_AUTO_SCAN *pst_auto_scan)
* 	autoscan function
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct NIM_AUTO_SCAN *pst_auto_scan
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_autoscan(struct nim_device *dev, struct NIM_AUTO_SCAN *pst_auto_scan)
{
    INT32 ret = SUCCESS;
	UINT8 data = 0;
	
    struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

    struct nim_c3505_autoscan_params params;
    
    if((NULL == dev) || (NULL == pst_auto_scan))
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n", __FUNCTION__);
        return ERR_NO_DEV;
    }

	if (pst_auto_scan->sfreq < AS_FREQ_MIN || pst_auto_scan->efreq > AS_FREQ_MAX || \
		pst_auto_scan->sfreq > pst_auto_scan->efreq)
	{
	    ERR_PRINTF("Exit Auto scan param ERROR in %s \n",__FUNCTION__);
	    ERR_PRINTF("Expect sfreq = %d real = %d , efreq = %d real = %d \n", \
            AS_FREQ_MIN, pst_auto_scan->sfreq, AS_FREQ_MAX, pst_auto_scan->efreq);

        return ERR_PARA;
	}
		
    AUTOSCAN_PRINTF("Enter %s, sfreq = %d, efreq = %d, band = %d\n", __FUNCTION__, pst_auto_scan->sfreq, pst_auto_scan->efreq, pst_auto_scan->freq_band);


	priv->plsn.super_scan = pst_auto_scan->super_scan;
	//priv->plsn.super_scan = 1;
	priv->plsn.plsn_now = 0;
	
	AUTOSCAN_PRINTF("priv->plsn.super_scan=%d\n", priv->plsn.super_scan);
	
    comm_memset(&params, 0, sizeof(params));

    // debug for auto scan time cost
#ifdef AUTOSCAN_DEBUG
    //UINT32 start_t,end_t,temp_t;
    params.start_t = os_get_tick_count();
#endif
 
    params.priv = priv;
    params.pst_auto_scan = pst_auto_scan;
    params.freq_band = pst_auto_scan->freq_band;
    
    ret = nim_c3505_autoscan_init(dev, &params);
    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3505_autoscan_init error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = nim_c3505_autoscan_get_spectrum(dev, &params);
    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3505_autoscan_get_spectrum error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

	if (GET_SPECTRUM_ONLY == priv->autoscan_control) //only get spectrum, below code need not execute
	{
		return SUCCESS;
	}
	
    ret = nim_c3505_autoscan_estimate_tp(dev, &params);
    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3505_autoscan_estimate_tp error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    // set iq swap freq_band
    if (1 == params.freq_band)  // C band used iq swap first
    {
        AUTOSCAN_PRINTF("\n\n\nForce set iq swap\n\n\n");
    	nim_reg_read(dev, R66_TR_SEARCH, &data, 1);
    	data |= 0x80;
    	nim_reg_write(dev, R66_TR_SEARCH, &data, 1);        
    }
    
    ret = nim_c3505_autoscan_try_tp(dev, &params);

    if (1 == params.freq_band)  // Recover
    {
    	nim_reg_read(dev, R66_TR_SEARCH, &data, 1);
    	data &= 0x7f;
    	nim_reg_write(dev, R66_TR_SEARCH, &data, 1);   
    }

    if(USER_STOP == ret)
	{
		return SUCCESS;
	}
    else if(SUCCESS != ret)
    {
    	AUTOSCAN_PRINTF("[%s %d]nim_c3505_autoscan_try_tp error! ret=%d\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    
    return ret;
}


/*****************************************************************************
 * INT32 nim_C3505_adc2mem_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, UINT32 *cap_buffer)
 * capture dram_len bytes RX_ADC data to DRAM, and calculate FFT result for spectrum for autoscan.
 *
 *
 * Arguments:
 *	Parameter1: struct nim_device *dev
 *	Parameter2: UINT32 startFreq
 *	Parameter3: UINT32 *cap_buffer
 *		 Not used, because the DRAM buffer is allocated and free in this function 
 *	Parameter4: UINT32 sym
 *		 always '0', set tuner_lpf_bandwidth = tuner's top bandwidth for autoscan
 *	Parameter5: UINT32 dram_len
 *
 *
 * Return Value: INT32
 *****************************************************************************/
INT32 nim_c3505_adc2mem_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, UINT32 dram_len)
{
	INT32	 channel_freq_err = 0;
	struct nim_c3505_private *dev_priv = NULL;
	UINT8	*dram_base_t = NULL;
	dev_priv = (struct nim_c3505_private *)dev->priv;

	dram_base_t = (UINT8 *) MALLOC(dram_len);
	if(NULL == dram_base_t)
	{
	NIM_PRINTF("C3505 adc2mem memory allocation error!\n"); 
	return ERR_FAILUE;
	}

	channel_freq_err = 0;
	call_tuner_command(dev, NIM_TUNER_SET_C3031B_FREQ_ERR, &channel_freq_err);
	if (SUCCESS == (dev_priv->nim_tuner_control(dev_priv->tuner_index, startFreq, sym)))
	{
		NIM_PRINTF("nim_Tuner_Control OK\n");
	}
	else
	{
		NIM_PRINTF("nim_Tuner_Control failed\n");
	}

	// check whether there is a need to delay as the agc1 freeze function is enable	
	//if(SUCCESS == call_tuner_command(dev, NIM_TUNER_C3031B_ID, &channel_freq_err))

	if (GET_SPECTRUM_ONLY == dev_priv->autoscan_control) //get spectrum or get frequency/sysbolrate
	{
		comm_delay(20000);
	}
	else												//autoscan or get tp
	{
		if(dev_priv->tuner_type == IS_M3031)
		{
			comm_delay(40000);//wait 20ms for agc1 stable	  
		}
		else
		{
			comm_delay(10000);	//wait 5ms for agc1 stable	   
		}	
	}

	nim_c3505_adc2mem_calculate_energy(dev,dram_base_t,dram_len);

	comm_free(dram_base_t);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_C3505_cap_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, UINT32 *cap_buffer)
*  start to capture RX-ADC data and calculate the spectrum energy
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 startFreq
*		input IF frequence that send to tuner
*  Parameter3: UINT32 sym
*		symbol rate is set to zero in autoscan mode
*  Parameter3:UINT32 *cap_buffer
*		1024 point fft result buffer	
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cap_start(struct nim_device *dev, UINT32 startFreq, UINT32 sym, INT32 *cap_buffer)
{
	UINT8   low_sym = 0;
	struct nim_c3505_private *dev_priv;
	dev_priv = (struct nim_c3505_private *)dev->priv;

	NIM_PRINTF("Enter nim_C3505_cap_start : Tuner_IF=%d\n",startFreq);

	nim_c3505_set_agc1(dev,low_sym,NIM_OPTR_CHL_CHANGE,NIM_FRZ_AGC1_OPEN);

	if (SUCCESS == (dev_priv->nim_tuner_control(dev_priv->tuner_index,startFreq, sym)))
	{
		NIM_PRINTF("nim_tuner_control OK\n");
	}
	else
    {
		NIM_PRINTF("nim_tuner_control failed\n");
    }
	
	nim_c3505_set_adc(dev);
    nim_c3505_cap_calculate_energy(dev);

	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_C3505_channel_search(struct nim_device *dev, UINT32 CRNum );
* Description: C3505 channel blind searching operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_channel_search(struct nim_device *dev, UINT32 cr_num)
{
    struct nim_c3505_private *dev_priv;
    struct nim_c3505_tp_scan_para *tp_scan_param;

	AUTOSCAN_PRINTF("[%s %d]Enter\n", __FUNCTION__, __LINE__);
    if(NULL == dev->priv)
    {
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
        return ERR_NO_DEV;
    }
    
    dev_priv = (struct nim_c3505_private *)dev->priv;

    tp_scan_param = (struct nim_c3505_tp_scan_para *) comm_malloc(sizeof(struct nim_c3505_tp_scan_para));
    
	if (NULL == tp_scan_param)
	{
		ERR_PRINTF("Alloc tp_scan_param memory error!/n");
		return ERR_NO_MEM;
	}
    
    comm_memset((int *)tp_scan_param, 0, sizeof(struct nim_c3505_tp_scan_para));

    AUTOSCAN_PRINTF("ENTER %s: TP -> %d. Freq= %d, Rs= %d\n",__FUNCTION__, \
        cr_num, dev_priv->ul_status.m_freq[cr_num], dev_priv->ul_status.m_rs[cr_num]);

    // get tp scan param
	tp_scan_param->est_rs = dev_priv->ul_status.m_rs[cr_num];
	tp_scan_param->est_freq = dev_priv->ul_status.m_freq[cr_num];

    /*
    tp_scan_param->delfreq = tp_scan_param->est_freq % 1024;
    tp_scan_param->est_freq = tp_scan_param->est_freq >> 10;
    
    if(tp_scan_param->delfreq > 512)
    {
        tp_scan_param->delfreq -= 1024;
        tp_scan_param->est_freq += 1;
    }
    */

    nim_c3505_cfg_tuner_get_freq_err(dev, tp_scan_param);    
    

    // correct m3031 tuner freq error 

    /*
	if(IS_M3031 == dev_priv->tuner_type)
	{	
		if(tp_scan_param->freq_err >= 128)
			tp_scan_param->est_freq += (tp_scan_param->freq_err + 128)/256 ;
		else if(tp_scan_param->freq_err <= -128)
			tp_scan_param->est_freq += (tp_scan_param->freq_err - 128)/256 ;
	}
	*/

    // record currect freq for soft search
    dev_priv->ul_status.m_cur_freq = tp_scan_param->est_freq;
    
    // start soft search
    if (SUCCESS == nim_c3505_soft_search(dev, tp_scan_param))
    {
        comm_free(tp_scan_param);
        return SUCCESS;
    }
    else if(((dev_priv->ul_status.m_freq[cr_num] <= 1846) && (dev_priv->ul_status.m_freq[cr_num] >= 1845)) && \
         ((tp_scan_param->est_rs <= 3000) && (tp_scan_param->est_rs >= 1500))) //Yatai5:12446/V/1537
    {
        re_search_num++;
        AUTOSCAN_PRINTF("*************Try_again*************\n");
        if (SUCCESS == nim_c3505_soft_search(dev, tp_scan_param))
        {            
            comm_free(tp_scan_param);
            return SUCCESS;
        }
    }
	else if((IS_M3031 == dev_priv->tuner_type) && \
			(GET_TP_ONLY == dev_priv->autoscan_control))
	{
		if (SUCCESS == nim_c3505_soft_search(dev, tp_scan_param))
        {
        	comm_free(tp_scan_param);
            return SUCCESS;
        }
	}

    comm_free(tp_scan_param);
    return ERR_FAILUE;
}
                                                                                                                  
  
                                                                                                      
/*****************************************************************************                                    
* INT32 nim_C3505_soft_search_init( struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param)         
* Description: C3505 soft-searching initial                                                                       
*                                                                                                                 
* Arguments:                                                                                                      
*  Parameter1: struct nim_device *dev                                                                             
*  Parameter2: struct nim_c3505_tp_scan_para *tp_scan_paraml                                                      
*                                                                                                                 
* Return Value: INT32                                                                                             
*****************************************************************************/                                                                                                                                               
INT32 nim_c3505_soft_search_init( struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param)           
{                                                                                                                 
    UINT8   data; 
    struct nim_c3505_private *priv = NULL;                                                                                                               
    if((NULL == dev->priv) || (NULL == tp_scan_param))                                                            
    {                                                                                                             
        ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);                                                 
        return ERR_NO_DEV;                                                                                        
    }                                                                                                             
                                                                                                                  
    priv = (struct nim_c3505_private *) dev->priv;                                      
                                                                                                                  
	if(tp_scan_param->est_rs < 44000)                                                    
    {
        nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[1]);     
    }
    else      
    {
        if (CHIP_ID_3503D == priv->ul_status.m_c3505_type)
        {
            nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[2]); 
        }
        else
        {
            nim_c3505_set_dsp_clk (dev, ADC_SAMPLE_FREQ[0]); 
        } 
    }

    final_est_freq = 0;
    final_est_sym_rate = 0;
                                                                           
	nim_c3505_interrupt_mask_clean(dev);                                                                          
   
	nim_c3505_set_hw_timeout(dev, 0x1ff); 

#if (NIM_OPTR_ACM == ACM_CCM_FLAG)	
	//data = 0x04;		 
	//nim_reg_write(dev, R97_S2_FEC_THR, &data, 1);//set S2_FEC_LOCK_THR[7:0] = 1
	
	//data = 0x20;
	//nim_reg_write(dev, R98_S2_FEC_FAIL_THR, &data, 1);//set S2_FEC_FAIL_THR[7:0] = ff 		 
#endif

	if(0 == tp_scan_param->search_status)
	{
		nim_c3505_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_SOFT_SEARCH, NIM_FRZ_AGC1_OPEN);////CR07  AGC setting
#ifdef ASKEY_PATCH3
        data = 0xb1;
        nim_reg_write(dev, R18_TR_CTRL + 2, &data, 1);

        data = 0x24;
        nim_reg_write(dev, R16_AGC2_REFVAL, &data, 1);  
#endif  
    }
	else
	{
		nim_c3505_set_agc1(dev, tp_scan_param->low_sym, NIM_OPTR_CHL_CHANGE, NIM_FRZ_AGC1_OPEN);////CR07  AGC setting   
#ifdef ASKEY_PATCH3
        data = 0xf2;
        nim_reg_write(dev, R18_TR_CTRL + 2, &data, 1);

        data = 0x18;
        nim_reg_write(dev, R16_AGC2_REFVAL, &data, 1);  
#endif
    }

	nim_c3505_open_ppll(dev);
		
#ifdef ASKEY_PATCH1	
	if ((((tp_scan_param->tuner_freq > 1447) && (tp_scan_param->tuner_freq < 1453)) || ((tp_scan_param->tuner_freq > 2047) && (tp_scan_param->tuner_freq < 2053))) &&\
		((tp_scan_param->est_rs > 14950) && (tp_scan_param->est_rs < 15050)))
	{
		nim_c3505_set_cr_prs_table(dev, 15, 0, priv->t_param.auto_adaptive_state);
		nim_c3505_askey_patch_in_set_tp(dev, 1, tp_scan_param);
	}
	else
	{
		nim_c3505_askey_patch_in_set_tp(dev, 0, tp_scan_param);
	}
#endif

	nim_c3505_set_rs(dev, tp_scan_param->est_rs);                                                                 
	nim_c3505_set_freq_offset(dev, tp_scan_param->delfreq);                                                       
	nim_c3505_set_fc_search_range(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs);                              
	nim_c3505_set_rs_search_range(dev, NIM_OPTR_SOFT_SEARCH, tp_scan_param->est_rs);                              
	nim_c3505_tso_soft_cbr_off(dev);  

     // FSM state monitor for PL Lock find
    data = 0x87;
    nim_reg_write(dev, R6F_FSM_STATE, (UINT8 *)&data, 1);	
    
	// paladin.ye 2015-11-12    
	// In ACM mode do not insertion of dummy TS packet  edit for vcr mosaic 
#if (NIM_OPTR_CCM == ACM_CCM_FLAG)        
	nim_c3505_tso_dummy_on(dev);  
#endif

	tp_scan_param->change_work_mode_flag = 0;

	if(0 == tp_scan_param->search_status)   
	{                                                                                                     
		if(tp_scan_param->est_rs < 2500)                                                                          
			tp_scan_param->tr_lock_thr = 20;                                                                      
		else                                                                                                      
			tp_scan_param->tr_lock_thr = 12 ;                                                                     
		                                                                        
		if(tp_scan_param->est_rs <= 3000)                                                                         
			tp_scan_param->cr_lock_thr = 60;                                                                      
		else if(tp_scan_param->est_rs < 10000)                                                                    
			tp_scan_param->cr_lock_thr = 40 ;                                                                      
		else                                                                                                      
			tp_scan_param->cr_lock_thr = 30;                                                                      

		if(tp_scan_param->est_rs < 4000)                                                                          
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 50;                                         
		else                                                                                                      
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 30; 
	}
	else
	{
		if(tp_scan_param->est_rs < 2500)                                                                          
			tp_scan_param->tr_lock_thr = 16;                                                                      
		else                                                                                                      
			tp_scan_param->tr_lock_thr = 8;     

		if (tp_scan_param->est_rs <= 2000)                                                                          
			tp_scan_param->cr_lock_thr = 70;                                                                      
		else if (tp_scan_param->est_rs <= 3000)                                                                                                          
			tp_scan_param->cr_lock_thr = 60; 
		else if (tp_scan_param->est_rs <= 10000)                                                                                                          
			tp_scan_param->cr_lock_thr = 40 ; 
		else
			tp_scan_param->cr_lock_thr = 30;

		if(tp_scan_param->est_rs < 4000)                                                                          
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 50;                                         
		else                                                                                                      
			tp_scan_param->fs_lock_thr = tp_scan_param->cr_lock_thr + 30 ;
    }

    // Max 400k symbol wait for pl find, the margin must be > 50% for noise
    tp_scan_param->pl_lock_thr = (600*1024)/tp_scan_param->est_rs;
    tp_scan_param->pl_lock_thr = (tp_scan_param->pl_lock_thr)/25;
    if(tp_scan_param->pl_lock_thr < 10)
        tp_scan_param->pl_lock_thr = 10;  
	
	return SUCCESS;                                                                                               
}                                                                                                                 


/*****************************************************************************
* INT32 nim_C3505_soft_search(struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param)
* Description: C3505 soft-searching TP
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: struct nim_c3505_tp_scan_para *tp_scan_paraml
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_soft_search(struct nim_device *dev, struct nim_c3505_tp_scan_para *tp_scan_param)
{
	UINT8 data = 0;
	UINT8 lock_status = 0;
	UINT8 intindex = 0;
    UINT8 fsm_state = 0;
	UINT8 intdata = 0;
	UINT8 lock_monitor = 0;
	UINT8 modcod = 0;
	UINT32 tempFreq = 0;
	UINT32 Rs, rs_rev=0;
	UINT8 code_rate, map_type, work_mode;//ver_data,
	UINT32 timeout;
	INT32 tr_lock_num;
	INT32 cr_lock_num;
	UINT32 tr_lock_flag = 0;
	INT32 last_freq, last_rs, del_freq, del_rs; // from UINT32 to INT32 by russell
	UINT32 s2_lock_cnt = 0;

	UINT8 try_index = 0;
	plsn_state state = PLSN_UNSUPPORT;

	UINT32 search_start_single = 0; //record timeout for one time
	UINT32 search_single_timeout = 0;
	UINT32 search_start_total = 0; //record timeout for total
	UINT32 search_total_timeout = 0;
	
	UINT8 err_flag = 0; 

	UINT8 pl_find_flag = 0;
	UINT32 stop_start = 0;//used for exit while loop

#ifdef FIND_VALUE_FIRST
	UINT8 use_default_flag = 1;
#endif

#ifdef AUTOSCAN_SPEEDUP
		UINT8 tr_loss_cnt = 0;
#endif


	AUTOSCAN_PRINTF("[%s %d]Enter\n", __FUNCTION__, __LINE__);
	
	if((NULL == dev->priv) || (NULL == tp_scan_param))
	{
		ERR_PRINTF("Exit with ERR_NO_DEV in %s\n",__FUNCTION__);
		return ERR_NO_DEV;
	}

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	AUTOSCAN_PRINTF("In %s Try TP : Freq = %d, RS = %d, delfc = %d, freq_err = %d\n",\
	__FUNCTION__, tp_scan_param->est_freq, tp_scan_param->est_rs, tp_scan_param->delfreq, tp_scan_param->freq_err);

	if (priv->plsn.super_scan)
	{
		priv->plsn.plsn_try = 0;
		priv->plsn.plsn_now = 0;
		nim_c3505_set_plsn(dev);
	}
	
	//reset first
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
	comm_delay(500);

	// initial hardware and set/get some parameter
	tp_scan_param->search_status = 0;
	nim_c3505_soft_search_init(dev, tp_scan_param);
	nim_c3505_set_work_mode(dev, 0x03);

	// start search
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

	//force stop
	if (priv->plsn.super_scan)
	{
		stop_start = osal_get_tick();
		priv->plsn.search_plsn_force_stop = 1;	
		PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
		while (1==priv->plsn.start_search)
		{			
			comm_sleep(2);

			if (osal_get_tick() - stop_start > 6000)
			{
				PLSN_PRINTF("[%s %d]force stop timeout = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
				break;
			}
		}
		PLSN_PRINTF("[%s %d]stop cost=%d ms\n", __FUNCTION__, __LINE__, osal_get_tick()-stop_start);
		try_index = 0;
		priv->plsn.search_plsn_force_stop = 0;
		PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
		priv->plsn.start_search = 1;//added by robin, start to search plsn
		priv->plsn.auto_scan_start = 1;
		PLSN_PRINTF("priv->plsn.auto_scan_start=%d\n", priv->plsn.auto_scan_start);
	}
	search_start_single = osal_get_tick();
	search_start_total = osal_get_tick();
	
	timeout = 0;
	lock_monitor = 0;
	tr_lock_num = 0;
	cr_lock_num = 0;
	last_freq = 0;
	last_rs = 0;

	while (1)
	{
		if ((tp_scan_param->fs_lock_thr + 20) < timeout)
		{	
			AUTOSCAN_PRINTF("[%s %d]Timeout: %d\n", __FUNCTION__, __LINE__, timeout);
			err_flag = 1;
			goto ERR_HANDLE;
		}

		if (priv->ul_status.c3505_autoscan_stop_flag)
		{
		    priv->plsn.search_plsn_force_stop = 1;
			AUTOSCAN_PRINTF("\tleave fuction nim_C3505_soft_search\n");
			return USER_STOP;
		}
		
	#ifdef FIND_VALUE_FIRST//when use default value can't lock, and have find new plsn, use this value
		if (use_default_flag)
		{
			state = nim_c3505_try_plsn(dev, &try_index);
			if (PLSN_FIND == state)
			{
				PLSN_PRINTF("[%s %d]try_index=%d\n", __FUNCTION__, __LINE__, try_index);
				PLSN_PRINTF("[%s %d]reinit time!\n", __FUNCTION__, __LINE__);
				use_default_flag = 0;
				timeout = 0;//reinit the start time
				err_flag = 0;
				lock_monitor = 0;
				tr_lock_flag = 0;
				tr_lock_num = 0;
				cr_lock_num = 0;					
				nim_c3505_interrupt_clear(dev);	
			}
		}
	#endif
		
		timeout ++ ;
		comm_sleep(25);

		// get demod lock status
		intindex = 0;
		nim_reg_read(dev, R04_STATUS, &intindex, 1);
		if (intindex & 0x40)
			s2_lock_cnt++;
		else
			s2_lock_cnt = 0;


         // PL find, Try DVB-S2 use default timeout
         nim_reg_read(dev, R6F_FSM_STATE, &work_mode, 1);
              
         if((0x40 == (work_mode&0x40)) && (pl_find_flag == 0) && (tr_lock_flag))	 
         {
             pl_find_flag = 1;
             AUTOSCAN_PRINTF("PL Find, timeout %d\n", timeout);
         }

#if 1//NIM_OPTR_ACM == ACM_CCM_FLAG    // All use same way to change search mode
	  	// Can't find pl
	   if ((0 == pl_find_flag) && (timeout > tp_scan_param->pl_lock_thr) && (tp_scan_param->change_work_mode_flag == 0) && tr_lock_flag)
	   {		 
		 	// Set force DVB-S mode timeout
		   	tp_scan_param->change_work_mode_flag = 1;
            nim_reg_read(dev, R6F_FSM_STATE, &fsm_state, 1);
		 	// Force DVB-S mode
			nim_c3505_set_work_mode(dev, 0x00);

		   	AUTOSCAN_PRINTF("Now change to DVB-S mode TP[freq %d , sym %d] timeout %d, lock_status = 0x%x, fsm_state = 0x%x\n",  \
			 tp_scan_param->est_freq, tp_scan_param->est_rs, timeout, intindex, fsm_state);

            if ((fsm_state&0x3f) < 8) 
            {
		 	    // Set HW
		  	    nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);
                AUTOSCAN_PRINTF("Reset FSM \n");
                // Start search
                nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);   
            }
   
		   	lock_monitor = 0;
		   	cr_lock_num = 0;
		   	timeout = 0;
	   }
#endif

		//FS_LOCK or CR_lock
		if(intindex & 0x18)
		{
			nim_c3505_get_symbol_rate(dev, &Rs);
			tempFreq = tp_scan_param->est_freq;
			nim_c3505_get_freq(dev, &tempFreq);
			nim_c3505_reg_get_work_mode(dev, &work_mode);
			del_freq = tempFreq - last_freq;
			if (del_freq < 0)
				del_freq = -del_freq;
			
			del_rs = Rs - last_rs;
			if (del_rs < 0)
				del_rs = -del_rs;
			
			//if(del_freq <= 2048)
			 //if(del_freq <= 2048)
	        if ((del_freq <= 100) && (intindex & 0x08))
	        {   
	            lock_monitor = lock_monitor | intindex;
	            cr_lock_num++;
                final_est_freq = tempFreq;
	        }				
			else
			{
				cr_lock_num = 0;
				last_freq = tempFreq;
				last_rs = Rs;
			}
			AUTOSCAN_PRINTF("       current lock Rs is %d,  Freq is %d at time %d\n",Rs, tempFreq,timeout);

			del_rs = rs_rev-Rs;
			if(del_rs < 0)
				del_rs = -del_rs;

			//if((Rs >= 1000)&&(del_rs >= 100))
			if((Rs >= (RS_LIMITED - 1))&&(del_rs >= 100))
				rs_rev=Rs;
			
			Rs=rs_rev;

			nim_reg_read(dev, R04_STATUS, &data, 1);
			if ((data & 0x3f) == 0x3f)
			{
				if (1 == work_mode)
				{
					nim_reg_read(dev, RAC_S2_FEC_RPT, &data, 1);
					if(0x7f != data)
					{
						lock_status = 1;
					}
					else
					{
						lock_status = 0;
					}
				}
				else if (0 == work_mode)
				{
					lock_status = 1;
				}
				else
				{
					lock_status = 0;
				}
			}

			//nim_reg_read(dev, R04_STATUS, &lock_status, 1);
			//if (((0 == work_mode) || (1 == work_mode) || (s2_lock_cnt > 2)) && (0x3f == (0x3f&data)))
			if (1 == lock_status)
			{
				NIM_PRINTF("    path2:lock chanel \n");
				NIM_PRINTF("            Freq is %d\n", tempFreq);
				priv->ul_status.m_cur_freq = tempFreq ;
				NIM_PRINTF("            Rs is %d\n", Rs);
				nim_c3505_reg_get_code_rate(dev, &code_rate);
				NIM_PRINTF("            code_rate is %d\n", code_rate);
				NIM_PRINTF("            work_mode is %d\n", work_mode);
				nim_c3505_reg_get_map_type(dev, &map_type);
				NIM_PRINTF("            map_type is %d\n", map_type);
				AUTOSCAN_PRINTF("\tLock freq %d Rs %d with del_Rs %d del_f %d \n",\
				tempFreq, Rs, Rs - tp_scan_param->est_rs, tempFreq - tp_scan_param->est_freq);

				AUTOSCAN_PRINTF("\t tr_lock_num %d, cr_lock_num %d, lock_monitor %d,\n",\
				tr_lock_num, cr_lock_num, lock_monitor);

				AUTOSCAN_PRINTF("work_mode= %d, s2_lock_cnt= %d \n",work_mode,s2_lock_cnt);
				//final_est_freq = (tempFreq + 512) / 1024;
				final_est_freq = tempFreq;
				final_est_sym_rate = Rs;

				c3505_real_freq[c3505_real_tp_number] = tempFreq/1024;//store the frequency and sysbolrate used by NIM_DRIVER_GET_VALID_FREQ	 
				c3505_real_sym_rate[c3505_real_tp_number] =Rs;				
				c3505_real_tp_number = c3505_real_tp_number + 1;
#ifdef NIM_S3501_ASCAN_TOOLS
				nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP,0x00,tempFreq, Rs, tempFreq>1550?1:0);
#endif
				nim_c3505_interrupt_clear(dev);
				AUTOSCAN_PRINTF("[%s %d]timeout = %d, s2_lock_cnt=%d\n", __FUNCTION__, __LINE__, timeout, s2_lock_cnt);
				return SUCCESS;
			}
		}

		//TR_lock
		if((intindex&0x04) == 0x04)
		{
		#ifdef AUTOSCAN_SPEEDUP
			nim_c3505_get_symbol_rate(dev, &Rs);
			if (Rs >= (RS_LIMITED - 1))
			{
				nim_c3505_get_symbol_rate(dev, &Rs);
				lock_monitor = lock_monitor | 0x04;
				tr_lock_num++;
				final_est_sym_rate = Rs;		
			}
			else
			{
				tr_lock_num--;
				AUTOSCAN_PRINTF("        RS=%d. Lower than RS_LIMITED: %d \n", Rs, RS_LIMITED);
			}
#else
			lock_monitor = lock_monitor | 0x04;
			tr_lock_num++;
            nim_c3505_get_symbol_rate(dev, &Rs);
            final_est_sym_rate = Rs;
#endif
			//lock_monitor = lock_monitor | 0x04;
			//tr_lock_num++;
            //nim_c3505_get_symbol_rate(dev, &Rs);
            //final_est_sym_rate = Rs;
			if (tr_lock_flag==0)
			{
				AUTOSCAN_PRINTF("        RS=%d \n", Rs);
				//if(Rs<=1000)
				if(Rs < (RS_LIMITED - 1))
				{
					tr_lock_num--;
					if (tr_lock_num < 0)
                    	tr_lock_num = 0;
				}
				else
				{
					tr_lock_flag = 1;

					// when lock the tr, refresh the RS value and set it to demod , now not to tuner ????
					tp_scan_param->est_rs = Rs;

					nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

					priv->nim_tuner_control(priv->tuner_index, tp_scan_param->tuner_freq, tp_scan_param->est_rs + tp_scan_param->correct_rs);

					comm_delay(500);

					// initial hardware and set/get some parameter
					tp_scan_param->search_status = 1;
					nim_c3505_soft_search_init(dev, tp_scan_param);

					// set DVB-S2 Physical Layer Frame Sync detector time out threshold
					if (CHIP_ID_3503D != priv->ul_status.m_c3505_type)
                    {               
    					data = 0x00;
    					nim_reg_write(dev, R28_PL_TIMEOUT_BND,&data,1);
    					nim_reg_read(dev,R28_PL_TIMEOUT_BND+1,&data,1);
    					data &= 0xe0;
    					data |= 0x01;
    					nim_reg_write(dev,R28_PL_TIMEOUT_BND+1,&data,1);
                    }

					// start search
					nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

					AUTOSCAN_PRINTF("Reset: freq = %d, Rs = %d at time %d\n", tp_scan_param->est_freq, Rs, timeout);
					lock_monitor = 0;
					tr_lock_num = 0;
					cr_lock_num = 0;
					rs_rev = Rs;
					timeout = 0;
				}
			}
		}

		// AUTOSCAN_PRINTF("     lock_monitor is 0x%x\n", lock_monitor );

		// check the time out value
		if(tp_scan_param->tr_lock_thr == timeout)
		{
			AUTOSCAN_PRINTF("tp_scan_param->tr_lock_thr=%d, timeout=%d\n", tp_scan_param->tr_lock_thr, timeout);
			if((lock_monitor&0xfc) == 0x00) // TR never lock
			{
                nim_c3505_interrupt_clear(dev);
                priv->plsn.search_plsn_force_stop = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
                PLSN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
                return ERR_FAILED; 
			}
		}
		else if(tp_scan_param->cr_lock_thr == timeout)
		{
			AUTOSCAN_PRINTF("tp_scan_param->cr_lock_thr=%d, timeout=%d\n", tp_scan_param->cr_lock_thr, timeout);
			if(((lock_monitor&0xf8) == 0x00) &  (tr_lock_num < 10)) // TR  lock
			{
				AUTOSCAN_PRINTF("        Fault TP, exit without CR lock,  tr_lock_num %d, time out %d\n", tr_lock_num, timeout);
				
				err_flag = 1;
				goto ERR_HANDLE;
			}
		}
		else if(tp_scan_param->fs_lock_thr == timeout)
		{
			AUTOSCAN_PRINTF("tp_scan_param->fs_lock_thr=%d, timeout=%d\n", tp_scan_param->fs_lock_thr, timeout);
			if(((lock_monitor&0xf0) == 0x00) &  (cr_lock_num < 10)) // CR  lock
			{
				AUTOSCAN_PRINTF("        Fault TP, exit without Frame lock, cr_lock_num %d, time out %d\n", cr_lock_num, timeout);
				err_flag = 1;
				goto ERR_HANDLE;
			}
		}

		// Get the interrupt status 
		nim_reg_read(dev, R00_CTRL, &intindex, 1);
		if (0 == (intindex & 0x10))
		{
			nim_reg_read(dev, R02_IERR, &intdata, 1);
			nim_c3505_interrupt_clear(dev); // 2015_10_30

			AUTOSCAN_PRINTF("       Interrupt register is 0x%02x\n", intdata);
			if (0x04 == (intdata & 0x04))
			{
				AUTOSCAN_PRINTF("        Can not lock chanel, time out %d\n", timeout);
				
				err_flag = 1;
				goto ERR_HANDLE;
			}          
			else if(0x08 == (intdata & 0x08))
			{
				AUTOSCAN_PRINTF("    HW timeout, time out %d\n", timeout);
				AUTOSCAN_PRINTF("\t tr_lock_num %d, cr_lock_num %d, lock_monitor %d,\n",\
				tr_lock_num, cr_lock_num, lock_monitor);
				err_flag = 1;		
				goto ERR_HANDLE;
			}
			
#ifdef AUTOSCAN_SPEEDUP
						// For autosacn speedup 20170803 paladin								
						if (0x10 == (intdata & 0x10))								
						{								
							AUTOSCAN_PRINTF("		 TR loss lock  \n");								
#if 1								
							tr_loss_cnt++;								
							//if ((tr_loss_cnt > 2) && (timeout > tp_scan_param->tr_lock_thr))		
							if ((tr_loss_cnt > 2) && (timeout > tp_scan_param->cr_lock_thr))		
							{								
								AUTOSCAN_PRINTF("\t TR loss lock, tr_loss_cnt %d, tr_lock_num %d, lock_monitor %d,\n",
								tr_loss_cnt, tr_lock_num, lock_monitor);
								if (priv->plsn.super_scan)
                                {
                                    err_flag = 1;
                                    goto ERR_HANDLE;
                                }
								else
									return ERR_FAILED;								
							}								
#endif								
						}								
						else								
						{								
							tr_loss_cnt = 0;								
						}
#endif


#if 1//NIM_OPTR_ACM == ACM_CCM_FLAG    // In acm work mode this bit is valid only for dvb-s try
			if ((intdata & 0x02) && tp_scan_param->change_work_mode_flag) 
#else
			if (intdata & 0x02)
#endif
			{
				NIM_PRINTF("    path1:lock chanel \n");
				tempFreq = tp_scan_param->est_freq;
				nim_c3505_get_freq(dev, &tempFreq);             

				NIM_PRINTF("            Freq is %d\n", tempFreq);

				priv->ul_status.m_cur_freq = tempFreq ;

				nim_c3505_get_symbol_rate(dev, &Rs);

				NIM_PRINTF("            Rs is %d\n", Rs);
				del_rs = rs_rev - Rs;
				if(del_rs < 0)
				del_rs = -del_rs;

				//if((Rs >= 1000)&&(del_rs >= 100))
				if((Rs >= RS_LIMITED)&&(del_rs >= 100))
					rs_rev = Rs;
				Rs = rs_rev;
				nim_c3505_reg_get_code_rate(dev, &code_rate);
				NIM_PRINTF("            code_rate is %d\n", code_rate);
				nim_c3505_reg_get_work_mode(dev, &work_mode);
				NIM_PRINTF("            work_mode is %d\n", work_mode);
				nim_c3505_reg_get_map_type(dev, &map_type);
				NIM_PRINTF("            map_type is %d\n", map_type);
				nim_c3505_reg_get_modcod(dev, &modcod);
				if(modcod & 0x01)
					NIM_PRINTF("            Pilot on \n");
				else
				{
					NIM_PRINTF("            Pilot off \n");
				}
				modcod = modcod >> 1;
				NIM_PRINTF("            Modcod is %x\n", modcod);

				//final_est_freq = (tempFreq + 512) / 1024;
				final_est_freq = tempFreq;
				final_est_sym_rate = Rs;
#ifdef NIM_S3501_ASCAN_TOOLS
				nim_s3501_ascan_add_tp(ASCAN_ADD_REAL_TP,0x00, tempFreq, Rs, tempFreq > 1550? 1:0);
#endif
				AUTOSCAN_PRINTF("\tLock path1 freq %d Rs %d with del_Rs %d del_f %d \n",\
				tempFreq, Rs, Rs - tp_scan_param->est_rs, tempFreq - tp_scan_param->est_freq);
				AUTOSCAN_PRINTF("\t tr_lock_num %d, cr_lock_num %d, lock_monitor %d,\n",\
				tr_lock_num, cr_lock_num, lock_monitor);

				AUTOSCAN_PRINTF("\t timeout = %d\n", timeout);
				//nim_c3505_interrupt_clear(dev);
				AUTOSCAN_PRINTF("[%s %d]timeout = %d\n", __FUNCTION__, __LINE__, timeout);
				return SUCCESS;
			}
			//nim_c3505_interrupt_clear(dev);
		}

ERR_HANDLE:				
		if (err_flag)
		{
			nim_c3505_get_search_timeout(dev, &search_single_timeout, &search_total_timeout);
			state = nim_c3505_try_plsn(dev, &try_index);
			if (pl_find_flag)
			{
				if (PLSN_FIND == state)
				{
					PLSN_PRINTF("[%s %d]try_index=%d\n", __FUNCTION__, __LINE__, try_index);
					PLSN_PRINTF("[%s %d]reinit time!\n", __FUNCTION__, __LINE__);
					timeout = 0;//reinit the start time
					err_flag = 0;
					lock_monitor = 0;
					tr_lock_flag = 0;
					tr_lock_num = 0;
					cr_lock_num = 0;				
					nim_c3505_interrupt_clear(dev);					
				}
				else if ((PLSN_OVER == state) || (PLSN_RUNNING == state))
				{
					comm_sleep(2);//sleep, so the system can dispatch search task
					if ((PLSN_OVER == state) || (osal_get_tick() - search_start_single > search_single_timeout))
					{
						if (osal_get_tick() - search_start_total < search_total_timeout)
						{
							PLSN_PRINTF("[%s %d]start next search plsn, state=%d!, search_single_timeout=%d ms, search_total_timeout=%d ms, timeout=%d ms\n", 
							__FUNCTION__, __LINE__, state, search_single_timeout, search_total_timeout, osal_get_tick() - search_start_single);				
						
							stop_start = osal_get_tick();
							priv->plsn.search_plsn_force_stop = 1;	
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
							while (1==priv->plsn.start_search)
							{			
								comm_sleep(2);

								if (osal_get_tick() - stop_start > 6000)
								{
									PLSN_PRINTF("[%s %d]force stop timeout = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
									break;
								}
							}
							PLSN_PRINTF("[%s %d]stop cost = %d ms!%d\n", __FUNCTION__, __LINE__, osal_get_tick() - stop_start);
							try_index = 0;
							priv->plsn.search_plsn_force_stop = 0;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);

							PLSN_PRINTF("[%s %d]reset demod before next search!\n", __FUNCTION__, __LINE__);
							//reset demod
							nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);	
							priv->plsn.plsn_try = 0;
							nim_c3505_set_plsn(dev);																			
							comm_sleep(2);							
							//start demod
							nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X51);

							priv->plsn.start_search = 1;//start to search plsn
							priv->plsn.auto_scan_start = 1;
							PLSN_PRINTF("priv->plsn.auto_scan_start=%d\n", priv->plsn.auto_scan_start);
							priv->plsn.search_plsn_stop = 0;//to avoid get wrong state when plsn task not start immediate
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_stop);
							try_index = 0;//reinit the try index
							comm_sleep(5);//sleep, so the system can dispatch search task
							PLSN_PRINTF("[%s %d]reinit try index!\n", __FUNCTION__, __LINE__);
							search_start_single = osal_get_tick();
						}
						else
						{
							PLSN_PRINTF("[%s %d]over,state=%d!, timeout=%d ms\n", __FUNCTION__, __LINE__, state, osal_get_tick() - search_start_single);
							PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
							nim_c3505_interrupt_clear(dev);
							priv->plsn.search_plsn_force_stop = 1;
							PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
							AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
							return ERR_FAILED;
						}
					}
				}
				else
				{
					PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
					nim_c3505_interrupt_clear(dev);
					priv->plsn.search_plsn_force_stop = 1;
					PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
					AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
					return ERR_FAILED;
				}
			}
			else
			{
				PLSN_PRINTF("[%s %d]lock fail, cost %d ms time\n", __FUNCTION__, __LINE__, osal_get_tick() - search_start_total);
				nim_c3505_interrupt_clear(dev);
				priv->plsn.search_plsn_force_stop = 1;
				PLSN_PRINTF("[%s %d]priv->plsn.search_plsn_force_stop=%d\n", __FUNCTION__, __LINE__, priv->plsn.search_plsn_force_stop);
				AUTOSCAN_PRINTF("[%s %d]Timeout, state=%d, pl_find_flag=%d\n", __FUNCTION__, __LINE__, state, pl_find_flag);
				return ERR_FAILED;
			}
		}
	}
}


/*****************************************************************************
* INT32 nim_C3505_FFT(struct nim_device *dev, UINT32 startFreq)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 startFreq
*
* Return Value: void
*****************************************************************************/
INT32 nim_c3505_fft(struct nim_device *dev, UINT32 startFreq)
{
	UINT32 freq = 0;
	UINT8 lock = 200;
	UINT32 cur_f, last_f, cur_rs, last_rs, ch_num;
	INT32 i = 0;
	INT32 channel_freq_err = 0;
	/**************** Get Current Time for Debug Convenient****************/
	struct nim_c3505_private *dev_priv;
	//date_time dt;
	//get_local_time(&dt);
	//AUTOSCAN_PRINTF("\nStep into nim_C3505_FFT:Blind Scan Current 90MHZ BW time %d:%d:%d\n",dt.hour,dt.min,dt.sec);

	dev_priv = (struct nim_c3505_private *)dev->priv;
	freq = startFreq;

	//clear interrupt for software search
	if (dev_priv->ul_status.c3505_autoscan_stop_flag)
	{
		NIM_PRINTF("    leave fuction nim_C3505_FFT\n");
		return SUCCESS;
	}
	
	channel_freq_err = 0;
	call_tuner_command(dev, NIM_TUNER_SET_C3031B_FREQ_ERR, &channel_freq_err);
	
	dev_priv->nim_tuner_control(dev_priv->tuner_index, freq, 0);

	comm_delay(0xffff*4);
	if (dev_priv->nim_tuner_status != NULL)
		dev_priv->nim_tuner_status(dev_priv->tuner_index, &lock);

	dev_priv->ul_status.m_cr_num = 0;
	dev_priv->ul_status.m_step_freq = nim_c3505_cap_fft_find_channel(dev, &startFreq);

    AUTOSCAN_PRINTF("\tCurrent Time Blind Scan Range: From  %dMHz to %dMHz  have find %d channels\n", \
		startFreq - 45, startFreq + 45, dev_priv->ul_status.m_cr_num);
	for (i = 0; i < dev_priv->ul_status.m_cr_num; i++)
	{
		AUTOSCAN_PRINTF("\tTP -> %d. Freq = %d, Rs = %d\n", i, dev_priv->ul_status.m_freq[i], dev_priv->ul_status.m_rs[i]);
	}

	//if(NIM_SCAN_SLOW == dev_priv->blscan_mode)
	{
		// amy add for double check, avoid one TP detected as two
		if (dev_priv->ul_status.m_cr_num > 1)
		{
			ch_num = dev_priv->ul_status.m_cr_num;
			for (i = 1; i < dev_priv->ul_status.m_cr_num; i++)
			{
				cur_f = dev_priv->ul_status.m_freq[i];
				last_f = dev_priv->ul_status.m_freq[i-1];

				cur_rs = dev_priv->ul_status.m_rs[i];
				last_rs = dev_priv->ul_status.m_rs[i-1];

				if (cur_f-last_f < (cur_rs + last_rs)/2000)
				{
					cur_f = last_f + (cur_f - last_f)*cur_rs / (cur_rs+last_rs);
					cur_rs = last_rs + (cur_f-last_f) * 2000;
					dev_priv->ul_status.m_freq[ch_num] = cur_f;
					dev_priv->ul_status.m_rs[ch_num] = cur_rs;
					ch_num ++;
					AUTOSCAN_PRINTF("\tError detected TP, modified to -> %d. Freq=%d, Rs=%d\n",ch_num,cur_f,cur_rs);
				}
			}
			if (dev_priv->ul_status.m_cr_num < ch_num)
			{
				AUTOSCAN_PRINTF("current FFT result is:\n");
				for (i = 0;i < 1024;i = i+1)
					AUTOSCAN_PRINTF("%d\n",fft_energy_1024[i]);
			}
		    dev_priv->ul_status.m_cr_num = ch_num;

	    }

		for (i = 0; i < dev_priv->ul_status.m_cr_num; i++)
		{
			//AUTOSCAN_PRINTF("\tAfter raw TP Modified,TP Distribution:TP -> %d. Freq = %d, Rs = %d\n", i, dev_priv->ul_status.m_freq[i], dev_priv->ul_status.m_rs[i]);
		}
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *tune_freq
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_cap_fft_find_channel(struct nim_device *dev, UINT32 *tune_freq)
{
	INT32 success;
	INT32 delta_fc_est[MAX_CH_NUMBER];
	INT32 SymbolRate_est[MAX_CH_NUMBER];
	INT32 IF_freq;
	INT32 ch_number;
	INT32 i;
	INT32 tune_jump_freq;
	INT32 tempFreq;
	INT32 temp, temp1;
	UINT8 data;
	UINT32 sample_rate;

	struct nim_c3505_private *priv = (struct nim_c3505_private *) dev->priv;

	nim_c3505_cap_calculate_energy(dev);
	NIM_PRINTF("tune_freq = %d\n", *tune_freq);

	if(NIM_SCAN_SLOW == priv->blscan_mode)
	{
		comm_memcpy( fft_energy_store,fft_energy_1024,sizeof(fft_energy_1024));
		temp1 = fft_energy_store[511];
		fft_energy_store[511] = (fft_energy_store[510]+fft_energy_store[512])/2;
		
		for (i=6;i<1024-6;i++)
		{
			temp = 0;
			for (data=0;data<12;data++)
			temp = temp + fft_energy_store[i-6+data];
			temp = temp/12;
			fft_energy_1024[i] = temp;
		}
		fft_energy_store[511] = temp1;
	}

	nim_s3501_autosearch((INT32 *) &success, (INT32 *) delta_fc_est, (INT32 *) SymbolRate_est, (INT32 *) &IF_freq, (INT32 *) &ch_number);

	nim_c3505_get_dsp_clk(dev, &sample_rate);

	if (1 == success)
	{
		for (i = 0; i < ch_number; i++)
		{
			temp = nim_c3505_multu64div(delta_fc_est[i], sample_rate, 90000); //
			tempFreq = (*tune_freq)*1024 + temp;
			priv->ul_status.m_freq[priv->ul_status.m_cr_num] = tempFreq;
			NIM_PRINTF("    m_Freq[%d] is %d\n", priv->ul_status.m_cr_num, priv->ul_status.m_freq[priv->ul_status.m_cr_num]);
			priv->ul_status.m_rs[priv->ul_status.m_cr_num] = nim_c3505_multu64div(SymbolRate_est[i], sample_rate, 90000);
			priv->ul_status.m_cr_num++;
		}
	}
	else
	{
		//NIM_PRINTF("err\n");
		NIM_PRINTF("    Base band width is %d, ch_number is%d\n", IF_freq / 1024, ch_number);
	}
	IF_freq = nim_c3505_multu64div(IF_freq, sample_rate, 90000); //
	tune_jump_freq = IF_freq / 1024;

	return tune_jump_freq;
}
/*****************************************************************************
*  INT32 nim_C3505_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq
*  Parameter3: UINT32 *start_adr
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32 *start_adr)
{
	int i = 0, index = 0;
	int ave_energy = 0, max_est = 0, maxvalue = 0;
	UINT8 data = 0x10;//,ver_data;
	UINT8 lock = 200;
	INT32 channel_freq_err = 0;
	struct nim_c3505_private *dev_priv = NULL;

	dev_priv = (struct nim_c3505_private *)dev->priv;

	if (NULL == start_adr)
		return ERR_ADDRESS;
	//reset first
	nim_c3505_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X90);

	data = 0x35;////0x2B;
	nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    if(NULL != dev_priv->nim_tuner_control)
    {
		channel_freq_err = 0;
		call_tuner_command(dev, NIM_TUNER_SET_C3031B_FREQ_ERR, &channel_freq_err);
		dev_priv->nim_tuner_control(dev_priv->tuner_index, freq, 0);
    }
	
	comm_sleep(10);
	dev_priv->nim_tuner_status(dev_priv->tuner_index, &lock);

	nim_c3505_cap_calculate_energy(dev);
	nim_s3501_smoothfilter();

	comm_memcpy((int *)start_adr, fft_energy_1024, sizeof(fft_energy_1024));
	//just deal with middle part of the data.
	//start from 256 to 767.
	//if needed, the range should be 0~1023, the whole array.

	start_adr[511] = (start_adr[510] + start_adr[512]) / 2;
	i = 0;ave_energy = 0;max_est = 0;
	for (index = 256; index <= 767; index++)
	{
		ave_energy += start_adr[index];
		if (i == (1 << (STATISTIC_LENGTH + 1)) - 1)
		{
			if (ave_energy > max_est)
				max_est = ave_energy;
			i = 0;
			ave_energy = 0;
		}
		else
		{
			i++;
		}
	}
	max_est = max_est / 8;
	maxvalue = max_est;

	for (index = 256; index <= 767; index++)
	{
		if ((start_adr[index] > (UINT32) maxvalue) && (start_adr[index] < (UINT32) (max_est * 2)))
			maxvalue = start_adr[index];
	}

	for (index = 256; index <= 767; index++)
	{
		if (start_adr[index] > (UINT32) maxvalue)
			start_adr[index] = maxvalue;
		//devider = maxvalue/250;
		start_adr[index] = start_adr[index] * 250 / maxvalue;
	}
	return SUCCESS;
}

/*****************************************************************************
*  nim_c3505_get_search_timeout(struct nim_device *dev, UINT32 timeout)
* 
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: single_timeout, uint is ms
*  Parameter3: total_timeout, uint is ms, some is same with parameter1, some is 2 times parameter1
* Return Value: result
*****************************************************************************/
INT32 nim_c3505_get_search_timeout(struct nim_device *dev, UINT32 *single_timeout, UINT32 *total_timeout)
{
	struct nim_c3505_private *priv = NULL;

	if(NULL == dev)
	{
		ERR_PRINTF("Exit with ERR_NO_DEV in %s \n",__FUNCTION__);
		return ERR_NO_DEV;
	}

	priv = (struct nim_c3505_private *) dev->priv;   
	if(NULL == priv)
	{
		ERR_PRINTF("[%s %d]NULL == priv\n", __FUNCTION__, __LINE__);
		return ERR_NO_DEV;
	}	

	if (priv->plsn.super_scan)//only in super_scan mode, it will start search plsn
	{
		if (CHIP_ID_3505 == priv->ul_status.m_c3505_type)
		{
			if (4 == priv->plsn.plsn_search_algo)//the second algorithm
			{
				*single_timeout = 20000; // 20ms
				*total_timeout = 40000;//(*single_timeout)*2
			}
			else//the first algotithm
			{
				*single_timeout = 20000; // 20ms
				*total_timeout = 20000;//same with single timeout
			}
		}
		else if ((CHIP_ID_3503C == priv->ul_status.m_c3505_type) || (CHIP_ID_3503D == priv->ul_status.m_c3505_type))
		{
			if (4 == priv->plsn.plsn_search_algo)
			{
				*single_timeout = 30000; // 30ms
				*total_timeout = 60000;//(*single_timeout)*2
			}
			else
			{
				*single_timeout = 20000; // 20ms
				*total_timeout = 20000;//same with single timeout
			}
		}
		else
		{
			*single_timeout = 10000; // 10ms
			*total_timeout = 10000;//same with single timeout
		}
	}
	else
	{
		*single_timeout = 0;
		*total_timeout = 0;
	}

	return SUCCESS;
}

