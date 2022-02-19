/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim_ed5056.h
*
*    Description:    Header file of sharp QM1D9PB0007 for Sharp module.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  Mar.25.2004        David Wang        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __LLD_IX2410_H__
#define __LLD_IX2410_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>            //51117-01Angus


//#define REF_OSC_FREQ    4 /* 4MHZ */        //51117-01Angus
//#define TUNER_STEP_FREQ    166            //51117-01Angus


#ifdef __cplusplus
extern "C"
{
#endif


INT32 nim_DTT75411_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);    //51117-01Angus
INT32 nim_DTT75411_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth, UINT8 AGC_Time_Const, UINT8 *data, UINT8 _i2c_cmd);    //51117-01Angus
INT32 nim_DTT75411_status(UINT32 tuner_id,UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __LLD_QM7_H__ */
