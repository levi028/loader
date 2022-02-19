/*-----------------------------------------------------------------------------
    Full nim 'Sharp6158' composed by demodulater mn88472 + tuner MXL301.

    History:
    2011/11/21 by WangWenKai.
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


#include "nim_cxd2834.h"

#include "sony_dvb_demod.h"
#include "sony_dvb_integration.h"
#include "sony_dvb_demod_monitorT2.h"
#include "sony_dvb_demodT2.h"
#include "sony_dvb_demod_monitorT.h"

//#define CXD2834_DEBUG        libc_printf
#define CXD2834_DEBUG(...)

#define BER_TIMEOUT_VAL  5000

#define ARRAY_SIZE(x)        (sizeof(x)/sizeof(x[0]))

#define NIM_FLAG_ENABLE    0x00000100
#define NIM_SCAN_END       0x00000001

static sony_i2c_t demodI2c; //I2C interface of DEM.

extern sony_result_t WaitDemodLock (sony_dvb_demod_t * pDemod);
extern sony_result_t sony_dvb_signalinfo(sony_dvb_demod_t * pDemod, uint8_t *pQuality,uint32_t *pIFAGC,uint32_t *pBER);

void cxd2834_log_i2c(sony_dvb_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len)
{
    int i;
    if ( ! (param->output_buffer && param->fn_output_string) )
        return;

    if (write)
        CXD2834_LOG(param, "I2C_Write,0x%02X", (slv_addr&0xFE));
    else
        CXD2834_LOG(param, "I2C_Read,0x%02X", (slv_addr|1));

    for ( i=0; i<len; ++i )
    {
        CXD2834_LOG(param, ",0x%02X", data[i]);
    }

    if (err)
        CXD2834_LOG(param, "\terror");
    CXD2834_LOG(param, "\r\n");
}




static void nim_cxd2834_switch_lock_led(struct nim_device *dev, BOOL On)
{
    if(((sony_dvb_demod_t *)dev->priv)->tc.nim_lock_cb)
        ((sony_dvb_demod_t *)dev->priv)->tc.nim_lock_cb(On);
}


static INT32 config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 *ptr_lock)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    UINT32 tuner_id = priv->tuner_id;
    UINT8 lock = 0, Tuner_Retry = 0;

//    set_bypass_mode(dev, TRUE);
    do
    {
        Tuner_Retry++;

        if(priv->tc.nim_tuner_control(tuner_id, freq, bandwidth, 0, NULL, 0)==ERR_FAILUE)
        {
            CXD2834_LOG(priv,"Config tuner failed, I2c failed!\r\n");
        }

        if(priv->tc.nim_tuner_status(tuner_id, &lock) != SUCCESS)
        {
            //if i2c read failure, no lock state can be report.
            lock = 0;
            CXD2834_LOG(priv,"Config tuner failed, I2c failed!\r\n");
        }
        CXD2834_LOG(priv,"Tuner Lock Times=0x%d,Status=0x%d\r\n",Tuner_Retry,lock);

        if(Tuner_Retry > 5)
            break;
    }while(0 == lock);

    if(ptr_lock != NULL)
        *ptr_lock = lock;

    if(Tuner_Retry > 5)
    {
        CXD2834_LOG(priv,"ERROR! Tuner Lock Fail\r\n");
        return ERR_FAILUE;
    }
    return SUCCESS;
}

BOOL wait_for_signal_become_stable(sony_dvb_demod_t * param)
{
    //UINT8 lock = 0;
    INT32 result = ERR_FAILUE;

//    DMD_wait(300);
    do
    {
//        if( DMD_scan( param ) != DMD_E_OK )  //DEM Locks the DVB-T2 signal.
//            break;

        result = SUCCESS;
    }while(0);

    return result;
}

static BOOL need_to_config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT16 if_freq)
{
    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;
    UINT32 tuner_id = param->tuner_id;
    UINT8 lock = 0;

    return ! ( param->Frequency == freq \
            && param->bandWidth == bandwidth \
            && param->if_freq == if_freq \
            && param->tc.nim_tuner_status(tuner_id, &lock) == SUCCESS );
}

static BOOL need_to_lock_DVBT_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para, BOOL NeedToConfigTuner)
{
    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

    if ( change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return FALSE;

    //USAGE_TYPE_CHANCHG: for play program.
    if (change_para->t2_signal) //Do nothing for play DVB-T2 program.
        return FALSE;
    if (param->system != SONY_DVB_SYSTEM_DVBT) //DEM is not be DVB-T mode now.
        return TRUE;
    if (param->priority != change_para->priority) //DVB-T Hierarchy mode: HP/LP.
        return TRUE;

    return NeedToConfigTuner;
}

static INT32 need_to_lock_DVBT2_signal(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para, BOOL NeedToConfigTuner, BOOL *p_play_program)
{
    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;

    *p_play_program = FALSE;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE \
        || change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

   //USAGE_TYPE_CHANCHG: for play program.
    if ( ! change_para->t2_signal ) //Do nothing for play DVB-T program.
        return FALSE;

    *p_play_program = TRUE;
    if (param->system != SONY_DVB_SYSTEM_DVBT2) //DEM is not be DVB-T2 mode now.
        return TRUE;

//    *p_play_program = TRUE;
    if (param->plp_index != change_para->plp_index) //Current selected PLP is different with the target PLP.
        return TRUE;

    return NeedToConfigTuner;
}

static INT32 try_to_lock_DVBT_signal(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner, struct NIM_CHANNEL_CHANGE *change_para)
{
    UINT8 lock = 0;
    INT32 result = ERR_FAILUE;
    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;
    sony_dvb_tune_params_t  TuneParam;
    sony_dvb_demod_lock_result_t  demod_lock;

    MEMSET(&TuneParam,0,sizeof(TuneParam));
    TuneParam.bwMHz = change_para->bandwidth;
    TuneParam.centreFreqkHz = change_para->freq;
    TuneParam.system = SONY_DVB_SYSTEM_DVBT;
    TuneParam.tParams.usePresets = 0;
    TuneParam.tParams.profile = change_para->priority; //SONY_DVBT_PROFILE_HP;

    do
    {

        if ( NeedToInitSystem || param->system != SONY_DVB_SYSTEM_DVBT )
        {
            param->t2_signal = 0;
//            param->system = SONY_DVB_SYSTEM_DVBT;

            // Setup any required preset information.
            result = sony_dvb_demod_TuneInitialize (param, &TuneParam);
            if (result != SONY_RESULT_OK)
            {
//                SONY_TRACE_RETURN (result);
                result = ERR_FAILUE;
                break;
            }
//            NeedToConfigTuner = TRUE;
        }

//        if ( param->system != SONY_DVB_SYSTEM_DVBT )
//            return result;

        // Wait for demodulator lock DVB-T signal.

        // Enable acquisition on the demodulator.
        // param->system and param->bandWidth will be compared and sync by sony_dvb_demod_Tune().
        result = sony_dvb_demod_Tune (param, TuneParam.system, TuneParam.bwMHz);
        if (result != SONY_RESULT_OK)
        {
//                SONY_TRACE_RETURN (result);
            result = ERR_FAILUE;
            break;
        }

        // Perform RF tuning.
        if ( NeedToConfigTuner )
        {
            result = config_tuner(dev, TuneParam.centreFreqkHz, TuneParam.bwMHz, &lock); //The tuner locks the frequency.
            if(result != SUCCESS)
            {
                result = ERR_FAILUE;
                break;
            }
        }

        result = sony_dvb_demod_TuneEnd (param);
        if (result != SONY_RESULT_OK)
        {
//                SONY_TRACE_RETURN (result);
            result = ERR_FAILUE;
            break;
        }

        sony_dvb_integration_WaitTSLock((void *)param);

        //Check the demodulator after acquisition, specific to the system in use.
        result = sony_dvb_demod_TuneFinalize (param, &TuneParam, SONY_RESULT_OK);
        if (result != SONY_RESULT_OK)
        {
            result = ERR_FAILUE;
            break;
        }

        sony_dvb_demod_CheckTSLock (param, &demod_lock);
        if ( DEMOD_LOCK_RESULT_LOCKED == demod_lock )
            result = SUCCESS;
        else
            result = ERR_FAILUE;
        break;
    }while(0);

    return result;
}


static INT32 DVBT2_change_PLP(struct nim_device *dev, UINT8 plp_id, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
    UINT8 lock = 0;
    INT32 result = ERR_FAILUE;
    sony_dvb_demod_t* param = (sony_dvb_demod_t *)dev->priv;
    sony_dvb_tune_params_t  TuneParam;
    //sony_dvb_demod_lock_result_t  demod_lock;

    CXD2834_LOG(param, "%s(%d,%d,%d)\r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner);

    MEMSET(&TuneParam,0,sizeof(TuneParam));
    TuneParam.bwMHz = param->bandWidth;
    TuneParam.centreFreqkHz = param->Frequency;
    TuneParam.system = SONY_DVB_SYSTEM_DVBT2;
    TuneParam.tParams.usePresets = 0;
    TuneParam.t2Params.dataPLPId = plp_id; //selected data PLP ID.

//    if (plp_index < param->plp_num)
//        TuneParam.t2Params.dataPLPId = param->plp_index; //selected data PLP ID.
//    else
//        TuneParam.t2Params.dataPLPId = 0; //change_para->plp_index;   //change_para->plp_id;//selected data PLP ID.  //

    param->t2_signal = 1;
//    param->plp_index = plp_index;
    do
    {
//        if ( NeedToInitSystem || param->system != SONY_DVB_SYSTEM_DVBT2)
        {
//            param->t2_signal = 1;
//            param->plp_num = 0;
//            param->system = SONY_DVB_SYSTEM_DVBT2;

            // param->system and param->bandWidth will be compared and sync by sony_dvb_demod_Tune().
            // Setup demodulator for acquisition with the given parameters.
            result = sony_dvb_demod_Tune (param, TuneParam.system, TuneParam.bwMHz);
            if (result != SONY_RESULT_OK)
            {
                CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: sony_dvb_demod_Tune()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
                result = ERR_FAILUE;
                break;
            }
        }

//        if ( param->system != SONY_DVB_SYSTEM_DVBT2 )
//            break;

        // Setup any required preset information.
        // Configure demodulator before acquisition, specific to the system in use.
        result = sony_dvb_demod_TuneInitialize (param, &TuneParam);
        if (result != SONY_RESULT_OK)
        {
            CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: sony_dvb_demod_TuneInitialize()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
            result = ERR_FAILUE;
            break;
        }

        // Perform RF tuning.
        if ( NeedToConfigTuner )
        {
            result = config_tuner(dev, TuneParam.centreFreqkHz, TuneParam.bwMHz, &lock); //The tuner locks the frequency.
            if(result != SUCCESS)
            {
                CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: config_tuner()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
                result = ERR_FAILUE;
                break;
            }
        }

        //Completes the demodulator acquisition setup. Must be called after sony_dvb_demod_Tune and an RF Tune.
        result = sony_dvb_demod_TuneEnd (param);
        if (result != SONY_RESULT_OK)
        {
            CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: sony_dvb_demod_TuneEnd()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
            result = ERR_FAILUE;
            break;
        }

        result = WaitDemodLock (param); //if (TuneParam.system == SONY_DVB_SYSTEM_DVBT2)
        if (result != SONY_RESULT_OK)
        {
            CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: WaitDemodLock()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
            result = ERR_FAILUE;
            break;
        }

//        sony_dvb_demod_CheckTSLock (param, &demod_lock);
//        if ( DEMOD_LOCK_RESULT_LOCKED != demod_lock )
//        {
//            result = ERR_FAILUE;
//            break;
//        }

        //Check the demodulator after acquisition, specific to the system in use.
        result = sony_dvb_demod_TuneFinalize (param, &TuneParam, SONY_RESULT_OK);
        if (result != SONY_RESULT_OK)
        {
            CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: sony_dvb_demod_TuneFinalize()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
            result = ERR_FAILUE;
            break;
        }
        if (TuneParam.t2Params.tuneResult & SONY_DVBT2_TUNE_RESULT_DATA_PLP_NOT_FOUND)
        {
            CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: SONY_DVBT2_TUNE_RESULT_DATA_PLP_NOT_FOUND : TuneParam.t2Params.tuneResult=%d  \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, TuneParam.t2Params.tuneResult);
            result = ERR_FAILUE;
            break;
        }

        result = sony_dvb_integration_WaitTSLock((void *)param);
        if (result != SONY_RESULT_OK)
        {
            CXD2834_LOG(param, "%s(%d,%d,%d)(%d,%d,%d) error: sony_dvb_integration_WaitTSLock()=%d \r\n", __FUNCTION__,plp_id,NeedToInitSystem,NeedToConfigTuner, param->system, param->bandWidth, param->Frequency, result);
            break;
        }
/*
        If TS is locked, then the first data PLP in the channel is selected automatically
        and is output by the demodulator.
        In order to determine the list of data PLPs carried, then use the function sony_dvb_demod_monitorT2_DataPLPs.
*/

