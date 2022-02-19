/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_r836.h
*
*    Description:    Header file for alpstdae.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20131201		Joey.Gao	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_R836_H__
#define __LLD_TUN_R836_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_r836_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_r836_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);

INT32 tun_r836_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_R836_H__ */


