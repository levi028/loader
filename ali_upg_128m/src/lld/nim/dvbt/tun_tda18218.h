/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_tda18218.h
*
*    Description:    Header file for alpstdae.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*
*****************************************************************************/

#ifndef __LLD_TUN_TDA18218_H__
#define __LLD_TUN_TDA18218_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_tda18218_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_tda18218_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const, UINT8 *data, UINT8 _i2c_cmd);
INT32 tun_tda18218_status(UINT32 tuner_id, UINT8 *lock);
unsigned int  tun_tda18218write    (UINT32  uAddress, UINT32  uSubAddress, UINT32  uNbData, UINT32* pDataBuff);
/*Bool    tun_tda18252writebit(UInt32  uAddress,UInt32  uSubAddress,UInt32  uMaskValue,UInt32  uValue);*/unsigned int    tun_tda18218read(UINT32  uAddress, UINT32  uSubAddress,UINT32  uNbData,UINT32* pDataBuff);
INT32 tun_tda18218_powcontrol(UINT32 tuner_id,UINT8 stdby);
INT32 tun_tda18218_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_TDA18218_H__ */