//        param->t2_signal = 1;
//        param->plp_id = plp_id;
//        param->plp_index = plp_index;

        return SUCCESS;   //The data PLP is locked.
    }while(0);

    return result;
}


static INT32 try_to_lock_next_data_plp(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
    INT32 result = ERR_FAILUE;
    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    UINT8 retry = 0;
    UINT8 plp_index = 0;

    for (plp_index = param->plp_index+1; plp_index < param->plp_num; ++plp_index )
    {
        if (plp_index >= param->plp_num)
            return ERR_FAILUE;
        result = DVBT2_change_PLP(dev, param->all_plp_id[plp_index], FALSE, FALSE);

        for ( retry=0; retry<30; ++retry )
        {
            SONY_SLEEP (30);
            if (param->autoscan_stop_flag)
                return ERR_FAILUE;
            if (param->do_not_wait_t2_signal_locked)
                return ERR_FAILUE;

            result = sony_dvb_demod_monitorT2_L1Pre (param, &L1Pre);
            if (result != SONY_RESULT_OK)
            {
                CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d) error: sony_dvb_demod_monitorT2_L1Pre()=%d \r\n", __FUNCTION__,NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index],result);
                continue;
//                return result;
            }

            // Get Active PLP information.
            result = sony_dvb_demod_monitorT2_ActivePLP (param, SONY_DVBT2_PLP_DATA, &plpInfo);
            if (result != SONY_RESULT_OK)
            {
                CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d) error: sony_dvb_demod_monitorT2_ActivePLP()=%d \r\n", __FUNCTION__,NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index],result);
                continue;
//                return result;
            }

            if (result == SONY_RESULT_OK)
            {
                if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
                {
                    if (plpInfo.id != param->all_plp_id[plp_index])
                    {
                        CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), plpInfo.id=%d, error PLP locked: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], plpInfo.id, retry);
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
                    CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
                return SONY_RESULT_OK;
            }
            else
            {
                    CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times. error: Not DataPLP: (type=%d, id=%d)\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry, plpInfo.type, plpInfo.id);
            }
        }
        else
        {
            CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), error: fail to lock the PLP.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
        }
    }
    return ERR_FAILUE;
}

