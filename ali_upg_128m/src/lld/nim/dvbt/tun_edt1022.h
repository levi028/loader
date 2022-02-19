/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    tun_edt1022.h
*
*    Description:    Header file of THOMSON DTT7596 TUNER.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  3.16.2007        PengHui    Ver 0.1     Create file.
*****************************************************************************/

#ifndef __TUN_EDT1022_H__
#define __TUN_EDT1022_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_edt1022_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_edt1022_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_edt1022_status(UINT32 tuner_id, UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_EDT1022_H__ */





