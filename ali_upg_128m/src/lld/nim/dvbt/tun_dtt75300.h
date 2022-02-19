/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    nim_dtt75300.h
*
*    Description:    Header file of THOMSON DTT75300(75301) TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  6.19.2006        Joey Gao        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __NIM_DTT75300_H__
#define __NIM_DTT75300_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_dtt75300_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_dtt75300_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_dtt75300_status(UINT32 tuner_id,UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif //__NIM_DTT75300_H__
