#include "nim_cxd2856_common.h"

static INT32 need_to_check_current_info(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_integ_t * priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	//UINT32 tuner_id = pdemod->tuner_id;
	UINT8 lock = 0;
	if(pdemod->Frequency == change_para->freq &&
	   pdemod->bandwidth == change_para->bandwidth &&
	   nim_cxd2856_get_lock(dev,&lock) == SUCCESS)
	{
		if(1 == lock) //current status ok,don't channel change again
		{
			return SUCCESS;
		}
		else
		{
			return ERR_FAILUE;
		}
		
	}
	else
	{
		return ERR_FAILUE;
	}
}

static INT32 need_to_check_t2_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod 	= priv->pDemod;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE \
        || change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.
	
    if (pdemod->plp_index != change_para->plp_index) //Current selected PLP is different with the target PLP.
        return TRUE;

    if (pdemod->t2_profile != change_para->t2_profile) //DVB-T2 profile: base or lite.
        return TRUE;
	 
    return TRUE;
}
static INT32 Dvbt2_tune_unknown_profile(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_result_t tuneResult 	= SONY_RESULT_OK;
	sony_integ_t * priv 		= (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod 		= priv->pDemod;
	sony_dvbt2_profile_t t2ProfileTuned = SONY_DVBT2_PROFILE_ANY;//blind tune only

	tuneResult = sony_integ_dvbt2_BlindTune(priv,change_para->freq,change_para->bandwidth,SONY_DVBT2_PROFILE_ANY,&t2ProfileTuned);
	switch(tuneResult)
	{
        case SONY_RESULT_OK:
			pdemod->t2_profile = t2ProfileTuned; //get current profile
			CXD2856_LOG("[%s]:DVB-T2 TS Locked and get profile is %s %d.\n",__FUNCTION__, \
			((t2ProfileTuned == SONY_DVBT2_PROFILE_BASE)? "SONY_DVBT2_PROFILE_BASE":"SONY_DVBT2_PROFILE_LITE"),t2ProfileTuned);
	        return SUCCESS;
        case SONY_RESULT_ERROR_UNLOCK:
	        CXD2856_LOG("[%s]:DVB-T2 TS Unlocked on unknown_profile mode.\n",__FUNCTION__);
	        return ERR_FAILUE;
        case SONY_RESULT_ERROR_TIMEOUT:
            CXD2856_LOG("[%s]:DVB-T2 Wait TS Lock but Timeout on unknown_profile mode.\n",__FUNCTION__);
	        return ERR_FAILUE;
        default:
	        CXD2856_LOG("[%s]:Error in sony_integ_dvbt2_Tune on unknown_profile mode\n",__FUNCTION__);
	        return ERR_FAILUE;
    }
}

static INT32 Dvbt2_tune_plp(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_result_t tuneResult 	= SONY_RESULT_OK;
	sony_integ_t * priv 		= (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod 		= priv->pDemod;

	pdemod->t2_signal = DVBT2_TYPE;
	pdemod->plp_id = change_para->plp_id;

	sony_dvbt2_tune_param_t tuneParam;
	memset(&tuneParam,0,sizeof(tuneParam));
	tuneParam.centerFreqKHz = change_para->freq;
	tuneParam.bandwidth		= change_para->bandwidth;
	tuneParam.dataPlpId 	= change_para->plp_id; //selected data PLP ID. 
	tuneParam.profile 		= change_para->t2_profile;
	tuneParam.tuneInfo 		= SONY_DEMOD_DVBT2_TUNE_INFO_OK;
		
	tuneResult = sony_integ_dvbt2_Tune(priv, &tuneParam);

	switch(tuneResult){
        case SONY_RESULT_OK:
	        CXD2856_LOG("[%s]:DVB-T2 TS Locked on plp_ID %d.\n",__FUNCTION__,tuneParam.dataPlpId);
	        return SUCCESS;
        case SONY_RESULT_ERROR_UNLOCK:
	        CXD2856_LOG("[%s]:DVB-T2 TS Unlocked on plp_ID %d.\n",__FUNCTION__,tuneParam.dataPlpId);
	        return ERR_FAILUE;
        case SONY_RESULT_ERROR_TIMEOUT:
            CXD2856_LOG("[%s]:DVB-T2 Wait TS Lock but Timeout on plp_ID %d.\n",__FUNCTION__,tuneParam.dataPlpId);
	        return ERR_FAILUE;
        default:
	        CXD2856_LOG("[%s]:Error in sony_integ_dvbt2_Tune on plp_ID %d.\n",__FUNCTION__,tuneParam.dataPlpId);
	        return ERR_FAILUE;
    }

}
static INT32 Dvbt2_to_lock_next_data_plp(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
    INT32 result 			= ERR_FAILUE;
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	sony_demod_t * param 	= priv->pDemod;
	sony_dvbt2_l1pre_t L1Pre;
	UINT8 plp_index = 0; 
	
	plp_index = param->plp_index+1;
	if (plp_index >= param->plp_num)
    {
    	libc_printf("11\n");
        return ERR_FAILUE;
    }
	change_para->plp_id = param->all_plp_id[plp_index];
	
	result = Dvbt2_tune_plp(dev, change_para);
	if (result == SONY_RESULT_OK) 
	{
		param->plp_id = param->all_plp_id[plp_index];
	    param->t2_system_id = L1Pre.systemId;
	    param->plp_index = plp_index;
		libc_printf("%s %d next\n",__FUNCTION__,__LINE__);
		return SONY_RESULT_OK;
	}
	return ERR_FAILUE;
	#if 0
	
    for (plp_index = param->plp_index+1; plp_index < param->plp_num; ++plp_index )
    {
        if (plp_index >= param->plp_num)
            return ERR_FAILUE;
        result = DVBT2_change_PLP(dev, param->all_plp_id[plp_index], FALSE, FALSE, 0);
        
        for ( retry=0; retry<30; ++retry )
        {
            SONY_SLEEP (30);
            if (param->autoscan_stop_flag)
                return ERR_FAILUE;            
            if (param->do_not_wait_t2_signal_locked)
                return ERR_FAILUE;
            
            result = sony_demod_dvbt2_monitor_L1Pre(param, &L1Pre);//sony_dvb_demod_monitorT2_L1Pre
            if (result != SONY_RESULT_OK) 
            {
                CXD2856_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d) error: sony_dvb_demod_monitorT2_L1Pre()=%d \r\n", __FUNCTION__,NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index],result);
                continue;
				//return result;
            }
        
            // Get Active PLP information. 
            result = sony_demod_dvbt2_monitor_ActivePLP(param, SONY_DVBT2_PLP_DATA, &plpInfo);//sony_dvb_demod_monitorT2_ActivePLP
            if (result != SONY_RESULT_OK) 
            {
				CXD2856_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d) error: sony_dvb_demod_monitorT2_ActivePLP()=%d \r\n", __FUNCTION__,NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index],result);
				continue;
				//return result;
            }
            
            if (result == SONY_RESULT_OK) 
            {
                if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
                {
                    if (plpInfo.id != param->all_plp_id[plp_index])
                    {
                        CXD2856_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), plpInfo.id=%d, error PLP locked: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], plpInfo.id, retry);
                        continue;
                    }
                    else
                        break; //correct PLP is locked.
                }
            }
        }

        if (result == SONY_RESULT_OK  && (plpInfo.id == param->all_plp_id[plp_index]) ) 
        {
            if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
            {
                param->plp_id = plpInfo.id;
                param->t2_system_id = L1Pre.systemId;
                param->plp_index = plp_index;
                
                if (retry!=0)
                    CXD2856_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
                return SONY_RESULT_OK;
            }
            else
            {
                    CXD2856_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times. error: Not DataPLP: (type=%d, id=%d)\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry, plpInfo.type, plpInfo.id);
            }
        }
        else
        {
            CXD2856_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), error: fail to lock the PLP.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
        }
    }
    return ERR_FAILUE;
	#endif
}

