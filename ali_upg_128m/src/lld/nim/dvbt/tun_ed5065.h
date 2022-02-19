/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    tun_ed5065.h
*
*    Description:    Header file of THOMSON DTT7596 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  6.21.2006        Joey Gao        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __TUN_ED5065_H__
#define __TUN_ED5065_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_ed5065_init(UINT8 tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_ed5065_control(UINT8 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_ed5065_status(UINT8 tuner_id, UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_ED5065_H__ */





