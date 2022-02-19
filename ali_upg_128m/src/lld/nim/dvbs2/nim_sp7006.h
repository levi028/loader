/*****************************************************************************
*    Copyright (C) 2010 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: nim_sp7006.h
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2013/1/7        David        Ver 0.1
*   
*****************************************************************************/
#ifndef __NIM_SP7006_H
#define __NIM_SP7006_H

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_sp7006_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_sp7006_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_sp7006_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __NIM_SP7006_H */


