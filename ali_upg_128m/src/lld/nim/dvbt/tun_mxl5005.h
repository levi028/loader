/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_MxL5005.h
*
*    Description:    Header file of MAXLINEAR MxL5005 TUNER.
*    History:
*        Date          Athor        Version          Reason
*        ============    =============    =========    =================
*    1.  8.1.2007            PengHui          Ver 0.1         Create file.
*****************************************************************************/

#ifndef __TUN_MxL5005_H__
#define __TUN_MxL5005_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#include "MXL5005_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_mxl5005_init(UINT32* tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl5005_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
INT32 tun_mxl5005_status(UINT32 tuner_id, UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_MxL5005_H__ */



