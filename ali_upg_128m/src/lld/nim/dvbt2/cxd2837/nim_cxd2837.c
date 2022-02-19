/*-----------------------------------------------------------------------------
	GAIA3 Solution:  demodulater CXD3837 + tuner CXD2861.
    History:
    2013/04/18 by JARY.
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <bus/tsi/tsi.h>


#include "nim_cxd2837.h"
#include "sony_demod.h"
#include "sony_demod_integ.h"
#include "sony_demod_dvbt2_monitor.h"
#include "sony_demod_dvbt_monitor.h"

#define BER_TIMEOUT_VAL  5000

#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0]))

#define NIM_FLAG_ENABLE    0x00000100
#define NIM_SCAN_END       0x00000001

sony_i2c_t demodI2c; //I2C interface of DEM.

static UINT8  t2_only =0;//ony search T2 Signal flag
static UINT8  t2_lite_support =0;
BOOL play_program, NeedToInitSystem = FALSE, NeedToConfigTuner = FALSE;

sony_result_t sony_integ_dvbt2_BlindTune(sony_demod_t * pDemod, sony_dvbt2_tune_param_t * pTuneParam, BOOL NeedToConfigTuner, UINT8 t2_lite_support_flag);
sony_result_t sony_integ_CalcBER(sony_demod_t * pDemod, uint32_t *pBER);
sony_result_t sony_integ_CalcSSI(sony_demod_t * pDemod, uint32_t *pSSI, uint32_t rfLevel);
sony_result_t sony_integ_CalcSQI(sony_demod_t * pDemod, uint32_t *pSQI);
sony_result_t sony_integ_demod_CheckTSLock (sony_demod_t * pDemod, sony_demod_lock_result_t * pLock);
const char *FormatResult (sony_result_t result);

void cxd2837_log_i2c(sony_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len)
{
    int i;
    if ( ! (param->output_buffer && param->fn_output_string) )
        return;

    if (write)
        CXD2837_LOG(param, "I2C_Write,0x%02X", (slv_addr&0xFE));
    else
        CXD2837_LOG(param, "I2C_Read,0x%02X", (slv_addr|1));

    for ( i=0; i<len; ++i )
    {
        CXD2837_LOG(param, ",0x%02X", data[i]);
    }

    if (err)
        CXD2837_LOG(param, "\terror");
    CXD2837_LOG(param, "\r\n");
}

static void nim_cxd2837_switch_lock_led(struct nim_device *dev, BOOL On)
{
    if(((sony_demod_t *)dev->priv)->tc.nim_lock_cb)
        ((sony_demod_t *)dev->priv)->tc.nim_lock_cb(On);
}

static INT32 config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 *ptr_lock)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	UINT32 tuner_id = priv->tuner_id;
	UINT8 lock = 0, Tuner_Retry = 0;

	//set_bypass_mode(dev, TRUE);
	do
	{
		Tuner_Retry++;

		if(priv->tc.nim_tuner_control(tuner_id, freq, bandwidth, 0, (UINT8*)&(priv->system),0) == ERR_FAILUE)
		{
			CXD2837_LOG(priv,"Config tuner failed, I2c failed!\r\n");
		}

		if(priv->tc.nim_tuner_status(tuner_id, &lock) != SUCCESS)
		{
			//if i2c read failure, no lock state can be report.
			lock = 0;
			CXD2837_LOG(priv,"Config tuner failed, I2c failed!\r\n");
		}
		CXD2837_LOG(priv,"Tuner Lock Times=0x%d,Status=0x%d\r\n",Tuner_Retry,lock);

		if(Tuner_Retry > 5)
			break;
	}while(0 == lock);

	if(ptr_lock != NULL)
		*ptr_lock = lock;

	if(Tuner_Retry > 5)
	{
		CXD2837_LOG(priv,"ERROR! Tuner Lock Fail\r\n");
		return ERR_FAILUE;
	}
	return SUCCESS;
}

static BOOL need_to_config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth)
{
    sony_demod_t * param = (sony_demod_t *)dev->priv;
    //UINT32 tuner_id = param->tuner_id;
    UINT8 lock = 0;
	nim_cxd2837_get_lock(dev, &lock);
		
    return ! ( param->Frequency == freq \
            && param->bandwidth == bandwidth \
            && lock);
}

static BOOL need_to_lock_DVBT_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para, BOOL NeedToConfigTuner)
{
    sony_demod_t * param = (sony_demod_t *)dev->priv;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

    if ( change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return FALSE;

    //USAGE_TYPE_CHANCHG: for play program.
    //if (change_para->t2_signal) //Do nothing for play DVB-T2 program.
    //    return FALSE;
    //if (param->system != SONY_DTV_SYSTEM_DVBT) //DEM is not be DVB-T mode now.
    //    return TRUE;
    if (param->priority != change_para->priority) //DVB-T Hierarchy mode: HP/LP.
        return TRUE;

    return NeedToConfigTuner;
}

static INT32 need_to_lock_DVBT2_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para,
	BOOL NeedToConfigTuner, BOOL *p_play_program)
{
    sony_demod_t * param = (sony_demod_t *)dev->priv;
    *p_play_program = FALSE;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE \
        || change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

   //USAGE_TYPE_CHANCHG: for play program.
   // if ( !change_para->t2_signal ) //Do nothing for play DVB-T program.
   //     return FALSE;

    *p_play_program = TRUE;
    //if (param->system != SONY_DTV_SYSTEM_DVBT2) //DEM is not be DVB-T2 mode now.
    //    return TRUE;

    if (param->plp_index != change_para->plp_index || param->plp_id != change_para->plp_id) //Current selected PLP is different with the target PLP.
        return TRUE;

    if (param->t2_profile != change_para->t2_profile) //DVB-T2 profile: base or lite.
        return TRUE;
	 
    return NeedToConfigTuner;
}

static INT32 try_to_lock_DVBT_signal(struct nim_device *dev, BOOL NeedToInitSystem,
	BOOL NeedToConfigTuner, struct NIM_CHANNEL_CHANGE *change_para)
{

	sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
	sony_dvbt_tune_param_t TuneParam;
	sony_result_t tuneResult;
	//sony_result_t result;
	//sony_demod_lock_result_t  demod_lock;

    if(SONY_DEMOD_BW_1_7_MHZ == change_para->bandwidth)    
    {        
        PRINTF("DVB-T not support 1.7MHZ!\n");
        return ERR_FAILUE;
    }
	memset(&TuneParam,0,sizeof(TuneParam));
	TuneParam.centerFreqKHz = change_para->freq;
	TuneParam.bandwidth = change_para->bandwidth;
	TuneParam.profile = change_para->priority; //SONY_DVBT_PROFILE_HP;

	if( NeedToInitSystem || (pDemod->system != SONY_DTV_SYSTEM_DVBT))
	{
		pDemod->t2_signal = 2;
		//to do...
	}

	tuneResult = sony_integ_dvbt_Tune(pDemod, &TuneParam,NeedToConfigTuner);
	switch(tuneResult){
	case SONY_RESULT_OK:
		//(("DVB-T TS Locked.\n"));
		return SUCCESS;
	case SONY_RESULT_ERROR_UNLOCK:
		//(("DVB-T TS Unlocked.\n"));
		return ERR_FAILUE;
	case SONY_RESULT_ERROR_TIMEOUT:
		//(("DVB-T Wait TS Lock but Timeout.\n"));
		return ERR_FAILUE;
	default:
		//(("Error in sony_integ_dvbt_Tune (%s)\n", sony_FormatResult(sony_result)));
		return ERR_FAILUE;
	}
}

static INT32 DVBT2_change_PLP(struct nim_device *dev, UINT8 plp_id, BOOL NeedToInitSystem, BOOL NeedToConfigTuner, BOOL AUTOSCAN)
{
	sony_result_t tuneResult = SONY_RESULT_OK;

	sony_demod_t* pDemod = (sony_demod_t *)dev->priv;
	pDemod->t2_signal = 1;
	pDemod->plp_id = plp_id;

	sony_dvbt2_tune_param_t tuneParam;
	memset(&tuneParam,0,sizeof(tuneParam));
	tuneParam.centerFreqKHz = pDemod->Frequency;
	tuneParam.bandwidth= pDemod->bandwidth;
	tuneParam.dataPlpId = plp_id; //selected data PLP ID. 
	tuneParam.profile = pDemod->t2_profile;
	tuneParam.tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_OK;

	CXD2837_LOG(pDemod,"[%s]:PLP_id(%d),autoScan(%d).\n",__FUNCTION__,plp_id,AUTOSCAN);
	if(AUTOSCAN==0)
	{
		tuneResult = sony_integ_dvbt2_Tune(pDemod, &tuneParam,NeedToConfigTuner);
	}
	else if(AUTOSCAN==1)
	{
		tuneResult = sony_integ_dvbt2_BlindTune(pDemod, &tuneParam,NeedToConfigTuner,t2_lite_support);	//By SONY AEC 20130701
	}
	switch(tuneResult){
        case SONY_RESULT_OK:
	        CXD2837_LOG(pDemod,"[%s]:DVB-T2 TS Locked.\n",__FUNCTION__);
	        return SUCCESS;
        case SONY_RESULT_ERROR_UNLOCK:
	        CXD2837_LOG(pDemod,"[%s]:DVB-T2 TS Unlocked.\n",__FUNCTION__);
	        return ERR_FAILUE;
        case SONY_RESULT_ERROR_TIMEOUT:
            CXD2837_LOG(pDemod,"[%s]:DVB-T2 Wait TS Lock but Timeout.\n",__FUNCTION__);
	        return ERR_FAILUE;
        default:
	        CXD2837_LOG(pDemod,"[%s]:Error in sony_integ_dvbt2_Tune (%d)\n",__FUNCTION__,tuneResult);
	        return ERR_FAILUE;
    }

}

static INT32 try_to_lock_next_data_plp(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
    INT32 result = ERR_FAILUE;
    sony_demod_t * param = (sony_demod_t *)dev->priv;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    UINT8 retry = 0; 
    UINT8 plp_index = 0; 

    for (plp_index = param->plp_index + 1; plp_index < param->plp_num; ++plp_index )
    {
        if (plp_index >= param->plp_num)
            return ERR_FAILUE;
        result = DVBT2_change_PLP(dev, param->all_plp_id[plp_index], FALSE, FALSE, 0);
        
        for ( retry=0; retry<30; ++retry )
        {
            SONY_SLEEP (30);
            if (param->autoscan_stop_flag)
                return SONY_RESULT_OK;            
            if (param->do_not_wait_t2_signal_locked)
                return SONY_RESULT_OK;
            
            result = sony_demod_dvbt2_monitor_L1Pre(param, &L1Pre);//sony_dvb_demod_monitorT2_L1Pre
            if (result != SONY_RESULT_OK) 
            {
                CXD2837_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d) error: sony_dvb_demod_monitorT2_L1Pre()=%d \r\n", 
					__FUNCTION__, NeedToInitSystem, NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], (int)result);
                continue;
				//return result;
            }
        
            // Get Active PLP information. 
            result = sony_demod_dvbt2_monitor_ActivePLP(param, SONY_DVBT2_PLP_DATA, &plpInfo);//sony_dvb_demod_monitorT2_ActivePLP
            if (result != SONY_RESULT_OK) 
            {
				CXD2837_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d) error: sony_dvb_demod_monitorT2_ActivePLP()=%d \r\n", 
					__FUNCTION__, NeedToInitSystem, NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], (int)result);
				continue;
				//return result;
            }
            
            if (result == SONY_RESULT_OK) 
            {
                if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
                {
                    if (plpInfo.id != param->all_plp_id[plp_index])
                    {
                        CXD2837_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), plpInfo.id=%d, error PLP locked: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], plpInfo.id, retry);
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
                    CXD2837_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
                return SONY_RESULT_OK;
            }
            else
            {
                    CXD2837_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times. error: Not DataPLP: (type=%d, id=%d)\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry, plpInfo.type, plpInfo.id);
            }
        }
        else
        {
            CXD2837_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), error: fail to lock the PLP.\r\n",
				__FUNCTION__, (int)NeedToInitSystem, (int)NeedToConfigTuner, (int)param->plp_num, (int)plp_index, (int)param->all_plp_id[plp_index]);
        }
    }
    return ERR_FAILUE;
}

static INT32 try_to_lock_DVBT2_signal(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{	
	UINT16 waitTime = 0;
	//UINT8 plp_index = 0; 
	//UINT8 retry = 0; 

	INT32 result = ERR_FAILUE;
	sony_demod_t * param = (sony_demod_t *)dev->priv;
	//sony_dvbt2_l1pre_t L1Pre;
	//sony_dvbt2_plp_t plpInfo;

	param->plp_num = 0;
	param->plp_id = 0;
	param->plp_index = 0;
        
	result = DVBT2_change_PLP(dev, 0, NeedToInitSystem, NeedToConfigTuner,1); //for lock signal first.
  
	//get_the_first_data_PLP_info
	do
	{
		result = sony_demod_dvbt2_monitor_DataPLPs(param, param->all_plp_id, &(param->plp_num));//sony_dvb_demod_monitorT2_DataPLPs
		if (result == SONY_RESULT_OK) 
		{
			UINT8 plp_idx;
			CXD2837_LOG(param, "\t[%s]: plp_num=%d\n ", __FUNCTION__, param->plp_num);
			for (plp_idx=0; plp_idx < param->plp_num; ++plp_idx)
			{
				CXD2837_LOG(param, "\t[plp_id=%d]\n", param->all_plp_id[plp_idx]);
			}
			param->plp_id = param->all_plp_id[0];//get first plp_id
			break;
		}
		else if (result == SONY_RESULT_ERROR_HW_STATE) 
		{
			if (waitTime >= DTV_DEMOD_TUNE_T2_L1POST_TIMEOUT)
			{
				CXD2837_LOG(param, "%s() error: timeout for get the first data_PLP\r\n", __FUNCTION__);
				param->plp_num = 0;
				return ERR_FAILUE;
			}
			else 
			{
				SONY_SLEEP (DEMOD_TUNE_POLL_INTERVAL); //10
				waitTime += DEMOD_TUNE_POLL_INTERVAL; 
			}
		} 
		else 
		{
			CXD2837_LOG(param, "%s()=%d error: Fail to get the first data_PLP\r\n", __FUNCTION__, (int)result);
			param->plp_num = 0;
			return ERR_FAILUE; // Other (fatal) error.
		}
	}while (1);
    
	return result;
}
static INT32 nim_cxd2837_lock_T_signal(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_demod_t * param = (sony_demod_t *)dev->priv;
	if ((!t2_only) && (need_to_lock_DVBT_signal(dev, change_para, NeedToConfigTuner)))
	{
		param->priority = change_para->priority;
		return try_to_lock_DVBT_signal(dev, NeedToInitSystem, NeedToConfigTuner, change_para);
	}
	else
	{
		CXD2837_LOG(param, "error: [%s][%d]para repetition\r\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}
}
static INT32 nim_cxd2837_lock_T2_signal(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_demod_t * param = (sony_demod_t *)dev->priv;
	INT32 result = ERR_FAILUE;
	if (need_to_lock_DVBT2_signal(dev, change_para, NeedToConfigTuner, &play_program))
	{
		if (play_program)
		{
			param->do_not_wait_t2_signal_locked = 1;
			param->plp_index = change_para->plp_index;
			param->t2_profile = change_para->t2_profile;
			result = DVBT2_change_PLP(dev, change_para->plp_id, NeedToInitSystem, NeedToConfigTuner,0);
		}
		else
		{
			if (change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN)
			{
				result = try_to_lock_next_data_plp(dev, NeedToInitSystem, NeedToConfigTuner);
			}
			else
			{
				result = try_to_lock_DVBT2_signal(dev, NeedToInitSystem, NeedToConfigTuner);
				change_para->plp_num = param->plp_num;
				change_para->t2_profile = param->t2_profile;
			}
			change_para->plp_index = param->plp_index;
			change_para->plp_id = param->plp_id;
			change_para->t2_system_id = param->t2_system_id;
		}
	}
	else
	{
		CXD2837_LOG(param, "error: [%s][%d]para repetition\r\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}
	return result;
}
static INT32 nim_cxd2837_lock_combo_signal(struct nim_device *dev,struct NIM_CHANNEL_CHANGE *change_para)
{
	sony_demod_t * param = (sony_demod_t *)dev->priv;
	param->do_not_wait_t2_signal_locked = ( change_para->usage_type == USAGE_TYPE_AERIALTUNE ? 1:0 );
	do
	{
		//first judge the t2_only for ui_app
		if(1 == t2_only) //use only t2 to compatible ui
		{
			if (nim_cxd2837_lock_T2_signal(dev,change_para) == SUCCESS)
			{
				break;
			}		
		}else if( 2 == t2_only)
		{
			if (nim_cxd2837_lock_T_signal(dev,change_para) == SUCCESS)
			{
				break;
			}	
		}
		//here judge the change_para->t2_signa for aui_app
		if(2 == change_para->t2_signal)   //use change_para->t2_signal  to compatible aui
		{
			if (nim_cxd2837_lock_T_signal(dev,change_para) == SUCCESS)
			{			
				break;
			}
		}
		else if(1 == change_para->t2_signal)
		{
			if (nim_cxd2837_lock_T2_signal(dev,change_para) == SUCCESS)
			{
				break;
			}
		}
		else  //first deal with T2 signal when unknown work mode and no t2_only no change_para->t2_signal;
		{
			if (nim_cxd2837_lock_T2_signal(dev,change_para) == SUCCESS)
			{
				break;
			}
			if (param->autoscan_stop_flag)
			{
			    return ERR_FAILUE;
			}
			if (nim_cxd2837_lock_T_signal(dev,change_para) == SUCCESS)
			{
				break;
			}
			else
			{
				CXD2837_LOG(param,"signal try lock fail%s %d\n",__FUNCTION__,__LINE__);
				return ERR_FAILUE;
			}
		}

	}while(0);
	return SUCCESS;	
}
static INT32 nim_cxd2837_channel_change_smart(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{	
	INT32 result = ERR_FAILUE;
	sony_demod_t * param = (sony_demod_t *)dev->priv;
	UINT32 flgptn = 0;
	if ((change_para->freq <= 40000) || (change_para->freq >= 900000))
	{
		return ERR_FAILUE;
	}
	if (change_para->bandwidth != SONY_DEMOD_BW_1_7_MHZ && change_para->bandwidth != SONY_DEMOD_BW_5_MHZ
        && change_para->bandwidth != SONY_DEMOD_BW_6_MHZ && change_para->bandwidth != SONY_DEMOD_BW_7_MHZ
        && change_para->bandwidth != SONY_DEMOD_BW_8_MHZ)
	{
		return ERR_FAILUE;
	}

	result = osal_flag_wait(&flgptn, param->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 2000); //OSAL_WAIT_FOREVER_TIME
	if(OSAL_E_OK != result)
	{
		return ERR_FAILUE;
	}

	CXD2837_LOG(param, "%s: usage_type %d, freq %d, bandwidth %d, priority %d, t2_signal %d, plp_index %d, plp_id  %d, profile %d\r\n",
                __FUNCTION__, (int)change_para->usage_type, (int)change_para->freq, (int)change_para->bandwidth, (int)change_para->priority, 
                (int)change_para->t2_signal, (int)change_para->plp_index, (int)change_para->plp_id, (int)change_para->t2_profile);
	osal_mutex_lock(param->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
	param->autoscan_stop_flag = 0;
	if ( need_to_config_tuner(dev, change_para->freq, change_para->bandwidth) )
	{
		if(param->bandwidth != change_para->bandwidth)
			NeedToInitSystem = TRUE;
		param->Frequency = change_para->freq;
		param->bandwidth = change_para->bandwidth;
		NeedToConfigTuner = TRUE;
	}
	CXD2837_LOG(param,"%s %d the sys signal = %d\n",__FUNCTION__,__LINE__,param->sys_signal_type);
	CXD2837_LOG(param,"%s %d the current t2_signal = %d\n",__FUNCTION__,__LINE__,change_para->t2_signal);
	switch(param->sys_signal_type)
	{
		case 1: 
			result = nim_cxd2837_lock_T_signal(dev,change_para);
			break;
		case 2:
			result = nim_cxd2837_lock_T2_signal(dev,change_para);
			break;
		case 3: 
			result = nim_cxd2837_lock_combo_signal(dev,change_para); //unknown work mode
			break;
		default:
			break;
	}
    change_para->t2_signal = param->t2_signal; //current signal type transfer to app
   CXD2837_LOG(param,"%s %d last sys signal = %d\n",__FUNCTION__,__LINE__,param->sys_signal_type);
    osal_mutex_unlock(param->demodMode_mutex_id);
    osal_flag_set(param->flag_id,NIM_SCAN_END);
    return result;
}

static UINT8 cxd2837_modulation_map_to_ali_modulation(sony_dtv_system_t system, UINT8 modulation)
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

static INT32 nim_cxd2837_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    //sony_dvbt2_ofdm_t ofdm;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    sony_dvbt_tpsinfo_t tps;
    *modulation = 0;
	
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    switch (pDemod->system) {
	    case SONY_DTV_SYSTEM_DVBT2:
	        result = sony_demod_dvbt2_CheckDemodLock(pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBT:
	        result = sony_demod_dvbt_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBC:
	        result = sony_demod_dvbc_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_UNKNOWN:
	    default:
	        result = SONY_RESULT_OK;
    }
    if (result != SONY_RESULT_OK || lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

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
            if (plpInfo.id != pDemod->plp_id)
            {
                CXD2837_LOG(pDemod, "%s(): plp_id=%d, plpInfo.id=%d, error PLP locked.\r\n", __FUNCTION__,  pDemod->plp_id, plpInfo.id);
                return ERR_FAILUE;
            }

            *modulation = cxd2837_modulation_map_to_ali_modulation(pDemod->system, plpInfo.constell);
            return SONY_RESULT_OK;
        }
    }
    else
    {
        result = sony_demod_dvbt_monitor_TPSInfo(pDemod, &tps);//sony_dvb_demod_monitorT_TPSInfo
        if (result != SONY_RESULT_OK || tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3 ) 
            return ERR_FAILUE;
        *modulation = cxd2837_modulation_map_to_ali_modulation(pDemod->system, tps.constellation);
    }

    return SUCCESS;
}


static UINT8 cxd2837_FEC_map_to_ali_FEC(sony_dtv_system_t system, UINT8 fec)
{
    //T_NODE:	UINT16 FEC_inner			: 4;
    //T: 0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:7/8  //T2: 5:3/5, 6:4/5 //0xF:unknown
    
   if (system == SONY_DTV_SYSTEM_DVBT)
    {
        switch (fec)
        {
            case SONY_DVBT_CODERATE_1_2:
                return 0;
            case SONY_DVBT_CODERATE_2_3:
                return 1;
            case SONY_DVBT_CODERATE_3_4:
                return 2;
            case SONY_DVBT_CODERATE_5_6:
                return 3;
            case SONY_DVBT_CODERATE_7_8:
                return 4;
            default:
                return 0xF;   //unknown.
        }
    }
    else
    {
        switch (fec)
        {
			case SONY_DVBT2_R1_2:
				return 0;
			case SONY_DVBT2_R2_3:
				return 1;
			case SONY_DVBT2_R3_4:
				return 2;
			case SONY_DVBT2_R5_6:
				return 3;
			case SONY_DVBT2_R3_5:
				return 5;
			case SONY_DVBT2_R4_5:
				return 6;
			case SONY_DVBT2_R1_3: 
				return 7;
			case SONY_DVBT2_R2_5:
				return 8;
			default:
			return 0xF;   //unknown.
        }
    }
}

static INT32 nim_cxd2837_get_FEC(struct nim_device *dev, UINT8* FEC)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    //sony_dvbt2_ofdm_t ofdm;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    sony_dvbt_tpsinfo_t tps;

    *FEC = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    switch (pDemod->system) {
	    case SONY_DTV_SYSTEM_DVBT2:
	        result = sony_demod_dvbt2_CheckDemodLock(pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBT:
	        result = sony_demod_dvbt_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBC:
	        result = sony_demod_dvbc_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_UNKNOWN:
	    default:
	        result = SONY_RESULT_OK;
    }
    if (result != SONY_RESULT_OK || lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

    if (pDemod->system == SONY_DTV_SYSTEM_DVBT2) 
    {
        sony_demod_dvbt2_monitor_L1Pre(pDemod, &L1Pre);//sony_dvb_demod_monitorT2_L1Pre
        if (result != SONY_RESULT_OK) 
            return ERR_FAILUE;
    
        // Get Active PLP information. 
        result = sony_demod_dvbt2_monitor_ActivePLP(pDemod, SONY_DVBT2_PLP_DATA, &plpInfo);//sony_dvb_demod_monitorT2_ActivePLP
        if (result != SONY_RESULT_OK) 
            return ERR_FAILUE;

        if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
        {
            if (plpInfo.id != pDemod->plp_id)
            {
                CXD2837_LOG(pDemod, "%s(): plp_id=%d, plpInfo.id=%d, error PLP locked.\r\n", __FUNCTION__,  pDemod->plp_id, plpInfo.id);
                return ERR_FAILUE;
            }

            *FEC = cxd2837_FEC_map_to_ali_FEC(pDemod->system, plpInfo.plpCr);
            return SUCCESS;
        }
    }
    else
    {
        result = sony_demod_dvbt_monitor_TPSInfo(pDemod, &tps);//sony_dvb_demod_monitorT_TPSInfo
        if (result != SONY_RESULT_OK || tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3 ) 
            return ERR_FAILUE;

        *FEC = cxd2837_FEC_map_to_ali_FEC(pDemod->system, tps.rateHP);
    }
   
    return SUCCESS;
}

static UINT8 cxd2837_gi_map_to_ali_gi(sony_dtv_system_t system, UINT8 guard_interval)
{
    //T_NODE:	UINT32 guard_interval : 8; 	
    //4: 1/4, 8: 1/8, 16: 1/16, 32:1/32  //T2: 128:1/128, (19+128):19/128, 19:19/256

    if (system == SONY_DTV_SYSTEM_DVBT)
    {
        switch (guard_interval)
        {
            case SONY_DVBT_GUARD_1_32:
                return 32;
            case SONY_DVBT_GUARD_1_16:
                return 16;
            case SONY_DVBT_GUARD_1_8:
                return 8;
            case SONY_DVBT_GUARD_1_4:
                return 4;
            default:
                return 0xFF;   //unknown.
        }
    }
    else
    {
        switch (guard_interval)
        {
            case SONY_DVBT2_G1_32:
                return 32;
            case SONY_DVBT2_G1_16:
                return 16;
            case SONY_DVBT2_G1_8:
                return 8;
            case SONY_DVBT2_G1_4:
                return 4;
            case SONY_DVBT2_G1_128:
                return 128;
            case SONY_DVBT2_G19_128:
                return (19+128);
            case SONY_DVBT2_G19_256:
                return 19;
            default:
                return 0xFF;   //unknown.
        }
    }
}

static INT32 nim_cxd2837_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_dvbt2_ofdm_t ofdm;
    sony_dvbt_mode_t fft_mode_t;
    sony_dvbt_guard_t gi_t;
    
    *guard_interval = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    switch (pDemod->system) {
	    case SONY_DTV_SYSTEM_DVBT2:
	        result = sony_demod_dvbt2_CheckDemodLock(pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBT:
	        result = sony_demod_dvbt_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBC:
	        result = sony_demod_dvbc_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_UNKNOWN:
	    default:
	        result = SONY_RESULT_OK;
    }
    if (result != SONY_RESULT_OK || lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;


    if (pDemod->system == SONY_DTV_SYSTEM_DVBT2) 
    {
        result = sony_demod_dvbt2_monitor_Ofdm(pDemod, &ofdm);//sony_dvb_demodT2_OptimizeMISO
        if (result != SONY_RESULT_OK) 
        {
            return ERR_FAILUE;
        }
        *guard_interval = cxd2837_gi_map_to_ali_gi(pDemod->system, ofdm.gi);
    }
    else
    {
        result = sony_demod_dvbt_monitor_ModeGuard(pDemod, &fft_mode_t, &gi_t);//sony_dvb_demod_monitorT_ModeGuard       
        if (result != SONY_RESULT_OK) 
        {
            return ERR_FAILUE;
        }
        *guard_interval = cxd2837_gi_map_to_ali_gi(pDemod->system, gi_t);;
    }

   	return SUCCESS;
}


static UINT8 cxd2837_fft_mode_map_to_ali_fft_mode(sony_dtv_system_t system, UINT8 fft_mode)
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

static INT32 nim_cxd2837_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_dvbt2_ofdm_t ofdm;
    sony_dvbt_mode_t fft_mode_t;
    sony_dvbt_guard_t gi_t;
  
    *fft_mode = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    switch (pDemod->system) {
	    case SONY_DTV_SYSTEM_DVBT2:
	        result = sony_demod_dvbt2_CheckDemodLock(pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBT:
	        result = sony_demod_dvbt_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_DVBC:
	        result = sony_demod_dvbc_CheckDemodLock (pDemod, &lock);
	        break;
	    case SONY_DTV_SYSTEM_UNKNOWN:
	    default:
	        result = SONY_RESULT_OK;
    }
    if (result != SONY_RESULT_OK || lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

    if (pDemod->system == SONY_DTV_SYSTEM_DVBT2) 
    {
        result = sony_demod_dvbt2_monitor_Ofdm(pDemod, &ofdm);//sony_dvb_demodT2_OptimizeMISO
        if (result != SONY_RESULT_OK) 
        {
            return ERR_FAILUE;
        }
        *fft_mode = cxd2837_fft_mode_map_to_ali_fft_mode(pDemod->system, ofdm.mode);
    }
    else
    {
        result = sony_demod_dvbt_monitor_ModeGuard(pDemod, &fft_mode_t, &gi_t);//sony_dvb_demod_monitorT_ModeGuard       
        if (result != SONY_RESULT_OK) 
        {
            return ERR_FAILUE;
        }
        *fft_mode = cxd2837_fft_mode_map_to_ali_fft_mode(pDemod->system, fft_mode_t);
    }

	return SUCCESS;
}

static INT32 nim_cxd2837_get_freq(struct nim_device *dev, UINT32 *freq)
{
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    *freq = priv->Frequency;
    return SUCCESS;
}

static INT32 nim_cxd2837_channel_change(
	struct nim_device *dev, 
	UINT32 freq, 
	UINT32 bandwidth, 
	UINT8 guard_interval, 
	UINT8 fft_mode, 
	UINT8 modulation, 
	UINT8 fec, 
	UINT8 usage_type, 
	UINT8 inverse,
	UINT8 priority)
{
	struct NIM_CHANNEL_CHANGE change_para;

	MEMSET(&change_para, 0, sizeof(struct NIM_CHANNEL_CHANGE));

	change_para.freq = freq;

	change_para.bandwidth = bandwidth;

	change_para.guard_interval = guard_interval;
		
	change_para.fft_mode = fft_mode;
		
	change_para.modulation = modulation;

	change_para.fec = fec;

	change_para.usage_type = usage_type;

	change_para.inverse = inverse;

	change_para.inverse = priority;		
	
	return nim_cxd2837_channel_change_smart(dev, &change_para);
}

static INT32 nim_cxd2837_get_SSI(struct nim_device *dev, UINT8 *data)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;
	uint32_t  ssi=0;
	static uint32_t PreSSI =0;

	uint32_t rfLevel;
	UINT32 flgptn;
	
	result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);

	if(OSAL_E_OK==result)
	{
		osal_mutex_lock(priv->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
		//!get rfLevel first
		if ( nim_cxd2837_ioctl(dev, NIM_TUNER_GET_RF_POWER_LEVEL, (UINT32)&rfLevel) == ERR_FAILUE )
		{
			CXD2837_LOG(priv, "error: Tuner does not support command 'NIM_TUNER_GET_RF_POWER_LEVEL'.\r\n"); 
			osal_mutex_unlock(priv->demodMode_mutex_id);
			return ERR_FAILUE;
		}
		
#if (SYS_TUN_MODULE == CXD2872)	//modify according to real test									
		if (rfLevel<-90000)
		{
			rfLevel += 2000;
		}
		else if (rfLevel<-80000)
		{
			rfLevel += 1000;
		}
		else if (rfLevel<-70000)
		{
			rfLevel += 500;
		}
		else if (rfLevel<-60000)
		{
			rfLevel += 5000;
		}
		else if (rfLevel<-50000)
		{
			rfLevel += 2000;
		}
		else if (rfLevel<-40000)
		{
			rfLevel += 1000;
		}
		else
		{
		}
#endif
		
#if (SYS_TUN_MODULE == MXL603)//modify for mxl603 tuner	AGC display
		if (rfLevel > -4000)
		{
			rfLevel = rfLevel*10 + 2000;
		}
		else if (rfLevel > -9000)
		{
			rfLevel = rfLevel*10 + 3500;
		}		
		else
		{
			rfLevel = rfLevel*10 + 1500;
		}
#endif

		//!Get SSI Value
		CalcResult = sony_integ_CalcSSI(priv, &ssi, rfLevel);
		if(SONY_RESULT_OK == CalcResult){
			*data = ssi;
			PreSSI = ssi;//save this data
			//CXD2837_LOG(priv, "CalcSSI: ssi=%d\r\n",ssi); 
		}
		else{
			*data = PreSSI;//use the pre-value
			PreSSI = PreSSI/2;
			CXD2837_LOG(priv, "error: CalcSSI failure,use pre value!\r\n");
		}
		osal_mutex_unlock(priv->demodMode_mutex_id);
		return SUCCESS;
	}
	else
	{
		*data = 0;
	}
	return ERR_FAILUE;
}

static INT32 nim_cxd2837_get_SQI(struct nim_device *dev, UINT8 *snr)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;
	uint32_t  quality=0;
	static uint32_t PreQuality =0;

	UINT32 flgptn;
	
	result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
	if(OSAL_E_OK==result)
	{
		osal_mutex_lock(priv->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
		CalcResult = sony_integ_CalcSQI(priv, &quality);
		if(SONY_RESULT_OK == CalcResult){
			*snr = quality;
			PreQuality = quality;//save this data
			//CXD2837_LOG(priv, "CalcSQI: sqi=%d\r\n",quality);
		}
		else{
			*snr = PreQuality;//use the pre-value
			PreQuality = PreQuality/2;
			CXD2837_LOG(priv, "error: CalcSQI failure,use pre value!\r\n");
		}
		osal_mutex_unlock(priv->demodMode_mutex_id);
		return SUCCESS;
	}
	else
	{
		*snr = 0;
	}
	return ERR_FAILUE;
}

static INT32 nim_cxd2837_get_BER(struct nim_device *dev, UINT32 *BER)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	OSAL_ER result;
	sony_result_t CalcResult = SONY_RESULT_OK;

	uint32_t  ber;
	static uint32_t PreBer;
	UINT32 flgptn;
	
	result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
	if(OSAL_E_OK==result)
	{
		osal_mutex_lock(priv->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
		CalcResult = sony_integ_CalcBER(priv, &ber);
		if(SONY_RESULT_OK == CalcResult){
			*BER = ber;
			PreBer = ber;
			//CXD2837_LOG(priv, "CalcBER: BER=%d\r\n",ber);
		}
		else{
			*BER = PreBer;
			PreBer =  PreBer/2;
			CXD2837_LOG(priv, "error: CalcBER failure, use pre value!\r\n");
		}
		osal_mutex_unlock(priv->demodMode_mutex_id);
		return SUCCESS;
	}
	else
	{
		*BER = 0;
	}
	return ERR_FAILUE;
}

INT32 nim_cxd2837_get_lock(struct nim_device *dev, UINT8 *lock)
{
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    //UINT8 data=0;
    INT32 i;
    OSAL_ER result;
    UINT32 flgptn;
    sony_demod_lock_result_t demod_lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;//DEMOD_LOCK_RESULT_NOTDETECT

    *lock = 0;
    for ( i=0; i<1; ++i)
    {
        result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
        if(OSAL_E_OK != result)
        {
            //CXD2837_LOG(priv, "%s: Unlock. osal_flag_wait() fail.\r\n", __FUNCTION__);
            break;
        }
        osal_mutex_lock(priv->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
        sony_integ_demod_CheckTSLock(priv, &demod_lock);
        osal_mutex_unlock(priv->demodMode_mutex_id);
        if ( SONY_DEMOD_LOCK_RESULT_LOCKED == demod_lock )
        {
            *lock = 1;
            break;
        }
    }

    //put light the panel lock here. if *lock=ff, should not light on the led.
    nim_cxd2837_switch_lock_led(dev, (*lock)?TRUE:FALSE);
    return SUCCESS;
}

static INT32 nim_cxd2837_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    //INT32 freq_offset;
    //UINT8 data[2];
    INT32 ret_val = SUCCESS;

    if(cmd & NIM_TUNER_COMMAND)
    {
        if(NIM_TUNER_CHECK == cmd)
            return SUCCESS;

        if(priv->tc.nim_tuner_command != NULL)
        {	
        	if((UINT32)cmd == NIM_TUNER_SET_STANDBY_CMD)
        	{
        		ret_val = priv->tc.nim_tuner_command(priv->tuner_id,NIM_TUNER_POWER_CONTROL, TRUE);
        	}
			else
			{
				ret_val = priv->tc.nim_tuner_command(priv->tuner_id, cmd, param);
			}
        }
        else
            ret_val = ERR_FAILUE;
        return ret_val;
    }

    switch( cmd )
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            priv->autoscan_stop_flag = param;
            break;

        case NIM_DRIVER_DISABLE_DEBUG_LOG:
            param = (UINT32)NULL;
        case NIM_DRIVER_ENABLE_DEBUG_LOG:
            if(NULL == priv->output_buffer)
            {
                priv->output_buffer = (char *)MALLOC(LOG_STRING_BUFFER_SIZE);
                ASSERT(priv->output_buffer != NULL);
            }
            priv->fn_output_string = (LOG_STRING_FUNCTION)param;
            break;

        case NIM_DRIVER_DISABLED:
            break;
        case NIM_DRIVER_GET_AGC:
            return nim_cxd2837_get_SSI(dev, (UINT8 *)param);						
            break;
        case NIM_DRIVER_GET_BER:
            return nim_cxd2837_get_BER(dev, (UINT32 *)param);
            break;
        case NIM_DRIVER_GET_GUARD_INTERVAL:
            return nim_cxd2837_get_GI(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_FFT_MODE:
            return nim_cxd2837_get_fftmode(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_MODULATION:
            return nim_cxd2837_get_modulation(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_DEMOD_LOCK_MODE:
		*(UINT8*)param = TRUE;
			break;
		case NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY:
			t2_only = (UINT8)param;
			break;
		case NIM_DRIVER_T2_LITE_ENABLE:
			t2_lite_support =(UINT8)param;
			break;
        case NIM_DRIVER_GET_SPECTRUM_INV:
            break;
        default:
            ret_val = ERR_FAILUE;
            break;
    }

    return ret_val;
}

static INT32 nim_cxd2837_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
	struct NIM_CHANNEL_CHANGE *change_para;

	switch (cmd)
	{
        case NIM_DRIVER_CHANNEL_CHANGE:
            change_para = (struct NIM_CHANNEL_CHANGE *)param_list;
            return nim_cxd2837_channel_change_smart(dev, change_para);
            break;
        default:
            break;
	}
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_cxd2834_close(struct nim_device *dev)
* Description: cxd2834 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_cxd2837_close(struct nim_device *dev)
{
    //UINT8 data;
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    INT32 result = SUCCESS;

    // tuner power off
    nim_cxd2837_ioctl(dev, NIM_TUNER_POWER_CONTROL, TRUE);

    result = sony_demod_Shutdown(priv);//sony_dvb_demod_Finalize
    if (result != SONY_RESULT_OK)
        result = ERR_FAILUE;

    nim_cxd2837_switch_lock_led(dev, FALSE);
    return result;
}

__ATTRIBUTE_REUSE_
static INT32 nim_cxd2837_open(struct nim_device *dev)
{
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    //struct COFDM_TUNER_CONFIG_API * config_info = &(priv->tc);
    //sony_dvbt2_tune_param_t  TuneParam;
    sony_result_t result = SONY_RESULT_OK;

	// Initialize the demod.
    result = sony_demod_InitializeT_C(priv);//sony_demod_InitializeT_C
    if (result != SONY_RESULT_OK)
    {
        CXD2837_LOG(priv, "sony_dvb_demod_Initialize() error: %d.\r\n", result);
        return result;
    }
    
    //Demod can be config now.
    //Demod shall can access tuner through the I2C gateway.
    // Initialize the tuner.
    nim_cxd2837_ioctl(dev, NIM_TUNER_POWER_CONTROL, FALSE); //tuner power on


    //Config TS output mode: SSI/SPI. Default is SPI.	
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_PARALLEL_SEL, ((NIM_COFDM_TS_SSI == priv->tc.ts_mode)?0:1) );
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

    //Confif Data output pin: 0:TS0(pin13), 1:TS7(pin20), default:1.
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, ((NIM_COFDM_TS_SSI == priv->tc.ts_mode)?0:1) );
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

    //TS Error output.
	result = sony_demod_SetConfig(priv,SONY_DEMOD_CONFIG_TSERR_ACTIVE_HI, 1);//DEMOD_CONFIG_TSERR_ENABLE
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure DEMOD_CONFIG_TSERR_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

	/* IFAGC setup. Modify to suit connected tuner. */
	/* IFAGC: 0 for positive and 1 for negtive*/
