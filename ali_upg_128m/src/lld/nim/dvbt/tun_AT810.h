/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    tun_at810.h
*
*    Description:    Header file of ABILIS AT810 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  2010.8.13        V. Pilloux        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __TUN_AT810_H__
#define __TUN_AT810_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef AT810_DEBUG
#define NIM_PRINTF libc_printf
#else
#define NIM_PRINTF(...)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_AT810_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_AT810_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_AT810_status(UINT32 tuner_id, UINT8 *lock);
INT32 tun_AT810_command(UINT32 tuner_id, INT32 cmd, UINT32 param);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_TD1611ALF_H__ */




