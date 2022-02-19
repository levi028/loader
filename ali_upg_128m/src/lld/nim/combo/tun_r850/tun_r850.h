/*****************************************************************************
*    Copyright (C)2016 Ali Corporation. All Rights Reserved.
*
*    File:    tun_r850.h
*
*    Description:    Header file for tuner.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20160106		Joey.Gao	Ver 1.0		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_R850_H__
#define __LLD_TUN_R850_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_r850_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_r850_dvbc_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_r850_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_r850_dvbc_control(UINT32 tuner_id, UINT32 freq, UINT32 bandwidth, UINT8 agc_time_const, UINT8 _i2c_cmd);

INT32 tun_r850_status(UINT32 tuner_id, UINT8 *lock);
INT32 tun_r850_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_R850_H__ */