#ifdef TUNER_IFAGCPOS
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#else
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_IFAGCNEG, 1);
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#endif

	/*IFAGC ADC range[0-2]     0 : 1.4Vpp, 1 : 1.0Vpp, 2 : 0.7Vpp*/   
	result = sony_demod_SetConfig(priv,SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 0);//DEMOD_CONFIG_TSERR_ENABLE
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure SONY_DEMOD_CONFIG_IFAGC_ADC_FS. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

	//Ben Debug 140221#1
	//add by AEC for TS error enable 2013-09-09
	 // TSERR output enable from GPIO2 pin
	result = sony_demod_GPIOSetConfig(priv, 2, 1, SONY_DEMOD_GPIO_MODE_TS_ERROR);
	if(result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv,"Error in sony_demod_GPIOSetConfig for TS error.\n");
		return result;
	}
	//end for TS error enable 2013-09-09  
	
	/* Spectrum Inversion setup. Modify to suit connected tuner. */
	/* Spectrum inverted, value = 1. */
#ifdef TUNER_SPECTRUM_INV
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_SPECTRUM_INV, 1);
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure SPECTRUM_INV. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#endif

    /* RFAIN ADC and monitor enable/disable. */
    /* Default is disabled. 1: Enable, 0: Disable. */
#ifdef RFAIN_ADC_ENABLE
        result = sony_demod_SetConfig(&priv, SONY_DEMOD_CONFIG_RFAIN_ENABLE, 0);
        if (result == SONY_RESULT_OK) {
            CXD2837_LOG(priv,"Demodulator configured to enable RF level monitoring.\n");
        }
        else {
            CXD2837_LOG(priv,"Error: Unable to configure RFLVMON_ENABLE. (result = %d)\n", result);
            return -1;
        }