static INT32 Dvbt2_to_search_plp_sum(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{	
	UINT16 waitTime 		= 0;
	//UINT8 plp_index 		= 0; 
	//UINT8 retry 			= 0; 
	INT32 result 			= ERR_FAILUE;
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	sony_demod_t * param 	= priv->pDemod;
	//sony_dvbt2_l1pre_t L1Pre;
	//sony_dvbt2_plp_t plpInfo;
	UINT8 plp_idx;
	
	param->plp_num 		= 0;
	param->plp_id 		= 0;
	param->plp_index 	= 0;
        
	Dvbt2_tune_unknown_profile(dev,change_para);
	memset(param->all_plp_id,0,sizeof(param->all_plp_id)/sizeof(param->all_plp_id[0])); //clear array
	do
	{
		result = sony_demod_dvbt2_monitor_DataPLPs(param, (uint8_t *)(&(param->all_plp_id)), &(param->plp_num));//sony_dvb_demod_monitorT2_DataPLPs
		if (result == SONY_RESULT_OK) 
		{
			
			for (plp_idx=0; plp_idx < param->plp_num; ++plp_idx)
			{
				libc_printf( "\t[plp_id=%d]\n", param->all_plp_id[plp_idx]);
			}
			param->plp_id = param->all_plp_id[0];//back first plp id;
			break;
		}
		else if (result == SONY_RESULT_ERROR_HW_STATE) 
		{
			if (waitTime >= DTV_DEMOD_TUNE_T2_L1POST_TIMEOUT)
			{
				libc_printf("%s() error: timeout for get the first data_PLP\r\n", __FUNCTION__);
				param->plp_num = 0;
				return ERR_FAILUE;
			}
			else 
			{
				SONY_SLEEP (10); //10
				waitTime += 10; 
			}
		} 
		else 
		{
			libc_printf("%s()=%d error: Fail to get the first data_PLP\r\n", __FUNCTION__, result);
			param->plp_num = 0;
			return ERR_FAILUE; // Other (fatal) error.
		}
	}while (1);
    //result = Dvbt2_tune_plp(dev,change_para);
	return result;
}

static INT32 nim_cxd2856_lock_t2_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
	INT32 result 			= SONY_RESULT_OK;
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod 	= priv->pDemod;
	//sony_dvbt_tune_param_t tuneParam;

	pdemod->Frequency = change_para->freq;
	pdemod->bandwidth = change_para->bandwidth;
	
	switch(change_para->usage_type)
	{
		case USAGE_TYPE_AERIALTUNE:
		case USAGE_TYPE_AUTOSCAN://step 1: to search the sum of plp_id and return the first plpid
		case USAGE_TYPE_CHANSCAN:
			result = Dvbt2_to_search_plp_sum(dev,change_para);
			change_para->plp_num 		= pdemod->plp_num;
			change_para->t2_profile 	= pdemod->t2_profile;

			change_para->plp_index 		= pdemod->plp_index;
			change_para->plp_id 		= pdemod->plp_id;//return the first plpid
			change_para->t2_system_id 	= pdemod->t2_system_id;
			break;
		case USAGE_TYPE_NEXT_PIPE_SCAN:// setp 2: to return all the remaining plp_id
			result = Dvbt2_to_lock_next_data_plp(dev,change_para);

			change_para->plp_index 		= pdemod->plp_index;
			change_para->t2_system_id 	= pdemod->t2_system_id;
			break;
		case USAGE_TYPE_CHANCHG://step 3:to channel change
			result = Dvbt2_tune_plp(dev,change_para);//only to tune current chennnel
			break;
		default:
			break;
	}
	return result;
	
}

