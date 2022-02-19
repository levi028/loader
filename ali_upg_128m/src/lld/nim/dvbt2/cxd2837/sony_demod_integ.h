/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-08-10 10:34:51 #$
  File Revision : $Revision:: 5923 $
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_integ.h

          This file provides the integration layer control interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_INTEG_H
#define SONY_DEMOD_INTEG_H


#include "sony_demod_dvbt.h"
#include "sony_demod_dvbt2.h"
#include "sony_demod_dvbc.h"


#ifndef DEMOD_TUNE_POLL_INTERVAL
#define DEMOD_TUNE_POLL_INTERVAL    10
#endif


/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

sony_result_t sony_integ_dvbt_Tune(sony_demod_t * pDemod, sony_dvbt_tune_param_t * pTuneParam, BOOL NeedToConfigTuner);

sony_result_t sony_integ_dvbt2_Tune(sony_demod_t * pDemod, sony_dvbt2_tune_param_t * pTuneParam,BOOL NeedToConfigTuner);

#endif