#endif

	/* RF level monitoring (RFAIN/RFAGC) enable/disable. */
	/* Default is enabled. 1: Enable, 0: Disable. */
#ifdef TUNER_RFLVLMON_DISABLE
	result = sony_demod_SetConfig(priv, DEMOD_CONFIG_RFLVMON_ENABLE, 0);
	if (result != SONY_RESULT_OK)
	{
		CXD2837_LOG(priv, "Error: Unable to configure RFLVMON_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#endif
    osal_flag_set(priv->flag_id, NIM_SCAN_END);
    return SUCCESS;
}

static ID create_i2c_mutex(UINT32 i2c_type_id)
{
    struct nim_device *dev;
    //sony_demod_t * priv;
    UINT16 dev_id = 0;
    UINT16 mutex_id = OSAL_INVALID_ID;

    while(1)
    {
        dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, dev_id++);
        if(NULL == dev)
            break;
        //priv = (sony_demod_t *)dev->priv;
    }

    mutex_id = osal_mutex_create();
    return mutex_id;
}


/*****************************************************************************
* INT32  nim_m2837_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
* Description: cxd2837 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
__ATTRIBUTE_REUSE_
INT32 nim_cxd2837_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
{
    struct nim_device *dev;
    sony_demod_t *priv;
    DEM_WRITE_READ_TUNER ThroughMode;
    
    //UINT32 TunerIF;

    if(NULL == pConfig)
    {
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if(NULL == dev)
    {
        return ERR_NO_MEM;
    }

/* Alloc structure space of private */
    priv = (sony_demod_t*)MALLOC(sizeof(sony_demod_t));
    if(NULL == priv)
    {
        dev_free(dev);
        return ERR_NO_MEM;
    }
    MEMSET((UINT8*)priv, 0, sizeof(sony_demod_t));
    