static INT32 try_to_lock_DVBT2_signal(struct nim_device *dev, BOOL NeedToInitSystem, BOOL NeedToConfigTuner)
{
//    uint8_t numPLPs = 0;
//    uint8_t PLPIds[SONY_DVBT2_MAX_PLPS];
    uint16_t waitTime = 0;
    UINT8 plp_index = 0;
    UINT8 retry = 0;

    INT32 result = ERR_FAILUE;
    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;

    param->plp_num = 0;
    param->plp_id = 0;
    param->plp_index = 0;

//    change_para->plp_num = 0;
//    change_para->plp_id = 0;
//    change_para->plp_index = 0;

    result = DVBT2_change_PLP(dev, 0, NeedToInitSystem, NeedToConfigTuner); //for lock signal first.
//    if (result != SUCCESS)
//    {
//        return result;
//    }

    //get_the_first_data_PLP_info
    do
    {
        result = sony_dvb_demod_monitorT2_DataPLPs ( param, (uint8_t *)(&(param->all_plp_id)), &(param->plp_num) );
        if (result == SONY_RESULT_OK)
        {
            UINT8 plp_idx;
            CXD2834_LOG(param, "%s: plp_num=%d [", __FUNCTION__, param->plp_num);
            for (plp_idx=0; plp_idx < param->plp_num; ++plp_idx)
            {
                CXD2834_LOG(param, ", %d", param->all_plp_id[plp_idx]);
            }
            CXD2834_LOG(param, " ]\r\n");

            break;
        }
        else if (result == SONY_RESULT_ERROR_HW_STATE)
            {
                if (waitTime >= DVB_DEMOD_TUNE_T2_L1POST_TIMEOUT)
                {
                    CXD2834_LOG(param, "%s() error: timeout for get the first data_PLP\r\n", __FUNCTION__);
                    param->plp_num = 0;
                    result = SONY_RESULT_ERROR_TIMEOUT;
                    return result;
                } else
                {
                    SONY_SLEEP (10); //DEMOD_TUNE_POLL_INTERVAL
                    waitTime += 10; //DEMOD_TUNE_POLL_INTERVAL
                }
            }
            else
            {
                CXD2834_LOG(param, "%s()=%d error: Fail to get the first data_PLP\r\n", __FUNCTION__, result);
                param->plp_num = 0;
                return result; // Other (fatal) error.
            }
    }while (1);

    //The first data PLP is locked now.
    //Try to find the first data PLP that can be locked.
    for (plp_index = 0; plp_index < param->plp_num;  )  //In fact, this loop is not necessary.
    {
        for ( retry=0; retry<30; ++retry )
        {
            SONY_SLEEP (30);
            if (param->autoscan_stop_flag)
                return ERR_FAILUE;
            if (param->do_not_wait_t2_signal_locked)
                return ERR_FAILUE;

            result = sony_dvb_demod_monitorT2_L1Pre (param, &L1Pre);
            if (result != SONY_RESULT_OK)
            {
                continue;
//                return result;
            }

            // Get Active PLP information.
            result = sony_dvb_demod_monitorT2_ActivePLP (param, SONY_DVBT2_PLP_DATA, &plpInfo);
            if (result != SONY_RESULT_OK)
            {
                continue;
//                return result;
            }

            if (result == SONY_RESULT_OK)
            {
                if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
                {
                    if (plpInfo.id != param->all_plp_id[plp_index])
                    {
                        CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), plpInfo.id=%d, error PLP locked: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], plpInfo.id, retry);
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
                    CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
                return SONY_RESULT_OK;
            }
            else
            {
                    CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), ok: retry %d times. error: Not DataPLP: (type=%d, id=%d)\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry, plpInfo.type, plpInfo.id);
            }
        }
        else
        {
            CXD2834_LOG(param, "%s(%d,%d)(plp_num=%d, all_plp_id[%d]=%d), error: fail to lock the PLP.\r\n", __FUNCTION__, NeedToInitSystem,NeedToConfigTuner, param->plp_num, plp_index, param->all_plp_id[plp_index], retry);
        }

        //In fact, this loop is not necessary.
        ++plp_index;
        if (plp_index >= param->plp_num)
            return ERR_FAILUE;
        result = DVBT2_change_PLP(dev, param->all_plp_id[plp_index], FALSE, FALSE);

    }

    return result;
}

