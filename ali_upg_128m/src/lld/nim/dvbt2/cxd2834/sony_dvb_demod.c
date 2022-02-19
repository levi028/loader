/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 4598 $
    $Author: vanstone $

</dev:header>

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    demod hardware state chart
                                       PowerOn
                                          |
                                          |
                                          V
                                  +---------------+
                                  | Shutdown-ULP  |
                 OSCENBN=H ------>|               |
                 (From any state) +---------------+
                                          |
                                          | OSCENBN=L, RESETN=L
                                          V
          +----------------------------------------------------------------+
          |                             Sleep                              |
          |                                                                |
          +----------------------------------------------------------------+
              |       ^        |       ^      |        ^      |        ^
              |       |        |       |      |        |      |        |
            SL2NOT  NOT2SL   SL2NOG  NOG2SL  SL2NOC  NOC2SL  SL2NOV  NOV2SL
              |       |        |       |      |        |      |        |
              V       |        V       |      V        |      V        |
          +----------------------------------------------------------------+
          |    Normal-T    |   Normal-T2   |    Normal-C   |    Normal-C2  |
          |   (DVB-T)      |   (DVB-T2)    |   (DVB-C)     |    (DVB-C2)   |
          +----------------------------------------------------------------+


      SL2NOT : Sleep      -> Normal-T
      NOT2SL : Normal-T   -> Sleep
      SL2NOG : Sleep      -> Normal-T2
      NOG2SL : Normal-T2  -> Sleep
      SL2NOC : Sleep      -> Normal-C
      NOC2SL : Normal-C   -> Sleep
      SL2NOV : Sleep      -> Normal-C2
      NOV2SL : Normal-C2  -> Sleep

------------------------------------------------------------------------------*/
#include "sony_stdlib.h"
#include "sony_dvb_demodT.h"
#include "sony_dvb_demodT2.h"
#include "sony_dvb_demodC.h"
#include "sony_dvb_demodC2.h"
#include "sony_dvb_demod_monitor.h"
#include "sony_dvb_demod_monitorT2.h"
#include "sony_dvb_demod_monitorT.h"
#include "sony_dvb_demod_monitorC.h"

#define DEMOD_MAX_TS_CLK_KHZ   10250   /**< Maximum allowed manual TS clock rate setting. */

#ifndef DEMOD_TUNE_POLL_INTERVAL
#define DEMOD_TUNE_POLL_INTERVAL    10
#endif

/*------------------------------------------------------------------------------
  Local types
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
  Static functions
------------------------------------------------------------------------------*/
static sony_result_t CheckBandwidth (sony_dvb_system_t system, uint8_t bandWidth);
/*------------------------------------------------------------------------------
  Implementation of public functions
------------------------------------------------------------------------------*/

const char *FormatResult (sony_result_t result)
{
    char *pErrorName = "UNKNOWN";
    switch (result) {
    case SONY_RESULT_OK:
        pErrorName = "OK";
        break;
    case SONY_RESULT_ERROR_TIMEOUT:
        pErrorName = "ERROR_TIMEOUT";
        break;
    case SONY_RESULT_ERROR_UNLOCK:
        pErrorName = "ERROR_UNLOCK";
        break;
    case SONY_RESULT_ERROR_CANCEL:
        pErrorName = "ERROR_CANCEL";
        break;
    case SONY_RESULT_ERROR_ARG:
        pErrorName = "ERROR_ARG";
        break;
    case SONY_RESULT_ERROR_I2C:
        pErrorName = "ERROR_I2C";
        break;
    case SONY_RESULT_ERROR_SW_STATE:
        pErrorName = "ERROR_SW_STATE";
        break;
    case SONY_RESULT_ERROR_HW_STATE:
        pErrorName = "ERROR_HW_STATE";
        break;
    case SONY_RESULT_ERROR_RANGE:
        pErrorName = "ERROR_RANGE";
        break;
    case SONY_RESULT_ERROR_NOSUPPORT:
        pErrorName = "ERROR_NOSUPPORT";
        break;
    case SONY_RESULT_ERROR_OTHER:
        pErrorName = "ERROR_OTHER";
        break;
    default:
        pErrorName = "ERROR_UNKNOWN";
        break;
    }
    return pErrorName;
}

const char *FormatTsLock (sony_dvb_demod_lock_result_t tsLockResult)
{
    char *pName = "Unknown";
    switch (tsLockResult) {
    case DEMOD_LOCK_RESULT_NOTDETECT:
        pName = "Not detected";
        break;
    case DEMOD_LOCK_RESULT_LOCKED:
        pName = "Locked";
        break;
    case DEMOD_LOCK_RESULT_UNLOCKED:
        pName = "Unlocked";
        break;
    default:
        break;
    }
    return pName;
}



sony_result_t sony_dvb_demod_Create (sony_dvb_xtal_t xtalFreq,
                                         uint8_t I2CAddress, sony_i2c_t * pDemodI2c, sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_Create");

    if ((!pDemod) || (!pDemodI2c)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pDemodI2c->pDemod = pDemod;
    
    sony_memset (pDemod, 0, sizeof (sony_dvb_demod_t));
    pDemod->i2cAddress = I2CAddress;
    pDemod->i2cAddressXtal = I2CAddress + 4;    //It's fixed to be 'I2CAddress + 4'.
    pDemod->pI2c = pDemodI2c;
    pDemod->xtalFreq = xtalFreq;
    pDemod->state = SONY_DVB_DEMOD_STATE_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  Initialize demod

  Initialize the demod chip and the demod struct.
  The demod chip is set to "Sleep" state.

  NOTE: I2C addresses and pI2c must be set before this function calls.

  sony_dvb_demod_t *pDemod   Instance of demod control struct
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_Initialize (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_Initialize");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Initialize sony_dvb_demod_t */
    pDemod->state = SONY_DVB_DEMOD_STATE_UNKNOWN;
    pDemod->system = SONY_DVB_SYSTEM_UNKNOWN;
    pDemod->bandWidth = 0;

    pDemod->enable_rflvmon = 1; /* ON is default */

    /* set for 20.5 MHz xtal (SD2SL) */
    if (pDemod->xtalFreq == SONY_DVB_20500kHz) {

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressXtal, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressXtal, 0x00, 0x0F) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressXtal, 0x14, 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressXtal, 0x11, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressXtal, 0x14, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        SONY_SLEEP (1);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x02, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set the Pre-RS BER period to the default setting 0x0E */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x60, 0x0E, 0x1F) !=  SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Soft reset */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xFE, 0x01) != SONY_RESULT_OK) {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = sony_dvb_demod_monitor_ChipID (pDemod, &(pDemod->chipId));
    if (result != SONY_RESULT_OK) {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (result);
    }

    if (pDemod->chipId != SONY_DVB_CXD2835 && 
        pDemod->chipId != SONY_DVB_CXD2834 &&
        pDemod->chipId != SONY_DVB_CXD2836) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }
#if (SONY_DVB_DEVICE_CXD2834 == 1)
    if (pDemod->chipId == SONY_DVB_CXD2835) {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

#endif

#if (SONY_DVB_DEVICE_CXD2835 == 1)
    if (pDemod->chipId == SONY_DVB_CXD2834) {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }
#endif
    
    pDemod->state = SONY_DVB_DEMOD_STATE_SLEEP;

    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  Finalize demod

  Finalize the demod chip. The demod chip is set to "ShutDown" state.
  Call sony_dvb_demod_Initialize if re-activate demod.

  sony_dvb_demod_t *pDemod   Instance of demod control struct
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_Finalize (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_Finalize");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    if (pDemod->state == SONY_DVB_DEMOD_STATE_ACTIVE) {
        result = sony_dvb_demod_Sleep (pDemod);
    }
    if (result == SONY_RESULT_OK) {
        pDemod->state = SONY_DVB_DEMOD_STATE_SHUTDOWN;
    }
    else {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_ScanInitialize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_ScanInitialize");
    
    if (!pDemod || !pScanParams) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Ensure the scan parameters are valid. */
    if (pScanParams->endFrequencykHz < pScanParams->startFrequencykHz) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pScanParams->stepFrequencykHz == 0) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Bandwidth check */
    if (pScanParams->multiple) {
        if ((pScanParams->system == SONY_DVB_SYSTEM_DVBT) || (pScanParams->system == SONY_DVB_SYSTEM_DVBT2)) {
            result = CheckBandwidth (SONY_DVB_SYSTEM_DVBT, pScanParams->bwMHz);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
            result = CheckBandwidth (SONY_DVB_SYSTEM_DVBT2, pScanParams->bwMHz);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } 
        else if((pScanParams->system == SONY_DVB_SYSTEM_DVBC) || (pScanParams->system == SONY_DVB_SYSTEM_DVBC2)) {
            result = CheckBandwidth (SONY_DVB_SYSTEM_DVBC, pScanParams->bwMHz);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
            result = CheckBandwidth (SONY_DVB_SYSTEM_DVBC2, pScanParams->bwMHz);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
        else{      
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
    } else {
        result = CheckBandwidth (pScanParams->system, pScanParams->bwMHz);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Configure driver/device for scanning */
    switch (pScanParams->system) {
        /* For DVB-T2 scan, assume DVB-T is scanned also. */
    case SONY_DVB_SYSTEM_DVBT:
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demodT_ScanInitialize (pDemod, pScanParams);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        result = sony_dvb_demodT2_ScanInitialize (pDemod, pScanParams);
        break;
    case SONY_DVB_SYSTEM_DVBC:
        result = sony_dvb_demodC_ScanInitialize (pDemod, pScanParams);
        break;
       /*  dead code
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demodC2_ScanInitialize (pDemod, pScanParams);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;
    */
        /* Intentional fall through */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN (result);

}

sony_result_t sony_dvb_demod_ScanFinalize (sony_dvb_demod_t * pDemod, sony_dvb_scan_params_t * pScanParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_ScanFinalize");

    if (!pDemod || !pScanParams) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Configure driver device for normal operation. */
    switch (pScanParams->system) {
        /* For DVB-T2 scan, assume DVB-T is also scanned. */
    case SONY_DVB_SYSTEM_DVBT:
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demodT_ScanFinalize (pDemod, pScanParams);
        break;
    case SONY_DVB_SYSTEM_DVBC:
        result = sony_dvb_demodC_ScanFinalize (pDemod, pScanParams);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demodC2_ScanFinalize (pDemod, pScanParams);
        break;

        /* Intentional fall through */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  Setup demod

  Set the demod chip to DVB-T2/T/C/C2 specific configurations.

  sony_dvb_demod_t          *pDemod             Instance of demod control struct
  sony_dvb_tune_params_t    *pTuneParams        Instance of Tune paramters
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_TuneInitialize (sony_dvb_demod_t * pDemod, sony_dvb_tune_params_t * pTuneParams)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_TuneInitialize");

    if (!pDemod || !pTuneParams) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Bandwidth check */
    result = CheckBandwidth (pTuneParams->system, pTuneParams->bwMHz);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pTuneParams->system) {
    case SONY_DVB_SYSTEM_DVBT:
        result = sony_dvb_demodT_TuneInitialize (pDemod, pTuneParams);
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demodT2_TuneInitialize (pDemod, pTuneParams);
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demodC2_TuneInitialize (pDemod, pTuneParams);
        break;
    case SONY_DVB_SYSTEM_DVBC:
        break;
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);

}

/*--------------------------------------------------------------------
  Activate demod

  Set the demod chip to DVB-T2/T/C state.

  sony_dvb_demod_t  *pDemod         Instance of demod control struct
  sony_dvb_system_t system          System enum value
  uint8_t           bandWidth   Bandwidth(MHz) 6or7or8(DVB-T/T2), 8(DVB-C/C2)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_Tune (sony_dvb_demod_t * pDemod, sony_dvb_system_t system, uint8_t bwMHz)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_Tune");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Argument check */
    result = CheckBandwidth (system, bwMHz);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Enable Acquisition. */
    if (system != pDemod->system) {
        /* System changed ! */
        /* NOTE: In SLEEP state, pDemod->system is SONY_DVB_SYSTEM_UNKNOWN,
         * so this line will be executed. */
        if (pDemod->state == SONY_DVB_DEMOD_STATE_ACTIVE) {
            /* Set chip to Sleep state */
            result = sony_dvb_demod_Sleep (pDemod);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }

        /* Set new system/bandWidth */
        pDemod->system = system;
        pDemod->bandWidth = bwMHz;
        switch (pDemod->system) {
        case SONY_DVB_SYSTEM_DVBT:
            result = sony_dvb_demodT_Tune (pDemod);
            break;
        case SONY_DVB_SYSTEM_DVBC:
            result = sony_dvb_demodC_Tune (pDemod);
            break;
        case SONY_DVB_SYSTEM_DVBT2:
            result = sony_dvb_demodT2_Tune (pDemod);
            break;
        case SONY_DVB_SYSTEM_DVBC2:
            result = sony_dvb_demodC2_Tune (pDemod);
            break;

            /* Intentional fall through. */
        case SONY_DVB_SYSTEM_UNKNOWN:
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG); /* Unknown system */
        }
    }
    else {
        if (bwMHz != pDemod->bandWidth) {
            /* Bandwidth changed */
            pDemod->system = system;
            pDemod->bandWidth = bwMHz;
            switch (pDemod->system) {

            case SONY_DVB_SYSTEM_DVBT:
                result = sony_dvb_demodT_BandwidthChanged (pDemod);
                break;

            case SONY_DVB_SYSTEM_DVBT2:
                result = sony_dvb_demodT2_BandwidthChanged (pDemod);
                break;

            case SONY_DVB_SYSTEM_DVBC2:
                result = sony_dvb_demodC2_BandwidthChanged (pDemod);
                break;

                /* Intentional fall through. */
            case SONY_DVB_SYSTEM_DVBC:
            case SONY_DVB_SYSTEM_UNKNOWN:
            default:
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
            }
            
        }
    }

    if (result != SONY_RESULT_OK) {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (result);
    }

    if (system == SONY_DVB_SYSTEM_DVBT2) {
        result = sony_dvb_demodT2_InitializeMISOSetting (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_TuneEnd (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_TuneEnd");
    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /*
     * SW reset                           SLV-T, 00h , FEh , 01h Write
     * TS output enable                   SLV-T, 00h , F1h , 00h Write
     */
    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xFE, 0x01) != SONY_RESULT_OK) {
            pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xF1, 0x00) != SONY_RESULT_OK) {
            pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        pDemod->state = SONY_DVB_DEMOD_STATE_ACTIVE;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_TuneFinalize (sony_dvb_demod_t * pDemod, sony_dvb_tune_params_t * pTuneParams, sony_result_t tuneResult)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_TuneFinalize");

    if (!pDemod || !pTuneParams) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pTuneParams->system) {
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demodT2_TuneFinalize (pDemod, pTuneParams, tuneResult);
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        result = sony_dvb_demodC2_TuneFinalize (pDemod, pTuneParams, tuneResult);
        break;
    case SONY_DVB_SYSTEM_DVBT:
        break;
    case SONY_DVB_SYSTEM_DVBC:
        break;
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        break;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_demod_SoftReset (sony_dvb_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_SoftReset");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Soft reset (rstgen_hostrst) */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0xFE, 0x01) != SONY_RESULT_OK) {
        pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pDemod->state = SONY_DVB_DEMOD_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*--------------------------------------------------------------------
  Sleep demod

  Set the demod chip to "Sleep" state.

  sony_dvb_demod_t *pDemod   Instance of demod control struct
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_Sleep (sony_dvb_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_Sleep");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state == SONY_DVB_DEMOD_STATE_ACTIVE) {
        sony_result_t result = SONY_RESULT_OK;

        switch (pDemod->system) {
        case SONY_DVB_SYSTEM_DVBT2:
            result = sony_dvb_demodT2_Sleep (pDemod);
            break;
        case SONY_DVB_SYSTEM_DVBT:
            result = sony_dvb_demodT_Sleep (pDemod);
            break;
        case SONY_DVB_SYSTEM_DVBC:
            result = sony_dvb_demodC_Sleep (pDemod);
            break;
        case SONY_DVB_SYSTEM_DVBC2:
            result = sony_dvb_demodC2_Sleep (pDemod);
            break;

            /* Intentional fall through. */
        case SONY_DVB_SYSTEM_UNKNOWN:
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        if (result == SONY_RESULT_OK) {
            pDemod->state = SONY_DVB_DEMOD_STATE_SLEEP;
            pDemod->system = SONY_DVB_SYSTEM_UNKNOWN;
            pDemod->bandWidth = 0;
        }
        else {
            pDemod->state = SONY_DVB_DEMOD_STATE_INVALID;   /* Should be finalized ... */
            pDemod->system = SONY_DVB_SYSTEM_UNKNOWN;
            pDemod->bandWidth = 0;
        }
        SONY_TRACE_RETURN (result);
    }
    else if (pDemod->state == SONY_DVB_DEMOD_STATE_SLEEP) {
        SONY_TRACE_RETURN (SONY_RESULT_OK);    /* Nothing to do */
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
}

sony_result_t sony_dvb_demod_EnableBlindAcquisition (sony_dvb_demod_t * pDemod, sony_dvb_system_t system)
{

    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_EnableBlindAcquisition");

    if ((!pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if ((pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE)
        && (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Configure the demodulator for blind acquisition in specified system. */
    switch (system) {
        /* Intentional fall through. */
    case SONY_DVB_SYSTEM_DVBC:
    case SONY_DVB_SYSTEM_DVBC2:
        break;
    case SONY_DVB_SYSTEM_DVBT:
        /* Disable Forced Mode/GI */
        result = sony_dvb_demodT_EnableBlindAcquisition (pDemod);
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        result = sony_dvb_demodT2_EnableBlindAcquisition (pDemod);
        break;
        /* Intentional fall through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        result = SONY_RESULT_ERROR_ARG;
    }

    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  Check Demod Lock

  Check lock state of demod. (no wait)
  Integration part Tune function call this function to decide "lock" or not.
  Which state is considered as "lock" is system dependent.
  (DVB-T2: OFDM lock (P1/L1-pre/L1-post), DVB-T : TPS Lock, DVB-C : TS Lock)

  sony_dvb_demod_t               *pDemod   Instance of demod control struct
  sony_dvb_demod_lock_result_t   *pLock    Demod lock state
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_CheckDemodLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    SONY_TRACE_ENTER ("sony_dvb_demod_CheckDemodLock");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    if (pDemod->state == SONY_DVB_DEMOD_STATE_ACTIVE) {
        switch (pDemod->system) {
        case SONY_DVB_SYSTEM_DVBT:
        {
            /* DVB-T, Demod lock == TPS Lock. */
            sony_result_t result = SONY_RESULT_OK;
            result = sony_dvb_demodT_CheckDemodLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
        case SONY_DVB_SYSTEM_DVBC:
        {
            /* DVB-C, Demod lock == AR Lock. */
            sony_result_t result = SONY_RESULT_OK;
            result = sony_dvb_demodC_CheckDemodLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
        case SONY_DVB_SYSTEM_DVBT2:
        {
            /* DVB-T2, Demod lock == OFDM core lock. */
            sony_result_t result = SONY_RESULT_OK;
            result = sony_dvb_demodT2_CheckDemodLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
        case SONY_DVB_SYSTEM_DVBC2:
        {
            /* DVB-C2 */
            sony_result_t result = SONY_RESULT_OK;
            result = sony_dvb_demodC2_CheckDemodLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
            /* Intentional fall through. */
        case SONY_DVB_SYSTEM_UNKNOWN:
        default:
            *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }
    else {
        *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }
}

/*--------------------------------------------------------------------
  Check TS Lock

  Check TS lock state of demod. (no wait)

  sony_dvb_demod_t               *pDemod   Instance of demod control struct
  sony_dvb_demod_lock_result_t   *pLock    TS lock state
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_CheckTSLock (sony_dvb_demod_t * pDemod, sony_dvb_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_dvb_demod_CheckTSLock");

    if (!pDemod || !pLock)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state == SONY_DVB_DEMOD_STATE_ACTIVE) {
        switch (pDemod->system) {
        case SONY_DVB_SYSTEM_DVBT:
        {
            result = sony_dvb_demodT_CheckTSLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
        case SONY_DVB_SYSTEM_DVBC:
        {
            result = sony_dvb_demodC_CheckTSLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
        case SONY_DVB_SYSTEM_DVBT2:
        {
            result = sony_dvb_demodT2_CheckTSLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
        case SONY_DVB_SYSTEM_DVBC2:
        {
            result = sony_dvb_demodC2_CheckTSLock (pDemod, pLock);
            SONY_TRACE_RETURN (result);
        }
            /* Intentional fall through. */
        case SONY_DVB_SYSTEM_UNKNOWN:
        default:
            *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }
    else {
        CXD2834_LOG(pDemod, "%s(): Unlock. pDemod->state = %d .\r\n", __FUNCTION__, pDemod->state);
        *pLock = DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

}

/*--------------------------------------------------------------------
  Configuration of the demod

  Set several parameters for the demod chip and this driver.
  Should be call this function after Initialization.

  sony_dvb_demod_t             *pDemod   Instance of demod control struct
  sony_dvb_SONY_DVB_DEMOD_id_t configId  Enum value defined in this file
  int32_t                      value     Value to be set
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_SetConfig (sony_dvb_demod_t * pDemod, sony_dvb_demod_config_id_t configId, int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_dvb_demod_SetConfig");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Software state check */
    if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE && pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP) {
        /* This api is accepted in ACTIVE or SLEEP states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (configId) {
        /* ======================== Common ============================= */
    case DEMOD_CONFIG_PARALLEL_SEL:
        /* OSERIALEN (Serial/Parallel output select) */
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OSERIALEN (bit 7) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF3,
                                          (uint8_t) (value ? 0x00 : 0x80), 0x80) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_SER_DATA_ON_MSB:
        /* OSEREXCHGB7 (Output pin of TS serial data) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OSEREXCHGB7 (bit 3) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF3,
                                          (uint8_t) (value ? 0x08 : 0x00), 0x08) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_OUTPUT_SEL_MSB:
        /* OWFMT_LSB1STON (Bit order on parallel/serial mode) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OWFMT_LSB1STON (bit 4) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF3,
                                          (uint8_t) (value ? 0x00 : 0x10), 0x10) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSVALID_ACTIVE_HI:
        /* OWFMT_VALINV */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OWFMT_VALIDV (bit 1) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF4,
                                          (uint8_t) (value ? 0x00 : 0x02), 0x02) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSSYNC_ACTIVE_HI:
        /* OWFMT_STIV */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OWFMT_STIV (bit 2) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF4,
                                          (uint8_t) (value ? 0x00 : 0x04), 0x04) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSERR_ACTIVE_HI:
        /* OWFMT_ERRINV */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OWFMT_ERRINV (bit 0) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFA,
                                          (uint8_t) (value ? 0x00 : 0x01), 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_LATCH_ON_POSEDGE:
        /* OWFMT_CKINV (TS Clock invert) */
        /* This register can change only in SLEEP state */
        if (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OWFMT_CKINV (bit 0) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF4,
                                          (uint8_t) (value ? 0x01 : 0x00), 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSCLK_CONT:
        /* OSERCKMODE (TS clock continuous/data only) */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        /* Set OSERCKMODE (bit 1) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF3,
                                          (uint8_t) (value ? 0x00 : 0x02), 0x02) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSCLK_MASK:
        /* OWFMT_CKDISABLE */
        /* Set bank 0x00 */
        if (value > 0x1F) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF5, (uint8_t) value, 0x1F) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSVALID_MASK:
        /* OWFMT_VALDISABLE */
        /* Set bank 0x00 */
        if (value > 0x1F) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF7, (uint8_t) value, 0x1F) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_TSERR_ENABLE:
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* Set OGPIO2_HIZ */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, value ? 0x00 : 0x02, 0x02) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set OREG_GPIO2_IOMODE = 01 */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x89, 0x04, 0x0C) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;

    case DEMOD_CONFIG_TSERR_MASK:    
        if (value > 0x1F) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* OWFMT_ERRDISABLE */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xF8, (uint8_t) value, 0x1F) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;

    case DEMOD_CONFIG_IFAGCNEG:
        /* OCTL_IFAGCNEG */
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* Set OCTL_IFAGCNEG_T (bit 6) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xCB, (uint8_t) (value ? 0x40 : 0x00), 0x40) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* Set OCTL_IFAGCNEG (bit 6) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xCB, (uint8_t) (value ? 0x40 : 0x00), 0x40) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;

    case DEMOD_CONFIG_SPECTRUM_INV:
        /* Spectrum Inversion */
        /* Store the configured sense. */
        pDemod->confSense = value ? DVB_DEMOD_SPECTRUM_INV : DVB_DEMOD_SPECTRUM_NORMAL;
        break;

    case DEMOD_CONFIG_RFLVMON_ENABLE:
        /* RF level monitor enable/disable */
        /* This setting can change only in SLEEP state */
        if (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        pDemod->enable_rflvmon = value ? 1 : 0;
        break;

    case DEMOD_CONFIG_RFAGC_ENABLE:
        /* RFAGC PWM output enable/disable. */
        /* This setting can change only in SLEEP state */
        if (pDemod->state != SONY_DVB_DEMOD_STATE_SLEEP) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set OGPIO1_HIZ */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, value ? 0x00 : 0x08, 0x08) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set OREG_GPIO1_IOMODE = 01 */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x89, 0x10, 0x30) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Enable RFAGC PWM output. */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xBD, value ? 0x00 : 0x02, 0x02) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x01 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>  01h     39h     [0]      8'h00      8'h01     OCTL_RFAGCMANUAL
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x39, value ? 0x01 : 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    
        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* Enable RFAGC PWM output. */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xBD, value ? 0x00 : 0x02, 0x02) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x21 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x21) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>  21h     39h     [0]      8'h00      8'h01     OCTL_RFAGCMANUAL
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x39, value ? 0x01 : 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        
        break;

    case DEMOD_CONFIG_RFAGC_VALUE:
        /* Software state check */
        if (pDemod->state != SONY_DVB_DEMOD_STATE_ACTIVE) {
            /* This api is accepted in ACTIVE state only */
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }

        /* Set bank 0x01 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Write OCTL_RFAGCMANUAL_VAL[11:0] */
        {
            uint8_t data[2];
            data[0] = (uint8_t) (((uint16_t)value >> 8) & 0x0F);
            data[1] = (uint8_t) ((uint16_t)value & 0xFF);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x3A, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* Set bank 0x21 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x21) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Write OCTL_RFAGCMANUAL_VAL[11:0] */
        {
            uint8_t data[2];
            data[0] = (uint8_t) (((uint16_t)value >> 8) & 0x0F);
            data[1] = (uint8_t) ((uint16_t)value & 0xFF);
            /* slave    Bank    Addr    Bit    default    Value          Name
             * ----------------------------------------------------------------------------------
             * <SLV-T>  21h     3Ah     [3:0]    8'h08      8'hxx     OCTL_RFAGCMANUAL_VAL[11:8]
             * <SLV-T>  21h     3Bh     [7:0]    8'h01      8'hxx     OCTL_RFAGCMANUAL_VAL[7:0]
             */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0x3A, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_TS_AUTO_RATE_ENABLE:
        /* Set bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>  00h     E3h     [0]      8'h01      8'h00     OREG_TSIF_AUTO_RATE_EN
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xE3, value ? 0x01 : 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set bank 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

        /* slave    Bank    Addr    Bit    default    Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>  20h     D8h     [0]      8'h01      8'h00     OREG_SP_AUTO_RATE_EN
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xD8, value ? 0x01 : 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;

    case DEMOD_CONFIG_TSIF_SDPR:
        /* Argument checking. */
        if ((value <= 0) || (value > DEMOD_MAX_TS_CLK_KHZ)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }

        {
            uint32_t val = 0x00;
            uint8_t data[3];
            uint32_t uvalue = (uint32_t) value;

            /* Set Bank 0x00. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /*
             * OREG_TSIF_WR_SMOOTH_DP = (82(MHz) / TSCLK(MHz)) * 2^14 ;
             * Notes: 82000 * (2^14) = 1343488000
             */
            val = (1343488000u + uvalue / 2) / uvalue;
            data[0] = (uint8_t) ((val & 0x3F0000) >> 16);
            data[1] = (uint8_t) ((val & 0xFF00) >> 8);
            data[2] = (uint8_t) (val & 0xFF);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xE4, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        {
            uint32_t val = 0x00;
            uint8_t data[4];
            uint32_t uvalue = (uint32_t) value;

            /* Set Bank 0x20. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /*
             * OREG_SP_WR_SMOOTH_DP = (82(MHz) / TSCLK(MHz)) * 2^15 ;
             * Notes: 82000 * (2^15) = 2686976000
             */
            val = (2686976000u + uvalue / 2) / uvalue;
            data[0] = (uint8_t) ((val & 0x0F000000) >> 24);
            data[1] = (uint8_t) ((val & 0xFF0000) >> 16);
            data[2] = (uint8_t) ((val & 0xFF00) >> 8);
            data[3] = (uint8_t) (val & 0xFF);

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddress, 0xDC, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_DVBTC_BERN_PERIOD:
        /* Set the measurment period for Pre-RS BER (DVB-T/C). */
        /* Verify range of value */
        if ((value < 0) || (value > 31)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
      
        {
            uint8_t data = (uint8_t) (value & 0x1F);
            /* Set Bank 0x00. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr   Bit     Name
             * ------------------------------------------------
             * <SLV-T>  00h     60h    [4:0]   OREG_BERN_PERIOD
             */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x60, data, 0x1F) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_DVBT_VBER_PERIOD:
        /* Set the measurment period for Pre-Viterbi BER (DVB-T). */
        /* Verify range of value */
        if ((value < 0) || (value > 7)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
      
        {
            uint8_t data = (uint8_t) (value & 0x07);
            /* Set Bank 0x00. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr   Bit     Name
             * ----------------------------------------------------
             * <SLV-T>  00h     6Fh    [2:0]   OREG_VBER_PERIOD_SEL
             */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x6F, data, 0x07) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_DVBT2C2_BBER_MES:
        /* Set the measurment period for Pre-BCH BER (DVB-T/C) and Post-BCH FER (DVB-T2/C2). */
        /* Verify range of value */
        if ((value < 0) || (value > 15)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
      
        {
            uint8_t data = (uint8_t) (value & 0x0F);
            /* Set Bank 0x20. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr   Bit     Name
             * ---------------------------------------------
             * <SLV-T>  20h     72h    [3:0]   OREG_BBER_MES
             */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x72, data, 0x0F) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_DVBT2C2_LBER_MES:
        /* Set the measurment period for Pre-LDPC BER (DVB-T2/C2). */
        /* Verify range of value */
        if ((value < 0) || (value > 15)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
      
        {
            uint8_t data = (uint8_t) (value & 0x0F);
            /* Set Bank 0x20. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x20) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr   Bit     Name
             * ---------------------------------------------
             * <SLV-T>  20h     6Fh    [3:0]   OREG_LBER_MES
             */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x6F, data, 0x0F) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_DVBTC_PER_MES:
        /* Set the measurment period for PER (DVB-T/C). */
        /* Verify range of value */
        if ((value < 0) || (value > 15)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
      
        {
            uint8_t data = (uint8_t) (value & 0x0F);
            /* Set Bank 0x00. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr   Bit     Name
             * --------------------------------------------
             * <SLV-T>  00h     5Ch    [3:0]   OREG_PER_MES
             */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x5C, data, 0x0F) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_DVBT2C2_PER_MES:
        /* Set the measurment period for PER (DVB-T2/C2). */
        /* Verify range of value */
        if ((value < 0) || (value > 15)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
      
        {
            uint8_t data = (uint8_t) (value & 0x0F);
            /* Set Bank 0x24 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x24) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* slave    Bank    Addr   Bit     Name
             * -----------------------------------------------
             * <SLV-T>  24h     DCh    [3:0]   OREG_SP_PER_MES
             */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xDC, data, 0x0F) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_GPIO_0_DISABLE_PULLUP:
        /* Set Bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        

        /* slave    Bank    Addr    Bit    default    Name
         * -----------------------------------------------------------
         * <SLV-T>  00h     8Bh     [4]    1'b0       OREG_GPIO0_PUDIS
         */
        {
            uint8_t data = value? 0x10 : 0x00;
                
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8B, data, 0x10) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_GPIO_1_DISABLE_PULLUP:
        /* Set Bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        

        /* slave    Bank    Addr    Bit    default    Name
         * -----------------------------------------------------------
         * <SLV-T>  00h     8Bh     [5]    1'b0       OREG_GPIO1_PUDIS
         */
        {
            uint8_t data = value? 0x20 : 0x00;
                
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8B, data, 0x20) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_GPIO_2_DISABLE_PULLUP:
        /* Set Bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        

        /* slave    Bank    Addr    Bit    default    Name
         * -----------------------------------------------------------
         * <SLV-T>  00h     8Bh     [6]    1'b0       OREG_GPIO2_PUDIS
         */
        {
            uint8_t data = value? 0x40 : 0x00;
            
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8B, data, 0x40) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_TS_ENABLE_PULLUP:
        /* Set Bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        

        /* slave    Bank    Addr    Bit    default    Name
         * -----------------------------------------------------------
         * <SLV-T>  00h     8Bh     [3]    1'b0       OREG_TS_PUEN
         */
        {
            uint8_t data = value? 0x08 : 0x00;
            
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8B, data, 0x08) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case DEMOD_CONFIG_TSCLK_CURRENT_12mA:
        /* Set Bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        

        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     8Bh     [1]    1'b0       OTSCLK_C
         */
        {
            uint8_t data = value? 0x02 : 0x00;
                
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8B, data, 0x02) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }        
        }
        break;

    case DEMOD_CONFIG_TS_CURRENT_12mA:
        /* Set Bank 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }        

        /* slave    Bank    Addr    Bit    default    Name
         * ---------------------------------------------------
         * <SLV-T>  00h     8Bh     [0]    1'b0       OTS_C
         */
        {
            uint8_t data = value? 0x01 : 0x00;
            
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8B, data, 0x02) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }        
        }
        break;

        /* ============================= DVB-T ================================== */
    case DEMOD_CONFIG_DVBT_LPSELECT:
    case DEMOD_CONFIG_DVBT_FORCE_MODEGI:
    case DEMOD_CONFIG_DVBT_MODE:
    case DEMOD_CONFIG_DVBT_GI:
        result = sony_dvb_demodT_SetConfig (pDemod, configId, value);
        break;

        /* ============================= DVB-C ================================== */
    case DEMOD_CONFIG_DVBC_SCANMODE:
        result = sony_dvb_demodC_SetConfig (pDemod, configId, value);
        break;

        /* ============================= DVB-C2================================== */
    case DEMOD_CONFIG_DVBC2_SCANMODE:
        result = sony_dvb_demodC2_SetConfig (pDemod, configId, value);
        break;

        /* ============================= All Modes =============================== */
    case DEMOD_CONFIG_SHARED_IF:
        pDemod->sharedIf = value ? 0x01 : 0x00;
        break;

    default:
        /* Unknown ID */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}

/*--------------------------------------------------------------------
  GPIO functions

  GPIO enable/disable and read/write

  sony_dvb_demod_t           *pDemod    Instance of demod control struct
  int                        id         GPIO number (0 or 1 or 2 )
  int                        is_enable  Set enable (1) or disable (0)
  int                        is_write   Read (0) or Write (1)
  int                        *pValue    Output value (0 or 1)
  int                        value      Input value (0 or 1)
--------------------------------------------------------------------*/
sony_result_t sony_dvb_demod_GPIOSetting (sony_dvb_demod_t * pDemod, int id, int is_enable, int is_write)
{

    SONY_TRACE_ENTER ("sony_dvb_demod_GPIOSetting");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
                  +-------------------+-------------------+-------------------+
                  |      GPIO 0       |      GPIO 1       |      GPIO 2       |
                  +-------------------+-------------------+-------------------+
        Bank      |0x00|0x00|0x00|0x00|0x00|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
        Addr      |0xFF|0x89|0x89|0x8E|0xFF|0x89|0x89|0x8E|0xFF|0x89|0x89|0x8E|
        Bit       | [4]| [7]| [6]| [3]| [3]| [5]| [4]| [4]| [1]| [3]| [2]| [5]|
        ----------+----+----+----+----+----+----+----+----+----+----+----+----+
        Hi-Z      |  1 |  0 |  1 |  - |  1 |  0 |  1 |  - |  1 |  0 |  1 |  - |
        GPI       |  1 |  1 |  0 |  1 |  1 |  1 |  0 |  1 |  1 |  1 |  0 |  1 |
        GPO       |  1 |  1 |  0 |  0 |  1 |  1 |  0 |  0 |  1 |  1 |  0 |  0 |
        RFAGC     |  - |  - |  - |  - |  0 |  0 |  1 |  - |  - |  - |  - |  - |
        TSERR     |  - |  - |  - |  - |  - |  - |  - |  - |  0 |  0 |  1 |  - |
    */

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch (id) {
    case 0:                    /* GPIO 0 */
        /* Set OGPIO0_HIZ = 1 */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x10, 0x10) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set OREG_GPIO0_IOMODE */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x89, is_enable ? 0x80 : 0x40, 0xC0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (is_enable) {
            /* Set OREG_GPIEN[0] */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8E, is_write ? 0x00 : 0x08, 0x08) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;
    case 1:                    /* GPIO 1 */
        /* Set OGPIO1_HIZ = 1 */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x08, 0x08) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set OREG_GPIO1_IOMODE */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x89, is_enable ? 0x20 : 0x10, 0x30) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (is_enable) {
            /* Set OREG_GPIEN[1] */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8E, is_write ? 0x00 : 0x10, 0x10) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;
    case 2:                    /* GPIO 2 */
        /* Set OGPIO2_HIZ = 1 */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0xFF, 0x02, 0x02) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set OREG_GPIO2_IOMODE */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x89, is_enable ? 0x08 : 0x04, 0x0C) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (is_enable) {
            /* Set OREG_GPIEN[2] */
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8E, is_write ? 0x00 : 0x20, 0x20) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_GPIORead (sony_dvb_demod_t * pDemod, int id, int *pValue)
{

    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_dvb_demod_GPIORead");

    if (!pDemod || !pValue)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    /* Read o_reg_gpi */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddress, 0x8F, &rdata, 1) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    switch (id) {
    case 0:
        *pValue = (rdata & 0x01) ? 1 : 0;
        break;
    case 1:
        *pValue = (rdata & 0x02) ? 1 : 0;
        break;
    case 2:
        *pValue = (rdata & 0x04) ? 1 : 0;
        break;
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_dvb_demod_GPIOWrite (sony_dvb_demod_t * pDemod, int id, int value)
{

    SONY_TRACE_ENTER ("sony_dvb_demod_GPIOWrite");

    if (!pDemod)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    /* Set bank 0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddress, 0x00, 0x00) != SONY_RESULT_OK)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);

    switch (id) {
    case 0:
        /* OREG_GPO (bit 0) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8E,
                                          (uint8_t) (value ? 0x01 : 0x00), 0x01) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;
    case 1:
        /* OREG_GPO (bit 1) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8E,
                                          (uint8_t) (value ? 0x02 : 0x00), 0x02) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;
    case 2:
        /* OREG_GPO (bit 2) */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddress, 0x8E,
                                          (uint8_t) (value ? 0x04 : 0x00), 0x04) != SONY_RESULT_OK)
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        break;
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_WriteRegisters (sony_dvb_demod_t * pDemod,
                                             uint8_t address, const sony_dvb_demod_reg_t * regs, uint8_t regCount)
{
    uint8_t i = 0;
    uint8_t currentBank = 0xFF;

    SONY_TRACE_ENTER ("sony_demod_WriteRegisters");
    if (!pDemod || !regs)
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    for (i = 0; i < regCount; i++) {

        /* Handle new banks. */
        if (currentBank != regs[i].bank) {
            currentBank = regs[i].bank;
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, address, 0x00, currentBank) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* Write the register values. */
        if (regs[i].mask != 0xFF) {
            if (sony_i2c_SetRegisterBits (pDemod->pI2c, address, regs[i].subAddress, regs[i].value, regs[i].mask) !=
                SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        else {
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, address, regs[i].subAddress, regs[i].value) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
  Implementation of static functions
------------------------------------------------------------------------------*/
static sony_result_t CheckBandwidth (sony_dvb_system_t system, uint8_t bandWidth)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("CheckBandwidth");
    /* Argument check */
    switch (system) {
        case SONY_DVB_SYSTEM_DVBT:
             result = sony_dvb_demodT_CheckBandwidth(bandWidth);
        break;
        case SONY_DVB_SYSTEM_DVBC:
             result = sony_dvb_demodC_CheckBandwidth(bandWidth);
        break;
        case SONY_DVB_SYSTEM_DVBT2:
            result = sony_dvb_demodT2_CheckBandwidth(bandWidth);
        break;
        case SONY_DVB_SYSTEM_DVBC2:
            result = sony_dvb_demodC2_CheckBandwidth(bandWidth);
        break;
        default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG); /* Unknown system */
        
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t WaitDemodLock (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    sony_dvbt2_ofdm_t ofdm;
    
    uint16_t timeout = 0;
    sony_stopwatch_t timer, timer_for_no_signal;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;
    uint32_t elapsed_for_no_signal = 0, timeout_for_no_signal = 45*5; //about 45ms/time.
    SONY_TRACE_ENTER ("WaitDemodLock");

    if ( ! pDemod ) 
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch (pDemod->system) {

    case SONY_DVB_SYSTEM_DVBT2:
        timeout = DVB_DEMOD_DVBT2_WAIT_LOCK;
        break;

    case SONY_DVB_SYSTEM_DVBC2:
        timeout = DVB_DEMOD_DVBC2_WAIT_LOCK;
        break;

    case SONY_DVB_SYSTEM_DVBC:
        /* DVB-C: Wait TS lock timeout. */
        timeout = DVB_DEMOD_DVBC_WAIT_TS_LOCK;
        break;

    case SONY_DVB_SYSTEM_DVBT:
        /* DVB-T: Wait for early unlock valid. */
        timeout = DVB_DEMOD_DVBT_WAIT_LOCK;
        break;

        /* Intentional fall-through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* Wait for demod lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        
        // Check cancellation.
        if (pDemod->autoscan_stop_flag)
        {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
        
        if (pDemod->do_not_wait_t2_signal_locked)
        {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
        
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
  
        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_dvb_demod_CheckDemodLock (pDemod, &lock);
//        if (result != SONY_RESULT_OK) {
//            SONY_TRACE_RETURN (result);
//        }
        if (result == SONY_RESULT_OK)
        {
            switch (lock) {
            case DEMOD_LOCK_RESULT_LOCKED:
                elapsed_for_no_signal = 0;  //reset the no signal counter.
                if (pDemod->system == SONY_DVB_SYSTEM_DVBT2) {
                    result = sony_dvb_demodT2_OptimizeMISO(pDemod, &ofdm);
                    if (result != SONY_RESULT_OK) 
                    {
//                        SONY_TRACE_RETURN (result);
                        CXD2834_LOG(pDemod, "%s: DEMOD_LOCK_RESULT_LOCKED, but sony_dvb_demodT2_OptimizeMISO()=%d. elapsed = %d\r\n", __FUNCTION__, result, elapsed);
                        lock = DEMOD_LOCK_RESULT_NOTDETECT;
                        break;
                    }
                }
                SONY_TRACE_RETURN (SONY_RESULT_OK);

            case DEMOD_LOCK_RESULT_UNLOCKED:
                result = SONY_RESULT_ERROR_UNLOCK;
                CXD2834_LOG(pDemod, "%s: DEMOD_LOCK_RESULT_UNLOCKED. elapsed = %d\r\n", __FUNCTION__, elapsed);
                if (elapsed_for_no_signal == 0)
                {
                    result = sony_stopwatch_start(&timer_for_no_signal);
                    if (result != SONY_RESULT_OK) 
                    {
                        SONY_TRACE_RETURN (result);
                    }
                    
                    elapsed_for_no_signal = 1; //make it start count in the next time.
                }
                else
                {
                    result = sony_stopwatch_elapsed(&timer_for_no_signal, \
                                                    &elapsed_for_no_signal);
                    if (result != SONY_RESULT_OK) 
                    {
                        SONY_TRACE_RETURN (result);
                    }
                }
                
                if (elapsed_for_no_signal >= timeout_for_no_signal)
                {
                    SONY_TRACE_RETURN (result);     //quit for no signal quickly.
                }
                break;              /* continue waiting... */
            case DEMOD_LOCK_RESULT_NOTDETECT:
                result = SONY_RESULT_ERROR_UNLOCK;
                CXD2834_LOG(pDemod, "%s: DEMOD_LOCK_RESULT_NOTDETECT. elapsed = %d\r\n", __FUNCTION__, elapsed);
                break;              /* continue waiting... */
            default:
                break;              /* continue waiting... */
            }
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, DEMOD_TUNE_POLL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }

    }
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_dvb_integration_WaitTSLock (sony_dvb_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;

    uint16_t timeout = 0;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;
    
    SONY_TRACE_ENTER ("sony_dvb_integration_WaitTSLock");

    /* Argument verification. */
    if ( !pDemod )
    {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch (pDemod->system) {
    case SONY_DVB_SYSTEM_DVBC:
        timeout = DVB_DEMOD_DVBC_WAIT_TS_LOCK;
        break;
    case SONY_DVB_SYSTEM_DVBT2:
        timeout = DVB_DEMOD_DVBT2_WAIT_TS_LOCK;
        break;
    case SONY_DVB_SYSTEM_DVBT:
        timeout = DVB_DEMOD_DVBT_WAIT_TS_LOCK;
        break;
    case SONY_DVB_SYSTEM_DVBC2:
        timeout = DVB_DEMOD_DVBC2_WAIT_TS_LOCK;
        break;

        /* Intentional fall-through. */
    case SONY_DVB_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Wait for TS lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {

        // Check cancellation.
        if (pDemod->autoscan_stop_flag)
        {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
        
        if (pDemod->do_not_wait_t2_signal_locked)
        {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }

        result = sony_stopwatch_elapsed(&timer,&elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_dvb_demod_CheckTSLock (pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        switch (lock) {
        case DEMOD_LOCK_RESULT_LOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        case DEMOD_LOCK_RESULT_UNLOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);

            /* Intentional fall-through. */
        case DEMOD_LOCK_RESULT_NOTDETECT:
        default:
            break;              /* continue waiting... */
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, DEMOD_TUNE_POLL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } 
        else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_dvb_signalinfo(sony_dvb_demod_t * pDemod, uint8_t *pQuality,uint32_t *pIFAGC,uint32_t *pBER)
{	
	sony_result_t result = SONY_RESULT_OK;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    UINT8	cn=0;
	
    if((pDemod == NULL)||(pQuality == NULL)||(pIFAGC == NULL)||(pBER == NULL))
	   	return SONY_RESULT_ERROR_ARG;

	cn = 0;	
	*pQuality = 0;
    *pIFAGC = 2804; //large AGC, refer to SONYT2_Tuner_AGC_Table[]
	*pBER = 1;
    
	result = sony_dvb_demod_CheckTSLock(pDemod, &lock);
    if( lock != DEMOD_LOCK_RESULT_LOCKED)
	{
        CXD2834_LOG(pDemod, "%s(): sony_dvb_demod_CheckTSLock()=%d, failed!\n", __FUNCTION__, result);
	}
	else	
	{
    	switch(pDemod->system)
    	{
    		case SONY_DVB_SYSTEM_DVBT2:
    		{
    			result = sony_dvb_demod_monitorT2_Quality(pDemod,pQuality);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "sony_dvb_demod_monitorT2_Quality()=%d, failed!\n", result);
    			}
    			result = sony_dvb_demod_monitorT2_IFAGCOut(pDemod,pIFAGC);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "sony_dvb_demod_monitorT2_IFAGCOut()=%d, failed!\n", result);
    			}
    			result = sony_dvb_demod_monitorT2_PreBCHBER(pDemod,pBER);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "sony_dvb_demod_monitorT2_PreBCHBER()=%d, failed!\n", result);
    			}
    			
    		}	
    		break;
    		case SONY_DVB_SYSTEM_DVBT:
    		{
    			result = sony_dvb_demod_monitorT_Quality(pDemod,pQuality);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "sony_dvb_demod_monitorT_Quality()=%d, failed!\n", result);
    			}
    			result = sony_dvb_demod_monitorT_IFAGCOut(pDemod,pIFAGC);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "sony_dvb_demod_monitorT_IFAGCOut()=%d, failed!\n", result);
    			}
    			result = sony_dvb_demod_monitorT_PreRSBER(pDemod,pBER);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "sony_dvb_demod_monitorT_PreRSBER()=%d, failed!\n", result);
    			}
    			*pBER = (*pBER)*100;
    		}	
    		break;
    		case SONY_DVB_SYSTEM_DVBC:
    		{
    			int32_t pSNR = 0;
    			result = sony_dvb_demod_monitorC_SNR(pDemod,&pSNR);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "[sony_dvb_demod_monitorT_Quality] -> failed!\n");
    			}
    			if(pSNR<0)
    				*pQuality = 0;
    			else
    				*pQuality = pSNR;
    			result = sony_dvb_demod_monitorC_IFAGCOut(pDemod,pIFAGC);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "[sony_dvb_demod_monitorT_IFAGCOut] -> failed!\n");
    			}
    			result = sony_dvb_demod_monitorC_PreRSBER(pDemod,pBER);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2834_LOG(pDemod, "[sony_dvb_demod_monitorT_PreRSBER] -> failed!\n");
    			}
    			*pBER = (*pBER)*100;
    		}	
    		break;
    		default:
    			return SONY_RESULT_ERROR_ARG;
    	}
	}

		if(*pQuality>90)
			*pQuality =90;
		if(*pBER == 0)
			*pBER = 1;
		while(*pBER/10 >=1)
		{
			cn++;
			(*pBER) /= 10;
		}

	return  SONY_RESULT_OK;
}

