#include "nim_cxd2856_common.h"
INT32 nim_cxd2856_get_lock(struct nim_device *dev, UINT8 *lock)
{
	sony_integ_t * priv = NULL;
	INT32 result = SONY_RESULT_OK;

	if(dev == NULL || lock == NULL)
	{
		return ERR_FAILUE;
	}
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	sony_demod_lock_result_t demod_lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;//DEMOD_LOCK_RESULT_NOTDETECT
	*lock = 0;
    osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	switch (pdemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{			
		    result =  sony_demod_dvbt_CheckTSLock(pdemod, &demod_lock);//sony_dvb_demodT_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		    result =  sony_demod_dvbt2_CheckTSLock(pdemod, &demod_lock);//sony_dvb_demodT2_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_ISDBT:
		{
			result =  sony_demod_isdbt_CheckTSLock(pdemod, &demod_lock);//sony_dvb_demodT2_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			demod_lock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
			break;
    }
    osal_mutex_unlock(pdemod->demodMode_mutex_id);

    if ( SONY_DEMOD_LOCK_RESULT_LOCKED == demod_lock && SONY_RESULT_OK == result)
    {
        *lock = 1;
		CXD2856_LOG("[%s %d]lock\n", __FUNCTION__, __LINE__);
    }else
    {
    	*lock = 0;
		CXD2856_LOG("[%s %d]unlock\n", __FUNCTION__, __LINE__);
    }
	return SUCCESS;
}
UINT8 cxd2856_fft_mode_map_to_ali_fft_mode(sony_dtv_system_t system, UINT8 fft_mode)
{
    //T_NODE:	UINT32 FFT : 8;	
    //2:2k, 8:8k //T2: 4:4k, 16:16k, 32:32k

    if (system == SONY_DTV_SYSTEM_DVBT)
    {
        switch (fft_mode)
        {
            case SONY_DVBT_MODE_2K:
                return 2;
            case SONY_DVBT_MODE_8K:
                return 8;
            default:
                return 0xFF;   //unknown.
        }
    }
    else
    {
        switch (fft_mode)
        {
            case SONY_DVBT2_M2K:
                return 2;
            case SONY_DVBT2_M8K:
                return 8;
            case SONY_DVBT2_M4K:
                return 4;
            case SONY_DVBT2_M1K:
                return 1;
            case SONY_DVBT2_M16K:
                return 16;
            case SONY_DVBT2_M32K:
                return 32;
            default:
                return 0xFF;   //unknown.
        }
    }
}

INT32 nim_cxd2856_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    //UINT32 flgptn;
    OSAL_ER result = SUCCESS;
    sony_integ_t *priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pDemod = priv->pDemod;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_dvbt2_ofdm_t ofdm;
    sony_dvbt_mode_t fft_mode_t;
    sony_dvbt_guard_t gi_t;
  
    *fft_mode = 0;

	switch (pDemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{			
		    result =  sony_demod_dvbt_CheckTSLock(pDemod, &lock);//sony_dvb_demodT_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		    result =  sony_demod_dvbt2_CheckTSLock(pDemod, &lock);//sony_dvb_demodT2_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			lock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
			break;
    }
    if (result != SONY_RESULT_OK || lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
	{
        return ERR_FAILUE;
    }
    if (pDemod->system == SONY_DTV_SYSTEM_DVBT2) 
    {
    	
		result = sony_demod_dvbt2_monitor_OFDM(pDemod, &ofdm);//sony_dvb_demodT2_OptimizeMISO
        //result = sony_demod_dvbt2_monitor_Ofdm(pDemod, &ofdm);//sony_dvb_demodT2_OptimizeMISO
        if (result != SONY_RESULT_OK) 
        {
            return ERR_FAILUE;
        }
        *fft_mode = cxd2856_fft_mode_map_to_ali_fft_mode(pDemod->system, ofdm.mode);
    }
    else
    {
        result = sony_demod_dvbt_monitor_ModeGuard(pDemod, &fft_mode_t, &gi_t);//sony_dvb_demod_monitorT_ModeGuard       
        if (result != SONY_RESULT_OK) 
        {
            return ERR_FAILUE;
        }
        *fft_mode = cxd2856_fft_mode_map_to_ali_fft_mode(pDemod->system, fft_mode_t);
    }
	return SUCCESS;
}

INT32 nim_cxd2856_get_freq(struct nim_device *dev, UINT32 *freq)
{
    sony_integ_t *priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pDemod = priv->pDemod;
    *freq = pDemod->Frequency;
    return SUCCESS;
}
static UINT8 cxd2856_modulation_map_to_ali_modulation(sony_dtv_system_t system, UINT8 modulation)
{
    //T_NODE:	UINT32 modulation : 8;	
    //2:DQPSK 4:QPSK, 16:16 QAM, 64:64 QAM //T2: (64+1):256 QAM

   if (system == SONY_DTV_SYSTEM_DVBT)
    {
        switch (modulation)
        {
            case SONY_DVBT_CONSTELLATION_QPSK:
                return 4;
            case SONY_DVBT_CONSTELLATION_16QAM:
                return 16;
            case SONY_DVBT_CONSTELLATION_64QAM:
                return 64;
            default:
                return 0xFF;   //unknown.
        }
    }
    else
    {
        switch (modulation)
        {
            case SONY_DVBT2_QPSK:
                return 4;
            case SONY_DVBT2_QAM16:
                return 16;
            case SONY_DVBT2_QAM64:
                return 64;
            case SONY_DVBT2_QAM256:
                return (64+1);
            default:
                return 0xFF;   //unknown.
        }
    }
}

INT32 nim_cxd2856_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    //UINT32 flgptn;
    OSAL_ER result = SUCCESS;
	sony_integ_t *priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pDemod = priv->pDemod;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    //sony_dvbt2_ofdm_t ofdm;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    sony_dvbt_tpsinfo_t tps;
    *modulation = 0;
	switch (pDemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{			
		    result =  sony_demod_dvbt_CheckTSLock(pDemod, &lock);//sony_dvb_demodT_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		    result =  sony_demod_dvbt2_CheckTSLock(pDemod, &lock);//sony_dvb_demodT2_CheckTSLock
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			lock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
			break;
    }
    if (result != SONY_RESULT_OK || lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
    {
        return ERR_FAILUE;
    }
    if (pDemod->system == SONY_DTV_SYSTEM_DVBT2) 
    {
        sony_demod_dvbt2_monitor_L1Pre(pDemod, &L1Pre);//sony_dvb_demod_monitorT2_L1Pre
        if (result != SONY_RESULT_OK) 
            return ERR_FAILUE;
        // Get Active PLP information. 
        result = sony_demod_dvbt2_monitor_ActivePLP(pDemod, SONY_DVBT2_PLP_DATA, &plpInfo);//sony_dvb_demod_monitorT2_ActivePLP
        if (result != SONY_RESULT_OK || plpInfo.constell > SONY_DVBT2_QAM256 ) 
            return ERR_FAILUE;
        if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
        {
            /*if (plpInfo.id != pDemod->plp_id)
            {
                CXD2856_LOG(pDemod, "%s(): plp_id=%d, plpInfo.id=%d, error PLP locked.\r\n", __FUNCTION__,  pDemod->plp_id, plpInfo.id);
                return ERR_FAILUE;
            }*/
            *modulation = cxd2856_modulation_map_to_ali_modulation(pDemod->system, plpInfo.constell);
            return SONY_RESULT_OK;
        }
    }
    else
    {
        result = sony_demod_dvbt_monitor_TPSInfo(pDemod, &tps);//sony_dvb_demod_monitorT_TPSInfo
        if (result != SONY_RESULT_OK || tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3 ) 
            return ERR_FAILUE;
        *modulation = cxd2856_modulation_map_to_ali_modulation(pDemod->system, tps.constellation);
    }

    return SUCCESS;
}

INT32 nim_cxd2856_get_SSI(struct nim_device *dev, UINT8 *ssi)
{
	sony_integ_t * priv = NULL;
	//OSAL_ER result = ERR_FAILUE;
	sony_result_t CalcResult = SONY_RESULT_ERROR_OTHER;
	int32_t rfLevel;
	//UINT32 flgptn;
	if(dev == NULL || ssi == NULL)
	{
		return ERR_FAILUE;
	}
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	
	osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	
	//get rfLevel from tuner first
	if (dev->do_ioctl)
	{
		/* Enable the I2C repeater */			
		if(sony_demod_I2cRepeaterEnable (pdemod, 0x01) !=SONY_RESULT_OK) 
		{
			goto ERROR_EXIT;
		}
		NIM_MUTEX_ENTER(pdemod);
		if(pdemod->dvbt_config.nim_tuner_command(pdemod->tuner_id, NIM_TUNER_GET_RF_POWER_LEVEL, (UINT32)&rfLevel) != SUCCESS)//0.01dbm
		{
			NIM_MUTEX_LEAVE(pdemod);
			goto ERROR_EXIT;
		}
		NIM_MUTEX_LEAVE(pdemod);
		/* disable the I2C repeater */
	   	if(sony_demod_I2cRepeaterEnable (pdemod, 0x00) !=SONY_RESULT_OK) 
		{
			goto ERROR_EXIT;
		}
	  
	}else
	{
		goto ERROR_EXIT;
	}
	/*Because the unitt of the tuner rf is different in different code trees,
	   so need to make changes according to each tree,and ulimately need to transfer 0.001dbm  
	*/
	rfLevel = rfLevel *10;//transfer to 0.001dbm

	switch (pdemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{
		    CalcResult = sony_integ_dvbt_monitor_SSI (priv, ssi, rfLevel);//unit of rfLevel is 0.001dbm
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		    CalcResult =  sony_integ_dvbt2_monitor_SSI(priv, ssi, rfLevel);
		    break;
		}
		case SONY_DTV_SYSTEM_ISDBT:
		{
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			break;
	}
	if(SONY_RESULT_OK != CalcResult)
	{
		CXD2856_LOG("error: CalcSSI failure,use pre value 1 !\r\n");
		goto ERROR_EXIT;
	}
	else
	{
		CXD2856_LOG(" SSI                     | Signal Strength | %u\n", *ssi);
		osal_mutex_unlock(pdemod->demodMode_mutex_id);
		return SUCCESS;
		
	}
	
ERROR_EXIT:
	
	*ssi = 0;
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	return SUCCESS;
	
}
INT32 nim_cxd2856_get_SQI(struct nim_device *dev, UINT8 *sqi)
{
	sony_integ_t * priv = NULL;
	//OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;
	//UINT32 flgptn;
	if(dev == NULL || sqi == NULL)
	{
		return ERR_FAILUE;
	}
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	CXD2856_LOG("[%s %d]pdemod->system=%d\n", __FUNCTION__, __LINE__, pdemod->system);
	osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	switch (pdemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{
		    CalcResult = sony_demod_dvbt_monitor_Quality(pdemod,sqi);
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		    CalcResult =  sony_demod_dvbt2_monitor_Quality(pdemod,sqi);
		    break;
		}
		case SONY_DTV_SYSTEM_ISDBT:
		{
			//
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			break;
	}
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	if(SONY_RESULT_OK != CalcResult)
	{
		CXD2856_LOG("error: CalcSQI failure,use pre value!\r\n");
		*sqi = 0;
		
		return SUCCESS;
	}
	else
	{
		CXD2856_LOG(" SQI                     | Signal Strength | %u\n", *sqi);
		return SUCCESS;
	}
		
}
INT32 nim_cxd2856_get_per(struct nim_device *dev, UINT32 *per)
{
	sony_integ_t * priv = NULL;
	//OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;
	//UINT32 flgptn;
	if(dev == NULL || per == NULL)
	{
		return ERR_FAILUE;
	}
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	switch (pdemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{
		    CalcResult = sony_demod_dvbt_monitor_PER(pdemod, (uint32_t *)per);
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		    CalcResult =  sony_demod_dvbt2_monitor_PER(pdemod, (uint32_t *)per);
		    break;
		}
		case SONY_DTV_SYSTEM_ISDBT:
		{
			//
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			break;
	}
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	if(SONY_RESULT_OK != CalcResult)
	{
		CXD2856_LOG("error: CalcPer failure,use pre value!\r\n");
		*per = INVALID_VALUE;
	}
	return SUCCESS;

}

INT32 nim_cxd2856_get_cn(struct nim_device *dev, UINT16 *cn)
{
	sony_integ_t * priv = NULL;
	//OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;
	//UINT32 flgptn;
	INT32 cn_tmp = 0;

	if(dev == NULL || cn == NULL)
	{
		return ERR_FAILUE;
	}
	*cn = 0;
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	switch (pdemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{	
		    CalcResult =  sony_demod_dvbt_monitor_SNR(pdemod, (int32_t *)&cn_tmp);
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
			CalcResult = sony_demod_dvbt2_monitor_SNR(pdemod, (int32_t *)&cn_tmp);//0.001db,23100 = 23.1db
		    break;
		}
		case SONY_DTV_SYSTEM_ISDBT:
		{
			//
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			break;
	}
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	if(SONY_RESULT_OK != CalcResult)
	{
		CXD2856_LOG("error: CalcCn failure,use pre value!\r\n");
		*cn = 0;
		return SUCCESS;
	}
	*cn = cn_tmp / 10;//0.01db,23100 /10= 2310 = 2310*0.01 = 23.1 
	CXD2856_LOG(" cn = %u\n", *cn);
	return SUCCESS;
}
INT32 nim_cxd2856_get_ber(struct nim_device *dev, UINT32 *ber)
{
	sony_integ_t * priv = NULL;
	//OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;
	//UINT32 flgptn;
	
	if(dev == NULL || ber == NULL)
	{
		return ERR_FAILUE;
	}
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	switch (pdemod->system)
	{
		case SONY_DTV_SYSTEM_DVBT:
		{
		    CalcResult = sony_demod_dvbt_monitor_PreRSBER(pdemod,(uint32_t *)ber);
		    break;
		}
		case SONY_DTV_SYSTEM_DVBT2:
		{
		     CalcResult =  sony_demod_dvbt2_monitor_PreBCHBER(pdemod,(uint32_t *)ber);
		    break;
		}
		case SONY_DTV_SYSTEM_ISDBT:
		{
			//
		    break;
		}
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			break;
	}
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	if(SONY_RESULT_OK != CalcResult)
	{
		CXD2856_LOG("error: CalCBer failure,use pre value!\r\n");
		*ber = 0;
		return SUCCESS;
	}
	CXD2856_LOG(" ber = %u\n", *ber);
	return SUCCESS;
}

INT32 nim_cxd2856_get_rf_level(struct nim_device *dev, UINT16 *rf_level)
{
	sony_integ_t * priv = NULL;
	//OSAL_ER result = ERR_FAILUE;
	//sony_result_t CalcResult = SONY_RESULT_ERROR_OTHER;
	int32_t rf_value;
	//UINT32 flgptn;
	if(dev == NULL || rf_level == NULL)
	{
		return ERR_FAILUE;
	}
	priv = (sony_integ_t *)dev->priv;
	sony_demod_t * pdemod = priv->pDemod;
	osal_mutex_lock(pdemod->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	
	//if tuner is mxl608/603,can get rfLevel from tuner 
	if(dev->do_ioctl)
	{
		/* Enable the I2C repeater */			
		if(sony_demod_I2cRepeaterEnable (pdemod, 0x01) !=SONY_RESULT_OK) 
		{
			goto ERROR_EXIT;
		}
		if(dev->do_ioctl(dev,NIM_TUNER_GET_RF_POWER_LEVEL,(UINT32)(&rf_value)) != SUCCESS)//unit is 0.01dbm
		{
			goto ERROR_EXIT;
		}
		/* disable the I2C repeater */
	   	if(sony_demod_I2cRepeaterEnable (pdemod, 0x00) !=SONY_RESULT_OK) 
		{
			goto ERROR_EXIT;
		}
	}
	else
	{
		goto ERROR_EXIT;
	}

	rf_value = rf_value / 10;// unit is 0.1dbm;462 = 462*0.1= 46.2 dbm
	if(rf_value < 0)
	{
		rf_value = 0 - rf_value; //unit is -0.1dbm
	}
	*rf_level = rf_value;
	CXD2856_LOG(" rf = %d\n", *rf_level);
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	return SUCCESS;
	
ERROR_EXIT:
	*rf_level = 0;
	osal_mutex_unlock(pdemod->demodMode_mutex_id);
	return ERR_FAILUE;
	
}