static INT32 nim_cxd2856_lock_t_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
	INT32 result 			= SONY_RESULT_OK;
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	//sony_demod_t * pdemod 	= priv->pDemod;
	sony_dvbt_tune_param_t tuneParam;
	if(SUCCESS == need_to_check_current_info(dev, change_para))
	{
		CXD2856_LOG( "%d %s freq is the same as last time,and still lock successfully,so return!",__LINE__,__FUNCTION__);
		return SUCCESS;
	}
	CXD2856_LOG( "[%s %d]Enter!\n", __FUNCTION__, __LINE__);
    /* Configure the DVBT tune parameters based on the channel requirements */
    tuneParam.centerFreqKHz = change_para->freq;    /* Channel centre frequency in KHz */
    tuneParam.bandwidth = change_para->bandwidth;   /* Channel bandwidth */
    //tuneParam.profile = SONY_DVBT_PROFILE_HP;       /* Channel profile for hierachical modes.  For non-hierachical use HP */
	tuneParam.profile = change_para->priority;

	/* Perform DVB-T Tune */
    result = sony_integ_dvbt_Tune (priv, &tuneParam);
    if (result != SONY_RESULT_OK) {
		CXD2856_LOG( "%s: Error : sony_integ_dvbt_Tune failed\r\n", __FUNCTION__);
        return ERR_FAILUE;
    }
	return SUCCESS;

}

