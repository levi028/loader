/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/

#include "sony_common.h"
#include "sony_demod.h"
#include "sony_integ.h"

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_Create (sony_integ_t * pInteg,
                                 sony_demod_t * pDemod,
                                 sony_demod_create_param_t * pCreateParam,
                                 sony_i2c_t * pDemodI2c
#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
                                 ,sony_tuner_terr_cable_t * pTunerTerrCable
#endif
#ifdef SONY_DEMOD_SUPPORT_SAT
                                 ,sony_tuner_sat_t * pTunerSat
#endif
#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
                                 ,sony_lnbc_t * pLnbc
#endif
                                 )
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_Create");

    if ((!pInteg) || (!pDemod) || (!pCreateParam) || (!pDemodI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Create demodulator instance */
    result = sony_demod_Create (pDemod, pCreateParam, pDemodI2c);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Populate the integration structure */
    pInteg->pDemod = pDemod;

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    pInteg->pTunerTerrCable = pTunerTerrCable;
#endif

#ifdef SONY_DEMOD_SUPPORT_SAT
    pInteg->pTunerSat = pTunerSat;
#endif

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    pInteg->pLnbc = pLnbc;
#endif

    sony_atomic_set (&(pInteg->cancel), 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_Initialize (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_Initialize");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_Initialize (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Enable the I2C repeater */
    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Initialize)) {
        /* Initialize the terrestrial / cable tuner. */
        result = pInteg->pTunerTerrCable->Initialize (pInteg->pTunerTerrCable);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_SAT
    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Initialize)) {
        /* Initialize the satellite tuner. */
        result = pInteg->pTunerSat->Initialize (pInteg->pTunerSat);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

    /* Disable the I2C repeater */
    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    if ((pInteg->pLnbc) && (pInteg->pLnbc->Initialize)) {
        /* Initialize the lnb controller. */
        result = pInteg->pLnbc->Initialize (pInteg->pLnbc);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    if ((pInteg->pLnbc) && (pInteg->pLnbc->Sleep)) {
        /* Sleep the lnb controller */
        result = pInteg->pLnbc->Sleep (pInteg->pLnbc);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_Sleep (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_Sleep");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_SHUTDOWN)) {
        /* This api is accepted in Sleep, Active and Shutdown states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Call the demodulator Sleep function */
    result = sony_demod_Sleep (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Enable the I2C repeater */
    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Sleep)) {
        /* Call the terrestrial / cable tuner Sleep implementation */
        result = pInteg->pTunerTerrCable->Sleep (pInteg->pTunerTerrCable);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_SAT
    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Sleep)) {
        /* Call the satellite tuner Sleep implementation */
        result = pInteg->pTunerSat->Sleep (pInteg->pTunerSat);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

    /* Disable the I2C repeater */
    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_Shutdown (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_Shutdown");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_SHUTDOWN)) {
        /* This api is accepted in Sleep, Active and Shutdown states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->state == SONY_DEMOD_STATE_SHUTDOWN) {
        /* Nothing to do */
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    /* At first, set demod to Sleep state to stop TS output. */
    result = sony_demod_Sleep (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Enable the I2C repeater */
    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Shutdown)) {
        /* Call the terrestrial / cable tuner Shutdown implementation */
        result = pInteg->pTunerTerrCable->Shutdown (pInteg->pTunerTerrCable);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

#ifdef SONY_DEMOD_SUPPORT_SAT
    if ((pInteg->pTunerSat) && (pInteg->pTunerSat->Shutdown)) {
        /* Call the satellite tuner Shutdown implementation */
        result = pInteg->pTunerSat->Shutdown (pInteg->pTunerSat);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

    /* Disable the I2C repeater */
    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
    if ((pInteg->pLnbc) && (pInteg->pLnbc->Sleep)) {
        /* Sleep the lnb controller. */
        result = pInteg->pLnbc->Sleep (pInteg->pLnbc);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
#endif

    /* After controlling tuner and LNBC, set demod to Shutdown state. */
    result = sony_demod_Shutdown (pInteg->pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_Cancel (sony_integ_t * pInteg)
{
    SONY_TRACE_ENTER ("sony_integ_Cancel");

    /* Argument verification. */
    if (!pInteg) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set the cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 1);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_integ_CheckCancellation (sony_integ_t * pInteg)
{
    SONY_TRACE_ENTER ("sony_integ_CheckCancellation");

    /* Argument verification. */
    if (!pInteg) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Check the cancellation flag. */
    if (sony_atomic_read (&(pInteg->cancel)) != 0) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