// Setup demod I2C interfaces.
    demodI2c.i2c_type_id = pConfig->ext_dm_config.i2c_type_id;
    demodI2c.ReadRegister = sony_i2c_CommonReadRegister;
    demodI2c.WriteRegister = sony_i2c_CommonWriteRegister;
    demodI2c.WriteOneRegister = sony_i2c_CommonWriteOneRegister;

/* Create demodulator instance */
    if (sony_demod_Create (priv, SONY_DEMOD_XTAL_20500KHz, pConfig->ext_dm_config.i2c_base_addr, &demodI2c) != SONY_RESULT_OK)
    {
        return ERR_NO_DEV;
    }

//Keep Ali config. It must be run after sony_dvb_demod_Create(),because sony_dvb_demod_Create() will empty all setting.
	MEMCPY((UINT8*)&(priv->tc), (UINT8*)pConfig, sizeof(struct COFDM_TUNER_CONFIG_API));

/* ---------------------------------------------------------------------------------
* Configure the Demodulator
* ------------------------------------------------------------------------------ */
/* DVB-T demodulator IF configuration for terrestrial / cable tuner */
	priv->iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_5MHz_IF);
	priv->iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_6MHz_IF);
	priv->iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_7MHz_IF);
	priv->iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT_8MHz_IF);