static INT32 nim_cxd2856_lock_isdbt_signal(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
	INT32 result 			= SONY_RESULT_OK;
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	//sony_demod_t * pdemod 	= priv->pDemod;
    sony_isdbt_tune_param_t tuneParam;
	if(SUCCESS == need_to_check_current_info(dev, change_para))
	{
		CXD2856_LOG( "%d %s freq is the same as last time,and still lock successfully,so return!",__LINE__,__FUNCTION__);
		return SUCCESS;
	}
	CXD2856_LOG( "[%s %d]Enter!\n", __FUNCTION__, __LINE__);
    /* Configure the isdbt tune parameters based on the channel requirements */
    tuneParam.centerFreqKHz = change_para->freq;    /* Channel centre frequency in KHz */
    tuneParam.bandwidth = change_para->bandwidth;   /* Channel bandwidth */

    /* Perform ISDB-T Tune */
    result = sony_integ_isdbt_Tune (priv, &tuneParam);
    if (result != SONY_RESULT_OK) {
		CXD2856_LOG( "%s:Error : sony_integ_isdbt_Tune failed\r\n", __FUNCTION__);
		return ERR_FAILUE;
    }
	return SUCCESS;
}
static INT32 nim_cxd2856_lock_combo_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
	//if system don't issue the signal type,need to try signal type,only t/t2 neerd to try,first try T2 type,isdbt don't need,
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod 	= priv->pDemod;
	
	//according to current market,first try T2 mode
	if(SONY_RESULT_OK == nim_cxd2856_lock_t2_signal(dev,change_para))
	{
		CXD2856_LOG( "[%s %d]Enter!\n", __FUNCTION__, __LINE__);
		pdemod->t2_signal = DVBT2_TYPE;//t2
	}
	else if(SONY_RESULT_OK == nim_cxd2856_lock_t_signal(dev,change_para))
	{
		CXD2856_LOG( "[%s %d]Enter!\n", __FUNCTION__, __LINE__);
		pdemod->t2_signal = DVBT_TYPE;//t
	}
	else
	{
		CXD2856_LOG( "[%s %d]Enter!\n", __FUNCTION__, __LINE__);
		//pdemod->t2_signal = 3;//unknown system signal
		return ERR_FAILUE;//try fail
	}
	return SUCCESS;
}


INT32 nim_cxd2856_channel_change(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
	INT32 result 			= ERR_FAILUE;
	sony_integ_t * priv 	= (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod 	= priv->pDemod;
	BOOL flag = TRUE;
	if(NULL == dev || NULL == change_para)
	{
		return ERR_FAILUE;
	}
	if ((change_para->freq <= 40000) || (change_para->freq >= 900000))
	{
		return ERR_FAILUE;
	}
	if (change_para->bandwidth != SONY_DTV_BW_1_7_MHZ && change_para->bandwidth != SONY_DTV_BW_5_MHZ
        && change_para->bandwidth != SONY_DTV_BW_6_MHZ && change_para->bandwidth != SONY_DTV_BW_7_MHZ
        && change_para->bandwidth != SONY_DTV_BW_8_MHZ)
	{
		return ERR_FAILUE;
	}
	//beceuse previous way that 
	CXD2856_LOG("[%s %d]change_para->freq=%d, change_para->bandwidth=%d,\
		change_para->t2_signal=%d, change_para->usage_type=%d pdemod->system = %d\n",\
		__FUNCTION__, __LINE__, change_para->freq, change_para->bandwidth,\
		change_para->t2_signal, change_para->usage_type,pdemod->system);
	switch(change_para->t2_signal)
	{
		case ISDBT_TYPE://isdbt
			result = nim_cxd2856_lock_isdbt_signal(dev,change_para);	
			break;	
		case DVBT_TYPE://dvbt
			result = nim_cxd2856_lock_t_signal(dev,change_para);
			break;
		case DVBT2_TYPE://dvbt2
			result = nim_cxd2856_lock_t2_signal(dev,change_para);
			break;
		case DVBT2_COMBO://combo
			result = nim_cxd2856_lock_combo_signal(dev,change_para);
			if(SUCCESS == result)// try success
			{
				change_para->t2_signal = pdemod->t2_signal; //pass system signal back to the user app
				flag = FALSE;
			}
			break;
		default:
			break;
	}
	if(SUCCESS == result || result == SONY_RESULT_OK)
	{
		//if lock successfully,update the current info
		pdemod->bandwidth = change_para->bandwidth;
		pdemod->Frequency = change_para->freq;
		return SUCCESS;
	}
	else
	{
		//if lock fail,clear saved info 
		pdemod->bandwidth = pdemod->bandwidth-1;
		pdemod->Frequency = pdemod->Frequency-1;
		return ERR_FAILUE;//unlock
	}
}