static INT32 nim_cxd2834_channel_change_smart(struct nim_device *dev, struct NIM_CHANNEL_CHANGE *change_para)
{
    //UINT8 lock = 0, Tuner_Retry = 0;
    //UINT8 WAIT_LOCK_TIME=1;
    //UINT8 try_time;
    INT32 result;

    sony_dvb_demod_t * param = (sony_dvb_demod_t *)dev->priv;
    //PCOFDM_TUNER_CONFIG_API config_info = &(param->tc);
    BOOL play_program, NeedToInitSystem = FALSE, NeedToConfigTuner = FALSE;
    UINT16 if_freq;
    UINT32 flgptn;

    if ((change_para->freq <= 40000) || (change_para->freq >= 900000))
    {
        return ERR_FAILUE;
    }

     if (change_para->bandwidth != 6 && change_para->bandwidth != 7 && change_para->bandwidth != 8)
    {
        return ERR_FAILUE;
    }

#if 0
     switch(change_para->bandwidth)
    {
      case 6:
        if_freq = 4570;
        break;

      case 7:
        if_freq = 4570;
        break;

      case 8:
        if_freq = 4570;
        break;

      default:
        return ERR_FAILUE;
    }
#endif
    if_freq = 4570;

    result = osal_flag_wait(&flgptn, param->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 30*1000); //OSAL_WAIT_FOREVER_TIME
    if(OSAL_E_OK != result)
    {
         return ERR_FAILUE;
    }

    CXD2834_LOG(param, "%s: usage_type %d, freq %d, bandwidth %d, priority %d, t2_signal %d, plp_index %d, plp_id %d\r\n",__FUNCTION__,change_para->usage_type, change_para->freq,change_para->bandwidth, change_para->priority, change_para->t2_signal, change_para->plp_index, change_para->plp_id);

    do
    {
        param->autoscan_stop_flag = 0;
        param->do_not_wait_t2_signal_locked = ( change_para->usage_type == USAGE_TYPE_AERIALTUNE ? 1:0 );

        if ( need_to_config_tuner(dev, change_para->freq, change_para->bandwidth, if_freq) )
        {
            if ( param->bandWidth != change_para->bandwidth \
              || param->if_freq != if_freq  )
              NeedToInitSystem = TRUE;

            param->Frequency = change_para->freq;
//            param->bandWidth = change_para->bandwidth;
            param->if_freq = if_freq;

//            result = config_tuner(dev, param->Frequency, param->bandwidth, &lock); //The tuner locks the frequency.
//            if(result != SUCCESS)
//            {
//                break;
//            }
            NeedToConfigTuner = TRUE;
        }

        if ( need_to_lock_DVBT_signal(dev, change_para, NeedToConfigTuner) )
        {
            param->priority = change_para->priority;
            result = try_to_lock_DVBT_signal(dev, NeedToInitSystem, NeedToConfigTuner, change_para);
            if (result == SUCCESS)
            {
                break;
            }
        }
        if (param->autoscan_stop_flag)
        {
            break;
        }


        if ( need_to_lock_DVBT2_signal(dev, change_para, NeedToConfigTuner, &play_program) )
        {
            if (play_program)
            {
                param->do_not_wait_t2_signal_locked = 1;
                param->plp_index = change_para->plp_index;
                result = DVBT2_change_PLP(dev, change_para->plp_id, NeedToInitSystem, NeedToConfigTuner);
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
                }
                change_para->plp_index = param->plp_index;
                change_para->plp_id = param->plp_id;
                change_para->t2_system_id = param->t2_system_id;
            }
        }

    }while (0);
    change_para->t2_signal = param->t2_signal;

    osal_flag_set(param->flag_id,NIM_SCAN_END);
    return result;
}


#if 0
/*****************************************************************************
* INT32 nim_cxd2834_get_specinv(struct nim_device *dev, UINT8 *Inv)
*
*  Read FEC lock status
*
*Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 *Inv
*
*Return Value: INT32
*****************************************************************************/
static INT32 nim_cxd2834_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    *Inv = priv->info[DMD_E_INFO_DVBT2_FEF_INTERVAL];
    return SUCCESS;
}


static INT32 nim_cxd2834_get_PER(struct nim_device *dev, UINT8 *per)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    DMD_u32_t err;
    DMD_u32_t sum;
    OSAL_ER result;
    UINT32 flgptn;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        DMD_PER(priv, priv->system, &err, &sum);
        if (sum != 0)
            *per =  100 - (err*100)/sum;
        else
            *per =  0;
    }
    else
    {
        *per =  0;
    }
    return SUCCESS;
}

static INT32 nim_cxd2834_get_CNR(struct nim_device *dev, UINT8 *data)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    OSAL_ER result;
    UINT32  flgptn;
    DMD_u32_t cnr_i=0, cnr_d=0;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        DMD_CNR(priv, priv->system, &cnr_i , &cnr_d );
        *data = cnr_i;
    }
    else
    {
        *data = 0;
    }
    return SUCCESS;
}


#endif