/* DVB-T2 demodulator IF configuration for terrestrial / cable tuner */
	priv->iffreqConfig.configDVBT2_1_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_1_7MHz_IF);
	priv->iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_5MHz_IF);
	priv->iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_6MHz_IF);
	priv->iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_7MHz_IF);
	priv->iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBT2_8MHz_IF);

/* DVB-C demodulator IF configuration for terrestrial / cable tuner */
	priv->iffreqConfig.configDVBC_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBC_6MHz_IF);
	priv->iffreqConfig.configDVBC_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBC_7MHz_IF);
	priv->iffreqConfig.configDVBC_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (SONY_DVBC_8MHz_IF);

/* Demod tunerOptimize member allows the demod to be optimized internally when connected to Sony RF parts. */
#if(SYS_TUN_MODULE == CXD2861)
	priv->tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E;
#elif(SYS_TUN_MODULE == CXD2872)
	priv->tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3;	
#else
	priv->tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN;	
#endif
/*   transfer the system signal type to private data    */
	priv->sys_signal_type= pConfig->config_data.flag & 0x00000007 ; //0:ISDBT_TYPE, 1:DVBT_TYPE, 2:DVBT2_TYPE, 3:DVBT2-COMBO, 4...
	CXD2837_LOG(priv,"%s %d the t2_signal =  %d\n",__FUNCTION__,__LINE__,priv->sys_signal_type);
	dev->priv = (void*)priv;
	dev->init = NULL;
	dev->open = nim_cxd2837_open;
	dev->stop = nim_cxd2837_close;
	dev->channel_change = NULL; 
	dev->channel_search = NULL;
	dev->do_ioctl = nim_cxd2837_ioctl;
	dev->do_ioctl_ext = nim_cxd2837_ioctl_ext;

	dev->get_freq = nim_cxd2837_get_freq;
	dev->get_lock = nim_cxd2837_get_lock;
	dev->get_agc = nim_cxd2837_get_SSI;
	dev->get_snr = nim_cxd2837_get_SQI;
	dev->get_ber = nim_cxd2837_get_BER;

	dev->get_guard_interval = nim_cxd2837_get_GI;
	dev->get_fftmode = nim_cxd2837_get_fftmode;
	dev->get_modulation = nim_cxd2837_get_modulation;
	dev->get_fec = nim_cxd2837_get_FEC;
    
	dev->get_hier = NULL;
	dev->get_freq_offset = NULL;

	priv->flag_id = OSAL_INVALID_ID;
	priv->i2c_mutex_id = OSAL_INVALID_ID;
	priv->demodMode_mutex_id = OSAL_INVALID_ID;

	priv->flag_id = osal_flag_create(NIM_FLAG_ENABLE);
	if (priv->flag_id==OSAL_INVALID_ID)
	{
		CXD2837_LOG(priv, "%s: no more flag\r\n", __FUNCTION__);
		return ERR_ID_FULL;
	}

    priv->i2c_mutex_id = create_i2c_mutex(priv->tc.ext_dm_config.i2c_type_id);
    if(priv->i2c_mutex_id == OSAL_INVALID_ID)
    {
        CXD2837_LOG(priv, "%s: no more mutex\r\n", __FUNCTION__);
        return ERR_ID_FULL;
    }

	priv->demodMode_mutex_id = osal_mutex_create();
    if(priv->demodMode_mutex_id == OSAL_INVALID_ID)
    {
        CXD2837_LOG(priv, "%s: creat demodMode_mutex_id error\r\n", __FUNCTION__);
    }

    /* Add this device to queue */
    if(dev_register(dev) != SUCCESS)
    {
        CXD2837_LOG(priv,"Error: Register nim device error!\r\n");
        FREE(priv);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if(priv->tc.nim_tuner_init != NULL)
    {
    	if(priv->tc.nim_tuner_init(&(priv->tuner_id), &(priv->tc.tuner_config)) != SUCCESS)
        {
            CXD2837_LOG(priv,"Error: Init Tuner Failure!\r\n");
            return ERR_NO_DEV;
        }
    	ThroughMode.nim_dev_priv = dev->priv;
    	ThroughMode.dem_write_read_tuner = (void *)sony_i2c_TunerGateway;
    	nim_cxd2837_ioctl(dev, NIM_TUNER_SET_THROUGH_MODE, (UINT32)&ThroughMode);
    }
    return SUCCESS;
}




