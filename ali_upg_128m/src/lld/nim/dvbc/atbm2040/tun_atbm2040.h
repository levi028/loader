/*****************************************************************************
*    Copyright (C)2017 Gospell Corporation. All Rights Reserved.
*
*    File:    tun_atbm2040.h
*
*    Description:    Header file for atbm tuner.
*    History:
*           Date                   Athor                   Version                           Reason
*    =========           =========   =============	     =============
*	1.  2019/09/20	    Liang Ming  	  Ver 0.1		                 Create file.
*****************************************************************************/

#ifndef __LLD_TUN_ATBM2040__
#define __LLD_TUN_ATBM2040__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif
INT32 tun_atbm2040_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_atbm2040_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);

INT32 tun_atbm2040_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_ATBM2040__ */
