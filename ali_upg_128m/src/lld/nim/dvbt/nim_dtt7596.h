/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim_dtt7596.h
*
*    Description:    Header file of THOMSON DTT7596 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  2.14.2006        Michael Xie        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __NIM_DTT7596_H__
#define __NIM_DTT7596_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>            //51117-01Angus


//#define REF_OSC_FREQ    4 /* 4MHZ */        //51117-01Angus
//#define TUNER_STEP_FREQ    166            //51117-01Angus
//#define REF_DIVISION_RATIO    24        //51117-01Angus

//#define TEST_BIT_MASK 0xC7
//#define ASSIST_REPLACE 0x18
//#define TUNER_SETP 0x4
//#define ATC  0
//#define AGC_TOP 0x20 //112dBuV




#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_DTT7596_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);    //51117-01Angus
INT32 nim_DTT7596_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);    //51117-01Angus
INT32 nim_DTT7596_status(UINT32 tuner_id,UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __LLD_QM7_H__ */

