/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    tun_rd2004.h
*
*    Description:    Header file of Radio 2004 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  5.09.2007        Joey Gao        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __TUN_RADIO3432_H__
#define __TUN_RADIO3432_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_rd3432_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_rd3432_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth, UINT8 AGC_Time_Const, UINT8 * data, UINT8 _i2c_cmd);
INT32 tun_rd3432_status(UINT32 tuner_id, UINT8 * lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_RADIO3432_H__ */



