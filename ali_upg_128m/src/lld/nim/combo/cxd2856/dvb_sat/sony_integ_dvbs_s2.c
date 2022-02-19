/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_integ_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_seq.h"
#include "sony_demod_dvbs_s2_tune_srs_seq.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_dvbs_s2_Tune (sony_integ_t * pInteg,
                                       sony_dvbs_s2_tune_param_t * pTuneParam)
{
    sony_stopwatch_t stopwatch;
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_DVBS2;
    uint32_t elapsedTime = 0;
    uint8_t isContinue = 1;
    uint32_t timeout = 0;
    sony_demod_lock_result_t lockStatus = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

    SONY_TRACE_ENTER ("sony_integ_dvbs_s2_Tune");

    if ((!pInteg) || (!pInteg->pDemod) ||
        (!pTuneParam) || (pTuneParam->symbolRateKSps == 0)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    sony_atomic_set (&(pInteg->cancel), 0);

    result = sony_demod_dvbs_s2_Tune (pInteg->pDemod, pTuneParam);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pTuneParam->system == SONY_DTV_SYSTEM_ANY){
        dtvSystem = SONY_DTV_SYSTEM_DVBS;
    } else {
        dtvSystem = pTuneParam->system;
    }

    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Tune)) {
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                          pTuneParam->centerFreqKHz,
                                          dtvSystem,
                                          pTuneParam->symbolRateKSps);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    result = sony_demod_TuneEnd(pInteg->pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_stopwatch_start (&stopwatch);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pTuneParam->system == SONY_DTV_SYSTEM_ANY){
        timeout = ((3600000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
    } else {
        timeout = ((3000000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
    }

    isContinue = 1;
    while (isContinue) {
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_dvbs_s2_CheckTSLock (pInteg->pDemod, &lockStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        if (lockStatus == SONY_DEMOD_LOCK_RESULT_LOCKED) {
            /* Lock */
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }

        if (elapsedTime > timeout) {
            isContinue = 0;
        } else {
            result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_TUNE_POLLING_INTERVAL);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
    }

    {
        /* Additional wait sequence for pilot off signal */
        uint8_t plscLock = 0;
        uint8_t pilotOn = 0;
        result = sony_demod_dvbs_s2_monitor_Pilot (pInteg->pDemod, &plscLock, &pilotOn);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if ((plscLock != 0) && (pilotOn == 0)) {
            result = sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if (pTuneParam->system == SONY_DTV_SYSTEM_ANY){
                timeout = ((3600000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
            } else {
                timeout = ((3000000 + (pTuneParam->symbolRateKSps - 1)) / pTuneParam->symbolRateKSps) + 150;
            }

            isContinue = 1;
            while (isContinue) {
                /* Check cancellation. */
                result = sony_integ_CheckCancellation (pInteg);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                result = sony_demod_dvbs_s2_CheckTSLock (pInteg->pDemod, &lockStatus);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                if (lockStatus == SONY_DEMOD_LOCK_RESULT_LOCKED) {
                    /* Lock */
                    SONY_TRACE_RETURN (SONY_RESULT_OK);
                }

                if (elapsedTime > timeout) {
                    isContinue = 0;
                } else {
                    result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_TUNE_POLLING_INTERVAL);
                    if (result != SONY_RESULT_OK){
                        SONY_TRACE_RETURN (result);
                    }
                }
            }
        }
    }
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
}

sony_result_t sony_integ_dvbs_s2_TuneSRS (sony_integ_t * pInteg,
                                          sony_demod_dvbs_s2_tune_srs_seq_t * pSeq,
                                          sony_integ_dvbs_s2_tune_srs_param_t * pParam,
                                          sony_dvbs_s2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;
    SONY_TRACE_ENTER ("sony_integ_dvbs_s2_TuneSRS");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pParam) || (!pTuneParam) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->AGCLevel2AGCdB) ||
        (!pInteg->pTunerSat->Tune)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    sony_atomic_set (&(pInteg->cancel), 0);

    result = sony_demod_dvbs_s2_tune_srs_seq_Initialize (pSeq,
                                                         pInteg->pDemod,
                                                         pParam->centerFreqKHz,
                                                         pParam->freqRange);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    while(pSeq->isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (pSeq->commonParams.waitTime == 0){
            /* Execute one sequence */
            result = sony_demod_dvbs_s2_tune_srs_seq_Sequence (pSeq);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Start stopwatch */
            result= sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* AGC calculate is requested */
            if (pSeq->commonParams.agcInfo.isRequest){

                /* Clear request flag. */
                pSeq->commonParams.agcInfo.isRequest = 0;

                result = pInteg->pTunerSat->AGCLevel2AGCdB (pInteg->pTunerSat,
                                                            pSeq->commonParams.agcInfo.agcLevel,
                                                            &(pSeq->commonParams.agcInfo.agc_x100dB));
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            /* Tune is requested */
            if (pSeq->commonParams.tuneReq.isRequest){
                uint32_t symbolRateKSps = pSeq->commonParams.tuneReq.symbolRateKSps;

                /* Clear request flag. */
                pSeq->commonParams.tuneReq.isRequest = 0;

                /* Symbol rate */
                if (symbolRateKSps == 0) {
                    /* Symbol rate setting for power spectrum */
                    symbolRateKSps = pInteg->pTunerSat->symbolRateKSpsForSpectrum;
                }

                /* Enable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* RF Tune */
                result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                                  pSeq->commonParams.tuneReq.frequencyKHz,
                                                  pSeq->commonParams.tuneReq.system,
                                                  symbolRateKSps);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* Set actual frequency to the driver. */
                pSeq->commonParams.tuneReq.frequencyKHz = pInteg->pTunerSat->frequencyKHz;
                /* Disable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }
        } else {
            /* waiting */
            result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_TUNE_SRS_POLLING_INTERVAL);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if(elapsedTime > pSeq->commonParams.waitTime){
                pSeq->commonParams.waitTime = 0;
            }
        }
    }

    if (pSeq->isDetect){
        /* Tune successful. */
        pTuneParam->centerFreqKHz = pSeq->tuneParam.centerFreqKHz;
        pTuneParam->symbolRateKSps = pSeq->tuneParam.symbolRateKSps;
        pTuneParam->system = pSeq->tuneParam.system;

        /* TS output enable */
        result = sony_demod_dvbs_s2_blindscan_SetTSOut (pInteg->pDemod, 1);
    } else {
        result = SONY_RESULT_ERROR_UNLOCK;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbs_s2_BlindScan (sony_integ_t * pInteg,
                                            sony_demod_dvbs_s2_blindscan_seq_t * pSeq,
                                            sony_integ_dvbs_s2_blindscan_param_t * pParam,
                                            sony_integ_dvbs_s2_blindscan_callback_t callback)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t elapsedTime = 0;
    sony_stopwatch_t stopwatch;
    sony_integ_dvbs_s2_blindscan_result_t blindscanResult;

    SONY_TRACE_ENTER("sony_integ_dvbs_s2_BlindScan");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSeq) || (!pParam) || (!callback) ||
        (!pInteg->pTunerSat) || (!pInteg->pTunerSat->AGCLevel2AGCdB) ||
        (!pInteg->pTunerSat->Tune) || (!pInteg->pTunerSat->Sleep)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    sony_atomic_set (&(pInteg->cancel), 0);

    result = sony_demod_dvbs_s2_blindscan_seq_Initialize (pSeq,
                                                          pInteg->pDemod,
                                                          pParam->minFreqKHz,
                                                          pParam->maxFreqKHz,
                                                          pParam->minSymbolRateKSps,
                                                          pParam->maxSymbolRateKSps);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    while(pSeq->isContinue){
        /* Check cancellation. */
        result = sony_integ_CheckCancellation (pInteg);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (pSeq->commonParams.waitTime == 0){
            /* Execute one sequence */
            result = sony_demod_dvbs_s2_blindscan_seq_Sequence (pSeq);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            /* Start stopwatch */
            result = sony_stopwatch_start (&stopwatch);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if (pSeq->commonParams.agcInfo.isRequest){

                /* Clear request flag. */
                pSeq->commonParams.agcInfo.isRequest = 0;

                result = pInteg->pTunerSat->AGCLevel2AGCdB (pInteg->pTunerSat,
                                                            pSeq->commonParams.agcInfo.agcLevel,
                                                            &(pSeq->commonParams.agcInfo.agc_x100dB));
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            if (pSeq->commonParams.tuneReq.isRequest){
                uint32_t symbolRateKSps = pSeq->commonParams.tuneReq.symbolRateKSps;

                /* Clear request flag. */
                pSeq->commonParams.tuneReq.isRequest = 0;

                /* Symbol rate */
                if (symbolRateKSps == 0) {
                    /* Symbol rate setting for power spectrum */
                    symbolRateKSps = pInteg->pTunerSat->symbolRateKSpsForSpectrum;
                }

                /* Enable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                /* RF Tune */
                result = pInteg->pTunerSat->Tune (pInteg->pTunerSat,
                                                  pSeq->commonParams.tuneReq.frequencyKHz,
                                                  pSeq->commonParams.tuneReq.system,
                                                  symbolRateKSps);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
                pSeq->commonParams.tuneReq.frequencyKHz = pInteg->pTunerSat->frequencyKHz;
                /* Disable the I2C repeater */
                result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            if (pSeq->commonParams.detInfo.isDetect){

                /* Clear detect flag */
                pSeq->commonParams.detInfo.isDetect = 0;

                /* Prepare callback information.(Detected channel) */
                blindscanResult.eventId = SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_DETECT;
                blindscanResult.tuneParam.system = pSeq->commonParams.detInfo.system;
                blindscanResult.tuneParam.centerFreqKHz = pSeq->commonParams.detInfo.centerFreqKHz;
                blindscanResult.tuneParam.symbolRateKSps = pSeq->commonParams.detInfo.symbolRateKSps;

                /* TS output enable */
                result = sony_demod_dvbs_s2_blindscan_SetTSOut (pInteg->pDemod, 1);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }

                /* Callback */
                callback(pInteg, &blindscanResult);

                /* TS output disable */
                result = sony_demod_dvbs_s2_blindscan_SetTSOut (pInteg->pDemod, 0);
                if (result != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (result);
                }
            }

            {
                /* Progress calculation */
                uint8_t progress = 0;
                uint8_t rangeMin = pSeq->commonParams.progressInfo.majorMinProgress;
                uint8_t rangeMax = pSeq->commonParams.progressInfo.majorMaxProgress;
                uint8_t minorProgress = pSeq->commonParams.progressInfo.minorProgress;
                progress = rangeMin + (((rangeMax - rangeMin) * minorProgress) / 100);

                if (pSeq->commonParams.progressInfo.progress < progress){
                    pSeq->commonParams.progressInfo.progress = progress;
                    /* Prepare callback information.(Progress) */
                    blindscanResult.eventId = SONY_INTEG_DVBS_S2_BLINDSCAN_EVENT_PROGRESS;
                    blindscanResult.progress = progress;
                    /* Callback */
                    callback(pInteg, &blindscanResult);
                }
            }

        } else {
            /* waiting */
            result = sony_stopwatch_sleep (&stopwatch, SONY_INTEG_DVBS_S2_BLINDSCAN_POLLING_INTERVAL);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed (&stopwatch, &elapsedTime);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }

            if(elapsedTime > pSeq->commonParams.waitTime){
                pSeq->commonParams.waitTime = 0;
            }
        }
    }

    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = pInteg->pTunerSat->Sleep(pInteg->pTunerSat);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbs_s2_Sleep (pSeq->commonParams.pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_integ_dvbs_s2_monitor_RFLevel (sony_integ_t * pInteg,
                                                  int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t ifagc = 0;
    int32_t gain = 0;
    SONY_TRACE_ENTER("sony_integ_dvbs_s2_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pInteg->pTunerSat) ||
        (!pInteg->pTunerSat->AGCLevel2AGCdB) || (!pRFLeveldB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE)){
        /* This api is accepted in Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_dvbs_s2_monitor_IFAGCOut (pInteg->pDemod, &ifagc);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    result = pInteg->pTunerSat->AGCLevel2AGCdB (pInteg->pTunerSat, ifagc, &gain);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* RF_Level = Input_Ref_Level - RF_Gain
     * For this demodulator
     *  - Input_Ref_Level = 0dB
     *  - Gain is in units dB*100
     * Therefore:
     * RF_Level (dB*1000) =  -10 * RF_Gain
     */
    *pRFLeveldB = gain * (-10);

    SONY_TRACE_RETURN(result);
}