static UINT8 cxd2834_modulation_map_to_ali_modulation(sony_dvb_system_t system, UINT8 modulation)
{
    //T_NODE:    UINT32 modulation : 8;
    //2:DQPSK 4:QPSK, 16:16 QAM, 64:64 QAM //T2: (64+1):256 QAM

   if (system == SONY_DVB_SYSTEM_DVBT)
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

static INT32 nim_cxd2834_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_dvb_demod_t * pDemod = (sony_dvb_demod_t *)dev->priv;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    //sony_dvbt2_ofdm_t ofdm;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    sony_dvbt_tpsinfo_t tps;

    *modulation = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    result = sony_dvb_demod_CheckDemodLock(pDemod, &lock);
    if (result != SONY_RESULT_OK || lock != DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

    if (pDemod->system == SONY_DVB_SYSTEM_DVBT2)
    {
        sony_dvb_demod_monitorT2_L1Pre (pDemod, &L1Pre);
        if (result != SONY_RESULT_OK)
            return ERR_FAILUE;

        // Get Active PLP information.
        result = sony_dvb_demod_monitorT2_ActivePLP (pDemod, SONY_DVBT2_PLP_DATA, &plpInfo);
        if (result != SONY_RESULT_OK || plpInfo.constell > SONY_DVBT2_QAM256 )
            return ERR_FAILUE;

        if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
        {
            if (plpInfo.id != pDemod->plp_id)
            {
                CXD2834_LOG(pDemod, "%s(): plp_id=%d, plpInfo.id=%d, error PLP locked.\r\n", __FUNCTION__,  pDemod->plp_id, plpInfo.id);
                return ERR_FAILUE;
            }

            *modulation = cxd2834_modulation_map_to_ali_modulation(pDemod->system, plpInfo.constell);
            return SONY_RESULT_OK;
        }
    }
    else
    {
        result = sony_dvb_demod_monitorT_TPSInfo (pDemod, &tps);
        if (result != SONY_RESULT_OK || tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3 )
            return ERR_FAILUE;
        *modulation = cxd2834_modulation_map_to_ali_modulation(pDemod->system, tps.constellation);
    }

    return SUCCESS;
}


static UINT8 cxd2834_FEC_map_to_ali_FEC(sony_dvb_system_t system, UINT8 fec)
{
    //T_NODE:    UINT16 FEC_inner            : 4;
    //T: 0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:7/8  //T2: 5:3/5, 6:4/5 //0xF:unknown

   if (system == SONY_DVB_SYSTEM_DVBT)
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
            default:
                return 0xF;   //unknown.
        }
    }
}

static INT32 nim_cxd2834_get_FEC(struct nim_device *dev, UINT8* FEC)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_dvb_demod_t * pDemod = (sony_dvb_demod_t *)dev->priv;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    //sony_dvbt2_ofdm_t ofdm;
    sony_dvbt2_l1pre_t L1Pre;
    sony_dvbt2_plp_t plpInfo;
    sony_dvbt_tpsinfo_t tps;

    *FEC = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    result = sony_dvb_demod_CheckDemodLock(pDemod, &lock);
    if (result != SONY_RESULT_OK || lock != DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

    if (pDemod->system == SONY_DVB_SYSTEM_DVBT2)
    {
        sony_dvb_demod_monitorT2_L1Pre (pDemod, &L1Pre);
        if (result != SONY_RESULT_OK)
            return ERR_FAILUE;

        // Get Active PLP information.
        result = sony_dvb_demod_monitorT2_ActivePLP (pDemod, SONY_DVBT2_PLP_DATA, &plpInfo);
        if (result != SONY_RESULT_OK)
            return ERR_FAILUE;

        if (plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA1 || plpInfo.type == SONY_DVBT2_PLP_TYPE_DATA2)
        {
            if (plpInfo.id != pDemod->plp_id)
            {
                CXD2834_LOG(pDemod, "%s(): plp_id=%d, plpInfo.id=%d, error PLP locked.\r\n", __FUNCTION__,  pDemod->plp_id, plpInfo.id);
                return ERR_FAILUE;
            }

            *FEC = cxd2834_FEC_map_to_ali_FEC(pDemod->system, plpInfo.plpCr);
            return SUCCESS;
        }
    }
    else
    {
        result = sony_dvb_demod_monitorT_TPSInfo (pDemod, &tps);
        if (result != SONY_RESULT_OK || tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3 )
            return ERR_FAILUE;

        *FEC = cxd2834_FEC_map_to_ali_FEC(pDemod->system, tps.rateHP);
    }

    return SUCCESS;
}

static UINT8 cxd2834_gi_map_to_ali_gi(sony_dvb_system_t system, UINT8 guard_interval)
{
    //T_NODE:    UINT32 guard_interval : 8;
    //4: 1/4, 8: 1/8, 16: 1/16, 32:1/32  //T2: 128:1/128, (19+128):19/128, 19:19/256

    if (system == SONY_DVB_SYSTEM_DVBT)
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

static INT32 nim_cxd2834_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_dvb_demod_t * pDemod = (sony_dvb_demod_t *)dev->priv;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    sony_dvbt2_ofdm_t ofdm;
    sony_dvbt_mode_t fft_mode_t;
    sony_dvbt_guard_t gi_t;

    *guard_interval = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    result = sony_dvb_demod_CheckDemodLock(pDemod, &lock);
    if (result != SONY_RESULT_OK || lock != DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

    if (pDemod->system == SONY_DVB_SYSTEM_DVBT2)
    {
        result = sony_dvb_demodT2_OptimizeMISO(pDemod, &ofdm);
        if (result != SONY_RESULT_OK)
        {
            return ERR_FAILUE;
        }
        *guard_interval = cxd2834_gi_map_to_ali_gi(pDemod->system, ofdm.gi);
    }
    else
    {
        result = sony_dvb_demod_monitorT_ModeGuard (pDemod, &fft_mode_t, &gi_t);
        if (result != SONY_RESULT_OK)
        {
            return ERR_FAILUE;
        }
        *guard_interval = cxd2834_gi_map_to_ali_gi(pDemod->system, gi_t);;
    }


    return SUCCESS;
}


static UINT8 cxd2834_fft_mode_map_to_ali_fft_mode(sony_dvb_system_t system, UINT8 fft_mode)
{
    //T_NODE:    UINT32 FFT : 8;
    //2:2k, 8:8k //T2: 4:4k, 16:16k, 32:32k

    if (system == SONY_DVB_SYSTEM_DVBT)
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

static INT32 nim_cxd2834_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    UINT32 flgptn;
    OSAL_ER result;
    sony_dvb_demod_t * pDemod = (sony_dvb_demod_t *)dev->priv;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    sony_dvbt2_ofdm_t ofdm;
    sony_dvbt_mode_t fft_mode_t;
    sony_dvbt_guard_t gi_t;

    *fft_mode = 0;
    result = osal_flag_wait(&flgptn, pDemod->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK != result)
        return ERR_FAILUE;

    result = sony_dvb_demod_CheckDemodLock(pDemod, &lock);
    if (result != SONY_RESULT_OK || lock != DEMOD_LOCK_RESULT_LOCKED)
        return ERR_FAILUE;

    if (pDemod->system == SONY_DVB_SYSTEM_DVBT2)
    {
        result = sony_dvb_demodT2_OptimizeMISO(pDemod, &ofdm);
        if (result != SONY_RESULT_OK)
        {
            return ERR_FAILUE;
        }
        *fft_mode = cxd2834_fft_mode_map_to_ali_fft_mode(pDemod->system, ofdm.mode);
    }
    else
    {
        result = sony_dvb_demod_monitorT_ModeGuard (pDemod, &fft_mode_t, &gi_t);
        if (result != SONY_RESULT_OK)
        {
            return ERR_FAILUE;
        }
        *fft_mode = cxd2834_fft_mode_map_to_ali_fft_mode(pDemod->system, fft_mode_t);
    }
    return SUCCESS;
}



static INT32 nim_cxd2834_get_freq(struct nim_device *dev, UINT32 *freq)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    *freq = priv->Frequency;
    return SUCCESS;
}


