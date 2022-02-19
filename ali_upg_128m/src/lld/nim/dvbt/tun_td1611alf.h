/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    tun_td1611alf.h
*
*    Description:    Header file of THOMSON DTT7596 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  6.21.2006        Joey Gao        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __TUN_TD1611ALF_H__
#define __TUN_TD1611ALF_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_td1611alf_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_td1611alf_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_td1611alf_status(UINT32 tuner_id, UINT8 *lock);
INT32 tun_td1611alf_cal_agc(UINT32 tuner_id, UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_TD1611ALF_H__ */




