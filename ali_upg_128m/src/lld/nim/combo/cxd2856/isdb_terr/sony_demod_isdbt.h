/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_isdbt.h

          This file provides the demodulator control interface specific to ISDB-T.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_ISDBT_H
#define SONY_DEMOD_ISDBT_H

#if 1 //ali modify
#include "../sony_common.h"
#include "../sony_demod.h"
#ifdef SONY_DEMOD_SUPPORT_ISDBT
#include "sony_isdbt.h"
#endif
#else //sony
#include "sony_common.h"
#include "sony_demod.h"
#endif

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The tune parameters for a ISDB-T signal
*/
typedef struct sony_isdbt_tune_param_t {
    uint32_t centerFreqKHz;             /**< Center frequency(kHz) of the ISDB-T channel */
    sony_dtv_bandwidth_t bandwidth;   /**< Bandwidth of the ISDB-T channel */
} sony_isdbt_tune_param_t;

/**
 @brief The preset information for a ISDB-T signal
*/
typedef struct sony_demod_isdbt_preset_info_t {
    uint8_t data[13];                   /**< TMCC information defined by ISDB-T */
} sony_demod_isdbt_preset_info_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Enable acquisition on the demodulator for ISDB-T channels.  Called from
        the integration layer ::sony_integ_isdbt_Tune API.

 @param pDemod The demodulator instance
 @param pTuneParam The tune parameters.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_Tune (sony_demod_t * pDemod,
                                     sony_isdbt_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into ::SONY_DEMOD_STATE_SLEEP state.  Can be called
        from Active, Shutdown or Sleep states.  Called from the integration layer
        ::sony_integ_Sleep API.

 @param pDemod  The demodulator instance

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_Sleep (sony_demod_t * pDemod);

/**
 @brief Check ISDB-T demodulator lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_isdbt_monitor_SyncStat instead
        to check current lock status.

 @param pDemod  The demodulator instance
 @param pLock Demod lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_CheckDemodLock (sony_demod_t * pDemod,
                                               sony_demod_lock_result_t * pLock);

/**
 @brief Check ISDB-T TS lock status.

 @note  This API is for checking lock status in tuning stage.
        After tuning (while receiving the signal),
        please use sony_demod_isdbt_monitor_SyncStat instead
        to check current lock status.

 @param pDemod  The demodulator instance
 @param pLock TS lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_CheckTSLock (sony_demod_t * pDemod,
                                            sony_demod_lock_result_t * pLock);


/**
 @brief Set Preset Infomation to enable Fast acquisition mode. Preset information
        can be obtained from ::sony_demod_isdbt_monitor_PresetInfo API.

 @param pDemod The demodulator instance
 @param pPresetInfo Preset information in TMCC.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_SetPreset (sony_demod_t * pDemod,
                                          sony_demod_isdbt_preset_info_t* pPresetInfo);


/**
 @brief Enable acquisition on the demodulator for ISDB-T channels in EWS mode.
        Called from the integration layer ::sony_integ_isdbt_EWSTune API.

 @param pDemod     The demodulator instance
 @param pTuneParam Tune parameters structure.

 @return SONY_RESULT_OK if successful.
 */
sony_result_t sony_demod_isdbt_EWSTune(sony_demod_t*            pDemod,
                                       sony_isdbt_tune_param_t * pTuneParam);

/**
 @brief Completes the demodulator acquisition setup only the case for ISDB-T EWS tune.
        Must be called after system specific demod and RF tunes.
        Called from the integration layer ::sony_integ_isdbt_EWSTune API.

 @param pDemod The demodulator instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_isdbt_EWSTuneEnd (sony_demod_t * pDemod);


#endif /* SONY_DEMOD_ISDBT_H */
