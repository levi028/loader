/*****************************************************************************
*    Copyright (C)2009 Ali Corporation. All Rights Reserved.
*
*    File:    tun_nm120.h
*
*    Description:    Header file of NM120 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  1.11. 2012        Joey Gao        Ver 0.1        Create file.
*****************************************************************************/

#ifndef __TUN_NM120_H__
#define __TUN_NM120_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_nm120_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_nm120_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_nm120_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __TUN_TD1611ALF_H__ */



