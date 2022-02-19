/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    tun_max3580.h
*
*    Description:    Header file of MAXIM max3580 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  6.19.2006        Joey Gao        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __TUN_MAX3580_H__
#define __TUN_MAX3580_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_max3580_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_max3580_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_max3580_status(UINT32 tuner_id, UINT8 *lock);
INT32 tun_max3580_powcontrol(UINT32 tuner_id,UINT8 stdby);
INT32 tun_max3580_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#ifdef __cplusplus
}
#endif

#endif //__TUN_MAX3580_H__