//For CXD2834+MXL603
#define SONYT2_MAX_STRENGTH_ENTRTY 71
static AGC_Table_t SONYT2_Tuner_AGC_Table[SONYT2_MAX_STRENGTH_ENTRTY] =
{
    {4095,0},{2830,29},
    {2804,30},{2776,31},{2748,32},{2722,33},{2695,34},{2668,35},{2641,36},{2613,37},{2586,38},{2559,39},
    {2532,40},{2504,41},{2476,42},{2450,43},{2421,44},{2394,45},{2366,46},{2339,47},{2312,48},{2284,49},
    {2257,50},{2229,51},{2203,52},{2175,53},{2149,54},{2121,55},{2093,56},{2059,57},{2033,58},{2007,59},
    {1979,60},{1945,61},{1918,62},{1891,63},{1864,64},{1838,65},{1810,66},{1777,67},{1751,68},{1723,69},
    {1688,70},{1660,71},{1633,72},{1606,73},{1577,74},{1550,75},{1522,76},{1496,77},{1469,78},{1442,79},
    {1414,80},{1384,81},{1357,82},{1331,83},{1304,84},{1276,85},{1249,86},{1221,87},{1194,88},{1173,89},
    {1146,90},{1117,91},{1090,92},{1060,93},{1027,94},{999,95},  {969,96},  {942,97}, {914,98}
};

static UINT32 AGC2SignalStrength(UINT32 AGC)
{
    int i=0;
    for(i =0;i<SONYT2_MAX_STRENGTH_ENTRTY;i++)
    {
        if(AGC >=  SONYT2_Tuner_AGC_Table[i].AGC)
            break;
    }
    if(i >= SONYT2_MAX_STRENGTH_ENTRTY)
        i = SONYT2_MAX_STRENGTH_ENTRTY-1;

    return SONYT2_Tuner_AGC_Table[i].SignalStrength;

}

static INT32 nim_cxd2834_channel_change(
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

    return nim_cxd2834_channel_change_smart(dev, &change_para);
}

static INT32 nim_cxd2834_get_AGC(struct nim_device *dev, UINT8 *data)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    OSAL_ER result;
    uint32_t  agc = 0, ber = 0, quality = 0;
    UINT32 flgptn = 0;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        sony_dvb_signalinfo(priv, (uint8_t *)(&quality), &agc, &ber);
        *data = AGC2SignalStrength(agc);
    }
    else
    {
        *data = 0;
    }
    return SUCCESS;
}

static INT32 nim_cxd2834_get_quality(struct nim_device *dev, UINT8 *snr)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    OSAL_ER result;
    uint32_t  agc = 0, ber = 0, quality = 0;
    UINT32 flgptn = 0;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        sony_dvb_signalinfo(priv, (uint8_t *)(&quality), &agc, &ber);
        *snr = quality;
    }
    else
    {
        *snr = 0;
    }
    return SUCCESS;
}

static INT32 nim_cxd2834_get_BER(struct nim_device *dev, UINT32 *BER)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    OSAL_ER result;
    uint32_t  agc = 0, ber = 0, quality = 0;
    UINT32 flgptn = 0;
    result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
    if(OSAL_E_OK==result)
    {
        sony_dvb_signalinfo(priv, (uint8_t *)(&quality), &agc, &ber);
        *BER = ber;
    }
    else
    {
        *BER = 0;
    }
    return SUCCESS;
}


static INT32 nim_cxd2834_get_lock(struct nim_device *dev, UINT8 *lock)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    //UINT8 data=0;
    INT32 i;
    OSAL_ER result;
    UINT32 flgptn;
    sony_dvb_demod_lock_result_t demod_lock = DEMOD_LOCK_RESULT_NOTDETECT;

//    CXD2834_LOG(priv, "%s : enter ------------------------------------------\r\n", __FUNCTION__);

    *lock = 0;
    for ( i=0; i<1; ++i)
    {
        result = osal_flag_wait(&flgptn, priv->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW, 0);
        if(OSAL_E_OK != result)
        {
//            CXD2834_LOG(priv, "%s: Unlock. osal_flag_wait() fail.\r\n", __FUNCTION__);
            break;
        }

        sony_dvb_demod_CheckTSLock (priv, &demod_lock);
        if ( DEMOD_LOCK_RESULT_LOCKED == demod_lock )
        {
            if (i>0)
            {
//                CXD2834_LOG(priv, "%s: Lock. retry %d times.\r\n", __FUNCTION__, i);
            }
            *lock = 1;
            break;
        }
    }

    if (0 == *lock)
    {
//        CXD2834_LOG(priv, "%s: Unlock. sony_dvb_demod_CheckTSLock().\r\n", __FUNCTION__);
    }

    //put light the panel lock here. if *lock=ff, should not light on the led.
    nim_cxd2834_switch_lock_led(dev, (*lock)?TRUE:FALSE);

//    CXD2834_LOG(priv, "%s : end ------------------------------------------\r\n", __FUNCTION__);
    return SUCCESS;
}



static INT32 nim_cxd2834_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
//    nim_ad_gain_table_t *agt, *d_agt;
//    nim_rf_if_ad_val_t *p_rf_if_ad_val;
//    nim_rec_performance_t * p_nim_rec_performance;
    //INT32 freq_offset;
    //UINT8 data[2];
    INT32 ret_val = SUCCESS;
