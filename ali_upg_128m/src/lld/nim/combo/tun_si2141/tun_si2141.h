/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:    tun_si2141.h
*
*    Description:    Header file for alpstdae.
*    History:
*           Date            Author        Version          Reason
*	    ===================================================
*	1.  20150723	   Joey.Gao	Ver 1.0		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_SI2141_H__
#define __LLD_TUN_SI2141_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_si2141_init_isdbt(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_si2141_init_dvbt(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_si2141_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);

INT32 tun_si2141_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_SI2141_H__ */