//    UINT16 if_ad_val, rf_ad_val;

    if(cmd & NIM_TUNER_COMMAND)
    {
        if(NIM_TUNER_CHECK == cmd)
            return SUCCESS;

        if(priv->tc.nim_tuner_command != NULL)
        {
            ret_val = priv->tc.nim_tuner_command(priv->tuner_id, cmd, param);
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
            return nim_cxd2834_get_AGC(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_BER:
            return nim_cxd2834_get_BER(dev, (UINT32 *)param);
            break;
        case NIM_DRIVER_GET_GUARD_INTERVAL:
            return nim_cxd2834_get_GI(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_FFT_MODE:
            return nim_cxd2834_get_fftmode(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_MODULATION:
            return nim_cxd2834_get_modulation(dev, (UINT8 *)param);
            break;
        case NIM_DRIVER_GET_SPECTRUM_INV:
            break;
        default:
            ret_val = ERR_FAILUE;
            break;
    }

    return ret_val;
}


static INT32 nim_cxd2834_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
    struct NIM_CHANNEL_CHANGE *change_para;
//    struct NIM_CHANNEL_SEARCH *search_para;
    switch (cmd)
    {
        case NIM_DRIVER_CHANNEL_CHANGE:
            change_para = (struct NIM_CHANNEL_CHANGE *)param_list;
            return nim_cxd2834_channel_change_smart(dev, change_para);
            break;
/*        case NIM_DRIVER_CHANNEL_SEARCH:
            search_para = (struct NIM_CHANNEL_SEARCH *)param_list;
            return nim_cxd2834_channel_search(dev, search_para->freq, search_para->bandwidth, \
                search_para->guard_interval, search_para->fft_mode, search_para->modulation, \
                search_para->fec, search_para->usage_type, search_para->inverse, \
                search_para->freq_offset, search_para->priority);
            break;*/
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
static INT32 nim_cxd2834_close(struct nim_device *dev)
{
    //UINT8 data;
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    INT32 result = SUCCESS;

    // tuner power off
    nim_cxd2834_ioctl(dev, NIM_TUNER_POWER_CONTROL, TRUE);

    result = sony_dvb_demod_Finalize (priv);
    if (result != SONY_RESULT_OK)
        result = ERR_FAILUE;

    nim_cxd2834_switch_lock_led(dev, FALSE);
    return result;
}

__ATTRIBUTE_REUSE_
static INT32 nim_cxd2834_open(struct nim_device *dev)
{
    sony_dvb_demod_t * priv = (sony_dvb_demod_t *)dev->priv;
    //struct COFDM_TUNER_CONFIG_API * config_info = &(priv->tc);
    sony_dvb_tune_params_t  TuneParam;
    sony_result_t result = SONY_RESULT_OK;

    // Initialize the demod.
    result = sony_dvb_demod_Initialize (priv);
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "sony_dvb_demod_Initialize() error: %d.\r\n", result);
        return result;
    }
    //Demod can be config now.


    //Demod shall can access tuner through the I2C gateway.
    // Initialize the tuner.
    nim_cxd2834_ioctl(dev, NIM_TUNER_POWER_CONTROL, FALSE); //tuner power on


    //Config TS output mode: SSI/SPI. Default is SPI.
    result = sony_dvb_demod_SetConfig (priv, DEMOD_CONFIG_PARALLEL_SEL, ((NIM_COFDM_TS_SSI == priv->tc.ts_mode)?0:1) );
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
        return result;
    }

    //Confif Data output pin: 0:TS0(pin13), 1:TS7(pin20), default:1.
    result = sony_dvb_demod_SetConfig (priv, DEMOD_CONFIG_SER_DATA_ON_MSB, ((NIM_COFDM_TS_SSI == priv->tc.ts_mode)?0:1) );
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
        return result;
    }

    //TS Error output.
    result = sony_dvb_demod_SetConfig (priv, DEMOD_CONFIG_TSERR_ENABLE, 1);
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "Error: Unable to configure DEMOD_CONFIG_TSERR_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
        return result;
    }

    /* IFAGC setup. Modify to suit connected tuner. */
    /* IFAGC positive, value = 0. */

#ifdef TUNER_IFAGCPOS
    result = sony_dvb_demod_SetConfig (priv, DEMOD_CONFIG_IFAGCNEG, 0);
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
        return result;
    }
#endif

    /* Spectrum Inversion setup. Modify to suit connected tuner. */
    /* Spectrum inverted, value = 1. */
#ifdef TUNER_SPECTRUM_INV
    result = sony_dvb_demod_SetConfig (priv, DEMOD_CONFIG_SPECTRUM_INV, 1);
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "Error: Unable to configure SPECTRUM_INV. (status=%d, %s)\r\n", result, FormatResult(result));
        return result;
    }
#endif

    /* RF level monitoring (RFAIN/RFAGC) enable/disable. */
    /* Default is enabled. 1: Enable, 0: Disable. */
#ifdef TUNER_RFLVLMON_DISABLE
    result = sony_dvb_demod_SetConfig (priv, DEMOD_CONFIG_RFLVMON_ENABLE, 0);
    if (result != SONY_RESULT_OK)
    {
        CXD2834_LOG(priv, "Error: Unable to configure RFLVMON_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
        return result;
    }
#endif

    priv->system = SONY_DVB_SYSTEM_UNKNOWN;
    priv->bandWidth = 8;
    priv->Frequency = 474000;

    MEMSET(&TuneParam,0,sizeof(TuneParam));
    TuneParam.bwMHz = priv->bandWidth;
    TuneParam.centreFreqkHz = priv->Frequency;
    TuneParam.system = SONY_DVB_SYSTEM_DVBT2;
    TuneParam.tParams.usePresets = 0;
    TuneParam.t2Params.dataPLPId = 0;
    result = sony_dvb_demod_Tune (priv, TuneParam.system, TuneParam.bwMHz);
    if (result != SONY_RESULT_OK)
    {
        result = ERR_FAILUE;
        return result;
    }

#if 0
    OSAL_T_CTSK t_ctsk;
    OSAL_ID thread_id;

    t_ctsk.stksz = 0x1000;
    t_ctsk.quantum = 5;
    t_ctsk.task = nim_cxd2834_main_thread;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.para1 = (UINT32)dev;
    thread_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == thread_id)
    {
        ASSERT(0);
    }
#endif

    osal_flag_set(priv->flag_id, NIM_SCAN_END);

    return SUCCESS;
}

static ID create_i2c_mutex(UINT32 i2c_type_id)
{
    struct nim_device *dev;
    sony_dvb_demod_t * priv;
    UINT16 dev_id = 0;
    UINT16 mutex_id = OSAL_INVALID_ID;

    while(1)
    {
        dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, dev_id++);
        if(NULL == dev)
            break;
        priv = (sony_dvb_demod_t *)dev->priv;
//        if((NIM_COFDM_ONLY_MODE == priv->tc.work_mode) && (i2c_type_id == priv->tc.ext_dm_config.i2c_type_id))
//        {
//            mutex_id = priv->i2c_mutex_id;
//            break;
//        }
    }
//    if(OSAL_INVALID_ID == mutex_id)
//        mutex_id = osal_mutex_create();

    mutex_id = osal_mutex_create();
    return mutex_id;
}

/*****************************************************************************
* INT32  nim_m31_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
* Description: cxd2834 initialization
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
__ATTRIBUTE_REUSE_
INT32 nim_cxd2834_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig)
{
    struct nim_device *dev;
    sony_dvb_demod_t *priv;
    DEM_WRITE_READ_TUNER ThroughMode;

    UINT32 TunerIF;

//    if(m_dev_num >= MAX_TUNER_SUPPORT_NUM)
//        return ERR_FAILUE;

    if(NULL == pConfig)
    {
        //CXD2834_DEBUG("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if(NULL == dev)
    {
        //CXD2834_DEBUG("Error: Alloc nim device error!\r\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv = (sony_dvb_demod_t*)MALLOC(sizeof(sony_dvb_demod_t));
    if(NULL == priv)
    {
        dev_free(dev);
        //CXD2834_DEBUG("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    MEMSET((UINT8*)priv, 0, sizeof(sony_dvb_demod_t));

//    // Setup demod I2C interfaces.
    demodI2c.i2c_type_id = pConfig->ext_dm_config.i2c_type_id;
    demodI2c.ReadRegister = sony_i2c_CommonReadRegister;
    demodI2c.WriteRegister = sony_i2c_CommonWriteRegister;
    demodI2c.WriteOneRegister = sony_i2c_CommonWriteOneRegister;
//    demodI2c.gwAddress = 0x00;
//    demodI2c.gwSub = 0x00;      // N/A
//    demodI2c.Read = sony_i2c_Read;
//    demodI2c.Write = sony_i2c_Write;
//    demodI2c.user = (void *) &useri2c;

    // Setup demodulator.
    if (sony_dvb_demod_Create (SONY_DVB_20500kHz, pConfig->ext_dm_config.i2c_base_addr, &demodI2c, priv) != SONY_RESULT_OK)
        return ERR_NO_DEV;

    //Keep Ali config. It must be run after sony_dvb_demod_Create(),because sony_dvb_demod_Create() will empty all setting.
    MEMCPY((UINT8*)&(priv->tc), (UINT8*)pConfig, sizeof(struct COFDM_TUNER_CONFIG_API));

    // Configure demod driver for tuner IF.
    if (priv->tc.tuner_config.w_tuner_if_freq == 4570) //Only support IF 4570KHz.
    {
        TunerIF = 0x1C88DE; //DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57) -> 1870046.2834146341463414634146341 -> 1870046 -> 0x1C88DE.

        priv->iffreq_config.config_DVBT5 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60)
        priv->iffreq_config.config_DVBT6 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60)
        priv->iffreq_config.config_DVBT7 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.20)
        priv->iffreq_config.config_DVBT8 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57)
        priv->iffreq_config.config_DVBC  = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.90)

        // T2 ITB setup.
        priv->iffreq_config.config_DVBT2_5 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60)
        priv->iffreq_config.config_DVBT2_6 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (3.60)
        priv->iffreq_config.config_DVBT2_7 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.20)
        priv->iffreq_config.config_DVBT2_8 = TunerIF;  //DVB_DEMOD_MAKE_IFFREQ_CONFIG (4.57)
    }
    else
    {
        //Unsupported IF setting.
        return ERR_PARA;
    }


    dev->priv = (void*)priv;

    dev->init = NULL;
    dev->open = nim_cxd2834_open;
    dev->stop = nim_cxd2834_close;
    dev->channel_change = NULL; //nim_cxd2834_channel_change
    dev->channel_search = NULL;
    dev->do_ioctl = nim_cxd2834_ioctl;
    dev->do_ioctl_ext = nim_cxd2834_ioctl_ext;

    dev->get_freq = nim_cxd2834_get_freq;
    dev->get_lock = nim_cxd2834_get_lock;
    dev->get_agc = nim_cxd2834_get_AGC;
    dev->get_snr = nim_cxd2834_get_quality;
    dev->get_ber = nim_cxd2834_get_BER;

    dev->get_guard_interval = nim_cxd2834_get_GI;
    dev->get_fftmode = nim_cxd2834_get_fftmode;
    dev->get_modulation = nim_cxd2834_get_modulation;
    dev->get_fec = nim_cxd2834_get_FEC;

#if 0
//added for DVB-T additional elements
    dev->disable = NULL;    //nim_cxd2834_disable;
    dev->get_spectrum_inv = NULL; //nim_cxd2834_get_specinv
#endif

    dev->get_hier= NULL;
    dev->get_freq_offset = NULL;    //nim_cxd2834_get_freq_offset

    priv->flag_id = OSAL_INVALID_ID;
    priv->i2c_mutex_id = OSAL_INVALID_ID;

    priv->flag_id = osal_flag_create(NIM_FLAG_ENABLE);
    if (priv->flag_id==OSAL_INVALID_ID)
    {
        CXD2834_LOG(priv, "%s: no more flag\r\n", __FUNCTION__);
        return ERR_ID_FULL;
    }

    priv->i2c_mutex_id = create_i2c_mutex(priv->tc.ext_dm_config.i2c_type_id);
    if(priv->i2c_mutex_id == OSAL_INVALID_ID)
    {
        CXD2834_LOG(priv, "%s: no more mutex\r\n", __FUNCTION__);
        return ERR_ID_FULL;
    }

    /* Add this device to queue */
    if(dev_register(dev) != SUCCESS)
    {
        //CXD2834_DEBUG("Error: Register nim device error!\r\n");
        FREE(priv);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    if(priv->tc.nim_tuner_init != NULL)
    {
        if(priv->tc.nim_tuner_init(&(priv->tuner_id), &(priv->tc.tuner_config)) != SUCCESS)
        {
            //CXD2834_DEBUG("Error: Init Tuner Failure!\r\n");
            return ERR_NO_DEV;
        }

        ThroughMode.nim_dev_priv = dev->priv;
        ThroughMode.dem_write_read_tuner = (void *)sony_i2c_TunerGateway;
        nim_cxd2834_ioctl(dev, NIM_TUNER_SET_THROUGH_MODE, (UINT32)&ThroughMode);
    }

    return SUCCESS;
}





